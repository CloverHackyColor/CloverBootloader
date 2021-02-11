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

//CONST INTN SecureHashDef = 0; // jief : not used ??

#ifdef ENABLE_SECURE_BOOT

#include "entry_scan.h"

#include "../../Library/OpensslLib/openssl-1.0.1e/include/openssl/sha.h"

#include <Guid/ImageAuthentication.h>

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

#define SECDIR_ALIGNMENT_SIZE 8
#define CERT_SIZE (sizeof(UINT32) + sizeof(UINT16) + sizeof(UINT16))
#define PKCS1_1_5_SIZE (CERT_SIZE + sizeof(EFI_GUID))
#define EFIGUID_SIZE (CERT_SIZE + sizeof(EFI_GUID))

// Check database for signature
STATIC EFI_STATUS CheckSignatureIsInDatabase(IN void     *Database,
                                             IN UINTN     DatabaseSize,
                                             IN EFI_GUID *SignatureType,
                                             IN void     *Signature,
                                             IN UINTN     SignatureSize)
{
  UINT8 *DatabasePtr;
  UINT8 *DatabaseEnd;
  // Check parameters
  if ((SignatureType == NULL) || (Signature == NULL) || (SignatureSize == 0)) {
    return EFI_INVALID_PARAMETER;
  }
  if ((Database == NULL) || (DatabaseSize <= sizeof(EFI_SIGNATURE_LIST))) {
    return EFI_NOT_FOUND;
  }
  // Get the database start and end
  DatabasePtr = (UINT8 *)Database;
  DatabaseEnd = DatabasePtr + DatabaseSize;
  // Traverse the database
  while (DatabasePtr < DatabaseEnd) {
    EFI_SIGNATURE_LIST *SignatureList = (EFI_SIGNATURE_LIST *)DatabasePtr;
    UINT8              *Ptr, *End;
    // Check the list is valid
    if ((SignatureList->SignatureListSize <= sizeof(EFI_SIGNATURE_LIST)) || (SignatureList->SignatureSize <= sizeof(EFI_GUID))) {
      return EFI_INVALID_PARAMETER;
    }
    // Check if this signature list can contain the signature
    if (((SignatureSize + sizeof(EFI_GUID)) == SignatureList->SignatureSize) &&
        (CompareMem(SignatureType, &(SignatureList->SignatureType), sizeof(EFI_GUID)))) {
      // Get signature list data start
      UINTN Offset = SignatureList->SignatureHeaderSize + sizeof(EFI_SIGNATURE_LIST);
      Ptr = ((UINT8 *)SignatureList) + Offset;
      End = Ptr + (SignatureList->SignatureListSize - Offset);
      Ptr += sizeof(EFI_GUID);
      // Check for the signature
      while (Ptr < End) {
        if (CompareMem(Ptr, Signature, SignatureSize) == 0) {
          // The signature was found
          return EFI_SUCCESS;
        }
        Ptr += SignatureList->SignatureSize;
      }
    }
    // Get the next signature list
    DatabasePtr += SignatureList->SignatureListSize;
  }
  return EFI_NOT_FOUND;
}

// Append a signature to a signature list
STATIC EFI_STATUS AppendSignatureToList(IN OUT EFI_SIGNATURE_LIST **SignatureList,
                                        IN     EFI_GUID            *SignatureType,
                                        IN     void                *Signature,
                                        IN     UINTN                SignatureSize)
{
  EFI_SIGNATURE_LIST *OldSignatureList;
  EFI_SIGNATURE_LIST *NewSignatureList;
  UINT8              *Ptr, *End;
  UINTN               Offset;
  UINT32              DataSize = (UINT32)(SignatureSize + sizeof(EFI_GUID));
  // Check parameters
  if ((SignatureList == NULL) || (SignatureType == NULL) ||
      (Signature == NULL) || (SignatureSize == 0)) {
    return EFI_INVALID_PARAMETER;
  }
  // Check if there is an old signature list
  OldSignatureList = *SignatureList;
  if (OldSignatureList == NULL) {
    // There is no list so create a new signature list
    NewSignatureList = (EFI_SIGNATURE_LIST *)AllocateZeroPool(sizeof(EFI_SIGNATURE_LIST) + DataSize);
    if (NewSignatureList == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    // Copy the signature to the list
    CopyMem(&(NewSignatureList->SignatureType), SignatureType, sizeof(EFI_GUID));
    NewSignatureList->SignatureListSize = (UINT32)(DataSize + sizeof(EFI_SIGNATURE_LIST));
    NewSignatureList->SignatureSize = (UINT32)DataSize;
    *SignatureList = NewSignatureList;
    return EFI_SUCCESS;
  }
  // Check the signature type and size matches this list
  if ((OldSignatureList->SignatureListSize <= sizeof(EFI_SIGNATURE_LIST)) ||
      (OldSignatureList->SignatureSize <= sizeof(EFI_GUID)) ||
      (DataSize != OldSignatureList->SignatureSize) ||
      (CompareMem(SignatureType, &(OldSignatureList->SignatureType), sizeof(EFI_GUID)) != 0)) {
    return EFI_INVALID_PARAMETER;
  }
  // Get the start of signatures data but offset by sizeof(EFI_GUID) so we can skip owner
  Offset = sizeof(EFI_SIGNATURE_LIST) + OldSignatureList->SignatureHeaderSize;
  Ptr = ((UINT8 *)OldSignatureList) + Offset;
  End = Ptr + (OldSignatureList->SignatureListSize - Offset);
  Ptr += sizeof(EFI_GUID);
  // Check the signature doesn't already exist in list
  while (Ptr < End) {
    if (CompareMem(Ptr + sizeof(EFI_GUID), Signature, SignatureSize) == 0) {
      // Just pretend like we added it if it exists already
      return EFI_SUCCESS;
    }
    Ptr += OldSignatureList->SignatureSize;
  }
  // Create a new list for signatures
  NewSignatureList = (EFI_SIGNATURE_LIST *)AllocateZeroPool(OldSignatureList->SignatureListSize + DataSize);
  if (NewSignatureList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  // Copy old list to new
  CopyMem(NewSignatureList, OldSignatureList, OldSignatureList->SignatureListSize);
  // Increase list size
  NewSignatureList->SignatureListSize += DataSize;
  // Copy new signature
  CopyMem(((UINT8 *)NewSignatureList) + OldSignatureList->SignatureListSize + sizeof(EFI_GUID), Signature, SignatureSize);
  // Update the list and free old
  *SignatureList = NewSignatureList;
  FreePool(OldSignatureList);
  return EFI_SUCCESS;
}

// Append a signature list to a signature database
STATIC EFI_STATUS AppendSignatureListToDatabase(IN OUT void               **Database,
                                                IN OUT UINTN               *DatabaseSize,
                                                IN     EFI_SIGNATURE_LIST  *SignatureList)
{
  EFI_SIGNATURE_LIST *List = NULL;
  UINT8              *Ptr, *End;
  UINT8              *OldDatabase;
  UINT8              *NewDatabase;
  UINTN               OldDatabaseSize;
  UINTN               NewDatabaseSize;
  UINTN               Size;
  // Check parameters
  if ((Database == NULL) || (DatabaseSize == NULL) || (SignatureList == NULL) ||
      (SignatureList->SignatureListSize <= sizeof(EFI_SIGNATURE_LIST)) ||
      (SignatureList->SignatureSize <= sizeof(EFI_GUID))) {
    return EFI_INVALID_PARAMETER;
  }
  // Get old database
  OldDatabase = (UINT8 *)*Database;
  OldDatabaseSize = *DatabaseSize;
  if ((OldDatabase == NULL) || (OldDatabaseSize <= sizeof(EFI_SIGNATURE_LIST))) {
    // No database so just set it to the signature list
    NewDatabaseSize = SignatureList->SignatureListSize;
    NewDatabase = (UINT8 *)AllocatePool(NewDatabaseSize);
    if (NewDatabase == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    // Free the old database if needed
    if (OldDatabase != NULL) {
      FreePool(OldDatabase);
    }
    // Copy the signature list to the database
    CopyMem(*Database = NewDatabase, SignatureList, *DatabaseSize = NewDatabaseSize);
    return EFI_SUCCESS;
  }
  // Rebuild the signature list with only signatures that aren't found in database
  Ptr = ((UINT8 *)SignatureList) + SignatureList->SignatureHeaderSize + sizeof(EFI_SIGNATURE_LIST) + sizeof(EFI_GUID);
  End = ((UINT8 *)SignatureList) + SignatureList->SignatureListSize;
  Size = (SignatureList->SignatureSize - sizeof(EFI_GUID));
  while (Ptr < End) {
    // Check signature is already in database
    EFI_STATUS Status = CheckSignatureIsInDatabase(OldDatabase, OldDatabaseSize, &(SignatureList->SignatureType), Ptr, Size);
    if (Status == EFI_NOT_FOUND) {
      // Add to new signature list if not found in database
      Status = AppendSignatureToList(&List, &(SignatureList->SignatureType), Ptr, Size);
    }
    if (EFI_ERROR(Status)) {
      if (List != NULL) {
        FreePool(List);
      }
      return Status;
    }
    Ptr += SignatureList->SignatureSize;
  }
  // Check any signatures remain to be added
  if (List == NULL) {
    return EFI_SUCCESS;
  }
  // Check the list is valid
  if ((List->SignatureListSize <= sizeof(EFI_SIGNATURE_LIST)) ||
      (List->SignatureSize <= sizeof(EFI_GUID))) {
    FreePool(List);
    // Unsure if this should return an error or success
    return EFI_SUCCESS;
  }
  // Create a new database
  NewDatabaseSize = OldDatabaseSize + List->SignatureListSize;
  NewDatabase = (UINT8 *)AllocatePool(NewDatabaseSize);
  if (NewDatabase == NULL) {
    FreePool(List);
    return EFI_OUT_OF_RESOURCES;
  }
  // Copy original database and free it
  CopyMem(*Database = NewDatabase, OldDatabase, OldDatabaseSize);
  FreePool(OldDatabase);
  // Append signature list to end of database
  CopyMem(NewDatabase + OldDatabaseSize, List, List->SignatureListSize);
  *DatabaseSize = NewDatabaseSize;
  FreePool(List);
  return EFI_SUCCESS;
}

// Append a signature to a signature database
EFI_STATUS AppendSignatureToDatabase(IN OUT void     **Database,
                                     IN OUT UINTN     *DatabaseSize,
                                     IN     EFI_GUID  *SignatureType,
                                     IN     void      *Signature,
                                     IN     UINTN      SignatureSize)
{
  // Create a new signature list
  EFI_SIGNATURE_LIST *List = NULL;
  EFI_STATUS          Status = AppendSignatureToList(&List, SignatureType, Signature, SignatureSize);
  if (EFI_ERROR(Status)) {
    if (List != NULL) {
      FreePool(List);
    }
    return Status;
  }
  // Add the signature list to database
  Status = AppendSignatureListToDatabase(Database, DatabaseSize, List);
  FreePool(List);
  return Status;
}

// Append a signature database to another signature database
EFI_STATUS AppendSignatureDatabaseToDatabase(IN OUT void  **Database,
                                             IN OUT UINTN  *DatabaseSize,
                                             IN     void   *SignatureDatabase,
                                             IN     UINTN   SignatureDatabaseSize)
{
  UINT8 *Ptr, *End;
  // Check parameters
  if ((Database == NULL) || (DatabaseSize == NULL) ||
      (SignatureDatabase == NULL) || (SignatureDatabaseSize <= sizeof(EFI_SIGNATURE_LIST))) {
    return EFI_INVALID_PARAMETER;
  }
  // Get the signature database start
  Ptr = (UINT8 *)SignatureDatabase;
  End = Ptr + SignatureDatabaseSize;
  while (Ptr < End) {
    // Get each signature list in signature database
    EFI_SIGNATURE_LIST *List = (EFI_SIGNATURE_LIST *)Ptr;
    EFI_STATUS          Status;
    if ((List->SignatureListSize <= sizeof(EFI_SIGNATURE_LIST)) || (List->SignatureSize <= sizeof(EFI_GUID))) {
      return EFI_INVALID_PARAMETER;
    }
    // Add the signature list to the database
    Status = AppendSignatureListToDatabase(Database, DatabaseSize, List);
    if (EFI_ERROR(Status)) {
      return Status;
    }
    Ptr += List->SignatureListSize;
  }
  return EFI_SUCCESS;
}

// Add image signature database to authorized database
EFI_STATUS AppendImageDatabaseToAuthorizedDatabase(IN void  *Database,
                                                   IN UINTN  DatabaseSize)
{
  EFI_STATUS  Status;
  void       *AuthDatabase;
  UINTN       AuthDatabaseSize;
  // Check parameters
  if ((Database == NULL) || (DatabaseSize <= sizeof(EFI_SIGNATURE_LIST))) {
    return EFI_INVALID_PARAMETER;
  }
  // Get the authorized database
  AuthDatabase = GetAuthorizedDatabase(&AuthDatabaseSize);
  // Add the signature database to the authorized database
  Status = AppendSignatureDatabaseToDatabase(&AuthDatabase, &AuthDatabaseSize, Database, DatabaseSize);
  if (EFI_ERROR(Status)) {
    if (AuthDatabase != NULL) {
      FreePool(AuthDatabase);
    }
    return Status;
  }
  // Check there is any to set
  if (AuthDatabase == NULL) {
    return ClearAuthorizedDatabase();
  }
  if (AuthDatabaseSize <= sizeof(EFI_SIGNATURE_LIST)) {
    FreePool(AuthDatabase);
    return ClearAuthorizedDatabase();
  }
  // Set the authorized database
  Status = SetAuthorizedDatabase(AuthDatabase, AuthDatabaseSize);
  FreePool(AuthDatabase);
  return Status;
}

STATIC EFI_STATUS RemoveSignatureFromDatabase(IN OUT void     **Database,
                                              IN OUT UINTN     *DatabaseSize,
                                              IN     EFI_GUID  *SignatureType,
                                              IN     void      *Signature,
                                              IN     UINTN      SignatureSize)
{
  UINT8 *Ptr, *End;
  void  *OldDatabase;
  void  *NewDatabase = NULL;
  UINTN  OldDatabaseSize;
  UINTN  NewDatabaseSize = 0;
  // Check parameters
  if ((Database == NULL) || (DatabaseSize == NULL) || (SignatureType == NULL) || 
      (Signature == NULL) || (SignatureSize <= sizeof(EFI_SIGNATURE_LIST))) {
    return EFI_INVALID_PARAMETER;
  }
  OldDatabase = (UINT8 *)*Database;
  OldDatabaseSize = *DatabaseSize;
  if ((OldDatabase == NULL) || (OldDatabaseSize <= sizeof(EFI_SIGNATURE_LIST))) {
    // Nothing to do if the database is empty
    return EFI_SUCCESS;
  }
  // Get the signature database start
  Ptr = (UINT8 *)OldDatabase;
  End = Ptr + OldDatabaseSize;
  while (Ptr < End) {
    // Get each signature list in signature database
    EFI_SIGNATURE_LIST *List = (EFI_SIGNATURE_LIST *)Ptr;
    EFI_STATUS          Status = EFI_SUCCESS;
    if ((List->SignatureListSize <= sizeof(EFI_SIGNATURE_LIST)) || (List->SignatureSize <= sizeof(EFI_GUID))) {
      if (NewDatabase != NULL) {
        FreePool(NewDatabase);
      }
      return EFI_INVALID_PARAMETER;
    }
    // Check this signature could be found in this list
    if (((List->SignatureSize - sizeof(EFI_GUID)) == SignatureSize) &&
        (CompareMem(SignatureType, &(List->SignatureType), sizeof(EFI_GUID)) == 0)) {
      // Remove the signature list from the database
      UINT8 *ListPtr = Ptr + List->SignatureHeaderSize + sizeof(EFI_SIGNATURE_LIST) + sizeof(EFI_GUID);
      UINT8 *ListEnd = Ptr + List->SignatureListSize;
      EFI_SIGNATURE_LIST *NewList = NULL;
      while (ListPtr < ListEnd) {
        // Compare the signatures
        if (CompareMem(Ptr, Signature, SignatureSize) != 0) {
          // Not a match so append it to new list
          Status = AppendSignatureToList(&NewList, SignatureType, Ptr, SignatureSize);
          if (EFI_ERROR(Status)) {
            if (NewList != NULL) {
              FreePool(NewList);
            }
            if (NewDatabase != NULL) {
              FreePool(NewDatabase);
            }
            return Status;
          }
        }
        ListPtr += List->SignatureSize;
      }
      // Append new list if any
      if (NewList != NULL) {
        Status = AppendSignatureListToDatabase(&NewDatabase, &NewDatabaseSize, NewList);
        FreePool(NewList);
      }
    } else {
      // Append this whole list as it can't hold the signature
      Status = AppendSignatureListToDatabase(&NewDatabase, &NewDatabaseSize, List);
    }
    if (EFI_ERROR(Status)) {
      if (NewDatabase != NULL) {
        FreePool(NewDatabase);
      }
      return Status;
    }
    Ptr += List->SignatureListSize;
  }
  // Set new database
  *Database = NewDatabase;
  *DatabaseSize = NewDatabaseSize;
  FreePool(OldDatabase);
  return EFI_SUCCESS;
}

STATIC EFI_STATUS RemoveSignatureListFromDatabase(IN OUT void               **Database,
                                                  IN OUT UINTN               *DatabaseSize,
                                                  IN     EFI_SIGNATURE_LIST  *SignatureList)
{
  UINT8              *Ptr, *End;
  UINT8              *OldDatabase;
  UINTN               OldDatabaseSize;
  UINTN               Size;
  // Check parameters
  if ((Database == NULL) || (DatabaseSize == NULL) || (SignatureList == NULL) ||
      (SignatureList->SignatureListSize <= sizeof(EFI_SIGNATURE_LIST)) ||
      (SignatureList->SignatureSize <= sizeof(EFI_GUID))) {
    return EFI_INVALID_PARAMETER;
  }
  // Get old database
  OldDatabase = (UINT8 *)*Database;
  OldDatabaseSize = *DatabaseSize;
  if ((OldDatabase == NULL) || (OldDatabaseSize == 0)) {
    // Nothing to remove
    return EFI_SUCCESS;
  }
  // Remove the signatures found in the list from the database
  Ptr = ((UINT8 *)SignatureList) + SignatureList->SignatureHeaderSize + sizeof(EFI_SIGNATURE_LIST) + sizeof(EFI_GUID);
  End = ((UINT8 *)SignatureList) + SignatureList->SignatureListSize;
  Size = (SignatureList->SignatureSize - sizeof(EFI_GUID));
  while (Ptr < End) {
    // Remove signature from database
    EFI_STATUS Status = RemoveSignatureFromDatabase(Database, DatabaseSize, &(SignatureList->SignatureType), Ptr, Size);
    if (EFI_ERROR(Status)) {
      return Status;
    }
    Ptr += SignatureList->SignatureSize;
  }
  return EFI_SUCCESS;
}

STATIC EFI_STATUS RemoveSignatureDatabaseFromDatabase(IN OUT void  **Database,
                                                      IN OUT UINTN  *DatabaseSize,
                                                      IN     void   *SignatureDatabase,
                                                      IN     UINTN   SignatureDatabaseSize)
{
  UINT8 *Ptr, *End;
  // Check parameters
  if ((Database == NULL) || (DatabaseSize == NULL) ||
      (SignatureDatabase == NULL) || (SignatureDatabaseSize <= sizeof(EFI_SIGNATURE_LIST))) {
    return EFI_INVALID_PARAMETER;
  }
  // Get the signature database start
  Ptr = (UINT8 *)SignatureDatabase;
  End = Ptr + SignatureDatabaseSize;
  while (Ptr < End) {
    // Get each signature list in signature database
    EFI_SIGNATURE_LIST *List = (EFI_SIGNATURE_LIST *)Ptr;
    EFI_STATUS          Status;
    if ((List->SignatureListSize <= sizeof(EFI_SIGNATURE_LIST)) || (List->SignatureSize <= sizeof(EFI_GUID))) {
      return EFI_INVALID_PARAMETER;
    }
    // Remove the signature list from the database
    Status = RemoveSignatureListFromDatabase(Database, DatabaseSize, List);
    if (EFI_ERROR(Status)) {
      return Status;
    }
    Ptr += List->SignatureListSize;
  }
  return EFI_SUCCESS;
}

// Remove image signature database from authorized database
EFI_STATUS RemoveImageDatabaseFromAuthorizedDatabase(IN void  *Database,
                                                     IN UINTN  DatabaseSize)
{
  EFI_STATUS  Status;
  void       *AuthDatabase;
  UINTN       AuthDatabaseSize;
  // Check parameters
  if ((Database == NULL) || (DatabaseSize <= sizeof(EFI_SIGNATURE_LIST))) {
    return EFI_INVALID_PARAMETER;
  }
  // Get the authorized database
  AuthDatabase = GetAuthorizedDatabase(&AuthDatabaseSize);
  // Remove the signature database from the authorized database
  Status = RemoveSignatureDatabaseFromDatabase(&AuthDatabase, &AuthDatabaseSize, Database, DatabaseSize);
  if (EFI_ERROR(Status)) {
    FreePool(AuthDatabase);
    return Status;
  }
  // Check there is any to set
  if (AuthDatabase == NULL) {
    return ClearAuthorizedDatabase();
  }
  if (AuthDatabaseSize <= sizeof(EFI_SIGNATURE_LIST)) {
    FreePool(AuthDatabase);
    return ClearAuthorizedDatabase();
  }
  // Set the authorized database
  Status = SetAuthorizedDatabase(AuthDatabase, AuthDatabaseSize);
  FreePool(AuthDatabase);
  return Status;
}

void *GetAuthorizedDatabase(UINTN *DatabaseSize)
{
   return GetSignatureDatabase(AUTHORIZED_DATABASE_NAME, &AUTHORIZED_DATABASE_GUID, DatabaseSize);
}
EFI_STATUS SetAuthorizedDatabase(IN void  *Database,
                                 IN UINTN  DatabaseSize)
{
   return SetSignatureDatabase(AUTHORIZED_DATABASE_NAME, &AUTHORIZED_DATABASE_GUID, Database, DatabaseSize);
}

// Clear authorized signature database
EFI_STATUS ClearAuthorizedDatabase(void)
{
   return SetAuthorizedDatabase(NULL, 0);
}

// Create a secure boot image signature
STATIC void *CreateImageSignatureDatabase(IN void   *FileBuffer,
                                          IN UINT64  FileSize,
                                          IN UINTN  *DatabaseSize)
{
  UINTN                                Index, Size = 0;
  UINTN                                BytesHashed, HashSize;
  UINT8                               *Database = NULL;
  UINT8                               *ImageBase = (UINT8 *)FileBuffer;
  UINT8                               *HashBase = ImageBase;
  UINT8                               *HashPtr;
  SHA256_CTX                           HashCtx;
  EFI_SIGNATURE_LIST                  *SignatureListPtr;
  EFI_IMAGE_SECTION_HEADER            *Sections = NULL;
  EFI_IMAGE_SECTION_HEADER            *SectionPtr;
  EFI_IMAGE_SECTION_HEADER            *SectionEnd;
  EFI_IMAGE_DOS_HEADER                *DosHeader;
  EFI_IMAGE_OPTIONAL_HEADER_PTR_UNION  PeHeader;
  UINT32                               PeHeaderOffset;
  UINT32                               CertSize = 0;
  UINT16                               Magic;
  // Check parameters
  if (DatabaseSize == NULL) {
    return NULL;
  }
  *DatabaseSize = 0;
  if ((FileBuffer == NULL) || (FileSize == 0)) {
    return NULL;
  }
  // Check for DOS PE header
  DosHeader = (EFI_IMAGE_DOS_HEADER *)FileBuffer;
  if (DosHeader->e_magic == EFI_IMAGE_DOS_SIGNATURE) {
    PeHeaderOffset = DosHeader->e_lfanew;
  } else {
    PeHeaderOffset = 0;
  }
  // Check for PE header
  PeHeader.Pe32 = (EFI_IMAGE_NT_HEADERS32 *)(((UINT8 *)FileBuffer) + PeHeaderOffset);
  if (PeHeader.Pe32->Signature != EFI_IMAGE_NT_SIGNATURE) {
    // Invalid PE image
    return NULL;
  }
  // Fix magic number if needed
  if ((PeHeader.Pe32->FileHeader.Machine == IMAGE_FILE_MACHINE_IA64) &&
      (PeHeader.Pe32->OptionalHeader.Magic == EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC)) {
    Magic = EFI_IMAGE_NT_OPTIONAL_HDR64_MAGIC;
  } else {
    Magic = PeHeader.Pe32->OptionalHeader.Magic;
  }
  // Check magic number to get size
  if (Magic == EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
    // PE32
    HashPtr = (UINT8 *)(&(PeHeader.Pe32->OptionalHeader.CheckSum));
  } else if (Magic == EFI_IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
    // PE32+
    HashPtr = (UINT8 *)(&(PeHeader.Pe32Plus->OptionalHeader.CheckSum));
  } else {
    // Invalid image
    DBG("Invalid image: 0x%hhX (0x%hhX)\n", FileBuffer, FileSize);
    return NULL;
  }
  HashSize = (UINTN)(HashPtr - HashBase);
  // Initialize the hash context
  if (SHA256_Init(&HashCtx) == 0) {
    goto Failed;
  }
  // Begin hashing the pe image
  if (SHA256_Update(&HashCtx, HashBase, HashSize) == 0) {
    goto Failed;
  }
  // Skip the checksum
  HashBase = HashPtr + sizeof(UINT32);
  // Skip over the security directory if present
  if (Magic == EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
    // PE32
    if (PeHeader.Pe32->OptionalHeader.NumberOfRvaAndSizes > EFI_IMAGE_DIRECTORY_ENTRY_SECURITY) {
      // Hash before the security directory
      HashPtr = (UINT8 *)(&(PeHeader.Pe32->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_SECURITY]));
      HashSize = (HashPtr - HashBase);
      if (HashSize != 0) {
        if (SHA256_Update(&HashCtx, HashBase, HashSize) == 0) {
          goto Failed;
        }
      }
      // Set to point at the remaining data if any
      HashBase = (UINT8 *)(&(PeHeader.Pe32->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_SECURITY + 1]));
      CertSize = PeHeader.Pe32->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_SECURITY].Size;
    }
    BytesHashed = PeHeader.Pe32->OptionalHeader.SizeOfHeaders;
  } else {
    // PE32+
    if (PeHeader.Pe32Plus->OptionalHeader.NumberOfRvaAndSizes > EFI_IMAGE_DIRECTORY_ENTRY_SECURITY) {
      // Hash before the security directory
      HashPtr = (UINT8 *)(&(PeHeader.Pe32Plus->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_SECURITY]));
      HashSize = (HashPtr - HashBase);
      if (HashSize != 0) {
        if (SHA256_Update(&HashCtx, HashBase, HashSize) == 0) {
          goto Failed;
        }
      }
      // Set to point at the remaining data if any
      HashBase = (UINT8 *)(&(PeHeader.Pe32Plus->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_SECURITY + 1]));
      CertSize = PeHeader.Pe32Plus->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_SECURITY].Size;
    }
    BytesHashed = PeHeader.Pe32->OptionalHeader.SizeOfHeaders;
  }
  // Hash the rest of the data directories if any
  HashSize = BytesHashed - (UINTN)(HashBase - ImageBase);
  if (HashSize != 0) {
    if (SHA256_Update(&HashCtx, HashBase, HashSize) == 0) {
      goto Failed;
    }
  }
  // Get the image section headers
  SectionPtr = (EFI_IMAGE_SECTION_HEADER *)(ImageBase + PeHeaderOffset + sizeof(EFI_IMAGE_FILE_HEADER) +
                                            sizeof(UINT32) + PeHeader.Pe32->FileHeader.SizeOfOptionalHeader);
  // Allocate a new array for the image section headers
  Sections = (__typeof__(Sections))AllocateZeroPool(sizeof(EFI_IMAGE_SECTION_HEADER) * PeHeader.Pe32->FileHeader.NumberOfSections);
  if (Sections == NULL) {
    goto Failed;
  }
  // Sort the image section headers
  Index = 0;
  while (Index < PeHeader.Pe32->FileHeader.NumberOfSections) {
    UINTN Pos = Index++;
    while ((Pos > 0) && (SectionPtr->PointerToRawData < Sections[Pos - 1].PointerToRawData)) {
      CopyMem(&Sections[Pos], &Sections[Pos - 1], sizeof(EFI_IMAGE_SECTION_HEADER));
      --Pos;
    }
    CopyMem(&Sections[Pos], SectionPtr++, sizeof(EFI_IMAGE_SECTION_HEADER));
  }
  // Hash each image section
  SectionEnd = Sections + PeHeader.Pe32->FileHeader.NumberOfSections;
  for (SectionPtr = Sections; SectionPtr < SectionEnd; ++SectionPtr) {
    // Nothing to do if no size
    if (SectionPtr->SizeOfRawData == 0) {
      continue;
    }
    // Calculate hash base and size
    HashBase  = ImageBase + SectionPtr->PointerToRawData;
    HashSize  = (UINTN)SectionPtr->SizeOfRawData;
    // Hash the image section
    if (SHA256_Update(&HashCtx, HashBase, HashSize) == 0) {
      goto Failed;
    }
    BytesHashed += HashSize;
  }
  // Hash any data remaining after the sections
  if (BytesHashed < FileSize) {
    HashBase = ImageBase + BytesHashed;
    if (FileSize < (BytesHashed + CertSize)) {
      goto Failed;
    }
    HashSize = (UINTN)(FileSize - (BytesHashed + CertSize));
    if (HashSize != 0) {
      if (SHA256_Update(&HashCtx, HashBase, HashSize) == 0) {
        goto Failed;
      }
    }
  }
  // Create the signature list
  Size = (sizeof(EFI_SIGNATURE_LIST) + sizeof(EFI_GUID) + 256);
  Database = (__typeof__(Database))AllocateZeroPool(Size);
  if (Database == NULL) {
    goto Failed;
  }
  // Copy the hash to the signature list
  SignatureListPtr = (EFI_SIGNATURE_LIST *)Database;
  CopyMem(&(SignatureListPtr->SignatureType), &gEfiCertSha256Guid, sizeof(EFI_GUID));
  SignatureListPtr->SignatureListSize = (UINT32)Size;
  SignatureListPtr->SignatureSize = (UINT32)(Size - sizeof(EFI_SIGNATURE_LIST));
  // Finalize the hash by placing it in the signature list
  HashPtr = ((UINT8 *)(Database)) + sizeof(EFI_SIGNATURE_LIST) + sizeof(EFI_GUID);
  if (SHA256_Final(HashPtr, &HashCtx) == 0) {
    goto Failed;
  }
  // Cleanup and return success
  FreePool(Sections);
  *DatabaseSize = Size;
  return Database;

Failed:
  // Hashing failed
  if (Database != NULL) {
    FreePool(Database);
  }
  if (Sections != NULL) {
    FreePool(Sections);
  }
  return NULL;
}

// Get a secure boot image signature
void *GetImageSignatureDatabase(IN void    *FileBuffer,
                                IN UINT64   FileSize,
                                IN UINTN   *DatabaseSize,
                                IN BOOLEAN  HashIfNoDatabase)
{
  UINTN                                Size = 0;
  void                                *Database = NULL;
  UINT8                               *Ptr, *End;
  WIN_CERTIFICATE_UEFI_GUID           *GuidCert;
  EFI_IMAGE_DOS_HEADER                *DosHeader;
  EFI_IMAGE_DATA_DIRECTORY            *SecDataDir = NULL;
  EFI_IMAGE_OPTIONAL_HEADER_PTR_UNION  PeHeader;
  UINT32                               PeHeaderOffset;
  UINT16                               Magic;
  // Check parameters
  if (DatabaseSize == NULL) {
    return NULL;
  }
  *DatabaseSize = 0;
  if ((FileBuffer == NULL) || (FileSize == 0)) {
    return NULL;
  }
  // Check for DOS PE header
  DosHeader = (EFI_IMAGE_DOS_HEADER *)FileBuffer;
  if (DosHeader->e_magic == EFI_IMAGE_DOS_SIGNATURE) {
    PeHeaderOffset = DosHeader->e_lfanew;
  } else {
    PeHeaderOffset = 0;
  }
  // Check for PE header
  PeHeader.Pe32 = (EFI_IMAGE_NT_HEADERS32 *)(((UINT8 *)FileBuffer) + PeHeaderOffset);
  if (PeHeader.Pe32->Signature != EFI_IMAGE_NT_SIGNATURE) {
    // Invalid PE image
    DBG("Invalid PE image for signature retrieval (no NT signature)\n");
    return NULL;
  }
  // Fix magic number if needed
  if ((PeHeader.Pe32->FileHeader.Machine == IMAGE_FILE_MACHINE_IA64) &&
      (PeHeader.Pe32->OptionalHeader.Magic == EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC)) {
    Magic = EFI_IMAGE_NT_OPTIONAL_HDR64_MAGIC;
  } else {
    Magic = PeHeader.Pe32->OptionalHeader.Magic;
  }
  // Get the security data directory of the image
  if (Magic == EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
    // PE32
    if (PeHeader.Pe32->OptionalHeader.NumberOfRvaAndSizes > EFI_IMAGE_DIRECTORY_ENTRY_SECURITY) {
      SecDataDir = (EFI_IMAGE_DATA_DIRECTORY *)&(PeHeader.Pe32->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_SECURITY]);
    }
  } else if (PeHeader.Pe32Plus->OptionalHeader.NumberOfRvaAndSizes > EFI_IMAGE_DIRECTORY_ENTRY_SECURITY) {
    // PE32+
    SecDataDir = (EFI_IMAGE_DATA_DIRECTORY *)&(PeHeader.Pe32Plus->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_SECURITY]);
  }
  DBG("Get image database: 0x%hhX (0x%hhX) 0x%hhX 0x%hhX 0x%hhX (0x%hhX)\n", FileBuffer, FileSize, SecDataDir, SecDataDir->VirtualAddress, ((UINT8 *)FileBuffer) + SecDataDir->VirtualAddress, SecDataDir->Size);
  // Check the security data directory is found and valid
  if ((SecDataDir->VirtualAddress >= FileSize) || ((SecDataDir->VirtualAddress + SecDataDir->Size) > FileSize)) {
    DBG("Security directory exceeds the file limits\n");
    SecDataDir = NULL;
  }
  if ((SecDataDir == NULL) || (SecDataDir->Size == 0)) {
    if (HashIfNoDatabase) {
      // Try to hash the image instead
      return CreateImageSignatureDatabase(FileBuffer, FileSize, DatabaseSize);
    }
    // No certificate
    DBG("Security directory not found in image!\n");
    return NULL;
  }
  // There may be multiple certificates so grab each and update signature list
  Ptr = (((UINT8 *)FileBuffer) + SecDataDir->VirtualAddress);
  End = Ptr + SecDataDir->Size;
  while ((Ptr + CERT_SIZE) < End) {
    WIN_CERTIFICATE *Cert = (WIN_CERTIFICATE *)Ptr;
    UINTN            Length = Cert->dwLength;
    UINTN            Alignment = (Length % SECDIR_ALIGNMENT_SIZE);
    UINTN            SigSize = 0;
    void            *Signature = NULL;
    EFI_GUID        *SigGuid = NULL;
    // Get the alignment length
    if (Alignment != 0) {
      Alignment = SECDIR_ALIGNMENT_SIZE - Alignment;
    }
    DBG("Embedded certificate: 0x%hhX (0x%hhX) [0x%hhX]\n", Cert, Length, Cert->wCertificateType);
    // Get the certificate's type
    if (Cert->wCertificateType == WIN_CERT_TYPE_PKCS_SIGNED_DATA) {
      // PKCS#7
      if (Length < CERT_SIZE) {
        break;
      }
      Signature = Ptr + CERT_SIZE;
      SigSize = Length - CERT_SIZE;
      SigGuid = &gEfiCertPkcs7Guid;
    } else if (Cert->wCertificateType == WIN_CERT_TYPE_EFI_GUID) {
      // EFI GUID
      if (Length < EFIGUID_SIZE) {
        break;
      }
      Signature = Ptr + EFIGUID_SIZE;
      SigSize = Length - EFIGUID_SIZE;
      // Get the appropriate signature GUID
      GuidCert = (WIN_CERTIFICATE_UEFI_GUID *)Cert;
      if (CompareMem(&(GuidCert->CertType), &gEfiCertX509Guid, sizeof(EFI_GUID)) == 0) {
        SigGuid = &gEfiCertX509Guid;
      } else if (CompareMem(&(GuidCert->CertType), &gEfiCertTypeRsa2048Sha256Guid, sizeof(EFI_GUID)) == 0) {
        SigGuid = &gEfiCertRsa2048Sha256Guid;
      } else if (CompareMem(&(GuidCert->CertType), &gEfiCertPkcs7Guid, sizeof(EFI_GUID)) == 0) {
        SigGuid = &gEfiCertPkcs7Guid;
      }
    } else if (Cert->wCertificateType == WIN_CERT_TYPE_EFI_PKCS115) {
      // PKCS#1v1.5
      if (Length < PKCS1_1_5_SIZE) {
        break;
      }
      Signature = Ptr + PKCS1_1_5_SIZE;
      SigSize = (Length - PKCS1_1_5_SIZE);
      GuidCert = (WIN_CERTIFICATE_UEFI_GUID *)Cert;
      SigGuid = &(GuidCert->CertType);
    }
    // Append the signature if valid
    if ((SigGuid != NULL) && (Signature != NULL) && (SigSize > 0)) {
      DBG("Found signature certificate: 0x%hhX (0x%hhX) %s\n", Signature, SigSize, strguid(SigGuid));
      if (EFI_ERROR(AppendSignatureToDatabase(&Database, &Size, SigGuid, Signature, SigSize))) {
        break;
      }
    } else {
      DBG("Skipping non-signature certificate: 0x%hhX (0x%hhX) [0x%hhX]\n", Cert, Length, Cert->wCertificateType);
    }
    // Advance to next certificate
    Ptr += (Length + Alignment);
  }
  // Check if there is some sort of corruption
  if (Ptr != End) {
    DBG("Failed to retrieve image database: 0x%hhX - 0x%hhX @ 0x%hhX\n", (((UINT8 *)FileBuffer) + SecDataDir->VirtualAddress), End, Ptr);
    // Don't return anything if not at end
    if (Database != NULL) {
      FreePool(Database);
      Database = NULL;
    }
  }
  if (Database != NULL) {
    *DatabaseSize = Size;
  } else if (HashIfNoDatabase) {
    // Try to hash the image instead
    return CreateImageSignatureDatabase(FileBuffer, FileSize, DatabaseSize);
  }
  return Database;
}

#endif // ENABLE_SECURE_BOOT
