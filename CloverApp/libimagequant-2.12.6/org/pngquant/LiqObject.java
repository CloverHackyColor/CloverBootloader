package org.pngquant;

abstract class LiqObject {
    static {
        // libimagequant.jnilib or libimagequant.so must be in java.library.path
        System.loadLibrary("imagequant");
    }

    long handle;

    /**
     * Free memory used by the library. The object must not be used after this call.
     */
    abstract public void close();

    protected void finalize() throws Throwable {
        close();
    }
}
