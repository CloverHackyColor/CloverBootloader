/*
 * refit/scan/uefibootoption.c
 *
 * Copyright (c) 2006-2010 Christoph Pfisterer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *  * Neither the name of Christoph Pfisterer nor the names of the
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 
 VOID AddUEFIBootOption(BO_BOOT_OPTION *bootOption)
 {
 UINTN                       volumeIndex;
 REFIT_VOLUME                *volume;
 EFI_DEVICE_PATH_PROTOCOL    *devPathNode;
 LOADER_ENTRY                *loaderEntry;
 FILEPATH_DEVICE_PATH        *loaderDevicePath;
 CHAR16                      *loaderPath;
 CHAR16                      *bootParams;
 CHAR16                      *title;
 EFI_GUID                    *volumeGUID;
 EFI_GUID                    *bootGUID;
 
 for (volumeIndex = 0; volumeIndex < VolumesCount; volumeIndex++) {
 volume = Volumes[volumeIndex];
 volumeGUID = FindGPTPartitionGuidInDevicePath(volume->DevicePath);
 bootGUID = FindGPTPartitionGuidInDevicePath(bootOption->FilePathList);
 if (!volumeGUID || !bootGUID || !CompareGuid(volumeGUID, bootGUID)) continue;
 
 devPathNode = bootOption->FilePathList;
 loaderDevicePath = (FILEPATH_DEVICE_PATH *) FindDevicePathNodeWithType(devPathNode, MEDIA_DEVICE_PATH,
 MEDIA_FILEPATH_DP);
 if (!loaderDevicePath) continue;
 
 loaderPath = loaderDevicePath->PathName;
 bootParams = (CHAR16 *) bootOption->OptionalData;
 title = bootOption->Description;
 
 volume->OSIconName = L"unknown";
 volume->BootType = BOOTING_BY_EFI;
 
 loaderEntry = AddLoaderEntry(loaderPath, title, volume, OSTYPE_EFI);
 loaderEntry->LoadOptions = EfiStrDuplicate(bootParams);
 }
 }
 
 VOID ScanUEFIBootOptions(BOOLEAN allBootOptions)
 {
 EFI_STATUS          status;
 UINT16              *bootOrder;
 UINTN               bootOrderCnt;
 UINT16              i;
 BO_BOOT_OPTION      bootOption;
 
 DBG("Scanning boot options from UEFI ...\n");
 
 if (allBootOptions) {
 for (i = 0; i <= 0xFFFF; i++) {
 status = GetBootOption(i, &bootOption);
 if (EFI_ERROR(status)) continue;
 
 AddUEFIBootOption(&bootOption);
 
 FreePool(bootOption.Variable);
 }
 } else {
 status = GetBootOrder(&bootOrder, &bootOrderCnt);
 if (EFI_ERROR(status)) return;
 
 for (i = 0; i < bootOrderCnt; i++) {
 status = GetBootOption(bootOrder[i], &bootOption);
 if (EFI_ERROR(status)) continue;
 
 AddUEFIBootOption(&bootOption);
 
 FreePool(bootOption.Variable);
 }
 
 FreePool(bootOrder);
 }
 }
 
 // */