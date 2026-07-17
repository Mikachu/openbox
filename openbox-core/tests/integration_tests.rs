//! Integration tests for Openbox Core
//! 
//! Tests the complete window manager flow with multiple clients,
//! focus changes, and stacking operations.

use openbox_core::client::{ClientManager, WindowState, WindowType};
use openbox_core::focus::{FocusModel, FocusReason, FocusTracker};
use openbox_core::stacking::{StackingLayer, StackingManager, StackingPosition};
use openbox_core::types::{Geometry, WindowHandle};

/// Test complete window lifecycle
#[tokio::test]
async fn test_window_lifecycle() {
    let mut clients = ClientManager::new();
    let mut stacking = StackingManager::new();
    let mut focus = FocusTracker::new();
    
    // Create window
    let id = clients.add_client(WindowHandle::X11(0x1000001), WindowType::Normal).unwrap();
    clients.set_geometry(id, Geometry::new(100, 100, 800, 600)).unwrap();
    
    // Add to stacking
    stacking.add_client(id, WindowType::Normal, None).unwrap();
    
    // Set state
    clients.set_state(id, WindowState::Mapped, true).unwrap();
    
    // Focus it
    focus.set_focus(id, FocusReason::Map).unwrap();
    assert_eq!(focus.focused(), Some(id));
    
    // Iconify
    clients.set_state(id, WindowState::Hidden, true).unwrap();
    
    // Unmap should clear focus
    focus.clear_focus().unwrap();
    assert!(focus.focused().is_none());
    
    // Destroy
    stacking.remove_client(id).unwrap();
    clients.remove_client(id).unwrap();
    
    assert_eq!(clients.count(), 0);
}

/// Test focus stacking with multiple windows
#[tokio::test]
async fn test_focus_stacking() {
    let mut clients = ClientManager::new();
    let mut stacking = StackingManager::new();
    let mut focus = FocusTracker::new();
    
    // Create three windows
    let ids: Vec<_> = (1..=3)
        .map(|i| {
            let id = clients.add_client(WindowHandle::X11(0x1000000 + i), WindowType::Normal).unwrap();
            stacking.add_client(id, WindowType::Normal, None).unwrap();
            clients.set_state(id, WindowState::Mapped, true).unwrap();
            id
        })
        .collect();
    
    // Focus windows in order
    for &id in &ids {
        focus.set_focus(id, FocusReason::Click).unwrap();
    }
    
    // Last should be focused
    assert_eq!(focus.focused(), Some(ids[2]));
    
    // Previous should be ids[1]
    assert_eq!(focus.previous_focus(), Some(ids[1]));
    
    // Focus history should have all three
    let history: Vec<_> = focus.get_focus_history().iter().copied().collect();
    assert_eq!(history.len(), 3);
    
    // Close top window, focus should fall back
    focus.clear_focus().unwrap();
    focus.set_focus(ids[1], FocusReason::Restore).unwrap();
    assert_eq!(focus.focused(), Some(ids[1]));
}

/// Test transient window stacking
#[tokio::test]
async fn test_transient_stacking() {
    let mut clients = ClientManager::new();
    let mut stacking = StackingManager::new();
    
    // Create parent window
    let parent = clients.add_client(WindowHandle::X11(0x2000001), WindowType::Normal).unwrap();
    stacking.add_client(parent, WindowType::Normal, None).unwrap();
    
    // Create dialog (transient)
    let dialog = clients.add_client(WindowHandle::X11(0x2000002), WindowType::Dialog).unwrap();
    stacking.add_client(dialog, WindowType::Dialog, Some(parent)).unwrap();
    
    // Dialog should be above parent
    assert!(stacking.is_above(dialog, parent));
    
    // Raise parent, dialog should stay above
    stacking.raise(parent).unwrap();
    assert!(stacking.is_above(dialog, parent));
    
    // Create second dialog
    let dialog2 = clients.add_client(WindowHandle::X11(0x2000003), WindowType::Dialog).unwrap();
    stacking.add_client(dialog2, WindowType::Dialog, Some(parent)).unwrap();
    
    // Both dialogs should be above parent
    assert!(stacking.is_above(dialog, parent));
    assert!(stacking.is_above(dialog2, parent));
}

/// Test desktop switching
#[tokio::test]
async fn test_desktop_switching() {
    let mut clients = ClientManager::new();
    let mut focus = FocusTracker::new();
    
    // Create windows on different desktops
    let win1 = clients.add_client(WindowHandle::X11(0x3000001), WindowType::Normal).unwrap();
    clients.set_geometry(win1, Geometry::new(0, 0, 800, 600)).unwrap();
    {
        let mut c = clients.get(win1).unwrap().write();
        c.desktop = 0;
    }
    
    let win2 = clients.add_client(WindowHandle::X11(0x3000002), WindowType::Normal).unwrap();
    clients.set_geometry(win2, Geometry::new(0, 0, 800, 600)).unwrap();
    {
        let mut c = clients.get(win2).unwrap().write();
        c.desktop = 1;
    }
    
    let win3 = clients.add_client(WindowHandle::X11(0x3000003), WindowType::Normal).unwrap();
    clients.set_geometry(win3, Geometry::new(0, 0, 800, 600)).unwrap();
    {
        let mut c = clients.get(win3).unwrap().write();
        c.desktop = 1;
        c.states.insert(WindowState::Sticky);
    }
    
    // Get clients on desktop 0
    let desktop0_clients = clients.get_clients_on_desktop(0);
    assert!(desktop0_clients.contains(&win1));
    assert!(!desktop0_clients.contains(&win2));
    assert!(desktop0_clients.contains(&win3)); // Sticky
    
    // Get clients on desktop 1
    let desktop1_clients = clients.get_clients_on_desktop(1);
    assert!(!desktop1_clients.contains(&win1));
    assert!(desktop1_clients.contains(&win2));
    assert!(desktop1_clients.contains(&win3)); // Sticky
}

/// Test fullscreen handling
#[tokio::test]
async fn test_fullscreen() {
    let mut clients = ClientManager::new();
    let mut stacking = StackingManager::new();
    
    let id = clients.add_client(WindowHandle::X11(0x4000001), WindowType::Normal).unwrap();
    stacking.add_client(id, WindowType::Normal, None).unwrap();
    
    // Initially on Normal layer
    assert_eq!(stacking.get_layer(id), Some(StackingLayer::Normal));
    
    // Set fullscreen
    stacking.set_fullscreen(id, true).unwrap();
    assert_eq!(stacking.get_layer(id), Some(StackingLayer::Fullscreen));
    
    // Unset fullscreen
    stacking.set_fullscreen(id, false).unwrap();
    assert_eq!(stacking.get_layer(id), Some(StackingLayer::Normal));
}

/// Test focus models
#[tokio::test]
async fn test_focus_models() {
    let mut focus = FocusTracker::new();
    
    // Test click-to-focus
    focus.set_model(FocusModel::ClickToFocus);
    assert_eq!(focus.model(), FocusModel::ClickToFocus);
    
    // Test focus follows mouse
    focus.set_model(FocusModel::FocusFollowsMouse);
    assert_eq!(focus.model(), FocusModel::FocusFollowsMouse);
    
    // Test strict focus follows mouse
    focus.set_model(FocusModel::StrictFocusFollowsMouse);
    assert_eq!(focus.model(), FocusModel::StrictFocusFollowsMouse);
}

/// Test concurrent client operations
#[tokio::test]
async fn test_concurrent_operations() {
    use std::sync::Arc;
    use tokio::sync::RwLock;
    
    let clients = Arc::new(RwLock::new(ClientManager::new()));
    
    // Spawn multiple tasks adding clients concurrently
    let mut handles = vec![];
    
    for i in 0..10 {
        let clients_clone = Arc::clone(&clients);
        let handle = tokio::spawn(async move {
            let mut clients = clients_clone.write().await;
            clients.add_client(
                WindowHandle::X11(0x5000000 + i),
                WindowType::Normal
            ).unwrap();
        });
        handles.push(handle);
    }
    
    // Wait for all to complete
    for h in handles {
        h.await.unwrap();
    }
    
    // Verify all clients were added
    let clients = clients.read().await;
    assert_eq!(clients.count(), 10);
}

/// Test stacking with complex relationships
#[tokio::test]
async fn test_complex_stacking() {
    let mut clients = ClientManager::new();
    let mut stacking = StackingManager::new();
    
    // Create dock (always on top of normal)
    let dock = clients.add_client(WindowHandle::X11(0x6000001), WindowType::Dock).unwrap();
    stacking.add_client(dock, WindowType::Dock, None).unwrap();
    assert_eq!(stacking.get_layer(dock), Some(StackingLayer::Dock));
    
    // Create normal windows
    let win1 = clients.add_client(WindowHandle::X11(0x6000002), WindowType::Normal).unwrap();
    let win2 = clients.add_client(WindowHandle::X11(0x6000003), WindowType::Normal).unwrap();
    stacking.add_client(win1, WindowType::Normal, None).unwrap();
    stacking.add_client(win2, WindowType::Normal, None).unwrap();
    
    // Dock should be above normal windows
    assert!(stacking.is_above(dock, win1));
    assert!(stacking.is_above(dock, win2));
    
    // Create dialog for win1
    let dialog = clients.add_client(WindowHandle::X11(0x6000004), WindowType::Dialog).unwrap();
    stacking.add_client(dialog, WindowType::Dialog, Some(win1)).unwrap();
    
    // Dialog should be above win1 but below dock
    assert!(stacking.is_above(dialog, win1));
    assert!(stacking.is_above(dock, dialog));
}

/// Test geometry operations
#[tokio::test]
async fn test_geometry_operations() {
    let mut clients = ClientManager::new();
    
    let id = clients.add_client(WindowHandle::X11(0x7000001), WindowType::Normal).unwrap();
    
    // Set geometry
    let geom = Geometry::new(100, 200, 800, 600);
    clients.set_geometry(id, geom).unwrap();
    
    // Verify
    let retrieved = clients.get_geometry(id).unwrap();
    assert_eq!(retrieved.x, 100);
    assert_eq!(retrieved.y, 200);
    assert_eq!(retrieved.width, 800);
    assert_eq!(retrieved.height, 600);
    
    // Test contains
    assert!(geom.contains(500, 400));
    assert!(!geom.contains(50, 50));
    
    // Test intersection
    let other = Geometry::new(500, 400, 400, 400);
    let intersection = geom.intersection(&other);
    assert!(intersection.is_some());
    
    let inter = intersection.unwrap();
    assert_eq!(inter.x, 500);
    assert_eq!(inter.y, 400);
    assert_eq!(inter.width, 400);
    assert_eq!(inter.height, 300);
}
