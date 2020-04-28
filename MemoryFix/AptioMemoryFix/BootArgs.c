/**

  Methods for finding, checking and fixing boot args

  by dmazar

**/

#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>

#include "Config.h"
#include "BootArgs.h"

STATIC AMF_BOOT_ARGUMENTS mBootArgs;

AMF_BOOT_ARGUMENTS *
GetBootArgs (
  IN VOID  *BootArgs
  )
{
  BootArgs1  *BA1 = BootArgs;
  BootArgs2  *BA2 = BootArgs;

  ZeroMem (&mBootArgs, sizeof(mBootArgs));

  if (BA1->Version == kBootArgsVersion1) {
    //
    // Pre Lion
    //
    mBootArgs.MemoryMap = &BA1->MemoryMap;
    mBootArgs.MemoryMapSize = &BA1->MemoryMapSize;
    mBootArgs.MemoryMapDescriptorSize = &BA1->MemoryMapDescriptorSize;
    mBootArgs.MemoryMapDescriptorVersion = &BA1->MemoryMapDescriptorVersion;

    mBootArgs.CommandLine = &BA1->CommandLine[0];

    mBootArgs.deviceTreeP = &BA1->deviceTreeP;
    mBootArgs.deviceTreeLength = &BA1->deviceTreeLength;
  } else {
    //
    // Lion and newer
    //
    mBootArgs.MemoryMap = &BA2->MemoryMap;
    mBootArgs.MemoryMapSize = &BA2->MemoryMapSize;
    mBootArgs.MemoryMapDescriptorSize = &BA2->MemoryMapDescriptorSize;
    mBootArgs.MemoryMapDescriptorVersion = &BA2->MemoryMapDescriptorVersion;

    mBootArgs.CommandLine = &BA2->CommandLine[0];

    mBootArgs.deviceTreeP = &BA2->deviceTreeP;
    mBootArgs.deviceTreeLength = &BA2->deviceTreeLength;

    if (BA2->flags & kBootArgsFlagCSRActiveConfig) {
      mBootArgs.csrActiveConfig = &BA2->csrActiveConfig;
    }
  }

  return &mBootArgs;
}

CONST CHAR8 *
GetArgumentFromCommandLine (
  IN CONST CHAR8  *CommandLine,
  IN CONST CHAR8  *Argument,
  IN CONST UINTN  ArgumentLength
  )
{
  CHAR8 *Str;

  Str = AsciiStrStr (CommandLine, Argument);

  //
  // Invalidate found boot arg if:
  // - it is neither the beginning of Cmd, nor has space prefix            -> boot arg is a suffix of another arg
  // - it has neither space suffix, nor \0 suffix, and does not end with = -> boot arg is a prefix of another arg
  //
  if (!Str || (Str != CommandLine && *(Str - 1) != ' ') ||
      (Str[ArgumentLength] != ' ' && Str[ArgumentLength] != '\0' &&
       Str[ArgumentLength - 1] != '=')) {
    return NULL;
  }

  return Str;
}

VOID
RemoveArgumentFromCommandLine (
  IN OUT CHAR8        *CommandLine,
  IN     CONST CHAR8  *Argument
  )
{
  CHAR8 *Match = NULL;

  do {
    Match = AsciiStrStr (CommandLine, Argument);
    if (Match && (Match == CommandLine || *(Match - 1) == ' ')) {
      while (*Match != ' ' && *Match != '\0') {
        *Match++ = ' ';
      }
    }
  } while (Match != NULL);

  //
  // Write zeroes to reduce data leak
  //
  CHAR8 *Updated = CommandLine;

  while (CommandLine[0] == ' ') {
    CommandLine++;
  }

  while (CommandLine[0] != '\0') {
    while (CommandLine[0] == ' ' && CommandLine[1] == ' ') {
      CommandLine++;
    }

    *Updated++ = *CommandLine++;
  }

  ZeroMem (Updated, CommandLine - Updated);
}

BOOLEAN
AppendArgumentToCommandLine (
  IN OUT CHAR8        *CommandLine,
  IN     CONST CHAR8  *Argument,
  IN     CONST UINTN  ArgumentLength
  )
{
  UINTN Len = AsciiStrLen(CommandLine);

  //
  // Account for extra space.
  //
  if (Len + (Len > 0 ? 1 : 0) + ArgumentLength >= BOOT_LINE_LENGTH) {
    DEBUG ((DEBUG_INFO, "boot-args are invalid, ignoring\n"));
    return FALSE;
  }

  if (Len > 0) {
    CommandLine   += Len;
    *CommandLine++ = ' ';
  }

  AsciiStrnCpyS (CommandLine, ArgumentLength + 1, Argument, ArgumentLength + 1);
  return TRUE;
}
