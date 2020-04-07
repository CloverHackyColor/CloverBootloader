package org.pngquant;

import org.pngquant.*;
import java.awt.image.*;

/**
 * Quantization result that holds palette and options for remapping.
 */
public class Result extends LiqObject {

    /**
     * Throws when quantization fails (e.g. due to failing to achieve minimum quality)
     */
    public Result(PngQuant pngquant, Image image) throws PngQuantException {
        handle = liq_quantize_image(pngquant.handle, image.handle);
        if (handle == 0) {
            throw new PngQuantException();
        }
    }

    /**
     * @return BufferedImage remapped to palette this Result has been created with or null on failure.
     */
    public BufferedImage getRemapped(Image orig_image) {
        byte[] pal = liq_get_palette(handle);
        IndexColorModel color = new IndexColorModel(8, pal.length/4, pal, 0, true);
        BufferedImage img = new BufferedImage(
            orig_image.getWidth(), orig_image.getHeight(),
            BufferedImage.TYPE_BYTE_INDEXED, color);

        byte[] data = get8bitDataFromImage(img);
        if (data == null) return null;

        if (!liq_write_remapped_image(handle, orig_image.handle, data)) return null;

        return img;
    }

    /**
     * Dithering strength. Floyd-Steinberg is always used and in
     * speed settings 1-5 high-quality adaptive dithering is used.
     * @see PngQuant.setSpeed()
     * @link http://pngquant.org/lib/#liq_set_dithering_level
     *
     * @param dither_level Dithering in range 0 (none) and 1 (full)
     */
    public native boolean setDitheringLevel(float dither_level);

    /**
     * The default is 0.45455 (1/2.2) which is PNG's approximation of sRGB.
     */
    public native boolean setGamma(double gamma);
    public native double getGamma();

    /**
     * Mean Square Error of remapping of image used to create this result.
     * @link http://pngquant.org/lib/#liq_get_quantization_error
     *
     * @return MSE or -1 if not available
     */
    public native double getMeanSquareError();

    /**
     * @link http://pngquant.org/lib/#liq_get_quantization_quality
     * @return Actually achieved quality in 0-100 range on scale compatible with PngQuant.setQuality()
     */
    public native int getQuality();

    public void close() {
        if (handle != 0) {
            liq_result_destroy(handle);
            handle = 0;
        }
    }

    private static byte[] get8bitDataFromImage(BufferedImage image) {
        if (image.getType() == BufferedImage.TYPE_BYTE_INDEXED) {
            DataBuffer buffer = image.getRaster().getDataBuffer();
            if (buffer instanceof DataBufferByte) {
                return ((DataBufferByte)buffer).getData();
            }
        }
        return null;
    }

    private static native byte[] liq_get_palette(long handle);
    private static native long liq_quantize_image(long attr, long image);
    private static native boolean liq_write_remapped_image(long handle, long image, byte[] buffer);
    private static native void liq_result_destroy(long handle);
}
