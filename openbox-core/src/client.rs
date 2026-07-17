//! Client (Window) Management
//! 
//! Provides memory-safe management of X11/Wayland windows with proper
//! lifecycle tracking and state management.
//! 
//! BUG FIXES 4.0.1:
//! - Fixed memory leak in client destruction (not releasing Arc references)
//! - Fixed race condition in client list iteration
//! - Added proper cleanup of client resources

use crate::error::{OpenboxError, Result};
use crate::types::*;
use parking_lot::{RwLock, Mutex};
use slotmap::{new_key_type, SlotMap};
use std::collections::{HashMap, HashSet, VecDeque};
use std::sync::{Arc, Weak};
use tracing::{debug, info, warn};

new_key_type! {
    /// Unique identifier for a client/window
    pub struct ClientId;
}

/// Window type classification
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, serde::Serialize, serde::Deserialize)]
pub enum WindowType {
    /// Normal application window
    Normal,
    /// Dialog window
    Dialog,
    /// Utility window (toolbar, palette)
    Utility,
    /// Menu window
    Menu,
    /// Toolbar window
    Toolbar,
    /// Splash screen
    Splash,
    /// Dock/panel window
    Dock,
    /// Desktop window
    Desktop,
    /// Notification window
    Notification,
    /// Unknown type
    Unknown,
}

/// Window state flags
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, serde::Serialize, serde::Deserialize)]
pub enum WindowState {
    /// Window is modal
    Modal,
    /// Window is sticky (appears on all desktops)
    Sticky,
    /// Window is maximized vertically
    MaximizedVert,
    /// Window is maximized horizontally
    MaximizedHorz,
    /// Window is shaded (rolled up)
    Shaded,
    /// Window skips taskbar
    SkipTaskbar,
    /// Window skips pager
    SkipPager,
    /// Window is hidden
    Hidden,
    /// Window is fullscreen
    Fullscreen,
    /// Window is above others
    Above,
    /// Window is below others
    Below,
    /// Window demands attention
    DemandsAttention,
    /// Window is focused
    Focused,
}

/// Client/window data structure
#[derive(Debug)]
pub struct Client {
    /// Unique identifier
    pub id: ClientId,
    /// Platform-specific window handle (X11 Window ID or Wayland surface)
    pub handle: WindowHandle,
    /// Window title
    pub title: RwLock<String>,
    /// Window class (application class)
    pub class: RwLock<String>,
    /// Window instance name
    pub instance: RwLock<String>,
    /// Window role
    pub role: RwLock<Option<String>>,
    /// Window type
    pub window_type: RwLock<WindowType>,
    /// Current states
    pub states: RwLock<HashSet<WindowState>>,
    /// Window geometry (position and size)
    pub geometry: RwLock<Geometry>,
    /// Desktop/workspace number
    pub desktop: RwLock<u32>,
    /// Parent window (for dialogs, etc.)
    pub parent: RwLock<Option<ClientId>>,
    /// Transient windows (children)
    pub transient: RwLock<Vec<ClientId>>,
    /// Client is being destroyed (prevents re-entrancy)
    pub destroying: Mutex<bool>,
    /// Creation timestamp
    pub created_at: std::time::Instant,
    /// Last update timestamp
    pub updated_at: RwLock<std::time::Instant>,
}

impl Client {
    /// Create a new client
    pub fn new(id: ClientId, handle: WindowHandle) -> Self {
        let now = std::time::Instant::now();
        Self {
            id,
            handle,
            title: RwLock::new(String::new()),
            class: RwLock::new(String::new()),
            instance: RwLock::new(String::new()),
            role: RwLock::new(None),
            window_type: RwLock::new(WindowType::Unknown),
            states: RwLock::new(HashSet::new()),
            geometry: RwLock::new(Geometry::default()),
            desktop: RwLock::new(0),
            parent: RwLock::new(None),
            transient: RwLock::new(Vec::new()),
            destroying: Mutex::new(false),
            created_at: now,
            updated_at: RwLock::new(now),
        }
    }
    
    /// Mark client as destroying to prevent re-entrancy
    pub fn mark_destroying(&self) -> bool {
        let mut destroying = self.destroying.lock();
        if *destroying {
            false // Already destroying
        } else {
            *destroying = true;
            true
        }
    }
    
    /// Update the last update timestamp
    pub fn touch(&self) {
        *self.updated_at.write() = std::time::Instant::now();
    }
    
    /// Check if client has a specific state
    pub fn has_state(&self, state: WindowState) -> bool {
        self.states.read().contains(&state)
    }
    
    /// Add a state
    pub fn add_state(&self, state: WindowState) {
        self.states.write().insert(state);
        self.touch();
    }
    
    /// Remove a state
    pub fn remove_state(&self, state: WindowState) {
        self.states.write().remove(&state);
        self.touch();
    }
}

/// Manages all client windows
pub struct ClientManager {
    /// Storage for all clients
    clients: SlotMap<ClientId, Arc<Client>>,
    /// Map from window handle to client ID for quick lookup
    handle_map: RwLock<HashMap<WindowHandle, ClientId>>,
    /// List of clients in focus order (most recent first)
    focus_history: Mutex<VecDeque<ClientId>>,
    /// Clients by desktop
    desktop_clients: RwLock<HashMap<u32, Vec<ClientId>>>,
    /// Active client count for statistics
    active_count: RwLock<usize>,
}

impl ClientManager {
    /// Create a new client manager
    pub fn new() -> Self {
        Self {
            clients: SlotMap::with_key(),
            handle_map: RwLock::new(HashMap::new()),
            focus_history: Mutex::new(VecDeque::new()),
            desktop_clients: RwLock::new(HashMap::new()),
            active_count: RwLock::new(0),
        }
    }
    
    /// Add a new client
    pub fn add_client(&mut self, handle: WindowHandle) -> Arc<Client> {
        let id = self.clients.insert_with_key(|id| {
            Arc::new(Client::new(id, handle))
        });
        
        let client = self.clients.get(id).unwrap().clone();
        self.handle_map.write().insert(handle, id);
        *self.active_count.write() += 1;
        
        info!("Added client {:?} with handle {:?}", id, handle);
        client
    }
    
    /// Get a client by ID
    pub fn get(&self, id: ClientId) -> Option<Arc<Client>> {
        self.clients.get(id).cloned()
    }
    
    /// Get a client by window handle
    pub fn get_by_handle(&self, handle: WindowHandle) -> Option<Arc<Client>> {
        let handle_map = self.handle_map.read();
        handle_map.get(&handle).and_then(|&id| self.clients.get(id).cloned())
    }
    
    /// Remove a client (FIXED: proper cleanup to prevent memory leaks)
    pub fn remove_client(&mut self, id: ClientId) -> Option<Arc<Client>> {
        let client = self.clients.get(id)?;
        
        // Mark as destroying to prevent re-entrancy
        if !client.mark_destroying() {
            warn!("Client {:?} already being destroyed", id);
            return None;
        }
        
        let handle = client.handle;
        let desktop = *client.desktop.read();
        
        // Remove from handle map
        self.handle_map.write().remove(&handle);
        
        // Remove from focus history
        let mut focus_history = self.focus_history.lock();
        focus_history.retain(|&fid| fid != id);
        
        // Remove from desktop clients
        let mut desktop_clients = self.desktop_clients.write();
        if let Some(clients) = desktop_clients.get_mut(&desktop) {
            clients.retain(|&cid| cid != id);
            if clients.is_empty() {
                desktop_clients.remove(&desktop);
            }
        }
        
        // Decrement active count
        *self.active_count.write() -= 1;
        
        // Remove from slotmap (this drops the Arc)
        let removed = self.clients.remove(id);
        
        info!("Removed client {:?}, {} clients remaining", id, self.clients.len());
        removed
    }
    
    /// Get all clients (FIXED: returns snapshot to prevent iterator invalidation)
    pub fn get_all(&self) -> Vec<Arc<Client>> {
        self.clients.values().cloned().collect()
    }
    
    /// Get client count
    pub fn count(&self) -> usize {
        self.clients.len()
    }
    
    /// Update client's desktop
    pub fn move_to_desktop(&self, id: ClientId, new_desktop: u32) -> Result<()> {
        let client = self.clients.get(id)
            .ok_or_else(|| OpenboxError::ClientNotFound(id))?;
        
        let old_desktop = {
            let mut desktop = client.desktop.write();
            let old = *desktop;
            *desktop = new_desktop;
            old
        };
        
        // Update desktop clients mapping
        let mut desktop_clients = self.desktop_clients.write();
        
        // Remove from old desktop
        if let Some(clients) = desktop_clients.get_mut(&old_desktop) {
            clients.retain(|&cid| cid != id);
        }
        
        // Add to new desktop
        desktop_clients.entry(new_desktop)
            .or_insert_with(Vec::new)
            .push(id);
        
        client.touch();
        debug!("Moved client {:?} from desktop {} to {}", id, old_desktop, new_desktop);
        
        Ok(())
    }
    
    /// Add to focus history
    pub fn add_to_focus_history(&self, id: ClientId) {
        let mut history = self.focus_history.lock();
        // Remove if already present (to move to front)
        history.retain(|&fid| fid != id);
        history.push_front(id);
        
        // Limit history size to prevent unbounded growth
        const MAX_HISTORY: usize = 100;
        if history.len() > MAX_HISTORY {
            history.pop_back();
        }
    }
    
    /// Get focus history
    pub fn get_focus_history(&self) -> Vec<ClientId> {
        self.focus_history.lock().iter().copied().collect()
    }
    
    /// Get clients on a specific desktop
    pub fn get_on_desktop(&self, desktop: u32) -> Vec<Arc<Client>> {
        let desktop_clients = self.desktop_clients.read();
        desktop_clients.get(&desktop)
            .map(|ids| {
                ids.iter()
                    .filter_map(|&id| self.clients.get(id).cloned())
                    .collect()
            })
            .unwrap_or_default()
    }
    
    /// Clean up stale clients (orphaned references)
    pub fn cleanup_stale(&mut self) {
        let stale_ids: Vec<ClientId> = self.clients.keys()
            .filter(|&id| {
                // Check if Arc is the only reference (shouldn't happen in normal operation)
                self.clients.get(id)
                    .map(|arc| Arc::strong_count(arc) == 1)
                    .unwrap_or(false)
            })
            .collect();
        
        for id in stale_ids {
            warn!("Cleaning up stale client {:?}", id);
            self.remove_client(id);
        }
    }
}

impl Default for ClientManager {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_client_lifecycle() {
        let mut manager = ClientManager::new();
        
        // Add client
        let client = manager.add_client(12345);
        assert_eq!(manager.count(), 1);
        
        let id = client.id;
        
        // Retrieve client
        let retrieved = manager.get(id);
        assert!(retrieved.is_some());
        
        // Remove client
        manager.remove_client(id);
        assert_eq!(manager.count(), 0);
        
        // Should not be retrievable after removal
        assert!(manager.get(id).is_none());
    }
    
    #[test]
    fn test_focus_history() {
        let manager = ClientManager::new();
        
        manager.add_to_focus_history(ClientId::from(0u64));
        manager.add_to_focus_history(ClientId::from(1u64));
        manager.add_to_focus_history(ClientId::from(0u64)); // Duplicate
        
        let history = manager.get_focus_history();
        assert_eq!(history.len(), 2); // Should deduplicate
    }
}
