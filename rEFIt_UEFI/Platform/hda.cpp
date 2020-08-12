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
#include "FixBiosDsdt.h"
#include "../include/Devices.h"
#include "../include/Pci.h"
#include "device_inject.h"

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
  //DBG("poll ICS[0] == 0: Status=%s, ICS=%X, ICS[0]=%d\n", strerror(Status), ics, (ics & 0x0001));
  if (EFI_ERROR(Status)) return 0;
  // prepare and write verb to ICO
  data32 = codecAdr << 28 | ((nodeId & 0xFF)<<20) | (verb & 0xFFFFF);
  Status = PciIo->Mem.Write(PciIo, EfiPciIoWidthUint32, 0, HDA_ICO, 1, &data32);
  //DBG("ICO write verb Codec=%X, Node=%X, verb=%X, command verb=%X: Status=%s\n", codecAdr, nodeId, verb, data32, strerror(Status));
  if (EFI_ERROR(Status)) return 0;
  // write 11b to ICS[1:0] to send command
  ics |= 0x3;
  Status = PciIo->Mem.Write(PciIo, EfiPciIoWidthUint16, 0, HDA_ICS, 1, &ics);
  //DBG("ICS[1:0] = 11b: Status=%s\n", strerror(Status));
  if (EFI_ERROR(Status)) return 0;
  // poll ICS[1:0] to become 10b
  Status = PciIo->PollMem(PciIo, EfiPciIoWidthUint16, 0/*bar*/, HDA_ICS/*offset*/, 0x3/*mask*/, 0x2/*value*/, 100000/*delay in 100ns*/, &data64);
  //DBG("poll ICS[0] == 0: Status=%s\n", strerror(Status));
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

BOOLEAN setup_hda_devprop(EFI_PCI_IO_PROTOCOL *PciIo, pci_dt_t *hda_dev, const XString8& OSVersion)
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
    if (!gSettings.HDMIInjection) {
      return FALSE;
    }

    if (hda_dev && !hda_dev->used) {
      device = devprop_add_device_pci(device_inject_string, hda_dev, NULL);
      hda_dev->used = TRUE;
    }
    if (!device) {
      return FALSE;
    }

    if (gSettings.NrAddProperties != 0xFFFE) {
      for (i = 0; i < gSettings.NrAddProperties; i++) {
        if (gSettings.AddProperties[i].Device != DEV_HDMI) {
          continue;
        }
        Injected = TRUE;

        if (!gSettings.AddProperties[i].MenuItem.BValue) {
          //DBG("  disabled property Key: %s, len: %d\n", gSettings.AddProperties[i].Key, gSettings.AddProperties[i].ValueLen);
        } else {
          devprop_add_value(device,
                            gSettings.AddProperties[i].Key,
                            (UINT8*)gSettings.AddProperties[i].Value,
                            gSettings.AddProperties[i].ValueLen);
          //DBG("  added property Key: %s, len: %d\n", gSettings.AddProperties[i].Key, gSettings.AddProperties[i].ValueLen);
        }
      }
    }
    if (Injected) {
      DBG("Additional HDMI properties injected, continue\n");
      //return TRUE;
    } else {
      if (gSettings.UseIntelHDMI) {
        DBG(" HDMI Audio, used with HDA setting hda-gfx=onboard-2\n");
        devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-2", 10);
      } else {
        DBG(" HDMI Audio, used without HDA setting hda-gfx=onboard-1\n");
        devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
      }
    }
  } else {
    if (!gSettings.HDAInjection) {
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
    if (gSettings.HDALayoutId > 0) {
      // layoutId is specified - use it
      layoutId = (UINT32)gSettings.HDALayoutId;
      DBG(" setting specified layout-id=%d (0x%X)\n", layoutId, layoutId);
    } else {
      layoutId = 12;
    }
    if (gSettings.NrAddProperties != 0xFFFE) {
      for (i = 0; i < gSettings.NrAddProperties; i++) {
        if (gSettings.AddProperties[i].Device != DEV_HDA) {
          continue;
        }
        Injected = TRUE;

        if (!gSettings.AddProperties[i].MenuItem.BValue) {
          //DBG("  disabled property Key: %s, len: %d\n", gSettings.AddProperties[i].Key, gSettings.AddProperties[i].ValueLen);
        } else {
          devprop_add_value(device,
                            gSettings.AddProperties[i].Key,
                            (UINT8*)gSettings.AddProperties[i].Value,
                            gSettings.AddProperties[i].ValueLen);
          //DBG("  added property Key: %s, len: %d\n", gSettings.AddProperties[i].Key, gSettings.AddProperties[i].ValueLen);
        }
      }
    }
    if (!Injected) {
      if ((OSVersion.notEmpty() && AsciiOSVersionToUint64(OSVersion) < AsciiOSVersionToUint64("10.8"_XS8)) || (gSettings.HDALayoutId > 0)) {
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

