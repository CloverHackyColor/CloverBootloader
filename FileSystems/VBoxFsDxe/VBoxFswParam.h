/* $Id: VBoxFswParam.h 29125 2010-05-06 09:43:05Z vboxsync $ */
/** @file
 * VBoxFswParam.h
 */

/*
 * Copyright (C) 2010 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 */

#ifndef VBOXFSPARAM_H
#define VBOXFSPARAM_H
/*
 * Here is common declarations for EDK<->EDK2 compatibility
 */
# include <Base.h>
# include <Uefi.h>

# include <Library/DebugLib.h>
# include <Library/BaseLib.h>
# include <Library/BaseMemoryLib.h>
# include <Library/UefiRuntimeServicesTableLib.h>
# include <Library/UefiDriverEntryPoint.h>
# include <Library/UefiBootServicesTableLib.h>
# include <Library/MemoryAllocationLib.h>
# include <Library/DevicePathLib.h>
# include <Library/PrintLib.h>
# include <Library/UefiLib.h>

# include <Protocol/DriverBinding.h>
# include <Protocol/DevicePathFromText.h>
# include <Protocol/DevicePathToText.h>
# include <Protocol/DebugPort.h>
# include <Protocol/DebugSupport.h>
# include <Protocol/SimpleFileSystem.h>
# include <Protocol/BlockIo.h>
# include <Protocol/BlockIo2.h>
# include <Protocol/DiskIo.h>
# include <Protocol/DiskIo2.h>
# include <Protocol/ComponentName.h>
# include <Protocol/ComponentName2.h>

# include <Guid/FileSystemInfo.h>
# include <Guid/FileInfo.h>
# include <Guid/FileSystemVolumeLabelInfo.h>

# define BS gBS
# define PROTO_NAME(x) gEfi ## x ## Guid
# define GUID_NAME(x) gEfi ## x ## Guid

# define EFI_FILE_HANDLE_REVISION EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION
# define SIZE_OF_EFI_FILE_SYSTEM_VOLUME_LABEL_INFO  SIZE_OF_EFI_FILE_SYSTEM_VOLUME_LABEL
# define EFI_FILE_SYSTEM_VOLUME_LABEL_INFO EFI_FILE_SYSTEM_VOLUME_LABEL
# define EFI_SIGNATURE_32(a, b, c, d) SIGNATURE_32(a, b, c, d)
# define DivU64x32(x,y,z) DivU64x32((x),(y))


INTN CompareGuidEdk1(
  IN EFI_GUID     *Guid1,
  IN EFI_GUID     *Guid2
                     );

//#define CompareGuid(x, y) CompareGuidEdk1((x),(y))
# define HOST_EFI 1
//# define FSW_DEBUG_LEVEL 3

int fsw_streq_ISO88591_UTF16(void *s1data, void *s2data, int len);
#endif
