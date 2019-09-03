/**

  Various helper functions.

  by dmazar

**/


/** For type to string conversion */
extern CHAR16 *EfiMemoryTypeDesc[EfiMaxMemoryType];
extern CHAR16 *EfiAllocateTypeDesc[MaxAllocateType];
extern CHAR16 *EfiLocateSearchType[];

extern EFI_GUID gVendorGuid;

// MemMap reversed scan
#define PREV_MEMORY_DESCRIPTOR(MemoryDescriptor, Size) \
  ((EFI_MEMORY_DESCRIPTOR *)((UINT8 *)(MemoryDescriptor) - (Size)))


/** Map of known guids and friendly names. Searchable with GuidStr() */
typedef struct {
	EFI_GUID 	*Guid;
	CHAR16		*Str;
} MAP_EFI_GUID_STR;

#define EFI_DATA_HUB_PROTOCOL_GUID \
{ 0xae80d021, 0x618e, 0x11d4, {0xbc, 0xd7, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 } }

#define EFI_CONSOLE_CONTROL_PROTOCOL_GUID \
{ 0xf42f7782, 0x12e, 0x4c12, {0x99, 0x56, 0x49, 0xf9, 0x43, 0x4, 0xf7, 0x21} }

#define EFI_LEGACY_8259_PROTOCOL_GUID \
{ 0x38321dba, 0x4fe0, 0x4e17, {0x8a, 0xec, 0x41, 0x30, 0x55, 0xea, 0xed, 0xc1} }

extern EFI_GUID gEfiAppleBootGuid;

/** Returns GUID as string, with friendly name for known guids. */
CHAR16*
EFIAPI
GuidStr(IN EFI_GUID *Guid);

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


/** Returns the first occurrence of a SearchString in a String.
  * Compares just first 8 bits of chars (valid for ASCII), case insensitive.
  */
CHAR16*
EFIAPI
StrStriBasic(IN CONST CHAR16 *String, IN CONST CHAR16 *SearchString);

/** Returns TRUE if String1 starts with String2, FALSE otherwise. Compares just first 8 bits of chars (valid for ASCII), case insensitive. */
BOOLEAN
EFIAPI
StriStartsWithBasic(IN CHAR16 *String1, IN CHAR16 *String2);

/** Applies some fixes to mem map. */
VOID EFIAPI
FixMemMap(
	IN UINTN					MemoryMapSize,
	IN EFI_MEMORY_DESCRIPTOR	*MemoryMap,
	IN UINTN					DescriptorSize,
	IN UINT32					DescriptorVersion
);
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
