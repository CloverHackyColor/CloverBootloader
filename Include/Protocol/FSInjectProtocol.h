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


/**
 * FSINJECTION_PROTOCOL.Install() type definition
 * @param TgtHandle     target volume handler
 * @param TgtDir        dir on target volume where content of SrcDir will be injected
 * @param SrcHandle     volume where SrcDir exists
 * @param SrcDir        dir whose content will be injected
 * @param BlacklistCnt  number of file names listed in Blacklist
 * @param Blacklist     list of file names that will be blocked on target volume
 */
typedef EFI_STATUS(EFIAPI * FSINJECTION_INSTALL)(IN EFI_HANDLE TgtHandle, IN CHAR16 *TgtDir, IN EFI_HANDLE SrcHandle, IN CHAR16 *SrcDir, IN UINTN BlacklistCnt, CHAR16 *Blacklist[]);

/**
 * FSINJECTION_PROTOCOL that can be used to install FSInjection to some existing volume handle
 */
typedef struct {
	FSINJECTION_INSTALL		Install;	// installs FSInjection EFI_SIMPLE_FILE_SYSTEM_PROTOCOL
} FSINJECTION_PROTOCOL;

#define FSINJECTION_PROTOCOL_GUID \
  { \
    0x3F048284, 0x6D4C, 0x11E1, {0xA4, 0xD7, 0x37, 0xE3, 0x48, 0x24, 0x01, 0x9B } \
  }

/** FSINJECTION_PROTOCOL GUID */
extern EFI_GUID gFSInjectProtocolGuid;


#endif