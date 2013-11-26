/*
 * refit/scan/securevars.c
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
#define DEBUG_SECURE_VARS 1
#else
#define DEBUG_SECURE_VARS DEBUG_ALL
#endif

#if DEBUG_SECURE_VARS == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_SECURE_VARS, __VA_ARGS__)
#endif

// TODO: Enroll the secure boot keys
EFI_STATUS EnrollSecureBootKeys(VOID)
{
  return EFI_NOT_FOUND;
}

// Read signature database
VOID *GetSignatureDatabase(IN  CHAR16   *DatabaseName,
                           IN  EFI_GUID *DatabaseGuid,
                           OUT UINTN    *DatabaseSize)
{
  UINTN  Size = 0;
  VOID  *Database;
  // Check parameters
  if (DatabaseSize == NULL) {
    return NULL;
  }
  *DatabaseSize = 0;
  if ((DatabaseName == NULL) || (DatabaseGuid == NULL)) {
    return NULL;
  }
  // Get database size
  if (gRT->GetVariable(DatabaseName, DatabaseGuid, NULL, &Size, NULL) != EFI_BUFFER_TOO_SMALL) {
    return NULL;
  }
  if (Size == 0) {
    return NULL;
  }
  // Allocate a buffer large enough to hold the database
  Database = AllocateZeroPool(Size);
  if (Database == NULL) {
    return NULL;
  }
  // Read database
  if (EFI_ERROR(gRT->GetVariable(DatabaseName, DatabaseGuid, NULL, &Size, Database)) || (Size == 0)) {
    FreePool(Database);
    return NULL;
  }
  // Return database
  *DatabaseSize = Size;
  return Database;
}

// TODO: Write signature database
EFI_STATUS SetSignatureDatabase(IN CHAR16   *DatabaseName,
                                IN EFI_GUID *DatabaseGuid,
                                IN VOID     *Database,
                                IN UINTN     DatabaseSize)
{
  // Check parameters
  if ((DatabaseName == NULL) || (DatabaseGuid == NULL) ||
      (Database == NULL) || (DatabaseSize == 0)) {
    return EFI_INVALID_PARAMETER;
  }
  return EFI_ABORTED;
}
