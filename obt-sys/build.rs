//! Build script for obt-sys FFI bindings

use std::env;
use std::path::PathBuf;

fn main() {
    println!("cargo:rerun-if-changed=wrapper.h");
    println!("cargo:rerun-if-changed=../../obt");
    
    // Tell cargo to link the C libraries
    println!("cargo:rustc-link-lib=obt");
    
    // Generate bindings
    let bindings = bindgen::Builder::default()
        .header("wrapper.h")
        .clang_arg("-I../../obt")
        .clang_arg("-I/usr/include/glib-2.0")
        .clang_arg("-I/usr/lib/glib-2.0/include")
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))
        .allowlist_function("obt_.*")
        .allowlist_type("Obt.*")
        .allowlist_var("OBT_.*")
        .generate()
        .expect("Unable to generate bindings");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");
}
