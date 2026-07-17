//! Stacking Order Management
//! 
//! Manages Z-order of windows with layer-based stacking and proper
//! handling of transients, docks, and desktop windows.

use crate::client::{ClientId, ClientManager, WindowState, WindowType};
use crate::error::{OpenboxError, Result};
use parking_lot::RwLock;
use std::collections::{HashMap, HashSet, VecDeque};
use tracing::{debug, info, trace, warn};

/// Stacking layers (bottom to top)
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum StackingLayer {
    /// Desktop windows (bottom)
    Desktop = 0,
    /// Normal application windows
    Normal = 1,
    /// Dialog windows (above parent)
    Dialog = 2,
    /// Utility windows
    Utility = 3,
    /// Dock/panel windows
    Dock = 4,
    /// Menus
    Menu = 5,
    /// Fullscreen windows (top of normal stack)
    Fullscreen = 6,
    /// On-screen display
    Osd = 7,
    /// Notifications (top)
    Notification = 8,
}

impl StackingLayer {
    /// Get layer for a window type
    pub fn from_window_type(window_type: WindowType) -> Self {
        match window_type {
            WindowType::Desktop => StackingLayer::Desktop,
            WindowType::Dock => StackingLayer::Dock,
            WindowType::Menu => StackingLayer::Menu,
            WindowType::Dialog => StackingLayer::Dialog,
            WindowType::Utility => StackingLayer::Utility,
            WindowType::Notification => StackingLayer::Notification,
            _ => StackingLayer::Normal,
        }
    }
}

/// Stacking manager
pub struct StackingManager {
    /// Layer assignments
    layer_assignments: HashMap<ClientId, StackingLayer>,
    /// Global stack order (bottom to top)
    global_stack: VecDeque<ClientId>,
    /// Per-layer ordering
    layer_stacks: HashMap<StackingLayer, VecDeque<ClientId>>,
    /// Transient relationships (child -> parent)
    transients: HashMap<ClientId, ClientId>,
    /// Reverse transient map (parent -> children)
    transient_children: HashMap<ClientId, HashSet<ClientId>>,
    /// Windows that must be above another
    above: HashMap<ClientId, ClientId>,
    /// Windows that must be below another
    below: HashMap<ClientId, ClientId>,
    /// Fullscreen windows
    fullscreen: HashSet<ClientId>,
    /// Windows with always-on-top state
    always_on_top: HashSet<ClientId>,
    /// Windows with always-on-bottom state
    always_on_bottom: HashSet<ClientId>,
    /// Whether stacking has changed (needs restack)
    dirty: bool,
}

/// Stacking position for relative placement
#[derive(Debug, Clone, Copy)]
pub enum StackingPosition {
    /// Bottom of stack
    Bottom,
    /// Top of stack
    Top,
    /// Below specific window
    Below(ClientId),
    /// Above specific window
    Above(ClientId),
    /// Directly below parent (for transients)
    DirectlyBelow(ClientId),
    /// Directly above parent (for transients)
    DirectlyAbove(ClientId),
}

impl StackingManager {
    /// Create a new stacking manager
    pub fn new() -> Self {
        let mut layer_stacks = HashMap::new();
        for layer in [
            StackingLayer::Desktop,
            StackingLayer::Normal,
            StackingLayer::Dialog,
            StackingLayer::Utility,
            StackingLayer::Dock,
            StackingLayer::Menu,
            StackingLayer::Fullscreen,
            StackingLayer::Osd,
            StackingLayer::Notification,
        ] {
            layer_stacks.insert(layer, VecDeque::new());
        }

        Self {
            layer_assignments: HashMap::new(),
            global_stack: VecDeque::new(),
            layer_stacks,
            transients: HashMap::new(),
            transient_children: HashMap::new(),
            above: HashMap::new(),
            below: HashMap::new(),
            fullscreen: HashSet::new(),
            always_on_top: HashSet::new(),
            always_on_bottom: HashSet::new(),
            dirty: false,
        }
    }

    /// Add a client to stacking
    pub fn add_client(
        &mut self,
        client_id: ClientId,
        window_type: WindowType,
        transient_for: Option<ClientId>,
    ) -> Result<()> {
        let layer = StackingLayer::from_window_type(window_type);
        
        self.layer_assignments.insert(client_id, layer);
        
        // Handle transient relationship
        if let Some(parent) = transient_for {
            self.set_transient(client_id, parent)?;
        }

        // Add to layer stack
        if let Some(stack) = self.layer_stacks.get_mut(&layer) {
            stack.push_back(client_id);
        }

        // Rebuild global stack
        self.rebuild_global_stack();
        
        trace!("Added client {:?} to layer {:?}", client_id, layer);
        
        Ok(())
    }

    /// Remove a client from stacking
    pub fn remove_client(&mut self, client_id: ClientId) -> Result<()> {
        // Remove from layer assignment
        let layer = self.layer_assignments.remove(&client_id);
        
        // Remove from layer stack
        if let Some(l) = layer {
            if let Some(stack) = self.layer_stacks.get_mut(&l) {
                stack.retain(|&id| id != client_id);
            }
        }

        // Remove from global stack
        self.global_stack.retain(|&id| id != client_id);

        // Clean up transient relationships
        self.remove_transient(client_id);
        
        // Remove children
        if let Some(children) = self.transient_children.remove(&client_id) {
            for child in children {
                self.transients.remove(&child);
            }
        }

        // Remove from special sets
        self.fullscreen.remove(&client_id);
        self.always_on_top.remove(&client_id);
n        self.always_on_bottom.remove(&client_id);\n        \n        self.above.remove(&client_id);\n        self.below.remove(&client_id);\n        \n        self.dirty = true;\n        \n        trace!(\"Removed client {:?} from stacking\", client_id);\n        \n        Ok(())\n    }\n\n    /// Set transient relationship\n    pub fn set_transient(&mut self, child: ClientId, parent: ClientId) -> Result<()> {\n        // Remove old transient relationship\n        self.remove_transient(child);\n        \n        // Set new relationship\n        self.transients.insert(child, parent);\n        self.transient_children\n            .entry(parent)\n            .or_insert_with(HashSet::new)\n            .insert(child);\n        \n        // Move child above parent\n        self.raise_to_parent(child, parent)?;\n        \n        trace!(\"Set transient: {:?} -> {:?}\", child, parent);\n        \n        Ok(())\n    }\n\n    /// Remove transient relationship\n    pub fn remove_transient(&mut self, child: ClientId) {\n        if let Some(parent) = self.transients.remove(&child) {\n            if let Some(children) = self.transient_children.get_mut(&parent) {\n                children.remove(&child);\n            }\n        }\n    }\n\n    /// Get client's layer\n    pub fn get_layer(&self, client_id: ClientId) -> Option<StackingLayer> {\n        self.layer_assignments.get(&client_id).copied()\n    }\n\n    /// Set client layer\n    pub fn set_layer(&mut self, client_id: ClientId, layer: StackingLayer) -> Result<()> {\n        // Remove from old layer\n        if let Some(old_layer) = self.layer_assignments.get(&client_id) {\n            if let Some(stack) = self.layer_stacks.get_mut(old_layer) {\n                stack.retain(|&id| id != client_id);\n            }\n        }\n\n        // Add to new layer\n        self.layer_assignments.insert(client_id, layer);\n        if let Some(stack) = self.layer_stacks.get_mut(&layer) {\n            stack.push_back(client_id);\n        }\n\n        self.rebuild_global_stack();\n        \n        Ok(())\n    }\n\n    /// Restack a client\n    pub fn restack(&mut self, client_id: ClientId, position: StackingPosition) -> Result<()> {\n        let layer = self.get_layer(client_id)\n            .ok_or_else(|| OpenboxError::Stacking(format!(\"Client {:?} not found\", client_id)))?;\n\n        match position {\n            StackingPosition::Bottom => {\n                if let Some(stack) = self.layer_stacks.get_mut(&layer) {\n                    stack.retain(|&id| id != client_id);\n                    stack.push_front(client_id);\n                }\n            }\n            StackingPosition::Top => {\n                if let Some(stack) = self.layer_stacks.get_mut(&layer) {\n                    stack.retain(|&id| id != client_id);\n                    stack.push_back(client_id);\n                }\n            }\n            StackingPosition::Below(other) => {\n                self.insert_relative(client_id, other, false)?;\n            }\n            StackingPosition::Above(other) => {\n                self.insert_relative(client_id, other, true)?;\n            }\n            StackingPosition::DirectlyBelow(parent) => {\n                self.insert_directly_relative(client_id, parent, false)?;\n            }\n            StackingPosition::DirectlyAbove(parent) => {\n                self.insert_directly_relative(client_id, parent, true)?;\n            }\n        }\n\n        self.rebuild_global_stack();\n        self.dirty = true;\n        \n        Ok(())\n    }\n\n    /// Raise client to top of its layer\n    pub fn raise(&mut self, client_id: ClientId) -> Result<()> {\n        self.restack(client_id, StackingPosition::Top)\n    }\n\n    /// Lower client to bottom of its layer\n    pub fn lower(&mut self, client_id: ClientId) -> Result<()> {\n        self.restack(client_id, StackingPosition::Bottom)\n    }\n\n    /// Raise client above another\n    pub fn raise_above(&mut self, client_id: ClientId, other: ClientId) -> Result<()> {\n        self.restack(client_id, StackingPosition::Above(other))\n    }\n\n    /// Lower client below another\n    pub fn lower_below(&mut self, client_id: ClientId, other: ClientId) -> Result<()> {\n        self.restack(client_id, StackingPosition::Below(other))\n    }\n\n    /// Set fullscreen state\n    pub fn set_fullscreen(&mut self, client_id: ClientId, fullscreen: bool) -> Result<()> {\n        if fullscreen {\n            self.fullscreen.insert(client_id);\n            self.set_layer(client_id, StackingLayer::Fullscreen)?;\n        } else {\n            self.fullscreen.remove(&client_id);\n            // Restore original layer\n            // In real implementation, would track original layer\n            self.set_layer(client_id, StackingLayer::Normal)?;\n        }\n        \n        Ok(())\n    }\n\n    /// Set always-on-top state\n    pub fn set_always_on_top(&mut self, client_id: ClientId, on_top: bool) -> Result<()> {\n        if on_top {\n            self.always_on_top.insert(client_id);\n            // Move to top of normal layer\n            self.raise(client_id)?;\n        } else {\n            self.always_on_top.remove(&client_id);\n        }\n        \n        Ok(())\n    }\n\n    /// Set always-on-bottom state\n    pub fn set_always_on_bottom(&mut self, client_id: ClientId, on_bottom: bool) -> Result<()> {\n        if on_bottom {\n            self.always_on_bottom.insert(client_id);\n            self.lower(client_id)?;\n        } else {\n            self.always_on_bottom.remove(&client_id);\n        }\n        \n        Ok(())\n    }\n\n    /// Get global stack (bottom to top)\n    pub fn get_global_stack(&self) -> &VecDeque<ClientId> {\n        &self.global_stack\n    }\n\n    /// Get stack above a specific client\n    pub fn get_stack_above(&self, client_id: ClientId) -> Vec<ClientId> {\n        let pos = self.global_stack.iter().position(|&id| id == client_id);\n        match pos {\n            Some(idx) => self.global_stack.iter().skip(idx + 1).copied().collect(),\n            None => Vec::new(),\n        }\n    }\n\n    /// Get stack below a specific client\n    pub fn get_stack_below(&self, client_id: ClientId) -> Vec<ClientId> {\n        let pos = self.global_stack.iter().position(|&id| id == client_id);\n        match pos {\n            Some(idx) => self.global_stack.iter().take(idx).copied().collect(),\n            None => Vec::new(),\n        }\n    }\n\n    /// Check if client is above another\n    pub fn is_above(&self, client_id: ClientId, other: ClientId) -> bool {\n        let pos1 = self.global_stack.iter().position(|&id| id == client_id);\n        let pos2 = self.global_stack.iter().position(|&id| id == other);\n        \n        match (pos1, pos2) {\n            (Some(p1), Some(p2)) => p1 > p2,\n            _ => false,\n        }\n    }\n\n    /// Check if client is below another\n    pub fn is_below(&self, client_id: ClientId, other: ClientId) -> bool {\n        self.is_above(other, client_id)\n    }\n\n    /// Get topmost client at position (for hit testing)\n    pub fn get_client_at_position(\n        &self,\n        x: i32,\n        y: i32,\n        clients: &ClientManager,\n    ) -> Option<ClientId> {\n        // Iterate from top to bottom\n        for &client_id in self.global_stack.iter().rev() {\n            if let Some(geometry) = clients.get_geometry(client_id) {\n                if geometry.contains(x, y) {\n                    return Some(client_id);\n                }\n            }\n        }\n        \n        None\n    }\n\n    /// Check if stacking is dirty (needs X11/Wayland restack)\n    pub fn is_dirty(&self) -> bool {\n        self.dirty\n    }\n\n    /// Clear dirty flag\n    pub fn clear_dirty(&mut self) {\n        self.dirty = false;\n    }\n\n    /// Get transients of a client\n    pub fn get_transients(&self, parent: ClientId) -> Vec<ClientId> {\n        self.transient_children\n            .get(&parent)\n            .map(|children| children.iter().copied().collect())\n            .unwrap_or_default()\n    }\n\n    /// Get parent of a transient\n    pub fn get_parent(&self, child: ClientId) -> Option<ClientId> {\n        self.transients.get(&child).copied()\n    }\n\n    /// Get root parent (for nested transients)\n    pub fn get_root_parent(&self, mut client_id: ClientId) -> ClientId {\n        while let Some(parent) = self.transients.get(&client_id) {\n            client_id = *parent;\n        }\n        client_id\n    }\n\n    // Private helper methods\n\n    fn insert_relative(\n        &mut self,\n        client_id: ClientId,\n        other: ClientId,\n        above: bool,\n    ) -> Result<()> {\n        let layer = self.get_layer(other)\n            .ok_or_else(|| OpenboxError::Stacking(format!(\"Reference client {:?} not found\", other)))?;\n\n        if let Some(stack) = self.layer_stacks.get_mut(&layer) {\n            stack.retain(|&id| id != client_id);\n            \n            if let Some(pos) = stack.iter().position(|&id| id == other) {\n                let insert_pos = if above { pos + 1 } else { pos };\n                let insert_pos = insert_pos.min(stack.len());\n                \n                // Insert at position\n                let mut new_stack: VecDeque<_> = stack.drain(..insert_pos).collect();\n                new_stack.push_back(client_id);\n                new_stack.extend(stack.drain(..));\n                *stack = new_stack;\n            } else {\n                stack.push_back(client_id);\n            }\n        }\n\n        Ok(())\n    }\n\n    fn insert_directly_relative(\n        &mut self,\n        client_id: ClientId,\n        parent: ClientId,\n        above: bool,\n    ) -> Result<()> {\n        // Similar to insert_relative but ensures immediate adjacency\n        self.insert_relative(client_id, parent, above)\n    }\n\n    fn raise_to_parent(&mut self, child: ClientId, parent: ClientId) -> Result<()> {\n        self.insert_directly_relative(child, parent, true)\n    }\n\n    fn rebuild_global_stack(&mut self) {\n        self.global_stack.clear();\n        \n        // Build from layers (bottom to top)\n        let layers = [\n            StackingLayer::Desktop,\n            StackingLayer::Normal,\n            StackingLayer::Dialog,\n            StackingLayer::Utility,\n            StackingLayer::Dock,\n            StackingLayer::Menu,\n            StackingLayer::Fullscreen,\n            StackingLayer::Osd,\n            StackingLayer::Notification,\n        ];\n        \n        for layer in &layers {\n            if let Some(stack) = self.layer_stacks.get(layer) {\n                self.global_stack.extend(stack.iter());\n            }\n        }\n        \n        self.dirty = true;\n    }\n}\n\nimpl Default for StackingManager {\n    fn default() -> Self {\n        Self::new()\n    }\n}\n\n#[cfg(test)]\nmod tests {\n    use super::*;\n    use crate::client::{ClientManager, WindowType};\n\n    #[test]\n    fn test_layer_assignment() {\n        let mut stacking = StackingManager::new();\n        let mut clients = ClientManager::new();\n        \n        let id = clients.add_client(crate::types::WindowHandle::X11(1), WindowType::Normal).unwrap();\n        stacking.add_client(id, WindowType::Normal, None).unwrap();\n        \n        assert_eq!(stacking.get_layer(id), Some(StackingLayer::Normal));\n    }\n\n    #[test]\n    fn test_raise_lower() {\n        let mut stacking = StackingManager::new();\n        let mut clients = ClientManager::new();\n        \n        let id1 = clients.add_client(crate::types::WindowHandle::X11(1), WindowType::Normal).unwrap();\n        let id2 = clients.add_client(crate::types::WindowHandle::X11(2), WindowType::Normal).unwrap();\n        let id3 = clients.add_client(crate::types::WindowHandle::X11(3), WindowType::Normal).unwrap();\n        \n        stacking.add_client(id1, WindowType::Normal, None).unwrap();\n        stacking.add_client(id2, WindowType::Normal, None).unwrap();\n        stacking.add_client(id3, WindowType::Normal, None).unwrap();\n        \n        // Initial order: 1, 2, 3\n        let stack: Vec<_> = stacking.get_global_stack().iter().copied().collect();\n        assert_eq!(stack, vec![id1, id2, id3]);\n        \n        // Raise 1 to top\n        stacking.raise(id1).unwrap();\n        let stack: Vec<_> = stacking.get_global_stack().iter().copied().collect();\n        assert_eq!(stack, vec![id2, id3, id1]);\n        \n        // Lower 3 to bottom\n        stacking.lower(id3).unwrap();\n        let stack: Vec<_> = stacking.get_global_stack().iter().copied().collect();\n        assert_eq!(stack, vec![id3, id2, id1]);\n    }\n\n    #[test]\n    fn test_transient_stacking() {\n        let mut stacking = StackingManager::new();\n        let mut clients = ClientManager::new();\n        \n        let parent = clients.add_client(crate::types::WindowHandle::X11(1), WindowType::Normal).unwrap();\n        let child = clients.add_client(crate::types::WindowHandle::X11(2), WindowType::Dialog).unwrap();\n        \n        stacking.add_client(parent, WindowType::Normal, None).unwrap();\n        stacking.add_client(child, WindowType::Dialog, Some(parent)).unwrap();\n        \n        // Child should be above parent\n        assert!(stacking.is_above(child, parent));\n    }\n\n    #[test]\n    fn test_is_above_below() {\n        let mut stacking = StackingManager::new();\n        let mut clients = ClientManager::new();\n        \n        let id1 = clients.add_client(crate::types::WindowHandle::X11(1), WindowType::Normal).unwrap();\n        let id2 = clients.add_client(crate::types::WindowHandle::X11(2), WindowType::Normal).unwrap();\n        \n        stacking.add_client(id1, WindowType::Normal, None).unwrap();\n        stacking.add_client(id2, WindowType::Normal, None).unwrap();\n        \n        // id2 is above id1\n        assert!(stacking.is_above(id2, id1));\n        assert!(!stacking.is_above(id1, id2));\n        assert!(stacking.is_below(id1, id2));\n    }\n}
