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

//CONST INTN SecureVarsDef = 0; // jief : not used ??

#ifdef ENABLE_SECURE_BOOT

#include "entry_scan.h"

#include <Guid/ImageAuthentication.h>

#include "securebootkeys.h"

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

#define SET_DATABASE_ATTRIBUTES (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | \
                                 EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS | EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS)
#define SET_ADD_DATABASE_ATTRIBUTES (SET_DATABASE_ATTRIBUTES | EFI_VARIABLE_APPEND_WRITE)

// Clear the secure boot keys
EFI_STATUS ClearSecureBootKeys(VOID)
{
  // Clear the platform database
  return gRT->SetVariable(PLATFORM_DATABASE_NAME, &PLATFORM_DATABASE_GUID, SET_DATABASE_ATTRIBUTES, sizeof(gSecureBootPlatformNullSignedKey), (VOID *)gSecureBootPlatformNullSignedKey);
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

  FreePool(AuthorizedDatabase);

  if (WantDefaultKeys) {
    // Get default authorized database
    DBG("Retrieving default authorized database ...\n");
    Database = GetSignatureDatabase(DEFAULT_AUTHORIZED_DATABASE_NAME, &DEFAULT_AUTHORIZED_DATABASE_GUID, &DatabaseSize);
    if ((DatabaseSize == 0) && (Database != NULL)) {
      FreePool(Database);
      Database = NULL;
    }
  }
  // Set the authorized database
  if (Database != NULL) {
    DBG("Appending default authorized database to authorized database ...\n");
    //Status = AppendSignatureDatabaseToDatabase(&Database, &DatabaseSize, AuthorizedDatabase, AuthorizedDatabaseSize);
    Status = AppendSignatureToDatabase(&Database, &DatabaseSize, &gEfiCertX509Guid, (VOID *)gSecureBootDatabaseKey, sizeof(gSecureBootDatabaseKey));
    if (EFI_ERROR(Status)) {
      FreePool(Database);
      return Status;
    }
    DBG("Setting authorized database ...\n");
    Status = SetAuthorizedDatabase(Database, DatabaseSize);
    FreePool(Database);
    DatabaseSize = 0;
    Database = NULL;
  } else {
    // Set clover signature as only unless default keys are also wanted
    DBG("Setting authorized database ...\n");
    if (WantDefaultKeys) {
      DBG("No default authorized database found, using built-in default keys ...\n");
      DatabaseSize = 0;
      Database = NULL;
      Status = AppendSignatureToDatabase(&Database, &DatabaseSize, &gEfiCertX509Guid, (VOID *)gSecureBootCanonicalDatabaseKey, sizeof(gSecureBootCanonicalDatabaseKey));
      if (EFI_ERROR(Status)) {
        if (Database != NULL) {
          FreePool(Database);
        }
        DBG("Failed to modify authorized database with Canonical key! %s\n", strerror(Status));
        return Status;
      }
      Status = AppendSignatureToDatabase(&Database, &DatabaseSize, &gEfiCertX509Guid, (VOID *)gSecureBootMSPCADatabaseKey, sizeof(gSecureBootMSPCADatabaseKey));
      if (EFI_ERROR(Status)) {
        if (Database != NULL) {
          FreePool(Database);
        }
        DBG("Failed to modify authorized database with MS PCA key! %s\n", strerror(Status));
        return Status;
      }
      Status = AppendSignatureToDatabase(&Database, &DatabaseSize, &gEfiCertX509Guid, (VOID *)gSecureBootMSUEFICADatabaseKey, sizeof(gSecureBootMSUEFICADatabaseKey));
      if (EFI_ERROR(Status)) {
        if (Database != NULL) {
          FreePool(Database);
        }
        DBG("Failed to modify authorized database with MS UEFICA key! %s\n", strerror(Status));
        return Status;
      }
    }
    Status = AppendSignatureToDatabase(&Database, &DatabaseSize, &gEfiCertX509Guid, (VOID *)gSecureBootDatabaseKey, sizeof(gSecureBootDatabaseKey));
    //Status = AppendSignatureToDatabase(&Database, &DatabaseSize, &gEfiCertX509Guid, AuthorizedDatabase, AuthorizedDatabaseSize);
    Status = SetAuthorizedDatabase(Database, DatabaseSize);
    FreePool(Database);
    DatabaseSize = 0;
    Database = NULL;
    //Status = SetAuthorizedDatabase(AuthorizedDatabase, AuthorizedDatabaseSize);
    // Append keys if needed...
  }
  if (EFI_ERROR(Status)) {
    DBG("Failed to set the authorized database! %s\n", strerror(Status));
    return Status;
  }
  // We set the unauthorized database
  if (WantDefaultKeys) {
    // Get the default authorized database
    DBG("Retrieving the default unauthorized database ...\n");
    DatabaseSize = 0;
    Database = NULL;
    Database = GetSignatureDatabase(DEFAULT_UNAUTHORIZED_DATABASE_NAME, &DEFAULT_UNAUTHORIZED_DATABASE_GUID, &DatabaseSize);
    if ((DatabaseSize == 0) && (Database != NULL)) {
      FreePool(Database);
      Database = NULL;
    }
    // Set the unauthorized database
    if (Database != NULL) {
      Status = SetSignatureDatabase(UNAUTHORIZED_DATABASE_NAME, &UNAUTHORIZED_DATABASE_GUID, Database, DatabaseSize);
      FreePool(Database);
      DatabaseSize = 0;
      if (EFI_ERROR(Status)) {
        DBG("Failed to set the unauthorized database! %s\n", strerror(Status));
        return Status;
      }
    }
  }
  // We need to enroll our own exchange database because we may update databases outside of setup mode
  DatabaseSize = 0;
  Database = NULL;
  if (WantDefaultKeys) {
    // Get the default exchange database
    DBG("Retrieving default exchange database ...\n");
    Database = GetSignatureDatabase(DEFAULT_EXCHANGE_DATABASE_NAME, &DEFAULT_EXCHANGE_DATABASE_GUID, &DatabaseSize);
    if ((DatabaseSize == 0) && (Database != NULL)) {
      FreePool(Database);
      Database = NULL;
      DatabaseSize = 0;
    }
    if (Database == NULL) {
      DBG("No default exchange database found, using built-in default keys ...\n");
      DatabaseSize = 0;
      Database = NULL;
      Status = AppendSignatureToDatabase(&Database, &DatabaseSize, &gEfiCertX509Guid, (VOID *)gSecureBootMSExchangeKey, sizeof(gSecureBootMSExchangeKey));
      if (EFI_ERROR(Status)) {
        if (Database != NULL) {
          FreePool(Database);
        }
        DBG("Failed to modify exchange database with MS exchange key! %s\n", strerror(Status));
        return Status;
      }
    }
  }
  // Set the exchange database
  DBG("Modifying exchange database ...\n");
  Status = AppendSignatureToDatabase(&Database, &DatabaseSize, &gEfiCertX509Guid, (VOID *)gSecureBootExchangeKeyDER, sizeof(gSecureBootExchangeKeyDER));
    
  if (EFI_ERROR(Status)) {
    if (Database != NULL) {
      FreePool(Database);
    }
    DBG("Failed to modify exchange database! %s\n", strerror(Status));
    return Status;
  }
  DBG("Setting the exchange database ...\n");
  Status = SetSignatureDatabase(EXCHANGE_DATABASE_NAME, &EXCHANGE_DATABASE_GUID, Database, DatabaseSize);
  FreePool(Database);
  DatabaseSize = 0;
  Database = NULL;
  if (EFI_ERROR(Status)) {
    DBG("Failed to set exchange database key! %s\n", strerror(Status));
    return Status;
  }
  // Unsure if default platform database should be enrolled.....???
  // Set the platform database - NOT ENROLLING DEFAULT PLATFORM DATABASE, ONLY CLOVER SHOULD OWN PLATFORM(?)
  DBG("Setting the platform database ...\n");
  Status = gRT->SetVariable(PLATFORM_DATABASE_NAME, &PLATFORM_DATABASE_GUID, SET_DATABASE_ATTRIBUTES, sizeof(gSecureBootPlatformSignedKey), (VOID *)gSecureBootPlatformSignedKey);

  return Status;
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
  Database = (__typeof__(Database))AllocateZeroPool(Size);
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

#define IsLeap(y)   (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))
#define SECSPERMIN  (60)
#define SECSPERHOUR (60 * 60)
#define SECSPERDAY  (24 * SECSPERHOUR)

STATIC UINTN CumulativeDays[2][14] = {
  {
    0,
    0,
    31,
    31 + 28,
    31 + 28 + 31,
    31 + 28 + 31 + 30,
    31 + 28 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31
  },
  {
    0,
    0,
    31,
    31 + 29,
    31 + 29 + 31,
    31 + 29 + 31 + 30,
    31 + 29 + 31 + 30 + 31,
    31 + 29 + 31 + 30 + 31 + 30,
    31 + 29 + 31 + 30 + 31 + 30 + 31,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31 
  }
};

STATIC EFI_STATUS GetUTCTime(OUT EFI_TIME *Timestamp)
{
  EFI_STATUS Status;
  UINTN      Timer = 0;
  UINTN      Year, YearNo, MonthNo;
  UINTN      DayNo, DayRemainder;
  if (Timestamp == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  // Get current time
  Status = gRT->GetTime(Timestamp, NULL);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  // Get the current offset of the year in seconds since epoch
  for (Year = 1970; Year < Timestamp->Year; ++Year) {
    Timer += (UINTN)((IsLeap (Year) ? 366 : 365) * SECSPERDAY);
  }
  // Get the current offset
  Timer += (UINTN)((Timestamp->TimeZone != EFI_UNSPECIFIED_TIMEZONE) ? (Timestamp->TimeZone * 60) : 0) +
           (UINTN)(CumulativeDays[IsLeap(Timestamp->Year)][Timestamp->Month] * SECSPERDAY) + 
           (UINTN)(((Timestamp->Day > 0) ? Timestamp->Day - 1 : 0) * SECSPERDAY) + 
           (UINTN)(Timestamp->Hour * SECSPERHOUR) + 
           (UINTN)(Timestamp->Minute * 60) + 
           (UINTN)Timestamp->Second;
  // Convert back to time
  ZeroMem(Timestamp, sizeof(EFI_TIME));
  DayNo        = (UINTN)(Timer / SECSPERDAY);
  DayRemainder = (UINTN)(Timer % SECSPERDAY);

  Timestamp->Second = (UINT8)(DayRemainder % SECSPERMIN);
  Timestamp->Minute = (UINT8)((DayRemainder % SECSPERHOUR) / SECSPERMIN);
  Timestamp->Hour   = (UINT8)(DayRemainder / SECSPERHOUR);

  for (Year = 1970, YearNo = 0; DayNo > 0; ++Year) {
    UINTN TotalDays = (IsLeap(Year) ? 366 : 365);
    if (DayNo >= TotalDays) {
      DayNo = (DayNo - TotalDays);
      YearNo++;
    } else {
      break;
    }
  }
  Timestamp->Year = (UINT16)(YearNo + 1970);

  for (MonthNo = 12; MonthNo > 1; --MonthNo) {
    if (DayNo >= CumulativeDays[IsLeap(Year)][MonthNo]) {
      DayNo = (UINT16) (DayNo - (UINT16) (CumulativeDays[IsLeap(Year)][MonthNo]));
      break;
    }
  }

  Timestamp->Month = (UINT8)MonthNo;
  Timestamp->Day   = (UINT8)(DayNo + 1);
  Timestamp->Nanosecond = 0;
  Timestamp->TimeZone = 0;
  Timestamp->Daylight = 0;
  Timestamp->Pad1 = 0;
  Timestamp->Pad2 = 0;

  return EFI_SUCCESS;
}

// Write signed variable
EFI_STATUS SetSignedVariable(IN CHAR16   *DatabaseName,
                             IN EFI_GUID *DatabaseGuid,
                             IN UINT32    Attributes,
                             IN VOID     *Database,
                             IN UINTN     DatabaseSize)
{
  EFI_STATUS                     Status;
  EFI_VARIABLE_AUTHENTICATION_2 *Authentication;
  UINTN                          Size, NameLen;
  UINTN                          DataSize = 0;
  EFI_TIME                       Timestamp;
  VOID                          *Data = NULL;
  BIO                           *BioData = NULL;
  PKCS7                         *p7;
  X509                          *Certificate = NULL;
  EVP_PKEY                      *PrivateKey = NULL;
  const EVP_MD                  *md;
  // Check parameters
  if ((DatabaseName == NULL) || (DatabaseGuid == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  DBG("Setting secure variable: %s %ls 0x%X (0x%X)\n", strguid(DatabaseGuid), DatabaseName, Database, DatabaseSize);
  NameLen = StrLen(DatabaseName);
  if (NameLen == 0) {
    return EFI_INVALID_PARAMETER;
  }
  // Get the current time
  DBG("Getting timestamp ...\n");
  Status = GetUTCTime(&Timestamp);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  DBG("Timestamp: %t\n", Timestamp);
  // In user mode we need to sign the database with exchange key
  if (!gSettings.SecureBootSetupMode) {
    // Initialize the cyphers and digests
    ERR_load_crypto_strings();
    OpenSSL_add_all_digests();
    OpenSSL_add_all_ciphers();
    // Create signing certificate
    BioData = BIO_new_mem_buf((void *)gSecureBootExchangeKey, sizeof(gSecureBootExchangeKey));
    if (BioData == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    Certificate = PEM_read_bio_X509(BioData, NULL, NULL, NULL);
    BIO_free(BioData);
    if (Certificate == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    // Create signing private key
    BioData = BIO_new_mem_buf((void *)gSecureBootExchangePrivateKey, sizeof(gSecureBootExchangePrivateKey));
    if (BioData == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    PrivateKey = PEM_read_bio_PrivateKey(BioData, NULL, NULL, NULL);
    BIO_free(BioData);
    if (PrivateKey == NULL) {
      X509_free(Certificate);
      return EFI_OUT_OF_RESOURCES;
    }
    // Do the actual signing process
    BioData = BIO_new(BIO_s_mem());
    BIO_write(BioData, DatabaseName, (int)StrLen(DatabaseName));
    BIO_write(BioData, DatabaseGuid, sizeof(EFI_GUID));
    BIO_write(BioData, &Attributes, sizeof(UINT32));
    BIO_write(BioData, &Timestamp, sizeof(EFI_TIME));
    BIO_write(BioData, Database, (int)DatabaseSize);

    md = EVP_get_digestbyname("SHA256");

    p7 = PKCS7_new();
    PKCS7_set_type(p7, NID_pkcs7_signed);

    PKCS7_content_new(p7, NID_pkcs7_data);

    PKCS7_sign_add_signer(p7, Certificate, PrivateKey, md, PKCS7_BINARY | PKCS7_DETACHED | PKCS7_NOSMIMECAP);

    PKCS7_set_detached(p7, 1);

    PKCS7_final(p7, BioData, PKCS7_BINARY | PKCS7_DETACHED | PKCS7_NOSMIMECAP);
    
    X509_free(Certificate);
    EVP_PKEY_free(PrivateKey);

    DataSize = i2d_PKCS7(p7, NULL);
    Data = (__typeof__(Data))AllocateZeroPool(DataSize);

    i2d_PKCS7(p7, (unsigned char **)&Data);

    PKCS7_free(p7);

    // Set the authentication buffer size
    Size = sizeof(EFI_TIME) + sizeof(EFI_GUID) + sizeof(UINT32) + sizeof(UINT16) + sizeof(UINT16) + DataSize;
  } else {
    // In setup mode we don't need to sign, so just set the database
    DBG("In setup mode, not signing ...\n");
    Size = sizeof(EFI_TIME) + sizeof(EFI_GUID) + sizeof(UINT32) + sizeof(UINT16) + sizeof(UINT16) + DatabaseSize;
  }
  // Create the authentication buffer
  DBG("Creating authentication ...\n");
  Authentication = (EFI_VARIABLE_AUTHENTICATION_2 *)AllocateZeroPool(Size);
  if (Authentication == NULL) {
    if (Data != NULL) {
      FreePool(Data);
    }
    return EFI_OUT_OF_RESOURCES;
  }
  // Set the certificate elements
  CopyMem(&(Authentication->TimeStamp), &Timestamp, sizeof(EFI_TIME));
  Authentication->AuthInfo.Hdr.dwLength         = (UINT32)(sizeof(EFI_GUID) + sizeof(UINT32) + sizeof(UINT16) + sizeof(UINT16) + DataSize);
  Authentication->AuthInfo.Hdr.wRevision        = 0x0200;
  Authentication->AuthInfo.Hdr.wCertificateType = WIN_CERT_TYPE_EFI_GUID;
  CopyMem(&(Authentication->AuthInfo.CertType), &gEfiCertPkcs7Guid, sizeof(EFI_GUID));
  // Copy the data into the authentication
  if (Data != NULL) {
    CopyMem(((UINT8 *)Authentication) + sizeof(EFI_TIME) + sizeof(EFI_GUID) + sizeof(UINT32) + sizeof(UINT16) + sizeof(UINT16), Data, DataSize);
    FreePool(Data);
  } else {
    CopyMem(((UINT8 *)Authentication) + sizeof(EFI_TIME) + sizeof(EFI_GUID) + sizeof(UINT32) + sizeof(UINT16) + sizeof(UINT16), Database, DatabaseSize); //Payload, PayloadSize);
  }
  DBG("Writing secure variable 0x%X (0x%X) ...\n", Authentication, Size);
  // Write the database variable
  Status = gRT->SetVariable(DatabaseName, DatabaseGuid, SET_DATABASE_ATTRIBUTES, Size, Authentication);
  // Cleanup the authentication buffer
  FreePool(Authentication);
  return Status;
}

// Write signature database
EFI_STATUS SetSignatureDatabase(IN CHAR16   *DatabaseName,
                                IN EFI_GUID *DatabaseGuid,
                                IN VOID     *Database,
                                IN UINTN     DatabaseSize)
{
  EFI_STATUS Status;
  // Check is valid to set database
  if ((gSettings.SecureBoot && gSettings.SecureBootSetupMode) ||
      (!gSettings.SecureBoot && !gSettings.SecureBootSetupMode)) {
    return EFI_NOT_FOUND;
  }
  // Erase database
  Status = SetSignedVariable(DatabaseName, DatabaseGuid, 0, NULL, 0);
  // Check if database was not found which is cool cause we are removing it
  if (Status == EFI_NOT_FOUND) {
    if ((Database == NULL) || (DatabaseSize == 0)) {
      return EFI_SUCCESS;
    }
  } else if (EFI_ERROR(Status) || (Database == NULL) || (DatabaseSize == 0)) {
    // Return status if only erasing
    return Status;
  }
  return SetSignedVariable(DatabaseName, DatabaseGuid, SET_DATABASE_ATTRIBUTES, Database, DatabaseSize);
}

#endif // ENABLE_SECURE_BOOT
