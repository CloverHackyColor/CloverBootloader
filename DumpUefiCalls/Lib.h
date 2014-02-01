/** @file

  Various helper functions.

  By dmazar, 26/09/2012             

**/

#ifndef __DMP_LIB_H__
#define __DMP_LIB_H__


/** Memory type to string conversion */
extern CHAR16 *EfiMemoryTypeDesc[EfiMaxMemoryType];

/** Allocation type to string conversion */
extern CHAR16 *EfiAllocateTypeDesc[MaxAllocateType];

/** Locate search type to string conversion */
extern CHAR16 *EfiLocateSearchType[];

/** Reset type to string conversion */
extern CHAR16 *EfiResetType[];

extern EFI_GUID mEfiApplePlatformInfoGuid;


//
// MemMap reversed scan
//
#define PREV_MEMORY_DESCRIPTOR(MemoryDescriptor, Size) \
  ((EFI_MEMORY_DESCRIPTOR *)((UINT8 *)(MemoryDescriptor) - (Size)))


/** GUID - friendly name mapping */
typedef struct {
	EFI_GUID 	*Guid;
	CHAR16		*Str;
} MAP_EFI_GUID_STR;

/** Print buffer for unknown GUID printing. Allocated as RT mem
 *  and gets converted in RuntimeServices.c VirtualAddressChangeEvent().
 */
extern CHAR16	*GuidPrintBuffer;


/** Buffer for RT variable names. Allocated as RT mem
 *  and gets converted in RuntimeServices.c VirtualAddressChangeEvent().
 */
extern CHAR16 *gVariableNameBuffer;

/** Buffer for RT variable data. Allocated as RT mem
 *  and gets converted in RuntimeServices.c VirtualAddressChangeEvent().
 */
extern CHAR8  *gVariableDataBuffer;



/** Returns GUID as string, with friendly name for known guids. */
CHAR16*
EFIAPI
GuidStr(IN EFI_GUID *Guid);

/** Prints series of bytes. */
VOID
EFIAPI
PrintBytes(IN CHAR8 *Bytes, IN UINTN Number);

/** Returns pointer to last Char in String or NULL. */
CHAR16*
EFIAPI
GetStrLastChar(IN CHAR16 *String);

/** Returns pointer to last occurence of Char in String or NULL. */
CHAR16*
EFIAPI
GetStrLastCharOccurence(IN CHAR16 *String, IN CHAR16 Char);

/** Returns upper case version of char - valid only for ASCII chars in unicode. */
CHAR16
EFIAPI
ToUpperChar(IN CHAR16 Chr);


/** Returns 0 if two strings are equal, !=0 otherwise. Compares just first 8 bits of chars (valid for ASCII), case insensitive. */
UINTN
EFIAPI
StrCmpiBasic(IN CHAR16 *String1, IN CHAR16 *String2);

/** Returns the first occurrence of a Null-terminated Unicode SearchString
  * in a Null-terminated Unicode String.
  * Compares just first 8 bits of chars (valid for ASCII), case insensitive.
  * Copied from MdePkg/Library/BaseLib/String.c and modified
  */
CHAR16*
EFIAPI
StrStriBasic(IN CONST CHAR16 *String, IN CONST CHAR16 *SearchString);

/** Returns TRUE if String1 starts with String2, FALSE otherwise. Compares just first 8 bits of chars (valid for ASCII), case insensitive. */
BOOLEAN
EFIAPI
StriStartsWithBasic(IN CHAR16 *String1, IN CHAR16 *String2);

/** Shrinks mem map by joining EfiBootServicesCode and EfiBootServicesData records. */
VOID EFIAPI
ShrinkMemMap(
	IN UINTN					*MemoryMapSize,
	IN EFI_MEMORY_DESCRIPTOR	*MemoryMap,
	IN UINTN					DescriptorSize,
	IN UINT32					DescriptorVersion
);

/** Prints mem map. */
VOID EFIAPI
PrintMemMap(
	IN UINTN					MemoryMapSize,
	IN EFI_MEMORY_DESCRIPTOR	*MemoryMap,
	IN UINTN					DescriptorSize,
	IN UINT32					DescriptorVersion
);

/** Prints some values from Sys table and Runt. services. */
VOID EFIAPI
PrintSystemTable(IN EFI_SYSTEM_TABLE  *ST);

/** Prints Message and waits for a key press. */
VOID
WaitForKeyPress(CHAR16 *Message);

/** Returns file path from FilePath device path in allocated memory. Mem should be released by caller.*/
CHAR16 *
EFIAPI
FileDevicePathToText(EFI_DEVICE_PATH_PROTOCOL *FilePathProto);

/** Helper function that calls GetMemoryMap(), allocates space for mem map and returns it. */
EFI_STATUS
EFIAPI
GetMemoryMapAlloc (
	IN EFI_GET_MEMORY_MAP			GetMemoryMapFunction,
	OUT UINTN						*MemoryMapSize,
	OUT EFI_MEMORY_DESCRIPTOR		**MemoryMap,
	OUT UINTN						*MapKey,
	OUT UINTN						*DescriptorSize,
	OUT UINT32						*DescriptorVersion
);

/** Alloctes Pages from the top of mem, up to address specified in Memory. Returns allocated address in Memory. */
EFI_STATUS
EFIAPI
AllocatePagesFromTop(IN EFI_MEMORY_TYPE MemoryType, IN UINTN Pages, IN OUT EFI_PHYSICAL_ADDRESS *Memory);

/** Prints RT vars. */
VOID
EFIAPI
PrintRTVariables(IN EFI_RUNTIME_SERVICES *RT);


#endif // __DMP_LIB_H__
