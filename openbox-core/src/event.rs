//! Event Handling System
//! 
//! Async event handling using Tokio for concurrent processing of
//! X11/Wayland events with proper ordering and priority.

use crate::client::{ClientId, ClientManager};
use crate::error::{OpenboxError, Result};
use crate::focus::{FocusReason, FocusTracker};
use crate::stacking::StackingManager;
use crate::types::*;
use parking_lot::Mutex;
use std::collections::{BTreeMap, VecDeque};
use std::sync::Arc;
use tokio::sync::{mpsc, oneshot, RwLock};
use tokio::time::{Duration, Instant};
use tracing::{debug, error, info, trace, warn};

/// Event priority levels
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum EventPriority {
    /// Critical events (window destruction, etc.)
    Critical = 0,
    /// User input events
    Input = 1,
    /// Focus changes
    Focus = 2,
    /// Configuration changes
    Configure = 3,
    /// Property changes
    Property = 4,
    /// Low priority updates
    Low = 5,
}

/// Event wrapper with metadata
#[derive(Debug)]
pub struct Event {
    /// Event type
    pub event_type: EventType,
    /// Client involved (if any)
    pub client_id: Option<ClientId>,
    /// Window handle
    pub window: WindowHandle,
    /// Priority
    pub priority: EventPriority,
    /// Timestamp
    pub timestamp: Instant,
    /// Event data
    pub data: EventData,
    /// Sequence number for ordering
    pub sequence: u64,
}

/// Event-specific data
#[derive(Debug, Clone)]
pub enum EventData {
    /// No data
    None,
    /// Configure event data
    Configure {
        x: i32,
        y: i32,
        width: u32,
        height: u32,
        above_sibling: Option<WindowHandle>,
    },
    /// Property change data
    Property {
        atom: Atom,
        deleted: bool,
    },
    /// Key event data
    Key {
        keycode: KeyCode,
        keysym: KeySym,
        modifiers: Modifiers,
    },
    /// Button event data
    Button {
        button: MouseButton,
        modifiers: Modifiers,
        x: i32,
        y: i32,
    },
    /// Motion event data
    Motion {
        x: i32,
        y: i32,
        x_root: i32,
        y_root: i32,
    },
    /// Client message data
    ClientMessage {
        message_type: Atom,
        data: ClientMessageData,
    },
    /// Crossing event data (enter/leave)
    Crossing {
        mode: CrossingMode,
        detail: CrossingDetail,
    },
}

/// Crossing mode
#[derive(Debug, Clone, Copy)]
pub enum CrossingMode {
    Normal,
    Grab,
    Ungrab,
}

/// Crossing detail
#[derive(Debug, Clone, Copy)]
pub enum CrossingDetail {
    Ancestor,
    Virtual,
    Inferior,
    Nonlinear,
    NonlinearVirtual,
}

/// Event dispatcher
pub struct EventDispatcher {
    /// Event queue (priority -> events)
    event_queue: Arc<Mutex<BTreeMap<EventPriority, VecDeque<Event>>>>,
    /// Sequence counter
    sequence_counter: Arc<Mutex<u64>>,
    /// Client manager reference
    clients: Arc<RwLock<ClientManager>>,
    /// Focus tracker reference
    focus: Arc<RwLock<FocusTracker>>,
    /// Stacking manager reference
    stacking: Arc<RwLock<StackingManager>>,
    /// Event sender
    sender: mpsc::UnboundedSender<Event>,
    /// Event receiver
    receiver: Arc<Mutex<Option<mpsc::UnboundedReceiver<Event>>>>,
    /// Running flag
    running: Arc<Mutex<bool>>,
    /// Event handlers
    handlers: Arc<Mutex<EventHandlers>>,
}

/// Event handlers registry
#[derive(Default)]
struct EventHandlers {
    /// Global handlers (run for all events)
    global: Vec<Box<dyn Fn(&Event) -> Result<bool> + Send>>,
    /// Per-type handlers
    by_type: BTreeMap<EventType, Vec<Box<dyn Fn(&Event) -> Result<bool> + Send>>>,
    /// Per-client handlers
    by_client: BTreeMap<ClientId, Vec<Box<dyn Fn(&Event) -> Result<bool> + Send>>>,
}

impl EventDispatcher {
    /// Create a new event dispatcher
    pub fn new(
        clients: Arc<RwLock<ClientManager>>,
        focus: Arc<RwLock<FocusTracker>>,
        stacking: Arc<RwLock<StackingManager>>,
    ) -> Self {
        let (sender, receiver) = mpsc::unbounded_channel();
        
        Self {
            event_queue: Arc::new(Mutex::new(BTreeMap::new())),
            sequence_counter: Arc::new(Mutex::new(0)),
            clients,
            focus,
            stacking,
            sender,
            receiver: Arc::new(Mutex::new(Some(receiver))),
            running: Arc::new(Mutex::new(false)),
            handlers: Arc::new(Mutex::new(EventHandlers::default())),
        }
    }

    /// Start the event loop
    pub async fn run(&self) -> Result<()> {
        let mut receiver = self.receiver.lock().take()
            .ok_or_else(|| OpenboxError::Event("Event loop already running".to_string()))?;

        *self.running.lock() = true;
        info!("Event loop started");

        while *self.running.lock() {
            // Process events from channel
            tokio::select! {
                Some(event) = receiver.recv() => {
                    self.process_event(event).await?;
                }
                _ = tokio::time::sleep(Duration::from_millis(1)) => {
                    // Process any queued events
                    self.process_queued_events().await?;
                }
            }
        }

        info!("Event loop stopped");
        Ok(())
    }

    /// Stop the event loop
    pub fn stop(&self) {
        info!("Stopping event loop");
        *self.running.lock() = false;
    }

    /// Send an event to the dispatcher
    pub fn send(&self, event: Event) -> Result<()> {
        self.sender.send(event).map_err(|_| {
            OpenboxError::Event("Failed to send event".to_string())
        })
    }

    /// Queue an event for processing
    pub fn queue(&self, event_type: EventType, window: WindowHandle, data: EventData) {
        let priority = Self::priority_for_event(&event_type);
        let sequence = {
            let mut counter = self.sequence_counter.lock();
            *counter += 1;
            *counter
        };

        let event = Event {
            event_type,
            client_id: None, // Will be resolved during processing
            window,
            priority,
            timestamp: Instant::now(),
            data,
            sequence,
        };

        let mut queue = self.event_queue.lock();
        queue.entry(priority).or_default().push_back(event);
    }

    /// Register a global event handler
    pub fn register_global_handler<F>(&self, handler: F)
    where
        F: Fn(&Event) -> Result<bool> + Send + 'static,
    {
        self.handlers.lock().global.push(Box::new(handler));
    }

    /// Register a handler for a specific event type
    pub fn register_type_handler<F>(&self, event_type: EventType, handler: F)
    where
        F: Fn(&Event) -> Result<bool> + Send + 'static,
    {
        self.handlers.lock()
            .by_type
            .entry(event_type)
            .or_default()
            .push(Box::new(handler));
    }

    /// Process a single event
    async fn process_event(&self, mut event: Event) -> Result<()> {
        trace!("Processing event: {:?} (seq: {})", event.event_type, event.sequence);

        // Resolve client ID from window handle
        {
            let clients = self.clients.read().await;
            if let Some(client) = clients.get_by_handle(event.window) {
                // We can't easily get the ClientId from the RwLock<Client>
                // In real implementation, this would be handled differently
                // For now, we'll look it up differently
            }
        }

        // Run global handlers
        let handlers = self.handlers.lock();
        for handler in &handlers.global {
            match handler(&event) {
                Ok(true) => {
                    // Event was handled, stop propagation
                    trace!("Event handled by global handler");
                    return Ok(());
                }
                Ok(false) => continue,
                Err(e) => {
                    warn!("Global handler error: {}", e);
                }
            }
        }

        // Run type-specific handlers
        if let Some(type_handlers) = handlers.by_type.get(&event.event_type) {
            for handler in type_handlers {
                match handler(&event) {
                    Ok(true) => {
                        trace!("Event handled by type handler");
                        return Ok(());
                    }
                    Ok(false) => continue,
                    Err(e) => {
                        warn!("Type handler error: {}", e);
                    }
                }
            }
        }

        // Default handling
        self.default_handle(&event).await
    }

    /// Process queued events by priority
    async fn process_queued_events(&self) -> Result<()> {
        let mut queue = self.event_queue.lock();
        
        // Process in priority order
        for priority in [EventPriority::Critical, EventPriority::Input, 
                        EventPriority::Focus, EventPriority::Configure,
                        EventPriority::Property, EventPriority::Low] {
            if let Some(events) = queue.get_mut(&priority) {
                while let Some(event) = events.pop_front() {
                    drop(queue); // Release lock during processing
                    self.process_event(event).await?;
                    queue = self.event_queue.lock();
                }
            }
        }

        Ok(())
    }

    /// Default event handling
    async fn default_handle(&self, event: &Event) -> Result<()> {
        match event.event_type {
            EventType::Create => {
                debug!("Window created: {:?}", event.window);
                // Client manager will handle this
            }
            EventType::Destroy => {
                debug!("Window destroyed: {:?}", event.window);
                // Clean up client
            }
            EventType::Map => {
                debug!("Window mapped: {:?}", event.window);
                // Handle window mapping
            }
            EventType::Unmap => {
                debug!("Window unmapped: {:?}", event.window);
            }
            EventType::Configure => {
                if let EventData::Configure { x, y, width, height, .. } = &event.data {
                    trace!("Configure: {}x{}+{}+{}", width, height, x, y);
                }
            }
            EventType::FocusIn => {
                // Update focus tracking
            }
            EventType::Enter => {
                // Handle mouse enter for focus follows mouse
                if let Some(client_id) = event.client_id {
                    let mut focus = self.focus.write().await;
                    focus.on_mouse_enter(client_id, &*self.clients.read().await)?;
                }
            }
            EventType::ButtonPress => {
                // Handle mouse clicks
                if let EventData::Button { button, modifiers, x, y } = &event.data {
                    trace!("Button {:?} at ({}, {}) mods={:?}", button, x, y, modifiers);
                }
            }
            EventType::KeyPress => {
                // Handle key bindings
                if let EventData::Key { keycode, keysym, modifiers } = &event.data {
                    trace!("Key {} (sym: {}) mods={:?}", keycode, keysym, modifiers);
                }
            }
            _ => {
                trace!("Unhandled event: {:?}", event.event_type);
            }
        }

        Ok(())
    }

    /// Get priority for event type
    fn priority_for_event(event_type: &EventType) -> EventPriority {
        match event_type {
            EventType::Destroy => EventPriority::Critical,
            EventType::Create => EventPriority::Critical,
            EventType::Map => EventPriority::Configure,
            EventType::Unmap => EventPriority::Configure,
            EventType::Enter | EventType::Leave => EventPriority::Input,
            EventType::ButtonPress | EventType::ButtonRelease => EventPriority::Input,
            EventType::Motion => EventPriority::Input,
            EventType::KeyPress | EventType::KeyRelease => EventPriority::Input,
            EventType::FocusIn | EventType::FocusOut => EventPriority::Focus,
            EventType::Configure => EventPriority::Configure,
            EventType::Property => EventPriority::Property,
            _ => EventPriority::Low,
        }
    }

    /// Wait for a specific event type (with timeout)
    pub async fn wait_for_event(
        &self,
        event_type: EventType,
        timeout: Duration,
    ) -> Result<Option<Event>> {
        let (tx, rx) = oneshot::channel();
        let tx = Arc::new(Mutex::new(Some(tx)));

        // Register temporary handler
        let type_clone = event_type;
        self.register_type_handler(event_type, move |event| {
            if event.event_type == type_clone {
                if let Some(tx) = tx.lock().take() {
                    let _ = tx.send(event.clone());
                }
                Ok(true)
            } else {
                Ok(false)
            }
        });

        // Wait with timeout
        match tokio::time::timeout(timeout, rx).await {
            Ok(Ok(event)) => Ok(Some(event)),
            Ok(Err(_)) => Ok(None),
            Err(_) => Ok(None),
        }
    }
}

impl Clone for EventDispatcher {
    fn clone(&self) -> Self {
        // Create new channel for cloned dispatcher
        let (sender, receiver) = mpsc::unbounded_channel();
        
        Self {
            event_queue: Arc::clone(&self.event_queue),
            sequence_counter: Arc::clone(&self.sequence_counter),
            clients: Arc::clone(&self.clients),
            focus: Arc::clone(&self.focus),
            stacking: Arc::clone(&self.stacking),
            sender,
            receiver: Arc::new(Mutex::new(Some(receiver))),
            running: Arc::clone(&self.running),
            handlers: Arc::clone(&self.handlers),
        }
    }
}

/// Event builder for convenient event creation
pub struct EventBuilder {
    event_type: EventType,
    window: WindowHandle,
    priority: Option<EventPriority>,
    data: EventData,
}

impl EventBuilder {
    /// Create a new event builder
    pub fn new(event_type: EventType, window: WindowHandle) -> Self {
        Self {
            event_type,
            window,
            priority: None,
            data: EventData::None,
        }
    }

    /// Set priority
    pub fn priority(mut self, priority: EventPriority) -> Self {
        self.priority = Some(priority);
        self
    }

    /// Set configure data
    pub fn configure(
        mut self,
        x: i32,
        y: i32,
        width: u32,
        height: u32,
    ) -> Self {
        self.data = EventData::Configure {
            x,
            y,
            width,
            height,
            above_sibling: None,
        };
        self
    }

    /// Set button data
    pub fn button(mut self, button: MouseButton, modifiers: Modifiers, x: i32, y: i32) -> Self {
        self.data = EventData::Button {
            button,
            modifiers,
            x,
            y,
        };
        self
    }

    /// Build the event
    pub fn build(self, sequence: u64) -> Event {
        Event {
            event_type: self.event_type,
            client_id: None,
            window: self.window,
            priority: self.priority.unwrap_or(EventPriority::Low),
            timestamp: Instant::now(),
            data: self.data,
            sequence,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::client::ClientManager;
    use crate::focus::FocusTracker;
    use crate::stacking::StackingManager;

    fn create_test_dispatcher() -> EventDispatcher {
        let clients = Arc::new(RwLock::new(ClientManager::new()));
        let focus = Arc::new(RwLock::new(FocusTracker::new()));
        let stacking = Arc::new(RwLock::new(StackingManager::new()));
        
        EventDispatcher::new(clients, focus, stacking)
    }

    #[tokio::test]
    async fn test_event_priority() {
        assert_eq!(
            EventDispatcher::priority_for_event(&EventType::Destroy),
            EventPriority::Critical
        );
        assert_eq!(
            EventDispatcher::priority_for_event(&EventType::ButtonPress),
            EventPriority::Input
        );
        assert_eq!(
            EventDispatcher::priority_for_event(&EventType::Property),
            EventPriority::Property
        );
    }

    #[tokio::test]
    async fn test_event_building() {
        let event = EventBuilder::new(EventType::Configure, WindowHandle::Generic(1))
            .configure(100, 200, 800, 600)
            .priority(EventPriority::Configure)
            .build(1);

        assert_eq!(event.event_type, EventType::Configure);
        assert_eq!(event.priority, EventPriority::Configure);
        
        if let EventData::Configure { x, y, width, height, .. } = event.data {
            assert_eq!(x, 100);
            assert_eq!(y, 200);
            assert_eq!(width, 800);
            assert_eq!(height, 600);
        } else {
            panic!("Expected Configure data");
        }
    }
}
