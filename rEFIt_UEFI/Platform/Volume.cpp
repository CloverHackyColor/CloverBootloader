/*
 * Volume.cpp
 *
 *  Created on: Apr 21, 2021
 *      Author: jief
 */

#include "Volume.h"
#include <Platform.h>
#include "../Platform/plist/plist.h"
#include "../Platform/guid.h"
#include "../refit/lib.h"

//Get the UUID of the AppleRaid or CoreStorage volume from the boot helper partition
EFI_STATUS
GetRootUUID (IN  REFIT_VOLUME *Volume)
{
  EFI_STATUS Status;
  CHAR8      *PlistBuffer;
  UINTN      PlistLen;
  TagDict*     Dict;
  const TagStruct*     Prop;

  CONST CHAR16*    SystemPlistR;
  CONST CHAR16*    SystemPlistP;
  CONST CHAR16*    SystemPlistS;

  BOOLEAN    HasRock;
  BOOLEAN    HasPaper;
  BOOLEAN    HasScissors;

  Status = EFI_NOT_FOUND;
  if (Volume == NULL) {
    return EFI_NOT_FOUND;
  }

  SystemPlistR = L"\\com.apple.boot.R\\Library\\Preferences\\SystemConfiguration\\com.apple.Boot.plist";
  if (FileExists (Volume->RootDir, SystemPlistR)) {
    HasRock      = FileExists (Volume->RootDir,     SystemPlistR);
  } else {
    SystemPlistR = L"\\com.apple.boot.R\\com.apple.Boot.plist";
    HasRock      = FileExists (Volume->RootDir,     SystemPlistR);
  }

  SystemPlistP = L"\\com.apple.boot.P\\Library\\Preferences\\SystemConfiguration\\com.apple.Boot.plist";
  if (FileExists (Volume->RootDir, SystemPlistP)) {
    HasPaper     = FileExists (Volume->RootDir,     SystemPlistP);
  } else {
    SystemPlistP = L"\\com.apple.boot.P\\com.apple.Boot.plist";
    HasPaper     = FileExists (Volume->RootDir,     SystemPlistP);
  }

  SystemPlistS = L"\\com.apple.boot.S\\Library\\Preferences\\SystemConfiguration\\com.apple.Boot.plist";
  if (FileExists (Volume->RootDir, SystemPlistS)) {
    HasScissors  = FileExists (Volume->RootDir,     SystemPlistS);
  } else {
    SystemPlistS = L"\\com.apple.boot.S\\com.apple.Boot.plist";
    HasScissors  = FileExists (Volume->RootDir,     SystemPlistS);
  }

  PlistBuffer = NULL;
  // Playing Rock, Paper, Scissors to chose which settings to load.
  if (HasRock && HasPaper && HasScissors) {
    // Rock wins when all three are around
    Status = egLoadFile(Volume->RootDir, SystemPlistR, (UINT8 **)&PlistBuffer, &PlistLen);
  } else if (HasRock && HasPaper) {
    // Paper beats rock
    Status = egLoadFile(Volume->RootDir, SystemPlistP, (UINT8 **)&PlistBuffer, &PlistLen);
  } else if (HasRock && HasScissors) {
    // Rock beats scissors
    Status = egLoadFile(Volume->RootDir, SystemPlistR, (UINT8 **)&PlistBuffer, &PlistLen);
  } else if (HasPaper && HasScissors) {
    // Scissors beat paper
    Status = egLoadFile(Volume->RootDir, SystemPlistS, (UINT8 **)&PlistBuffer, &PlistLen);
  } else if (HasPaper) {
    // No match
    Status = egLoadFile(Volume->RootDir, SystemPlistP, (UINT8 **)&PlistBuffer, &PlistLen);
  } else if (HasScissors) {
    // No match
    Status = egLoadFile(Volume->RootDir, SystemPlistS, (UINT8 **)&PlistBuffer, &PlistLen);
  } else {
    // Rock wins by default
    Status = egLoadFile(Volume->RootDir, SystemPlistR, (UINT8 **)&PlistBuffer, &PlistLen);
  }

  if (!EFI_ERROR(Status)) {
    Dict = NULL;
    if (ParseXML(PlistBuffer, &Dict, 0) != EFI_SUCCESS) {
      FreePool(PlistBuffer);
      return EFI_NOT_FOUND;
    }

    Prop = Dict->propertyForKey("Root UUID");
    if ( Prop != NULL ) {
      if ( !Prop->isString() ) {
        MsgLog("ATTENTION : property not string in Root UUID\n");
      }else{
        Status = StrToGuidBE(Prop->getString()->stringValue(), &Volume->RootUUID);
      }
    }

    Dict->FreeTag();
    FreePool(PlistBuffer);
  }

  return Status;
}
