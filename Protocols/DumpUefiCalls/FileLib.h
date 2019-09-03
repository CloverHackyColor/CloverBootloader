/** @file

  File system helper functions.

  By dmazar, 26/09/2012             

**/

#ifndef __DMP_FILE_LIB_H__
#define __DMP_FILE_LIB_H__


#include <Protocol/SimpleFileSystem.h>


/** Retrieves loaded image protocol from our image. */
VOID
FsGetLoadedImage(VOID);

/** Returns file system protocol from specified volume device. */
EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *
FsGetFileSystem(IN EFI_HANDLE VolumeHandle);

/** Returns file system protocol from volume device we are loaded from. */
EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *
FsGetSelfFileSystem(VOID);

/** Returns root dir from given file system. */
EFI_FILE_PROTOCOL *
FsGetRootDir(IN EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Volume);

/** Returns root dir file from file system we are loaded from. */
EFI_FILE_PROTOCOL *
FsGetSelfRootDir(VOID);

/** Returns dir from file system we are loaded from. */
EFI_FILE_PROTOCOL *
FsGetSelfDir(VOID);

/** Finds first EFI partition and returns it's root dir. */
EFI_FILE_PROTOCOL *
FsGetEspRootDir(VOID);

/** Checks if file exists. */
BOOLEAN
FsFileExists(
	IN EFI_FILE_PROTOCOL	*Dir,
	IN CHAR16				*FileName
);

/** Checks if file exists. */
EFI_STATUS
FsDeleteFile(
	IN EFI_FILE_PROTOCOL	*Dir,
	IN CHAR16				*FileName
);

/** Saves memory block to a file. */
EFI_STATUS
FsSaveMemToFile(
	IN EFI_FILE_PROTOCOL	*Dir,
	IN CHAR16				*FileName,
	IN VOID					*Data,
	IN UINTN				DataSize
);

/** Saves memory block to a file. Tries to save in "self dir",
 *  and if this is not possible then to first ESP/EFI partition.
 */
EFI_STATUS
FsSaveMemToFileToDefaultDir(
	IN CHAR16			*FileName,
	IN VOID				*Data,
	IN UINTN			DataSize
);

/** Appends memory block to a file. */
EFI_STATUS
FsAppendMemToFile(
	IN EFI_FILE_PROTOCOL	*Dir,
	IN CHAR16				*FileName,
	IN VOID					*Data,
	IN UINTN				DataSize
);

/** Appends memory block to a file. Tries to save in "self dir",
 *  and if this is not possible then to first ESP/EFI partition.
 */
EFI_STATUS
FsAppendMemToFileToDefaultDir(
	IN CHAR16			*FileName,
	IN VOID				*Data,
	IN UINTN			DataSize
);

/* Closes previously opened file/dir used for appending */
EFI_STATUS
FsAppendMemClose(
	IN BOOLEAN 			CloseDir
);

#endif // __DMP_FILE_LIB_H__
