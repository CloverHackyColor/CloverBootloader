//! This is a build script for Cargo https://crates.io/
//! It produces a static library that can be used by C or Rust.

extern crate cc;

use std::env;
use std::path::PathBuf;
use std::fs::canonicalize;

fn main() {
    let mut cc = cc::Build::new();
    let compiler = cc.get_compiler();
    cc.warnings(false);

    if env::var("PROFILE").map(|p|p != "debug").unwrap_or(true) {
        cc.define("NDEBUG", Some("1"));
    } else {
        cc.define("DEBUG", Some("1"));
    }

    if cfg!(feature = "openmp") {
        cc.flag(&env::var("DEP_OPENMP_FLAG").unwrap());
    }

    let target_arch = env::var("CARGO_CFG_TARGET_ARCH").expect("Needs CARGO_CFG_TARGET_ARCH");
    if target_arch =="x86_64" ||
       (target_arch == "x86" && cfg!(feature = "sse")) {
        cc.flag(if compiler.is_like_msvc() {"/arch:SSE2"} else {"-msse"});
        cc.define("USE_SSE", Some("1"));
    }

    let outdated_c_compiler = env::var("TARGET").unwrap().contains("windows-msvc");
    let has_msvc_files = PathBuf::from("msvc-dist/libimagequant.c").exists();

    if outdated_c_compiler && has_msvc_files {
        println!("cargo:include={}", canonicalize("msvc-dist").unwrap().display());
        cc.file("msvc-dist/libimagequant.c")
            .file("msvc-dist/nearest.c")
            .file("msvc-dist/kmeans.c")
            .file("msvc-dist/mediancut.c")
            .file("msvc-dist/mempool.c")
            .file("msvc-dist/pam.c")
            .file("msvc-dist/blur.c");
    } else {
        // This is so that I don't forget to publish MSVC version as well
        if !has_msvc_files {
            println!("cargo:warning=msvc-dist/ directory not present. MSVC builds may fail");
        }
        println!("cargo:include={}", canonicalize(".").unwrap().display());
        if !compiler.is_like_msvc() {
            cc.flag("-std=c99");
        }
        cc.file("libimagequant.c")
            .file("nearest.c")
            .file("kmeans.c")
            .file("mediancut.c")
            .file("mempool.c")
            .file("pam.c")
            .file("blur.c");
    }

    cc.compile("libimagequant.a");
}
