/*
 *  usbfix.c
 *
 *  Created by Slice on 21.09.11.
 *
 *  based on works by mackerintel 2008, orByte 2006, Signal64, THeKiNG
 */

#include "Platform.h"

#ifndef DEBUG_ALL
#define DEBUG_USB 0
#else
#define DEBUG_USB DEBUG_ALL
#endif

#if DEBUG_USB == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_USB, __VA_ARGS__)
#endif


#define PCI_IF_OHCI      0x10
#define PCI_IF_XHCI     0x30
#define OHCI_CTRL_MASK  (1 << 9)
#define OHCI_CONTROL    0x04
#define OHCI_INTRDISABLE  0x14
#define OHCI_INTRSTATUS    0x0c

EFI_STATUS
FixOwnership(VOID)
/*++
 
 Routine Description:
 Disable the USB legacy Support in all Ehci and Uhci.
 This function assume all PciIo handles have been created in system.
 Slice - added also OHCI and more advanced algo. Better then known to Intel and Apple :)
 Arguments:
 None
 
 Returns:
 EFI_SUCCESS
 EFI_NOT_FOUND
 --*/
{
  EFI_STATUS          Status;
  EFI_HANDLE          *HandleArray = NULL;
  UINTN             HandleArrayCount = 0;
  UINTN             Index;
  EFI_PCI_IO_PROTOCOL      *PciIo;
  PCI_TYPE00              Pci;
  UINT16            Command;
  UINT32            HcCapParams;
  UINT32            ExtendCap;
  UINT32            Value;
  INT32            TimeOut;
  UINT32            Base;
  UINT32            PortBase;
  volatile UINT32        opaddr;      
//  UINT8            eecp;      
  UINT32            usbcmd, usbsts, usbintr;      
  UINT32            usblegsup, usblegctlsts;    
  
  UINTN             isOSowned;
  UINTN             isBIOSowned;
  BOOLEAN           isOwnershipConflict;
  
  //
  // Find the usb host controller 
  //   
  Status = gBS->LocateHandleBuffer (
                    ByProtocol,
                    &gEfiPciIoProtocolGuid,
                    NULL,
                    &HandleArrayCount,
                    &HandleArray
                    );
  if (!EFI_ERROR (Status)) {
    for (Index = 0; Index < HandleArrayCount; Index++) {
      Status = gBS->HandleProtocol (
                      HandleArray[Index],
                      &gEfiPciIoProtocolGuid,
                      (VOID **)&PciIo
                      );
      if (!EFI_ERROR (Status)) {
        //
        // Find the USB host controller
        //
        Status = PciIo->Pci.Read (
                      PciIo,
                      EfiPciIoWidthUint32,
                      0,
                      sizeof (Pci) / sizeof (UINT32),
                      &Pci
                      );
        
        if (!EFI_ERROR (Status)) {
          if ((PCI_CLASS_SERIAL == Pci.Hdr.ClassCode[2]) &&
            (PCI_CLASS_SERIAL_USB == Pci.Hdr.ClassCode[1])) {
            switch (Pci.Hdr.ClassCode[0]) {
              case PCI_IF_UHCI:
                //
                // Found the UHCI, then disable the legacy support
                //
                Base = 0;
                Status = PciIo->Pci.Read(PciIo, EfiPciIoWidthUint32, 0x20, 1, &Base);
                PortBase = (Base >> 5) & 0x07ff;
                DBG("USB UHCI Base=%X PortBase=%X\n", Base, PortBase);                 
                Command = 0x8f00;
                Status = PciIo->Pci.Write (PciIo, EfiPciIoWidthUint16, 0xC0, 1, &Command);
                if (PortBase) {
                  IoWrite16 (PortBase, 0x0002);
                  gBS->Stall (500);
                  IoWrite16 (PortBase+4, 0);
                  gBS->Stall (500);
                  IoWrite16 (PortBase, 0);
                }
                
                MsgLog("USB UHCI reset for device %04X\n", Pci.Hdr.DeviceId); 
                break;
  /*            case PCI_IF_OHCI:
                
                Base = 0;
                Status = PciIo->Pci.Read(PciIo, EfiPciIoWidthUint32, 0x10, 1, &Base);
                
                Command = *(UINT32 *)(UINTN)(Base + OHCI_CONTROL);
                *(UINT32 *)(UINTN)(Base + OHCI_CONTROL) = Command & OHCI_CTRL_MASK;
                Command = *(UINT32 *)(UINTN)(Base + OHCI_CONTROL);
                MsgLog("USB OHCI reset for device %04X control=0x%X\n", Pci.Hdr.DeviceId, Command);
                break;*/
              case PCI_IF_EHCI:
                //Slice - the algo is reworked from Chameleon
                // it looks like redundant but it works so I will not reduce it
                //
                // Found the EHCI, then disable the legacy support
                //
                Value = 0x0002;
                PciIo->Pci.Write (PciIo, EfiPciIoWidthUint16, 0x04, 1, &Value);
                
                Base = 0;
                Status = PciIo->Pci.Read(PciIo, EfiPciIoWidthUint32, 0x10, 1, &Base);
                if (*((UINT8*)(UINTN)Base) < 0x0C) {
                  DBG("Config space too small: no legacy implementation\n");
                  break;
                }
                
                // opaddr = Operational Registers = capaddr + offset (8bit CAPLENGTH in Capability Registers + offset 0)
                opaddr = Base + *((UINT8*)(UINTN)(Base));
                // eecp = EHCI Extended Capabilities offset = capaddr HCCPARAMS bits 15:8
                //UEFI
                Status = PciIo->Mem.Read (
                              PciIo,
                              EfiPciIoWidthUint32,
                              0,                   //EHC_BAR_INDEX
                              (UINT64) 0x08,       //EHC_HCCPARAMS_OFFSET
                              1,
                              &HcCapParams
                              );
                
                ExtendCap = (HcCapParams >> 8) & 0xFF;
                DBG("Base=%X Oper=%X eecp=%X\n", Base, opaddr, ExtendCap);
                
                usbcmd = *((UINT32*)(UINTN)(opaddr));      // Command Register
                usbsts = *((UINT32*)(UINTN)(opaddr + 4));    // Status Register
                usbintr = *((UINT32*)(UINTN)(opaddr + 8));    // Interrupt Enable Register
                
                DBG("usbcmd=%08X usbsts=%08X usbintr=%08X\n", usbcmd, usbsts, usbintr);
                
                // read PCI Config 32bit USBLEGSUP (eecp+0) 
                Status = PciIo->Pci.Read(PciIo, EfiPciIoWidthUint32, ExtendCap, 1, &usblegsup);
                
                // informational only
                isBIOSowned = !!((usblegsup) & (1 << (16)));
                isOSowned = !!((usblegsup) & (1 << (24)));
                
                // read PCI Config 32bit USBLEGCTLSTS (eecp+4) 
                PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, ExtendCap + 0x4, 1, &usblegctlsts);
					DBG("usblegsup=%08X isOSowned=%llu isBIOSowned=%llu usblegctlsts=%08X\n", usblegsup, isOSowned, isBIOSowned, usblegctlsts);
                //
                // Disable the SMI in USBLEGCTLSTS firstly
                //
                
                usblegctlsts &= 0xFFFF0000;
                PciIo->Pci.Write (PciIo, EfiPciIoWidthUint32, ExtendCap + 0x4, 1, &usblegctlsts);
                
                //double
                // if delay value is in milliseconds it doesn't appear to work. 
                // setting value to anything up to 65535 does not add the expected delay here.
                gBS->Stall (500);                
                usbcmd = *((UINT32*)(UINTN)(opaddr));      // Command Register
                usbsts = *((UINT32*)(UINTN)(opaddr + 4));    // Status Register
                usbintr = *((UINT32*)(UINTN)(opaddr + 8));    // Interrupt Enable Register
                DBG("usbcmd=%08X usbsts=%08X usbintr=%08X\n", usbcmd, usbsts, usbintr);
                
                // clear registers to default
                usbcmd = (usbcmd & 0xffffff00);
                *((UINT32*)(UINTN)(opaddr)) = usbcmd;
                *((UINT32*)(UINTN)(opaddr + 8)) = 0;      //usbintr - clear interrupt registers
                *((UINT32*)(UINTN)(opaddr + 4)) = 0x1000;    //usbsts - clear status registers 
                Value = 1;
                PciIo->Pci.Write (PciIo, EfiPciIoWidthUint32, ExtendCap, 1, &Value);
                
                  // get the results
                usbcmd = *((UINT32*)(UINTN)(opaddr));      // Command Register
                usbsts = *((UINT32*)(UINTN)(opaddr + 4));    // Status Register
                usbintr = *((UINT32*)(UINTN)(opaddr + 8));    // Interrupt Enable Register
                DBG("usbcmd=%08X usbsts=%08X usbintr=%08X\n", usbcmd, usbsts, usbintr);
                
                // read 32bit USBLEGSUP (eecp+0) 
                PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, ExtendCap, 1, &usblegsup);
                // informational only
                isBIOSowned = !!((usblegsup) & (1 << (16)));
                isOSowned = !!((usblegsup) & (1 << (24)));
                
                // read 32bit USBLEGCTLSTS (eecp+4) 
                PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, ExtendCap + 0x4, 1, &usblegctlsts);
                
					DBG("usblegsup=%08X isOSowned=%llu isBIOSowned=%llu usblegctlsts=%08X\n", usblegsup, isOSowned, isBIOSowned, usblegctlsts);
                MsgLog("Legacy USB Off Done\n");  
                
                
                //
                // Get EHCI Ownership from legacy bios
                //
                PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, ExtendCap, 1, &usblegsup);
                isOwnershipConflict = isBIOSowned && isOSowned;
                if (isOwnershipConflict) {
                  DBG("EHCI - Ownership conflict - attempting soft reset ...\n");
                  Value = 0;
                  PciIo->Pci.Write (PciIo, EfiPciIoWidthUint8, ExtendCap + 3, 1, &Value);
                  TimeOut = 40;
                  while (TimeOut--) {
                    gBS->Stall (500);
                    
                    PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, ExtendCap, 1, &Value);
                    
                    if ((Value & 0x01000000) == 0x0) {
                      break;
                    }
                  }
                }  
                
                PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, ExtendCap, 1, &Value);
                Value |= (0x1 << 24);
                PciIo->Pci.Write (PciIo, EfiPciIoWidthUint32, ExtendCap, 1, &Value);
                
                TimeOut = 40;
                while (TimeOut--) {
                  gBS->Stall (500);
                  
                  PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, ExtendCap, 1, &Value);
                  
                  if ((Value & 0x00010000) == 0x0) {
                    break;
                  }
                }
                isOwnershipConflict = ((Value & 0x00010000) != 0x0);
                if (isOwnershipConflict) {
                  // Soft reset has failed. Assume SMI being ignored
                  // Hard reset
                  DBG("Soft reset has failed - attempting hard reset ...\n");
                  Value = 0;
                  PciIo->Pci.Write (PciIo, EfiPciIoWidthUint8, ExtendCap + 2, 1, &Value);
                  TimeOut = 40;
                  while (TimeOut--) {
                    gBS->Stall (500);
                    
                    PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, ExtendCap, 1, &Value);
                    
                    if ((Value & 0x00010000) == 0x0) {
                      break;
                    }
                  }
                  // Disable further SMI events
                  PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, ExtendCap + 0x4, 1, &usblegctlsts);
                  usblegctlsts &= 0xFFFF0000;
                  PciIo->Pci.Write (PciIo, EfiPciIoWidthUint32, ExtendCap + 0x4, 1, &usblegctlsts);
                }                
                if (Value & 0x00010000) {          
                  MsgLog("EHCI controller unable to take control from BIOS\n");
                  Status = EFI_NOT_FOUND; //Slice - why? :)
                  break;
                }
                MsgLog("USB EHCI Ownership for device %04X value=%X\n", Pci.Hdr.DeviceId, Value); 
                
                break;
             case PCI_IF_XHCI:
                //
                // Found the XHCI, then disable the legacy support, if present
                //
                Status = PciIo->Mem.Read(PciIo, EfiPciIoWidthUint32, 0 /* BAR0 */, (UINT64) 0x10 /* HCCPARAMS1 */, 1, &HcCapParams);
                ExtendCap = EFI_ERROR(Status) ? 0 : ((HcCapParams >> 14) & 0x3FFFC);
                while (ExtendCap) {
                  Status = PciIo->Mem.Read(PciIo, EfiPciIoWidthUint32, 0 /* BAR0 */, (UINT64) ExtendCap, 1, &Value);
                  if (EFI_ERROR(Status))
                    break;
                  if ((Value & 0xFF) == 1) {
                    //
                    // Do nothing if Bios Ownership clear
                    //
                    if (!(Value & (0x1 << 16)))
                      break;
                    Value |= (0x1 << 24);
                    (VOID) PciIo->Mem.Write(PciIo, EfiPciIoWidthUint32, 0 /* BAR0 */, (UINT64) ExtendCap, 1, &Value);
                    TimeOut = 40;
                    while (TimeOut--) {
                      gBS->Stall(500);
                      Status = PciIo->Mem.Read(PciIo, EfiPciIoWidthUint32, 0 /* BAR0 */, (UINT64) ExtendCap, 1, &Value);
                      if (EFI_ERROR(Status) || !(Value & (0x1 << 16)))
                        break;
                    }
                    //
                    // Disable all SMI in USBLEGCTLSTS
                    //
                    Status = PciIo->Mem.Read(PciIo, EfiPciIoWidthUint32, 0 /* BAR0 */, (UINT64) ExtendCap + 4, 1, &Value);
                    if (EFI_ERROR(Status))
                      break;
                    Value &= 0x1F1FEE;
                    Value |= 0xE0000000;
                    (VOID) PciIo->Mem.Write(PciIo, EfiPciIoWidthUint32, 0 /* BAR0 */, (UINT64) ExtendCap + 4, 1, &Value);
                    //
                    // Clear all ownership
                    //
                    Status = PciIo->Mem.Read(PciIo, EfiPciIoWidthUint32, 0 /* BAR0 */, (UINT64) ExtendCap, 1, &Value);
                    if (EFI_ERROR(Status))
                      break;
                    Value &= ~((0x1 << 24) | (0x1 << 16));
                    (VOID) PciIo->Mem.Write(PciIo, EfiPciIoWidthUint32, 0 /* BAR0 */, (UINT64) ExtendCap, 1, &Value);
                    break;
                  } //Value & FF
                  if (!(Value & 0xFF00))
                    break;
                  ExtendCap += ((Value >> 6) & 0x3FC);
                } //while ExtendCap
                break;
              default:
                break;
            } //switch class code
          } 
        }
      }
    }
  } else {
    return Status;
  }
  gBS->FreePool (HandleArray);
  return Status;
}
