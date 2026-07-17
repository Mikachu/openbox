//! Client (Window) Management
//! 
//! Provides memory-safe management of X11/Wayland windows with proper
//! lifecycle tracking and state management.

use crate::error::{OpenboxError, Result};
use crate::types::*;
use parking_lot::{RwLock, Mutex};
use slotmap::{new_key_type, SlotMap};
use std::collections::{HashMap, HashSet, VecDeque};
use std::sync::Arc;
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
    /// Window type
    pub window_type: WindowType,
    /// Window states
    pub states: HashSet<WindowState>,
    /// Window geometry
    pub geometry: Geometry,
    /// Original geometry (before maximize/fullscreen)
    pub original_geometry: Option<Geometry>,
    /// Window title
    pub title: String,
    /// Window class (WM_CLASS)
    pub class: String,
    /// Window name/instance (WM_CLASS)
    pub name: String,
    /// Window role
    pub role: Option<String>,
    /// Icon data
    pub icon: Option<IconData>,
    /// Desktop/workspace number (-1 for sticky)
    pub desktop: i32,
    /// Transient for (parent window)
    pub transient_for: Option<ClientId>,
    /// Client machine (hostname)
    pub machine: Option<String>,
    /// Process ID
    pub pid: Option<u32>,
    /// User time (for focus stealing prevention)
    pub user_time: u64,
    /// Frame decoration extents
    pub frame_extents: FrameExtents,
    /// Protocols supported by client
    pub protocols: HashSet<String>,
    /// Whether client is being managed
    pub managed: bool,
    /// Whether window is mapped
    pub mapped: bool,
    /// Whether client is destroyed (pending cleanup)
    pub destroyed: bool,
    /// Window gravity
    pub gravity: u32,
    /// Normal hints (size constraints)
    pub size_hints: SizeHints,
    /// Window manager hints
    pub wm_hints: WmHints,
    /// Client group leader
    pub group_leader: Option<ClientId>,
    /// Window group
    pub window_group: Option<u32>,
    /// Last focus time
    pub last_focus_time: std::time::Instant,
    /// Creation time
    pub created_at: std::time::Instant,
    /// Custom properties (for extensions)
    pub properties: HashMap<String, serde_json::Value>,
}

/// Size hints (ICCCM)
#[derive(Debug, Clone, Default, serde::Serialize, serde::Deserialize)]
pub struct SizeHints {
    pub min_width: Option<i32>,
    pub min_height: Option<i32>,
    pub max_width: Option<i32>,
    pub max_height: Option<i32>,
    pub width_inc: Option<i32>,
    pub height_inc: Option<i32>,
    pub min_aspect: Option<f64>,
    pub max_aspect: Option<f64>,
    pub base_width: Option<i32>,
    pub base_height: Option<i32>,
    pub win_gravity: Option<u32>,
}

/// WM hints
#[derive(Debug, Clone, Default, serde::Serialize, serde::Deserialize)]
pub struct WmHints {
    pub input: bool,
    pub initial_state: Option<u32>,
    pub icon_pixmap: Option<u64>,
    pub icon_mask: Option<u64>,
    pub icon_window: Option<u64>,
    pub window_group: Option<u64>,
    pub urgency: bool,
}

/// Frame decoration extents
#[derive(Debug, Clone, Copy, Default, serde::Serialize, serde::Deserialize)]
pub struct FrameExtents {
    pub left: u32,
    pub right: u32,
    pub top: u32,
    pub bottom: u32,
}

/// Icon data
#[derive(Debug, Clone, serde::Serialize, serde::Deserialize)]
pub struct IconData {
    pub width: u32,
    pub height: u32,
    pub data: Vec<u8>, // RGBA
}

/// Client manager - owns all client data
pub struct ClientManager {
    /// Storage for all clients
    clients: SlotMap<ClientId, RwLock<Client>>,
    /// Map from window handle to client ID
    handle_map: HashMap<WindowHandle, ClientId>,
    /// Stack order (bottom to top)
    stack: VecDeque<ClientId>,
    /// Focus history
    focus_history: VecDeque<ClientId>,
    /// Client groups
    groups: HashMap<u32, HashSet<ClientId>>,
    /// Pending destruction
    pending_destroy: Vec<ClientId>,
}

impl ClientManager {
    /// Create a new client manager
    pub fn new() -> Self {
        Self {
            clients: SlotMap::with_key(),
            handle_map: HashMap::new(),
            stack: VecDeque::new(),
            focus_history: VecDeque::new(),
            groups: HashMap::new(),
            pending_destroy: Vec::new(),
        }
    }

    /// Add a new client
    pub fn add_client(&mut self, handle: WindowHandle, window_type: WindowType) -> Result<ClientId> {
        if self.handle_map.contains_key(&handle) {
            return Err(OpenboxError::Client(format!(
                "Client with handle {:?} already exists", handle
            )));
        }

        let id = self.clients.insert(RwLock::new(Client {
            id: ClientId::default(), // Will be updated
            handle,
            window_type,
            states: HashSet::new(),
            geometry: Geometry::default(),
            original_geometry: None,
            title: String::new(),
            class: String::new(),
            name: String::new(),
            role: None,
            icon: None,
            desktop: 0,
            transient_for: None,
            machine: None,
            pid: None,
            user_time: 0,
            frame_extents: FrameExtents::default(),
            protocols: HashSet::new(),
            managed: false,
            mapped: false,
            destroyed: false,
            gravity: 0,
            size_hints: SizeHints::default(),
            wm_hints: WmHints::default(),
            group_leader: None,
            window_group: None,
            last_focus_time: std::time::Instant::now(),
            created_at: std::time::Instant::now(),
            properties: HashMap::new(),
        }));

        // Update the ID in the client
        {
            let mut client = self.clients[id].write();
            client.id = id;
        }

        self.handle_map.insert(handle, id);
        
        info!("Added client {:?} with handle {:?}", id, handle);
        
        Ok(id)
    }

    /// Get a client by ID
    pub fn get(&self, id: ClientId) -> Option<&RwLock<Client>> {
        self.clients.get(id)
    }

    /// Get client by window handle
    pub fn get_by_handle(&self, handle: WindowHandle) -> Option<&RwLock<Client>> {
        self.handle_map.get(&handle).and_then(|&id| self.clients.get(id))
    }

    /// Remove a client
    pub fn remove_client(&mut self, id: ClientId) -> Result<()> {
        let client = self.clients.get(id)
            .ok_or_else(|| OpenboxError::Client(format!("Client {:?} not found", id)))?;
        
        let handle = {
            let c = client.read();
            c.handle
        };

        // Remove from all collections
        self.handle_map.remove(&handle);
        self.stack.retain(|&cid| cid != id);
        self.focus_history.retain(|&cid| cid != id);
        
        // Remove from groups
        for group in self.groups.values_mut() {
            group.remove(&id);
        }

        // Actually remove the client
        self.clients.remove(id);
        
        info!("Removed client {:?}", id);
        
        Ok(())
    }

    /// Mark client for destruction (deferred cleanup)
    pub fn destroy_client(&mut self, id: ClientId) -> Result<()> {
        if let Some(client) = self.clients.get(id) {
            let mut c = client.write();
            c.destroyed = true;
            c.managed = false;
        }
        
        self.pending_destroy.push(id);
        Ok(())
    }

    /// Process pending destructions
    pub fn process_destructions(&mut self) -> Vec<ClientId> {
        let to_destroy: Vec<_> = self.pending_destroy.drain(..).collect();
        
        for id in &to_destroy {
            if let Err(e) = self.remove_client(*id) {
                warn!("Failed to destroy client {:?}: {}", id, e);
            }
        }
        
        to_destroy
    }

    /// Set client state
    pub fn set_state(&mut self, id: ClientId, state: WindowState, value: bool) -> Result<()> {
        let client = self.clients.get(id)
            .ok_or_else(|| OpenboxError::Client(format!("Client {:?} not found", id)))?;
        
        let mut c = client.write();
        
        if value {
            c.states.insert(state);
        } else {
            c.states.remove(&state);
        }
        
        debug!("Client {:?} state {:?} = {}", id, state, value);
        
        Ok(())
    }

    /// Check if client has state
    pub fn has_state(&self, id: ClientId, state: WindowState) -> bool {
        self.clients.get(id)
            .map(|c| c.read().states.contains(&state))
            .unwrap_or(false)
    }

    /// Set client geometry
    pub fn set_geometry(&mut self, id: ClientId, geometry: Geometry) -> Result<()> {
        let client = self.clients.get(id)
            .ok_or_else(|| OpenboxError::Client(format!("Client {:?} not found", id)))?;
        
        let mut c = client.write();
        c.geometry = geometry;
        
        Ok(())
    }

    /// Get client geometry
    pub fn get_geometry(&self, id: ClientId) -> Option<Geometry> {
        self.clients.get(id).map(|c| c.read().geometry)
    }

    /// Update stack order
    pub fn restack(&mut self, id: ClientId, position: StackPosition) -> Result<()> {
        self.stack.retain(|&cid| cid != id);
        
        match position {
            StackPosition::Top => self.stack.push_back(id),
            StackPosition::Bottom => self.stack.push_front(id),
            StackPosition::Below(other) => {
                if let Some(pos) = self.stack.iter().position(|&cid| cid == other) {
                    self.stack.insert(pos, id);
                } else {
                    self.stack.push_back(id);
                }
            }
            StackPosition::Above(other) => {
                if let Some(pos) = self.stack.iter().position(|&cid| cid == other) {
                    self.stack.insert(pos + 1, id);
                } else {
                    self.stack.push_back(id);
                }
            }
        }
        
        Ok(())
    }

    /// Get stack position
    pub fn get_stack_position(&self, id: ClientId) -> Option<usize> {
        self.stack.iter().position(|&cid| cid == id)
    }

    /// Get stack as ordered list
    pub fn get_stack(&self) -> &VecDeque<ClientId> {
        &self.stack
    }

    /// Add to focus history
    pub fn record_focus(&mut self, id: ClientId) {
        self.focus_history.retain(|&cid| cid != id);
        self.focus_history.push_front(id);
    }

    /// Get focus history
    pub fn get_focus_history(&self) -> &VecDeque<ClientId> {
        &self.focus_history
    }

    /// Get last focused client (excluding current)
    pub fn get_last_focused(&self, current: ClientId) -> Option<ClientId> {
        self.focus_history.iter()
            .find(|&&cid| cid != current)
            .copied()
    }

    /// Get clients on a specific desktop
    pub fn get_clients_on_desktop(&self, desktop: i32) -> Vec<ClientId> {
        self.clients.iter()
            .filter(|(_, c)| {
                let client = c.read();
                client.desktop == desktop || client.states.contains(&WindowState::Sticky)
            })
            .map(|(id, _)| id)
            .collect()
    }

    /// Get all client IDs
    pub fn get_all_clients(&self) -> Vec<ClientId> {
        self.clients.keys().collect()
    }

    /// Get count of managed clients
    pub fn count(&self) -> usize {
        self.clients.len()
    }

    /// Save state to file
    pub async fn save_state(&self) -> Result<()> {
        let state: Vec<_> = self.clients.iter()
            .map(|(id, c)| {
                let client = c.read();
                serde_json::json!({\n                    "id": id,\n                    "title": client.title,\n                    "class\": client.class,\n                    \"desktop\": client.desktop,\n                    \"states\": client.states.iter().collect::<Vec<_>>(),\n                    \"geometry\": client.geometry\n                })\n            })\n            .collect();\n        \n        // In real implementation, save to file\n        debug!(\"Saving state for {} clients\", state.len());\n        Ok(())\n    }\n\n    /// Find client by class/name\n    pub fn find_by_class(&self, class: &str) -> Vec<ClientId> {\n        self.clients.iter()\n            .filter(|(_, c)| {\n                let client = c.read();\n                client.class == class\n            })\n            .map(|(id, _)| id)\n            .collect()\n    }\n\n    /// Find client by title pattern\n    pub fn find_by_title(&self, pattern: &regex::Regex) -> Vec<ClientId> {\n        self.clients.iter()\n            .filter(|(_, c)| {\n                let client = c.read();\n                pattern.is_match(&client.title)\n            })\n            .map(|(id, _)| id)\n            .collect()\n    }\n}\n\nimpl Default for ClientManager {\n    fn default() -> Self {\n        Self::new()\n    }\n}\n\n/// Stack position options\n#[derive(Debug, Clone, Copy)]\npub enum StackPosition {\n    /// Top of stack\n    Top,\n    /// Bottom of stack\n    Bottom,\n    /// Below specific window\n    Below(ClientId),\n    /// Above specific window\n    Above(ClientId),\n}\n\n#[cfg(test)]\nmod tests {\n    use super::*;\n\n    #[test]\n    fn test_client_lifecycle() {\n        let mut manager = ClientManager::new();\n        \n        // Add client\n        let id = manager.add_client(WindowHandle::X11(12345), WindowType::Normal).unwrap();\n        assert_eq!(manager.count(), 1);\n        \n        // Get client\n        let client = manager.get(id).unwrap();\n        assert_eq!(client.read().handle, WindowHandle::X11(12345));\n        \n        // Set state\n        manager.set_state(id, WindowState::Focused, true).unwrap();\n        assert!(manager.has_state(id, WindowState::Focused));\n        \n        // Remove client\n        manager.remove_client(id).unwrap();\n        assert_eq!(manager.count(), 0);\n    }\n\n    #[test]\n    fn test_stack_order() {\n        let mut manager = ClientManager::new();\n        \n        let id1 = manager.add_client(WindowHandle::X11(1), WindowType::Normal).unwrap();\n        let id2 = manager.add_client(WindowHandle::X11(2), WindowType::Normal).unwrap();\n        let id3 = manager.add_client(WindowHandle::X11(3), WindowType::Normal).unwrap();\n        \n        // Default order\n        manager.restack(id1, StackPosition::Bottom).unwrap();\n        manager.restack(id2, StackPosition::Top).unwrap();\n        manager.restack(id3, StackPosition::Top).unwrap();\n        \n        let stack: Vec<_> = manager.get_stack().iter().copied().collect();\n        assert_eq!(stack, vec![id1, id2, id3]);\n        \n        // Move id2 to top\n        manager.restack(id2, StackPosition::Top).unwrap();\n        let stack: Vec<_> = manager.get_stack().iter().copied().collect();\n        assert_eq!(stack, vec![id1, id3, id2]);\n    }\n\n    #[test]\n    fn test_focus_history() {\n        let mut manager = ClientManager::new();\n        \n        let id1 = manager.add_client(WindowHandle::X11(1), WindowType::Normal).unwrap();\n        let id2 = manager.add_client(WindowHandle::X11(2), WindowType::Normal).unwrap();\n        let id3 = manager.add_client(WindowHandle::X11(3), WindowType::Normal).unwrap();\n        \n        manager.record_focus(id1);\n        manager.record_focus(id2);\n        manager.record_focus(id3);\n        \n        let last = manager.get_last_focused(id3);\n        assert_eq!(last, Some(id2));\n    }\n}
