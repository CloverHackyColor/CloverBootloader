/*
 * AcpiPatcher.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_ACPIPATCHER_H_
#define PLATFORM_ACPIPATCHER_H_



#pragma pack(push)
#pragma pack(1)

typedef struct {

  EFI_ACPI_DESCRIPTION_HEADER Header;
  UINT32                      Entry;

} RSDT_TABLE;

typedef struct {

  EFI_ACPI_DESCRIPTION_HEADER Header;
  UINT64                      Entry;

} XSDT_TABLE;

#pragma pack(pop)



extern UINT64                          BiosDsdt;
extern UINT32                          BiosDsdtLen;
#define                                acpi_cpu_max 128
extern UINT8                           acpi_cpu_count;
extern CHAR8                           *acpi_cpu_name[];
extern UINT8                           acpi_cpu_processor_id[];
extern CHAR8                           *acpi_cpu_score;

extern UINT64                    machineSignature;


//ACPI
EFI_STATUS
PatchACPI(IN REFIT_VOLUME *Volume, CHAR8 *OSVersion);

EFI_STATUS
PatchACPI_OtherOS(CONST CHAR16* OsSubdir, BOOLEAN DropSSDT);

UINT8
Checksum8 (
  VOID *startPtr,
  UINT32 len
  );

void FixChecksum(EFI_ACPI_DESCRIPTION_HEADER* Table);


VOID
SaveOemDsdt (
  BOOLEAN FullPatch
  );

VOID
SaveOemTables (VOID);

EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE
*GetFadt (VOID);


VOID
GetAcpiTablesList (VOID);


#endif /* PLATFORM_ACPIPATCHER_H_ */
