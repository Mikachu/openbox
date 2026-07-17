//! Openbox FFI - C API Compatibility Layer
//! 
//! This crate provides a C-compatible API for the Rust core,
//! allowing existing C plugins and tools to work with the new
//! Rust implementation.

use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int, c_void};
use std::sync::Arc;
use tokio::runtime::Runtime;

/// Opaque handle to window manager
pub struct WmHandle {
    pub runtime: Arc<Runtime>,
    // Would contain actual WindowManager reference
}

/// Initialize the FFI layer
/// 
/// # Safety
/// Must be called before any other FFI functions
#[no_mangle]
pub unsafe extern "C" fn openbox_init() -> c_int {
    // Initialize logging
    if let Err(_) = std::panic::catch_unwind(|| {
        tracing_subscriber::fmt::init();
    }) {
        return -1;
    }
    
    0 // Success
}

/// Shutdown the FFI layer
#[no_mangle]
pub extern "C" fn openbox_shutdown() {
    // Cleanup
}

/// Get version string
/// 
/// # Safety
/// Caller must not modify returned string
#[no_mangle]
pub unsafe extern "C" fn openbox_version() -> *const c_char {
    static VERSION: &str = concat!("Openbox Rust ", env!("CARGO_PKG_VERSION"), "\0");
    VERSION.as_ptr() as *const c_char
}

/// Create a window manager instance
/// 
/// # Safety
/// Returns a valid handle or null on error
#[no_mangle]
pub unsafe extern "C" fn openbox_wm_create() -> *mut WmHandle {
    let runtime = match Runtime::new() {
        Ok(rt) => Arc::new(rt),
        Err(_) => return std::ptr::null_mut(),
    };
    
    let handle = Box::new(WmHandle { runtime });
    Box::into_raw(handle)
}

/// Destroy a window manager instance
/// 
/// # Safety
/// handle must be a valid pointer from openbox_wm_create
#[no_mangle]
pub unsafe extern "C" fn openbox_wm_destroy(handle: *mut WmHandle) {
    if !handle.is_null() {
        drop(Box::from_raw(handle));
    }
}

/// Run the main event loop
/// 
/// # Safety
/// handle must be valid
#[no_mangle]
pub unsafe extern "C" fn openbox_wm_run(handle: *mut WmHandle) -> c_int {
    if handle.is_null() {
        return -1;
    }
    
    // In real implementation, would run the event loop
    // For now, just block
    std::thread::park();
    
    0
}

/// Client management
pub mod client {
    use super::*;
    use openbox_core::client::{ClientId, ClientManager};
    
    /// Opaque client handle
    pub type ClientHandle = usize;
    
    /// Get client count
    #[no_mangle]
    pub unsafe extern "C" fn openbox_client_count() -> c_int {
        // Would access actual client manager
        0
    }
    
    /// Get client by index
    #[no_mangle]
    pub unsafe extern "C" fn openbox_client_get(index: c_int) -> ClientHandle {
        index as ClientHandle
    }
    
    /// Get client title
    /// 
    /// # Safety
    /// Returns allocated string that caller must free with openbox_free_string
    #[no_mangle]
    pub unsafe extern "C" fn openbox_client_get_title(handle: ClientHandle) -> *mut c_char {
        let title = format!("Client {}\0", handle);
        let c_title = match CString::new(title) {
            Ok(s) => s,
            Err(_) => return std::ptr::null_mut(),
        };
        
        c_title.into_raw()
    }
    
    /// Focus a client
    #[no_mangle]
    pub unsafe extern "C" fn openbox_client_focus(handle: ClientHandle) -> c_int {
        // Would set focus through focus tracker
        0
    }
    
    /// Close a client
    #[no_mangle]
    pub unsafe extern "C" fn openbox_client_close(handle: ClientHandle) -> c_int {
        // Would send close message
        0
    }
}

/// Configuration
pub mod config {
    use super::*;
    
    /// Load configuration from file
    /// 
    /// # Safety
    /// path must be a valid null-terminated UTF-8 string
    #[no_mangle]
    pub unsafe extern "C" fn openbox_config_load(path: *const c_char) -> c_int {
        if path.is_null() {
            return -1;
        }
        
        let path_str = match CStr::from_ptr(path).to_str() {
            Ok(s) => s,
            Err(_) => return -1,
        };
        
        // Would load config through config module
        tracing::info!("Loading config from: {}", path_str);
        
        0
    }
}

/// String utilities
/// 
/// # Safety
/// s must have been allocated by openbox
#[no_mangle]
pub unsafe extern "C" fn openbox_free_string(s: *mut c_char) {
    if !s.is_null() {
        drop(CString::from_raw(s));
    }
}

/// Action execution
pub mod actions {
    use super::*;
    use openbox_core::types::Action;
    
    /// Execute an action by name
    /// 
    /// # Safety
    /// name must be a valid null-terminated UTF-8 string
    #[no_mangle]
    pub unsafe extern "C" fn openbox_action_execute(name: *const c_char) -> c_int {
        if name.is_null() {
            return -1;
        }
        
        let name_str = match CStr::from_ptr(name).to_str() {
            Ok(s) => s,
            Err(_) => return -1,
        };
        
        tracing::info!("Executing action: {}", name_str);
        
        // Would parse and execute action
        0
    }
}

/// Error handling
#[no_mangle]
pub unsafe extern "C" fn openbox_last_error() -> *const c_char {
    static NO_ERROR: &[u8] = b"No error\0";
    NO_ERROR.as_ptr() as *const c_char
}

/// Logging
#[no_mangle]
pub unsafe extern "C" fn openbox_log(level: c_int, message: *const c_char) {
    if message.is_null() {
        return;
    }
    
    let msg = match CStr::from_ptr(message).to_str() {
        Ok(s) => s,
        Err(_) => return,
    };
    
    match level {
        0 => tracing::error!("{}", msg),
        1 => tracing::warn!("{}", msg),
        2 => tracing::info!("{}", msg),
        3 => tracing::debug!("{}", msg),
        _ => tracing::trace!("{}", msg),
    }
}
