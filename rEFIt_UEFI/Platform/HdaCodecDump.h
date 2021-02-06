/*
 * File: HdaCodecDump.h
 *
 * Copyright (c) 2018 John Davis
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _EFI_HDA_CODEC_DUMP_H_
#define _EFI_HDA_CODEC_DUMP_H_

//extern "C" {
//// Common UEFI includes and library classes.
//}

#define HDA_MAX_CONNS		32
#define HDA_MAX_NAMELEN		32

typedef struct {
	UINT8 NodeId;
    UINT8 Type;
    UINT32 Capabilities;
    UINT8 DefaultUnSol;
    UINT32 ConnectionListLength;
    UINT8 ConnectionSelect;
    UINT16 Connections[HDA_MAX_CONNS];
    UINT32 SupportedPowerStates;
    UINT32 DefaultPowerState;
    BOOLEAN AmpOverride;
    UINT32 AmpInCapabilities;
    UINT32 AmpOutCapabilities;
	UINT8 AmpInLeftDefaultGainMute[HDA_MAX_CONNS];
	UINT8 AmpInRightDefaultGainMute[HDA_MAX_CONNS];
    UINT8 AmpOutLeftDefaultGainMute;
    UINT8 AmpOutRightDefaultGainMute;
    UINT32 SupportedPcmRates;
    UINT32 SupportedFormats;
    UINT16 DefaultConvFormat;
    UINT8 DefaultConvStreamChannel;
    UINT8 DefaultConvChannelCount;
    UINT32 PinCapabilities;
    UINT8 DefaultEapd;
    UINT8 DefaultPinControl;
    UINT32 DefaultConfiguration;
    UINT32 VolumeCapabilities;
    UINT8 DefaultVolume;
} HdaWidget;

typedef struct {
	UINT8 Header[4];
	CHAR8 Name[HDA_MAX_NAMELEN];
	UINT8 CodecAddress;
	UINT8 AudioFuncId;
	UINT8 Unsol;
	UINT32 VendorId;
	UINT32 RevisionId;
	UINT32 Rates;
	UINT32 Formats;
	UINT32 AmpInCaps;
	UINT32 AmpOutCaps;
	UINT32 WidgetCount;
} HdaCodec;

EFI_STATUS SaveHdaDumpTxt();
EFI_STATUS SaveHdaDumpBin();

#endif
