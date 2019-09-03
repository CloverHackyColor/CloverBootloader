/** @file

  Simple File System overrides module.

  By dmazar, 01/02/2014

**/

#ifndef __DMP_FS_H__
#define __DMP_FS_H__


/**
 * Our EFI_SIMPLE_FILE_SYSTEM_PROTOCOL private structure
 */
typedef struct {
	UINT64								Signature;		// identification signature
	
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL		Fs;				// public EFI_SIMPLE_FILE_SYSTEM_PROTOCOL
	
	EFI_HANDLE							*Handle;		// handle we are attached to
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL		*OrigFs;		// original FS we are replacing
} DUC_SIMPLE_FILE_SYSTEM_PROTOCOL;

/** Signature for DUC_SIMPLE_FILE_SYSTEM_PROTOCOL */
#define DUC_SIMPLE_FILE_SYSTEM_PROTOCOL_SIGNATURE  SIGNATURE_32('d', 'u', 'f', 's')

/** Macro to access DUC_SIMPLE_FILE_SYSTEM_PROTOCOL from public Fs */
#define DUC_FROM_SIMPLE_FILE_SYSTEM(a)  CR (a, DUC_SIMPLE_FILE_SYSTEM_PROTOCOL, Fs, DUC_SIMPLE_FILE_SYSTEM_PROTOCOL_SIGNATURE)


/**
 *  Our EFI_FILE_PROTOCOL private structure.
 */
typedef struct {
	UINT64								Signature;		// identification signature
	
	EFI_FILE_PROTOCOL					Fp;				// public EFI_FILE_PROTOCOL
    
	CHAR16								*FName;			// file name this structure represents
	EFI_FILE_PROTOCOL					*OrigFp;		// target EFI_FILE_PROTOCOL
} DUC_FILE_PROTOCOL;

/** Signature for DUC_FILE_PROTOCOL */
#define DUC_FILE_PROTOCOL_SIGNATURE  SIGNATURE_32('d', 'u', 'f', 'p')

/** Macro to access DUC_FILE_PROTOCOL from public FP */
#define DUC_FROM_FILE_PROTOCOL(a)  CR (a, DUC_FILE_PROTOCOL, Fp, DUC_FILE_PROTOCOL_SIGNATURE)





/** Installs our Simple File System overrides. */
EFI_STATUS EFIAPI
OvrFs(VOID);


#endif // __DMP_FS_H__
