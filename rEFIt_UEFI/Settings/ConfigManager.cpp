/*
 * ConfigManager.cpp
 *
 *  Created on: Apr 21, 2021
 *      Author: jief
 */

#include "ConfigManager.h"
#include "../Settings/SelfOem.h"
#include "../refit/lib.h"
#include "../Platform/Settings.h"
#include "../Platform/platformdata.h"
#include "../Platform/VersionString.h"
#include "../Platform/Nvram.h"

#include "../Platform/smbios.h"

#include "../Platform/gma.h"
#include "../Platform/ati.h"
#include "../Platform/ati_reg.h"
#include "../Platform/nvidia.h"

#include "../Platform/hda.h"

#include "../include/Net.h"
#include "../entry_scan/secureboot.h"


#ifndef DEBUG_ALL
#define DEBUG_CONFIGMANAGER 1
#else
#define DEBUG_CONFIGMANAGER DEBUG_ALL
#endif

#if DEBUG_CONFIGMANAGER == 0
#define DBG(...)
#else
#define DBG(...) DebugLog (DEBUG_CONFIGMANAGER, __VA_ARGS__)
#endif

void ConfigManager::DiscoverDevices()
{
  EFI_STATUS          Status;
  UINT16              PreviousVendor = 0;
  XStringW            GopDevicePathStr;

  DbgHeader("GetDevices");

  {
    // Get GOP handle, in order to check to which GPU the monitor is currently connected
    UINTN               HandleCount  = 0;
    EFI_HANDLE          *HandleArray = NULL;
    Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiGraphicsOutputProtocolGuid, NULL, &HandleCount, &HandleArray);
    if (!EFI_ERROR(Status)) {
      if ( HandleCount == 0 ) {
        log_technical_bug("HandleCount == 0");
      }else{
        if ( HandleCount > 1 ) {
          MsgLog("Found more than one GOP protocol ??? Using the first one\n");
        }
        GopDevicePathStr = DevicePathToXStringW(DevicePathFromHandle(HandleArray[0]));
        DBG("GOP found at: %ls\n", GopDevicePathStr.wc_str());
      }
    }
  }

  // Scan PCI handles
  UINTN               HandleCount  = 0;
  EFI_HANDLE          *HandleArray = NULL;
  Status = gBS->LocateHandleBuffer (
                                    ByProtocol,
                                    &gEfiPciIoProtocolGuid,
                                    NULL,
                                    &HandleCount,
                                    &HandleArray
                                    );

  if (!EFI_ERROR(Status)) {
    for (UINTN Index = 0; Index < HandleCount; ++Index) {
      EFI_PCI_IO_PROTOCOL *PciIo;
      Status = gBS->HandleProtocol(HandleArray[Index], &gEfiPciIoProtocolGuid, (void **)&PciIo);
      if (!EFI_ERROR(Status)) {
        // Read PCI BUS
        UINTN               Segment      = 0;
        UINTN               Bus          = 0;
        UINTN               Device       = 0;
        UINTN               Function     = 0;
        PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
        PCI_TYPE00          Pci;
        Status = PciIo->Pci.Read (
                                  PciIo,
                                  EfiPciIoWidthUint32,
                                  0,
                                  sizeof (Pci) / sizeof (UINT32),
                                  &Pci
                                  );

      DBG("PCI (%02llX|%02llX:%02llX.%02llX) : %04hX %04hX class=%02hhX%02hhX%02hhX\n",
             Segment,
             Bus,
             Device,
             Function,
             Pci.Hdr.VendorId,
             Pci.Hdr.DeviceId,
             Pci.Hdr.ClassCode[2],
             Pci.Hdr.ClassCode[1],
             Pci.Hdr.ClassCode[0]
             );

        // GFX
        //if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_DISPLAY) &&
        //    (Pci.Hdr.ClassCode[1] == PCI_CLASS_DISPLAY_VGA) &&
        //    (NGFX < 4)) {

        if ( Pci.Hdr.ClassCode[2] == PCI_CLASS_DISPLAY &&
             ( Pci.Hdr.ClassCode[1] == PCI_CLASS_DISPLAY_VGA  ||  Pci.Hdr.ClassCode[1] == PCI_CLASS_DISPLAY_OTHER )
            ) {
          CONST CHAR8 *CardFamily = "";
          UINT16 UFamily;
          DiscoveredGfx *gfx = new DiscoveredGfx;


          gfx->DeviceID       = Pci.Hdr.DeviceId;
          gfx->Segment        = Segment;
          gfx->Bus            = Bus;
          gfx->Device         = Device;
          gfx->Function       = Function;
          gfx->Handle         = HandleArray[Index];

          switch (Pci.Hdr.VendorId) {
            case 0x1002: {
              const radeon_card_info_t *info = NULL;
              gfx->Vendor = Ati;

              size_t i = 0;
              do {
                info      = &radeon_cards[i];
                if (info->device_id == Pci.Hdr.DeviceId) {
                  break;
                }
              } while (radeon_cards[i++].device_id != 0);

              gfx->Model.takeValueFrom(info->model_name);
              gfx->Config.takeValueFrom(card_configs[info->cfg_name].name);
              gfx->Ports                  = card_configs[info->cfg_name].ports;
              DBG(" - GFX: Model=%s (ATI/AMD)\n", gfx->Model.c_str());

              //get mmio
              if (info->chip_family < CHIP_FAMILY_HAINAN) {
                gfx->Mmio = (UINT8 *)(UINTN)(Pci.Device.Bar[2] & ~0x0f);
              } else {
                gfx->Mmio = (UINT8 *)(UINTN)(Pci.Device.Bar[5] & ~0x0f);
              }
              gfx->Connectors = *(UINT32*)(gfx->Mmio + RADEON_BIOS_0_SCRATCH);
              //           DBG(" - RADEON_BIOS_0_SCRATCH = 0x%08X\n", gfx->Connectors);
              gfx->ConnChanged = FALSE;

              DiscoveredSlotDeviceClass* SlotDevice = new DiscoveredSlotDeviceClass;
              SlotDeviceArrayNonConst.AddReference(SlotDevice, true);
              SlotDevice->Index = 0;
              SlotDevice->SegmentGroupNum = (UINT16)Segment;
              SlotDevice->BusNum          = (UINT8)Bus;
              SlotDevice->DevFuncNum      = (UINT8)((Device << 3) | (Function & 0x07));
              //SlotDevice->Valid           = TRUE;
              SlotDevice->SlotName        = "PCI Slot 0"_XS8;
              SlotDevice->SlotID          = 1;
              SlotDevice->SlotType        = SlotTypePciExpressX16;
              break;
            }
            case 0x8086:{
              gfx->Vendor                 = Intel;
              gfx->Model.takeValueFrom(get_gma_model (Pci.Hdr.DeviceId));
              DBG(" - GFX: Model=%s (Intel)\n", gfx->Model.c_str());
              gfx->Ports = 1;
              gfx->Connectors = (1 << GfxPropertiesArrayNonConst.size());
              gfx->ConnChanged = FALSE;
              break;
            }
            case 0x10de: {
              gfx->Vendor = Nvidia;
              UINT32 Bar0 = Pci.Device.Bar[0];
              gfx->Mmio   = (UINT8*)(UINTN)(Bar0 & ~0x0f);
              //DBG("BAR: 0x%p\n", Mmio);
              // get card type
              gfx->Family = (REG32(gfx->Mmio, 0) >> 20) & 0x1ff;
              UFamily = gfx->Family & 0x1F0;
              if ((UFamily == NV_ARCH_KEPLER1) ||
                  (UFamily == NV_ARCH_KEPLER2) ||
                  (UFamily == NV_ARCH_KEPLER3)) {
                CardFamily = "Kepler";
              }
              else if ((UFamily == NV_ARCH_FERMI1) ||
                       (UFamily == NV_ARCH_FERMI2)) {
                CardFamily = "Fermi";
              }
              else if ((UFamily == NV_ARCH_MAXWELL1) ||
                       (UFamily == NV_ARCH_MAXWELL2)) {
                CardFamily = "Maxwell";
              }
              else if (UFamily == NV_ARCH_PASCAL) {
                CardFamily = "Pascal";
              }
              else if (UFamily == NV_ARCH_VOLTA) {
                CardFamily = "Volta";
              }
              else if (UFamily == NV_ARCH_TURING) {
                CardFamily = "Turing";
              }
              else if ((UFamily >= NV_ARCH_TESLA) && (UFamily < 0xB0)) { //not sure if 0xB0 is Tesla or Fermi
                CardFamily = "Tesla";
              } else {
                CardFamily = "NVidia unknown";
              }

              gfx->Model.takeValueFrom(
                          get_nvidia_model (((Pci.Hdr.VendorId << 16) | Pci.Hdr.DeviceId),
                                            ((Pci.Device.SubsystemVendorID << 16) | Pci.Device.SubsystemID)
                                           )
                          );

            DBG(" - GFX: Model=%s family %hX (%s)\n", gfx->Model.c_str(), gfx->Family, CardFamily);
              gfx->Ports                  = 0;

              DiscoveredSlotDeviceClass* SlotDevice = new DiscoveredSlotDeviceClass;
              SlotDeviceArrayNonConst.AddReference(SlotDevice, true);
              SlotDevice->Index = 1;
              SlotDevice->SegmentGroupNum = (UINT16)Segment;
              SlotDevice->BusNum          = (UINT8)Bus;
              SlotDevice->DevFuncNum      = (UINT8)((Device << 3) | (Function & 0x07));
              //SlotDevice->Valid           = TRUE;
              SlotDevice->SlotName = "PCI Slot 0"_XS8;
              SlotDevice->SlotID          = 1;
              SlotDevice->SlotType        = SlotTypePciExpressX16;
              break;
            }
            default: {
              gfx->Vendor = Unknown;
              gfx->Model.S8Printf("pci%hx,%hx", Pci.Hdr.VendorId, Pci.Hdr.DeviceId);
              gfx->Ports  = 1;
              gfx->Connectors = (1 << GfxPropertiesArrayNonConst.size());
              gfx->ConnChanged = FALSE;

              break;
            }
          }

          // GOP device path should contain the device path of the GPU to which the monitor is connected
          XStringW DevicePathStr = DevicePathToXStringW(DevicePathFromHandle(HandleArray[Index]));
          if (StrStr(GopDevicePathStr.wc_str(), DevicePathStr.wc_str())) {
            DBG(" - GOP: Provided by device\n");
            if ( GfxPropertiesArrayNonConst.size() != 0 ) {
               // we found GOP on a GPU scanned later, make space for this GPU at first position
              GfxPropertiesArrayNonConst.InsertRef(gfx, 0, true);
            }else{
              GfxPropertiesArrayNonConst.AddReference(gfx, true);
            }
          }
        }   //if gfx

        else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_NETWORK) &&
                 (Pci.Hdr.ClassCode[1] == PCI_CLASS_NETWORK_OTHER)) {
          DiscoveredSlotDeviceClass* SlotDevice = new DiscoveredSlotDeviceClass;
          SlotDeviceArrayNonConst.AddReference(SlotDevice, true);
          SlotDevice->Index = 6;
          SlotDevice->SegmentGroupNum = (UINT16)Segment;
          SlotDevice->BusNum          = (UINT8)Bus;
          SlotDevice->DevFuncNum      = (UINT8)((Device << 3) | (Function & 0x07));
          //SlotDevice->Valid           = TRUE;
          SlotDevice->SlotName = "AirPort"_XS8;
          SlotDevice->SlotID          = 0;
          SlotDevice->SlotType        = SlotTypePciExpressX1;
          DBG(" - WIFI: Vendor= ");
          switch (Pci.Hdr.VendorId) {
            case 0x11ab:
              DBG("Marvell\n");
              break;
            case 0x10ec:
              DBG("Realtek\n");
              break;
            case 0x14e4:
              DBG("Broadcom\n");
              break;
            case 0x1969:
            case 0x168C:
              DBG("Atheros\n");
              break;
            case 0x1814:
              DBG("Ralink\n");
              break;
            case 0x8086:
              DBG("Intel\n");
              break;

            default:
              DBG(" 0x%04X\n", Pci.Hdr.VendorId);
              break;
          }
        }

        else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_NETWORK) &&
                 (Pci.Hdr.ClassCode[1] == PCI_CLASS_NETWORK_ETHERNET)) {
          DiscoveredSlotDeviceClass* SlotDevice = new DiscoveredSlotDeviceClass;
          SlotDeviceArrayNonConst.AddReference(SlotDevice, true);
          SlotDevice->Index = 5;
          SlotDevice->SegmentGroupNum = (UINT16)Segment;
          SlotDevice->BusNum          = (UINT8)Bus;
          SlotDevice->DevFuncNum      = (UINT8)((Device << 3) | (Function & 0x07));
          //SlotDevice->Valid           = TRUE;
          SlotDevice->SlotName = "Ethernet"_XS8;
          SlotDevice->SlotID          = 2;
          SlotDevice->SlotType        = SlotTypePciExpressX1;
          LanCardClass* lanCard = new LanCardClass;
          LanCardArrayNonConst.AddReference(lanCard, true);
          UINT16 Vendor           = Pci.Hdr.VendorId;
          UINT32 Bar0             = Pci.Device.Bar[0];
          UINT8* Mmio             = (UINT8*)(UINTN)(Bar0 & ~0x0f);
          DBG(" - LAN: %zu Vendor=", LanCardArrayNonConst.size());
          switch (Pci.Hdr.VendorId) {
            case 0x11ab:
              DBG("Marvell\n");
              break;
            case 0x10ec:
              DBG("Realtek\n");
              break;
            case 0x14e4:
              DBG("Broadcom\n");
              break;
            case 0x1969:
            case 0x168C:
              DBG("Atheros\n");
              break;
            case 0x8086:
              DBG("Intel\n");
              break;
            case 0x10de:
              DBG("Nforce\n");
              break;

            default:
              DBG("Unknown\n");
              break;
          }

          //
          //  Get MAC-address from hardwaredirectly
          //
          if ( Mmio != NULL ) {
            UINTN Offset = 0;
            BOOLEAN Swab = FALSE;
            UINT32 Mac0, Mac4;
            switch ( Vendor ) {
              case 0x11ab:   //Marvell Yukon
                if (PreviousVendor == Vendor) {
                  Offset = B2_MAC_2;
                } else {
                  Offset = B2_MAC_1;
                }
                CopyMem(&lanCard->MacAddress[0], Mmio + Offset, 6);
                goto done;

              case 0x10ec:   //Realtek
                Mac0 = IoRead32((UINTN)Mmio);
                Mac4 = IoRead32((UINTN)Mmio + 4);
                goto copy;

              case 0x14e4:   //Broadcom
                if (PreviousVendor == Vendor) {
                  Offset = EMAC_MACADDR1_HI;
                } else {
                  Offset = EMAC_MACADDR0_HI;
                }
                break;
              case 0x1969:   //Atheros
                Offset = L1C_STAD0;
                Swab = TRUE;
                break;
              case 0x8086:   //Intel
                if (PreviousVendor == Vendor) {
                  Offset = INTEL_MAC_2;
                } else {
                  Offset = INTEL_MAC_1;
                }
                break;

              default:
                break;
            }
            if (!Offset) {
              continue;
            }
            Mac0 = *(UINT32*)(Mmio + Offset);
            Mac4 = *(UINT32*)(Mmio + Offset + 4);
            if (Swab) {
              lanCard->MacAddress[0] = (UINT8)((Mac4 & 0xFF00) >> 8);
              lanCard->MacAddress[1] = (UINT8)(Mac4 & 0xFF);
              lanCard->MacAddress[2] = (UINT8)((Mac0 & 0xFF000000) >> 24);
              lanCard->MacAddress[3] = (UINT8)((Mac0 & 0x00FF0000) >> 16);
              lanCard->MacAddress[4] = (UINT8)((Mac0 & 0x0000FF00) >> 8);
              lanCard->MacAddress[5] = (UINT8)(Mac0 & 0x000000FF);
              goto done;
            }
          copy:
            CopyMem(&lanCard->MacAddress[0], &Mac0, 4);
            CopyMem(&lanCard->MacAddress[4], &Mac4, 2);

          done:
            PreviousVendor = Vendor;
            DBG("Legacy MAC address of LAN #%zu= ", LanCardArrayNonConst.size()-1); // size() can't be 0 here.
            for (size_t Index2 = 0; Index2 < sizeof(lanCard->MacAddress); Index2++) {
              DBG("%02hhX:", lanCard->MacAddress[Index2]);
            }
            DBG("\n");
          }
        }
        else if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_SERIAL) &&
                 (Pci.Hdr.ClassCode[1] == PCI_CLASS_SERIAL_FIREWIRE)) {
          DiscoveredSlotDeviceClass* SlotDevice = new DiscoveredSlotDeviceClass;
          SlotDeviceArrayNonConst.AddReference(SlotDevice, true);
          SlotDevice->Index = 12;
          SlotDevice->SegmentGroupNum = (UINT16)Segment;
          SlotDevice->BusNum          = (UINT8)Bus;
          SlotDevice->DevFuncNum      = (UINT8)((Device << 3) | (Function & 0x07));
          //SlotDevice->Valid           = TRUE;
          SlotDevice->SlotName = "FireWire"_XS8;
          SlotDevice->SlotID          = 3;
          SlotDevice->SlotType        = SlotTypePciExpressX4;
        }

        else if ( Pci.Hdr.ClassCode[2] == PCI_CLASS_MEDIA &&
                  ( Pci.Hdr.ClassCode[1] == PCI_CLASS_MEDIA_HDA || Pci.Hdr.ClassCode[1] == PCI_CLASS_MEDIA_AUDIO )
                ) {
          DiscoveredHdaProperties *hda = new DiscoveredHdaProperties;

          // Populate Controllers IDs
          hda->controller_vendor_id       = Pci.Hdr.VendorId;
          hda->controller_device_id       = Pci.Hdr.DeviceId;

          // HDA Controller Info
          HdaControllerGetName(((hda->controller_device_id << 16) | hda->controller_vendor_id), &hda->controller_name);


          if (IsHDMIAudio(HandleArray[Index])) {
            DBG(" - HDMI Audio: \n");

            DiscoveredSlotDeviceClass* SlotDevice = new DiscoveredSlotDeviceClass;
            SlotDeviceArrayNonConst.AddReference(SlotDevice, true);
            SlotDevice->Index = 4;
            SlotDevice->SegmentGroupNum = (UINT16)Segment;
            SlotDevice->BusNum          = (UINT8)Bus;
            SlotDevice->DevFuncNum      = (UINT8)((Device << 3) | (Function & 0x07));
            //SlotDevice->Valid           = TRUE;
            SlotDevice->SlotName = "HDMI port"_XS8;
            SlotDevice->SlotID          = 5;
            SlotDevice->SlotType        = SlotTypePciExpressX4;
          }
// TODO not done here anymore! Here, we discover devices. No more. No other action.
//          if (gSettings.Devices.Audio.ResetHDA) {
//            //Slice method from VoodooHDA
//            //PCI_HDA_TCSEL_OFFSET = 0x44
//            UINT8 Value = 0;
//            Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint8, 0x44, 1, &Value);
//
//            if (EFI_ERROR(Status)) {
//              continue;
//            }
//
//            Value &= 0xf8;
//            PciIo->Pci.Write (PciIo, EfiPciIoWidthUint8, 0x44, 1, &Value);
//            //ResetControllerHDA();
//          }
          HdaPropertiesArrayNonConst.AddReference(hda, true);
        } // if Audio device
      }
    }
  }
}


template<class C>
EFI_STATUS LoadPlist(const XStringW& ConfName, C* plist)
{
  EFI_STATUS Status = EFI_NOT_FOUND;
  UINTN      Size = 0;
  CHAR8*     ConfigPtr = NULL;
//  XStringW   ConfigPlistPath;
//  XStringW   ConfigOemPath;

  //  DbgHeader("LoadUserSettings");

  // load config
  if ( ConfName.isEmpty() /*|| Dict == NULL*/ ) {
    return EFI_NOT_FOUND;
  }

//  ConfigOemPath   = SWPrintf("%ls\\%ls.plist", selfOem.getOOEMPath.wc_str(), ConfName.wc_str());
  Status = EFI_NOT_FOUND;
  XStringW configFilename = SWPrintf("%ls.plist", ConfName.wc_str());
  XStringW configPlistPath;
  if ( selfOem.oemDirExists() ) {
    configPlistPath = SWPrintf("%ls\\%ls.plist", selfOem.getOemFullPath().wc_str(), ConfName.wc_str());
    if (FileExists (&selfOem.getOemDir(), configFilename)) {
      Status = egLoadFile(&selfOem.getOemDir(), configFilename.wc_str(), (UINT8**)&ConfigPtr, &Size);
      if (EFI_ERROR(Status)) {
        DBG("Cannot find %ls at path (%s): '%ls', trying '%ls'\n", configFilename.wc_str(), efiStrError(Status), selfOem.getOemFullPath().wc_str(), self.getCloverDirFullPath().wc_str());
      }else{
        DBG("Using %ls at path: %ls\n", configFilename.wc_str(), selfOem.getOemFullPath().wc_str());
      }
    }
  }
  if ( !selfOem.oemDirExists()  ||  EFI_ERROR(Status)) {
    configPlistPath = SWPrintf("%ls\\%ls.plist", self.getCloverDirFullPath().wc_str(), ConfName.wc_str());
    if ( FileExists(&self.getCloverDir(), configFilename.wc_str())) {
      Status = egLoadFile(&self.getCloverDir(), configFilename.wc_str(), (UINT8**)&ConfigPtr, &Size);
    }
    if (EFI_ERROR(Status)) {
      DBG("Cannot find %ls at path '%ls' : %s\n", configFilename.wc_str(), self.getCloverDirFullPath().wc_str(), efiStrError(Status));
    } else {
      DBG("Using %ls at path: %ls\n", configFilename.wc_str(), self.getCloverDirFullPath().wc_str());
    }
  }
  if ( EFI_ERROR(Status) ) {
    MsgLog("'%ls' not loaded. Efi error %s\n", configPlistPath.wc_str(), efiStrError(Status));
    return Status;
  }
  
  XmlLiteParser xmlLiteParser;
  bool parsingOk = plist->parse((const CHAR8*)ConfigPtr, Size, ""_XS8, &xmlLiteParser);
  if ( xmlLiteParser.getErrorsAndWarnings().size() ) {
    if ( xmlLiteParser.getErrorsAndWarnings().size() > 1 ) {
      DebugLog(2, "There are problems in plist '%ls'\n", configPlistPath.wc_str());
    }else{
      DebugLog(2, "There is a problem in plist '%ls'\n", configPlistPath.wc_str());
    }
    for ( size_t idx = 0 ; idx < xmlLiteParser.getErrorsAndWarnings().size() ; idx++ ) {
      const XmlParserMessage& xmlMsg = xmlLiteParser.getErrorsAndWarnings()[idx];
      DebugLog(2, "%s: %s\n", xmlMsg.isError ? "Error" : "Warning", xmlMsg.msg.c_str());
    }
    DebugLog(2, "Use CloverConfigPlistValidator or look in the log\n");
  }
  if ( !parsingOk ) {
    DebugLog(2, "Parsing error while parsing '%ls'.\n", configPlistPath.wc_str());
    Status = EFI_LOAD_ERROR;
  }

  if ( !parsingOk || xmlLiteParser.getErrorsAndWarnings().size() > 0 ) gBS->Stall(3000000); // 3 seconds delay

return Status;
}

/*
 * Load a plist into configPlist global object
 * ConfName : name of the file, without .plist extension. File will be searched in OEM or main folder
 */
EFI_STATUS ConfigManager::LoadConfigPlist(const XStringW& ConfName)
{
  EFI_STATUS Status = LoadPlist(ConfName, &configPlist);

  return Status;
}

/*
 * Load a plist into smbiosPlist global object
 * ConfName : name of the file, without .plist extension. File will be searched in OEM or main folder
 */
EFI_STATUS ConfigManager::LoadSMBIOSPlist(const XStringW& ConfName)
{
  EFI_STATUS Status = LoadPlist(ConfName, &smbiosPlist);

  if ( EFI_ERROR(Status) ) {
    smbiosPlist.reset();
  }
  return Status;
}


void ConfigManager::FillSmbiosWithDefaultValue(MACHINE_TYPES Model, const SmbiosPlistClass::SmbiosDictClass& smbiosDictClass)
{
  GlobalConfig.CurrentModel = Model;

  //GlobalConfig.BiosVersionUsed = ApplePlatformData[Model].firmwareVersion;
  // Check for BiosVersion and BiosReleaseDate by Sherlocks
  if ( smbiosDictClass.getBiosVersion().isDefined() ) {
    int c = compareBiosVersion(GlobalConfig.BiosVersionUsed, smbiosDictClass.dgetBiosVersion());
    if ( c == 0 ) {
      DBG("Found same BiosVersion in clover and config\n");
    }else
    if ( c < 0 ) {
      DBG("Using latest BiosVersion from config\n");
      GlobalConfig.BiosVersionUsed = smbiosDictClass.dgetBiosVersion();
    }else{
      DBG("Using latest BiosVersion from clover\n");
    }
  }else{
    DBG("BiosVersion: not set, Using BiosVersion from clover\n");
  }
  DBG("BiosVersion: %s\n", GlobalConfig.BiosVersionUsed.c_str());


  //GlobalConfig.ReleaseDateUsed = GetReleaseDate(Model); // AppleReleaseDate
  int compareReleaseDateResult = 0;
  if ( smbiosDictClass.getBiosReleaseDate().isDefined() ) {
    compareReleaseDateResult = compareReleaseDate(GetReleaseDate(Model), smbiosDictClass.dgetBiosReleaseDate());
    if ( compareReleaseDateResult == 0 ) {
      DBG("Found same BiosReleaseDate in clover and config\n");
    }else
    if ( compareReleaseDateResult == -1 ) {
      DBG("Using latest BiosReleaseDate from config\n");
      GlobalConfig.ReleaseDateUsed = smbiosDictClass.dgetBiosReleaseDate();
    }else
    if ( compareReleaseDateResult == 1 ) {
      DBG("Using latest BiosReleaseDate from clover\n");
    }
  }else{
    DBG("BiosReleaseDate: not set, Using BiosReleaseDate from clover\n");
  }
  if ( !smbiosDictClass.getBiosReleaseDate().isDefined() || compareReleaseDateResult == -2 )
  {
    //DBG("Found unknown date format from config\n");
    size_t len = GlobalConfig.ReleaseDateUsed.length();
    const char* j = GlobalConfig.BiosVersionUsed.c_str();

    j += AsciiStrLen(j);
    while (*j != '.') {
      j--;
    }

    if ( len == 8 ) {
      GlobalConfig.ReleaseDateUsed.S8Printf("%c%c/%c%c/%c%c\n", j[3], j[4], j[5], j[6], j[1], j[2]);
      //DBG("Using the date of used BiosVersion\n");
    } else if ( len == 10 ) {
      GlobalConfig.ReleaseDateUsed.S8Printf("%c%c/%c%c/20%c%c\n", j[3], j[4], j[5], j[6], j[1], j[2]);
      //DBG("Using the date of used BiosVersion\n");
    }
  }
  DBG("BiosReleaseDate: %s\n", GlobalConfig.ReleaseDateUsed.c_str());



//  GlobalConfig.EfiVersionUsed.takeValueFrom(ApplePlatformData[Model].efiversion);
  if ( smbiosDictClass.getEfiVersion().isDefined() ) {
    if (AsciiStrVersionToUint64(GlobalConfig.EfiVersionUsed, 4, 5) > AsciiStrVersionToUint64(smbiosDictClass.dgetEfiVersion(), 4, 5)) {
      DBG("Using latest EfiVersion from clover: %s\n", GlobalConfig.EfiVersionUsed.c_str());
    } else if (AsciiStrVersionToUint64(GlobalConfig.EfiVersionUsed, 4, 5) < AsciiStrVersionToUint64(smbiosDictClass.dgetEfiVersion(), 4, 5)) {
      GlobalConfig.EfiVersionUsed = smbiosDictClass.dgetEfiVersion();
      DBG("Using latest EfiVersion from config: %s\n", GlobalConfig.EfiVersionUsed.c_str());
    } else {
      DBG("Using EfiVersion from clover: %s\n", GlobalConfig.EfiVersionUsed.c_str());
    }
  } else if ( GlobalConfig.EfiVersionUsed.notEmpty() ) {
    DBG("Using EfiVersion from clover: %s\n", GlobalConfig.EfiVersionUsed.c_str());
  }


  if ( smbiosDictClass.getBiosVendor().isDefined() ) gSettings.Smbios.BiosVendor = smbiosDictClass.getBiosVendor().value();
  if ( smbiosDictClass.getManufacturer().isDefined() ) gSettings.Smbios.ManufactureName = smbiosDictClass.getManufacturer().value();
  if ( smbiosDictClass.getProductName().isDefined() ) gSettings.Smbios.ProductName = smbiosDictClass.getProductName().value();
  if ( smbiosDictClass.getVersion().isDefined() ) gSettings.Smbios.SystemVersion = smbiosDictClass.getVersion().value();
  if ( smbiosDictClass.getSerialNumber().isDefined() ) gSettings.Smbios.SerialNr = smbiosDictClass.getSerialNumber().value();
  if ( smbiosDictClass.getFamily().isDefined() ) gSettings.Smbios.FamilyName = smbiosDictClass.getFamily().value();
  if ( smbiosDictClass.getBoardManufacturer().isDefined() ) gSettings.Smbios.BoardManufactureName = smbiosDictClass.getBoardManufacturer().value();
  if ( smbiosDictClass.getBoardSerialNumber().isDefined() ) gSettings.Smbios.BoardSerialNumber = smbiosDictClass.getBoardSerialNumber().value();
  if ( smbiosDictClass.getBoardID().isDefined() ) gSettings.Smbios.BoardNumber = smbiosDictClass.getBoardID().value();
  if ( smbiosDictClass.getBoardVersion().isDefined() ) gSettings.Smbios.BoardVersion = smbiosDictClass.getBoardVersion().value();
  if ( smbiosDictClass.getLocationInChassis().isDefined() ) gSettings.Smbios.LocationInChassis = smbiosDictClass.getLocationInChassis().value();
  if ( smbiosDictClass.getChassisManufacturer().isDefined() ) gSettings.Smbios.ChassisManufacturer = smbiosDictClass.getChassisManufacturer().value();
  if ( smbiosDictClass.getChassisAssetTag().isDefined() ) gSettings.Smbios.ChassisAssetTag = smbiosDictClass.getChassisAssetTag().value();
  if ( smbiosDictClass.getFirmwareFeatures().isDefined() ) gSettings.Smbios.FirmwareFeatures = smbiosDictClass.getFirmwareFeatures().value();
  if ( smbiosDictClass.getFirmwareFeaturesMask().isDefined() ) gSettings.Smbios.FirmwareFeaturesMask = smbiosDictClass.getFirmwareFeaturesMask().value();

  //ExtendedFirmwareFeatures
  if ( smbiosDictClass.getExtendedFirmwareFeatures().isDefined() ) gSettings.Smbios.ExtendedFirmwareFeatures = smbiosDictClass.getExtendedFirmwareFeatures().value();
  if ( smbiosDictClass.getExtendedFirmwareFeaturesMask().isDefined() ) gSettings.Smbios.ExtendedFirmwareFeaturesMask = smbiosDictClass.getExtendedFirmwareFeaturesMask().value();
  if ( smbiosDictClass.getPlatformFeature().isDefined() ) gSettings.Smbios.gPlatformFeature = smbiosDictClass.getPlatformFeature().value();
  if ( smbiosDictClass.getBoardType().isDefined() ) gSettings.Smbios.BoardType = smbiosDictClass.getBoardType().value();
  if ( smbiosDictClass.getChassisType().isDefined() ) gSettings.Smbios.ChassisType = smbiosDictClass.getChassisType().value();
  if ( smbiosDictClass.getMobile().isDefined() ) gSettings.Smbios.Mobile = smbiosDictClass.getMobile().value();
}

void ConfigManager::applySettings() const
{
  // comes from GetDefaultSettings()
  {
    if ( !configPlist.Graphics.Inject.isInjectIntelDefined() )
    {
      gSettings.Graphics.InjectAsDict.InjectIntel =
              (gConf.GfxPropertiesArray.size() > 0 && gConf.GfxPropertiesArray[0].Vendor == Intel) ||
              (gConf.GfxPropertiesArray.size() > 1 && gConf.GfxPropertiesArray[1].Vendor == Intel);
    }
    if ( !configPlist.Graphics.Inject.isInjectATIDefined() )
    {
      gSettings.Graphics.InjectAsDict.InjectATI =
              (gConf.GfxPropertiesArray.size() > 0 && gConf.GfxPropertiesArray[0].Vendor == Ati && (gConf.GfxPropertiesArray[0].DeviceID & 0xF000) != 0x6000 )  ||
              (gConf.GfxPropertiesArray.size() > 1 && gConf.GfxPropertiesArray[1].Vendor == Ati && (gConf.GfxPropertiesArray[1].DeviceID & 0xF000) != 0x6000 );
    }
    if ( !configPlist.Graphics.Inject.isInjectNVidiaDefined() )
    {
      gSettings.Graphics.InjectAsDict.InjectNVidia =
        ( gConf.GfxPropertiesArray.isCardAtPosNvidia(0)  &&  gConf.GfxPropertiesArray[0].Family < 0xE0) ||
        ( gConf.GfxPropertiesArray.isCardAtPosNvidia(1)  &&  gConf.GfxPropertiesArray[1].Family < 0xE0);
    }
    if ( configPlist.RtVariables.dgetBooterCfgStr().isEmpty() )
    {
      CHAR8* OldCfgStr = (CHAR8*)GetNvramVariable(L"bootercfg", &gEfiAppleBootGuid, NULL, NULL);
      if ( OldCfgStr )
      {
        gSettings.RtVariables.BooterCfgStr.takeValueFrom(OldCfgStr);
        FreePool(OldCfgStr);
      }
    }
  }
  // comes from GetDefaultCpuSettings(SETTINGS_DATA& gSettings)
  {
    if ( gCPUStructure.Model >= CPU_MODEL_IVY_BRIDGE )
    {
      if ( !configPlist.ACPI.SSDT.Generate.getGeneratePStates().isDefined() )
        gSettings.ACPI.SSDT.Generate.GeneratePStates = TRUE;

      if ( !configPlist.ACPI.SSDT.Generate.getGenerateCStates().isDefined() )
        gSettings.ACPI.SSDT.Generate.GenerateCStates = TRUE;

      // backward compatibility, APFS, APLF, PluginType follow PStates
      if ( !configPlist.ACPI.SSDT.Generate.getGenerateAPSN().isDefined() )
        gSettings.ACPI.SSDT.Generate.GenerateAPSN = gSettings.ACPI.SSDT.Generate.GeneratePStates;

      if ( !configPlist.ACPI.SSDT.Generate.getGenerateAPLF().isDefined() )
        gSettings.ACPI.SSDT.Generate.GenerateAPLF = gSettings.ACPI.SSDT.Generate.GeneratePStates;

      if ( !configPlist.ACPI.SSDT.Generate.getGeneratePluginType().isDefined() )
        gSettings.ACPI.SSDT.Generate.GeneratePluginType = gSettings.ACPI.SSDT.Generate.GeneratePStates;

      if ( !configPlist.ACPI.SSDT.getEnableC6().isDefined() )
        gSettings.ACPI.SSDT._EnableC6 = TRUE;

      if ( !configPlist.ACPI.SSDT.getPluginType().isDefined() )
        gSettings.ACPI.SSDT.PluginType = 1;

      if ( gCPUStructure.Model == CPU_MODEL_IVY_BRIDGE )
      {
        if ( !configPlist.ACPI.SSDT.getMinMultiplier().isDefined() )
          gSettings.ACPI.SSDT.MinMultiplier = 7;
      }
      if ( !configPlist.ACPI.SSDT.getC3Latency().isDefined() )
        gSettings.ACPI.SSDT._C3Latency = 0x00FA;

    }
    //gSettings.CPU.Turbo                = gCPUStructure.Turbo;
    if ( gCPUStructure.Model >= CPU_MODEL_SKYLAKE_D )
    {
      if ( !configPlist.CPU.getUseARTFreq().isDefined() )
        gSettings.CPU.UseARTFreq = true;
    }
  }
  if ( gSettings.Smbios.SmUUID == nullGuidAsString )
  {
    gSettings.Smbios.SmUUID = getSmUUIDFromSmbios();
  }

  // comes from main.cpp
  {
    DBG("Calibrated TSC Frequency = %llu = %lluMHz\n", gCPUStructure.TSCCalibr, DivU64x32(gCPUStructure.TSCCalibr, Mega));
    if (gCPUStructure.TSCCalibr > 200000000ULL) {  //200MHz
      gCPUStructure.TSCFrequency = gCPUStructure.TSCCalibr;
    }
  //  DBG("print error level mask = %x\n", GetDebugPrintErrorLevel() );
    gCPUStructure.CPUFrequency = gCPUStructure.TSCFrequency;
    gCPUStructure.FSBFrequency = DivU64x32(MultU64x32(gCPUStructure.CPUFrequency, 10),
                                           (gCPUStructure.MaxRatio == 0) ? 1 : gCPUStructure.MaxRatio);
    gCPUStructure.MaxSpeed = (UINT32)DivU64x32(gCPUStructure.TSCFrequency + (Mega >> 1), Mega);

    switch (gCPUStructure.Model) {
      case CPU_MODEL_PENTIUM_M:
      case CPU_MODEL_ATOM://  Atom
      case CPU_MODEL_DOTHAN:// Pentium M, Dothan, 90nm
      case CPU_MODEL_YONAH:// Core Duo/Solo, Pentium M DC
      case CPU_MODEL_MEROM:// Core Xeon, Core 2 Duo, 65nm, Mobile
      //case CPU_MODEL_CONROE:// Core Xeon, Core 2 Duo, 65nm, Desktop like Merom but not mobile
      case CPU_MODEL_CELERON:
      case CPU_MODEL_PENRYN:// Core 2 Duo/Extreme, Xeon, 45nm , Mobile
      case CPU_MODEL_NEHALEM:// Core i7 LGA1366, Xeon 5500, "Bloomfield", "Gainstown", 45nm
      case CPU_MODEL_FIELDS:// Core i7, i5 LGA1156, "Clarksfield", "Lynnfield", "Jasper", 45nm
      case CPU_MODEL_DALES:// Core i7, i5, Nehalem
      case CPU_MODEL_CLARKDALE:// Core i7, i5, i3 LGA1156, "Westmere", "Clarkdale", , 32nm
      case CPU_MODEL_WESTMERE:// Core i7 LGA1366, Six-core, "Westmere", "Gulftown", 32nm
      case CPU_MODEL_NEHALEM_EX:// Core i7, Nehalem-Ex Xeon, "Beckton"
      case CPU_MODEL_WESTMERE_EX:// Core i7, Nehalem-Ex Xeon, "Eagleton"
        gCPUStructure.ExternalClock = (UINT32)DivU64x32(gCPUStructure.FSBFrequency + Kilo - 1, Kilo);
        //DBG(" Read TSC ExternalClock: %d MHz\n", (INT32)(DivU64x32(gCPUStructure.ExternalClock, Kilo)));
        break;
      default:
        //DBG(" Read TSC ExternalClock: %d MHz\n", (INT32)(DivU64x32(gCPUStructure.FSBFrequency, Mega)));

        // for sandy bridge or newer
        // to match ExternalClock 25 MHz like real mac, divide FSBFrequency by 4
        gCPUStructure.ExternalClock = ((UINT32)DivU64x32(gCPUStructure.FSBFrequency + Kilo - 1, Kilo) + 3) / 4;
        //DBG(" Corrected TSC ExternalClock: %d MHz\n", (INT32)(DivU64x32(gCPUStructure.ExternalClock, Kilo)));
        break;
    }
    if (gSettings.CPU.QEMU) {
  //    UINT64 Msrflex = 0ULL;

      if (!gSettings.CPU.UserChange) {
        gSettings.CPU.BusSpeed = 200000;
      }
      gCPUStructure.MaxRatio = (UINT32)DivU64x32(gCPUStructure.TSCCalibr, gSettings.CPU.BusSpeed * Kilo);
      DBG("Set MaxRatio for QEMU: %d\n", gCPUStructure.MaxRatio);
      gCPUStructure.MaxRatio *= 10;
      gCPUStructure.MinRatio = 60;
      gCPUStructure.FSBFrequency = DivU64x32(MultU64x32(gCPUStructure.CPUFrequency, 10),
                                             (gCPUStructure.MaxRatio == 0) ? 1 : gCPUStructure.MaxRatio);
      gCPUStructure.ExternalClock = (UINT32)DivU64x32(gCPUStructure.FSBFrequency + Kilo - 1, Kilo);
    }
  }


  // comes from SaveSettings()
  {
    gMobile = gSettings.Smbios.Mobile;
    if ( (gSettings.CPU.BusSpeed != 0) && (gSettings.CPU.BusSpeed > 10 * Kilo) && (gSettings.CPU.BusSpeed < 500 * Kilo) )
    {
      switch ( gCPUStructure.Model )
        {
        case CPU_MODEL_PENTIUM_M:
        case CPU_MODEL_ATOM: //  Atom
        case CPU_MODEL_DOTHAN: // Pentium M, Dothan, 90nm
        case CPU_MODEL_YONAH: // Core Duo/Solo, Pentium M DC
        case CPU_MODEL_MEROM: // Core Xeon, Core 2 Duo, 65nm, Mobile
          //case CPU_MODEL_CONROE:// Core Xeon, Core 2 Duo, 65nm, Desktop like Merom but not mobile
        case CPU_MODEL_CELERON:
        case CPU_MODEL_PENRYN: // Core 2 Duo/Extreme, Xeon, 45nm , Mobile
        case CPU_MODEL_NEHALEM: // Core i7 LGA1366, Xeon 5500, "Bloomfield", "Gainstown", 45nm
        case CPU_MODEL_FIELDS: // Core i7, i5 LGA1156, "Clarksfield", "Lynnfield", "Jasper", 45nm
        case CPU_MODEL_DALES: // Core i7, i5, Nehalem
        case CPU_MODEL_CLARKDALE: // Core i7, i5, i3 LGA1156, "Westmere", "Clarkdale", , 32nm
        case CPU_MODEL_WESTMERE: // Core i7 LGA1366, Six-core, "Westmere", "Gulftown", 32nm
        case CPU_MODEL_NEHALEM_EX: // Core i7, Nehalem-Ex Xeon, "Beckton"
        case CPU_MODEL_WESTMERE_EX: // Core i7, Nehalem-Ex Xeon, "Eagleton"
          gCPUStructure.ExternalClock = gSettings.CPU.BusSpeed;
          //DBG("Read ExternalClock: %d MHz\n", (INT32)(DivU64x32(gCPUStructure.ExternalClock, Kilo)));
          break;
        default:
          //DBG("Read ExternalClock: %d MHz\n", (INT32)(DivU64x32(gSettings.BusSpeed, Kilo)));
          // for sandy bridge or newer
          // to match ExternalClock 25 MHz like real mac, divide BusSpeed by 4
          gCPUStructure.ExternalClock = (gSettings.CPU.BusSpeed + 3) / 4;
          //DBG("Corrected ExternalClock: %d MHz\n", (INT32)(DivU64x32(gCPUStructure.ExternalClock, Kilo)));
          break;
        }
      gCPUStructure.FSBFrequency = MultU64x64(gSettings.CPU.BusSpeed, Kilo); //kHz -> Hz
      gCPUStructure.MaxSpeed = (UINT32) ((DivU64x32((UINT64) (gSettings.CPU.BusSpeed) * gCPUStructure.MaxRatio, 10000))); //kHz->MHz
    }
    if ( (gSettings.CPU.CpuFreqMHz > 100) && (gSettings.CPU.CpuFreqMHz < 20000) )
    {
      gCPUStructure.MaxSpeed = gSettings.CPU.CpuFreqMHz;
    }
    // to determine the use of Table 132
    if ( gSettings.CPU.QPI )
    {
      GlobalConfig.SetTable132 = TRUE;
      //DBG("QPI: use Table 132\n");
    } else
    {
      switch ( gCPUStructure.Model )
        {
        case CPU_MODEL_NEHALEM: // Core i7 LGA1366, Xeon 5500, "Bloomfield", "Gainstown", 45nm
        case CPU_MODEL_WESTMERE: // Core i7 LGA1366, Six-core, "Westmere", "Gulftown", 32nm
        case CPU_MODEL_NEHALEM_EX: // Core i7, Nehalem-Ex Xeon, "Beckton"
        case CPU_MODEL_WESTMERE_EX: // Core i7, Nehalem-Ex Xeon, "Eagleton"
          GlobalConfig.SetTable132 = TRUE;
          DBG("QPI: use Table 132\n");
          break;
        default:
          //DBG("QPI: disable Table 132\n");
          break;
        }
    }
    gCPUStructure.CPUFrequency = MultU64x64(gCPUStructure.MaxSpeed, Mega);
  }
}

EFI_STATUS ConfigManager::LoadConfig(const XStringW& ConfName)
{
  DbgHeader("GetUserSettings");

  if ( !selfOem.isInitialized() ) {
    log_technical_bug("%s : !selfOem.isInitialized()", __PRETTY_FUNCTION__);
  }
  EFI_STATUS Status = LoadConfigPlist(ConfName);
  if ( EFI_ERROR(Status) ) {
    DBG("LoadConfigPlist return %s. Config not loaded\n", efiStrError(Status));
    //return Status; // Let's try to continue with default values.
  }
  
  /*Status = */ LoadSMBIOSPlist(L"smbios"_XSW); // we don't need Status. If not loaded correctly, smbiosPlist is !defined and will be ignored by AssignOldNewSettings()

  MACHINE_TYPES  Model = iMac132;
  if ( smbiosPlist.SMBIOS.isDefined() && smbiosPlist.SMBIOS.hasModel()) {
    Model = smbiosPlist.SMBIOS.getModel();
  } else if ( configPlist.getSMBIOS().hasModel() ) {
    Model = configPlist.getSMBIOS().getModel();
  } else {
    Model = GetDefaultModel();
  }

  if ( !EFI_ERROR(Status) ) {
    gSettings.takeValueFrom(configPlist); // if load failed, keep default value.
  }
  // TODO improve this (avoid to delete settings to re-import them !)
  // restore default value for SMBIOS (delete values from configPlist)
  SetDMISettingsForModel(Model, &gSettings, &GlobalConfig);
  // import values from configPlist if they are defined
  FillSmbiosWithDefaultValue(Model, configPlist.getSMBIOS());
  if ( smbiosPlist.SMBIOS.isDefined() ) {
    // import values from smbiosPlist if they are defined
    FillSmbiosWithDefaultValue(Model, smbiosPlist.SMBIOS);
  }

  applySettings();
  return Status;
}

/*
 * Fill LanCardArrayNonConst with what is found through UEFI
 * LanCardArrayNonConst must be empty before clling, as there is no handling of duplicates (although it would be easy to do !)
 */
void ConfigManager::GetUEFIMacAddress()
{
  EFI_STATUS                  Status;

  if ( LanCardArrayNonConst.notEmpty() ) {
    log_technical_bug("LanCardArrayNonConst.notEmpty()"); // this function "could" be called if LanCardArrayNonConst is not empty, because it just add into the array. But that ends up in duplicates.
                                                    // Other possibility is to call setEmpty(), but that'll hide a technical bug.
  }
  //
  // Locate Service Binding handles.
  //
  UINTN NumberOfHandles = 0;
  EFI_HANDLE* HandleBuffer = NULL;
  Status = gBS->LocateHandleBuffer (
                                    ByProtocol,
                                    &gEfiDevicePathProtocolGuid,
                                    NULL,
                                    &NumberOfHandles,
                                    &HandleBuffer
                                    );
  if (EFI_ERROR(Status)) {
    return;
  }

  DbgHeader("GetUEFIMacAddress");

  for (size_t Index = 0; Index < NumberOfHandles; Index++) {
    EFI_DEVICE_PATH_PROTOCOL* Node = NULL;
    Status = gBS->HandleProtocol (
                                  HandleBuffer[Index],
                                  &gEfiDevicePathProtocolGuid,
                                  (void **) &Node
                                  );
    if (EFI_ERROR(Status)) {
      continue;
    }
    EFI_DEVICE_PATH_PROTOCOL* DevicePath = (EFI_DEVICE_PATH_PROTOCOL *) Node;


    while (!IsDevicePathEnd (DevicePath)) {
      if ((DevicePathType (DevicePath) == MESSAGING_DEVICE_PATH) &&
          (DevicePathSubType (DevicePath) == MSG_MAC_ADDR_DP)) {

        //
        // Get MAC address.
        //
        MAC_ADDR_DEVICE_PATH* MacAddressNode = (MAC_ADDR_DEVICE_PATH*)DevicePath;
        if ( !LanCardArrayNonConst.containsMacAddress(&MacAddressNode->MacAddress.Addr[0]) ) {
          LanCardClass* lanPath = new LanCardClass;
          CopyMem(&lanPath->MacAddress, &MacAddressNode->MacAddress.Addr[0], sizeof(lanPath->MacAddress));
          DBG("UEFI MAC address of %ls LAN #%zu= ", DevicePathToXStringW(DevicePath).wc_str(), LanCardArrayNonConst.size());
          for (size_t Index2 = 0; Index2 < sizeof(lanPath->MacAddress); Index2++) {
            DBG("%02hhX:", lanPath->MacAddress[Index2]);
          }
          DBG("\n");
          LanCardArrayNonConst.AddReference(lanPath, true);
        }
      }
      DevicePath = NextDevicePathNode (DevicePath);
    }
  }
  if (HandleBuffer != NULL) {
    FreePool(HandleBuffer);
  }
}

EFI_STATUS ConfigManager::ReLoadConfig(const XStringW& ConfName)
{
  /* I'm pretty sure, one day, there will be other things to do than just LoadConfig */
  return LoadConfig(ConfName);
}

EFI_STATUS ConfigManager::InitialisePlatform()
{
  EFI_STATUS Status;

  PrepatchSmbios(&g_SmbiosDiscoveredSettings);
  GlobalConfig.OEMBoardFromSmbios = g_SmbiosDiscoveredSettings.OEMBoardFromSmbios;
  GlobalConfig.OEMProductFromSmbios = g_SmbiosDiscoveredSettings.OEMProductFromSmbios;
  GlobalConfig.OEMVendorFromSmbios = g_SmbiosDiscoveredSettings.OEMVendorFromSmbios;

  //replace / with _
  GlobalConfig.OEMProductFromSmbios.replaceAll(U'/', U'_');
  GlobalConfig.OEMBoardFromSmbios.replaceAll(U'/', U'_');
  DBG("Running on: '%s' with board '%s'\n", GlobalConfig.OEMProductFromSmbios.c_str(), GlobalConfig.OEMBoardFromSmbios.c_str());

  gCPUStructure.ExternalClock = g_SmbiosDiscoveredSettings.ExternalClock;
  gCPUStructure.CurrentSpeed = g_SmbiosDiscoveredSettings.CurrentSpeed;
  gCPUStructure.MaxSpeed = g_SmbiosDiscoveredSettings.MaxSpeed;

  GetCPUProperties();
  DiscoverDevices();

  //SavingMode

  if ( g_SmbiosDiscoveredSettings.EnabledCores ) {
    GlobalConfig.EnabledCores = g_SmbiosDiscoveredSettings.EnabledCores;
  }else{
    GlobalConfig.EnabledCores = gCPUStructure.Cores;
  }

  selfOem.initialize("config"_XS8, gFirmwareClover, GlobalConfig.OEMBoardFromSmbios, GlobalConfig.OEMProductFromSmbios, (INT32)(DivU64x32(gCPUStructure.CPUFrequency, Mega)), gConf.LanCardArray);
  Status = gConf.LoadConfig(L"config"_XSW);

  GlobalConfig.C3Latency = gSettings.ACPI.SSDT._C3Latency;
  GlobalConfig.KPKernelPm = gSettings.KernelAndKextPatches._KPKernelPm;

  for ( size_t idx = 0 ; idx < GfxPropertiesArrayNonConst.size() ; ++idx ) {
    GfxPropertiesArrayNonConst[idx].LoadVBios = gSettings.Graphics.LoadVBios;
  }

  if (gSettings.Devices.Audio.ResetHDA) ResetHDA();

#ifdef ENABLE_SECURE_BOOT
  InitializeSecureBoot();
#endif // ENABLE_SECURE_BOOT

  return Status;
}

ConfigManager gConf;
