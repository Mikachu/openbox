//! Focus Tracking and Management
//! 
//! Implements multiple focus models with proper focus stealing prevention
//! and focus history tracking.

use crate::client::{ClientId, ClientManager, WindowState};
use crate::error::{OpenboxError, Result};
use crate::types::*;
use parking_lot::RwLock;
use std::collections::{HashMap, VecDeque};
use std::sync::Arc;
use tracing::{debug, info, trace, warn};

/// Focus model types
#[derive(Debug, Clone, Copy, PartialEq, Eq, serde::Serialize, serde::Deserialize)]
pub enum FocusModel {
    /// Click to focus - window focuses on mouse click
    ClickToFocus,
    /// Focus follows mouse - window focuses on mouse enter
    FocusFollowsMouse,
    /// Focus under mouse - focus stays under mouse pointer
    FocusUnderMouse,
    /// Strict focus follows mouse - immediate focus on enter
    StrictFocusFollowsMouse,
}

/// Focus change reasons
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum FocusReason {
    /// User clicked on window
    Click,
    /// Mouse entered window
    Enter,
    /// Keyboard navigation
    Keyboard,
    /// Window mapping
    Map,
    /// Window unmapping
    Unmap,
    /// Window destruction
    Destroy,
    /// Desktop switch
    DesktopChange,
    /// Application request
    ApplicationRequest,
    /// Startup notification
    Startup,
    /// Focus restoration
    Restore,
}

/// Focus policy configuration
#[derive(Debug, Clone)]
pub struct FocusPolicy {
    /// Focus model
    pub model: FocusModel,
    /// Delay before focus follows mouse (ms)
    pub focus_delay: u64,
    /// Whether to raise window on focus
    pub raise_on_focus: bool,
    /// Whether to focus new windows
    pub focus_new_windows: bool,
    /// Whether to focus last window on desktop switch
    pub focus_last_on_desktop: bool,
    /// Whether to prevent focus stealing
    pub prevent_focus_stealing: bool,
    /// Focus stealing prevention delay (ms)
    pub focus_steal_delay: u64,
    /// Whether to honor _NET_ACTIVE_WINDOW
    pub honor_active_window: bool,
}

impl Default for FocusPolicy {
    fn default() -> Self {
        Self {
            model: FocusModel::ClickToFocus,
            focus_delay: 0,
            raise_on_focus: false,
            focus_new_windows: true,
            focus_last_on_desktop: true,
            prevent_focus_stealing: true,
            focus_steal_delay: 200,
            honor_active_window: true,
        }
    }
}

/// Focus tracker state
pub struct FocusTracker {
    /// Current focus policy
    policy: FocusPolicy,
    /// Currently focused client
    focused: Option<ClientId>,
    /// Previous focused client
    previous_focus: Option<ClientId>,
    /// Focus history per desktop
    desktop_focus_history: HashMap<i32, VecDeque<ClientId>>,
    /// Focus history stack (global)
    focus_stack: Vec<ClientId>,
    /// Pending focus (for delayed focus)
    pending_focus: Option<(ClientId, std::time::Instant)>,
    /// Last focus time
    last_focus_time: std::time::Instant,
    /// Focus serial counter
    focus_serial: u64,
    /// Focus candidates by window
    focus_candidates: HashMap<ClientId, FocusCandidate>,
    /// Whether focus is currently being set
    setting_focus: bool,
    /// Startup focus protection
    startup_focus_protection: Option<std::time::Instant>,
}

/// Focus candidate information
#[derive(Debug, Clone)]
struct FocusCandidate {
    /// When the focus was requested
    pub time: std::time::Instant,
    /// Source of focus request
    pub source: FocusSource,
    /// Whether this is an urgent request
    pub urgent: bool,
}

/// Focus source
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum FocusSource {
    /// From application (_NET_ACTIVE_WINDOW)
    Application,
    /// From user action
    User,
    /// From window manager
    WindowManager,
    /// From pager/taskbar
    Pager,
}

impl FocusTracker {
    /// Create a new focus tracker
    pub fn new() -> Self {
        Self {
            policy: FocusPolicy::default(),
            focused: None,
            previous_focus: None,
            desktop_focus_history: HashMap::new(),
            focus_stack: Vec::new(),
            pending_focus: None,
            last_focus_time: std::time::Instant::now(),
            focus_serial: 0,
            focus_candidates: HashMap::new(),
            setting_focus: false,
            startup_focus_protection: Some(std::time::Instant::now()),
        }
    }

    /// Set focus policy
    pub fn set_policy(&mut self, policy: FocusPolicy) {
        info!("Setting focus policy: {:?}", policy.model);
        self.policy = policy;
    }

    /// Set focus model
    pub fn set_model(&mut self, model: FocusModel) {
        info!("Setting focus model: {:?}", model);
        self.policy.model = model;
    }

    /// Get current focus model
    pub fn model(&self) -> FocusModel {
        self.policy.model
    }

    /// Get currently focused client
    pub fn focused(&self) -> Option<ClientId> {
        self.focused
    }

    /// Get previously focused client
    pub fn previous_focus(&self) -> Option<ClientId> {
        self.previous_focus
    }

    /// Check if a client can be focused
    pub fn can_focus(&self, client_id: ClientId, clients: &ClientManager) -> bool {
        // Check if client exists and is manageable
        let Some(client) = clients.get(client_id) else {
            return false;
        };
        
        let c = client.read();
        
        // Can't focus destroyed or unmapped windows
        if c.destroyed || !c.mapped {
            return false;
        }
        
        // Check if window wants focus
        if c.states.contains(&WindowState::Hidden) {
            return false;
        }
        
        // Check focus stealing prevention
        if self.policy.prevent_focus_stealing && !self.allow_focus_steal(client_id) {
            return false;
        }
        
        true
    }

    /// Request focus for a client
    pub fn request_focus(
        &mut self,
        client_id: ClientId,
        source: FocusSource,
        urgent: bool,
        time: Option<std::time::Instant>,
    ) -> Result<bool> {
        // Check if we're already focused there
        if self.focused == Some(client_id) {
            trace!("Client {:?} already focused", client_id);
            return Ok(false);
        }

        // Record focus candidate
        let candidate = FocusCandidate {
            time: time.unwrap_or_else(std::time::Instant::now),
            source,
            urgent,
        };
        
        self.focus_candidates.insert(client_id, candidate);
        
        // Check if we should focus immediately or delay
        if self.should_focus_now(client_id, urgent) {
            self.set_focus(client_id, FocusReason::ApplicationRequest)?;
            Ok(true)
        } else {
            // Queue for later
            debug!("Queuing focus request for {:?}", client_id);
            self.pending_focus = Some((client_id, std::time::Instant::now()));
            Ok(false)
        }
    }

    /// Set focus to a specific client
    pub fn set_focus(&mut self, client_id: ClientId, reason: FocusReason) -> Result<()> {
        if self.setting_focus {
            warn!("Reentrant focus set detected");
            return Ok(());
        }

        if self.focused == Some(client_id) {
            trace!("Focus already on {:?}", client_id);
            return Ok(());
        }

        self.setting_focus = true;
        
        // Update previous focus
        if let Some(old) = self.focused {
            self.previous_focus = Some(old);
            self.focus_stack.retain(|&id| id != old);
            self.focus_stack.push(old);
        }

        // Set new focus
        debug!("Setting focus to {:?} (reason: {:?})", client_id, reason);
        self.focused = Some(client_id);
        self.last_focus_time = std::time::Instant::now();
        self.focus_serial += 1;
        
        // Clear pending focus if it matches
        if self.pending_focus.map(|(id, _)| id) == Some(client_id) {
            self.pending_focus = None;
        }

        self.setting_focus = false;
        
        info!("Focus changed to {:?} (serial: {})", client_id, self.focus_serial);
        
        Ok(())
    }

    /// Clear focus (set to none)
    pub fn clear_focus(&mut self) -> Result<()> {
        if self.focused.is_none() {
            return Ok(());
        }

        debug!("Clearing focus");
        
        if let Some(old) = self.focused {
            self.previous_focus = Some(old);
        }
        
        self.focused = None;
        
        Ok(())
    }

    /// Focus the next client in the stack
    pub fn focus_next(&mut self, clients: &ClientManager) -> Result<bool> {
        let candidates: Vec<_> = clients.get_all_clients()
            .into_iter()
            .filter(|&id| self.can_focus(id, clients))
            .collect();

        if candidates.is_empty() {
            self.clear_focus()?;
            return Ok(false);
        }

        // Find current position
        let current_pos = self.focused
            .and_then(|current| candidates.iter().position(|&id| id == current))
            .unwrap_or(0);

        // Get next
        let next_pos = (current_pos + 1) % candidates.len();
        let next = candidates[next_pos];

        self.set_focus(next, FocusReason::Keyboard)?;
        
        Ok(true)
    }

    /// Focus the previous client in the stack
    pub fn focus_prev(&mut self, clients: &ClientManager) -> Result<bool> {
        let candidates: Vec<_> = clients.get_all_clients()
            .into_iter()
            .filter(|&id| self.can_focus(id, clients))
            .collect();

        if candidates.is_empty() {
            self.clear_focus()?;
            return Ok(false);
        }

        // Find current position
        let current_pos = self.focused
            .and_then(|current| candidates.iter().position(|&id| id == current))
            .unwrap_or(0);

        // Get previous
        let prev_pos = if current_pos == 0 {
            candidates.len() - 1
        } else {
            current_pos - 1
        };
        let prev = candidates[prev_pos];

        self.set_focus(prev, FocusReason::Keyboard)?;
        
        Ok(true)
    }

    /// Focus the last focused client
    pub fn focus_last(&mut self) -> Result<bool> {
        if let Some(last) = self.previous_focus {
            self.set_focus(last, FocusReason::Restore)?;
            Ok(true)
        } else {
            Ok(false)
        }
    }

    /// Handle desktop change
    pub fn on_desktop_change(&mut self, new_desktop: i32, clients: &ClientManager) -> Result<()> {
        // Save current focus for old desktop
        if let Some(focused) = self.focused {
            let history = self.desktop_focus_history
                .entry(new_desktop)
                .or_insert_with(VecDeque::new);
            history.retain(|&id| id != focused);
            history.push_front(focused);
        }

        // Clear focus
        self.clear_focus()?;

        // Restore focus on new desktop if configured
        if self.policy.focus_last_on_desktop {
            if let Some(history) = self.desktop_focus_history.get(&new_desktop) {
                for &client_id in history.iter() {
                    if self.can_focus(client_id, clients) {
                        self.set_focus(client_id, FocusReason::DesktopChange)?;
                        break;
                    }
                }
            }
        }

        Ok(())
    }

    /// Handle mouse enter event
    pub fn on_mouse_enter(
        &mut self,
        client_id: ClientId,
        clients: &ClientManager,
    ) -> Result<bool> {
        match self.policy.model {
            FocusModel::FocusFollowsMouse | FocusModel::StrictFocusFollowsMouse => {
                // Check delay
                if self.policy.focus_delay > 0 && 
                   self.policy.model != FocusModel::StrictFocusFollowsMouse {
                    // TODO: Schedule delayed focus
                    self.pending_focus = Some((
                        client_id,
                        std::time::Instant::now() + 
                            std::time::Duration::from_millis(self.policy.focus_delay)
                    ));
                    Ok(false)
                } else {
                    if self.can_focus(client_id, clients) {
                        self.set_focus(client_id, FocusReason::Enter)?;
                        Ok(true)
                    } else {
                        Ok(false)
                    }
                }
            }
            FocusModel::FocusUnderMouse => {
                // Just update without actually focusing
                Ok(false)
            }
            FocusModel::ClickToFocus => {
                // Ignore enter in click-to-focus mode
                Ok(false)
            }
        }
    }

    /// Handle mouse click event
    pub fn on_mouse_click(
        &mut self,
        client_id: ClientId,
        clients: &ClientManager,
    ) -> Result<bool> {
        match self.policy.model {
            FocusModel::ClickToFocus | FocusModel::FocusFollowsMouse => {
                if self.can_focus(client_id, clients) {
                    let changed = self.set_focus(client_id, FocusReason::Click)?;
                    
                    if self.policy.raise_on_focus {
                        // TODO: Signal to raise window
                    }
                    
                    Ok(changed)
                } else {
                    Ok(false)
                }
            }
            _ => {
                // Other modes focus on enter, not click
                Ok(false)
            }
        }
    }

    /// Process pending focus operations
    pub fn process_pending(&mut self) -> Result<Option<ClientId>> {
        if let Some((client_id, when)) = self.pending_focus {
            if std::time::Instant::now() >= when {
                self.pending_focus = None;
                self.set_focus(client_id, FocusReason::Restore)?;
                return Ok(Some(client_id));
            }
        }
        
        Ok(None)
    }

    /// Check if we should focus now or delay
    fn should_focus_now(&self, client_id: ClientId, urgent: bool) -> bool {
        if urgent {
            return true;
        }

        if let Some(candidate) = self.focus_candidates.get(&client_id) {
            // User-initiated focus requests are immediate
            if candidate.source == FocusSource::User {
                return true;
            }

            // Check focus stealing prevention
            if self.policy.prevent_focus_stealing {
                // If user has been active recently, don't steal
                let user_active = std::time::Instant::now()
                    .duration_since(self.last_focus_time)
                    .as_millis() < self.policy.focus_steal_delay as u128;
                
                if user_active && candidate.source == FocusSource::Application {
                    return false;
                }
            }
        }

        true
    }

    /// Check if focus steal is allowed
    fn allow_focus_steal(&self, client_id: ClientId) -> bool {
        if let Some(candidate) = self.focus_candidates.get(&client_id) {
            if candidate.urgent {
                return true;
            }
            
            // Window manager can always steal
            if candidate.source == FocusSource::WindowManager {
                return true;
            }
        }
        
        // Check if we're in startup protection period
        if let Some(startup_time) = self.startup_focus_protection {
            let elapsed = std::time::Instant::now().duration_since(startup_time);
            if elapsed.as_secs() < 5 {
                return false;
            }
        }
        
        true
    }

    /// Get focus serial number
    pub fn focus_serial(&self) -> u64 {
        self.focus_serial
    }

    /// Get time since last focus change
    pub fn time_since_focus(&self) -> std::time::Duration {
        std::time::Instant::now().duration_since(self.last_focus_time)
    }

    /// End startup protection
    pub fn end_startup_protection(&mut self) {
        self.startup_focus_protection = None;
        info!("Ended startup focus protection");
    }
}

impl Default for FocusTracker {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_focus_model() {
        let mut tracker = FocusTracker::new();
        
        tracker.set_model(FocusModel::ClickToFocus);
        assert_eq!(tracker.model(), FocusModel::ClickToFocus);
        
        tracker.set_model(FocusModel::FocusFollowsMouse);
        assert_eq!(tracker.model(), FocusModel::FocusFollowsMouse);
    }

    #[test]
    fn test_focus_changes() {
        let mut tracker = FocusTracker::new();
        
        // Initial state
        assert!(tracker.focused().is_none());
        
        // Set focus
        let id1 = ClientId::default();
        tracker.set_focus(id1, FocusReason::Click).unwrap();
        assert_eq!(tracker.focused(), Some(id1));
        
        // Change focus
        let id2 = ClientId::default();
        tracker.set_focus(id2, FocusReason::Click).unwrap();
        assert_eq!(tracker.focused(), Some(id2));
        assert_eq!(tracker.previous_focus(), Some(id1));
        
        // Clear focus
        tracker.clear_focus().unwrap();
        assert!(tracker.focused().is_none());
    }

    #[test]
n    fn test_focus_serial() {\n        let mut tracker = FocusTracker::new();\n        \n        let initial = tracker.focus_serial();\n        tracker.set_focus(ClientId::default(), FocusReason::Click).unwrap();\n        assert_eq!(tracker.focus_serial(), initial + 1);\n        \n        tracker.set_focus(ClientId::default(), FocusReason::Click).unwrap();\n        assert_eq!(tracker.focus_serial(), initial + 2);\n    }\n}
