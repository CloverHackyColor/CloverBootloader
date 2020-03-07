package org.pngquant;

import org.pngquant.*;
import java.awt.image.*;

/**
 * Starting point for the library. Holds configuration. Equivalent of liq_attr* in libimagequant.
 */
public class PngQuant extends LiqObject {

    /**
     * Single instance can be "recycled" for many remappings.
     */
    public PngQuant() {
        handle = liq_attr_create();
    }

    public PngQuant(PngQuant other) {
        handle = liq_attr_copy(other.handle);
    }

    /**
     * 1-shot quantization and remapping with current settings.
     * @see quantize()
     *
     * @return 8-bit indexed image or null on failure
     */
    public BufferedImage getRemapped(BufferedImage bufimg) {
        try {
            Image liqimg = new Image(this, bufimg);
            BufferedImage remapped = getRemapped(liqimg);
            liqimg.close();
            return remapped;
        } catch(PngQuantException e) {
            return null;
        }
    }

    /** @return remapped image or null on failure */
    public BufferedImage getRemapped(Image liqimg) {
        Result result = quantize(liqimg);
        if (result == null) return null;
        BufferedImage remapped = result.getRemapped(liqimg);
        result.close();
        return remapped;
    }

    /**
     * Performs quantization (chooses optimal palette for the given Image).
     * Returned object can be used to customize remapping and reused to remap other images to the same palette.
     * @link http://pngquant.org/lib/#liq_quantize_image
     *
     * @return null on failure
     */
    public Result quantize(Image img) {
        try {
            return new Result(this, img);
        } catch(PngQuantException e) {
            return null;
        }
    }

    /**
     * Remapped images won't use more than given number of colors (may use less if setQuality() is used)
     *
     * @link http://pngquant.org/lib/#liq_set_max_colors
     */
    public native boolean setMaxColors(int colors);

    /**
     * Equivalent of setQuality(target/2, target)
     *
     * @link http://pngquant.org/lib/#liq_set_quality
     */
    public native boolean setQuality(int target);

    /**
     * Quality in range 0-100. Quantization will fail if minimum quality cannot
     * be achieved with given number of colors.
     *
     * @link http://pngquant.org/lib/#liq_set_quality
     */
    public native boolean setQuality(int min, int max);

    /**
     * Speed in range 1 (slowest) and 11 (fastest). 3 is the optimum.
     * Higher speeds quantize quicker, but at cost of quality and sometimes larger images.
     *
     * @link http://pngquant.org/lib/#liq_set_speed
     */
    public native boolean setSpeed(int speed);

    /**
     * Reduces color precision by truncating number of least significant bits.
     * Slightly improves speed and helps generating images for low-fidelity displays/textures.
     *
     * @link http://pngquant.org/lib/#liq_set_min_posterization
     */
    public native boolean setMinPosterization(int bits);

    public void close() {
        if (handle != 0) {
            liq_attr_destroy(handle);
            handle = 0;
        }
    }

    private static native long liq_attr_create();
    private static native long liq_attr_copy(long orig);
    private static native void liq_attr_destroy(long handle);
}
