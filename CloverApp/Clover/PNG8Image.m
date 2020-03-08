//
//  PNG8Image.m
//  Clover
//
//  Created by vector sigma on 07/03/2020.
//  Copyright © 2020 CloverHackyColor. All rights reserved.
//

#import "PNG8Image.h"

@implementation PNG8Image
- (nullable NSData *)png8ImageDataAtPath:(NSString *_Nonnull)imagePath
                                   error:(NSError *_Nullable*_Nullable)errorPtr {
  NSString *domain = @"org.slice.Clover.PNG8Image.Error";
  
  // Load PNG file and decode it as raw RGBA pixels
  // This uses lodepng library for PNG reading (not part of libimagequant)
  const char *input_png_file_path = [imagePath UTF8String];
  unsigned int width, height;
  unsigned char *raw_rgba_pixels;
  unsigned int status = lodepng_decode32_file(&raw_rgba_pixels, &width, &height, input_png_file_path);
  if (status) {
    NSString *desc = [NSString stringWithFormat:@"Can't load %s: %s\n",
                      input_png_file_path,
                      lodepng_error_text(status)];
    NSDictionary *userInfo = @{ NSLocalizedDescriptionKey : desc };
    *errorPtr  = [NSError errorWithDomain:domain
                                     code:1
                                 userInfo:userInfo];
    
    return nil;
  }
  
  // Use libimagequant to make a palette for the RGBA pixels
  
  liq_attr *handle = liq_attr_create();
  liq_image *input_image = liq_image_create_rgba(handle, raw_rgba_pixels, width, height, 0);
  // You could set more options here, like liq_set_quality
  liq_result *quantization_result;
  if (liq_image_quantize(input_image, handle, &quantization_result) != LIQ_OK) {
    NSString *desc = @"Quantization failed\n";
    NSDictionary *userInfo = @{ NSLocalizedDescriptionKey : desc };
    *errorPtr  = [NSError errorWithDomain:domain
                                     code:2
                                 userInfo:userInfo];
    return nil;
  }
  
  // Use libimagequant to make new image pixels from the palette
  
  size_t pixels_size = width * height;
  unsigned char *raw_8bit_pixels = malloc(pixels_size);
  liq_set_dithering_level(quantization_result, 1.0);
  
  liq_write_remapped_image(quantization_result, input_image, raw_8bit_pixels, pixels_size);
  const liq_palette *palette = liq_get_palette(quantization_result);
  
  // Save converted pixels as a PNG file
  // This uses lodepng library for PNG writing (not part of libimagequant)
  
  LodePNGState state;
  lodepng_state_init(&state);
  state.info_raw.colortype = LCT_PALETTE;
  state.info_raw.bitdepth = 8;
  state.info_png.color.colortype = LCT_PALETTE;
  state.info_png.color.bitdepth = 8;
  
  for(int i=0; i < palette->count; i++) {
    lodepng_palette_add(&state.info_png.color, palette->entries[i].r, palette->entries[i].g, palette->entries[i].b, palette->entries[i].a);
    lodepng_palette_add(&state.info_raw, palette->entries[i].r, palette->entries[i].g, palette->entries[i].b, palette->entries[i].a);
  }
  
  unsigned char *output_file_data;
  size_t output_file_size;
  unsigned int out_status = lodepng_encode(&output_file_data, &output_file_size, raw_8bit_pixels, width, height, &state);
  if (out_status) {
    NSString *desc = [NSString stringWithFormat:@"Can't encode image: %s\n",
                      lodepng_error_text(out_status)];
    NSDictionary *userInfo = @{ NSLocalizedDescriptionKey : desc };
    *errorPtr  = [NSError errorWithDomain:domain
                                     code:3
                                 userInfo:userInfo];
    return nil;
  }
  
  NSData *data = [NSData dataWithBytes: output_file_data length: output_file_size];
  NSImage *convertedImage = [[NSImage alloc] initWithData:data];
  if (convertedImage == nil) {
    NSString *desc = @"Can't convert data to NSImage\n";
    NSDictionary *userInfo = @{ NSLocalizedDescriptionKey : desc };
    *errorPtr  = [NSError errorWithDomain:domain
                                     code:4
                                 userInfo:userInfo];
    return nil;
  }
  
  
  liq_result_destroy(quantization_result); // Must be freed only after you're done using the palette
  liq_image_destroy(input_image);
  liq_attr_destroy(handle);
  
  free(raw_8bit_pixels);
  lodepng_state_cleanup(&state);
  
  return data;
}
@end
