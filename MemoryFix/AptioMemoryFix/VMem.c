/**

  Virtual memory functions.

  by dmazar

**/

#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>

#include "Config.h"
#include "VMem.h"
#include "MemoryMap.h"

/** Memory allocation for VM map pages that we will create with VmMapVirtualPage.
  * We need to have it preallocated during boot services.
  */
UINT8  *VmMemoryPool = NULL;
INTN   VmMemoryPoolFreePages = 0;

VOID
GetCurrentPageTable (
  PAGE_MAP_AND_DIRECTORY_POINTER  **PageTable,
  UINTN                           *Flags
  )
{
  UINTN   CR3;

  CR3 = AsmReadCr3();
  DEBUG ((DEBUG_VERBOSE, "GetCurrentPageTable: CR3 = 0x%lx\n", CR3));
  *PageTable = (PAGE_MAP_AND_DIRECTORY_POINTER*)(UINTN)(CR3 & CR3_ADDR_MASK);
  *Flags = CR3 & (CR3_FLAG_PWT | CR3_FLAG_PCD);
}

VOID
PrintPageTablePTE (
  PAGE_TABLE_4K_ENTRY  *PTE,
  VIRTUAL_ADDR         VA
  )
{
#if !defined (MDEPKG_NDEBUG)
  UINTN                Index;
  UINT64               Start;

  for (Index = 0; Index < 10; Index++) {
    VA.Pg4K.PTOffset = Index;
    DEBUG ((DEBUG_VERBOSE, "      PTE %03x at %p = %lx => ", Index, PTE, PTE->Uint64));
    // 4KB PTE
    Start = (PTE->Uint64 & PT_ADDR_MASK_4K);
    DEBUG ((DEBUG_VERBOSE, "4KB Fl: %lx VA: %lx - %lx ==> PH %lx - %lx\n",
      (PTE->Uint64 & ~PT_ADDR_MASK_4K), VA.Uint64, VA.Uint64 + 0x1000 - 1, Start, Start + 0x1000 - 1));
    PTE++;
  }
#endif
}

VOID
PrintPageTablePDE (
  PAGE_MAP_AND_DIRECTORY_POINTER  *PDE,
  VIRTUAL_ADDR                    VA
  )
{
#if !defined (MDEPKG_NDEBUG)
  UINTN                           Index;
  PAGE_TABLE_2M_ENTRY             *PT2M;
  PAGE_TABLE_4K_ENTRY             *PTE;
  UINT64                          Start;

  for (Index = 0; Index < 10; Index++) {
    VA.Pg4K.PDOffset = Index;
    DEBUG ((DEBUG_VERBOSE, "    PDE %03x at %p = %lx => ", Index, PDE, PDE->Uint64));
    if (PDE->Bits.MustBeZero & 0x1) {
      // 2MB PDE
      PT2M = (PAGE_TABLE_2M_ENTRY *)PDE;
      Start = (PT2M->Uint64 & PT_ADDR_MASK_2M);
      DEBUG ((DEBUG_VERBOSE, "2MB Fl: %lx VA: %lx - %lx ==> PH %lx - %lx\n",
        (PT2M->Uint64 & ~PT_ADDR_MASK_2M), VA.Uint64, VA.Uint64 + 0x200000 - 1, Start, Start + 0x200000 - 1));
    } else {
      DEBUG ((DEBUG_VERBOSE, "  Fl: %lx %lx ->\n", (PDE->Uint64 & ~PT_ADDR_MASK_4K), (PDE->Uint64 & PT_ADDR_MASK_4K)));
      PTE = (PAGE_TABLE_4K_ENTRY *)(PDE->Uint64 & PT_ADDR_MASK_4K);
      PrintPageTablePTE(PTE, VA);
    }
    PDE++;
  }
#endif
}

VOID
PrintPageTablePDPE (
  PAGE_MAP_AND_DIRECTORY_POINTER  *PDPE,
  VIRTUAL_ADDR                    VA
  )
{
#if !defined (MDEPKG_NDEBUG)
  UINTN                           Index;
  PAGE_TABLE_1G_ENTRY             *PT1G;
  PAGE_MAP_AND_DIRECTORY_POINTER  *PDE;
  UINT64                          Start;

  for (Index = 0; Index < 10; Index++) {
    VA.Pg4K.PDPOffset = Index;
    DEBUG ((DEBUG_VERBOSE, "  PDPE %03x at %p = %lx => ", Index, PDPE, PDPE->Uint64));
    if (PDPE->Bits.MustBeZero & 0x1) {
      // 1GB PDPE
      PT1G = (PAGE_TABLE_1G_ENTRY *)PDPE;
      Start = (PT1G->Uint64 & PT_ADDR_MASK_1G);
      DEBUG ((DEBUG_VERBOSE, "1GB Fl: %lx VA: %lx - %lx ==> PH %lx - %lx\n",
        (PT1G->Uint64 & ~PT_ADDR_MASK_1G), VA.Uint64, VA.Uint64 + 0x40000000 - 1, Start, Start + 0x40000000 - 1));
    } else {
      DEBUG ((DEBUG_VERBOSE, "  Fl: %lx %lx ->\n", (PDPE->Uint64 & ~PT_ADDR_MASK_4K), (PDPE->Uint64 & PT_ADDR_MASK_4K)));
      PDE = (PAGE_MAP_AND_DIRECTORY_POINTER *)(PDPE->Uint64 & PT_ADDR_MASK_4K);
      PrintPageTablePDE(PDE, VA);
    }
    PDPE++;
  }
#endif
}

VOID
PrintPageTable (
  PAGE_MAP_AND_DIRECTORY_POINTER  *PageTable,
  UINTN                           Flags
  )
{
  UINTN                           Index;
  VIRTUAL_ADDR                    VA;
  PAGE_MAP_AND_DIRECTORY_POINTER  *PML4;
  PAGE_MAP_AND_DIRECTORY_POINTER  *PDPE;

  DEBUG ((DEBUG_VERBOSE, "PrintPageTable: %p, Flags: PWT: %d, PCD: %d\n", PageTable, (Flags & CR3_FLAG_PWT), (Flags & CR3_FLAG_PCD)));
  PML4 = PageTable;
  for (Index = 0; Index < 3; Index++) {
    VA.Uint64 = 0;
    VA.Pg4K.PML4Offset = Index;
    VA_FIX_SIGN_EXTEND(VA);
    DEBUG ((DEBUG_VERBOSE, "PML4 %03x at %p = %lx => Fl: %lx %lx ->\n", Index, PML4, PML4->Uint64, (PML4->Uint64 & ~PT_ADDR_MASK_4K), (PML4->Uint64 & PT_ADDR_MASK_4K)));
    PDPE = (PAGE_MAP_AND_DIRECTORY_POINTER *)(PML4->Uint64 & PT_ADDR_MASK_4K);
    PrintPageTablePDPE(PDPE, VA);
    PML4++;
  }
}

EFI_STATUS
GetPhysicalAddr (
  PAGE_MAP_AND_DIRECTORY_POINTER   *PageTable,
  EFI_VIRTUAL_ADDRESS              VirtualAddr,
  EFI_PHYSICAL_ADDRESS             *PhysicalAddr
  )
{
  EFI_PHYSICAL_ADDRESS            Start;
  VIRTUAL_ADDR                    VA;
  VIRTUAL_ADDR                    VAStart;
  VIRTUAL_ADDR                    VAEnd;
  PAGE_MAP_AND_DIRECTORY_POINTER  *PML4;
  PAGE_MAP_AND_DIRECTORY_POINTER  *PDPE;
  PAGE_MAP_AND_DIRECTORY_POINTER  *PDE;
  PAGE_TABLE_4K_ENTRY             *PTE4K;
  PAGE_TABLE_2M_ENTRY             *PTE2M;
  PAGE_TABLE_1G_ENTRY             *PTE1G;

  VA.Uint64 = (UINT64)VirtualAddr;
  //VA_FIX_SIGN_EXTEND(VA);
  DEBUG ((DEBUG_VERBOSE, "PageTable: %p\n", PageTable));
  DEBUG ((DEBUG_VERBOSE, "VA: %lx => Indexes PML4=%x, PDP=%x, PD=%x, PT=%x\n",
    VA.Uint64, VA.Pg4K.PML4Offset, VA.Pg4K.PDPOffset, VA.Pg4K.PDOffset, VA.Pg4K.PTOffset));

  // PML4
  PML4 = PageTable;
  PML4 += VA.Pg4K.PML4Offset;
  // prepare region start and end
  VAStart.Uint64 = 0;
  VAStart.Pg4K.PML4Offset = VA.Pg4K.PML4Offset;
  VA_FIX_SIGN_EXTEND(VAStart);
  VAEnd.Uint64 = ~(UINT64)0;
  VAEnd.Pg4K.PML4Offset = VA.Pg4K.PML4Offset;
  VA_FIX_SIGN_EXTEND(VAEnd);
  // print it
  DEBUG ((DEBUG_VERBOSE, "PML4[%03x] at %p = %lx Region: %lx - %lx\n", VA.Pg4K.PML4Offset, PML4, PML4->Uint64, VAStart.Uint64, VAEnd.Uint64));
  if (!PML4->Bits.Present) {
    DEBUG ((DEBUG_VERBOSE, "-> Mapping not present!\n"));
    return EFI_NO_MAPPING;
  }
  DEBUG ((DEBUG_VERBOSE, "-> Nx:%x|A:%x|PCD:%x|PWT:%x|US:%x|RW:%x|P:%x -> %lx\n",
    PML4->Bits.Nx, PML4->Bits.Accessed,
    PML4->Bits.CacheDisabled, PML4->Bits.WriteThrough,
    PML4->Bits.UserSupervisor, PML4->Bits.ReadWrite, PML4->Bits.Present,
    (PML4->Uint64 & PT_ADDR_MASK_4K)
    ));

  // PDPE
  PDPE = (PAGE_MAP_AND_DIRECTORY_POINTER *)(PML4->Uint64 & PT_ADDR_MASK_4K);
  PDPE += VA.Pg4K.PDPOffset;
  VAStart.Pg4K.PDPOffset = VA.Pg4K.PDPOffset;
  VAEnd.Pg4K.PDPOffset = VA.Pg4K.PDPOffset;
  DEBUG ((DEBUG_VERBOSE, "PDPE[%03x] at %p = %lx Region: %lx - %lx\n", VA.Pg4K.PDPOffset, PDPE, PDPE->Uint64, VAStart.Uint64, VAEnd.Uint64));
  if (!PDPE->Bits.Present) {
    DEBUG ((DEBUG_VERBOSE, "-> Mapping not present!\n"));
    return EFI_NO_MAPPING;
  }
  if (PDPE->Bits.MustBeZero & 0x1) {
    // 1GB PDPE
    PTE1G = (PAGE_TABLE_1G_ENTRY *)PDPE;
    DEBUG ((DEBUG_VERBOSE, "-> Nx:%x|G:%x|PAT:%x|D:%x|A:%x|PCD:%x|PWT:%x|US:%x|RW:%x|P:%x\n",
      PTE1G->Bits.Nx, PTE1G->Bits.Global, PTE1G->Bits.PAT,
      PTE1G->Bits.Dirty, PTE1G->Bits.Accessed,
      PTE1G->Bits.CacheDisabled, PTE1G->Bits.WriteThrough,
      PTE1G->Bits.UserSupervisor, PTE1G->Bits.ReadWrite, PTE1G->Bits.Present
      ));
    Start = (PTE1G->Uint64 & PT_ADDR_MASK_1G);
    *PhysicalAddr = Start + VA.Pg1G.PhysPgOffset;
    DEBUG ((DEBUG_VERBOSE, "-> 1GB page %lx - %lx => %lx\n", Start, Start + 0x40000000 - 1, *PhysicalAddr));
    return EFI_SUCCESS;
  }
  DEBUG ((DEBUG_VERBOSE, "-> Nx:%x|A:%x|PCD:%x|PWT:%x|US:%x|RW:%x|P:%x -> %lx\n",
    PDPE->Bits.Nx, PDPE->Bits.Accessed,
    PDPE->Bits.CacheDisabled, PDPE->Bits.WriteThrough,
    PDPE->Bits.UserSupervisor, PDPE->Bits.ReadWrite, PDPE->Bits.Present,
    (PDPE->Uint64 & PT_ADDR_MASK_4K)
    ));

  // PDE
  PDE = (PAGE_MAP_AND_DIRECTORY_POINTER *)(PDPE->Uint64 & PT_ADDR_MASK_4K);
  PDE += VA.Pg4K.PDOffset;
  VAStart.Pg4K.PDOffset = VA.Pg4K.PDOffset;
  VAEnd.Pg4K.PDOffset = VA.Pg4K.PDOffset;
  DEBUG ((DEBUG_VERBOSE, "PDE[%03x] at %p = %lx Region: %lx - %lx\n", VA.Pg4K.PDOffset, PDE, PDE->Uint64, VAStart.Uint64, VAEnd.Uint64));
  if (!PDE->Bits.Present) {
    DEBUG ((DEBUG_VERBOSE, "-> Mapping not present!\n"));
    return EFI_NO_MAPPING;
  }
  if (PDE->Bits.MustBeZero & 0x1) {
    // 2MB PDE
    PTE2M = (PAGE_TABLE_2M_ENTRY *)PDE;
    DEBUG ((DEBUG_VERBOSE, "-> Nx:%x|G:%x|PAT:%x|D:%x|A:%x|PCD:%x|PWT:%x|US:%x|RW:%x|P:%x\n",
      PTE2M->Bits.Nx, PTE2M->Bits.Global, PTE2M->Bits.PAT,
      PTE2M->Bits.Dirty, PTE2M->Bits.Accessed,
      PTE2M->Bits.CacheDisabled, PTE2M->Bits.WriteThrough,
      PTE2M->Bits.UserSupervisor, PTE2M->Bits.ReadWrite, PTE2M->Bits.Present
      ));
    Start = (PTE2M->Uint64 & PT_ADDR_MASK_2M);
    *PhysicalAddr = Start + VA.Pg2M.PhysPgOffset;
    DEBUG ((DEBUG_VERBOSE, "-> 2MB page %lx - %lx => %lx\n", Start, Start + 0x200000 - 1, *PhysicalAddr));
    return EFI_SUCCESS;
  }
  DEBUG ((DEBUG_VERBOSE, "-> Nx:%x|A:%x|PCD:%x|PWT:%x|US:%x|RW:%x|P:%x -> %lx\n",
    PDE->Bits.Nx, PDE->Bits.Accessed,
    PDE->Bits.CacheDisabled, PDE->Bits.WriteThrough,
    PDE->Bits.UserSupervisor, PDE->Bits.ReadWrite, PDE->Bits.Present,
    (PDE->Uint64 & PT_ADDR_MASK_4K)
    ));

  // PTE
  PTE4K = (PAGE_TABLE_4K_ENTRY *)(PDE->Uint64 & PT_ADDR_MASK_4K);
  PTE4K += VA.Pg4K.PTOffset;
  VAStart.Pg4K.PTOffset = VA.Pg4K.PTOffset;
  VAEnd.Pg4K.PTOffset = VA.Pg4K.PTOffset;
  DEBUG ((DEBUG_VERBOSE, "PTE[%03x] at %p = %lx Region: %lx - %lx\n", VA.Pg4K.PTOffset, PTE4K, PTE4K->Uint64, VAStart.Uint64, VAEnd.Uint64));
  if (!PTE4K->Bits.Present) {
    DEBUG ((DEBUG_VERBOSE, "-> Mapping not present!\n"));
    return EFI_NO_MAPPING;
  }
  DEBUG ((DEBUG_VERBOSE, "-> Nx:%x|G:%x|PAT:%x|D:%x|A:%x|PCD:%x|PWT:%x|US:%x|RW:%x|P:%x -> %lx\n",
    PTE4K->Bits.Nx, PTE4K->Bits.Global, PTE4K->Bits.PAT,
    PTE4K->Bits.Dirty, PTE4K->Bits.Accessed,
    PTE4K->Bits.CacheDisabled, PTE4K->Bits.WriteThrough,
    PTE4K->Bits.UserSupervisor, PTE4K->Bits.ReadWrite, PTE4K->Bits.Present,
    (PTE4K->Uint64 & PT_ADDR_MASK_4K)
    ));
  Start = (PTE4K->Uint64 & PT_ADDR_MASK_4K);
  *PhysicalAddr = Start + VA.Pg4K.PhysPgOffset;

  return EFI_SUCCESS;
}

/** Inits vm memory pool. Should be called while boot services are still usable. */
EFI_STATUS
VmAllocateMemoryPool (
  VOID
  )
{
  EFI_STATUS              Status;
  EFI_PHYSICAL_ADDRESS    Addr;

  if (VmMemoryPool != NULL) {
    // already allocated
    return EFI_SUCCESS;
  }

  VmMemoryPoolFreePages = 0x200; // 2 MB should be enough
  Addr = BASE_4GB; // max address

  Status = AllocatePagesFromTop (EfiBootServicesData, VmMemoryPoolFreePages, &Addr, FALSE);
  if (EFI_ERROR(Status)) {
    Print (L"AMF: vm memory pool allocation failure - %r\n", Status);
  } else {
    VmMemoryPool = (UINT8*)Addr;
    DEBUG ((DEBUG_VERBOSE, "VmMemoryPool = %lx - %lx\n", VmMemoryPool, VmMemoryPool + EFI_PAGES_TO_SIZE(VmMemoryPoolFreePages) - 1));
  }
  return Status;
}

/** Central method for allocating pages for VM page maps. */
VOID *
VmAllocatePages (
  UINTN NumPages
  )
{
  VOID   *AllocatedPages = NULL;

  if (VmMemoryPoolFreePages >= (INTN)NumPages) {
    AllocatedPages = VmMemoryPool;
    VmMemoryPool += EFI_PAGES_TO_SIZE(NumPages);
    VmMemoryPoolFreePages -= NumPages;
  } else {
    DEBUG ((DEBUG_INFO, "VmAllocatePages - no more pages!\n"));
    CpuDeadLoop();
  }
  return AllocatedPages;
}

/** Maps (remaps) 4K page given by VirtualAddr to PhysicalAddr page in PageTable. */
EFI_STATUS
VmMapVirtualPage (
  PAGE_MAP_AND_DIRECTORY_POINTER  *PageTable,
  EFI_VIRTUAL_ADDRESS             VirtualAddr,
  EFI_PHYSICAL_ADDRESS            PhysicalAddr
  )
{
  EFI_PHYSICAL_ADDRESS            Start;
  VIRTUAL_ADDR                    VA;
  VIRTUAL_ADDR                    VAStart;
  VIRTUAL_ADDR                    VAEnd;
  PAGE_MAP_AND_DIRECTORY_POINTER  *PML4;
  PAGE_MAP_AND_DIRECTORY_POINTER  *PDPE;
  PAGE_MAP_AND_DIRECTORY_POINTER  *PDE;
  PAGE_TABLE_4K_ENTRY             *PTE4K;
  PAGE_TABLE_4K_ENTRY             *PTE4KTmp;
  PAGE_TABLE_2M_ENTRY             *PTE2M;
  PAGE_TABLE_1G_ENTRY             *PTE1G;
  UINTN                           Index;

  VA.Uint64 = (UINT64)VirtualAddr;
  //VA_FIX_SIGN_EXTEND(VA);
  DEBUG ((DEBUG_VERBOSE, "VmMapVirtualPage VA %lx => PA %lx\nPageTable: %p\n", VirtualAddr, PhysicalAddr, PageTable));
  DEBUG ((DEBUG_VERBOSE, "VA: %lx => Indexes PML4=%x, PDP=%x, PD=%x, PT=%x\n",
    VA.Uint64, VA.Pg4K.PML4Offset, VA.Pg4K.PDPOffset, VA.Pg4K.PDOffset, VA.Pg4K.PTOffset));

  // PML4
  PML4 = PageTable;
  PML4 += VA.Pg4K.PML4Offset;
  // there is a problem if our PML4 points to the same table as first PML4 entry
  // since we may mess the mapping of first virtual region (happens in VBox and probably DUET).
  // check for this on first call and if true, just clear our PML4 - we'll rebuild it in later step
  if (PML4 != PageTable && PML4->Bits.Present && PageTable->Bits.PageTableBaseAddress == PML4->Bits.PageTableBaseAddress) {
    DEBUG ((DEBUG_VERBOSE, "PML4 points to the same table as first PML4 - releasing it and rebuiding in a separate table\n"));
    PML4->Uint64 = 0;
  }

  // prepare region start and end
  VAStart.Uint64 = 0;
  VAStart.Pg4K.PML4Offset = VA.Pg4K.PML4Offset;
  VA_FIX_SIGN_EXTEND(VAStart);
  VAEnd.Uint64 = ~(UINT64)0;
  VAEnd.Pg4K.PML4Offset = VA.Pg4K.PML4Offset;
  VA_FIX_SIGN_EXTEND(VAEnd);
  // print it
  DEBUG ((DEBUG_VERBOSE, "PML4[%03x] at %p = %lx Region: %lx - %lx\n", VA.Pg4K.PML4Offset, PML4, PML4->Uint64, VAStart.Uint64, VAEnd.Uint64));
  if (!PML4->Bits.Present) {
    DEBUG ((DEBUG_VERBOSE, "-> Mapping not present, creating new PML4 entry and page with PDPE entries!\n"));
    PDPE = (PAGE_MAP_AND_DIRECTORY_POINTER *)VmAllocatePages(1);
    if (PDPE == NULL) {
      DEBUG ((DEBUG_VERBOSE, "No memory - exiting.\n"));
      return EFI_NO_MAPPING;
    }

    ZeroMem(PDPE, EFI_PAGE_SIZE);
    // init this whole 512GB region with 512 1GB entry pages to map first 512GB phys space
    PTE1G = (PAGE_TABLE_1G_ENTRY *)PDPE;
    Start = 0;
    for (Index = 0; Index < 512; Index++) {
      PTE1G->Uint64 = Start & PT_ADDR_MASK_1G;
      PTE1G->Bits.ReadWrite = 1;
      PTE1G->Bits.Present = 1;
      PTE1G->Bits.MustBe1 = 1;
      PTE1G++;
      Start += 0x40000000;
    }

    // put it to PML4
    PML4->Uint64 = ((UINT64)PDPE) & PT_ADDR_MASK_4K;
    PML4->Bits.ReadWrite = 1;
    PML4->Bits.Present = 1;
    DEBUG ((DEBUG_VERBOSE, "added to PLM4 as %lx\n", PML4->Uint64));
    // and continue with mapping ...
  }
  DEBUG ((DEBUG_VERBOSE, "-> Nx:%x|A:%x|PCD:%x|PWT:%x|US:%x|RW:%x|P:%x -> %lx\n",
    PML4->Bits.Nx, PML4->Bits.Accessed,
    PML4->Bits.CacheDisabled, PML4->Bits.WriteThrough,
    PML4->Bits.UserSupervisor, PML4->Bits.ReadWrite, PML4->Bits.Present,
    (PML4->Uint64 & PT_ADDR_MASK_4K)
    ));

  // PDPE
  PDPE = (PAGE_MAP_AND_DIRECTORY_POINTER *)(PML4->Uint64 & PT_ADDR_MASK_4K);
  PDPE += VA.Pg4K.PDPOffset;
  VAStart.Pg4K.PDPOffset = VA.Pg4K.PDPOffset;
  VAEnd.Pg4K.PDPOffset = VA.Pg4K.PDPOffset;
  DEBUG ((DEBUG_VERBOSE, "PDPE[%03x] at %p = %lx Region: %lx - %lx\n", VA.Pg4K.PDPOffset, PDPE, PDPE->Uint64, VAStart.Uint64, VAEnd.Uint64));
  if (!PDPE->Bits.Present || (PDPE->Bits.MustBeZero & 0x1)) {
    DEBUG ((DEBUG_VERBOSE, "-> Mapping not present or mapped as 1GB page, creating new PDPE entry and page with PDE entries!\n"));
    PDE = (PAGE_MAP_AND_DIRECTORY_POINTER *)VmAllocatePages(1);
    if (PDE == NULL) {
      DEBUG ((DEBUG_VERBOSE, "No memory - exiting.\n"));
      return EFI_NO_MAPPING;
    }
    ZeroMem(PDE, EFI_PAGE_SIZE);

    if (PDPE->Bits.MustBeZero & 0x1) {
      // was 1GB page - init new PDE array to get the same mapping but with 2MB pages
      DEBUG ((DEBUG_VERBOSE, "-> was 1GB page, initing new PDE array to get the same mapping but with 2MB pages!\n"));
      PTE2M = (PAGE_TABLE_2M_ENTRY *)PDE;
      Start = (PDPE->Uint64 & PT_ADDR_MASK_1G);
      for (Index = 0; Index < 512; Index++) {
        PTE2M->Uint64 = Start & PT_ADDR_MASK_2M;
        PTE2M->Bits.ReadWrite = 1;
        PTE2M->Bits.Present = 1;
        PTE2M->Bits.MustBe1 = 1;
        PTE2M++;
        Start += 0x200000;
      }
    }

    // put it to PDPE
    PDPE->Uint64 = ((UINT64)PDE) & PT_ADDR_MASK_4K;
    PDPE->Bits.ReadWrite = 1;
    PDPE->Bits.Present = 1;
    DEBUG ((DEBUG_VERBOSE, "added to PDPE as %lx\n", PDPE->Uint64));
    // and continue with mapping ...
  }
  DEBUG ((DEBUG_VERBOSE, "-> Nx:%x|A:%x|PCD:%x|PWT:%x|US:%x|RW:%x|P:%x -> %lx\n",
    PDPE->Bits.Nx, PDPE->Bits.Accessed,
    PDPE->Bits.CacheDisabled, PDPE->Bits.WriteThrough,
    PDPE->Bits.UserSupervisor, PDPE->Bits.ReadWrite, PDPE->Bits.Present,
    (PDPE->Uint64 & PT_ADDR_MASK_4K)
    ));

  // PDE
  PDE = (PAGE_MAP_AND_DIRECTORY_POINTER *)(PDPE->Uint64 & PT_ADDR_MASK_4K);
  PDE += VA.Pg4K.PDOffset;
  VAStart.Pg4K.PDOffset = VA.Pg4K.PDOffset;
  VAEnd.Pg4K.PDOffset = VA.Pg4K.PDOffset;
  DEBUG ((DEBUG_VERBOSE, "PDE[%03x] at %p = %lx Region: %lx - %lx\n", VA.Pg4K.PDOffset, PDE, PDE->Uint64, VAStart.Uint64, VAEnd.Uint64));
  if (!PDE->Bits.Present || (PDE->Bits.MustBeZero & 0x1)) {
    DEBUG ((DEBUG_VERBOSE, "-> Mapping not present or mapped as 2MB page, creating new PDE entry and page with PTE4K entries!\n"));
    PTE4K = (PAGE_TABLE_4K_ENTRY *)VmAllocatePages(1);
    if (PTE4K == NULL) {
      DEBUG ((DEBUG_VERBOSE, "No memory - exiting.\n"));
      return EFI_NO_MAPPING;
    }
    ZeroMem(PTE4K, EFI_PAGE_SIZE);

    if (PDE->Bits.MustBeZero & 0x1) {
      // was 2MB page - init new PTE array to get the same mapping but with 4KB pages
      DEBUG ((DEBUG_VERBOSE, "-> was 2MB page - initing new PTE array to get the same mapping but with 4KB pages!\n"));
      PTE4KTmp = (PAGE_TABLE_4K_ENTRY *)PTE4K;
      Start = (PDE->Uint64 & PT_ADDR_MASK_2M);
      for (Index = 0; Index < 512; Index++) {
        PTE4KTmp->Uint64 = Start & PT_ADDR_MASK_4K;
        PTE4KTmp->Bits.ReadWrite = 1;
        PTE4KTmp->Bits.Present = 1;
        PTE4KTmp++;
        Start += 0x1000;
      }
    }

    // put it to PDE
    PDE->Uint64 = ((UINT64)PTE4K) & PT_ADDR_MASK_4K;
    PDE->Bits.ReadWrite = 1;
    PDE->Bits.Present = 1;
    DEBUG ((DEBUG_VERBOSE, "added to PDE as %lx\n", PDE->Uint64));
    // and continue with mapping ...
  }
  DEBUG ((DEBUG_VERBOSE, "-> Nx:%x|A:%x|PCD:%x|PWT:%x|US:%x|RW:%x|P:%x -> %lx\n",
    PDE->Bits.Nx, PDE->Bits.Accessed,
    PDE->Bits.CacheDisabled, PDE->Bits.WriteThrough,
    PDE->Bits.UserSupervisor, PDE->Bits.ReadWrite, PDE->Bits.Present,
    (PDE->Uint64 & PT_ADDR_MASK_4K)
    ));

  // PTE
  PTE4K = (PAGE_TABLE_4K_ENTRY *)(PDE->Uint64 & PT_ADDR_MASK_4K);
  PTE4K += VA.Pg4K.PTOffset;
  VAStart.Pg4K.PTOffset = VA.Pg4K.PTOffset;
  VAEnd.Pg4K.PTOffset = VA.Pg4K.PTOffset;
  DEBUG ((DEBUG_VERBOSE, "PTE[%03x] at %p = %lx Region: %lx - %lx\n", VA.Pg4K.PTOffset, PTE4K, PTE4K->Uint64, VAStart.Uint64, VAEnd.Uint64));
  if (PTE4K->Bits.Present) {
    DEBUG ((DEBUG_VERBOSE, "mapping already present - remapping!\n"));
  }
  DEBUG ((DEBUG_VERBOSE, "-> Nx:%x|G:%x|PAT:%x|D:%x|A:%x|PCD:%x|PWT:%x|US:%x|RW:%x|P:%x -> %lx\n",
    PTE4K->Bits.Nx, PTE4K->Bits.Global, PTE4K->Bits.PAT,
    PTE4K->Bits.Dirty, PTE4K->Bits.Accessed,
    PTE4K->Bits.CacheDisabled, PTE4K->Bits.WriteThrough,
    PTE4K->Bits.UserSupervisor, PTE4K->Bits.ReadWrite, PTE4K->Bits.Present,
    (PTE4K->Uint64 & PT_ADDR_MASK_4K)
    ));
  // put it to PTE
  PTE4K->Uint64 = ((UINT64)PhysicalAddr) & PT_ADDR_MASK_4K;
  PTE4K->Bits.ReadWrite = 1;
  PTE4K->Bits.Present = 1;
  DEBUG ((DEBUG_VERBOSE, "added to PTE4K as %lx\n", PTE4K->Uint64));

  return EFI_SUCCESS;

}

/** Maps (remaps) NumPages 4K pages given by VirtualAddr to PhysicalAddr pages in PageTable. */
EFI_STATUS
VmMapVirtualPages (
  PAGE_MAP_AND_DIRECTORY_POINTER  *PageTable,
  EFI_VIRTUAL_ADDRESS             VirtualAddr,
  UINTN                           NumPages,
  EFI_PHYSICAL_ADDRESS            PhysicalAddr
  )
{
  EFI_STATUS  Status;

  Status = EFI_SUCCESS;
  while (NumPages > 0 && (Status == EFI_SUCCESS)) {
    Status = VmMapVirtualPage(PageTable, VirtualAddr, PhysicalAddr);
    VirtualAddr += 0x1000;
    PhysicalAddr += 0x1000;
    NumPages--;
    DEBUG ((DEBUG_VERBOSE, "NumPages: %d, %lx => %lx\n", NumPages, VirtualAddr, PhysicalAddr));
  }
  return Status;
}

/** Flashes TLB caches. */
VOID
VmFlushCaches (
  VOID
  )
{
  // just reload CR3
  AsmWriteCr3(AsmReadCr3());
}
