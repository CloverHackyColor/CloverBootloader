/**
  x64 Long Mode Virtual Memory Management Definitions  

  References:
    1) IA-32 Intel(R) Architecture Software Developer's Manual Volume 1:Basic Architecture, Intel
    2) IA-32 Intel(R) Architecture Software Developer's Manual Volume 2:Instruction Set Reference, Intel
    3) IA-32 Intel(R) Architecture Software Developer's Manual Volume 3:System Programmer's Guide, Intel
    4) AMD64 Architecture Programmer's Manual Volume 2: System Programming

Copyright (c) 2006 - 2011, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Extended by dmazar.

**/  


#define SYS_CODE64_SEL 0x38

#pragma pack(1)

//
// Page-Map Level-4 Offset (PML4) and
// Page-Directory-Pointer Offset (PDPE) entries 4K & 2MB
//

typedef union {
  struct {
    UINT64  Present:1;                // 0 = Not present in memory, 1 = Present in memory
    UINT64  ReadWrite:1;              // 0 = Read-Only, 1= Read/Write
    UINT64  UserSupervisor:1;         // 0 = Supervisor, 1=User
    UINT64  WriteThrough:1;           // 0 = Write-Back caching, 1=Write-Through caching
    UINT64  CacheDisabled:1;          // 0 = Cached, 1=Non-Cached
    UINT64  Accessed:1;               // 0 = Not accessed, 1 = Accessed (set by CPU)
    UINT64  Reserved:1;               // Reserved
    UINT64  MustBeZero:2;             // Must Be Zero
    UINT64  Available:3;              // Available for use by system software
    UINT64  PageTableBaseAddress:40;  // Page Table Base Address
    UINT64  AvabilableHigh:11;        // Available for use by system software
    UINT64  Nx:1;                     // No Execute bit
  } Bits;
  UINT64    Uint64;
} PAGE_MAP_AND_DIRECTORY_POINTER;

//
// Page Table Entry 4KB
//
typedef union {
  struct {
    UINT64  Present:1;                // 0 = Not present in memory, 1 = Present in memory
    UINT64  ReadWrite:1;              // 0 = Read-Only, 1= Read/Write
    UINT64  UserSupervisor:1;         // 0 = Supervisor, 1=User
    UINT64  WriteThrough:1;           // 0 = Write-Back caching, 1=Write-Through caching
    UINT64  CacheDisabled:1;          // 0 = Cached, 1=Non-Cached
    UINT64  Accessed:1;               // 0 = Not accessed, 1 = Accessed (set by CPU)
    UINT64  Dirty:1;                  // 0 = Not Dirty, 1 = written by processor on access to page
    UINT64  PAT:1;                    //
    UINT64  Global:1;                 // 0 = Not global page, 1 = global page TLB not cleared on CR3 write
    UINT64  Available:3;              // Available for use by system software
    UINT64  PageTableBaseAddress:40;  // Page Table Base Address
    UINT64  AvabilableHigh:11;        // Available for use by system software
    UINT64  Nx:1;                     // 0 = Execute Code, 1 = No Code Execution
  } Bits;
  UINT64    Uint64;
} PAGE_TABLE_4K_ENTRY;

//
// Page Table Entry 2MB
//
typedef union {
  struct {
    UINT64  Present:1;                // 0 = Not present in memory, 1 = Present in memory
    UINT64  ReadWrite:1;              // 0 = Read-Only, 1= Read/Write
    UINT64  UserSupervisor:1;         // 0 = Supervisor, 1=User
    UINT64  WriteThrough:1;           // 0 = Write-Back caching, 1=Write-Through caching
    UINT64  CacheDisabled:1;          // 0 = Cached, 1=Non-Cached
    UINT64  Accessed:1;               // 0 = Not accessed, 1 = Accessed (set by CPU)
    UINT64  Dirty:1;                  // 0 = Not Dirty, 1 = written by processor on access to page
    UINT64  MustBe1:1;                // Must be 1 
    UINT64  Global:1;                 // 0 = Not global page, 1 = global page TLB not cleared on CR3 write
    UINT64  Available:3;              // Available for use by system software
    UINT64  PAT:1;                    //
    UINT64  MustBeZero:8;             // Must be zero;
    UINT64  PageTableBaseAddress:31;  // Page Table Base Address
    UINT64  AvabilableHigh:11;        // Available for use by system software
    UINT64  Nx:1;                     // 0 = Execute Code, 1 = No Code Execution
  } Bits;
  UINT64    Uint64;
} PAGE_TABLE_2M_ENTRY;

//
// Page Table Entry 1GB
//
typedef union {
  struct {
    UINT64  Present:1;                // 0 = Not present in memory, 1 = Present in memory
    UINT64  ReadWrite:1;              // 0 = Read-Only, 1= Read/Write
    UINT64  UserSupervisor:1;         // 0 = Supervisor, 1=User
    UINT64  WriteThrough:1;           // 0 = Write-Back caching, 1=Write-Through caching
    UINT64  CacheDisabled:1;          // 0 = Cached, 1=Non-Cached
    UINT64  Accessed:1;               // 0 = Not accessed, 1 = Accessed (set by CPU)
    UINT64  Dirty:1;                  // 0 = Not Dirty, 1 = written by processor on access to page
    UINT64  MustBe1:1;                // Must be 1 
    UINT64  Global:1;                 // 0 = Not global page, 1 = global page TLB not cleared on CR3 write
    UINT64  Available:3;              // Available for use by system software
    UINT64  PAT:1;                    //
    UINT64  MustBeZero:17;            // Must be zero;
    UINT64  PageTableBaseAddress:22;  // Page Table Base Address
    UINT64  AvabilableHigh:11;        // Available for use by system software
    UINT64  Nx:1;                     // 0 = Execute Code, 1 = No Code Execution
  } Bits;
  UINT64    Uint64;
} PAGE_TABLE_1G_ENTRY;

typedef union {
  struct {
    UINT64  PhysPgOffset:12;          // 0 = Physical Page Offset
    UINT64  PTOffset:9;               // 0 = Page Table Offset
    UINT64  PDOffset:9;               // 0 = Page Directory Offset
    UINT64  PDPOffset:9;              // 0 = Page Directory Pointer Offset
    UINT64  PML4Offset:9;             // 0 = Page Map Level 4 Offset
    UINT64  SignExtend:16;            // 0 = Sign Extend
  } Pg4K;
  struct {
    UINT64  PhysPgOffset:21;          // 0 = Physical Page Offset
    UINT64  PDOffset:9;               // 0 = Page Directory Offset
    UINT64  PDPOffset:9;              // 0 = Page Directory Pointer Offset
    UINT64  PML4Offset:9;             // 0 = Page Map Level 4 Offset
    UINT64  SignExtend:16;            // 0 = Sign Extend
  } Pg2M;
  struct {
    UINT64  PhysPgOffset:30;          // 0 = Physical Page Offset
    UINT64  PDPOffset:9;              // 0 = Page Directory Pointer Offset
    UINT64  PML4Offset:9;             // 0 = Page Map Level 4 Offset
    UINT64  SignExtend:16;            // 0 = Sign Extend
  } Pg1G;
  UINT64    Uint64;
} VIRTUAL_ADDR;

#define VA_FIX_SIGN_EXTEND(VA)	VA.Pg4K.SignExtend = (VA.Pg4K.PML4Offset & 0x100) ? 0xFFFF : 0;

#pragma pack()


// 64 bit
#define CR3_ADDR_MASK	0x000FFFFFFFFFF000
#define CR3_FLAG_PWT	0x0000000000000008
#define CR3_FLAG_PCD	0x0000000000000010

#define PT_ADDR_MASK_4K	0x000FFFFFFFFFF000
#define PT_ADDR_MASK_2M	0x000FFFFFFFE00000
#define PT_ADDR_MASK_1G	0x000FFFFFC0000000


/** Returns pointer to PML4 table in PageTable and PWT and PCD flags in Flags. */
VOID
GetCurrentPageTable(PAGE_MAP_AND_DIRECTORY_POINTER **PageTable, UINTN *Flags);

/** Prints given PageTable. */
VOID
PrintPageTable(PAGE_MAP_AND_DIRECTORY_POINTER *PageTable, UINTN Flags);

/** Returns physical addr for given virtual addr. */
EFI_STATUS 
GetPhysicalAddr(PAGE_MAP_AND_DIRECTORY_POINTER *PageTable, EFI_VIRTUAL_ADDRESS  VirtualAddr, EFI_PHYSICAL_ADDRESS *PhysicalAddr);

/** Inits vm memory pool. Should be called while boot services are still usable. */
EFI_STATUS
VmAllocateMemoryPool(VOID);

/** Maps (remaps) 4K page given by VirtualAddr to PhysicalAddr page in PageTable. */
EFI_STATUS
VmMapVirtualPage(PAGE_MAP_AND_DIRECTORY_POINTER *PageTable, EFI_VIRTUAL_ADDRESS VirtualAddr, EFI_PHYSICAL_ADDRESS PhysicalAddr);

/** Maps (remaps) NumPages 4K pages given by VirtualAddr to PhysicalAddr pages in PageTable. */
EFI_STATUS
VmMapVirtualPages(PAGE_MAP_AND_DIRECTORY_POINTER *PageTable, EFI_VIRTUAL_ADDRESS VirtualAddr, UINTN NumPages, EFI_PHYSICAL_ADDRESS PhysicalAddr);

/** Flashes TLB caches. */
VOID
VmFlashCaches(VOID);
