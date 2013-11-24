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

#include <Library/BaseCryptLib.h>

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
#define PKCS1_1_5_SIZE (sizeof(WIN_CERTIFICATE_EFI_PKCS1_15) - 1)
#define PKCS7_SIZE (sizeof(WIN_CERTIFICATE) - 1)
#define EFIGUID_SIZE (sizeof(WIN_CERTIFICATE_UEFI_GUID) - 1)

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

// Create a secure boot image signature
STATIC VOID *CreateImageSignatureList(IN VOID   *FileBuffer,
                                      IN UINT64  FileSize,
                                      IN UINTN  *SignatureListSize)
{
  UINTN                                Index, Size = 0;
  UINTN                                BytesHashed, HashSize;
  VOID                                *SignatureList = NULL;
  VOID                                *HashCtx;
  UINT8                               *ImageBase = (UINT8 *)FileBuffer;
  UINT8                               *HashBase = ImageBase;
  UINT8                               *HashPtr;
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
  if (SignatureListSize == NULL) {
    return NULL;
  }
  *SignatureListSize = 0;
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
  if (PeHeader.Pe32->Signature == EFI_IMAGE_NT_SIGNATURE) {
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
    DBG("Invalid image: 0x%X (0x%X)\n", FileBuffer, FileSize);
    return NULL;
  }
  HashSize = (UINTN)(HashPtr - HashBase);
  // Allocate the hash context
  HashCtx = AllocateZeroPool(Sha256GetContextSize());
  if (HashCtx == NULL) {
    return NULL;
  }
  // Initialize the hash context
  if (!Sha256Init(HashCtx)) {
    goto Failed;
  }
  // Begin hashing the pe image
  if (!Sha256Update(HashCtx, HashBase, HashSize)) {
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
        if (!Sha256Update(HashCtx, HashBase, HashSize)) {
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
        if (!Sha256Update(HashCtx, HashBase, HashSize)) {
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
    if (!Sha256Update(HashCtx, HashBase, HashSize)) {
      goto Failed;
    }
  }
  // Get the image section headers
  SectionPtr = (EFI_IMAGE_SECTION_HEADER *)(ImageBase + PeHeaderOffset + sizeof(EFI_IMAGE_FILE_HEADER) +
                                            sizeof(UINT32) + PeHeader.Pe32->FileHeader.SizeOfOptionalHeader);
  // Allocate a new array for the image section headers
  Sections = AllocateZeroPool(sizeof(EFI_IMAGE_SECTION_HEADER) * PeHeader.Pe32->FileHeader.NumberOfSections);
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
    if (!Sha256Update(HashCtx, HashBase, HashSize)) {
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
      if (!Sha256Update(HashCtx, HashBase, HashSize)) {
        goto Failed;
      }
    }
  }
  // Create the signature list
  Size = (sizeof(EFI_SIGNATURE_LIST) + sizeof(EFI_GUID) + 256);
  SignatureList = AllocateZeroPool(Size);
  if (SignatureList == NULL) {
    goto Failed;
  }
  // Copy the hash to the signature list
  SignatureListPtr = (EFI_SIGNATURE_LIST *)SignatureList;
  CopyMem(&(SignatureListPtr->SignatureType), &gEfiCertSha256Guid, sizeof(EFI_GUID));
  SignatureListPtr->SignatureListSize = (UINT32)Size;
  SignatureListPtr->SignatureSize = (UINT32)(Size - sizeof(EFI_SIGNATURE_LIST));
  // Finalize the hash by placing it in the signature list
  HashPtr = ((UINT8 *)(SignatureList)) + sizeof(EFI_SIGNATURE_LIST) + sizeof(EFI_GUID);
  if (!Sha256Final(HashCtx, HashPtr)) {
    goto Failed;
  }
  // Cleanup and return success
  FreePool(Sections);
  FreePool(HashCtx);
  *SignatureListSize = Size;
  return SignatureList;

Failed:
  // Hashing failed
  if (SignatureList != NULL) {
    FreePool(SignatureList);
  }
  if (Sections != NULL) {
    FreePool(Sections);
  }
  FreePool(HashCtx);
  return NULL;
}

// Append a signature to list
STATIC VOID AppendSignature(IN OUT VOID     **SignatureList,
                            IN OUT UINTN     *SignatureListSize,
                            IN     EFI_GUID  *SignatureType,
                            IN     VOID      *Signature,
                            IN     UINTN      SignatureSize)
{
  VOID  *NewSignatureList;
  VOID  *OldSignatureList;
  UINTN  NewSignatureListSize;
  UINTN  OldSignatureListSize;
  UINTN  NewSignatureSize;
  UINTN  NewSignatureTotalSize;
  // Check parameters
  if ((SignatureList == NULL) || (SignatureListSize == NULL) ||
      (SignatureType == NULL) || (Signature == NULL) || (SignatureSize == 0)) {
    return;
  }
  // Get old signature list and size
  OldSignatureList = *SignatureList;
  OldSignatureListSize = (OldSignatureList == NULL) ? 0 : *SignatureListSize;
  // Get new signature list
  NewSignatureSize = sizeof(EFI_GUID) + SignatureSize;
  NewSignatureTotalSize = NewSignatureSize + sizeof(EFI_SIGNATURE_LIST);
  NewSignatureListSize = OldSignatureListSize + NewSignatureTotalSize;
  NewSignatureList = AllocateZeroPool(NewSignatureListSize);
  if (NewSignatureList != NULL) {
    EFI_SIGNATURE_LIST *NewSignature;
    // Copy old list if present
    if (OldSignatureList != NULL) {
      CopyMem(NewSignatureList, OldSignatureList, OldSignatureListSize);
      FreePool(OldSignatureList);
    }
    // Copy new signature
    NewSignature = (EFI_SIGNATURE_LIST *)(((UINT8 *)NewSignatureList) + OldSignatureListSize);
    CopyMem(&(NewSignature->SignatureType), SignatureType, sizeof(EFI_GUID));
    NewSignature->SignatureListSize = (UINT32)NewSignatureTotalSize;
    NewSignature->SignatureSize = (UINT32)NewSignatureSize;
    CopyMem((((UINT8 *)NewSignature) + sizeof(EFI_SIGNATURE_LIST) + sizeof(EFI_GUID)), Signature, SignatureSize);
  }
  // Update the list
  *SignatureList = NewSignatureList;
  *SignatureListSize = NewSignatureListSize;
}

// Get a secure boot image signature
VOID *GetImageSignatureList(IN VOID    *FileBuffer,
                            IN UINT64   FileSize,
                            IN UINTN   *SignatureListSize,
                            IN BOOLEAN  HashIfNoCertificate)
{
  UINTN                                Size = 0;
  VOID                                *SignatureList = NULL;
  UINT8                               *Ptr, *End;
  WIN_CERTIFICATE_UEFI_GUID           *GuidCert;
  EFI_IMAGE_DOS_HEADER                *DosHeader;
  EFI_IMAGE_DATA_DIRECTORY            *SecDataDir = NULL;
  EFI_IMAGE_OPTIONAL_HEADER_PTR_UNION  PeHeader;
  UINT32                               PeHeaderOffset;
  UINT16                               Magic;
  // Check parameters
  if (SignatureListSize == 0) {
    return NULL;
  }
  *SignatureListSize = 0;
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
  if (PeHeader.Pe32->Signature == EFI_IMAGE_NT_SIGNATURE) {
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
  DBG("Get image signature: 0x%X (0x%X)\n");
  // Check the security data directory is found and valid
  if ((SecDataDir == NULL) || (SecDataDir->Size == 0)) {
    if (HashIfNoCertificate) {
      // Try to hash the image instead
      return CreateImageSignatureList(FileBuffer, FileSize, SignatureListSize);
    }
    // No certificate
    return NULL;
  }
  // There may be multiple certificates so grab each and update signature list
  Ptr = (((UINT8 *)FileBuffer) + SecDataDir->VirtualAddress);
  End = Ptr + SecDataDir->Size;
  while (Ptr < End) {
    WIN_CERTIFICATE *Cert = (WIN_CERTIFICATE *)Ptr;
    UINTN            Length = Cert->dwLength;
    UINTN            Alignment = (Length % SECDIR_ALIGNMENT_SIZE);
    UINTN            SigSize = 0;
    VOID            *Signature = NULL;
    EFI_GUID        *SigGuid = NULL;
    // Check the signature length
    if (Length <= PKCS7_SIZE) {
      break;
    }
    // Get the alignment length
    if (Alignment != 0) {
      Alignment = SECDIR_ALIGNMENT_SIZE - Alignment;
    }
    // Get the certificate's type
    if (Cert->wCertificateType == WIN_CERT_TYPE_PKCS_SIGNED_DATA) {
      // PKCS#7
      Signature = Ptr + PKCS7_SIZE;
      SigSize = Length - PKCS7_SIZE;
      SigGuid = &gEfiCertPkcs7Guid;
    } else if (Cert->wCertificateType == WIN_CERT_TYPE_EFI_GUID) {
      // EFI GUID
      if (Length <= EFIGUID_SIZE) {
        break;
      }
      Signature = Ptr + EFIGUID_SIZE;
      SigSize = Length - EFIGUID_SIZE;
      // Get the appropriate signature GUID
      GuidCert = (WIN_CERTIFICATE_UEFI_GUID *)Cert;
      if (CompareMem(&(GuidCert->CertType), &gEfiCertTypeRsa2048Sha256Guid, sizeof(EFI_GUID)) == 0) {
        SigGuid = &gEfiCertRsa2048Sha256Guid;
      } else if (CompareMem(&(GuidCert->CertType), &gEfiCertPkcs7Guid, sizeof(EFI_GUID)) == 0) {
        SigGuid = &gEfiCertPkcs7Guid;
      }
    } else if (Cert->wCertificateType == WIN_CERT_TYPE_EFI_PKCS115) {
      // PKCS#1v1.5
      if (Length <= PKCS1_1_5_SIZE) {
        break;
      }
      Signature = Ptr + PKCS1_1_5_SIZE;
      SigSize = (Length - PKCS1_1_5_SIZE);
      if (SigSize == 256) {
        // Only accept 2048 bit key as RSA
        SigGuid = &gEfiCertRsa2048Guid;
      }
    }
    // Append the signature if valid
    if ((SigGuid != NULL) && (Signature != NULL) && (SigSize > 0)) {
      DBG("Found signature certificate: 0x%X (0x%X) %g\n", Signature, SigSize, SigGuid);
      AppendSignature(&SignatureList, &Size, SigGuid, Signature, SigSize);
    } else {
      DBG("Skipping non-signature certificate: 0x%X (0x%X) %d\n", Cert, Length, Cert->wCertificateType);
    }
    // Advance to next certificate
    Ptr += (Length + Alignment);
  }
  // Check if there is some sort of corruption
  if (Ptr != End) {
    // Don't return anything if not at end
    if (SignatureList != NULL) {
      FreePool(SignatureList);
      SignatureList = NULL;
    }
  }
  if (SignatureList != NULL) {
    *SignatureListSize = Size;
  } else if (HashIfNoCertificate) {
    // Try to hash the image instead
    return CreateImageSignatureList(FileBuffer, FileSize, SignatureListSize);
  }
  return SignatureList;
}
