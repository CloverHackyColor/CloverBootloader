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

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "FixBiosDsdt.h"
#include "../include/Devices.h"
#include "../include/Pci.h"
#include "device_inject.h"
#include "../Platform/Settings.h"

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

/* CODECs */
/*
 * ErmaC: There's definitely a lot of different versions of the same audio codec variant out there...
 * in the next struct you will find a "generic" but IMHO detailed list of
 * possible codec... anyway to specific a new one or find difference beetween revision
 * check it under linux enviroment with:
 * $cat /proc/asound/Intel/codec#0
 * --------------------------------
 *  Codec: Analog Devices AD1989B
 *  Address: 0
 *  AFG Function Id: 0x1 (unsol 0)
 *  Vendor Id: 0x11d4989b
 *  Subsystem Id: 0x10438372
 *  Revision Id: 0x100300
 * --------------------------------
 * or
 * $cat /proc/asound/NVidia/codec#0
 * --------------------------------
 *  Codec: Nvidia GPU 14 HDMI/DP
 *  Address: 0
 *  AFG Function Id: 0x1 (unsol 0)
 *  Vendor Id: 0x10de0014
 *  Subsystem Id: 0x10de0101
 *  Revision Id: 0x100100
 * --------------------------------
 */

/*****************
 * Device Methods
 *****************/

#if 0


// executing HDA verb command using Immediate Command Input and Output Registers
UINT32 HDA_IC_sendVerb(EFI_PCI_IO_PROTOCOL *PciIo, UINT32 codecAdr, UINT32 nodeId, UINT32 verb)
{
  EFI_STATUS  Status;
  UINT16    ics = 0;
  UINT32    data32 = 0;
  UINT64    data64 = 0;

  // about that polling below ...
  // spec says that delay is in 100ns units. value 1.000.000.0
  // should then be 1 second, but users of newer Aptio boards were reporting
  // delays of 10-20 secs when this value was used. maybe this polling timeout
  // value does not mean the same on all implementations?
  // anyway, delay is lowered now to 10.000.0 (10 millis).

  // poll ICS[0] to become 0
  Status = PciIo->PollMem(PciIo, EfiPciIoWidthUint16, 0/*bar*/, HDA_ICS/*offset*/, 0x1/*mask*/, 0/*value*/, 100000/*delay in 100ns*/, &data64);
  ics = (UINT16)(data64 & 0xFFFF);
  //DBG("poll ICS[0] == 0: Status=%s, ICS=%X, ICS[0]=%d\n", efiStrError(Status), ics, (ics & 0x0001));
  if (EFI_ERROR(Status)) return 0;
  // prepare and write verb to ICO
  data32 = codecAdr << 28 | ((nodeId & 0xFF)<<20) | (verb & 0xFFFFF);
  Status = PciIo->Mem.Write(PciIo, EfiPciIoWidthUint32, 0, HDA_ICO, 1, &data32);
  //DBG("ICO write verb Codec=%X, Node=%X, verb=%X, command verb=%X: Status=%s\n", codecAdr, nodeId, verb, data32, efiStrError(Status));
  if (EFI_ERROR(Status)) return 0;
  // write 11b to ICS[1:0] to send command
  ics |= 0x3;
  Status = PciIo->Mem.Write(PciIo, EfiPciIoWidthUint16, 0, HDA_ICS, 1, &ics);
  //DBG("ICS[1:0] = 11b: Status=%s\n", efiStrError(Status));
  if (EFI_ERROR(Status)) return 0;
  // poll ICS[1:0] to become 10b
  Status = PciIo->PollMem(PciIo, EfiPciIoWidthUint16, 0/*bar*/, HDA_ICS/*offset*/, 0x3/*mask*/, 0x2/*value*/, 100000/*delay in 100ns*/, &data64);
  //DBG("poll ICS[0] == 0: Status=%s\n", efiStrError(Status));
  if (EFI_ERROR(Status)) return 0;
  // read IRI for VendorId/DeviceId
  Status = PciIo->Mem.Read(PciIo, EfiPciIoWidthUint32, 0, HDA_IRI, 1, &data32);
  if (EFI_ERROR(Status)) return 0;
  return data32;
}
#endif


BOOLEAN EFIAPI IsHDMIAudio(EFI_HANDLE PciDevHandle)
{
  EFI_STATUS          Status;
  EFI_PCI_IO_PROTOCOL *PciIo;
  UINTN               Segment;
  UINTN               Bus;
  UINTN               Device;
  UINTN               Function;
  UINTN               Index;

  // get device PciIo protocol
  Status = gBS->OpenProtocol(PciDevHandle, &gEfiPciIoProtocolGuid, (void **)&PciIo, gImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
  if (EFI_ERROR(Status)) {
    return FALSE;
  }

  // get device location
  Status = PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
  if (EFI_ERROR(Status)) {
    return FALSE;
  }

  // iterate over all GFX devices and check for sibling
  for (Index = 0; Index < gConf.GfxPropertiesArray.size(); Index++) {
    if (gConf.GfxPropertiesArray[Index].Segment == Segment
        && gConf.GfxPropertiesArray[Index].Bus == Bus
        && gConf.GfxPropertiesArray[Index].Device == Device) {
      return TRUE;
    }
  }

  return FALSE;
}

BOOLEAN setup_hda_devprop(EFI_PCI_IO_PROTOCOL *PciIo, pci_dt_t *hda_dev, const MacOsVersion& OSVersion)
{
  DevPropDevice           *device = NULL;
  UINT32                  layoutId = 0;
  UINT32                  codecId = 0;
  BOOLEAN                 Injected = FALSE;
  UINTN                   i;

  if (!device_inject_string) {
    device_inject_string = devprop_create_string();
  }
  if (IsHDMIAudio(hda_dev->DeviceHandle)) {
    if (!gSettings.Devices.HDMIInjection) {
      return FALSE;
    }

    if (hda_dev && !hda_dev->used) {
      device = devprop_add_device_pci(device_inject_string, hda_dev, NULL);
      hda_dev->used = TRUE;
    }
    if (!device) {
      return FALSE;
    }

    if (gSettings.Devices.AddPropertyArray.size() != 0xFFFE) { // Looks like NrAddProperties == 0xFFFE is not used anymore
      for (i = 0; i < gSettings.Devices.AddPropertyArray.size(); i++) {
        if (gSettings.Devices.AddPropertyArray[i].Device != DEV_HDMI) {
          continue;
        }
        Injected = TRUE;

        if (!gSettings.Devices.AddPropertyArray[i].MenuItem.BValue) {
          //DBG("  disabled property Key: %s, len: %d\n", gSettings.Devices.AddPropertyArray[i].Key, gSettings.Devices.AddPropertyArray[i].ValueLen);
        } else {
          devprop_add_value(device, gSettings.Devices.AddPropertyArray[i].Key, gSettings.Devices.AddPropertyArray[i].Value);
          //DBG("  added property Key: %s, len: %d\n", gSettings.Devices.AddPropertyArray[i].Key, gSettings.Devices.AddPropertyArray[i].ValueLen);
        }
      }
    }
    if (Injected) {
      DBG("Additional HDMI properties injected, continue\n");
      //return TRUE;
    } else {
      if (gSettings.Devices.UseIntelHDMI) {
        DBG(" HDMI Audio, used with HDA setting hda-gfx=onboard-2\n");
        devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-2", 10);
      } else {
        DBG(" HDMI Audio, used without HDA setting hda-gfx=onboard-1\n");
        devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
      }
    }
  } else {
    if (!gSettings.Devices.Audio.HDAInjection) {
      return FALSE;
    }
    if (hda_dev && !hda_dev->used) {
      device = devprop_add_device_pci(device_inject_string, hda_dev, NULL);
      hda_dev->used = TRUE;
    }
    if (!device) {
      return FALSE;
    }
    // HDA - determine layout-id
    if (gSettings.Devices.Audio.HDALayoutId > 0) {
      // layoutId is specified - use it
      layoutId = (UINT32)gSettings.Devices.Audio.HDALayoutId;
      DBG(" setting specified layout-id=%d (0x%X)\n", layoutId, layoutId);
    } else {
      layoutId = 12;
    }
    if (gSettings.Devices.AddPropertyArray.size() != 0xFFFE) { // Looks like NrAddProperties == 0xFFFE is not used anymore
      for (i = 0; i < gSettings.Devices.AddPropertyArray.size(); i++) {
        if (gSettings.Devices.AddPropertyArray[i].Device != DEV_HDA) {
          continue;
        }
        Injected = TRUE;

        if (!gSettings.Devices.AddPropertyArray[i].MenuItem.BValue) {
          //DBG("  disabled property Key: %s, len: %d\n", gSettings.Devices.AddPropertyArray[i].Key, gSettings.Devices.AddPropertyArray[i].ValueLen);
        } else {
          devprop_add_value(device, gSettings.Devices.AddPropertyArray[i].Key, gSettings.Devices.AddPropertyArray[i].Value);
          //DBG("  added property Key: %s, len: %d\n", gSettings.Devices.AddPropertyArray[i].Key, gSettings.Devices.AddPropertyArray[i].ValueLen);
        }
      }
    }
    if (!Injected) {
      if ( (OSVersion.notEmpty()  &&  OSVersion < MacOsVersion("10.8"_XS8)) || gSettings.Devices.Audio.HDALayoutId > 0 ) {
        devprop_add_value(device, "layout-id", (UINT8 *)&layoutId, 4);
      }
      layoutId = 0; // reuse variable
      if (gSettings.Devices.UseIntelHDMI) {
        devprop_add_value(device, "hda-gfx", (UINT8 *)"onboard-1", 10);
      }
      codecId = 1; // reuse variable again
      if (gSettings.Devices.Audio.AFGLowPowerState) {
        devprop_add_value(device, "AFGLowPowerState", (UINT8 *)&codecId, 4);
      }

      devprop_add_value(device, "MaximumBootBeepVolume", (UINT8 *)&layoutId, 1);
      devprop_add_value(device, "PinConfigurations", (UINT8 *)&layoutId, 1);
    }
  }
  return TRUE;
}




void ResetHDA()
{
  EFI_STATUS          Status;
  UINTN               HandleCount  = 0;
  EFI_HANDLE          *HandleArray = NULL;
  EFI_PCI_IO_PROTOCOL *PciIo;
  PCI_TYPE00          Pci;
  UINTN               Index;
  UINTN               Segment      = 0;
  UINTN               Bus          = 0;
  UINTN               Device       = 0;
  UINTN               Function     = 0;

  XStringW GopDevicePathStr;


  // Get GOP handle, in order to check to which GPU the monitor is currently connected
  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiGraphicsOutputProtocolGuid, NULL, &HandleCount, &HandleArray);
  if (!EFI_ERROR(Status)) {
    GopDevicePathStr = DevicePathToXStringW(DevicePathFromHandle(HandleArray[0]));
    DBG("GOP found at: %ls\n", GopDevicePathStr.wc_str());
  }

  // Scan PCI handles
  Status = gBS->LocateHandleBuffer (
                                    ByProtocol,
                                    &gEfiPciIoProtocolGuid,
                                    NULL,
                                    &HandleCount,
                                    &HandleArray
                                    );

  if (!EFI_ERROR(Status)) {
    for (Index = 0; Index < HandleCount; ++Index) {
      Status = gBS->HandleProtocol(HandleArray[Index], &gEfiPciIoProtocolGuid, (void **)&PciIo);
      if (!EFI_ERROR(Status)) {
        // Read PCI BUS
        PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
        Status = PciIo->Pci.Read (
                                  PciIo,
                                  EfiPciIoWidthUint32,
                                  0,
                                  sizeof (Pci) / sizeof (UINT32),
                                  &Pci
                                  );

//        DBG("PCI (%02llX|%02llX:%02llX.%02llX) : %04hX %04hX class=%02hhX%02hhX%02hhX\n",
//             Segment,
//             Bus,
//             Device,
//             Function,
//             Pci.Hdr.VendorId,
//             Pci.Hdr.DeviceId,
//             Pci.Hdr.ClassCode[2],
//             Pci.Hdr.ClassCode[1],
//             Pci.Hdr.ClassCode[0]
//             );

        if ( Pci.Hdr.ClassCode[2] == PCI_CLASS_MEDIA &&
                  ( Pci.Hdr.ClassCode[1] == PCI_CLASS_MEDIA_HDA || Pci.Hdr.ClassCode[1] == PCI_CLASS_MEDIA_AUDIO )
                )
        {
          //Slice method from VoodooHDA
          //PCI_HDA_TCSEL_OFFSET = 0x44
          UINT8 Value = 0;
          Status = PciIo->Pci.Read(PciIo, EfiPciIoWidthUint8, 0x44, 1, &Value);

          if ( !EFI_ERROR(Status) ) {
            Value &= 0xf8;
            PciIo->Pci.Write(PciIo, EfiPciIoWidthUint8, 0x44, 1, &Value);
          }
          //ResetControllerHDA();
        }
      }
    }
  }
}


