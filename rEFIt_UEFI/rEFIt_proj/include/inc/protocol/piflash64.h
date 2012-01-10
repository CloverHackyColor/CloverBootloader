#ifndef _PIFLASH64_H
#define _PIFLASH64_H

/*++

Copyright (c) 1999  Intel Corporation

Module Name:

    PIflash64.h
    
Abstract:

    Iflash64.efi protocol to abstract iflash from
    the system.

Revision History

--*/

//
// Guid that identifies the IFLASH protocol
//
#define IFLASH64_PROTOCOL_PROTOCOL \
    { 0x65cba110, 0x74ab, 0x11d3, 0xbb, 0x89, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 };

//
// Unlock FLASH from StartAddress to EndAddress and return a LockKey
//
typedef
EFI_STATUS
(EFIAPI *UNLOCK_FLASH_API)(
    IN struct _IFLASH64_PROTOCOL_INTERFACE  *This
    );

//
// Lock the flash represented by the LockKey
//
typedef
EFI_STATUS
(EFIAPI *LOCK_FLASH_API)(
    IN struct _IFLASH64_PROTOCOL_INTERFACE  *This
    );

//
// Status callback for a utility like IFLASH64
//
//  Token would map to a list like Ted proposed. The utility has no idea what 
//      happens on the other side.
//  ErrorStatus - Level of Error or success. Independent of Token. If you 
//      don't know the token you will at least know pass or fail.
//  String - Optional extra information about the error. Could be used for 
//      debug or future expansion
//
//  Attributes - Options screen attributes for String. Could allow the string to be different colors.
//
typedef
EFI_STATUS
(EFIAPI *UTILITY_PROGRESS_API)(
    IN struct _IFLASH64_PROTOCOL_INTERFACE  *This,
    IN  UINTN                               Token,
    IN  EFI_STATUS                          ErrorStatus, 
    IN  CHAR16                              *String,    OPTIONAL
    IN  UINTN                               *Attributes OPTIONAL
    );

//
// Token Values
//
// IFlash64 Token Codes
#define IFLASH_TOKEN_IFLASHSTART    0xB0                // IFlash64 has started
#define IFLASH_TOKEN_READINGFILE    0xB1                // Reading File
#define IFLASH_TOKEN_INITVPP        0xB2                // Initializing Vpp
#define IFLASH_TOKEN_DISABLEVPP     0x10                // Disable Vpp
#define IFLASH_TOKEN_FLASHUNLOCK    0xB3                // Unlocking FLASH Devices
#define IFLASH_TOKEN_FLASHERASE     0xB4                // Erasing FLASH Devices
#define IFLASH_TOKEN_FLASHPROGRAM   0xB5                // Programming FLASH
#define IFLASH_TOKEN_FLASHVERIFY    0xB6                // Verifying FLASH
#define IFLASH_TOKEN_UPDATESUCCES   0xB7                // FLASH Updage Success!

#define IFLASH_TOKEN_PROGRESS_READINGFILE   0x11        // % Reading File
#define IFLASH_TOKEN_PROGRESS_FLASHUNLOCK   0x13        // % Unlocking FLASH Devices
#define IFLASH_TOKEN_PROGRESS_FLASHERASE    0x14        // % Erasing FLASH Devices
#define IFLASH_TOKEN_PROGRESS_FLASHPROGRAM  0x15        // % Programming FLASH
#define IFLASH_TOKEN_PROGRESS_FLASHVERIFY   0x16        // % Verifying FLASH

#define IFLASH_TOKEN_READINGFILE_ER 0xB8                // File Read Error
#define IFLASH_TOKEN_INITVPP_ER     0xB9                // Initialization of IFB Error
#define IFLASH_TOKEN_FLASHUNLOCK_ER 0xBA                // FLASH Unlock Error
#define IFLASH_TOKEN_FLASHERASE_ER  0xBB                // FLASH Erase Error
#define IFLASH_TOKEN_FLASHVERIFY_ER 0xBC                // FLASH Verify Error
#define IFLASH_TOKEN_FLASHPROG_ER   0xBD                // FLASH Program Error

#define IFLASH_TABLE_END            0x00

//
// If this number changes one of the existing API's has changes
//
#define IFLASH_PI_MAJOR_VERSION 0x01

//
// This number changes when new APIs or data variables get added to the end
//  of the data structure
//
#define IFLASH_PI_MINOR_VERSION 0x01

typedef struct _IFLASH64_PROTOCOL_INTERFACE {
    UINT32                  MajorVersion;       
    UINT32                  MinorVersion;   
    UNLOCK_FLASH_API        UnlockFlash;
    LOCK_FLASH_API          LockFlash;
    UTILITY_PROGRESS_API    Progress;
    
    //
    // Future expansion goes here
    //

} IFLASH64_PROTOCOL_INTERFACE;


#endif
