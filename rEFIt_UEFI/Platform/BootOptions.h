/*
 * BootOptions.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_BOOTOPTIONS_H_
#define PLATFORM_BOOTOPTIONS_H_



/** Finds and returns pointer to specified DevPath node in DevicePath or NULL. */
EFI_DEVICE_PATH_PROTOCOL *
FindDevicePathNodeWithType (
  IN  EFI_DEVICE_PATH_PROTOCOL *DevicePath,
  IN  UINT8                    Type,
  IN  UINT8                    SubType      OPTIONAL
  );

//Parses BootXXXX (XXXX = BootNum) var (from BootOption->Variable) and returns it in BootOption.
EFI_STATUS
ParseBootOption (OUT BO_BOOT_OPTION  *BootOption);

/** Prints BootXXXX vars found listed in BootOrder, plus print others if AllBootOptions == TRUE. */
VOID
PrintBootOptions (
  IN  BOOLEAN AllBootOptions
  );

/** Prints BootOrder with DBG. */
VOID
PrintBootOrder (
    IN  UINT16 BootOrder[],
    IN  UINTN  BootOrderLen
                );

/** Reads BootXXXX (XXXX = BootNum) var, parses it and returns in BootOption.
 *  Caller is responsible for releasing BootOption->Variable with FreePool().
 */
EFI_STATUS
GetBootOption (
  IN      UINT16         BootNum,
     OUT  BO_BOOT_OPTION *BootOption
  );

/** Returns gEfiGlobalVariableGuid:BootOrder as UINT16 array and it's length (num of elements).
 *  Caller is responsible for releasing BootOrder mem (FreePool()).
 */
EFI_STATUS
GetBootOrder (
  OUT  UINT16 *BootOrder[],
  OUT  UINTN  *BootOrderLen
  );

/** Searches BootXXXX vars for entry that points to given FileDeviceHandle/FileName
 *  and returns BootNum (XXXX in BootXXXX variable name) and BootIndex (index in BootOrder)
 *  if found.
 */
EFI_STATUS
FindBootOptionForFile (
       IN      EFI_HANDLE FileDeviceHandle,
       IN      CHAR16     *FileName,
       OUT     UINT16     *BootNum,
       OUT     UINTN      *BootIndex
    );

/** Adds new boot option for given file system device handle FileDeviceHandle, file path FileName
 *  and Description, to be BootIndex in the list of options (0 based).
 *  If UseShortForm == TRUE, then only the hard drive media dev path will be used instead
 *  of full device path.
 *  Long (full) form:
 *   PciRoot(0x0)/Pci(0x1f,0x2)/Sata(0x1,0x0)/HD(1,GPT,96004846-a018-49ad-bc9f-4e5a340adc4b,0x800,0x64000)/\EFI\BOOT\File.efi
 *  Short form:
 *   HD(1,GPT,96004846-a018-49ad-bc9f-4e5a340adc4b,0x800,0x64000)/\EFI\BOOT\File.efi
 */
EFI_STATUS
AddBootOptionForFile (
                      IN  EFI_HANDLE FileDeviceHandle,
                      IN  CONST CHAR16     *FileName,
                      IN  BOOLEAN    UseShortForm,
                      IN  CONST CHAR16     *Description,
                      IN  UINT8      *OptionalData,
                      IN  UINTN      OptionalDataSize,
                      IN  UINTN      BootIndex,
                      OUT UINT16     *BootNum
                      );

/** Deletes boot option specified with BootNum (XXXX in BootXXXX var name). */
EFI_STATUS
DeleteBootOption (
  IN  UINT16 BootNum
  );


/** Deletes boot option for file specified with FileDeviceHandle and FileName. */
EFI_STATUS
DeleteBootOptionForFile (
  IN  EFI_HANDLE FileDeviceHandle,
  IN  CONST CHAR16     *FileName
  );

/** Deletes all boot option that points to a file which contains FileName in it's path. */
EFI_STATUS
DeleteBootOptionsContainingFile (
  IN  CHAR16 *FileName
  );



#endif /* PLATFORM_BOOTOPTIONS_H_ */
