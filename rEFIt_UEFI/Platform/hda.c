/*
 * HDA injector, part of the Chameleon Boot Loader Project
 *
 * Ported and adapted by Fabio (ErmaC), October 2016.
 *
 * HDA injector is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HDA injector is distributed in the hope that it will be useful, but
 * WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Platform.h"
#include "hda.h"

#ifndef DEBUG_HDA
#ifndef DEBUG_ALL
#define DEBUG_HDA 1
#else
#define DEBUG_HDA DEBUG_ALL
#endif
#endif

#if DEBUG_HDA == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_HDA, __VA_ARGS__)
#endif

// HDA layout-id device injection by dmazar

#define HDA_VMIN	0x02 // Minor, Major Version
#define HDA_GCTL	0x08 // Global Control Register
#define HDA_ICO		0x60 // Immediate Command Output Interface
#define HDA_IRI		0x64 // Immediate Response Input Interface
#define HDA_ICS		0x68 // Immediate Command Status

/* Structures */

static hda_controller_devices know_hda_controller[] = {
    //8086  Intel Corporation
    { HDA_INTEL_OAK,	"Oaktrail"		/*, 0, 0 */ },
    { HDA_INTEL_BAY,	"BayTrail"		/*, 0, 0 */ },
    { HDA_INTEL_HSW1,	"Haswell"		/*, 0, 0 */ },
    { HDA_INTEL_HSW2,	"Haswell"		/*, 0, 0 */ },
    { HDA_INTEL_HSW3,	"Haswell"		/*, 0, 0 */ },
    { HDA_INTEL_BDW,	"Broadwell"		/*, 0, 0 */ },
    { HDA_INTEL_CPT,	"Cougar Point"		/*, 0, 0 */ },
    { HDA_INTEL_PATSBURG,	"Patsburg"		/*, 0, 0 */ },
    { HDA_INTEL_PPT1,	"Panther Point"		/*, 0, 0 */ },
    { HDA_INTEL_BRASWELL,	"Braswell"		/*, 0, 0 */ },
    { HDA_INTEL_82801F,	"82801F"		/*, 0, 0 */ },
    { HDA_INTEL_63XXESB,	"631x/632xESB"		/*, 0, 0 */ },
    { HDA_INTEL_82801G,	"82801G"		/*, 0, 0 */ },
    { HDA_INTEL_82801H,	"82801H"		/*, 0, 0 */ },
    { HDA_INTEL_82801I,	"82801I"		/*, 0, 0 */ },
    { HDA_INTEL_ICH9,	"ICH9"			/*, 0, 0 */ },
    { HDA_INTEL_82801JI,	"82801JI"		/*, 0, 0 */ },
    { HDA_INTEL_82801JD,	"82801JD"		/*, 0, 0 */ },
    { HDA_INTEL_PCH,	"5 Series/3400 Series"	/*, 0, 0 */ },
    { HDA_INTEL_PCH2,	"5 Series/3400 Series"	/*, 0, 0 */ },
    { HDA_INTEL_SCH,	"SCH"			/*, 0, 0 */ },
    { HDA_INTEL_LPT1,	"Lynx Point"		/*, 0, 0 */ },
    { HDA_INTEL_LPT2,	"Lynx Point"		/*, 0, 0 */ },
    { HDA_INTEL_WCPT,	"Wildcat Point"		/*, 0, 0 */ },
    { HDA_INTEL_WELLS1,	"Wellsburg"		/*, 0, 0 */ },
    { HDA_INTEL_WELLS2,	"Wellsburg"		/*, 0, 0 */ },
    { HDA_INTEL_WCPTLP,	"Wildcat Point-LP"	/*, 0, 0 */ },
    { HDA_INTEL_LPTLP1,	"Lynx Point-LP"		/*, 0, 0 */ },
    { HDA_INTEL_LPTLP2,	"Lynx Point-LP"		/*, 0, 0 */ },
    { HDA_INTEL_SRSPLP,	"Sunrise Point-LP"	/*, 0, 0 */ },
    { HDA_INTEL_SRSP,	"Sunrise Point"		/*, 0, 0 */ },
    
    //10de  NVIDIA Corporation
    { HDA_NVIDIA_MCP51,	"MCP51" /*, 0, HDAC_QUIRK_MSI */ },
    { HDA_NVIDIA_MCP55,	"MCP55" /*, 0, HDAC_QUIRK_MSI */ },
    { HDA_NVIDIA_MCP61_1,	"MCP61" /*, 0, 0 */ },
    { HDA_NVIDIA_MCP61_2,	"MCP61" /*, 0, 0 */ },
    { HDA_NVIDIA_MCP65_1,	"MCP65" /*, 0, 0 */ },
    { HDA_NVIDIA_MCP65_2,	"MCP65" /*, 0, 0 */ },
    { HDA_NVIDIA_MCP67_1,	"MCP67" /*, 0, 0 */ },
    { HDA_NVIDIA_MCP67_2,	"MCP67" /*, 0, 0 */ },
    { HDA_NVIDIA_MCP73_1,	"MCP73" /*, 0, 0 */ },
    { HDA_NVIDIA_MCP73_2,	"MCP73" /*, 0, 0 */ },
    { HDA_NVIDIA_MCP78_1,	"MCP78" /*, 0, HDAC_QUIRK_64BIT */ },
    { HDA_NVIDIA_MCP78_2,	"MCP78" /*, 0, HDAC_QUIRK_64BIT */ },
    { HDA_NVIDIA_MCP78_3,	"MCP78" /*, 0, HDAC_QUIRK_64BIT */ },
    { HDA_NVIDIA_MCP78_4,	"MCP78" /*, 0, HDAC_QUIRK_64BIT */ },
    { HDA_NVIDIA_MCP79_1,	"MCP79" /*, 0, 0 */ },
    { HDA_NVIDIA_MCP79_2,	"MCP79" /*, 0, 0 */ },
    { HDA_NVIDIA_MCP79_3,	"MCP79" /*, 0, 0 */ },
    { HDA_NVIDIA_MCP79_4,	"MCP79" /*, 0, 0 */ },
    { HDA_NVIDIA_MCP89_1,	"MCP89" /*, 0, 0 */ },
    { HDA_NVIDIA_MCP89_2,	"MCP89" /*, 0, 0 */ },
    { HDA_NVIDIA_MCP89_3,	"MCP89" /*, 0, 0 */ },
    { HDA_NVIDIA_MCP89_4,	"MCP89" /*, 0, 0 */ },
    { HDA_NVIDIA_0BE2,	"(0x0be2)" /*, 0, HDAC_QUIRK_MSI */ },
    { HDA_NVIDIA_0BE3,	"(0x0be3)" /*, 0, HDAC_QUIRK_MSI */ },
    { HDA_NVIDIA_0BE4,	"(0x0be4)" /*, 0, HDAC_QUIRK_MSI */ },
    { HDA_NVIDIA_GT100,	"GT100" /*, 0, HDAC_QUIRK_MSI */ },
    { HDA_NVIDIA_GT104,	"GT104" /*, 0, HDAC_QUIRK_MSI */ },
    { HDA_NVIDIA_GT106,	"GT106" /*, 0, HDAC_QUIRK_MSI */ },
    { HDA_NVIDIA_GT108,	"GT108" /*, 0, HDAC_QUIRK_MSI */ },
    { HDA_NVIDIA_GT116,	"GT116" /*, 0, HDAC_QUIRK_MSI */ },
    { HDA_NVIDIA_GF119,	"GF119" /*, 0, 0 */ },
    { HDA_NVIDIA_GF110_1,	"GF110" /*, 0, HDAC_QUIRK_MSI */ },
    { HDA_NVIDIA_GF110_2,	"GF110" /*, 0, HDAC_QUIRK_MSI */ },
    { HDA_NVIDIA_GK110,	"GK110" /*, 0, ? */ },
    { HDA_NVIDIA_GK106,	"GK106" /*, 0, ? */ },
    { HDA_NVIDIA_GK107,	"GK107" /*, 0, ? */ },
    { HDA_NVIDIA_GK104,	"GK104" /*, 0, ? */ },
    
    //1002  Advanced Micro Devices [AMD] nee ATI Technologies Inc
    { HDA_ATI_SB450,	"SB4x0" /*, 0, 0 */ },
    { HDA_ATI_SB600,	"SB600" /*, 0, 0 */ },
    { HDA_ATI_RS600,	"RS600" /*, 0, 0 */ },
    { HDA_ATI_HUDSON,	"Hudson" /*, 0, 0 */ },
    { HDA_ATI_RS690,	"RS690" /*, 0, 0 */ },
    { HDA_ATI_RS780,	"RS780" /*, 0, 0 */ },
    { HDA_ATI_RS880,	"RS880" /*, 0, 0 */ },
    { HDA_ATI_TRINITY,	"Trinity" /*, 0, ? */ },
    { HDA_ATI_R600,		"R600" /*, 0, 0 */ },
    { HDA_ATI_RV610,	"RV610" /*, 0, 0 */ },
    { HDA_ATI_RV620,	"RV620" /*, 0, 0 */ },
    { HDA_ATI_RV630,	"RV630" /*, 0, 0 */ },
    { HDA_ATI_RV635,	"RV635" /*, 0, 0 */ },
    { HDA_ATI_RV710,	"RV710" /*, 0, 0 */ },
    { HDA_ATI_RV730,	"RV730" /*, 0, 0 */ },
    { HDA_ATI_RV740,	"RV740" /*, 0, 0 */ },
    { HDA_ATI_RV770,	"RV770" /*, 0, 0 */ },
    { HDA_ATI_RV810,	"RV810" /*, 0, 0 */ },
    { HDA_ATI_RV830,	"RV830" /*, 0, 0 */ },
    { HDA_ATI_RV840,	"RV840" /*, 0, 0 */ },
    { HDA_ATI_RV870,	"RV870" /*, 0, 0 */ },
    { HDA_ATI_RV910,	"RV910" /*, 0, 0 */ },
    { HDA_ATI_RV930,	"RV930" /*, 0, 0 */ },
    { HDA_ATI_RV940,	"RV940" /*, 0, 0 */ },
    { HDA_ATI_RV970,	"RV970" /*, 0, 0 */ },
    { HDA_ATI_R1000,	"R1000" /*, 0, 0 */ }, // HDMi
    { HDA_ATI_SI,		"SI" /*, 0, 0 */ },
    { HDA_ATI_VERDE,	"Cape Verde" /*, 0, ? */ }, // HDMi
    
    //17f3  RDC Semiconductor, Inc.
    { HDA_RDC_M3010,	"M3010" /*, 0, 0 */ },
    
    //1106  VIA Technologies, Inc.
    { HDA_VIA_VT82XX,	"VT8251/8237A" /*, 0, 0 */ },
    
    //1039  Silicon Integrated Systems [SiS]
    { HDA_SIS_966,		"966" /*, 0, 0 */ },
    
    //10b9  ULi Electronics Inc.(Split off ALi Corporation in 2003)
    { HDA_ULI_M5461,	"M5461" /*, 0, 0 */ },
    
    /* Unknown */
    { HDA_INTEL_ALL,	"Unknown Intel device" /*, 0, 0 */ },
    { HDA_NVIDIA_ALL,	"Unknown NVIDIA device" /*, 0, 0 */ },
    { HDA_ATI_ALL,		"Unknown ATI device" /*, 0, 0 */ },
    { HDA_VIA_ALL,		"Unknown VIA device" /*, 0, 0 */ },
    { HDA_SIS_ALL,		"Unknown SiS device" /*, 0, 0 */ },
    { HDA_ULI_ALL,		"Unknown ULI device" /*, 0, 0 */ },
};
#define HDAC_DEVICES_LEN (sizeof(know_hda_controller) / sizeof(know_hda_controller[0]))

/*****************
 * Device Methods
 *****************/

/* get HDA device name */
CHAR8 *get_hda_controller_name(UINT16 controller_device_id, UINT16 controller_vendor_id)
{
    static char desc[128];
    
    CHAR8 *name_format  = "Unknown HD Audio device %a";
    UINT32 controller_model = ((controller_device_id << 16) | controller_vendor_id);
    INT32 i;
    
    /* Get format for vendor ID */
    switch (controller_vendor_id)
    {
        case ATI_VENDORID:
            name_format = "ATI %a HDA Controller (HDMi)"; break;
            
        case INTEL_VENDORID:
            name_format = "Intel %a HDA Controller"; break;
            
        case NVIDIA_VENDORID:
            name_format = "nVidia %a HDA Controller (HDMi)"; break;
            
        case RDC_VENDORID:
            name_format = "RDC %a HDA Controller"; break;
            
        case SIS_VENDORID:
            name_format = "SiS %a HDA Controller"; break;
            
        case ULI_VENDORID:
            name_format = "ULI %a HDA Controller"; break;
            
        case VIA_VENDORID:
            name_format = "VIA %a HDA Controller"; break;
            
        default:
            break;
    }
    
    for (i = 0; i < HDAC_DEVICES_LEN; i++)
    {
        if (know_hda_controller[i].model == controller_model)
        {
            AsciiSPrint(desc, sizeof(desc), name_format, know_hda_controller[i].desc);
            return desc;
        }
    }
    
    /* Not in table */
    AsciiSPrint(desc, sizeof(desc), "Unknown HDA device, vendor %04x, model %04x",
             controller_vendor_id, controller_device_id);
    return desc;
}

// executing HDA verb command using Immediate Command Input and Output Registers
UINT32 HDA_IC_sendVerb(EFI_PCI_IO_PROTOCOL *PciIo, UINT32 codecAdr, UINT32 nodeId, UINT32 verb)
{
    EFI_STATUS	Status;
    UINT16		ics = 0;
    UINT32		data32 = 0;
    UINT64		data64 = 0;
    
    // about that polling below ...
    // spec says that delay is in 100ns units. value 1.000.000.0
    // should then be 1 second, but users of newer Aptio boards were reporting
    // delays of 10-20 secs when this value was used. maybe this polling timeout
    // value does not mean the same on all implementations?
    // anyway, delay is lowered now to 10.000.0 (10 millis).
    
    // poll ICS[0] to become 0
    Status = PciIo->PollMem(PciIo, EfiPciIoWidthUint16, 0/*bar*/, HDA_ICS/*offset*/, 0x1/*mask*/, 0/*value*/, 100000/*delay in 100ns*/, &data64);
    ics = (UINT16)(data64 & 0xFFFF);
    //DBG("poll ICS[0] == 0: Status=%r, ICS=%x, ICS[0]=%d\n", Status, ics, (ics & 0x0001));
    if (EFI_ERROR(Status)) return 0;
    // prepare and write verb to ICO
    data32 = codecAdr << 28 | ((nodeId & 0xFF)<<20) | (verb & 0xFFFFF);
    Status = PciIo->Mem.Write(PciIo, EfiPciIoWidthUint32, 0, HDA_ICO, 1, &data32);
    //DBG("ICO write verb Codec=%x, Node=%x, verb=%x, command verb=%x: Status=%r\n", codecAdr, nodeId, verb, data32, Status);
    if (EFI_ERROR(Status)) return 0;
    // write 11b to ICS[1:0] to send command
    ics |= 0x3;
    Status = PciIo->Mem.Write(PciIo, EfiPciIoWidthUint16, 0, HDA_ICS, 1, &ics);
    //DBG("ICS[1:0] = 11b: Status=%r\n", Status);
    if (EFI_ERROR(Status)) return 0;
    // poll ICS[1:0] to become 10b
    Status = PciIo->PollMem(PciIo, EfiPciIoWidthUint16, 0/*bar*/, HDA_ICS/*offset*/, 0x3/*mask*/, 0x2/*value*/, 100000/*delay in 100ns*/, &data64);
    //DBG("poll ICS[0] == 0: Status=%r\n", Status);
    if (EFI_ERROR(Status)) return 0;
    // read IRI for VendorId/DeviceId
    Status = PciIo->Mem.Read(PciIo, EfiPciIoWidthUint32, 0, HDA_IRI, 1, &data32);
    if (EFI_ERROR(Status)) return 0;
    return data32;
}

UINT32 HDA_getCodecVendorAndDeviceIds(EFI_PCI_IO_PROTOCOL *PciIo) {
    EFI_STATUS	Status;
    //UINT8		ver[2];
    UINT32		data32 = 0;
    
    // check HDA version - should be 1.0
    /*
     Status = PciIo->Mem.Read(PciIo, EfiPciIoWidthUint8, 0, HDA_VMIN, 2, &ver[0]);
     DBG("HDA Version: Status=%r, version=%d.%d\n", Status, ver[1], ver[0]);
     if (EFI_ERROR(Status)) {
     return 0;
     }
     */
    
    // check if controller is out of reset - GCTL-08h[CRST-bit 0] == 1
    Status = PciIo->Mem.Read(PciIo, EfiPciIoWidthUint32, 0, HDA_GCTL, 1, &data32);
    //DBG("check CRST == 1: Status=%r, CRST=%d\n", Status, (data32 & 0x1));
    if (EFI_ERROR(Status)) {
        return 0;
    }
    if ((data32 & 0x1) == 0) {
        // this controller is not inited yet - we can not read codec ids
        // if needed, we can init it by:
        // - set Control reset bit in Global Control reg 08h[0] to 1
        // - poll it to become 1 again
        // - wait at least 521 micro sec for codecs to init
        // - we can check STATESTS reg 0eh where each running codec will set one bit
        //   codec addr 0 bit 0, codec addr 1 bit 1 ...
        
        return 0;
    }
    //Slice - TODO check codecAdr=2 - it is my Dell 1525.
    // all ok - read Ids
    return HDA_IC_sendVerb(PciIo, 0/*codecAdr*/, 0/*nodeId*/, 0xF0000/*verb*/);
}

UINT32 getLayoutIdFromVendorAndDeviceId(UINT32 vendorDeviceId)
{
    UINT32	layoutId = 0;
    UINT8	  hexDigit = 0;
    
    // extract device id - 2 lower bytes,
    // convert it to decimal like this: 0x0887 => 887 decimal
    hexDigit = vendorDeviceId & 0xF;
    if (hexDigit > 9) return 0;
    layoutId = hexDigit;
    
    vendorDeviceId = vendorDeviceId >> 4;
    hexDigit = vendorDeviceId & 0xF;
    if (hexDigit > 9) return 0;
    layoutId += hexDigit * 10;
    
    vendorDeviceId = vendorDeviceId >> 4;
    hexDigit = vendorDeviceId & 0xF;
    if (hexDigit > 9) return 0;
    layoutId += hexDigit * 100;
    
    vendorDeviceId = vendorDeviceId >> 4;
    hexDigit = vendorDeviceId & 0xF;
    if (hexDigit > 9) return 0;
    layoutId += hexDigit * 1000;
    
    return layoutId;
}

BOOLEAN IsHDMIAudio(EFI_HANDLE PciDevHandle)
{
    EFI_STATUS          Status;
    EFI_PCI_IO_PROTOCOL *PciIo;
    UINTN               Segment;
    UINTN               Bus;
    UINTN               Device;
    UINTN               Function;
    UINTN               Index;
    
    // get device PciIo protocol
    Status = gBS->OpenProtocol(PciDevHandle, &gEfiPciIoProtocolGuid, (VOID **)&PciIo, gImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if (EFI_ERROR(Status)) {
        return FALSE;
    }
    
    // get device location
    Status = PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
    if (EFI_ERROR(Status)) {
        return FALSE;
    }
    
    // iterate over all GFX devices and check for sibling
    for (Index = 0; Index < NGFX; Index++) {
        if (gGraphics[Index].Segment == Segment
            && gGraphics[Index].Bus == Bus
            && gGraphics[Index].Device == Device) {
            return TRUE;
        }
    }
    
    return FALSE;
}

BOOLEAN setup_hda_devprop(pci_dt_t *hda_dev, CHAR8 *OSVersion)
{
	EFI_PCI_IO_PROTOCOL	*PciIo = NULL;
	CHAR8                   *devicepath = NULL;
	DevPropDevice           *device;
	UINT32                  layoutId = 0;
	UINT32                  codecId = 0;
	BOOLEAN                 Injected = FALSE;
	INT32                   i;
	CHAR8			*controller_name = NULL;
	UINT16                  controller_vendor_id = hda_dev->vendor_id;
	UINT16                  controller_device_id = hda_dev->device_id;

    if (!gSettings.HDAInjection) {
        return FALSE;
    }
    
    if (!string) {
        string = devprop_create_string();
    }

    devicepath = get_pci_dev_path(hda_dev);

    controller_name = get_hda_controller_name(controller_device_id, controller_vendor_id);

    //device = devprop_add_device(string, devicepath);
    device = devprop_add_device_pci(string, hda_dev);
    if (!device) {
        return FALSE;
    }

    DBG("HDA Controller [%04x:%04x] :: %a =>", hda_dev->vendor_id, hda_dev->device_id, devicepath);
    
    if (IsHDMIAudio(hda_dev->DeviceHandle)) {
        if (gSettings.NrAddProperties != 0xFFFE) {
            for (i = 0; i < gSettings.NrAddProperties; i++) {
                if (gSettings.AddProperties[i].Device != DEV_HDMI) {
                    continue;
                }
                Injected = TRUE;
                devprop_add_value(device,
                                  gSettings.AddProperties[i].Key,
                                  (UINT8*)gSettings.AddProperties[i].Value,
                                  gSettings.AddProperties[i].ValueLen);
            }
        }
        if (Injected) {
            DBG("custom HDMI properties injected, continue\n");
            //    return TRUE;
        } else if (gSettings.UseIntelHDMI) {
            DBG(" HDMI Audio, setting hda-gfx=onboard-1\n");
            devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
        }
    } else {
        
        // HDA - determine layout-id
        if (gSettings.HDALayoutId > 0) {
            // layoutId is specified - use it
            layoutId = (UINT32)gSettings.HDALayoutId;
            DBG(" setting specified layout-id=%d (0x%x)\n", layoutId, layoutId);
        } else {
            // use detection: layoutId=codec dviceId or use default 12
            codecId = HDA_getCodecVendorAndDeviceIds(PciIo);
            if (codecId != 0) {
                DBG(" detected codec: %04x:%04x\n", (codecId >> 16), (codecId & 0xFFFF));
                layoutId = getLayoutIdFromVendorAndDeviceId(codecId);
            } else {
                DBG(" codec not detected\n");
            }
            // if not detected - use 12 as default
            if (layoutId == 0) {
                layoutId = 12;
            }
            //     DBG(", setting layout-id=%d (0x%x)\n", layoutId, layoutId);
        }
        if (gSettings.NrAddProperties != 0xFFFE) {
            for (i = 0; i < gSettings.NrAddProperties; i++) {
                if (gSettings.AddProperties[i].Device != DEV_HDA) {
                    continue;
                }
                Injected = TRUE;
                devprop_add_value(device,
                                  gSettings.AddProperties[i].Key,
                                  (UINT8*)gSettings.AddProperties[i].Value,
                                  gSettings.AddProperties[i].ValueLen);
            }
        }
        if (!Injected) {
            if ((OSVersion != NULL && AsciiOSVersionToUint64(OSVersion) < AsciiOSVersionToUint64("10.8")) || (gSettings.HDALayoutId > 0)) {
                devprop_add_value(device, "layout-id", (UINT8 *)&layoutId, 4);
            }
            layoutId = 0; // reuse variable
            if (gSettings.UseIntelHDMI) {
                devprop_add_value(device, "hda-gfx", (UINT8 *)"onboard-1", 10);
            }
            codecId = 1; // reuse variable again
            if (gSettings.AFGLowPowerState) {
                devprop_add_value(device, "AFGLowPowerState", (UINT8 *)&codecId, 4);
            }
            
            devprop_add_value(device, "MaximumBootBeepVolume", (UINT8 *)&layoutId, 1);
            devprop_add_value(device, "PinConfigurations", (UINT8 *)&layoutId, 1);
        }
        
    }
    
    return TRUE;
}
