/*
 * libeg/image.c
 * Image handling functions
 *
 * Copyright (c) 2006 Christoph Pfisterer
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

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include <Efi.h>

#include "libegint.h"
#include "lodepng.h"

#define MAX_FILE_SIZE (1024*1024*1024)

#ifndef DEBUG_ALL
#define DEBUG_IMG 1
#else
#define DEBUG_IMG DEBUG_ALL
#endif

#if DEBUG_IMG == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_IMG, __VA_ARGS__)
#endif

//
// Basic file operations should be separated into separate file
//
EFI_STATUS egLoadFile(const EFI_FILE* BaseDir, IN CONST CHAR16 *FileName,
                      OUT UINT8 **FileData, OUT UINTN *FileDataLength)
{
  EFI_STATUS          Status = EFI_NOT_FOUND;
  EFI_FILE*     FileHandle = 0;
  EFI_FILE_INFO       *FileInfo;
  UINT64              ReadSize;
  UINTN               BufferSize;
  UINT8               *Buffer;

  if (!BaseDir) {
    goto Error;
  }

  Status = BaseDir->Open(BaseDir, &FileHandle, (CHAR16*)FileName, EFI_FILE_MODE_READ, 0); // const missing const EFI_FILE*->Open
  if (EFI_ERROR(Status) || !FileHandle) {
    goto Error;
  }

  FileInfo = EfiLibFileInfo(FileHandle);
  if (FileInfo == NULL) {
    FileHandle->Close(FileHandle);
    goto Error;
  }
  ReadSize = FileInfo->FileSize;
  if (ReadSize > MAX_FILE_SIZE)
    ReadSize = MAX_FILE_SIZE;
  FreePool(FileInfo);

  BufferSize = (UINTN)ReadSize;   // was limited to 1 GB above, so this is safe
  Buffer = (UINT8 *) AllocatePool (BufferSize);
  if (Buffer == NULL) {
    FileHandle->Close(FileHandle);
    Status = EFI_OUT_OF_RESOURCES;
    goto Error;
  }

  Status = FileHandle->Read(FileHandle, &BufferSize, Buffer);
  FileHandle->Close(FileHandle);
  if (EFI_ERROR(Status)) {
    FreePool(Buffer);
    goto Error;
  }

  if(FileData) {
    *FileData = Buffer;
  }
  if (FileDataLength) {
    *FileDataLength = BufferSize;
  }
  return Status;
Error:
  if (FileData) {
    *FileData = NULL;
  }
  if (FileDataLength) {
    *FileDataLength = 0;
  }
  return Status;
}
//Slice - this is gEfiPartTypeSystemPartGuid
//static EFI_GUID ESPGuid = { 0xc12a7328, 0xf81f, 0x11d2, { 0xba, 0x4b, 0x00, 0xa0, 0xc9, 0x3e, 0xc9, 0x3b } };
//there is assumed only one ESP partition. What if there are two HDD gpt formatted?
EFI_STATUS egFindESP(OUT EFI_FILE** RootDir)
{
    EFI_STATUS          Status;
    UINTN               HandleCount = 0;
    EFI_HANDLE          *Handles;

    Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiPartTypeSystemPartGuid, NULL, &HandleCount, &Handles);
    if (!EFI_ERROR(Status) && HandleCount > 0) {
        *RootDir = EfiLibOpenRoot(Handles[0]);
        if (*RootDir == NULL)
            Status = EFI_NOT_FOUND;
        FreePool(Handles);
    }
    return Status;
}
//if (NULL, ...) then save to EFI partition
EFI_STATUS egSaveFile(const EFI_FILE* BaseDir OPTIONAL, IN CONST CHAR16 *FileName,
                      IN CONST void *FileData, IN UINTN FileDataLength)
{
  EFI_STATUS          Status;
  EFI_FILE*           FileHandle;
  UINTN               BufferSize;
  BOOLEAN             CreateNew = TRUE;
  CONST CHAR16        *p = FileName + StrLen(FileName);
  CHAR16              DirName[256];
  UINTN               dirNameLen;
  EFI_FILE*           espDir;

  if (BaseDir == NULL) {
    Status = egFindESP(&espDir);
    if (EFI_ERROR(Status)) {
      DBG("no ESP %s\n", efiStrError(Status));
      return Status;
    }
    BaseDir = espDir;
  }
    
  // syscl - make directory if not exist
  while (*p != L'\\' && p >= FileName) {
    // find the first '\\' traverse from the end to head of FileName
    p -= 1;
  }
  dirNameLen = p - FileName;
  StrnCpyS(DirName, 256, FileName, dirNameLen);
  DirName[dirNameLen] = L'\0';
  Status = BaseDir->Open(BaseDir, &FileHandle, DirName,
                           EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, EFI_FILE_DIRECTORY);
    
  if (EFI_ERROR(Status)) {
      // make dir
//    DBG("no dir %s\n", efiStrError(Status));
      Status = BaseDir->Open(BaseDir, &FileHandle, DirName,
                               EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, EFI_FILE_DIRECTORY);
//    DBG("cant make dir %s\n", efiStrError(Status));
  }
  // end of folder checking

  // Delete existing file if it exists
  Status = BaseDir->Open(BaseDir, &FileHandle, FileName,
                         EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
  if (!EFI_ERROR(Status)) {
    Status = FileHandle->Delete(FileHandle);
    if (Status == EFI_WARN_DELETE_FAILURE) {
      //This is READ_ONLY file system
      CreateNew = FALSE; // will write into existing file (Slice - ???)
//      DBG("RO FS %s\n", efiStrError(Status));
    }
  }

  if (CreateNew) {
    // Write new file
    Status = BaseDir->Open(BaseDir, &FileHandle, FileName,
                           EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
    if (EFI_ERROR(Status)) {
//      DBG("no write %s\n", efiStrError(Status));
      return Status;
    }
  } else {
    //to write into existing file we must sure it size larger then our data
//    EFI_FILE_INFO *Info = EfiLibFileInfo(FileHandle);
//    if (Info) {
//      if (Info->FileSize < FileDataLength) {
//        DBG("no old file %s\n", efiStrError(Status));
//        return EFI_NOT_FOUND;
//      }
//      FreePool(Info);
//    }
    Status = FileHandle->Delete(FileHandle); //don't write into existing file. Delete it!
  }

  if (!FileHandle) {
//    DBG("no FileHandle %s\n", efiStrError(Status));
    return EFI_DEVICE_ERROR;
  }

  BufferSize = FileDataLength;
  Status = FileHandle->Write(FileHandle, &BufferSize, (void*)FileData); // CONST missing const EFI_FILE*->write
  FileHandle->Close(FileHandle);
//  DBG("not written %s\n", efiStrError(Status));
  return Status;
}


EFI_STATUS egMkDir(const EFI_FILE* BaseDir OPTIONAL, const CHAR16 *DirName)
{
  EFI_STATUS          Status;
  EFI_FILE*     FileHandle;

  //DBG("Looking up dir assets (%ls):", DirName);

  if (BaseDir == NULL) {
    EFI_FILE* espDir;
    Status = egFindESP(&espDir);
    if (EFI_ERROR(Status)) {
      //DBG(" %s\n", efiStrError(Status));
      return Status;
    }
    BaseDir = espDir;
  }

  Status = BaseDir->Open(BaseDir, &FileHandle, DirName,
                         EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, EFI_FILE_DIRECTORY);

  if (EFI_ERROR(Status)) {
    // Write new dir
    //DBG("%s, attempt to create one:", efiStrError(Status));
    Status = BaseDir->Open(BaseDir, &FileHandle, DirName,
                           EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, EFI_FILE_DIRECTORY);
  }

  //DBG(" %s\n", efiStrError(Status));
  return Status;
}



/* EOF */
