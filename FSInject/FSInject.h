/** @file

Module Name:

  FSInject.h
  
  Definitions for private EFI_SIMPLE_FILE_SYSTEM_PROTOCOL and EFI_FILE_PROTOCOL implementations.

  initial version - dmazar
  
**/

#ifndef __FSInject_H__
#define __FSInject_H__

/**
 * FSInjection EFI_SIMPLE_FILE_SYSTEM_PROTOCOL private structure
 */
typedef struct {
	UINT64								Signature;		// identification signature
	
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL		FS;				// public EFI_SIMPLE_FILE_SYSTEM_PROTOCOL
	
	EFI_HANDLE							*TgtHandle;		// target handle we are attached to
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL		*TgtFS;			// target FS we are replacing
	CHAR16								*TgtDir;		// target dir where injection will be done

	EFI_HANDLE							*SrcHandle;		// handle where injection dir is
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL		*SrcFS;			// FS with injection dir we are replacing
	CHAR16								*SrcDir;		// injection dir that contains files that will be injected into TgtDir
	
	FSI_STRING_LIST						*Blacklist;		// linked list of file names to be blocked on target volume
	FSI_STRING_LIST						*ForceLoadKexts;// linked list of kext plists
} FSI_SIMPLE_FILE_SYSTEM_PROTOCOL;

/** Signature for FSI_SIMPLE_FILE_SYSTEM_PROTOCOL */
#define FSI_SIMPLE_FILE_SYSTEM_PROTOCOL_SIGNATURE  SIGNATURE_32('f', 'i', 'f', 's')

/** Macro to access FSI_SIMPLE_FILE_SYSTEM_PROTOCOL from public FS */
#define FSI_FROM_SIMPLE_FILE_SYSTEM(a)  CR (a, FSI_SIMPLE_FILE_SYSTEM_PROTOCOL, FS, FSI_SIMPLE_FILE_SYSTEM_PROTOCOL_SIGNATURE)



/**
 *  FSInjection EFI_FILE_PROTOCOL private structure.
 */
typedef struct {
	UINT64								Signature;		// identification signature
	
	EFI_FILE_PROTOCOL					FP;				// public EFI_FILE_PROTOCOL

	FSI_SIMPLE_FILE_SYSTEM_PROTOCOL		*FSI_FS;		// FS protocol that created this structure
	CHAR16								*FName;			// file name this structure represents
	BOOLEAN								IsDir;			// true after GetInfo if this file is directory
	EFI_FILE_PROTOCOL					*TgtFP;			// target EFI_FILE_PROTOCOL
	EFI_FILE_PROTOCOL					*SrcFP;			// EFI_FILE_PROTOCOL from injection volume
	BOOLEAN								FromTgt;		// TRUE if file is opened from original target volume, FALSE if from injection volume
} FSI_FILE_PROTOCOL;

/** Signature for FSI_FILE_PROTOCOL */
#define FSI_FILE_PROTOCOL_SIGNATURE  SIGNATURE_32('f', 'i', 'f', 'p')

/** Macro to access FSI_FILE_PROTOCOL from public FP */
#define FSI_FROM_FILE_PROTOCOL(a)  CR (a, FSI_FILE_PROTOCOL, FP, FSI_FILE_PROTOCOL_SIGNATURE)


#endif