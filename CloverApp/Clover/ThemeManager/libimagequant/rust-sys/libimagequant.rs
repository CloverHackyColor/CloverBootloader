//! Small, portable C library for high-quality conversion of RGBA images to 8-bit indexed-color (palette) images.
//! It's powering [pngquant2](https://pngquant.org).
//!
//! This is a low-level crate exposing a C API. If you're looking for a Rust library, see [imagequant](https://crates.io/crates/imagequant).
//!
//! ## License
//!
//! Libimagequant is dual-licensed:
//!
//! * For Free/Libre Open Source Software it's available under [GPL v3 or later](https://raw.github.com/ImageOptim/libimagequant/master/COPYRIGHT) with additional copyright notices for older parts of the code.
//!
//! * For use in non-GPL software (e.g. closed-source or App Store distribution) please ask kornel@pngquant.org for a commercial license.
//!
//! ## Overview
//!
//! The basic flow is:
//!
//! 1. Create attributes object and configure the library.
//! 2. Create image object from RGBA pixels or data source.
//! 3. Perform quantization (generate palette).
//! 4. Store remapped image and final palette.
//! 5. Free memory.
//!
//! Please note that libimagequant only handles raw uncompressed arrays of pixels in memory and is completely independent of any file format.
//!
//! There are 3 ways to create image object for quantization:
//!
//!   * `liq_image_create_rgba()` for simple, contiguous RGBA pixel arrays (width×height×4 bytes large bitmap).
//!   * `liq_image_create_rgba_rows()` for non-contiguous RGBA pixel arrays (that have padding between rows or reverse order, e.g. BMP).
//!   * `liq_image_create_custom()` for RGB, ABGR, YUV and all other formats that can be converted on-the-fly to RGBA (you have to supply the conversion function).
//!
//! Note that "image" here means raw uncompressed pixels. If you have a compressed image file, such as PNG, you must use another library (e.g. lodepng) to decode it first.

#![allow(non_camel_case_types)]

#[cfg(feature = "openmp")]
extern crate openmp_sys;

use std::os::raw::{c_int, c_uint, c_char, c_void};
use std::error;
use std::fmt;
use std::error::Error;

pub enum liq_attr {}
pub enum liq_image {}
pub enum liq_result {}
pub enum liq_histogram {}
pub type liq_color = rgb::RGBA8;

#[repr(C)]
#[derive(Copy, Clone, Debug, Eq, PartialEq)]
pub enum liq_error {
    LIQ_OK = 0,
    LIQ_QUALITY_TOO_LOW = 99,
    LIQ_VALUE_OUT_OF_RANGE = 100,
    LIQ_OUT_OF_MEMORY,
    LIQ_ABORTED,
    LIQ_BITMAP_NOT_AVAILABLE,
    LIQ_BUFFER_TOO_SMALL,
    LIQ_INVALID_POINTER,
    LIQ_UNSUPPORTED,
}

#[repr(C)]
#[derive(Copy, Clone)]
pub enum liq_ownership {
    LIQ_OWN_ROWS = 4,
    LIQ_OWN_PIXELS = 8,
    LIQ_COPY_PIXELS = 16,
}

#[repr(C)]
pub struct liq_palette {
    pub count: c_int,
    pub entries: [liq_color; 256],
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct liq_histogram_entry {
    pub color: liq_color,
    pub count: c_uint,
}

impl error::Error for liq_error {
    fn description(&self) -> &str {
        match *self {
            liq_error::LIQ_OK => "OK",
            liq_error::LIQ_QUALITY_TOO_LOW => "LIQ_QUALITY_TOO_LOW",
            liq_error::LIQ_VALUE_OUT_OF_RANGE => "VALUE_OUT_OF_RANGE",
            liq_error::LIQ_OUT_OF_MEMORY => "OUT_OF_MEMORY",
            liq_error::LIQ_ABORTED => "LIQ_ABORTED",
            liq_error::LIQ_BITMAP_NOT_AVAILABLE => "BITMAP_NOT_AVAILABLE",
            liq_error::LIQ_BUFFER_TOO_SMALL => "BUFFER_TOO_SMALL",
            liq_error::LIQ_INVALID_POINTER => "INVALID_POINTER",
            liq_error::LIQ_UNSUPPORTED => "LIQ_UNSUPPORTED",
        }
    }
}

impl fmt::Display for liq_error {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", self.description())
    }
}

impl liq_error {
    #[inline]
    pub fn is_ok(&self) -> bool {
        *self == liq_error::LIQ_OK
    }

    pub fn is_err(&self) -> bool {
        !self.is_ok()
    }

    #[inline]
    pub fn ok(&self) -> Result<(), liq_error> {
        match *self {
            liq_error::LIQ_OK => Ok(()),
            e => Err(e),
        }
    }

    #[inline]
    pub fn ok_or<E>(self, err: E) -> Result<(), E> {
        if self.is_ok() {
            Ok(())
        } else {
            Err(err)
        }
    }

    pub fn unwrap(&self) {
        assert!(self.is_ok(), "{}", self);
    }

    pub fn expect(&self, msg: &str) {
        assert!(self.is_ok(), "{}", msg);
    }
}

pub type liq_log_callback_function = Option<unsafe extern "C" fn(arg1: &liq_attr, message: *const c_char, user_info: *mut c_void)>;
pub type liq_log_flush_callback_function = Option<unsafe extern "C" fn(arg1: &liq_attr, user_info: *mut c_void)>;
pub type liq_progress_callback_function = Option<unsafe extern "C" fn(progress_percent: f32, user_info: *mut c_void) -> c_int>;
pub type liq_image_get_rgba_row_callback = unsafe extern "C" fn(row_out: *mut liq_color, row: c_int, width: c_int, user_info: *mut c_void);

#[link(name="imagequant", kind="static")]
extern "C" {

    /// Returns object that will hold initial settings (attributes) for the library.
    ///
    /// The object should be freed using `liq_attr_destroy()` after it's no longer needed.
    /// Returns `NULL` in the unlikely case that the library cannot run on the current machine (e.g. the library has been compiled for SSE-capable x86 CPU and run on VIA C3 CPU).
    pub fn liq_attr_create() -> *mut liq_attr;
    pub fn liq_attr_copy(orig: &liq_attr) -> *mut liq_attr;
    pub fn liq_attr_destroy(attr: &mut liq_attr);

    /// Specifies maximum number of colors to use.
    ///
    /// The default is 256. Instead of setting a fixed limit it's better to use `liq_set_quality()`.
    /// The first argument is attributes object from `liq_attr_create()`.
    /// Returns `LIQ_VALUE_OUT_OF_RANGE` if number of colors is outside the range 2-256.
    pub fn liq_set_max_colors(attr: &mut liq_attr, colors: c_int) -> liq_error;
    pub fn liq_get_max_colors(attr: &liq_attr) -> c_int;
    pub fn liq_set_speed(attr: &mut liq_attr, speed: c_int) -> liq_error;
    pub fn liq_get_speed(attr: &liq_attr) -> c_int;
    pub fn liq_set_min_posterization(attr: &mut liq_attr, bits: c_int) -> liq_error;
    pub fn liq_get_min_posterization(attr: &liq_attr) -> c_int;
    /// Quality is in range `0` (worst) to `100` (best) and values are analoguous to JPEG quality (i.e. `80` is usually good enough).
    ///
    /// Quantization will attempt to use the lowest number of colors needed to achieve `maximum` quality. `maximum` value of `100` is the default and means conversion as good as possible.
    /// If it's not possible to convert the image with at least `minimum` quality (i.e. 256 colors is not enough to meet the minimum quality), then `liq_image_quantize()` will fail. The default minumum is `0` (proceeds regardless of quality).
    /// Quality measures how well the generated palette fits image given to `liq_image_quantize()`. If a different image is remapped with `liq_write_remapped_image()` then actual quality may be different.
    /// Regardless of the quality settings the number of colors won't exceed the maximum (see `liq_set_max_colors()`).
    /// The first argument is attributes object from `liq_attr_create()`.
    ///
    /// Returns `LIQ_VALUE_OUT_OF_RANGE` if target is lower than minimum or any of them is outside the 0-100 range.
    /// Returns `LIQ_INVALID_POINTER` if `attr` appears to be invalid.
    pub fn liq_set_quality(attr: &mut liq_attr, minimum: c_int, maximum: c_int) -> liq_error;
    pub fn liq_get_min_quality(attr: &liq_attr) -> c_int;
    pub fn liq_get_max_quality(attr: &liq_attr) -> c_int;
    pub fn liq_set_last_index_transparent(attr: &mut liq_attr, is_last: c_int);

    pub fn liq_image_create_rgba_rows(attr: &liq_attr, rows: *const *const u8, width: c_int, height: c_int, gamma: f64) -> *mut liq_image;
    /// Creates an object that represents the image pixels to be used for quantization and remapping.
    ///
    /// The pixel array must be contiguous run of RGBA pixels (alpha is the last component, 0 = transparent, 255 = opaque).
    ///
    /// The first argument is attributes object from `liq_attr_create()`. The same `attr` object should be used for the entire process, from creation of images to quantization.
    ///
    /// The `pixels` array must not be modified or freed until this object is freed with `liq_image_destroy()`. See also `liq_image_set_memory_ownership()`.
    ///
    /// `width` and `height` are dimensions in pixels. An image 10x10 pixel large will need a 400-byte array.
    ///
    /// `gamma` can be `0` for images with the typical 1/2.2 [gamma](https://en.wikipedia.org/wiki/Gamma_correction).
    /// Otherwise `gamma` must be > 0 and < 1, e.g. `0.45455` (1/2.2) or `0.55555` (1/1.8). Generated palette will use the same gamma unless `liq_set_output_gamma()` is used. If `liq_set_output_gamma` is not used, then it only affects whether brighter or darker areas of the image will get more palette colors allocated.
    ///
    /// Returns `NULL` on failure, e.g. if `pixels` is `NULL` or `width`/`height` is <= 0.
    pub fn liq_image_create_rgba(attr: &liq_attr, bitmap: *const u8, width: c_int, height: c_int, gamma: f64) -> *mut liq_image;
    /// unsafe: It will crash if the owned memory wasn't allocated using `libc::malloc()` (or whatever allocator C side is using)
    pub fn liq_image_set_memory_ownership(image: &liq_image, own: liq_ownership) -> liq_error;

    pub fn liq_set_log_callback(arg1: &mut liq_attr, arg2: liq_log_callback_function, user_info: *mut c_void);
    pub fn liq_set_log_flush_callback(arg1: &mut liq_attr, arg2: liq_log_flush_callback_function, user_info: *mut c_void);
    pub fn liq_attr_set_progress_callback(arg1: &mut liq_attr, arg2: liq_progress_callback_function, user_info: *mut c_void);
    pub fn liq_result_set_progress_callback(arg1: &mut liq_result, arg2: liq_progress_callback_function, user_info: *mut c_void);
    pub fn liq_image_create_custom(attr: &liq_attr, row_callback: liq_image_get_rgba_row_callback, user_info: *mut c_void, width: c_int, height: c_int, gamma: f64) -> *mut liq_image;
    /// Remap assuming the image will be always presented exactly on top of this background.
    ///
    /// Takes ownership of the background image (i.e. do NOT use it afterwards, do NOT call liq_image_destroy on it).
    ///
    /// The background must have the same size. The foreground image must have a transparent color.
    pub fn liq_image_set_background(img: &mut liq_image, background: *mut liq_image) -> liq_error;
    pub fn liq_image_set_importance_map(img: &mut liq_image, buffer: *mut u8, buffer_size: usize, own: liq_ownership) -> liq_error;
    pub fn liq_image_add_fixed_color(img: &mut liq_image, color: liq_color) -> liq_error;
    pub fn liq_image_get_width(img: &liq_image) -> c_int;
    pub fn liq_image_get_height(img: &liq_image) -> c_int;
    pub fn liq_image_destroy(img: &mut liq_image);

    pub fn liq_histogram_create(attr: &liq_attr) -> *mut liq_histogram;
    pub fn liq_histogram_add_image(hist: &mut liq_histogram, attr: &liq_attr, image: &mut liq_image) -> liq_error;
    pub fn liq_histogram_add_colors(hist: &mut liq_histogram, attr: &liq_attr, entries: *const liq_histogram_entry, num_entries: c_int, gamma: f64) -> liq_error;
    pub fn liq_histogram_add_fixed_color(hist: &mut liq_histogram, color: liq_color) -> liq_error;
    pub fn liq_histogram_destroy(hist: &mut liq_histogram);

    /// Performs quantization (palette generation) based on settings in `attr` (from `liq_attr_create()`) and pixels of the image.
    ///
    /// Returns `LIQ_OK` if quantization succeeds and sets `liq_result` pointer in `out_result`. The last argument is used for receiving the `result` object:
    ///
    ///     liq_result *result;
    ///     if (LIQ_OK == liq_image_quantize(img, attr, &result)) { // Note &result
    ///         // result pointer is valid here
    ///     }
    ///
    /// Returns `LIQ_QUALITY_TOO_LOW` if quantization fails due to limit set in `liq_set_quality()`.
    ///
    /// See `liq_write_remapped_image()`.
    ///
    /// If you want to generate one palette for multiple images at once, see `liq_histogram_create()`.
    pub fn liq_quantize_image(options: &liq_attr, input_image: &liq_image) -> *mut liq_result;
    pub fn liq_histogram_quantize(input_hist: &liq_histogram, options: &liq_attr, result_output: &mut *mut liq_result) -> liq_error;
    pub fn liq_image_quantize(input_image: &liq_image, options: &liq_attr, result_output: &mut *mut liq_result) -> liq_error;
    /// Enables/disables dithering in `liq_write_remapped_image()`.
    ///
    /// Dithering level must be between `0` and `1` (inclusive). Dithering level `0` enables fast non-dithered remapping. Otherwise a variation of Floyd-Steinberg error diffusion is used.
    ///
    /// Precision of the dithering algorithm depends on the speed setting, see `liq_set_speed()`.
    ///
    /// Returns `LIQ_VALUE_OUT_OF_RANGE` if the dithering level is outside the 0-1 range.
    pub fn liq_set_dithering_level(res: &mut liq_result, dither_level: f32) -> liq_error;
    pub fn liq_set_output_gamma(res: &mut liq_result, gamma: f64) -> liq_error;
    pub fn liq_get_output_gamma(result: &liq_result) -> f64;
    /// Returns pointer to palette optimized for image that has been quantized or remapped
    /// (final refinements are applied to the palette during remapping).
    ///
    /// It's valid to call this method before remapping, if you don't plan to remap any images or want to use same palette for multiple images.
    ///
    /// `liq_palette->count` contains number of colors (up to 256), `liq_palette->entries[n]` contains RGBA value for nth palette color.
    pub fn liq_get_palette<'a>(result: &'a mut liq_result) -> &'a liq_palette;
    /// Remaps the image to palette and writes its pixels to the given buffer, 1 pixel per byte.
    ///
    /// The buffer must be large enough to fit the entire image, i.e. width×height bytes large. For safety, pass the size of the buffer as `buffer_size`.
    ///
    /// For best performance call `liq_get_palette()` *after* this function, as palette is improved during remapping (except when `liq_histogram_quantize()` is used).
    ///
    /// Returns `LIQ_BUFFER_TOO_SMALL` if given size of the buffer is not enough to fit the entire image.
    ///
    ///     int buffer_size = width*height;
    ///     char *buffer = malloc(buffer_size);
    ///     if (LIQ_OK == liq_write_remapped_image(result, input_image, buffer, buffer_size)) {
    ///         liq_palette *pal = liq_get_palette(result);
    ///         // save image
    ///     }
    ///
    /// See `liq_get_palette()`.
    ///
    /// The buffer is assumed to be contiguous, with rows ordered from top to bottom, and no gaps between rows. If you need to write rows with padding or upside-down order, then use `liq_write_remapped_image_rows()`.
    ///
    /// Please note that it only writes raw uncompressed pixels to memory. It does not perform any PNG compression. If you'd like to create a PNG file then you need to pass the raw pixel data to another library, e.g. libpng or lodepng. See `rwpng.c` in `pngquant` project for an example how to do that.
    pub fn liq_write_remapped_image(result: &mut liq_result, input_image: &mut liq_image, buffer: *mut u8, buffer_size: usize) -> liq_error;
    pub fn liq_write_remapped_image_rows(result: &mut liq_result, input_image: &mut liq_image, row_pointers: *const *mut u8) -> liq_error;
    pub fn liq_get_quantization_error(result: &liq_result) -> f64;
    pub fn liq_get_quantization_quality(result: &liq_result) -> c_int;

    pub fn liq_result_destroy(res: &mut liq_result);
    pub fn liq_get_remapping_error(result: &liq_result) -> f64;
    pub fn liq_get_remapping_quality(result: &liq_result) -> c_int;
    pub fn liq_version() -> c_int;
}

#[test]
fn links_and_runs() {
    use std::ptr;
    unsafe {
        assert!(liq_version() >= 20901);
        let attr = liq_attr_create();
        assert!(!attr.is_null());
        let hist = liq_histogram_create(&*attr);
        assert!(!hist.is_null());
        let mut res = ptr::null_mut();
        liq_histogram_quantize(&*hist, &*attr, &mut res);
        assert!(res.is_null());
        liq_histogram_destroy(&mut *hist);
        liq_attr_destroy(&mut *attr);
    }
}
