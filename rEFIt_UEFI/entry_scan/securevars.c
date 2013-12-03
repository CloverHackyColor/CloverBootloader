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

#include "securebootkeys.h"

#include <Guid/ImageAuthentication.h>

#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/sha.h>

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

#define SET_DATABASE_ATTRIBUTES (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS)

// Clear the secure boot keys
EFI_STATUS ClearSecureBootKeys(VOID)
{
  // Clear the platform database
  return gRT->SetVariable(PLATFORM_DATABASE_NAME, &PLATFORM_DATABASE_GUID, 0, sizeof(gSecureBootPlatformNullSignedKey), (VOID *)gSecureBootPlatformNullSignedKey);
}

// Enroll the secure boot keys
EFI_STATUS EnrollSecureBootKeys(IN VOID    *AuthorizedDatabase,
                                IN UINTN    AuthorizedDatabaseSize,
                                IN BOOLEAN  WantDefaultKeys)
{
  EFI_STATUS Status;
  // Enroll this image's certificate
  UINTN  DatabaseSize = 0;
  VOID  *Database = NULL;
  if (WantDefaultKeys) {
    // Get default authorized database
    Database = GetSignatureDatabase(DEFAULT_AUTHORIZED_DATABASE_NAME, &DEFAULT_AUTHORIZED_DATABASE_GUID, &DatabaseSize);
    if ((DatabaseSize == 0) && (Database != NULL)) {
      FreePool(Database);
      Database = NULL;
    }
  }
  // Set the authorized database
  if (Database != NULL) {
    Status = AppendSignatureDatabaseToDatabase(&Database, &DatabaseSize, AuthorizedDatabase, AuthorizedDatabaseSize);
    if (EFI_ERROR(Status)) {
      FreePool(Database);
      return Status;
    }
    Status = SetAuthorizedDatabase(Database, DatabaseSize);
    FreePool(Database);
  } else {
    // Set clover signature as only
    Status = SetAuthorizedDatabase(AuthorizedDatabase, AuthorizedDatabaseSize);
  }
  if (EFI_ERROR(Status)) {
    return Status;
  }
  // We don't need the unauthorized database
  if (WantDefaultKeys) {
    // Get the default authorized database
    DatabaseSize = 0;
    Database = GetSignatureDatabase(DEFAULT_UNAUTHORIZED_DATABASE_NAME, &DEFAULT_UNAUTHORIZED_DATABASE_GUID, &DatabaseSize);
    if ((DatabaseSize == 0) && (Database != NULL)) {
      FreePool(Database);
      Database = NULL;
    }
    // Set the default unauthorized database
    if (Database != NULL) {
      Status = SetSignatureDatabase(DEFAULT_UNAUTHORIZED_DATABASE_NAME, &DEFAULT_UNAUTHORIZED_DATABASE_GUID, Database, DatabaseSize);
      FreePool(Database);
    }
  }
  // We need to enroll our own exchange database because we may update databases outside of setup mode
  DatabaseSize = 0;
  Database = NULL;
  if (WantDefaultKeys) {
    // Get the default exchange database
    Database = GetSignatureDatabase(DEFAULT_EXCHANGE_DATABASE_NAME, &DEFAULT_EXCHANGE_DATABASE_GUID, &DatabaseSize);
    if ((DatabaseSize == 0) && (Database != NULL)) {
      FreePool(Database);
      Database = NULL;
    }
  }
  // Set the exchange database
  Status = AppendSignatureToDatabase(&Database, &DatabaseSize, &gEfiCertX509Guid, (VOID *)gSecureBootExchangeKey, sizeof(gSecureBootExchangeKey));
  if (EFI_ERROR(Status)) {
    if (Database != NULL) {
      FreePool(Database);
    }
    return Status;
  }
  Status = SetSignatureDatabase(EXCHANGE_DATABASE_NAME, &EXCHANGE_DATABASE_GUID, Database, DatabaseSize);
  FreePool(Database);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  // Unsure if default platform database should be enrolled.....???
  // Set the platform database - NOT ENROLLING DEFAULT PLATFORM DATABASE, ONLY CLOVER SHOULD OWN PLATFORM(?)
  return gRT->SetVariable(PLATFORM_DATABASE_NAME, &PLATFORM_DATABASE_GUID, SET_DATABASE_ATTRIBUTES, sizeof(gSecureBootPlatformSignedKey), (VOID *)gSecureBootPlatformSignedKey);
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

// Write signature database
EFI_STATUS SetSignatureDatabase(IN CHAR16   *DatabaseName,
                                IN EFI_GUID *DatabaseGuid,
                                IN VOID     *Database,
                                IN UINTN     DatabaseSize)
{
  EFI_STATUS                     Status;
  EFI_VARIABLE_AUTHENTICATION_2 *Authentication;
  UINTN                          Size, NameLen, DataSize = 0;
  EFI_TIME                       Timestamp;
  VOID                          *Data = NULL;
  // Check parameters
  if ((DatabaseName == NULL) || (DatabaseGuid == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  NameLen = StrLen(DatabaseName);
  if (NameLen == 0) {
    return EFI_INVALID_PARAMETER;
  }
  // Check is valid to set database
  if ((gSettings.SecureBoot && gSettings.SecureBootSetupMode) ||
      (!gSettings.SecureBoot && !gSettings.SecureBootSetupMode)) {
    return EFI_NOT_FOUND;
  }
  // Erase database
  if (gSettings.SecureBoot) {
    Status = gRT->SetVariable(DatabaseName, DatabaseGuid, 0, sizeof(gSecureBootExchangeNullSignedKey), (VOID *)gSecureBootExchangeNullSignedKey);
  } else {
    Status = gRT->SetVariable(DatabaseName, DatabaseGuid, 0, 0, NULL);
  }
  // Return status if only erasing
  if ((Database == NULL) || (DatabaseSize == 0)) {
    return Status;
  }
  // Get the current time
  Status = gRT->GetTime(&Timestamp, NULL);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  // Set some time elements to zero
  Timestamp.Pad1       = 0;
  Timestamp.Nanosecond = 0;
  Timestamp.TimeZone   = 0;
  Timestamp.Daylight   = 0;
  Timestamp.Pad2       = 0;
  // Get the required size of the buffer
  if (gSettings.SecureBoot) {
    // In user mode we need to sign the database with exchange key
    PKCS7    *Pkcs7 = NULL;
    X509     *Certificate = NULL;
    EVP_PKEY *PrivateKey = NULL;
    BIO      *BioData = NULL;
    UINT8    *Ptr, *Temp = (UINT8 *)AllocateZeroPool(DataSize = (NameLen + sizeof(EFI_GUID) + sizeof(EFI_TIME) + sizeof(UINT32) + DatabaseSize));
    if (Temp == 0) {
      return EFI_OUT_OF_RESOURCES;
    }
    // Create the signature data
    Ptr = Temp;
    // 1. Database name without null terminator
    CopyMem(Ptr, DatabaseName, NameLen);
    Ptr += NameLen;
    // 2. Database GUID
    CopyMem(Ptr, DatabaseGuid, sizeof(EFI_GUID));
    Ptr += sizeof(EFI_GUID);
    // 3. Database attributes
    *((UINT32 *)Ptr) = SET_DATABASE_ATTRIBUTES;
    Ptr += sizeof(UINT32);
    // 4. Database authentication time stamp
    CopyMem(Ptr, &Timestamp, sizeof(EFI_TIME));
    Ptr += sizeof(EFI_TIME);
    // 5. Database
    CopyMem(Ptr, Database, DatabaseSize);
    // Initialize the cyphers and digests
    ERR_load_crypto_strings();
    OpenSSL_add_all_digests();
    OpenSSL_add_all_ciphers();
    // Create signing certificate
    BioData = BIO_new_mem_buf((void *)gSecureBootExchangeKey, sizeof(gSecureBootExchangeKey));
    if (BioData == NULL) {
      FreePool(Temp);
      return EFI_OUT_OF_RESOURCES;
    }
    Certificate = PEM_read_bio_X509(BioData, NULL, NULL, NULL);
    BIO_free(BioData);
    if (Certificate == NULL) {
      FreePool(Temp);
      return EFI_OUT_OF_RESOURCES;
    }
    // Create signing private key
    BioData = BIO_new_mem_buf((void *)gSecureBootExchangePrivateKey, sizeof(gSecureBootExchangePrivateKey));
    if (BioData == NULL) {
      FreePool(Temp);
      return EFI_OUT_OF_RESOURCES;
    }
    PrivateKey = PEM_read_bio_PrivateKey(BioData, NULL, NULL, NULL);
    BIO_free(BioData);
    if (PrivateKey == NULL) {
      X509_free(Certificate);
      FreePool(Temp);
      return EFI_OUT_OF_RESOURCES;
    }
    // Create data reader
    BioData = BIO_new_mem_buf((void *)Temp, (int)DataSize);
    if (BioData == NULL) {
      X509_free(Certificate);
      EVP_PKEY_free(PrivateKey);
      FreePool(Temp);
      return EFI_OUT_OF_RESOURCES;
    }
    // Sign the data - sign it this way because we have modified openssl
    //  to default to SHA265 so we can support multiple versions
    Pkcs7 = PKCS7_sign(Certificate, PrivateKey, NULL, BioData, PKCS7_BINARY | PKCS7_DETACHED);
    X509_free(Certificate);
    EVP_PKEY_free(PrivateKey);
    BIO_free(BioData);
    FreePool(Temp);
    if (Pkcs7 == NULL) {
      return EFI_ABORTED;
    }
    // Get the size of the signature
    DataSize = i2d_PKCS7(Pkcs7, NULL);
    if (DataSize == 0) {
      PKCS7_free(Pkcs7);
      return EFI_OUT_OF_RESOURCES;
    }
    // Create the signature
    Data = AllocateZeroPool(DataSize);
    if (Data == NULL) {
      PKCS7_free(Pkcs7);
      return EFI_OUT_OF_RESOURCES;
    }
    i2d_PKCS7(Pkcs7, (unsigned char **)&Data);
    PKCS7_free(Pkcs7);
    // Set the authentication buffer size
    Size = sizeof(EFI_TIME) + sizeof(EFI_GUID) + sizeof(WIN_CERTIFICATE) + DataSize;
  } else {
    // In setup mode we don't need to sign, so just set the database
    Size = sizeof(EFI_TIME) + sizeof(EFI_GUID) + sizeof(WIN_CERTIFICATE) + DatabaseSize;
  }
  // Create the authentication buffer
  Authentication = (EFI_VARIABLE_AUTHENTICATION_2 *)AllocateZeroPool(Size);
  if (Authentication == NULL) {
    if (Data != NULL) {
      FreePool(Data);
    }
    return EFI_OUT_OF_RESOURCES;
  }
  // Set the certificate elements
  Authentication->AuthInfo.Hdr.dwLength         = sizeof(EFI_GUID) + sizeof(WIN_CERTIFICATE);
  Authentication->AuthInfo.Hdr.wRevision        = 0x0200;
  Authentication->AuthInfo.Hdr.wCertificateType = WIN_CERT_TYPE_EFI_GUID;
  CopyMem(&(Authentication->TimeStamp), &Timestamp, sizeof(EFI_TIME));
  CopyMem(&(Authentication->AuthInfo.CertType), &gEfiCertPkcs7Guid, sizeof(EFI_GUID));
  // Copy the data into the authentication
  if (Data != NULL) {
    CopyMem(((UINT8 *)Authentication) + sizeof(EFI_TIME) + sizeof(EFI_GUID) + sizeof(WIN_CERTIFICATE), Data, DataSize);
    FreePool(Data);
  } else {
    CopyMem(((UINT8 *)Authentication) + sizeof(EFI_TIME) + sizeof(EFI_GUID) + sizeof(WIN_CERTIFICATE), Database, DatabaseSize);
  }
  // Write the database variable
  Status = gRT->SetVariable(DatabaseName, DatabaseGuid, SET_DATABASE_ATTRIBUTES, Size, Authentication);
  // Cleanup the authentication buffer
  FreePool(Authentication);
  return Status;
}
