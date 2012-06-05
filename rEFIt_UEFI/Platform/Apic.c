///
/// Local APIC NMI Structure
///
typedef struct {
  UINT8   Type;
  UINT8   Length;
  UINT8   AcpiProcessorId;
  UINT16  Flags;
  UINT8   LocalApicLint;
} EFI_ACPI_2_0_LOCAL_APIC_NMI_STRUCTURE;

// -===== APIC =====-
  EFI_ACPI_DESCRIPTION_HEADER                           *ApicTable;
  EFI_ACPI_2_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER*ApicHeader;
  EFI_ACPI_2_0_PROCESSOR_LOCAL_APIC_STRUCTURE           *ProcLocalApic;
  
  xf = ScanXSDT(APIC_SIGN);
  if (xf) {
    ApicTable = (EFI_ACPI_DESCRIPTION_HEADER*)(UINTN)(*xf);
    ApicHeader = (EFI_ACPI_2_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER*)(*xf + sizeof(EFI_ACPI_DESCRIPTION_HEADER));
    ProcLocalApic = (EFI_ACPI_2_0_PROCESSOR_LOCAL_APIC_STRUCTURE *)(*xf + sizeof(EFI_ACPI_2_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER));
    Print(L"ApicTable = 0x%x, ApicHeader = 0x%x, ProcLocalApic = 0x%x\n\r", ApicTable, ApicHeader, ProcLocalApic);
    while ((ProcLocalApic->Type == 0)) { // && (ProcLocalApic->Flags == 1)) {
//      Print(L" ProcId = %d, ApicId = %d, Flags = %d\n\r", ProcLocalApic->AcpiProcessorId, ProcLocalApic->ApicId, ProcLocalApic->Flags);
//      if ((ProcLocalApic->AcpiProcessorId) != ProcLocalApic->ApicId) {
//        ProcLocalApic->AcpiProcessorId = ProcLocalApic->ApicId;
//        Pause(L"Found (ProcId ) != ApicId !!!\n\r");
      }
      ProcLocalApic++;
      Print (L"ProcLocalApic = 0x%x (ProcLocalApic->Length = %d)\n\r", ProcLocalApic, ProcLocalApic->Length);
    }
    ApicTable->Checksum = 0;
    ApicTable->Checksum = (UINT8)(256-Checksum8((CHAR8*)ApicTable,ApicTable->Length));
  } else Pause(L"No APIC table Found !!!\n\r");

UINT8 *SubTable;