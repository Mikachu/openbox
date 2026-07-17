//! D-Bus proxy for Openbox

use zbus::proxy;
use zbus::zvariant::OwnedValue;

#[proxy(
    interface = "org.openbox.WindowManager",
    default_service = "org.openbox.WindowManager",
    default_path = "/org/openbox/WindowManager"
)]
pub trait Openbox {
    /// Reconfigure Openbox
    fn reconfigure(&self) -> zbus::Result<()>;

    /// Restart Openbox
    fn restart(&self, save_session: bool) -> zbus::Result<()>;

    /// Exit Openbox
    fn exit(&self, save_session: bool) -> zbus::Result<()>;

    /// Get all windows
    #[zbus(name = "GetWindows")]
    fn get_windows(&self) -> zbus::Result<Vec<(u32, std::collections::HashMap<String, OwnedValue>)>>;

    /// Get window properties
    #[zbus(name = "GetWindow")]
    fn get_window(&self, window_id: u32) -> zbus::Result<std::collections::HashMap<String, OwnedValue>>;

    /// Focus a window
    #[zbus(name = "FocusWindow")]
    fn focus_window(&self, window_id: u32, raise: bool) -> zbus::Result<()>;

    /// Move a window
    #[zbus(name = "MoveWindow")]
    fn move_window(&self, window_id: u32, x: i32, y: i32) -> zbus::Result<()>;

    /// Resize a window
    #[zbus(name = "ResizeWindow")]
    fn resize_window(&self, window_id: u32, width: u32, height: u32) -> zbus::Result<()>;

    /// Close a window
    #[zbus(name = "CloseWindow")]
    fn close_window(&self, window_id: u32) -> zbus::Result<()>;

    /// Iconify a window
    #[zbus(name = "IconifyWindow")]
    fn iconify_window(&self, window_id: u32) -> zbus::Result<()>;

    /// Maximize a window
    #[zbus(name = "MaximizeWindow")]
    fn maximize_window(&self, window_id: u32, full: bool) -> zbus::Result<()>;

    /// Unmaximize a window
    #[zbus(name = "UnmaximizeWindow")]
    fn unmaximize_window(&self, window_id: u32) -> zbus::Result<()>;

    /// Shade a window
    #[zbus(name = "ShadeWindow")]
    fn shade_window(&self, window_id: u32) -> zbus::Result<()>;

    /// Unshade a window
    #[zbus(name = "UnshadeWindow")]
    fn unshade_window(&self, window_id: u32) -> zbus::Result<()>;

    /// Set window desktop
    #[zbus(name = "SetWindowDesktop")]
    fn set_window_desktop(&self, window_id: u32, desktop: i32, follow: bool) -> zbus::Result<()>;

    /// Set window layer
    #[zbus(name = "SetWindowLayer")]
    fn set_window_layer(&self, window_id: u32, layer: String) -> zbus::Result<()>;

    /// Get desktops
    #[zbus(name = "GetDesktops")]
    fn get_desktops(&self) -> zbus::Result<Vec<(i32, String)>>;

    /// Get current desktop
    #[zbus(name = "GetCurrentDesktop")]
    fn get_current_desktop(&self) -> zbus::Result<i32>;

    /// Set current desktop
    #[zbus(name = "SetCurrentDesktop")]
    fn set_current_desktop(&self, desktop: i32) -> zbus::Result<()>;

    /// Add desktop
    #[zbus(name = "AddDesktop")]
    fn add_desktop(&self, name: String) -> zbus::Result<i32>;

    /// Remove desktop
    #[zbus(name = "RemoveDesktop")]
    fn remove_desktop(&self, desktop: i32) -> zbus::Result<()>;

    /// Rename desktop
    #[zbus(name = "RenameDesktop")]
    fn rename_desktop(&self, desktop: i32, name: String) -> zbus::Result<()>;

    /// Get config
    #[zbus(name = "GetConfig")]
    fn get_config(&self, key: String) -> zbus::Result<OwnedValue>;

    /// Set config
    #[zbus(name = "SetConfig")]
    fn set_config(&self, key: String, value: OwnedValue, persistent: bool) -> zbus::Result<()>;

    /// Get theme
    #[zbus(name = "GetTheme")]
    fn get_theme(&self) -> zbus::Result<String>;

    /// Set theme
    #[zbus(name = "SetTheme")]
    fn set_theme(&self, theme_name: String) -> zbus::Result<()>;

    /// Show menu
    #[zbus(name = "ShowMenu")]
    fn show_menu(&self, menu_name: String, x: i32, y: i32) -> zbus::Result<()>;

    /// Execute command
    #[zbus(name = "Execute")]
    fn execute(&self, command: String, args: Vec<String>) -> zbus::Result<()>;

    /// Version property
    #[zbus(property)]
    fn version(&self) -> zbus::Result<String>;

    /// API version property
    #[zbus(property, name = "ApiVersion")]
    fn api_version(&self) -> zbus::Result<u32>;

    /// Window created signal
    #[zbus(signal)]
    fn window_created(&self, window_id: u32, properties: std::collections::HashMap<String, OwnedValue>);

    /// Window destroyed signal
    #[zbus(signal)]
    fn window_destroyed(&self, window_id: u32);

    /// Window focused signal
    #[zbus(signal)]
    fn window_focused(&self, window_id: u32, previous_window_id: u32);

    /// Desktop changed signal
    #[zbus(signal)]
    fn desktop_changed(&self, new_desktop: i32, old_desktop: i32);
}

impl OpenboxProxy<'_> {
    /// Create a new proxy with default connection
    pub async fn new() -> zbus::Result<Self> {
        let connection = zbus::Connection::session().await?;
        Self::new(&connection).await
    }
}
