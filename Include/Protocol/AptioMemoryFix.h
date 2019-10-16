/**

  Aptio Memory Fix protocol to inform bootloaders
  about driver availability.

  by cecekpawon, vit9696

**/

#ifndef APTIOFIX_MEMORY_PROTOCOL_H
#define APTIOFIX_MEMORY_PROTOCOL_H

#define APTIOMEMORYFIX_PACKAGE_VERSION L"R27"

#define APTIOMEMORYFIX_PROTOCOL_REVISION  27

#include <Library/UefiLib.h>

//#ifndef EFIAPI
//#if _MSC_EXTENSIONS
///
/// Define the standard calling convention regardless of optimization level.
/// __cdecl is Microsoft* specific C extension.
///
//#define EFIAPI __cdecl
//#endif
//#endif

//
// APTIOMEMORYFIX_PROTOCOL_GUID
// C7CBA84E-CC77-461D-9E3C-6BE0CB79A7C1
//
#define APTIOMEMORYFIX_PROTOCOL_GUID  \
  { 0xC7CBA84E, 0xCC77, 0x461D, { 0x9E, 0x3C, 0x6B, 0xE0, 0xCB, 0x79, 0xA7, 0xC1 } }

//
// Set NVRAM routing, returns previous value.
//
typedef
BOOLEAN
(EFIAPI *AMF_SET_NVRAM_REDIRECT) (
  IN BOOLEAN  NewValue
  );

//
// Includes a revision for debugging reasons
//
typedef struct {
  UINTN                   Revision;
  AMF_SET_NVRAM_REDIRECT  SetNvram;
} APTIOMEMORYFIX_PROTOCOL;

extern EFI_GUID gAptioMemoryFixProtocolGuid;

#endif // APTIOFIX_MEMORY_PROTOCOL_H
