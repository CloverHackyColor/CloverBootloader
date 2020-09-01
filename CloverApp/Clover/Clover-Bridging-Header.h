//
//  Use this file to import your target's public headers that you would like to expose to Swift.
//

#ifndef CLOVERAPPLICATION
#define CLOVERAPPLICATION 1
#endif
/*
#ifdef DEBUG
#define DEBUG_BACKUP DEBUG // backup original
#undef DEBUG
#endif

#include "../../rEFIt_UEFI/Platform/Platform.h"

#ifdef DEBUG_BACKUP
#undef DEBUG
#define DEBUG DEBUG_BACKUP // restore original
#endif
*/

#import "NSWindowFix.h"
#import "ThemeImage.h"
#import "gfxutil.h"
#import "efidevp.h"

/*
 NOTE for developers:
 Unless edk2 headers are imported (as a UEFI application), below definitions runs in a clean enviroment
 and are totally disconnected from Clover. They are just duplicates to deserialize the SETTINGS_DATA structure
 (and until compatible).
 This was done to mantains compatibility across revisions. The tuple is in fact accessed by
 strings (using the Swift Mirror class) so that the app can access variables if the element really exist
 inside the SETTINGS_DATA structure, and w/o crash if an element get removed, like can happen in newer commits.
 This header is used to expose c (or objective-c) code to Swift and doesn't affect Clover bootloader in no way.
 
 c++ code can also be imported, but only inside an obj-c Class.
 */

#define VOID void
#define CONST const

typedef signed char         INT8;
typedef unsigned char       UINT8;
typedef UINT8               BOOLEAN;
typedef char                CHAR8;
typedef unsigned short      CHAR16;
typedef short               INT16;
typedef unsigned short      UINT16;
typedef int                 INT32;
typedef unsigned int        UINT32;
typedef long long           INT64;
typedef INT64               INTN;
typedef unsigned long long  UINT64;
typedef UINT64              UINTN;
typedef  float              FLOAT;
typedef  double             DOUBLE;

typedef struct {
  UINT8 Type;       ///< 0x01 Hardware Device Path.
  ///< 0x02 ACPI Device Path.
  ///< 0x03 Messaging Device Path.
  ///< 0x04 Media Device Path.
  ///< 0x05 BIOS Boot Specification Device Path.
  ///< 0x7F End of Hardware Device Path.
  UINT8 SubType;    ///< Varies by Type
  ///< 0xFF End Entire Device Path, or
  ///< 0x01 End This Instance of a Device Path and start a new
  ///< Device Path.
  UINT8 Length[2];  ///< Specific Device Path data. Type and Sub-Type define
  ///< type of data. Size of data is included in Length.
} EFI_DEVICE_PATH_PROTOCOL;

typedef struct {
  UINT32  Data1;
  UINT16  Data2;
  UINT16  Data3;
  UINT8   Data4[8];
} EFI_GUID;

typedef struct {
  UINT8 b, g, r, a;
} EG_PIXEL;

#import "CloverOldHeaders.h"

