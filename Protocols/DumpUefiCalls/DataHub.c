/** @file

  DataHub overrides module.

**/

#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/DataHub.h>

#include "Common.h"


/** Original DataHub protocol. */
EFI_DATA_HUB_PROTOCOL gOrgDataHub;
EFI_DATA_HUB_PROTOCOL *gDataHub;

////////////////////////////////////////////
// 
// DataHub overrides
//


typedef struct _APPLE_SYSTEM_INFO_DATA_RECORD
{
	UINT32		Unknown[4];
	UINT32		NameLength;
	UINT32		ValueLength;
} APPLE_SYSTEM_INFO_DATA_RECORD;


EFI_STATUS EFIAPI
OvrGetNextRecord (
				  IN EFI_DATA_HUB_PROTOCOL		*This,
				  IN OUT  UINT64				*MonotonicCount,
				  IN  EFI_EVENT					*FilterDriver OPTIONAL,
				  OUT EFI_DATA_RECORD_HEADER	**Record
)
{
	EFI_STATUS	Status;
	UINT8		*DataRec;
	UINTN		DataSize;
	
	Status = gOrgDataHub.GetNextRecord(This, MonotonicCount, FilterDriver, Record);
	if (Record && ((*Record)->DataRecordClass == EFI_DATA_RECORD_CLASS_PROGRESS_CODE)) {
		return Status; //print nothing
	}
	
	PRINT("DataHub->GetNextRecord(%ld, %p, %p) = %r\n", *MonotonicCount, FilterDriver, Record != NULL ? *Record : NULL, Status);

	if (EFI_ERROR(Status) || !(Record && *Record)) {
		return Status;
	}
	
	PRINT(" DataHub Record: Version=0x%x, RecordSize=0x%x, LogMonotonicCount=%ld, ",
		  (*Record)->Version, (*Record)->RecordSize, (*Record)->LogMonotonicCount);
	PRINT(" ProducerName=%s\n", GuidStr(&(*Record)->ProducerName));
	PRINT(" DataRecordGuid=%s, DataRecordClass=0x%lx\n",
        GuidStr(&(*Record)->DataRecordGuid), (*Record)->DataRecordClass);
	
	DataRec = (UINT8 *)(*Record) + sizeof(EFI_DATA_RECORD_HEADER);
	
	if (CompareGuid(&(*Record)->ProducerName, &mEfiApplePlatformInfoGuid)) {
		APPLE_SYSTEM_INFO_DATA_RECORD *AppleRec = (APPLE_SYSTEM_INFO_DATA_RECORD*) DataRec;
		CHAR16 NameBuffer[256];
		
		//PrintBytes((CHAR8 *)(*Record), (*Record)->RecordSize);
		
		// We'll print apple specific record
		PRINT(" APPLE_SYSTEM_INFO_DATA_RECORD:\n");
		
		if (AppleRec->NameLength + 2 < sizeof(NameBuffer)) {
			CopyMem(NameBuffer, (DataRec + sizeof(APPLE_SYSTEM_INFO_DATA_RECORD)), AppleRec->NameLength);
			NameBuffer[AppleRec->NameLength / sizeof(CHAR16)] = L'\0';
			PRINT(" Name: %s\n", NameBuffer);
		}
		PRINT(" Value: %d bytes\n", AppleRec->ValueLength);
		// Limit output
		DataSize = AppleRec->ValueLength;
		if (DataSize > 256) {
			DataSize = 256;
		}
		PrintBytes((CHAR8 *)(DataRec + sizeof(APPLE_SYSTEM_INFO_DATA_RECORD) + AppleRec->NameLength), DataSize);
		if (DataSize != AppleRec->ValueLength) {
			PRINT("( ... truncated ... )\n");
		}
	} else {
		// Print the whole record
		// Limit output
		DataSize = (*Record)->RecordSize;
		if (DataSize > 256) {
			DataSize = 256;
		}
		PrintBytes((CHAR8 *)(*Record), DataSize);
		if (DataSize != (*Record)->RecordSize) {
			PRINT("( ... truncated ... )\n");
		}
	}
	return Status;
}


/** Installs our DataHub overrides. */
EFI_STATUS EFIAPI
OvrDataHub(VOID)
{
	EFI_STATUS				Status;
	
	PRINT("Overriding DataHub ...\n");
	
	// Locate DataHub protocol
	Status = gBS->LocateProtocol(&gEfiDataHubProtocolGuid, NULL, (VOID **) &gDataHub);
	if (EFI_ERROR(Status)) {
		PRINT("Error Overriding DataHub: %r\n", Status);
		return Status;
	}
	
	// Store originals
	CopyMem(&gOrgDataHub, gDataHub, sizeof(EFI_DATA_HUB_PROTOCOL));
	
	// Override with our implementation
	gDataHub->GetNextRecord = OvrGetNextRecord;
	
	PRINT("DataHub overriden!\n");
	return EFI_SUCCESS;
}

