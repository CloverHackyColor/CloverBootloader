#ifndef _PCI22_H
#define _PCI22_H

/*++

Copyright (c) 1999  Intel Corporation

Module Name:

    pci22.h
    
Abstract:      
    Support for PCI 2.2 standard.




Revision History

--*/

#ifdef SOFT_SDV
#define PCI_MAX_BUS     1
#else
#define PCI_MAX_BUS     255
#endif

#define PCI_MAX_DEVICE  31
#define PCI_MAX_FUNC    7

//
// Command
//
#define PCI_VGA_PALETTE_SNOOP_DISABLED   0x20

#pragma pack(1)
typedef struct {
    UINT16      VendorId;
    UINT16      DeviceId;
    UINT16      Command;
    UINT16      Status;
    UINT8       RevisionID;
    UINT8       ClassCode[3];
    UINT8       CacheLineSize;
    UINT8       LaytencyTimer;
    UINT8       HeaderType;
    UINT8       BIST;
} PCI_DEVICE_INDEPENDENT_REGION;

typedef struct {
    UINT32      Bar[6];
    UINT32      CISPtr;
    UINT16      SubsystemVendorID;
    UINT16      SubsystemID;
    UINT32      ExpansionRomBar;
    UINT32      Reserved[2];
    UINT8       InterruptLine;
    UINT8       InterruptPin;
    UINT8       MinGnt;
    UINT8       MaxLat;     
} PCI_DEVICE_HEADER_TYPE_REGION;

typedef struct {
    PCI_DEVICE_INDEPENDENT_REGION   Hdr;
    PCI_DEVICE_HEADER_TYPE_REGION   Device;
} PCI_TYPE00;

typedef struct {              
    UINT32      Bar[2];
    UINT8       PrimaryBus;
    UINT8       SecondaryBus;
    UINT8       SubordinateBus;
    UINT8       SecondaryLatencyTimer;
    UINT8       IoBase;
    UINT8       IoLimit;
    UINT16      SecondaryStatus;
    UINT16      MemoryBase;
    UINT16      MemoryLimit;
    UINT16      PrefetchableMemoryBase;
    UINT16      PrefetchableMemoryLimit;
    UINT32      PrefetchableBaseUpper32;
    UINT32      PrefetchableLimitUpper32;
    UINT16      IoBaseUpper16;
    UINT16      IoLimitUpper16;
    UINT32      Reserved;
    UINT32      ExpansionRomBAR;
    UINT8       InterruptLine;
    UINT8       InterruptPin;
    UINT16      BridgeControl;
} PCI_BRIDGE_CONTROL_REGISTER;

#define PCI_CLASS_DISPLAY_CTRL          0x03
#define PCI_CLASS_VGA                   0x00

#define PCI_CLASS_BRIDGE                0x06
#define PCI_CLASS_ISA                   0x01
#define PCI_CLASS_ISA_POSITIVE_DECODE   0x80

#define PCI_CLASS_NETWORK               0x02 
#define PCI_CLASS_ETHERNET              0x00
        
#define HEADER_TYPE_DEVICE              0x00
#define HEADER_TYPE_PCI_TO_PCI_BRIDGE   0x01
#define HEADER_TYPE_MULTI_FUNCTION      0x80
#define HEADER_LAYOUT_CODE              0x7f

#define IS_PCI_BRIDGE(_p) ((((_p)->Hdr.HeaderType) & HEADER_LAYOUT_CODE) == HEADER_TYPE_PCI_TO_PCI_BRIDGE)        
#define IS_PCI_MULTI_FUNC(_p)   (((_p)->Hdr.HeaderType) & HEADER_TYPE_MULTI_FUNCTION)         

typedef struct {
    PCI_DEVICE_INDEPENDENT_REGION   Hdr;
    PCI_BRIDGE_CONTROL_REGISTER     Bridge;
} PCI_TYPE01;

typedef struct {
    UINT8   Register;
    UINT8   Function;
    UINT8   Device;
    UINT8   Bus;
    UINT8   Reserved[4];
} DEFIO_PCI_ADDR;

typedef struct {
    UINT32  Reg     : 8;
    UINT32  Func    : 3;
    UINT32  Dev     : 5;
    UINT32  Bus     : 8;
    UINT32  Reserved: 7;
    UINT32  Enable  : 1;
} PCI_CONFIG_ACCESS_CF8;

#pragma pack()

#define EFI_ROOT_BRIDGE_LIST    'eprb'
typedef struct {
    UINTN           Signature;

    UINT16          BridgeNumber;
    UINT16          PrimaryBus;
    UINT16          SubordinateBus;

    EFI_DEVICE_PATH *DevicePath;

    LIST_ENTRY      Link;
} PCI_ROOT_BRIDGE_ENTRY;


#define PCI_EXPANSION_ROM_HEADER_SIGNATURE        0xaa55
#define EFI_PCI_EXPANSION_ROM_HEADER_EFISIGNATURE 0x0EF1
#define PCI_DATA_STRUCTURE_SIGNATURE              EFI_SIGNATURE_32('P','C','I','R')

#pragma pack(1)
typedef struct {
    UINT16          Signature;              // 0xaa55
    UINT8           Reserved[0x16];
    UINT16          PcirOffset;
} PCI_EXPANSION_ROM_HEADER;


typedef struct {
    UINT16          Signature;              // 0xaa55
    UINT16          InitializationSize;
    UINT16          EfiSignature;           // 0x0EF1
    UINT16          EfiSubsystem;
    UINT16          EfiMachineType;
    UINT8           Reserved[0x0A];
    UINT16          EfiImageHeaderOffset;
    UINT16          PcirOffset;
} EFI_PCI_EXPANSION_ROM_HEADER;

typedef struct {
    UINT32          Signature;              // "PCIR" 
    UINT16          VendorId;
    UINT16          DeviceId;
    UINT16          Reserved0;
    UINT16          Length;
    UINT8           Revision;
    UINT8           ClassCode[3];
    UINT16          ImageLength;
    UINT16          CodeRevision;
    UINT8           CodeType;
    UINT8           Indicator;
    UINT16          Reserved1;
} PCI_DATA_STRUCTURE;
#pragma pack()

#endif
    




    
