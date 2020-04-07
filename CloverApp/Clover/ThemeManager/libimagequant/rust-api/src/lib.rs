//! https://pngquant.org/lib/
//!
//! Converts RGBA images to 8-bit with alpha channel.
//!
//! This is based on imagequant library, which generates very high quality images.
//!
//! See `examples/` directory for example code.
#![doc(html_logo_url = "https://pngquant.org/pngquant-logo.png")]
#![warn(missing_docs)]

extern crate imagequant_sys as ffi;

pub use crate::ffi::liq_error;
pub use crate::ffi::liq_error::*;
use std::fmt;
use std::marker;
use std::mem;
use std::os::raw::c_int;
use std::ptr;

/// 8-bit RGBA. This is the only color format used by the library.
pub type Color = ffi::liq_color;

/// Number of pixels in a given color
///
/// Used if you're building histogram manually. Otherwise see `add_image()`
pub type HistogramEntry = ffi::liq_histogram_entry;

/// Settings for the conversion proces. Start here.
pub struct Attributes {
    handle: *mut ffi::liq_attr,
}

/// Describes image dimensions for the library.
pub struct Image<'a> {
    handle: *mut ffi::liq_image,
    /// Holds row pointers for images with stride
    _marker: marker::PhantomData<&'a [u8]>,
}

/// Palette inside.
pub struct QuantizationResult {
    handle: *mut ffi::liq_result,
}

/// Generate one shared palette for multiple images.
pub struct Histogram<'a> {
    attr: &'a Attributes,
    handle: *mut ffi::liq_histogram,
}

impl Drop for Attributes {
    fn drop(&mut self) {
        unsafe {
            if !self.handle.is_null() {
                ffi::liq_attr_destroy(&mut *self.handle);
            }
        }
    }
}

impl<'a> Drop for Image<'a> {
    fn drop(&mut self) {
        unsafe {
            ffi::liq_image_destroy(&mut *self.handle);
        }
    }
}

impl Drop for QuantizationResult {
    fn drop(&mut self) {
        unsafe {
            ffi::liq_result_destroy(&mut *self.handle);
        }
    }
}

impl<'a> Drop for Histogram<'a> {
    fn drop(&mut self) {
        unsafe {
            ffi::liq_histogram_destroy(&mut *self.handle);
        }
    }
}

impl Clone for Attributes {
    fn clone(&self) -> Attributes {
        unsafe { Attributes { handle: ffi::liq_attr_copy(&*self.handle) } }
    }
}

impl Default for Attributes {
    fn default() -> Attributes {
        Attributes::new()
    }
}

impl Attributes {
    /// New handle for library configuration
    ///
    /// See also `new_image()`
    pub fn new() -> Self {
        let handle = unsafe { ffi::liq_attr_create() };
        assert!(!handle.is_null(), "SSE-capable CPU is required for this build.");
        Attributes { handle }
    }

    /// It's better to use `set_quality()`
    pub fn set_max_colors(&mut self, value: i32) -> liq_error {
        unsafe { ffi::liq_set_max_colors(&mut *self.handle, value) }
    }

    /// Number of least significant bits to ignore.
    ///
    /// Useful for generating palettes for VGA, 15-bit textures, or other retro platforms.
    pub fn set_min_posterization(&mut self, value: i32) -> liq_error {
        unsafe { ffi::liq_set_min_posterization(&mut *self.handle, value) }
    }

    /// Returns number of bits of precision truncated
    pub fn min_posterization(&mut self) -> i32 {
        unsafe { ffi::liq_get_min_posterization(&*self.handle) }
    }

    /// Range 0-100, roughly like JPEG.
    ///
    /// If minimum quality can't be met, quantization will fail.
    ///
    /// Default is min 0, max 100.
    pub fn set_quality(&mut self, min: u32, max: u32) -> liq_error {
        unsafe { ffi::liq_set_quality(&mut *self.handle, min as c_int, max as c_int) }
    }

    /// Reads values set with `set_quality`
    pub fn quality(&mut self) -> (u32, u32) {
        unsafe {
            (ffi::liq_get_min_quality(&*self.handle) as u32,
             ffi::liq_get_max_quality(&*self.handle) as u32)
        }
    }

    /// 1-10.
    ///
    /// Faster speeds generate images of lower quality, but may be useful
    /// for real-time generation of images.
    pub fn set_speed(&mut self, value: i32) -> liq_error {
        unsafe { ffi::liq_set_speed(&mut *self.handle, value) }
    }

    /// Move transparent color to the last entry in the palette
    ///
    /// This is less efficient for PNG, but required by some broken software
    pub fn set_last_index_transparent(&mut self, value: bool) {
        unsafe { ffi::liq_set_last_index_transparent(&mut *self.handle, value as c_int) }
    }

    /// Return currently set speed/quality trade-off setting
    pub fn speed(&mut self) -> i32 {
        unsafe { ffi::liq_get_speed(&*self.handle) }
    }

    /// Return max number of colors set
    pub fn max_colors(&mut self) -> i32 {
        unsafe { ffi::liq_get_max_colors(&*self.handle) }
    }

    /// Describe dimensions of a slice of RGBA pixels
    ///
    /// Use 0.0 for gamma if the image is sRGB (most images are).
    pub fn new_image<'a, RGBA: Copy>(&self, bitmap: &'a [RGBA], width: usize, height: usize, gamma: f64) -> Result<Image<'a>, liq_error> {
        Image::new(self, bitmap, width, height, gamma)
    }

    /// Stride is in pixels. Allows defining regions of larger images or images with padding without copying.
    pub fn new_image_stride<'a, RGBA: Copy>(&self, bitmap: &'a [RGBA], width: usize, height: usize, stride: usize, gamma: f64) -> Result<Image<'a>, liq_error> {
        Image::new_stride(self, bitmap, width, height, stride, gamma)
    }

    /// Create new histogram
    ///
    /// Use to make one palette suitable for many images
    pub fn new_histogram(&self) -> Histogram<'_> {
        Histogram::new(&self)
    }

    /// Generate palette for the image
    pub fn quantize(&mut self, image: &Image<'_>) -> Result<QuantizationResult, liq_error> {
        unsafe {
            let mut h = ptr::null_mut();
            match ffi::liq_image_quantize(&*image.handle, &*self.handle, &mut h) {
                liq_error::LIQ_OK if !h.is_null() => Ok(QuantizationResult { handle: h }),
                err => Err(err),
            }
        }
    }
}

/// Start here: creates new handle for library configuration
pub fn new() -> Attributes {
    Attributes::new()
}

impl<'a> Histogram<'a> {
    /// Creates histogram object that will be used to collect color statistics from multiple images.
    ///
    /// All options should be set on `attr` before the histogram object is created. Options changed later may not have effect.
    pub fn new(attr: &'a Attributes) -> Self {
        Histogram {
            attr,
            handle: unsafe { ffi::liq_histogram_create(&*attr.handle) },
        }
    }

    /// "Learns" colors from the image, which will be later used to generate the palette.
    ///
    /// Fixed colors added to the image are also added to the histogram. If total number of fixed colors exceeds 256, this function will fail with `LIQ_BUFFER_TOO_SMALL`.
    pub fn add_image(&mut self, image: &mut Image<'_>) -> liq_error {
        unsafe { ffi::liq_histogram_add_image(&mut *self.handle, &*self.attr.handle, &mut *image.handle) }
    }

    /// Alternative to `add_image()`. Intead of counting colors in an image, it directly takes an array of colors and their counts.
    ///
    /// This function is only useful if you already have a histogram of the image from another source.
    pub fn add_colors(&mut self, colors: &[HistogramEntry], gamma: f64) -> liq_error {
        unsafe {
            ffi::liq_histogram_add_colors(&mut *self.handle, &*self.attr.handle, colors.as_ptr(), colors.len() as c_int, gamma)
        }
    }

    /// Generate palette for all images/colors added to the histogram.
    ///
    /// Palette generated using this function won't be improved during remapping.
    /// If you're generating palette for only one image, it's better not to use the `Histogram`.
    pub fn quantize(&mut self) -> Result<QuantizationResult, liq_error> {
        unsafe {
            let mut h = ptr::null_mut();
            match ffi::liq_histogram_quantize(&*self.handle, &*self.attr.handle, &mut h) {
                liq_error::LIQ_OK if !h.is_null() => Ok(QuantizationResult { handle: h }),
                err => Err(err),
            }
        }
    }
}

/// Generate image row on the fly
///
/// `output_row` is an array `width` RGBA elements wide.
/// `y` is the row (0-indexed) to write to the `output_row`
/// `user_data` is the data given to `Image::new_unsafe_fn()`
pub type ConvertRowUnsafeFn<UserData> = unsafe extern "C" fn(output_row: *mut Color, y: c_int, width: c_int, user_data: *mut UserData);

impl<'bitmap> Image<'bitmap> {
    /// Describe dimensions of a slice of RGBA pixels.
    ///
    /// `bitmap` must be either `&[u8]` or a slice with one element per pixel (`&[RGBA]`).
    ///
    /// Use `0.` for gamma if the image is sRGB (most images are).
    #[inline]
    pub fn new<PixelType: Copy>(attr: &Attributes, bitmap: &'bitmap [PixelType], width: usize, height: usize, gamma: f64) -> Result<Self, liq_error> {
        Self::new_stride(attr, bitmap, width, height, width, gamma)
    }

    /// Generate rows on demand using a callback function.
    ///
    /// The callback function must be cheap (e.g. just byte-swap pixels).
    /// It will be called multiple times per row. May be called in any order from any thread.
    ///
    /// The user data must be compatible with a primitive pointer
    /// (i.e. not a slice, not a Trait object. `Box` it if you must).
    pub fn new_unsafe_fn<CustomData: Send + Sync + 'bitmap>(attr: &Attributes, convert_row_fn: ConvertRowUnsafeFn<CustomData>, user_data: *mut CustomData, width: usize, height: usize, gamma: f64) -> Result<Self, liq_error> {
        unsafe {
            match ffi::liq_image_create_custom(&*attr.handle, mem::transmute(convert_row_fn), user_data as *mut _, width as c_int, height as c_int, gamma) {
                handle if !handle.is_null() => Ok(Image { handle, _marker: marker::PhantomData }),
                _ => Err(LIQ_INVALID_POINTER),
            }
        }
    }

    /// Stride is in pixels. Allows defining regions of larger images or images with padding without copying.
    pub fn new_stride<PixelType: Copy>(attr: &Attributes, bitmap: &'bitmap [PixelType], width: usize, height: usize, stride: usize, gamma: f64) -> Result<Self, liq_error> {
        let bytes_per_pixel = mem::size_of::<PixelType>();
        match bytes_per_pixel {
            1 | 4 => {}
            _ => return Err(LIQ_UNSUPPORTED),
        }
        if bitmap.len() * bytes_per_pixel < (stride * height + width - stride) * 4 {
            eprintln!("Buffer length is {}×{} bytes, which is not enough for {}×{}×4 RGBA bytes", bitmap.len(), bytes_per_pixel, stride, height);
            return Err(LIQ_BUFFER_TOO_SMALL);
        }
        unsafe {
            let rows = Self::malloc_image_rows(bitmap, stride, height, bytes_per_pixel);
            match ffi::liq_image_create_rgba_rows(&*attr.handle, rows, width as c_int, height as c_int, gamma) {
                h if !h.is_null() && ffi::liq_image_set_memory_ownership(&*h, ffi::liq_ownership::LIQ_OWN_ROWS).is_ok() => {
                    Ok(Image {
                        handle: h,
                        _marker: marker::PhantomData,
                    })
                },
                _ => {
                    libc::free(rows as *mut _);
                    Err(LIQ_INVALID_POINTER)
                }
            }
        }
    }

    /// For arbitrary stride libimagequant requires rows. It's most convenient if they're allocated using libc,
    /// so they can be owned and freed automatically by the C library.
    unsafe fn malloc_image_rows<PixelType: Copy>(bitmap: &'bitmap [PixelType], stride: usize, height: usize, bytes_per_pixel: usize) -> *mut *const u8 {
        let mut byte_ptr = bitmap.as_ptr() as *const u8;
        let stride_bytes = (stride * bytes_per_pixel) as isize;
        let rows = libc::malloc(mem::size_of::<*const u8>() * height) as *mut *const u8;
        for y in 0..height as isize {
            *rows.offset(y) = byte_ptr;
            byte_ptr = byte_ptr.offset(stride_bytes);
        }
        rows
    }

    /// Width of the image in pixels
    pub fn width(&self) -> usize {
        unsafe { ffi::liq_image_get_width(&*self.handle) as usize }
    }

    /// Height of the image in pixels
    pub fn height(&self) -> usize {
        unsafe { ffi::liq_image_get_height(&*self.handle) as usize }
    }

    /// Reserves a color in the output palette created from this image. It behaves as if the given color was used in the image and was very important.
    ///
    /// RGB values of liq_color are assumed to have the same gamma as the image.
    ///
    /// It must be called before the image is quantized.
    ///
    /// Returns error if more than 256 colors are added. If image is quantized to fewer colors than the number of fixed colors added, then excess fixed colors will be ignored.
    pub fn add_fixed_color(&mut self, color: ffi::liq_color) -> liq_error {
        unsafe {
            ffi::liq_image_add_fixed_color(&mut *self.handle, color)
        }
    }

    /// Remap pixels assuming they will be displayed on this background.
    ///
    /// Pixels that match the background color will be made transparent if there's a fully transparent color available in the palette.
    ///
    /// The background image's pixels must outlive this image
    pub fn set_background<'own, 'bg: 'own>(&'own mut self, background: Image<'bg>) -> Result<(), liq_error> {
        unsafe {
            ffi::liq_image_set_background(&mut *self.handle, background.into_raw()).ok()
        }
    }

    /// Set which pixels are more important (and more likely to get a palette entry)
    ///
    /// The map must be `width`×`height` pixels large. Higher numbers = more important.
    pub fn set_importance_map(&mut self, map: &[u8]) -> Result<(), liq_error> {
        unsafe {
            ffi::liq_image_set_importance_map(&mut *self.handle, map.as_ptr() as *mut _, map.len(), ffi::liq_ownership::LIQ_COPY_PIXELS).ok()
        }
    }

    fn into_raw(mut self) -> *mut ffi::liq_image {
        let handle = self.handle;
        self.handle = ptr::null_mut();
        handle
    }
}

impl QuantizationResult {
    /// Set to 1.0 to get nice smooth image
    pub fn set_dithering_level(&mut self, value: f32) -> liq_error {
        unsafe { ffi::liq_set_dithering_level(&mut *self.handle, value) }
    }

    /// The default is sRGB gamma (~1/2.2)
    pub fn set_output_gamma(&mut self, value: f64) -> liq_error {
        unsafe { ffi::liq_set_output_gamma(&mut *self.handle, value) }
    }

    /// Approximate gamma correction value used for the output
    ///
    /// Colors are converted from input gamma to this gamma
    pub fn output_gamma(&mut self) -> f64 {
        unsafe { ffi::liq_get_output_gamma(&*self.handle) }
    }

    /// Number 0-100 guessing how nice the input image will look if remapped to this palette
    pub fn quantization_quality(&self) -> i32 {
        unsafe { ffi::liq_get_quantization_quality(&*self.handle) as i32 }
    }

    /// Approximate mean square error of the palette
    pub fn quantization_error(&self) -> Option<f64> {
        match unsafe { ffi::liq_get_quantization_error(&*self.handle) } {
            x if x < 0. => None,
            x => Some(x),
        }
    }

    /// Final palette
    ///
    /// It's slighly better if you get palette from the `remapped()` call instead
    pub fn palette(&mut self) -> Vec<Color> {
        unsafe {
            let pal = &*ffi::liq_get_palette(&mut *self.handle);
            pal.entries.iter().cloned().take(pal.count as usize).collect()
        }
    }

    /// Remap image
    ///
    /// Returns palette and 1-byte-per-pixel uncompresed bitmap
    pub fn remapped(&mut self, image: &mut Image<'_>) -> Result<(Vec<Color>, Vec<u8>), liq_error> {
        let len = image.width() * image.height();
        let mut buf = Vec::with_capacity(len);
        unsafe {
            buf.set_len(len); // Creates uninitialized buffer
            match ffi::liq_write_remapped_image(&mut *self.handle, &mut *image.handle, buf.as_mut_ptr(), buf.len()) {
                LIQ_OK => Ok((self.palette(), buf)),
                err => Err(err),
            }
        }
    }
}

impl fmt::Debug for QuantizationResult {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "QuantizationResult(q={})", self.quantization_quality())
    }
}

unsafe impl Send for Attributes {}
unsafe impl Send for QuantizationResult {}
unsafe impl<'bitmap> Send for Image<'bitmap> {}
unsafe impl<'a> Send for Histogram<'a> {}

#[test]
fn takes_rgba() {
    let liq = Attributes::new();

    #[allow(dead_code)]
    #[derive(Copy, Clone)]
    struct RGBA {r:u8, g:u8, b:u8, a:u8};
    let img = vec![RGBA {r:0, g:0, b:0, a:0}; 8];


    liq.new_image(&img, 1, 1, 0.0).unwrap();
    liq.new_image(&img, 4, 2, 0.0).unwrap();
    liq.new_image(&img, 8, 1, 0.0).unwrap();
    assert!(liq.new_image(&img, 9, 1, 0.0).is_err());
    assert!(liq.new_image(&img, 4, 3, 0.0).is_err());

    #[allow(dead_code)]
    #[derive(Copy, Clone)]
    struct RGB {r:u8, g:u8, b:u8};
    let badimg = vec![RGB {r:0, g:0, b:0}; 8];
    assert!(liq.new_image(&badimg, 1, 1, 0.0).is_err());
    assert!(liq.new_image(&badimg, 100, 100, 0.0).is_err());
}

#[test]
fn histogram() {
    let attr = Attributes::new();
    let mut hist = attr.new_histogram();

    let bitmap1 = vec![0u8; 4];
    let mut image1 = attr.new_image(&bitmap1[..], 1, 1, 0.0).unwrap();
    hist.add_image(&mut image1);

    let bitmap2 = vec![255u8; 4];
    let mut image2 = attr.new_image(&bitmap2[..], 1, 1, 0.0).unwrap();
    hist.add_image(&mut image2);

    hist.add_colors(&[HistogramEntry{
        color: Color::new(255,128,255,128),
        count: 10,
    }], 0.0);

    let mut res = hist.quantize().unwrap();
    let pal = res.palette();
    assert_eq!(3, pal.len());
}

#[test]
fn poke_it() {
    let width = 10usize;
    let height = 10usize;
    let mut fakebitmap = vec![255u8; 4*width*height];

    fakebitmap[0] = 0x55;
    fakebitmap[1] = 0x66;
    fakebitmap[2] = 0x77;

    // Configure the library
    let mut liq = Attributes::new();
    liq.set_speed(5);
    liq.set_quality(70, 99);
    liq.set_min_posterization(1);
    assert_eq!(1, liq.min_posterization());
    liq.set_min_posterization(0);

    // Describe the bitmap
    let ref mut img = liq.new_image(&fakebitmap[..], width, height, 0.0).unwrap();

    // The magic happens in quantize()
    let mut res = match liq.quantize(img) {
        Ok(res) => res,
        Err(err) => panic!("Quantization failed, because: {:?}", err),
    };

    // Enable dithering for subsequent remappings
    res.set_dithering_level(1.0);

    // You can reuse the result to generate several images with the same palette
    let (palette, pixels) = res.remapped(img).unwrap();

    assert_eq!(width * height, pixels.len());
    assert_eq!(100, res.quantization_quality());
    assert_eq!(Color { r: 255, g: 255, b: 255, a: 255 }, palette[0]);
    assert_eq!(Color { r: 0x55, g: 0x66, b: 0x77, a: 255 }, palette[1]);
}

#[test]
fn set_importance_map() {
    use crate::ffi::liq_color as RGBA;
    let mut liq = new();
    let bitmap = &[RGBA::new(255, 0, 0, 255), RGBA::new(0u8, 0, 255, 255)];
    let ref mut img = liq.new_image(&bitmap[..], 2, 1, 0.).unwrap();
    let map = &[255, 0];
    img.set_importance_map(map).unwrap();
    let mut res = liq.quantize(img).unwrap();
    let pal = res.palette();
    assert_eq!(1, pal.len());
    assert_eq!(bitmap[0], pal[0]);
}

#[test]
fn thread() {
    let liq = Attributes::new();
    std::thread::spawn(move || {
        let b = vec![0u8;4];
        liq.new_image(&b, 1, 1, 0.).unwrap();
    }).join().unwrap();
}

#[test]
fn callback_test() {
    let mut called = 0;
    let mut res = {
        let mut a = new();
        unsafe extern "C" fn get_row(output_row: *mut Color, y: c_int, width: c_int, user_data: *mut i32) {
            assert!(y >= 0 && y < 5);
            assert_eq!(123, width);
            for i in 0..width as isize {
                let n = i as u8;
                *output_row.offset(i as isize) = Color::new(n,n,n,n);
            }
            *user_data += 1;
        }
        let mut img = Image::new_unsafe_fn(&a, get_row, &mut called, 123, 5, 0.).unwrap();
        a.quantize(&mut img).unwrap()
    };
    assert!(called > 5 && called < 50);
    assert_eq!(123, res.palette().len());
}
