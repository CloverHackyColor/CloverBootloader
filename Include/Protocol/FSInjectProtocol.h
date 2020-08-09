/** @file

Module Name:

  FSInjectProtocol.h

  FSInject driver - Replaces EFI_SIMPLE_FILE_SYSTEM_PROTOCOL on target volume
  and injects content of specified source folder on source (injection) volume
  into target folder in target volume.

  initial version - dmazar

**/

#ifndef __FSInjectProtocol_H__
#define __FSInjectProtocol_H__


/** String list entry. */
typedef struct {
	LIST_ENTRY	List;		// must be first in struct
	CHAR16		String[1];
} FSI_STRING_LIST_ENTRY;

typedef FSI_STRING_LIST_ENTRY FSI_STRING_LIST;


/**
 * FSINJECTION_PROTOCOL.Install() type definition
 * @param TgtHandle       target volume handler
 * @param TgtDir          dir on target volume where content of SrcDir will be injected
 * @param SrcHandle       volume where SrcDir exists
 * @param SrcDir         dir whose content will be injected
 * @param Blacklist      list of file names that will be blocked on target volume; caller should not release allocated list memory after this call.
 * @param ForceLoadKexts list of kexts (paths to their Info.plists, like L"\\ATI5000Controller.kext\\Contents\\Info.plist")
 *                       that we'll force to be loaded by boot.efi by changing OSBundleRequired to Root
 */
typedef EFI_STATUS (EFIAPI * FSINJECTION_INSTALL)(IN EFI_HANDLE TgtHandle, IN JCONST CHAR16 *TgtDir, IN EFI_HANDLE SrcHandle, IN JCONST CHAR16 *SrcDir, IN FSI_STRING_LIST *Blacklist, IN FSI_STRING_LIST *ForceLoadKexts);

/**
 * FSINJECTION_PROTOCOL.CreateStringList() type definition
 * Creates new string list. List can be populated with FSINJECTION_PROTOCOL.AddStringToList()
 * @return Created list or NULL if there is no memory.
 */
typedef FSI_STRING_LIST* (EFIAPI *FSINJECTION_CREATE_STRING_LIST) (VOID);


/**
 * FSINJECTION_PROTOCOL.AddStringToList() type definition
 * @param List			List created with CreateStringList()
 * @param String        String to add
 * @return List if ok, or NULL if no memory
 */
typedef FSI_STRING_LIST* (EFIAPI *FSINJECTION_ADD_STRING_TO_LIST) (FSI_STRING_LIST *List, JCONST CHAR16 *String);


/**
 * FSINJECTION_PROTOCOL that can be used to install FSInjection to some existing volume handle
 */
typedef struct _FSINJECTION_PROTOCOL {
	FSINJECTION_INSTALL				Install;			// installs FSInjection EFI_SIMPLE_FILE_SYSTEM_PROTOCOL
	FSINJECTION_CREATE_STRING_LIST	CreateStringList;	// creates new string list
	FSINJECTION_ADD_STRING_TO_LIST	AddStringToList;	// adds list to string
} FSINJECTION_PROTOCOL;

#define FSINJECTION_PROTOCOL_GUID \
  { \
    0x3F048284, 0x6D4C, 0x11E1, {0xA4, 0xD7, 0x37, 0xE3, 0x48, 0x24, 0x01, 0x9B } \
  }

/** FSINJECTION_PROTOCOL GUID */
extern EFI_GUID gFSInjectProtocolGuid;


#endif
