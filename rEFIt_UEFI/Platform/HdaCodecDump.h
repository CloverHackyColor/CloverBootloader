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
	UINT8 NodeId = 0;
  UINT8 Type = 0;
  UINT32 Capabilities = 0;
  UINT8 DefaultUnSol = 0;
  UINT32 ConnectionListLength = 0;
  UINT8 ConnectionSelect = 0;
  UINT16 Connections[HDA_MAX_CONNS] = { 0 };
  UINT32 SupportedPowerStates = 0;
  UINT32 DefaultPowerState = 0;
  XBool AmpOverride = false;
  UINT32 AmpInCapabilities = 0;
  UINT32 AmpOutCapabilities = 0;
  UINT8 AmpInLeftDefaultGainMute[HDA_MAX_CONNS] = { 0 };
	UINT8 AmpInRightDefaultGainMute[HDA_MAX_CONNS] = { 0 };
  UINT8 AmpOutLeftDefaultGainMute = 0;
  UINT8 AmpOutRightDefaultGainMute = 0;
  UINT32 SupportedPcmRates = 0;
  UINT32 SupportedFormats = 0;
  UINT16 DefaultConvFormat = 0;
  UINT8 DefaultConvStreamChannel = 0;
  UINT8 DefaultConvChannelCount = 0;
  UINT32 PinCapabilities = 0;
  UINT8 DefaultEapd = 0;
  UINT8 DefaultPinControl = 0;
  UINT32 DefaultConfiguration = 0;
  UINT32 VolumeCapabilities = 0;
  UINT8 DefaultVolume = 0;
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
