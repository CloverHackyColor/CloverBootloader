/*
 * refit/scan/securehash.c
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

#include "entry_scan.h"

#ifndef DEBUG_ALL
#define DEBUG_SECURE_HASH 1
#else
#define DEBUG_SECURE_HASH DEBUG_ALL
#endif

#if DEBUG_SECURE_HASH == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_SECURE_HASH, __VA_ARGS__)
#endif

// TODO: Add image signature list
EFI_STATUS AddImageSignatureList(IN VOID  *SignatureList,
                                 IN UINTN  SignatureListSize)
{
  // Check parameters
  if ((SignatureList == NULL) || (SignatureListSize == 0)) {
    return EFI_INVALID_PARAMETER;
  }
  return EFI_ABORTED;
}

// TODO: Remove image signatures
EFI_STATUS RemoveImageSignatureList(IN VOID  *SignatureList,
                                    IN UINTN  SignatureListSize)
{
  // Check parameters
  if ((SignatureList == NULL) || (SignatureListSize == 0)) {
    return EFI_INVALID_PARAMETER;
  }
  return EFI_ABORTED;
}

// TODO: Clear signatures database
EFI_STATUS ClearImageSignatureDatabase(VOID)
{
   return EFI_ABORTED;
}

// TODO: Create a secure boot image signature
STATIC VOID *CreateImageSignatureList(IN VOID   *FileBuffer,
                                      IN UINT64  FileSize,
                                      IN UINTN  *SignatureListSize)
{
  // Check parameters
  if (SignatureListSize == 0) {
    return NULL;
  }
  *SignatureListSize = 0;
  if ((FileBuffer == NULL) || (FileSize == 0)) {
    return NULL;
  }
  // TODO: Hash the pe image
  return NULL;
}

// TODO: Get a secure boot image signature
VOID *GetImageSignatureList(IN VOID    *FileBuffer,
                            IN UINT64   FileSize,
                            IN UINTN   *SignatureListSize,
                            IN BOOLEAN  HashIfNoCertificate)
{
   if ((FileBuffer == NULL) || (FileSize == 0) || (SignatureListSize == 0)) {
   }
   if (HashIfNoCertificate) {
     return CreateImageSignatureList(FileBuffer, FileSize, SignatureListSize);
   }
   return NULL;
}
