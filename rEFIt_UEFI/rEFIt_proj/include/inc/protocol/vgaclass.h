#ifndef _VGA_CLASS_H
#define _VGA_CLASS_H

/*++

Copyright (c) 1999  Intel Corporation

Module Name:

    VgaClass.h
    
Abstract:

    Vga Mini port binding to Vga Class protocol



Revision History

--*/

//
// VGA Device Structure
//

// {0E3D6310-6FE4-11d3-BB81-0080C73C8881}
#define VGA_CLASS_DRIVER_PROTOCOL \
    { 0xe3d6310, 0x6fe4, 0x11d3, {0xbb, 0x81, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81} }

typedef 
EFI_STATUS 
(* INIT_VGA_CARD) (
    IN  UINTN   VgaMode,
    IN  VOID    *Context
    );

typedef struct {
    UINTN   MaxColumns;
    UINTN   MaxRows;
} MAX_CONSOLE_GEOMETRY;

#define VGA_CON_OUT_DEV_SIGNATURE   EFI_SIGNATURE_32('c','v','g','a')
typedef struct {
    UINTN                           Signature;

    EFI_HANDLE                      Handle;
    SIMPLE_TEXT_OUTPUT_INTERFACE    ConOut;
    SIMPLE_TEXT_OUTPUT_MODE         ConOutMode;
    EFI_DEVICE_PATH                 *DevicePath;

    UINT8                           *Buffer;
    EFI_DEVICE_IO_INTERFACE         *DeviceIo;

    //
    // Video Card Context
    //
    INIT_VGA_CARD                   InitVgaCard;
    VOID                            *VgaCardContext;
    MAX_CONSOLE_GEOMETRY            *Geometry;
    //
    // Video buffer normally 0xb8000
    //
    UINT64                          VideoBuffer;

    //
    // Clear Screen & Default Attribute
    //
    UINT32                          Attribute;

    //
    // -1 means search for active VGA device
    //
    EFI_PCI_ADDRESS_UNION           Pci;
} VGA_CON_OUT_DEV;

#define VGA_CON_OUT_DEV_FROM_THIS(a) CR(a, VGA_CON_OUT_DEV, ConOut, VGA_CON_OUT_DEV_SIGNATURE)

//
// Vga Class Driver Protocol. 
// GUID defined in EFI Lib
//

typedef 
EFI_STATUS
(EFIAPI *INSTALL_VGA_DRIVER) (
    IN  VGA_CON_OUT_DEV    *ConOutDev 
    );

typedef struct {
    UINT32               Version;
    INSTALL_VGA_DRIVER   InstallGenericVgaDriver;
} INSTALL_VGA_DRIVER_INTERFACE;

#endif

