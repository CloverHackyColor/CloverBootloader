///** @file DevicePath.c
//  BDS internal function define the default device path string, it can be
//  replaced by platform device path.
//
//Copyright (c) 2004 - 2009, Intel Corporation. All rights reserved.<BR>
//This program and the accompanying materials
//are licensed and made available under the terms and conditions of the BSD License
//which accompanies this distribution.  The full text of the license may be found at
//http://opensource.org/licenses/bsd-license.php
//
//THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
//WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
//
//**/
//
//#include "Platform.h"
////#include <Library/GenericBdsLib.h>
////#include "InternalBdsLib.h"
//
///**
//  Concatenates a formatted unicode string to allocated pool.
//  The caller must free the resulting buffer.
//
//  @param  Str      Tracks the allocated pool, size in use, and amount of pool allocated.
//  @param  Fmt      The format string
//  @param  ...      The data will be printed.
//
//  @return Allocated buffer with the formatted string printed in it.
//          The caller must free the allocated buffer.
//          The buffer allocation is not packed.
//
//**/
//CHAR16 *
//EFIAPI
//CatPrint (
//  IN OUT POOL_PRINT   *Str,
//  IN CONST CHAR16           *Fmt,
//  ...
//  )
//{
//  CHAR16  *AppendStr;
//  VA_LIST Args;
//  UINTN   StringSize;
//
//  AppendStr = (__typeof__(AppendStr))AllocateZeroPool (0x1000);
//  if (AppendStr == NULL) {
//    return Str->Str;
//  }
//
//  VA_START (Args, Fmt);
//  UnicodeVSPrint (AppendStr, 0x1000, Fmt, Args);
//  VA_END (Args);
//  if (NULL == Str->Str) {
//    StringSize   = StrSize (AppendStr);
//    Str->Str = (__typeof__(Str->Str))AllocateZeroPool (StringSize);
////    ASSERT (Str->Str != NULL);
//  } else {
//    StringSize = StrSize (AppendStr);
//    StringSize += (StrSize (Str->Str) - sizeof (UINT16));
//
//    Str->Str = (__typeof__(Str->Str))ReallocatePool (
//                StrSize (Str->Str),
//                StringSize,
//                Str->Str
//                );
////    ASSERT (Str->Str != NULL);
//  }
//
//  Str->Maxlen = MAX_CHAR * sizeof (UINT16);
//  if (StringSize < Str->Maxlen) {
//    StrCatS (Str->Str, StringSize, AppendStr);
//    Str->Len = StringSize - sizeof (UINT16);
//  }
//
//  FreePool (AppendStr);
//  return Str->Str;
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathPci (
//  IN OUT POOL_PRINT       *Str,
//  IN PCI_DEVICE_PATH                 *DevPath
//  )
//{
//  PCI_DEVICE_PATH *Pci;
//
//  Pci = DevPath;
//  CatPrint (Str, L"Pci(%x|%x)", (UINTN) Pci->Device, (UINTN) Pci->Function);
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathPccard (
//  IN OUT POOL_PRINT       *Str,
//  IN PCCARD_DEVICE_PATH                 *DevPath
//  )
//{
//  PCCARD_DEVICE_PATH  *Pccard;
//
//  Pccard = DevPath;
//  CatPrint (Str, L"Pcmcia(Function%x)", (UINTN) Pccard->FunctionNumber);
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathMemMap (
//  IN OUT POOL_PRINT       *Str,
//  IN MEMMAP_DEVICE_PATH                 *DevPath
//  )
//{
//  MEMMAP_DEVICE_PATH  *MemMap;
//
//  MemMap = DevPath;
//  CatPrint (
//    Str,
//    L"MemMap(%d:%lx-%lx)",
//    (UINTN) MemMap->MemoryType,
//    MemMap->StartingAddress,
//    MemMap->EndingAddress
//    );
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathController (
//  IN OUT POOL_PRINT       *Str,
//  IN CONTROLLER_DEVICE_PATH                 *DevPath
//  )
//{
//  CONTROLLER_DEVICE_PATH  *Controller;
//
//  Controller = DevPath;
//  CatPrint (Str, L"Ctrl(%d)", (UINTN) Controller->ControllerNumber);
//}
//
//
///**
//  Convert Vendor device path to device name.
//
//  @param  Str      The buffer store device name
//  @param  DevPath  Pointer to vendor device path
//
//**/
//VOID
//DevPathVendor (
//  IN OUT POOL_PRINT       *Str,
//  IN VENDOR_DEVICE_PATH                 *DevPath
//  )
//{
//  VENDOR_DEVICE_PATH  *Vendor;
//  CONST CHAR16              *Type;
//  UINTN               DataLength;
//  UINTN               Index;
////  UINT32              FlowControlMap;
//
//  UINT16              Info;
//
//  Vendor  = DevPath;
//
//  switch (DevicePathType (&Vendor->Header)) {
//  case HARDWARE_DEVICE_PATH:
//    Type = L"Hw";
//    break;
//
//  case MESSAGING_DEVICE_PATH:
//    Type = L"Msg";
// /*   if (CompareGuid (&Vendor->Guid, &gEfiPcAnsiGuid)) {
//      CatPrint (Str, L"VenPcAnsi()");
//      return ;
//    } else if (CompareGuid (&Vendor->Guid, &gEfiVT100Guid)) {
//      CatPrint (Str, L"VenVt100()");
//      return ;
//    } else if (CompareGuid (&Vendor->Guid, &gEfiVT100PlusGuid)) {
//      CatPrint (Str, L"VenVt100Plus()");
//      return ;
//    } else if (CompareGuid (&Vendor->Guid, &gEfiVTUTF8Guid)) {
//      CatPrint (Str, L"VenUft8()");
//      return ;
//    } else if (CompareGuid (&Vendor->Guid, &gEfiUartDevicePathGuid     )) {
//      FlowControlMap = (((UART_FLOW_CONTROL_DEVICE_PATH *) Vendor)->FlowControlMap);
//      switch (FlowControlMap & 0x00000003) {
//      case 0:
//        CatPrint (Str, L"UartFlowCtrl(%s)", L"None");
//        break;
//
//      case 1:
//        CatPrint (Str, L"UartFlowCtrl(%s)", L"Hardware");
//        break;
//
//      case 2:
//        CatPrint (Str, L"UartFlowCtrl(%s)", L"XonXoff");
//        break;
//
//      default:
//        break;
//      }
//
//      return ;
//
//    } else
//*/
//	if (CompareGuid (&Vendor->Guid, &gEfiSasDevicePathGuid)) {
//      CatPrint (
//        Str,
//        L"SAS(%lx,%lx,%x,",
//        ((SAS_DEVICE_PATH *) Vendor)->SasAddress,
//        ((SAS_DEVICE_PATH *) Vendor)->Lun,
//        (UINTN) ((SAS_DEVICE_PATH *) Vendor)->RelativeTargetPort
//        );
//      Info = (((SAS_DEVICE_PATH *) Vendor)->DeviceTopology);
//      if ((Info & 0x0f) == 0) {
//        CatPrint (Str, L"NoTopology,0,0,0,");
//      } else if (((Info & 0x0f) == 1) || ((Info & 0x0f) == 2)) {
//        CatPrint (
//          Str,
//          L"%s,%s,%s,",
//          ((Info & (0x1 << 4)) != 0) ? L"SATA" : L"SAS",
//          ((Info & (0x1 << 5)) != 0) ? L"External" : L"Internal",
//          ((Info & (0x1 << 6)) != 0) ? L"Expanded" : L"Direct"
//          );
//        if ((Info & 0x0f) == 1) {
//          CatPrint (Str, L"0,");
//        } else {
//          CatPrint (Str, L"%x,", (UINTN) ((Info >> 8) & 0xff));
//        }
//      } else {
//        CatPrint (Str, L"0,0,0,0,");
//      }
//
//      CatPrint (Str, L"%x)", (UINTN) ((SAS_DEVICE_PATH *) Vendor)->Reserved);
//      return ;
//
//    } else if (CompareGuid (&Vendor->Guid, &gEfiDebugPortProtocolGuid)) {
//      CatPrint (Str, L"DebugPort()");
//      return ;
//    }
//    break;
//
//  case MEDIA_DEVICE_PATH:
//    Type = L"Media";
//    break;
//
//  default:
//    Type = L"?";
//    break;
//  }
//
//  CatPrint (Str, L"Ven%s(%g", Type, &Vendor->Guid);
//  DataLength = DevicePathNodeLength (&Vendor->Header) - sizeof (VENDOR_DEVICE_PATH);
//  if (DataLength > 0) {
//    CatPrint (Str, L",");
//    for (Index = 0; Index < DataLength; Index++) {
//      CatPrint (Str, L"%02x", (UINTN) ((VENDOR_DEVICE_PATH_WITH_DATA *) Vendor)->VendorDefinedData[Index]);
//    }
//  }
//  CatPrint (Str, L")");
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathAcpi (
//  IN OUT POOL_PRINT       *Str,
//  IN ACPI_HID_DEVICE_PATH                 *DevPath
//  )
//{
//  ACPI_HID_DEVICE_PATH  *Acpi;
//
//  Acpi = DevPath;
//  if ((Acpi->HID & PNP_EISA_ID_MASK) == PNP_EISA_ID_CONST) {
//    CatPrint (Str, L"Acpi(PNP%04x,%x)", (UINTN)  EISA_ID_TO_NUM (Acpi->HID), (UINTN) Acpi->UID);
//  } else {
//    CatPrint (Str, L"Acpi(%08x,%x)", (UINTN) Acpi->HID, (UINTN) Acpi->UID);
//  }
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathExtendedAcpi (
//  IN OUT POOL_PRINT       *Str,
//  IN ACPI_EXTENDED_HID_DEVICE_PATH                 *DevPath
//  )
//{
//  ACPI_EXTENDED_HID_DEVICE_PATH   *ExtendedAcpi;
//
//  //
//  // Index for HID, UID and CID strings, 0 for non-exist
//  //
//  UINT16                          HIDSTRIdx;
//  UINT16                          UIDSTRIdx;
//  UINT16                          CIDSTRIdx;
//  UINT16                          Index;
//  UINT16                          Length;
//  UINT16                          Anchor;
//  CHAR8                           *AsChar8Array;
//
//  HIDSTRIdx    = 0;
//  UIDSTRIdx    = 0;
//  CIDSTRIdx    = 0;
//  ExtendedAcpi = DevPath;
//  Length       = (UINT16) DevicePathNodeLength ((EFI_DEVICE_PATH_PROTOCOL *) ExtendedAcpi);
//
//  AsChar8Array = (CHAR8 *) ExtendedAcpi;
//
//  //
//  // find HIDSTR
//  //
//  Anchor = 16;
//  for (Index = Anchor; Index < Length && AsChar8Array[Index] != '\0'; Index++) {
//    ;
//  }
//  if (Index > Anchor) {
//    HIDSTRIdx = Anchor;
//  }
//  //
//  // find UIDSTR
//  //
//  Anchor = (UINT16) (Index + 1);
//  for (Index = Anchor; Index < Length && AsChar8Array[Index] != '\0'; Index++) {
//    ;
//  }
//  if (Index > Anchor) {
//    UIDSTRIdx = Anchor;
//  }
//  //
//  // find CIDSTR
//  //
//  Anchor = (UINT16) (Index + 1);
//  for (Index = Anchor; Index < Length && AsChar8Array[Index] != '\0'; Index++) {
//    ;
//  }
//  if (Index > Anchor) {
//    CIDSTRIdx = Anchor;
//  }
//
//  if (HIDSTRIdx == 0 && CIDSTRIdx == 0 && ExtendedAcpi->UID == 0) {
//    CatPrint (Str, L"AcpiExp(");
//    if ((ExtendedAcpi->HID & PNP_EISA_ID_MASK) == PNP_EISA_ID_CONST) {
//      CatPrint (Str, L"PNP%04x,", (UINTN) EISA_ID_TO_NUM (ExtendedAcpi->HID));
//    } else {
//      CatPrint (Str, L"%08x,", (UINTN) ExtendedAcpi->HID);
//    }
//    if ((ExtendedAcpi->CID & PNP_EISA_ID_MASK) == PNP_EISA_ID_CONST) {
//      CatPrint (Str, L"PNP%04x,", (UINTN)  EISA_ID_TO_NUM (ExtendedAcpi->CID));
//    } else {
//      CatPrint (Str, L"%08x,", (UINTN)  ExtendedAcpi->CID);
//    }
//    if (UIDSTRIdx != 0) {
//      CatPrint (Str, L"%a)", AsChar8Array + UIDSTRIdx);
//    } else {
//      CatPrint (Str, L"\"\")");
//    }
//  } else {
//    CatPrint (Str, L"AcpiEx(");
//    if ((ExtendedAcpi->HID & PNP_EISA_ID_MASK) == PNP_EISA_ID_CONST) {
//      CatPrint (Str, L"PNP%04x,", (UINTN)  EISA_ID_TO_NUM (ExtendedAcpi->HID));
//    } else {
//      CatPrint (Str, L"%08x,", (UINTN) ExtendedAcpi->HID);
//    }
//    if ((ExtendedAcpi->CID & PNP_EISA_ID_MASK) == PNP_EISA_ID_CONST) {
//      CatPrint (Str, L"PNP%04x,", (UINTN) EISA_ID_TO_NUM (ExtendedAcpi->CID));
//    } else {
//      CatPrint (Str, L"%08x,", (UINTN) ExtendedAcpi->CID);
//    }
//    CatPrint (Str, L"%x,", (UINTN) ExtendedAcpi->UID);
//
//    if (HIDSTRIdx != 0) {
//      CatPrint (Str, L"%a,", AsChar8Array + HIDSTRIdx);
//    } else {
//      CatPrint (Str, L"\"\",");
//    }
//    if (CIDSTRIdx != 0) {
//      CatPrint (Str, L"%a,", AsChar8Array + CIDSTRIdx);
//    } else {
//      CatPrint (Str, L"\"\",");
//    }
//    if (UIDSTRIdx != 0) {
//      CatPrint (Str, L"%a)", AsChar8Array + UIDSTRIdx);
//    } else {
//      CatPrint (Str, L"\"\")");
//    }
//  }
//
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathAdrAcpi (
//  IN OUT POOL_PRINT       *Str,
//  IN ACPI_ADR_DEVICE_PATH                 *DevPath
//  )
//{
//  ACPI_ADR_DEVICE_PATH    *AcpiAdr;
//  UINT16                  Index;
//  UINT16                  Length;
//  UINT16                  AdditionalAdrCount;
//
//  AcpiAdr            = DevPath;
//  Length             = (UINT16) DevicePathNodeLength ((EFI_DEVICE_PATH_PROTOCOL *) AcpiAdr);
//  AdditionalAdrCount = (UINT16) ((Length - 8) / 4);
//
//  CatPrint (Str, L"AcpiAdr(%x", (UINTN) AcpiAdr->ADR);
//  for (Index = 0; Index < AdditionalAdrCount; Index++) {
//    CatPrint (Str, L",%x", (UINTN) *(UINT32 *) ((UINT8 *) AcpiAdr + 8 + Index * 4));
//  }
//  CatPrint (Str, L")");
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathAtapi (
//  IN OUT POOL_PRINT       *Str,
//  IN ATAPI_DEVICE_PATH                 *DevPath
//  )
//{
//  ATAPI_DEVICE_PATH *Atapi;
//
//  Atapi = DevPath;
//  CatPrint (
//    Str,
//    L"Ata(%s,%s)",
//    (Atapi->PrimarySecondary != 0)? L"Secondary" : L"Primary",
//    (Atapi->SlaveMaster != 0)? L"Slave" : L"Master"
//    );
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathScsi (
//  IN OUT POOL_PRINT       *Str,
//  IN SCSI_DEVICE_PATH                 *DevPath
//  )
//{
//  SCSI_DEVICE_PATH  *Scsi;
//
//  Scsi = DevPath;
//  CatPrint (Str, L"Scsi(Pun%x,Lun%x)", (UINTN) Scsi->Pun, (UINTN) Scsi->Lun);
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathFibre (
//  IN OUT POOL_PRINT       *Str,
//  IN FIBRECHANNEL_DEVICE_PATH                 *DevPath
//  )
//{
//  FIBRECHANNEL_DEVICE_PATH  *Fibre;
//
//  Fibre = DevPath;
//  CatPrint (Str, L"Fibre(Wwn%lx,Lun%x)", Fibre->WWN, Fibre->Lun);
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPath1394 (
//  IN OUT POOL_PRINT       *Str,
//  IN F1394_DEVICE_PATH                 *DevPath
//  )
//{
//  F1394_DEVICE_PATH *F1394Path;
//
//  F1394Path = DevPath;
//  CatPrint (Str, L"1394(%lx)", &F1394Path->Guid);
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathUsb (
//  IN OUT POOL_PRINT       *Str,
//  IN USB_DEVICE_PATH                 *DevPath
//  )
//{
//  USB_DEVICE_PATH *Usb;
//
//  Usb = DevPath;
//  CatPrint (Str, L"Usb(%x,%x)", (UINTN) Usb->ParentPortNumber, (UINTN) Usb->InterfaceNumber);
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathUsbWWID (
//  IN OUT POOL_PRINT       *Str,
//  IN USB_WWID_DEVICE_PATH                 *DevPath
//  )
//{
//  USB_WWID_DEVICE_PATH  *UsbWWId;
//
//  UsbWWId = DevPath;
//  CatPrint (
//    Str,
//    L"UsbWwid(%x,%x,%x,\"WWID\")",
//    (UINTN) UsbWWId->VendorId,
//    (UINTN) UsbWWId->ProductId,
//    (UINTN) UsbWWId->InterfaceNumber
//    );
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathLogicalUnit (
//  IN OUT POOL_PRINT       *Str,
//  IN DEVICE_LOGICAL_UNIT_DEVICE_PATH                 *DevPath
//  )
//{
//  DEVICE_LOGICAL_UNIT_DEVICE_PATH *LogicalUnit;
//
//  LogicalUnit = DevPath;
//  CatPrint (Str, L"Unit(%x)", (UINTN) LogicalUnit->Lun);
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathUsbClass (
//  IN OUT POOL_PRINT       *Str,
//  IN USB_CLASS_DEVICE_PATH                 *DevPath
//  )
//{
//  USB_CLASS_DEVICE_PATH *UsbClass;
//
//  UsbClass = DevPath;
//  CatPrint (
//    Str,
//    L"Usb Class(%x,%x,%x,%x,%x)",
//    (UINTN) UsbClass->VendorId,
//    (UINTN) UsbClass->ProductId,
//    (UINTN) UsbClass->DeviceClass,
//    (UINTN) UsbClass->DeviceSubClass,
//    (UINTN) UsbClass->DeviceProtocol
//    );
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathSata (
//  IN OUT POOL_PRINT       *Str,
//  IN SATA_DEVICE_PATH                 *DevPath
//  )
//{
//  SATA_DEVICE_PATH *Sata;
//
//  Sata = DevPath;
//  if ((Sata->PortMultiplierPortNumber & SATA_HBA_DIRECT_CONNECT_FLAG) != 0) {
//    CatPrint (
//      Str,
//      L"Sata(%x,%x)",
//      (UINTN) Sata->HBAPortNumber,
//      (UINTN) Sata->Lun
//      );
//  } else {
//    CatPrint (
//      Str,
//      L"Sata(%x,%x,%x)",
//      (UINTN) Sata->HBAPortNumber,
//      (UINTN) Sata->PortMultiplierPortNumber,
//      (UINTN) Sata->Lun
//      );
//  }
//}
//
///**
// Converts a NVM Express Namespace device path structure to its string representative.
//
// @param Str             The string representative of input device.
// @param DevPath         The input device path structure.
// @param DisplayOnly     If DisplayOnly is TRUE, then the shorter text representation
// of the display node is used, where applicable. If DisplayOnly
// is FALSE, then the longer text representation of the display node
// is used.
// @param AllowShortcuts  If AllowShortcuts is TRUE, then the shortcut forms of text
// representation for a device node can be used, where applicable.
//
// **/
//VOID
//DevPathToTextNVMe (
//                   IN OUT POOL_PRINT  *Str,
//                   IN NVME_NAMESPACE_DEVICE_PATH            *DevPath /*,
//                   IN BOOLEAN         DisplayOnly,
//                   IN BOOLEAN         AllowShortcuts */
//                   )
//{
//  NVME_NAMESPACE_DEVICE_PATH *Nvme;
//  UINT8                      *Uuid;
//
//  Nvme = DevPath;
//  Uuid = (UINT8 *) &Nvme->NamespaceUuid;
//  CatPrint (
//            Str,
//            L"NVMe(0x%x,%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x)",
//            Nvme->NamespaceId,
//            Uuid[7], Uuid[6], Uuid[5], Uuid[4],
//            Uuid[3], Uuid[2], Uuid[1], Uuid[0]
//            );
//}
//
//
//
///**
// Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathI2O (
//  IN OUT POOL_PRINT       *Str,
//  IN I2O_DEVICE_PATH                 *DevPath
//  )
//{
//  I2O_DEVICE_PATH *I2OPath;
//
//  I2OPath = DevPath;
//  CatPrint (Str, L"I2O(%x)", (UINTN) I2OPath->Tid);
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathMacAddr (
//  IN OUT POOL_PRINT       *Str,
//  IN MAC_ADDR_DEVICE_PATH                 *DevPath
//  )
//{
//  MAC_ADDR_DEVICE_PATH  *MACDevPath;
//  UINTN                 HwAddressSize;
//  UINTN                 Index;
//
//  MACDevPath           = DevPath;
//
//  HwAddressSize = sizeof (EFI_MAC_ADDRESS);
//  if (MACDevPath->IfType == 0x01 || MACDevPath->IfType == 0x00) {
//    HwAddressSize = 6;
//  }
//
//  CatPrint (Str, L"Mac(");
//
//  for (Index = 0; Index < HwAddressSize; Index++) {
//    CatPrint (Str, L"%02x", (UINTN) MACDevPath->MacAddress.Addr[Index]);
//  }
//
//  CatPrint (Str, L")");
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathIPv4 (
//  IN OUT POOL_PRINT       *Str,
//  IN IPv4_DEVICE_PATH                 *DevPath
//  )
//{
//  IPv4_DEVICE_PATH  *IPDevPath;
//
//  IPDevPath = DevPath;
//  CatPrint (
//    Str,
//    L"IPv4(%d.%d.%d.%d:%d)",
//    (UINTN) IPDevPath->RemoteIpAddress.Addr[0],
//    (UINTN) IPDevPath->RemoteIpAddress.Addr[1],
//    (UINTN) IPDevPath->RemoteIpAddress.Addr[2],
//    (UINTN) IPDevPath->RemoteIpAddress.Addr[3],
//    (UINTN) IPDevPath->RemotePort
//    );
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathIPv6 (
//  IN OUT POOL_PRINT       *Str,
//  IN IPv6_DEVICE_PATH                 *DevPath
//  )
//{
//  IPv6_DEVICE_PATH  *IPv6DevPath;
//
//  IPv6DevPath = DevPath;
//  CatPrint (
//    Str,
//    L"IPv6(%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x)",
//    (UINTN) IPv6DevPath->RemoteIpAddress.Addr[0],
//    (UINTN) IPv6DevPath->RemoteIpAddress.Addr[1],
//    (UINTN) IPv6DevPath->RemoteIpAddress.Addr[2],
//    (UINTN) IPv6DevPath->RemoteIpAddress.Addr[3],
//    (UINTN) IPv6DevPath->RemoteIpAddress.Addr[4],
//    (UINTN) IPv6DevPath->RemoteIpAddress.Addr[5],
//    (UINTN) IPv6DevPath->RemoteIpAddress.Addr[6],
//    (UINTN) IPv6DevPath->RemoteIpAddress.Addr[7],
//    (UINTN) IPv6DevPath->RemoteIpAddress.Addr[8],
//    (UINTN) IPv6DevPath->RemoteIpAddress.Addr[9],
//    (UINTN) IPv6DevPath->RemoteIpAddress.Addr[10],
//    (UINTN) IPv6DevPath->RemoteIpAddress.Addr[11],
//    (UINTN) IPv6DevPath->RemoteIpAddress.Addr[12],
//    (UINTN) IPv6DevPath->RemoteIpAddress.Addr[13],
//    (UINTN) IPv6DevPath->RemoteIpAddress.Addr[14],
//    (UINTN) IPv6DevPath->RemoteIpAddress.Addr[15]
//    );
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathInfiniBand (
//  IN OUT POOL_PRINT       *Str,
//  IN INFINIBAND_DEVICE_PATH                 *DevPath
//  )
//{
//  INFINIBAND_DEVICE_PATH  *InfiniBand;
//
//  InfiniBand = DevPath;
//  CatPrint (
//    Str,
//    L"Infiniband(%x,%g,%lx,%lx,%lx)",
//    (UINTN) InfiniBand->ResourceFlags,
//    InfiniBand->PortGid,
//    InfiniBand->ServiceId,
//    InfiniBand->TargetPortId,
//    InfiniBand->DeviceId
//    );
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathUart (
//  IN OUT POOL_PRINT       *Str,
//  IN UART_DEVICE_PATH                 *DevPath
//  )
//{
//  UART_DEVICE_PATH  *Uart;
//  CHAR8             Parity;
//
//  Uart = DevPath;
//  switch (Uart->Parity) {
//  case 0:
//    Parity = 'D';
//    break;
//
//  case 1:
//    Parity = 'N';
//    break;
//
//  case 2:
//    Parity = 'E';
//    break;
//
//  case 3:
//    Parity = 'O';
//    break;
//
//  case 4:
//    Parity = 'M';
//    break;
//
//  case 5:
//    Parity = 'S';
//    break;
//
//  default:
//    Parity = 'x';
//    break;
//  }
//
//  if (Uart->BaudRate == 0) {
//    CatPrint (Str, L"Uart(DEFAULT,%c,", Parity);
//  } else {
//    CatPrint (Str, L"Uart(%ld,%c,", Uart->BaudRate, Parity);
//  }
//
//  if (Uart->DataBits == 0) {
//    CatPrint (Str, L"D,");
//  } else {
//    CatPrint (Str, L"%d,", (UINTN) Uart->DataBits);
//  }
//
//  switch (Uart->StopBits) {
//  case 0:
//    CatPrint (Str, L"D)");
//    break;
//
//  case 1:
//    CatPrint (Str, L"1)");
//    break;
//
//  case 2:
//    CatPrint (Str, L"1.5)");
//    break;
//
//  case 3:
//    CatPrint (Str, L"2)");
//    break;
//
//  default:
//    CatPrint (Str, L"x)");
//    break;
//  }
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathiSCSI (
//  IN OUT POOL_PRINT       *Str,
//  IN ISCSI_DEVICE_PATH_WITH_NAME                 *DevPath
//  )
//{
//  ISCSI_DEVICE_PATH_WITH_NAME *IScsi;
//  UINT16                      Options;
//
//  IScsi = DevPath;
//  CatPrint (
//    Str,
//    L"iSCSI(%a,%x,%lx,",
//    IScsi->TargetName,
//    (UINTN) IScsi->TargetPortalGroupTag,
//    IScsi->Lun
//    );
//
//  Options = IScsi->LoginOption;
//  CatPrint (Str, L"%s,", (((Options >> 1) & 0x0001) != 0) ? L"CRC32C" : L"None");
//  CatPrint (Str, L"%s,", (((Options >> 3) & 0x0001) != 0) ? L"CRC32C" : L"None");
//  if (((Options >> 11) & 0x0001) != 0) {
//    CatPrint (Str, L"%s,", L"None");
//  } else if (((Options >> 12) & 0x0001) != 0) {
//    CatPrint (Str, L"%s,", L"CHAP_UNI");
//  } else {
//    CatPrint (Str, L"%s,", L"CHAP_BI");
//
//  }
//
//  CatPrint (Str, L"%s)", (IScsi->NetworkProtocol == 0) ? L"TCP" : L"reserved");
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maixmum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathVlan (
//  IN OUT POOL_PRINT       *Str,
//  IN VLAN_DEVICE_PATH                 *DevPath
//  )
//{
//  VLAN_DEVICE_PATH  *Vlan;
//
//  Vlan = DevPath;
//  CatPrint (Str, L"Vlan(%d)", (UINTN) Vlan->VlanId);
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maixmum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathHardDrive (
//  IN OUT POOL_PRINT       *Str,
//  IN HARDDRIVE_DEVICE_PATH                 *DevPath
//  )
//{
//  HARDDRIVE_DEVICE_PATH *Hd;
//
//  Hd = DevPath;
//  switch (Hd->SignatureType) {
//  case SIGNATURE_TYPE_MBR:
//    CatPrint (
//      Str,
//      L"HD(Part%d,Sig%08x)",
//      (UINTN) Hd->PartitionNumber,
//      (UINTN) *((UINT32 *) (&(Hd->Signature[0])))
//      );
//    break;
//
//  case SIGNATURE_TYPE_GUID:
//    CatPrint (
//      Str,
//      L"HD(Part%d,Sig%g)",
//      (UINTN) Hd->PartitionNumber,
//      (EFI_GUID *) &(Hd->Signature[0])
//      );
//    break;
//
//  default:
//    CatPrint (
//      Str,
//      L"HD(Part%d,MBRType=%02x,SigType=%02x)",
//      (UINTN) Hd->PartitionNumber,
//      (UINTN) Hd->MBRType,
//      (UINTN) Hd->SignatureType
//      );
//    break;
//  }
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathCDROM (
//  IN OUT POOL_PRINT       *Str,
//  IN CDROM_DEVICE_PATH                 *DevPath
//  )
//{
//  CDROM_DEVICE_PATH *Cd;
//
//  Cd = DevPath;
//  CatPrint (Str, L"CDROM(Entry%x)", (UINTN) Cd->BootEntry);
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathFilePath (
//  IN OUT POOL_PRINT       *Str,
//  IN FILEPATH_DEVICE_PATH                 *DevPath
//  )
//{
//  FILEPATH_DEVICE_PATH  *Fp;
//
//  Fp = DevPath;
//  CatPrint (Str, L"%s", Fp->PathName);
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathMediaProtocol (
//  IN OUT POOL_PRINT       *Str,
//  IN MEDIA_PROTOCOL_DEVICE_PATH                 *DevPath
//  )
//{
//  MEDIA_PROTOCOL_DEVICE_PATH  *MediaProt;
//
//  MediaProt = DevPath;
//  CatPrint (Str, L"Media(%g)", &MediaProt->Protocol);
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathFvFilePath (
//  IN OUT POOL_PRINT       *Str,
//  IN MEDIA_FW_VOL_FILEPATH_DEVICE_PATH                 *DevPath
//  )
//{
//  MEDIA_FW_VOL_FILEPATH_DEVICE_PATH *FvFilePath;
//
//  FvFilePath = DevPath;
//  CatPrint (Str, L"%g", &FvFilePath->FvFileName);
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathRelativeOffsetRange0 (
//  IN OUT POOL_PRINT       *Str,
//  IN MEDIA_RELATIVE_OFFSET_RANGE_DEVICE_PATH                 *DevPath
//  )
//{
//  MEDIA_RELATIVE_OFFSET_RANGE_DEVICE_PATH *Offset;
//
//  Offset = DevPath;
//  CatPrint (
//    Str,
//    L"Offset(%lx,%lx)",
//    Offset->StartingOffset,
//    Offset->EndingOffset
//    );
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathBssBss (
//  IN OUT POOL_PRINT       *Str,
//  IN BBS_BBS_DEVICE_PATH                 *DevPath
//  )
//{
//  BBS_BBS_DEVICE_PATH *Bbs;
//  CONST CHAR16              *Type;
//
//  Bbs = DevPath;
//  switch (Bbs->DeviceType) {
//  case BBS_TYPE_FLOPPY:
//    Type = L"Floppy";
//    break;
//
//  case BBS_TYPE_HARDDRIVE:
//    Type = L"Harddrive";
//    break;
//
//  case BBS_TYPE_CDROM:
//    Type = L"CDROM";
//    break;
//
//  case BBS_TYPE_PCMCIA:
//    Type = L"PCMCIA";
//    break;
//
//  case BBS_TYPE_USB:
//    Type = L"Usb";
//    break;
//
//  case BBS_TYPE_EMBEDDED_NETWORK:
//    Type = L"Net";
//    break;
//
//  case BBS_TYPE_BEV:
//    Type = L"BEV";
//    break;
//
//  default:
//    Type = L"?";
//    break;
//  }
//  CatPrint (Str, L"Legacy-%s", Type);
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathEndInstance (
//  IN OUT POOL_PRINT       *Str,
//  IN VOID                 *DevPath
//  )
//{
//  CatPrint (Str, L",");
//}
//
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathNodeUnknown (
//  IN OUT POOL_PRINT       *Str,
//  IN VOID                 *DevPath
//  )
//{
//  CatPrint (Str, L"?");
//}
///**
//  Convert Device Path to a Unicode string for printing.
//
//  @param Str             The buffer holding the output string.
//                         This buffer contains the length of the
//                         string and the maximum length reserved
//                         for the string buffer.
//  @param DevPath         The device path.
//
//**/
//static VOID
//DevPathFvPath (
//  IN OUT POOL_PRINT       *Str,
//  IN MEDIA_FW_VOL_DEVICE_PATH                 *DevPath
//  )
//{
//  MEDIA_FW_VOL_DEVICE_PATH *FvPath;
//
//  FvPath = DevPath;
//  CatPrint (Str, L"Fv(%g)", &FvPath->FvName);
//}
//
//DEVICE_PATH_STRING_TABLE  DevPathTable[] = {
//  {
//    HARDWARE_DEVICE_PATH,
//    HW_PCI_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathPci
//  },
//  {
//    HARDWARE_DEVICE_PATH,
//    HW_PCCARD_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathPccard
//  },
//  {
//    HARDWARE_DEVICE_PATH,
//    HW_MEMMAP_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathMemMap
//  },
//  {
//    HARDWARE_DEVICE_PATH,
//    HW_VENDOR_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathVendor
//  },
//  {
//    HARDWARE_DEVICE_PATH,
//    HW_CONTROLLER_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathController
//  },
//  {
//    ACPI_DEVICE_PATH,
//    ACPI_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathAcpi
//  },
//  {
//    ACPI_DEVICE_PATH,
//    ACPI_EXTENDED_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathExtendedAcpi
//  },
//  {
//    ACPI_DEVICE_PATH,
//    ACPI_ADR_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathAdrAcpi
//  },
//  {
//    MESSAGING_DEVICE_PATH,
//    MSG_ATAPI_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathAtapi
//  },
//  {
//    MESSAGING_DEVICE_PATH,
//    MSG_SCSI_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathScsi
//  },
//  {
//    MESSAGING_DEVICE_PATH,
//    MSG_FIBRECHANNEL_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathFibre
//  },
//  {
//    MESSAGING_DEVICE_PATH,
//    MSG_1394_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPath1394
//  },
//  {
//    MESSAGING_DEVICE_PATH,
//    MSG_USB_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathUsb
//  },
//  {
//    MESSAGING_DEVICE_PATH,
//    MSG_USB_WWID_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathUsbWWID
//  },
//  {
//    MESSAGING_DEVICE_PATH,
//    MSG_DEVICE_LOGICAL_UNIT_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathLogicalUnit
//  },
//  {
//    MESSAGING_DEVICE_PATH,
//    MSG_USB_CLASS_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathUsbClass
//  },
//  {
//    MESSAGING_DEVICE_PATH,
//    MSG_SATA_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathSata
//  },
//  {
//    MESSAGING_DEVICE_PATH,
//    MSG_NVME_NAMESPACE_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathToTextNVMe
//  },
//  {
//    MESSAGING_DEVICE_PATH,
//    MSG_I2O_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathI2O
//  },
//  {
//    MESSAGING_DEVICE_PATH,
//    MSG_MAC_ADDR_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathMacAddr
//  },
//  {
//    MESSAGING_DEVICE_PATH,
//    MSG_IPv4_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathIPv4
//  },
//  {
//    MESSAGING_DEVICE_PATH,
//    MSG_IPv6_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathIPv6
//  },
//  {
//    MESSAGING_DEVICE_PATH,
//    MSG_INFINIBAND_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathInfiniBand
//  },
//  {
//    MESSAGING_DEVICE_PATH,
//    MSG_UART_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathUart
//  },
//  {
//    MESSAGING_DEVICE_PATH,
//    MSG_VENDOR_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathVendor
//  },
//  {
//    MESSAGING_DEVICE_PATH,
//    MSG_ISCSI_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathiSCSI
//  },
//  {
//    MESSAGING_DEVICE_PATH,
//    MSG_VLAN_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathVlan
//  },
//  {
//    MEDIA_DEVICE_PATH,
//    MEDIA_HARDDRIVE_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathHardDrive
//  },
//  {
//    MEDIA_DEVICE_PATH,
//    MEDIA_CDROM_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathCDROM
//  },
//  {
//    MEDIA_DEVICE_PATH,
//    MEDIA_VENDOR_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathVendor
//  },
//  {
//    MEDIA_DEVICE_PATH,
//    MEDIA_FILEPATH_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathFilePath
//  },
//  {
//    MEDIA_DEVICE_PATH,
//    MEDIA_PROTOCOL_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathMediaProtocol
//  },
//  {
//    MEDIA_DEVICE_PATH,
//    MEDIA_PIWG_FW_VOL_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathFvPath,
//  },
//  {
//    MEDIA_DEVICE_PATH,
//    MEDIA_PIWG_FW_FILE_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathFvFilePath
//  },
//  {
//    MEDIA_DEVICE_PATH,
//    MEDIA_RELATIVE_OFFSET_RANGE_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathRelativeOffsetRange0,
//  },
//  {
//    BBS_DEVICE_PATH,
//    BBS_BBS_DP,
//    (DEV_PATH_FUNCTION_VOID)DevPathBssBss
//  },
//  {
//    END_DEVICE_PATH_TYPE,
//    END_INSTANCE_DEVICE_PATH_SUBTYPE,
//    (DEV_PATH_FUNCTION_VOID)DevPathEndInstance
//  },
//  {
//    0,
//    0,
//    (DEV_PATH_FUNCTION_VOID)NULL
//  }
//};
//
//
///**
//  This function converts an input device structure to a Unicode string.
//
//  @param DevPath                  A pointer to the device path structure.
//
//  @return A new allocated Unicode string that represents the device path.
//
//**/
//CHAR16 *
//EFIAPI
//DevicePathToStr (
//  IN EFI_DEVICE_PATH_PROTOCOL     *DevPath
//  )
//{
//  POOL_PRINT                Str;
//  EFI_DEVICE_PATH_PROTOCOL  *DevPathNode;
//  VOID (*DumpNode) (POOL_PRINT *, VOID *);
//
//  UINTN Index;
//  UINTN NewSize;
//
//  EFI_STATUS                       Status;
//  CHAR16                           *ToText;
//  EFI_DEVICE_PATH_TO_TEXT_PROTOCOL *DevPathToText;
//
//  ZeroMem (&Str, sizeof (Str));
//
//  if (DevPath == NULL) {
//    goto Done;
//  }
//
//  Status = gBS->LocateProtocol (
//                  &gEfiDevicePathToTextProtocolGuid,
//                  NULL,
//                  (VOID **) &DevPathToText
//                  );
//  if (!EFI_ERROR (Status)) {
//    ToText = DevPathToText->ConvertDevicePathToText (
//                              DevPath,
//                              FALSE,
//                              TRUE
//                              );
////    ASSERT (ToText != NULL);
//    return ToText;
//  }
//
//  //
//  // Process each device path node
//  //
//  DevPathNode = DevPath;
//  while (!IsDevicePathEnd (DevPathNode)) {
//    //
//    // Find the handler to dump this device path node
//    //
//    DumpNode = NULL;
//    for (Index = 0; DevPathTable[Index].Function != NULL; Index += 1) {
//
//      if (DevicePathType (DevPathNode) == DevPathTable[Index].Type &&
//          DevicePathSubType (DevPathNode) == DevPathTable[Index].SubType
//          ) {
//        DumpNode = DevPathTable[Index].Function;
//        break;
//      }
//    }
//    //
//    // If not found, use a generic function
//    //
//    if (!DumpNode) {
//      DumpNode = DevPathNodeUnknown;
//    }
//    //
//    //  Put a path separator in if needed
//    //
//    if ((Str.Len != 0) && (DumpNode != DevPathEndInstance)) {
//      CatPrint (&Str, L"/");
//    }
//    //
//    // Print this node of the device path
//    //
//    DumpNode (&Str, DevPathNode);
//
//    //
//    // Next device path node
//    //
//    DevPathNode = NextDevicePathNode (DevPathNode);
//  }
//
//Done:
//  NewSize = (Str.Len + 1) * sizeof (CHAR16);
//  Str.Str = (__typeof__(Str.Str))ReallocatePool (NewSize, NewSize, Str.Str);
////  ASSERT (Str.Str != NULL);
//  if (!Str.Str) {
//    return NULL;
//  }
//
//  Str.Str[Str.Len] = 0;
//  return Str.Str;
//}
