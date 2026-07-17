//! D-Bus service implementation for Openbox

use openbox_core::client::{ClientId, ClientManager, WindowState, WindowType};
use openbox_core::focus::{FocusReason, FocusTracker};
use openbox_core::stacking::StackingManager;
use std::collections::HashMap;
use std::sync::Arc;
use tokio::sync::RwLock;
use tracing::{debug, error, info, warn};
use zbus::fdo::Result as ZbusResult;
use zbus::interface;
use zbus::zvariant::{OwnedValue, Value};

/// Window manager D-Bus service
pub struct WindowManagerService {
    clients: Arc<RwLock<ClientManager>>,
    focus: Arc<RwLock<FocusTracker>>,
    stacking: Arc<RwLock<StackingManager>>,
    version: String,
}

impl WindowManagerService {
    /// Create a new service instance
    pub fn new(
        clients: Arc<RwLock<ClientManager>>,
        focus: Arc<RwLock<FocusTracker>>,
        stacking: Arc<RwLock<StackingManager>>,
    ) -> Self {
        Self {
            clients,
            focus,
            stacking,
            version: env!("CARGO_PKG_VERSION").to_string(),
        }
    }

    /// Convert ClientId to D-Bus window ID (u32)
    fn client_id_to_u32(&self, id: ClientId) -> u32 {
        // Use the internal slotmap key as u32
        id.data().as_ffi()
    }

    /// Convert D-Bus window ID to ClientId
    fn u32_to_client_id(&self, id: u32) -> Option<ClientId> {
        // This is a simplified conversion - in practice would need proper mapping
        Some(ClientId::from(id))
    }
}

#[interface(name = "org.openbox.WindowManager")]
impl WindowManagerService {
    /// Get Openbox version
    #[zbus(property)]
    fn version(&self) -> String {
        self.version.clone()
    }

    /// Get API version
    #[zbus(property)]
    fn api_version(&self) -> u32 {
        1
    }

    /// Reconfigure Openbox (reload config)
    fn reconfigure(&self) {
        info!("D-Bus: Reconfigure requested");
        // Signal to main thread to reconfigure
        // Implementation would send message to main loop
    }

    /// Restart Openbox
    fn restart(&self, save_session: bool) {
        info!("D-Bus: Restart requested (save_session={})", save_session);
        // Signal to main thread to restart
    }

    /// Exit Openbox
    fn exit(&self, save_session: bool) {
        info!("D-Bus: Exit requested (save_session={})", save_session);
        // Signal to main thread to exit
    }

    /// Get all windows
    async fn get_windows(&self) -> ZbusResult<Vec<(u32, HashMap<String, OwnedValue>)>> {
        let clients = self.clients.read().await;
        let mut result = Vec::new();

        for id in clients.get_all_clients() {
            if let Some(client) = clients.get(id) {
                let c = client.read();
                let mut props = HashMap::new();

                props.insert("id".to_string(), OwnedValue::from(self.client_id_to_u32(id)));
                props.insert("title".to_string(), OwnedValue::from(c.title.clone()));
                props.insert("class".to_string(), OwnedValue::from(c.class.clone()));
                props.insert("desktop".to_string(), OwnedValue::from(c.desktop));
                props.insert("mapped".to_string(), OwnedValue::from(c.mapped));
                props.insert("managed".to_string(), OwnedValue::from(c.managed));

                // Add states
                let states: Vec<String> = c.states
                    .iter()
                    .map(|s| format!("{:?}", s))
                    .collect();
                props.insert("states".to_string(), OwnedValue::from(states));

                // Add geometry
                let geom = vec![
                    c.geometry.x,
                    c.geometry.y,
                    c.geometry.width as i32,
                    c.geometry.height as i32,
                ];
                props.insert("geometry".to_string(), OwnedValue::from(geom));

                result.push((self.client_id_to_u32(id), props));
            }
        }

        Ok(result)
    }

    /// Get specific window properties
    async fn get_window(&self, window_id: u32) -> ZbusResult<HashMap<String, OwnedValue>> {
        let id = self.u32_to_client_id(window_id)
            .ok_or_else(|| zbus::fdo::Error::InvalidArgs("Invalid window ID".to_string()))?;

        let clients = self.clients.read().await;
        
        if let Some(client) = clients.get(id) {
            let c = client.read();
            let mut props = HashMap::new();

            props.insert("id".to_string(), OwnedValue::from(window_id));
            props.insert("title".to_string(), OwnedValue::from(c.title.clone()));
            props.insert("class".to_string(), OwnedValue::from(c.class.clone()));
            props.insert("name".to_string(), OwnedValue::from(c.name.clone()));
            props.insert("role".to_string(), OwnedValue::from(c.role.clone().unwrap_or_default()));
            props.insert("desktop".to_string(), OwnedValue::from(c.desktop));
            props.insert("mapped".to_string(), OwnedValue::from(c.mapped));
            props.insert("managed".to_string(), OwnedValue::from(c.managed));
            props.insert("destroyed".to_string(), OwnedValue::from(c.destroyed));

            // Window type
            props.insert("type".to_string(), 
                OwnedValue::from(format!("{:?}", c.window_type)));

            // States
            let states: Vec<String> = c.states
                .iter()
                .map(|s| format!("{:?}", s))
                .collect();
            props.insert("states".to_string(), OwnedValue::from(states));

            // Geometry
            let geom = vec![
                c.geometry.x,
                c.geometry.y,
                c.geometry.width as i32,
                c.geometry.height as i32,
            ];
            props.insert("geometry".to_string(), OwnedValue::from(geom));

            // Process info
            if let Some(pid) = c.pid {
                props.insert("pid".to_string(), OwnedValue::from(pid));
            }
            if let Some(ref machine) = c.machine {
                props.insert("machine".to_string(), OwnedValue::from(machine.clone()));
            }

            Ok(props)
        } else {
            Err(zbus::fdo::Error::InvalidArgs("Window not found".to_string()))
        }
    }

    /// Focus a window
    async fn focus_window(&self, window_id: u32, raise: bool) -> ZbusResult<()> {
        let id = self.u32_to_client_id(window_id)
            .ok_or_else(|| zbus::fdo::Error::InvalidArgs("Invalid window ID".to_string()))?;

        let mut focus = self.focus.write().await;
        let clients = self.clients.read().await;

        focus.set_focus(id, FocusReason::ApplicationRequest)
            .map_err(|e| zbus::fdo::Error::Failed(e.to_string()))?;

        if raise {
            let mut stacking = self.stacking.write().await;
            stacking.raise(id)
                .map_err(|e| zbus::fdo::Error::Failed(e.to_string()))?;
        }

        Ok(())
    }

    /// Move a window
    async fn move_window(&self, window_id: u32, x: i32, y: i32) -> ZbusResult<()> {
        let id = self.u32_to_client_id(window_id)
            .ok_or_else(|| zbus::fdo::Error::InvalidArgs("Invalid window ID".to_string()))?;

        let mut clients = self.clients.write().await;
        
        clients.set_geometry(id, openbox_core::types::Geometry::new(
            x, y,
            clients.get_geometry(id)
                .map(|g| g.width)
                .unwrap_or(800),
            clients.get_geometry(id)
                .map(|g| g.height)
                .unwrap_or(600),
        )).map_err(|e| zbus::fdo::Error::Failed(e.to_string()))?;

        Ok(())
    }

    /// Resize a window
    async fn resize_window(&self, window_id: u32, width: u32, height: u32) -> ZbusResult<()> {
        let id = self.u32_to_client_id(window_id)
            .ok_or_else(|| zbus::fdo::Error::InvalidArgs("Invalid window ID".to_string()))?;

        let mut clients = self.clients.write().await;
        
        clients.set_geometry(id, openbox_core::types::Geometry::new(
            clients.get_geometry(id)
                .map(|g| g.x)
                .unwrap_or(0),
            clients.get_geometry(id)
                .map(|g| g.y)
                .unwrap_or(0),
            width,
            height,
        )).map_err(|e| zbus::fdo::Error::Failed(e.to_string()))?;

        Ok(())
    }

    /// Close a window
    fn close_window(&self, window_id: u32) -> ZbusResult<()> {
        info!("D-Bus: Close window {}", window_id);
        // Send WM_DELETE_WINDOW or destroy
        Ok(())
    }

    /// Iconify (minimize) a window
    fn iconify_window(&self, window_id: u32) -> ZbusResult<()> {
        info!("D-Bus: Iconify window {}", window_id);
        Ok(())
    }

    /// Maximize a window
    fn maximize_window(&self, window_id: u32, full: bool) -> ZbusResult<()> {
        info!("D-Bus: Maximize window {} (full={})", window_id, full);
        Ok(())
    }

    /// Unmaximize a window
    fn unmaximize_window(&self, window_id: u32) -> ZbusResult<()> {
        info!("D-Bus: Unmaximize window {}", window_id);
        Ok(())
    }

    /// Shade (roll up) a window
    fn shade_window(&self, window_id: u32) -> ZbusResult<()> {
        info!("D-Bus: Shade window {}", window_id);
        Ok(())
    }

    /// Unshade a window
    fn unshade_window(&self, window_id: u32) -> ZbusResult<()> {
        info!("D-Bus: Unshade window {}", window_id);
        Ok(())
    }

    /// Move window to desktop
    fn set_window_desktop(&self, window_id: u32, desktop: i32, follow: bool) -> ZbusResult<()> {
        info!("D-Bus: Set window {} to desktop {} (follow={})", window_id, desktop, follow);
        Ok(())
    }

    /// Set window layer (stacking)
    fn set_window_layer(&self, window_id: u32, layer: String) -> ZbusResult<()> {
        info!("D-Bus: Set window {} layer to {}", window_id, layer);
        Ok(())
    }

    /// Get desktops
    fn get_desktops(&self) -> ZbusResult<Vec<(i32, String)>> {
        // Return configured desktops
        Ok(vec![
            (0, "Desktop 1".to_string()),
            (1, "Desktop 2".to_string()),
            (2, "Desktop 3".to_string()),
            (3, "Desktop 4".to_string()),
        ])
    }

    /// Get current desktop
    fn get_current_desktop(&self) -> ZbusResult<i32> {
        Ok(0) // Would get actual current desktop
    }

    /// Set current desktop
    fn set_current_desktop(&self, desktop: i32) -> ZbusResult<()> {
        info!("D-Bus: Switch to desktop {}", desktop);
        Ok(())
    }

    /// Add a desktop
    fn add_desktop(&self, name: String) -> ZbusResult<i32> {
        info!("D-Bus: Add desktop '{}'", name);
        Ok(4) // Return new desktop number
    }

    /// Remove a desktop
    fn remove_desktop(&self, desktop: i32) -> ZbusResult<()> {
        info!("D-Bus: Remove desktop {}", desktop);
        Ok(())
    }

    /// Rename a desktop
    fn rename_desktop(&self, desktop: i32, name: String) -> ZbusResult<()> {
        info!("D-Bus: Rename desktop {} to '{}'", desktop, name);
        Ok(())
    }

    /// Get configuration value
    fn get_config(&self, key: String) -> ZbusResult<OwnedValue> {
        debug!("D-Bus: Get config '{}'", key);
        Ok(OwnedValue::from(String::new()))
    }

    /// Set configuration value
    fn set_config(&self, key: String, value: Value<'_>, persistent: bool) -> ZbusResult<()> {
        info!("D-Bus: Set config '{}' = {:?} (persistent={})", key, value, persistent);
        Ok(())
    }

    /// Get current theme
    fn get_theme(&self) -> ZbusResult<String> {
        Ok("Clearlooks".to_string()) // Would get actual theme
    }

    /// Set theme
    fn set_theme(&self, theme_name: String) -> ZbusResult<()> {
        info!("D-Bus: Set theme to '{}'", theme_name);
        Ok(())
    }

    /// Show a menu
    fn show_menu(&self, menu_name: String, x: i32, y: i32) -> ZbusResult<()> {
        info!("D-Bus: Show menu '{}' at ({}, {})", menu_name, x, y);
        Ok(())
    }

    /// Execute a command
    fn execute(&self, command: String, args: Vec<String>) -> ZbusResult<()> {
        info!("D-Bus: Execute '{}' with args {:?}", command, args);
        
        tokio::spawn(async move {
            match tokio::process::Command::new(&command)
                .args(&args)
                .spawn() {
                Ok(mut child) => {
                    if let Err(e) = child.wait().await {
                        error!("Command failed: {}", e);
                    }
                }
                Err(e) => {
                    error!("Failed to spawn command: {}", e);
                }
            }
        });
        
        Ok(())
    }

    // Signals - these are emitted by the service

    /// Window created signal
    #[zbus(signal)]
    async fn window_created(
        &self,
        window_id: u32,
        properties: HashMap<String, OwnedValue>,
    ) -> ZbusResult<()>;

    /// Window destroyed signal
    #[zbus(signal)]
    async fn window_destroyed(&self, window_id: u32) -> ZbusResult<()>;

    /// Window focused signal
    #[zbus(signal)]
    async fn window_focused(
        &self,
        window_id: u32,
        previous_window_id: u32,
    ) -> ZbusResult<()>;

    /// Window property changed signal
    #[zbus(signal)]
    async fn window_property_changed(
        &self,
        window_id: u32,
        property_name: String,
        new_value: OwnedValue,
    ) -> ZbusResult<()>;

    /// Desktop changed signal
    #[zbus(signal)]
    async fn desktop_changed(
        &self,
        new_desktop: i32,
        old_desktop: i32,
    ) -> ZbusResult<()>;

    /// Config changed signal
    #[zbus(signal)]
    async fn config_changed(
        &self,
        key: String,
        new_value: OwnedValue,
    ) -> ZbusResult<()>;

    /// Theme changed signal
    #[zbus(signal)]
    async fn theme_changed(&self, theme_name: String) -> ZbusResult<()>;
}

/// Run the D-Bus service
pub async fn run_service(
    clients: Arc<RwLock<ClientManager>>,
    focus: Arc<RwLock<FocusTracker>>,
    stacking: Arc<RwLock<StackingManager>>,
) -> anyhow::Result<()> {
    let service = WindowManagerService::new(clients, focus, stacking);
    
    let connection = zbus::connection::Builder::session()?
        .name("org.openbox.WindowManager")?
        .serve_at("/org/openbox/WindowManager", service)?
        .build()
        .await?;

    info!("D-Bus service running on org.openbox.WindowManager");

    // Keep connection alive
    loop {
        tokio::time::sleep(tokio::time::Duration::from_secs(1)).await;
    }
}
