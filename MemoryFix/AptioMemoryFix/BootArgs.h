/**

  Methods for finding, checking and fixing boot args

  by dmazar (defs from Clover)

**/

#ifndef APTIOFIX_BOOT_ARGS_H
#define APTIOFIX_BOOT_ARGS_H

#include <IndustryStandard/AppleBootArgs.h>
#include <Library/OcMiscLib.h>


/** Our internal structure to hold boot args params to make the code independent of the boot args version. */
typedef struct {
  UINT32  *MemoryMap;      /* We will change this value so we need pointer to original field. */
  UINT32  *MemoryMapSize;
  UINT32  *MemoryMapDescriptorSize;
  UINT32  *MemoryMapDescriptorVersion;

  CHAR8   *CommandLine;

  UINT32  *deviceTreeP;
  UINT32  *deviceTreeLength;

  UINT32  *csrActiveConfig;
} AMF_BOOT_ARGUMENTS;

AMF_BOOT_ARGUMENTS *
GetBootArgs (
  IN VOID  *BootArgs
  );

CONST CHAR8 *
GetArgumentFromCommandLine (
  IN CONST CHAR8  *CommandLine,
  IN CONST CHAR8  *Argument,
  IN CONST UINTN  ArgumentLength
  );

VOID
RemoveArgumentFromCommandLine (
  IN OUT CHAR8        *CommandLine,
  IN     CONST CHAR8  *Argument
  );

BOOLEAN
AppendArgumentToCommandLine (
  IN OUT CHAR8        *CommandLine,
  IN     CONST CHAR8  *Argument,
  IN     CONST UINTN  ArgumentLength
  );

#endif // APTIOFIX_BOOT_ARGS_H
