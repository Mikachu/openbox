//! # Openbox Core Library
//!
//! The core window management logic for Openbox 4, written in Rust.
//!
//! ## Architecture
//!
//! The library is organized into modules:
//!
//! - [`client`] - Window client management
//! - [`focus`] - Focus tracking and management
//! - [`stack`] - Window stacking order
//! - [`config`] - Configuration parsing and management
//! - [`event`] - X11/Wayland event handling
//! - [`theme`] - Theme loading and application
//!
//! ## Example Usage
//!
//! ```rust,no_run
//! use ob_core::WindowManager;
//!
//! #[tokio::main]
//! async fn main() -> Result<(), Box<dyn std::error::Error>> {
//!     let wm = WindowManager::new().await?;
//!     wm.run().await?;
//!     Ok(())
//! }
//! ```

#![warn(missing_docs)]
#![warn(rustdoc::missing_doc_code_examples)]

pub mod client;
pub mod config;
pub mod dbus;
pub mod error;
pub mod event;
pub mod focus;
pub mod keyboard;
pub mod mouse;
pub mod screen;
pub mod stack;
pub mod theme;
pub mod window;

use std::sync::Arc;
use tokio::sync::RwLock;

/// Main window manager instance.
///
/// This is the primary interface for controlling the window manager.
/// It maintains the global state and coordinates between subsystems.
///
/// # Thread Safety
///
/// The WindowManager is thread-safe and can be shared between tasks.
/// All mutable state is protected by appropriate synchronization primitives.
///
/// # Example
///
/// ```no_run
/// use ob_core::WindowManager;
/// use std::sync::Arc;
///
/// async fn example() {
///     let wm = Arc::new(WindowManager::new().await.unwrap());
///     // Window manager is now running...
/// }
/// ```
pub struct WindowManager {
    /// Configuration settings
    config: Arc<RwLock<config::Config>>,
    /// Client manager for window tracking
    clients: Arc<client::ClientManager>,
    /// Focus tracking
    focus: Arc<RwLock<focus::FocusTracker>>,
    /// Window stacking order
    stack: Arc<RwLock<stack::StackManager>>,
    /// Event dispatcher
    event: Arc<event::EventDispatcher>,
}

impl WindowManager {
    /// Creates a new WindowManager instance.
    ///
    /// # Errors
    ///
    /// Returns an error if:
    /// - Display connection fails
    /// - Configuration cannot be loaded
    /// - Required resources are unavailable
    ///
    /// # Examples
    ///
    /// ```no_run
    /// use ob_core::WindowManager;
    ///
    /// async fn create_wm() {
    ///     match WindowManager::new().await {
    ///         Ok(wm) => println!("Window manager created"),
    ///         Err(e) => eprintln!("Failed to create: {}", e),
    ///     }
    /// }
    /// ```
    pub async fn new() -> Result<Self, error::WindowManagerError> {
        let config = Arc::new(RwLock::new(config::Config::load_default().await?));
        let clients = Arc::new(client::ClientManager::new());
        let focus = Arc::new(RwLock::new(focus::FocusTracker::new()));
        let stack = Arc::new(RwLock::new(stack::StackManager::new()));
        let event = Arc::new(event::EventDispatcher::new());

        Ok(Self {
            config,
            clients,
            focus,
            stack,
            event,
        })
    }

    /// Runs the window manager main loop.
    ///
    /// This method blocks until the window manager exits.
    /// It processes events, updates state, and dispatches actions.
    ///
    /// # Returns
    ///
    /// Returns `Ok(())` on graceful shutdown, or an error if a fatal
    /// error occurs.
    pub async fn run(&self) -> Result<(), error::WindowManagerError> {
        // Implementation...
        Ok(())
    }

    /// Returns the current configuration.
    ///
    /// # Example
    ///
    /// ```no_run
    /// # use ob_core::WindowManager;
    /// # async fn example(wm: &WindowManager) {
    /// let config = wm.get_config().await;
    /// println!("Theme: {}", config.theme.name);
    /// # }
    /// ```
    pub async fn get_config(&self) -> config::Config {
        self.config.read().await.clone()
    }

    /// Reconfigures the window manager with new settings.
    ///
    /// Reloads configuration from disk and applies changes
    /// without restarting. Some changes may require a restart.
    ///
    /// # Errors
    ///
    /// Returns an error if configuration cannot be parsed.
    pub async fn reconfigure(&self) -> Result<(), error::WindowManagerError> {
        let new_config = config::Config::load_default().await?;
        let mut config = self.config.write().await;
        *config = new_config;
        Ok(())
    }
}

/// Version information for the Openbox core library.
///
/// # Example
///
/// ```
/// use ob_core::VERSION;
///
/// println!("Openbox core version: {}", VERSION);
/// ```
pub const VERSION: &str = env!("CARGO_PKG_VERSION");

/// Library name and version information.
///
/// Useful for logging and debugging.
pub const ID: &str = concat!("ob-core ", env!("CARGO_PKG_VERSION"));
