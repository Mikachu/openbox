//! Error types for Openbox Core

use thiserror::Error;

/// Result type alias for Openbox operations
pub type Result<T> = std::result::Result<T, OpenboxError>;

/// Main error type for Openbox Core
#[derive(Error, Debug, Clone)]
pub enum OpenboxError {
    /// Client-related errors
    #[error("Client error: {0}")]
    Client(String),

    /// Focus tracking errors
    #[error("Focus error: {0}")]
    Focus(String),

    /// Stacking order errors
    #[error("Stacking error: {0}")]
    Stacking(String),

    /// Screen/display errors
    #[error("Screen error: {0}")]
    Screen(String),

    /// Configuration errors
    #[error("Configuration error: {0}")]
    Config(String),

    /// Event handling errors
    #[error("Event error: {0}")]
    Event(String),

    /// Platform-specific errors (X11/Wayland)
    #[error("Platform error: {0}")]
    Platform(String),

    /// FFI errors
    #[error("FFI error: {0}")]
    Ffi(String),

    /// I/O errors
    #[error("I/O error: {0}")]
    Io(String),

    /// Invalid state
    #[error("Invalid state: {0}")]
    InvalidState(String),

    /// Not implemented
    #[error("Not implemented: {0}")]
    NotImplemented(String),
}

impl From<std::io::Error> for OpenboxError {
    fn from(e: std::io::Error) -> Self {
        OpenboxError::Io(e.to_string())
    }
}

impl From<serde_json::Error> for OpenboxError {
    fn from(e: serde_json::Error) -> Self {
        OpenboxError::Config(e.to_string())
    }
}
