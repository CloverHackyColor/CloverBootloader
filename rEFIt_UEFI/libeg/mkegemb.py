#!/usr/bin/env python

import sys, os.path
import Image

def enc_backbuffer(backbuffer):
    """Helper function for RLE compression, encodes a string of uncompressable data."""
    compdata = []
    if len(backbuffer) == 0:
        return compdata
    while len(backbuffer) > 128:
        compdata.append(127)
        compdata.extend(backbuffer[0:128])
        backbuffer = backbuffer[128:]
    compdata.append(len(backbuffer)-1)
    compdata.extend(backbuffer)
    return compdata

def compress_rle(rawdata):
    """Compresses the string using a RLE scheme."""
    compdata = []
    backbuffer = []
    
    while len(rawdata) >= 3:
        c = rawdata[0]
        if rawdata[1] == c and rawdata[2] == c:
            runlength = 3
            while runlength < 130 and len(rawdata) > runlength:
                if rawdata[runlength] == c:
                    runlength = runlength + 1
                else:
                    break
            compdata.extend(enc_backbuffer(backbuffer))
            backbuffer = []
            compdata.append(runlength + 125)
            compdata.append(c)
            rawdata = rawdata[runlength:]
        
        else:
            backbuffer.append(c)
            rawdata = rawdata[1:]
    
    backbuffer.extend(rawdata)
    compdata.extend(enc_backbuffer(backbuffer))
    
    return compdata

def encode_plane(rawdata, planename):
    """Encodes the data of a single plane."""
    
    rawlen = len(rawdata)
    compdata = compress_rle(rawdata)
    complen = len(compdata)
    print "  plane %s: compressed %d to %d (%.1f%%)" % (planename, rawlen, complen, float(complen) / float(rawlen) * 100.0)
    
    return compdata


### main loop

print "mkegemb 0.1, Copyright (c) 2006 Christoph Pfisterer"

planenames = ( "blue", "green", "red", "alpha", "grey" )

for filename in sys.argv[1:]:
    
    origimage = Image.open(filename)
    
    (width, height) = origimage.size
    mode = origimage.mode
    data = origimage.getdata()
    
    print "%s: %d x %d %s" % (filename, width, height, mode)
    
    (basename, extension) = os.path.splitext(filename)
    identname = basename.replace("-", "_")
    
    # extract image data from PIL object
    
    planes = [ [], [], [], [] ]
    
    if mode == "RGB":
        for pixcount in range(0, width*height):
            pixeldata = data[pixcount]
            planes[0].append(pixeldata[0])
            planes[1].append(pixeldata[1])
            planes[2].append(pixeldata[2])
    
    elif mode == "RGBA":
        for pixcount in range(0, width*height):
            pixeldata = data[pixcount]
            planes[0].append(pixeldata[0])
            planes[1].append(pixeldata[1])
            planes[2].append(pixeldata[2])
            planes[3].append(pixeldata[3])
    
    elif mode == "L":
        for pixcount in range(0, width*height):
            pixeldata = data[pixcount]
            planes[0].append(pixeldata)
            planes[1].append(pixeldata)
            planes[2].append(pixeldata)
    
    else:
        print " Error: Mode not supported!"
        continue
    
    # special treatment for fonts
    
    if basename[0:4] == "font":
        if planes[0] != planes[1] or planes[0] != planes[2]:
            print " Error: Font detected, but it is not greyscale!"
            continue
        print " font detected, encoding as alpha-only"
        # invert greyscale values for use as alpha
        planes[3] = map(lambda x: 255-x, planes[0])
        planes[0] = []
        planes[1] = []
        planes[2] = []
    
    # encode planes
    
    imagedata = []
    pixelformat = "EG_EIPIXELMODE"
    
    if len(planes[0]) > 0 and planes[0] == planes[1] and planes[0] == planes[2]:
        print " encoding as greyscale"
        imagedata.extend(encode_plane(planes[0], planenames[4]))
        pixelformat = pixelformat + "_GRAY"
    
    elif len(planes[0]) > 0:
        print " encoding as true color"
        imagedata.extend(encode_plane(planes[0], planenames[0]))
        imagedata.extend(encode_plane(planes[1], planenames[1]))
        imagedata.extend(encode_plane(planes[2], planenames[2]))
        pixelformat = pixelformat + "_COLOR"
    
    if len(planes[3]) > 0:
        if reduce(lambda x,y: x+y, planes[3]) == 0:
            print " skipping alpha plane because it is empty"
        else:
            imagedata.extend(encode_plane(planes[3], planenames[3]))
            pixelformat = pixelformat + "_ALPHA"
    
    # generate compilable header file
    
    output = "static const UINT8 egemb_%s_data[%d] = {\n" % (identname, len(imagedata))
    for i in range(0, len(imagedata)):
        output = output + " 0x%02x," % imagedata[i]
        if (i % 12) == 11:
            output = output + "\n"
    output = output + "\n};\n"
    output = output + "static EG_EMBEDDED_IMAGE egemb_%s = { %d, %d, %s, EG_EICOMPMODE_RLE, egemb_%s_data, %d };\n" % (identname, width, height, pixelformat, identname, len(imagedata))
    
    f = file("egemb_%s.h" % identname, "w")
    f.write(output)
    f.close()

print "Done!"
