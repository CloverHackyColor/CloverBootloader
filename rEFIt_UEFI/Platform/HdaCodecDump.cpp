/*
 * File: HdaCodecDump.c
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

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include <Efi.h>

#include "HdaCodecDump.h"
#include "StateGenerator.h"
#include "AmlGenerator.h"
#include "../Platform/Settings.h"
#include "../Settings/Self.h"

CONST CHAR8 *gWidgetNames[HDA_WIDGET_TYPE_VENDOR + 1] = {
	"Audio Output", "Audio Input", "Audio Mixer",
	"Audio Selector", "Pin Complex", "Power Widget",
	"Volume Knob Widget", "Beep Generator Widget",
	"Reserved", "Reserved", "Reserved", "Reserved",
	"Reserved", "Reserved", "Reserved",
	"Vendor Defined Widget" };
CONST CHAR8 *gPortConnectivities[4] = { "Jack", "None", "Fixed", "Int Jack" };
CONST CHAR8 *gDefaultDevices[HDA_CONFIG_DEFAULT_DEVICE_OTHER + 1] = {
	"Line Out", "Speaker", "HP Out", "CD", "SPDIF Out",
	"Digital Out", "Modem Line", "Modem Handset", "Line In", "Aux",
	"Mic", "Telephone", "SPDIF In", "Digital In", "Reserved", "Other" };
CONST CHAR8 *gSurfaces[4] = { "Ext", "Int", "Ext", "Other" };
CONST CHAR8 *gLocations[0xF + 1] = {
	"N/A", "Rear", "Front", "Left", "Right", "Top", "Bottom", "Special",
	"Special", "Special", "Reserved", "Reserved", "Reserved", "Reserved" };
CONST CHAR8 *gConnTypes[HDA_CONFIG_DEFAULT_CONN_OTHER + 1] = {
	"Unknown", "1/8", "1/4", "ATAPI", "RCA", "Optical", "Digital",
	"Analog", "Multi", "XLR", "RJ11", "Combo", "Other", "Other", "Other", "Other" };
CONST CHAR8 *gColors[HDA_CONFIG_DEFAULT_COLOR_OTHER + 1] = {
	"Unknown", "Black", "Grey", "Blue", "Green", "Red", "Orange",
	"Yellow", "Purple", "Pink", "Reserved", "Reserved", "Reserved",
	"Reserved", "White", "Other" };	

#define HDC_ID        { 'H','D','C','O' }

//#pragma clang diagnostic push
//#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"

#define HdaLog(format, ...)  MemLogf(FALSE, 0, format, ##__VA_ARGS__)

//#pragma clang diagnostic pop


CONST CHAR8  hdcID[4]       = HDC_ID;

extern EFI_AUDIO_IO_PROTOCOL	*AudioIo;
extern XStringW           		 OEMPath;

void
EFIAPI
HdaCodecDumpPrintRatesFormats(
    IN UINT32 Rates,
    IN UINT32 Formats) {
    // Print sample rates.
	HdaLog("    rates [0x%04hX]:", (UINT16)Rates);
    if (Rates & HDA_PARAMETER_SUPPORTED_PCM_SIZE_RATES_8KHZ)
        HdaLog(" 8000");
    if (Rates & HDA_PARAMETER_SUPPORTED_PCM_SIZE_RATES_11KHZ)
        HdaLog(" 11025");
    if (Rates & HDA_PARAMETER_SUPPORTED_PCM_SIZE_RATES_16KHZ)
        HdaLog(" 16000");
    if (Rates & HDA_PARAMETER_SUPPORTED_PCM_SIZE_RATES_22KHZ)
        HdaLog(" 22050");
    if (Rates & HDA_PARAMETER_SUPPORTED_PCM_SIZE_RATES_32KHZ)
        HdaLog(" 32000");
    if (Rates & HDA_PARAMETER_SUPPORTED_PCM_SIZE_RATES_44KHZ)
        HdaLog(" 44100");
    if (Rates & HDA_PARAMETER_SUPPORTED_PCM_SIZE_RATES_48KHZ)
        HdaLog(" 48000");
    if (Rates & HDA_PARAMETER_SUPPORTED_PCM_SIZE_RATES_88KHZ)
        HdaLog(" 88200");
    if (Rates & HDA_PARAMETER_SUPPORTED_PCM_SIZE_RATES_96KHZ)
        HdaLog(" 96000");
    if (Rates & HDA_PARAMETER_SUPPORTED_PCM_SIZE_RATES_176KHZ)
        HdaLog(" 176400");
    if (Rates & HDA_PARAMETER_SUPPORTED_PCM_SIZE_RATES_192KHZ)
        HdaLog(" 192000");
    if (Rates & HDA_PARAMETER_SUPPORTED_PCM_SIZE_RATES_384KHZ)
        HdaLog(" 384000");
    HdaLog("\n");

    // Print bits.
	HdaLog("    bits [0x%04hX]:", (UINT16)(Rates >> 16));
    if (Rates & HDA_PARAMETER_SUPPORTED_PCM_SIZE_RATES_8BIT)
        HdaLog(" 8");
    if (Rates & HDA_PARAMETER_SUPPORTED_PCM_SIZE_RATES_16BIT)
        HdaLog(" 16");
    if (Rates & HDA_PARAMETER_SUPPORTED_PCM_SIZE_RATES_20BIT)
        HdaLog(" 20");
    if (Rates & HDA_PARAMETER_SUPPORTED_PCM_SIZE_RATES_24BIT)
        HdaLog(" 24");
    if (Rates & HDA_PARAMETER_SUPPORTED_PCM_SIZE_RATES_32BIT)
        HdaLog(" 32");
    HdaLog("\n");

    // Print formats.
    HdaLog("    formats [0x%08X]:", Formats);
    if (Formats & HDA_PARAMETER_SUPPORTED_STREAM_FORMATS_PCM)
        HdaLog(" PCM");
    if (Formats & HDA_PARAMETER_SUPPORTED_STREAM_FORMATS_FLOAT32)
        HdaLog(" FLOAT32");
    if (Formats & HDA_PARAMETER_SUPPORTED_STREAM_FORMATS_AC3)
        HdaLog(" AC3");
    HdaLog("\n");
}

void
EFIAPI
HdaCodecDumpPrintAmpCaps(
    IN UINT32 AmpCaps) {
    if (AmpCaps) {
        HdaLog("ofs=0x%02hhX, nsteps=0x%02hhX, stepsize=%02hhX, mute=%u\n",
            HDA_PARAMETER_AMP_CAPS_OFFSET(AmpCaps), HDA_PARAMETER_AMP_CAPS_NUM_STEPS(AmpCaps),
            HDA_PARAMETER_AMP_CAPS_STEP_SIZE(AmpCaps), (AmpCaps & HDA_PARAMETER_AMP_CAPS_MUTE) != 0);
    } else
        HdaLog("N/A\n");
}

void
EFIAPI
HdaCodecDumpPrintWidgets(
	IN EFI_HDA_IO_PROTOCOL *HdaIo,
    //IN HDA_WIDGET *Widgets,
    IN HDA_WIDGET_DEV *Widgets,
    IN UINTN WidgetCount) {
    // Print each widget.
    for (UINTN w = 0; w < WidgetCount; w++) {    
        // Print header and capabilities.
		HdaLog("Node 0x%02hhX [%s] wcaps 0x%08X:", Widgets[w].NodeId,
            gWidgetNames[HDA_PARAMETER_WIDGET_CAPS_TYPE(Widgets[w].Capabilities)], Widgets[w].Capabilities);
        if (Widgets[w].Capabilities & HDA_PARAMETER_WIDGET_CAPS_STEREO)
            HdaLog(" Stereo");
        else
            HdaLog(" Mono");
        if (Widgets[w].Capabilities & HDA_PARAMETER_WIDGET_CAPS_DIGITAL)
            HdaLog(" Digital");
        if (Widgets[w].Capabilities & HDA_PARAMETER_WIDGET_CAPS_IN_AMP)
            HdaLog(" Amp-In");
        if (Widgets[w].Capabilities & HDA_PARAMETER_WIDGET_CAPS_OUT_AMP)
            HdaLog(" Amp-Out");
        if (Widgets[w].Capabilities & HDA_PARAMETER_WIDGET_CAPS_L_R_SWAP)
            HdaLog(" R/L");
        HdaLog("\n");

        // Print input amp info.
        if (Widgets[w].Capabilities & HDA_PARAMETER_WIDGET_CAPS_IN_AMP) {
            // Print caps.
            HdaLog("  Amp-In caps: ");
            HdaCodecDumpPrintAmpCaps(Widgets[w].AmpInCapabilities);

            // Print default values.
            HdaLog("  Amp-In vals:");
            for (UINT8 i = 0; i < HDA_PARAMETER_CONN_LIST_LENGTH_LEN(Widgets[w].ConnectionListLength); i++) {
                if (Widgets[w].Capabilities & HDA_PARAMETER_WIDGET_CAPS_STEREO)
                    HdaLog(" [0x%02hhX 0x%02hhX]", Widgets[w].AmpInLeftDefaultGainMute[i], Widgets[w].AmpInRightDefaultGainMute[i]);
                else
                    HdaLog(" [0x%02hhX]", Widgets[w].AmpInLeftDefaultGainMute[i]);
            }
            HdaLog("\n");
        }

        // Print output amp info.
        if (Widgets[w].Capabilities & HDA_PARAMETER_WIDGET_CAPS_OUT_AMP) {
            // Print caps.
            HdaLog("  Amp-Out caps: ");
            HdaCodecDumpPrintAmpCaps(Widgets[w].AmpOutCapabilities);

            // Print default values.
            HdaLog("  Amp-Out vals:");
            if (Widgets[w].Capabilities & HDA_PARAMETER_WIDGET_CAPS_STEREO)
                HdaLog(" [0x%02hhX 0x%02hhX]\n", Widgets[w].AmpOutLeftDefaultGainMute, Widgets[w].AmpOutRightDefaultGainMute);
            else
                HdaLog(" [0x%02hhX]\n", Widgets[w].AmpOutLeftDefaultGainMute);
        }

        // Print pin complexe info.
        if (HDA_PARAMETER_WIDGET_CAPS_TYPE(Widgets[w].Capabilities) == HDA_WIDGET_TYPE_PIN_COMPLEX) {
            // Print pin capabilities.
            HdaLog("  Pincap 0x%08X:", Widgets[w].PinCapabilities);
            if (Widgets[w].PinCapabilities & HDA_PARAMETER_PIN_CAPS_INPUT)
                HdaLog(" IN");
            if (Widgets[w].PinCapabilities & HDA_PARAMETER_PIN_CAPS_OUTPUT)
                HdaLog(" OUT");
            if (Widgets[w].PinCapabilities & HDA_PARAMETER_PIN_CAPS_HEADPHONE)
                HdaLog(" HP");
            if (Widgets[w].PinCapabilities & HDA_PARAMETER_PIN_CAPS_EAPD)
                HdaLog(" EAPD");
            if (Widgets[w].PinCapabilities & HDA_PARAMETER_PIN_CAPS_TRIGGER)
                HdaLog(" Trigger");
            if (Widgets[w].PinCapabilities & HDA_PARAMETER_PIN_CAPS_PRESENCE)
                HdaLog(" Detect");
            if (Widgets[w].PinCapabilities & HDA_PARAMETER_PIN_CAPS_HBR)
                HdaLog(" HBR");
            if (Widgets[w].PinCapabilities & HDA_PARAMETER_PIN_CAPS_HDMI)
                HdaLog(" HDMI");
            if (Widgets[w].PinCapabilities & HDA_PARAMETER_PIN_CAPS_DISPLAYPORT)
                HdaLog(" DP");
            HdaLog("\n");

            // Print EAPD info.
            if (Widgets[w].PinCapabilities & HDA_PARAMETER_PIN_CAPS_EAPD) {
                HdaLog("  EAPD 0x%hhX:", Widgets[w].DefaultEapd);
                if (Widgets[w].DefaultEapd & HDA_EAPD_BTL_ENABLE_BTL)
                    HdaLog(" BTL");
                if (Widgets[w].DefaultEapd & HDA_EAPD_BTL_ENABLE_EAPD)
                    HdaLog(" EAPD");
                if (Widgets[w].DefaultEapd & HDA_EAPD_BTL_ENABLE_L_R_SWAP)
                    HdaLog(" R/L");
                HdaLog("\n");
            }

            // Print pin default header.
            HdaLog("  Pin Default 0x%08X: [%s] %s at %s %s\n", Widgets[w].DefaultConfiguration,
                gPortConnectivities[HDA_VERB_GET_CONFIGURATION_DEFAULT_PORT_CONN(Widgets[w].DefaultConfiguration)],
                gDefaultDevices[HDA_VERB_GET_CONFIGURATION_DEFAULT_DEVICE(Widgets[w].DefaultConfiguration)],
                gSurfaces[HDA_VERB_GET_CONFIGURATION_DEFAULT_SURF(Widgets[w].DefaultConfiguration)],
                gLocations[HDA_VERB_GET_CONFIGURATION_DEFAULT_LOC(Widgets[w].DefaultConfiguration)]);

            // Print connection type and color.
            HdaLog("    Conn = %s, Color = %s\n",
                gConnTypes[HDA_VERB_GET_CONFIGURATION_DEFAULT_CONN_TYPE(Widgets[w].DefaultConfiguration)],
                gColors[HDA_VERB_GET_CONFIGURATION_DEFAULT_COLOR(Widgets[w].DefaultConfiguration)]);

            // Print default association and sequence.
            HdaLog("    DefAssociation = 0x%1hhX, Sequence = 0x%1hhX\n",
                HDA_VERB_GET_CONFIGURATION_DEFAULT_ASSOCIATION(Widgets[w].DefaultConfiguration),
                HDA_VERB_GET_CONFIGURATION_DEFAULT_SEQUENCE(Widgets[w].DefaultConfiguration));

            // Print default pin control.
            HdaLog("  Pin-ctls: 0x%02hhX:", Widgets[w].DefaultPinControl);
            if (Widgets[w].DefaultPinControl & HDA_PIN_WIDGET_CONTROL_VREF_EN)
                HdaLog(" VREF");
            if (Widgets[w].DefaultPinControl & HDA_PIN_WIDGET_CONTROL_IN_EN)
                HdaLog(" IN");
            if (Widgets[w].DefaultPinControl & HDA_PIN_WIDGET_CONTROL_OUT_EN)
                HdaLog(" OUT");
            if (Widgets[w].DefaultPinControl & HDA_PIN_WIDGET_CONTROL_HP_EN)
                HdaLog(" HP");
            HdaLog("\n");
        }
        
        // Print connections.
        if (Widgets[w].Capabilities & HDA_PARAMETER_WIDGET_CAPS_CONN_LIST) {
            HdaLog("  Connection: %hhu\n    ", HDA_PARAMETER_CONN_LIST_LENGTH_LEN(Widgets[w].ConnectionListLength));
            for (UINT8 c = 0; c < HDA_PARAMETER_CONN_LIST_LENGTH_LEN(Widgets[w].ConnectionListLength); c++) {
				HdaLog(" 0x%02hX", Widgets[w].Connections[c]);
                //if (c == Widgets[w].ConnectionSelect)
                //	HdaLog("*");
            }
            HdaLog("\n");
        }
    }
}

EFI_STATUS SaveHdaDumpTxt()
{
    EFI_STATUS Status = EFI_NOT_FOUND;
	AUDIO_IO_PRIVATE_DATA *AudioIoPrivateData;
   	HDA_CODEC_DEV *HdaCodecDev;
//    EFI_HDA_CODEC_INFO_PROTOCOL *HdaCodecInfo;
   	HDA_FUNC_GROUP *AudioFuncGroup;
   	EFI_HDA_IO_PROTOCOL *HdaIo;
	CHAR8 *MemLogStart;
	UINTN MemLogStartLen;

	// Print each codec found.
	for (UINTN i = 0; i < AudioList.size(); i++) {
		MemLogStartLen = GetMemLogLen();
		MemLogStart = GetMemLogBuffer() + MemLogStartLen;
		
	    HdaLog("HdaCodecDump Start\n");
	
		Status = gBS->HandleProtocol(AudioList[i].Handle, &gEfiAudioIoProtocolGuid, (void**)&AudioIo);
		
		if (EFI_ERROR(Status))
			continue;
	
		AudioIoPrivateData = AUDIO_IO_PRIVATE_DATA_FROM_THIS(AudioIo);

		if (!AudioIoPrivateData || !AudioIoPrivateData->HdaCodecDev || 
			!AudioIoPrivateData->HdaCodecDev->HdaIo || !AudioIoPrivateData->HdaCodecDev->AudioFuncGroup)
			continue;
		
		HdaCodecDev = AudioIoPrivateData->HdaCodecDev;
//		HdaCodecInfo = &HdaCodecDev->HdaCodecInfoData->HdaCodecInfo;
		AudioFuncGroup = HdaCodecDev->AudioFuncGroup;
		HdaIo = HdaCodecDev->HdaIo;
	
		// Get name.
		HdaLog("Codec: %ls\n", HdaCodecDev->Name);
		
		// Get address
		UINT8 CodecAddress;
		Status = HdaIo->GetAddress(HdaIo, &CodecAddress);
		HdaLog("Address: %hhu\n", CodecAddress);
		
		// Get AFG ID.
		HdaLog("AFG Function Id: 0x%1hhX (unsol %hhu)\n", AudioFuncGroup->NodeId, AudioFuncGroup->UnsolCapable);

		HdaLog("Vendor ID: 0x%08X\n", HdaCodecDev->VendorId);
		HdaLog("Revision ID: 0x%08X\n", HdaCodecDev->RevisionId);

		// Get supported rates/formats.
		if ((AudioFuncGroup->SupportedPcmRates != 0) || (AudioFuncGroup->SupportedFormats != 0)) {
			HdaLog("Default PCM:\n");
			HdaCodecDumpPrintRatesFormats(AudioFuncGroup->SupportedPcmRates, AudioFuncGroup->SupportedFormats);
		} else
			HdaLog("Default PCM: N/A\n");

		// Get default amp caps.
		HdaLog("Default Amp-In caps: ");
		HdaCodecDumpPrintAmpCaps(AudioFuncGroup->AmpInCapabilities);
		HdaLog("Default Amp-Out caps: ");
		HdaCodecDumpPrintAmpCaps(AudioFuncGroup->AmpOutCapabilities);
		
		// Get widgets.
		HDA_WIDGET_DEV *Widgets = AudioFuncGroup->Widgets;
		UINTN WidgetCount = AudioFuncGroup->WidgetsCount;
		HdaCodecDumpPrintWidgets(HdaIo, Widgets, WidgetCount);
		
		XStringW PathHdaDump = SWPrintf("misc\\HdaCodec#%llu (%ls).txt", i, HdaCodecDev->Name);

    Status = egSaveFile(&self.getCloverDir(), PathHdaDump.wc_str(), (void *)MemLogStart, GetMemLogLen() - MemLogStartLen);
    if (EFI_ERROR(Status)) {
      // Jief : don't write outside SelfDir
//      Status = egSaveFile(NULL, PathHdaDump.wc_str(), (void *)MemLogStart, GetMemLogLen() - MemLogStartLen);
		}
	}
  return Status;
}

EFI_STATUS SaveHdaDumpBin()
{
	EFI_STATUS Status = EFI_NOT_FOUND;
	AUDIO_IO_PRIVATE_DATA *AudioIoPrivateData;
   	HDA_CODEC_DEV *HdaCodecDev;
//    EFI_HDA_CODEC_INFO_PROTOCOL *HdaCodecInfo;
   	HDA_FUNC_GROUP *AudioFuncGroup;
   	EFI_HDA_IO_PROTOCOL *HdaIo;
	
	for (UINTN i = 0; i < AudioList.size(); i++) {
		HDA_WIDGET_DEV *Widgets;
		UINTN WidgetCount;
		UINT32 HdaCodecDataSize;
		HdaCodec HdaCodec;
		UINT8 CodecAddress;
		UINT8 *HdaCodecData;
		UINT8 *HdaCodecDataPtr;
		XStringW PathHdaDump;
		
		Status = gBS->HandleProtocol(AudioList[i].Handle, &gEfiAudioIoProtocolGuid, (void**)&AudioIo);
		
		if (EFI_ERROR(Status))
			continue;
	
		AudioIoPrivateData = AUDIO_IO_PRIVATE_DATA_FROM_THIS(AudioIo);

		if (!AudioIoPrivateData || !AudioIoPrivateData->HdaCodecDev || !AudioIoPrivateData->HdaCodecDev->HdaIo)
			continue;

		HdaCodecDev = AudioIoPrivateData->HdaCodecDev;
		if(!HdaCodecDev || !HdaCodecDev->AudioFuncGroup)
			continue;
		
//		HdaCodecInfo = &HdaCodecDev->HdaCodecInfoData->HdaCodecInfo;
		AudioFuncGroup = HdaCodecDev->AudioFuncGroup;
		HdaIo = HdaCodecDev->HdaIo;
		
		Widgets = AudioFuncGroup->Widgets;
		WidgetCount = AudioFuncGroup->WidgetsCount;
		HdaCodecDataSize = (UINT32)(sizeof(HdaCodec) + (WidgetCount * sizeof(HdaWidget)));
		
		// Copy codec data		
		ZeroMem(&HdaCodec, sizeof(HdaCodec));
		
		if(!EFI_ERROR(Status)) {
			for (UINT32 j = 0; j < HDA_MAX_NAMELEN; j++) {
				HdaCodec.Name[j] = (CHAR8)(HdaCodecDev->Name[j] & 0xFF);
				if (HdaCodecDev->Name[j] == 0x0000)
					break;
			}
		}
		
		Status = HdaIo->GetAddress(HdaIo, &CodecAddress);
		
		CopyMem(HdaCodec.Header, hdcID, 4);
		
		HdaCodec.CodecAddress = CodecAddress;
		HdaCodec.AudioFuncId = AudioFuncGroup->NodeId;
		HdaCodec.Unsol = AudioFuncGroup->UnsolCapable;
		HdaCodec.VendorId = HdaCodecDev->VendorId;
		HdaCodec.RevisionId = HdaCodecDev->RevisionId;
		HdaCodec.Rates = AudioFuncGroup->SupportedPcmRates;
		HdaCodec.Formats = AudioFuncGroup->SupportedFormats;
		HdaCodec.AmpInCaps = AudioFuncGroup->AmpInCapabilities;
		HdaCodec.AmpOutCaps = AudioFuncGroup->AmpOutCapabilities;
		HdaCodec.WidgetCount = AudioFuncGroup->WidgetsCount;
		
		// Allocate space for codec data
		HdaCodecData = (__typeof__(HdaCodecData))AllocateZeroPool(HdaCodecDataSize);
		HdaCodecDataPtr = HdaCodecData;
	
		if (!HdaCodecData)
			continue;
			
		CopyMem(HdaCodecDataPtr, &HdaCodec, sizeof(HdaCodec));
		HdaCodecDataPtr += sizeof(HdaCodec);

		for (UINT32 w = 0; w < WidgetCount; w++) {
			UINTN ConnectionListLength;
			HdaWidget HdaWidget;
			
			ConnectionListLength = HDA_PARAMETER_CONN_LIST_LENGTH_LEN(Widgets[w].ConnectionListLength);
			
			ZeroMem(&HdaWidget, sizeof(HdaWidget));
						
			HdaWidget.NodeId = Widgets[w].NodeId;
			HdaWidget.Type = Widgets[w].Type;
			HdaWidget.Capabilities = Widgets[w].Capabilities;
			HdaWidget.DefaultUnSol = Widgets[w].DefaultUnSol;
			HdaWidget.ConnectionListLength = Widgets[w].ConnectionListLength;
			HdaWidget.ConnectionSelect = 0;
			HdaWidget.SupportedPowerStates = Widgets[w].SupportedPowerStates;
			HdaWidget.DefaultPowerState = Widgets[w].DefaultPowerState;
			HdaWidget.AmpOverride = Widgets[w].AmpOverride;
			HdaWidget.AmpInCapabilities = Widgets[w].AmpInCapabilities;
			HdaWidget.AmpOutCapabilities = Widgets[w].AmpOutCapabilities;
			HdaWidget.AmpOutLeftDefaultGainMute = Widgets[w].AmpOutLeftDefaultGainMute;
			HdaWidget.AmpOutRightDefaultGainMute = Widgets[w].AmpOutRightDefaultGainMute;
			HdaWidget.SupportedPcmRates = Widgets[w].SupportedPcmRates;
			HdaWidget.SupportedFormats = Widgets[w].SupportedFormats;
			HdaWidget.DefaultConvFormat = Widgets[w].DefaultConvFormat;
			HdaWidget.DefaultConvStreamChannel = Widgets[w].DefaultConvStreamChannel;
			HdaWidget.DefaultConvChannelCount = Widgets[w].DefaultConvChannelCount;
			HdaWidget.PinCapabilities = Widgets[w].PinCapabilities;
			HdaWidget.DefaultEapd = Widgets[w].DefaultEapd;
			HdaWidget.DefaultPinControl = Widgets[w].DefaultPinControl;
			HdaWidget.DefaultConfiguration = Widgets[w].DefaultConfiguration;
			HdaWidget.VolumeCapabilities = Widgets[w].VolumeCapabilities;
			HdaWidget.DefaultVolume = Widgets[w].DefaultVolume;
	
			for (UINT32 c = 0; c < ConnectionListLength; c++) {
				if (Widgets[w].Capabilities & HDA_PARAMETER_WIDGET_CAPS_IN_AMP) {
					HdaWidget.AmpInLeftDefaultGainMute[c] = Widgets[w].AmpInLeftDefaultGainMute[c];
					
					if (Widgets[w].Capabilities & HDA_PARAMETER_WIDGET_CAPS_STEREO) {
						HdaWidget.AmpInRightDefaultGainMute[c] = Widgets[w].AmpInRightDefaultGainMute[c];
					}
				}
				
				if (Widgets[w].Capabilities & HDA_PARAMETER_WIDGET_CAPS_CONN_LIST) {
					HdaWidget.Connections[c] = Widgets[w].Connections[c];
				}
			}
			
			CopyMem(HdaCodecDataPtr, &HdaWidget, sizeof(HdaWidget));
			HdaCodecDataPtr += sizeof(HdaWidget);
		}
	
		PathHdaDump = SWPrintf("misc\\HdaCodec#%llu (%ls).bin", i, HdaCodecDev->Name);

		Status = egSaveFile(&self.getCloverDir(), PathHdaDump.wc_str(), HdaCodecData, HdaCodecDataSize);
    if (EFI_ERROR(Status)) {
      Status = egSaveFile(NULL, PathHdaDump.wc_str(), HdaCodecData, HdaCodecDataSize);
		}
	
		FreePool(HdaCodecData);
	}
	return EFI_SUCCESS;
}
