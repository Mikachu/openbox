//! Openbox Core - Modern Rust Window Manager
//! 
//! This crate provides the core window manager logic for Openbox,
//! implementing memory-safe window management, focus tracking,
//! and stacking order management.

#![warn(missing_docs)]
#![warn(rust_2018_idioms)]
#![deny(unsafe_code)]

pub mod client;
pub mod focus;
pub mod stacking;
pub mod screen;
pub mod config;
pub mod event;
pub mod error;
pub mod types;
pub mod utils;

use std::sync::Arc;
use tokio::sync::RwLock;

pub use client::{Client, ClientId, ClientManager};
pub use focus::{FocusModel, FocusTracker};
pub use stacking::{StackingLayer, StackingManager};
pub use types::*;
pub use error::{OpenboxError, Result};

/// Core window manager state
pub struct WindowManager {
    /// Client management
    pub clients: Arc<RwLock<ClientManager>>,
    /// Focus tracking
    pub focus: Arc<RwLock<FocusTracker>>,
    /// Stacking order management
    pub stacking: Arc<RwLock<StackingManager>>,
    /// Screen information
    pub screens: Arc<RwLock<screen::ScreenManager>>,
    /// Configuration
    pub config: Arc<RwLock<config::Config>>,
    /// Event dispatcher
    pub events: event::EventDispatcher,
}

impl WindowManager {
    /// Create a new window manager instance
    pub async fn new() -> Result<Self> {
        let clients = Arc::new(RwLock::new(ClientManager::new()));
        let focus = Arc::new(RwLock::new(FocusTracker::new()));
        let stacking = Arc::new(RwLock::new(StackingManager::new()));
        let screens = Arc::new(RwLock::new(screen::ScreenManager::new()?));
        let config = Arc::new(RwLock::new(config::Config::default()));
        
        let events = event::EventDispatcher::new(
            Arc::clone(&clients),
            Arc::clone(&focus),
            Arc::clone(&stacking),
        );

        Ok(Self {
            clients,
            focus,
            stacking,
            screens,
            config,
            events,
        })
    }

    /// Initialize the window manager
    pub async fn init(&mut self) -> Result<()> {
        tracing::info!("Initializing Openbox Core");
        
        // Initialize screen manager
        {
            let mut screens = self.screens.write().await;
            screens.init().await?;
        }
        
        // Set up initial focus model
        {
            let mut focus = self.focus.write().await;
            focus.set_model(FocusModel::ClickToFocus);
        }
        
        tracing::info!("Openbox Core initialized successfully");
        Ok(())
    }

    /// Run the main event loop
    pub async fn run(&mut self) -> Result<()> {
        tracing::info!("Starting main event loop");
        self.events.run().await
    }

    /// Graceful shutdown
    pub async fn shutdown(&mut self) -> Result<()> {
        tracing::info!("Shutting down Openbox Core");
        
        // Save state
        let clients = self.clients.read().await;
        clients.save_state().await?;
        
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[tokio::test]
    async fn test_wm_creation() {
        let wm = WindowManager::new().await;
        assert!(wm.is_ok());
    }
}
