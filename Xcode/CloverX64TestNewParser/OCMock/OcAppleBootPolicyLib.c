//
//  OcAppleBootPolicyLib.c
//  cpp_tests_compare_settings UTF16 signed char
//
//  Created by Jief on 05/02/2021.
//  Copyright © 2021 JF Knudsen. All rights reserved.
//

#include <Efi.h>
#include <Guid/AppleApfsInfo.h>

EFI_STATUS
InternalGetApfsSpecialFileInfo (
  IN     EFI_FILE_PROTOCOL          *Root,
  IN OUT APPLE_APFS_VOLUME_INFO     **VolumeInfo OPTIONAL,
  IN OUT APPLE_APFS_CONTAINER_INFO  **ContainerInfo OPTIONAL
  )
{
  panic("not yet");
}
