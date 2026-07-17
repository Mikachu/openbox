//! FFI bindings for the Openbox obt library
//! 
//! This crate provides raw FFI bindings to the obt (Openbox Toolkit)
//! library, generated automatically using bindgen.

#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(improper_ctypes)]

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

use std::ffi::CStr;
use std::fmt;

/// Error type for obt operations
#[derive(Debug)]
pub struct ObtError {
    pub message: String,
}

impl fmt::Display for ObtError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "obt error: {}", self.message)
    }
}

impl std::error::Error for ObtError {}

/// Initialize the obt library
pub fn init() -> Result<(), ObtError> {
    unsafe {
        // obt_init returns gboolean (0 = false, non-zero = true)
        let result = obt_init();
        if result != 0 {
            Ok(())
        } else {
            Err(ObtError {
                message: "Failed to initialize obt".to_string(),
            })
        }
    }
}

/// Shutdown the obt library
pub fn shutdown() {
    unsafe {
        obt_shutdown();
    }
}

/// Get obt version string
pub fn version() -> &'static str {
    unsafe {
        CStr::from_ptr(obt_version())
            .to_str()
            .unwrap_or("unknown")
    }
}

/// Safe wrapper for obt paths
pub mod paths {
    use super::*;
    use std::ffi::{CStr, CString};
    use std::path::PathBuf;

    /// Get the config directory
    pub fn config_dir() -> Option<PathBuf> {
        unsafe {
            let ptr = obt_paths_config_dir();
            if ptr.is_null() {
                return None;
            }
            CStr::from_ptr(ptr)
                .to_str()
                .ok()
                .map(PathBuf::from)
        }
    }

    /// Get the data directory
    pub fn data_dir() -> Option<PathBuf> {
        unsafe {
            let ptr = obt_paths_data_dir();
            if ptr.is_null() {
                return None;
            }
            CStr::from_ptr(ptr)
                .to_str()
                .ok()
                .map(PathBuf::from)
        }
    }

    /// Build a path to a config file
    pub fn build_config_path(filename: &str) -> Result<PathBuf, ObtError> {
        let c_filename = CString::new(filename)
            .map_err(|_| ObtError { message: "Invalid filename".to_string() })?;
        
        unsafe {
            let ptr = obt_paths_build_config_path(c_filename.as_ptr());
            if ptr.is_null() {
                return Err(ObtError {
                    message: "Failed to build config path".to_string(),
                });
            }
            
            let path = CStr::from_ptr(ptr)
                .to_str()
                .map_err(|_| ObtError { message: "Invalid path encoding".to_string() })?;
            
            // Free the returned string
            libc::free(ptr as *mut libc::c_void);
            
            Ok(PathBuf::from(path))
        }
    }
}

/// Safe wrapper for obt XML parsing
pub mod xml {
    use super::*;
    use std::ffi::CString;

    /// Parse an XML file
    pub fn parse_file(path: &str) -> Result<XmlDocPtr, ObtError> {
        let c_path = CString::new(path)
            .map_err(|_| ObtError { message: "Invalid path".to_string() })?;
        
        unsafe {
            let doc = obt_xml_parse_file(c_path.as_ptr());
            if doc.is_null() {
                return Err(ObtError {
                    message: format!("Failed to parse XML file: {}", path),
                });
            }
            Ok(doc)
        }
    }

    /// Free an XML document
    pub fn free_doc(doc: XmlDocPtr) {
        unsafe {
            if !doc.is_null() {
                obt_xml_free_doc(doc);
            }
        }
    }
}

// Re-export common types
pub type XmlDocPtr = *mut ObtXmlDoc;
