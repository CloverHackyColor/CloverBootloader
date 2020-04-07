package org.pngquant;

import org.pngquant.*;
import java.awt.image.*;

/**
 * PngQuant's representation of an Image constructed from BufferedImage.
 */
public class Image extends LiqObject {

    /**
     * Converts BufferedImage to internal representation (pixel data is copied).
     * It's best to use BufferedImage in RGB/RGBA format backed by DataBufferByte.
     * Throws if conversion fails.
     */
    public Image(BufferedImage image) throws PngQuantException {
        this(new PngQuant(), image);
    }

    public Image(PngQuant attr, BufferedImage image) throws PngQuantException {
        handle = handleFromImage(attr, image);

        if (handle == 0) {
            BufferedImage converted = new BufferedImage(image.getWidth(), image.getHeight(), BufferedImage.TYPE_4BYTE_ABGR);
            converted.getGraphics().drawImage(image, 0, 0, null);
            handle = handleFromImage(attr, converted);

            if (handle == 0) {
                throw new PngQuantException();
            }
        }
    }

    /**
     * Guarantees presence of the given color in the palette (subject to setMaxColors())
     * if this image is used for quantization.
     */
    public native boolean addFixedColor(int r, int g, int b, int a);
    public boolean addFixedColor(int r, int g, int b) {
        return addFixedColor(r, g, b, 255);
    }
    public native int getWidth();
    public native int getHeight();

    public void close() {
        if (handle != 0) {
            liq_image_destroy(handle);
            handle = 0;
        }
    }

    private static long handleFromImage(PngQuant attr, BufferedImage image) {
        // The JNI wrapper will accept non-premultiplied ABGR and BGR only.
        int type = image.getType();
        if (type != BufferedImage.TYPE_3BYTE_BGR &&
            type != BufferedImage.TYPE_4BYTE_ABGR &&
            type != BufferedImage.TYPE_4BYTE_ABGR_PRE) return 0;

        WritableRaster raster = image.getRaster();
        ColorModel color = image.getColorModel();
        if (type == BufferedImage.TYPE_4BYTE_ABGR_PRE) color.coerceData(raster, false);

        DataBuffer buffer = raster.getDataBuffer();
        if (buffer instanceof DataBufferByte) {
            byte[] imageData = ((DataBufferByte)buffer).getData();
            return liq_image_create(attr.handle, imageData,
                raster.getWidth(), raster.getHeight(), color.getNumComponents());
        }
        return 0;
    }

    private static native long liq_image_create(long attr, byte[] bitmap, int width, int height, int components);
    private static native void liq_image_destroy(long handle);
}
