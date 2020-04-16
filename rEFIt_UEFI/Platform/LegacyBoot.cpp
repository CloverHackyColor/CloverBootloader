/*++
Slice 2011
LegacyBoot.c - support for boot legacy OS such as WindowsXP and Linux

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

Portion from XOM project
Copyright (c) 2006 JLA 
*/
#include "Platform.h"
#include "LegacyBiosThunk.h"
//#include <Protocol/Bds.h>
#include "AcpiPatcher.h"


#ifndef DEBUG_ALL
#define DEBUG_LBOOT 1
#else
#define DEBUG_LBOOT DEBUG_ALL
#endif

#if DEBUG_LBOOT == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(0, __VA_ARGS__) // until a better solution is found, force DebugLog(0, ...) to prevent saving to DebugLog, which may cause legacy boot to fail
#endif

extern XObjArray<REFIT_VOLUME> Volumes;

#pragma pack(push)
#pragma pack(1)

//template
//CONST MBR_PARTITION_INFO tMBR = {0x80, {0xFE, 0xFF, 0xFF}, 0x06, {0xFE, 0xFF, 0xFF}, 0, 0};
CONST MBR_PARTITION_INFO tMBR = {0x80, {0xFE, 0xFF, 0xFF}, 0xEE, {0xFE, 0xFF, 0xFF}, 0, 0};

typedef struct {
  UINT8 loader[0x1BE];
  MBR_PARTITION_INFO p[4];
  UINT16 signature;
} MBR;

typedef struct {
  UINT64 signature;
  UINT32 revision;
  UINT32 headerSize;
  UINT32 headerCRC;
  UINT32 reserved;
  UINT64 myLBA;
  UINT64 alternateLBA;
  UINT64 firstUsableLBA;
  UINT64 lastUsableLBA;
  EFI_GUID diskGUID;
  UINT64 partitionEntryLBA;
  UINT32 numberOfPartitionEntries;
  UINT32 sizeOfPartitionEntry;
  UINT32 partitionEntryArrayCRC32;
  UINT8  filler[4];  //alignment
} GPT_HEADER;

typedef struct {
  EFI_GUID   partitionType;
  EFI_GUID   partitionGuid;
  UINT64 startingLBA;
  UINT64 endingLBA;
  UINT64 attributes;
  CHAR16 partitionName[72 / 2];
} GPT_ENTRY;

typedef struct Address_t {
  UINT32 offset;
  UINT16 segment;
  UINT8  type;
} Address;

// for BIOS INT 13h AH=42h: Extended Read Sectors From Drive
typedef struct {
	UINT8	size;			// size of struct = 0x10
	UINT8	unused;			// should be 0
	UINT16	numSectors;		// number of sectors
	UINT16	buffOffset;		// segment and offset to the buffer
	UINT16	buffSegment;
	UINT64	lba;			// LBA of starting sector
} BIOS_DISK_ADDRESS_PACKET;

//located at 0x7F00 
CONST UINT8 VideoTest[] = {
  0xb8, 0x02, 0x00,                   //mov ax,2
  0xcd, 0x10,                         //int 0x10
  0x66, 0xbb, 0x0f, 0x00, 0x00, 0x00, //mov ebx, 0x0f
  0x66, 0xb8, 0x38, 0x0e, 0x00, 0x00, //mov eax, 0x0e38
  0x66, 0xb9, 0x10, 0x00, 0x00, 0x00, //mov ecx, 0x10
  0xcd, 0x10,                         //int 0x10
  0xed, 0xe4, 0xfc                    //jmp near 0x7c00
}; //28bytes

#pragma pack(pop)

#define ADDRT_REAL   0   // Segment:Offset (16:16)
#define ADDRT_FLAT   1   // Segment is 0, Offset is 32 bit flat address

EFI_CPU_ARCH_PROTOCOL      *mCpu;
EFI_LEGACY_8259_PROTOCOL   *gLegacy8259;
THUNK_CONTEXT              *mThunkContext = NULL;

UINT8* floppyImage;
EFI_BLOCK_IO* hd82 = NULL;
EFI_BLOCK_IO* hd80 = NULL;
EFI_BLOCK_IO* pCDROMBlockIO = NULL;
EFI_HANDLE hCDROM = NULL;

Address addrRealFromSegOfs(UINT16 segment, UINT16 offset) {
  Address address;
  address.segment = segment;
  address.offset = offset;
  address.type = ADDRT_REAL;
  return address;
}

Address addrFlatFromOffset(UINT32 offset) {
  Address address;
  address.segment = 0;
  address.offset = offset;
  address.type = ADDRT_FLAT;
  return address;
}

Address addrNull() {
  Address address;
  address.type = 0xff;
	address.segment = 0;
	address.offset = 0;
  return address;
}

static UINTN addrPagingEnable;

UINTN addrIsPagingEnabled() {
	return addrPagingEnable;
}

void addrEnablePaging(UINTN f) {
	addrPagingEnable = f;
}

#define ADDR_FLAT_MAX      addrFlatFromOffset(0xFFFFFFFF)
#define ADDR_FLAT_MIN      addrFlatFromOffset(0x00000000)
#define ADDR_REAL_MAX      addrRealFromSegOfs(0xFFFF, 0xFFFF)
#define ADDR_REAL_MIN      addrRealFromSegOfs(0x0000, 0x0000)
#define ADDR_NULL          addrNull()

UINT32  addrToOffset(Address address) {
	if (address.type == ADDRT_REAL)
		return (address.segment << 4) + address.offset;
	if (address.type == ADDRT_FLAT)
		return addrIsPagingEnabled() ? address.offset & 0x7FFFFFFF : address.offset;
	return 0;
}

void*   addrToPointer(Address address) {
	return (UINT8*)(UINTN)addrToOffset(address);
}

static UINTN   addrLT (Address a1, Address a2) { return addrToOffset(a1) <  addrToOffset(a2); }
//static UINTN   addrLTE(Address a1, Address a2) { return addrToOffset(a1) <= addrToOffset(a2); }
//static UINTN   addrGT (Address a1, Address a2) { return addrToOffset(a1) >  addrToOffset(a2); }
static UINTN   addrGTE(Address a1, Address a2) { return addrToOffset(a1) >= addrToOffset(a2); }
//static UINTN   addrEQ (Address a1, Address a2) { return addrToOffset(a1) == addrToOffset(a2); }

Address krnMemoryTop;

/** Reads sectors from given DriveNum with bios into Buffer.
 *  Requires Dap and Buffer to be allocated in legacy memory region.
 */
EFI_STATUS BiosReadSectorsFromDrive(UINT8 DriveNum, UINT64 Lba, UINTN NumSectors, BIOS_DISK_ADDRESS_PACKET *Dap, void *Buffer)
{
	EFI_STATUS			    Status;
	IA32_REGISTER_SET       Regs;
	
	// init disk access packet
	Dap->size = sizeof(BIOS_DISK_ADDRESS_PACKET);
	Dap->unused = 0;
	Dap->numSectors = (UINT16)NumSectors;
	Dap->buffSegment = (UINT16) (((UINTN) Buffer >> 16) << 12);
	Dap->buffOffset = (UINT16) (UINTN) Buffer;
	Dap->lba = Lba;
	
	// set registers
	SetMem (&Regs, sizeof (Regs), 0);
  
	// first reset disk controller as the controller seems to be in an undefined state sometimes
	DBG("Reset disk controller: %X\n", DriveNum);
	Regs.H.AH = 0x00;		// INT 13h AH=00h: Reset disk controller
	Regs.H.DL = DriveNum;
	Status = EFI_SUCCESS;
	if (LegacyBiosInt86(0x13, &Regs)) {
		// TRUE = error
    DBG("Reset 0 disk controller: %X\n", DriveNum);
    Regs.H.AH = 0x0D;		// INT 13h AH=00h: Reset disk controller
    Regs.H.DL = DriveNum;
    if (LegacyBiosInt86(0x13, &Regs)) {
      Status = EFI_NOT_FOUND;
      DBG("reset controller error\n");
		  return Status;
    }
	}
	
	// next, read sector
	Regs.H.AH = 0x42;		// INT 13h AH=42h: Extended Read Sectors From Drive
	Regs.H.DL = DriveNum;
	Regs.E.DS = (UINT16) (((UINTN) Dap >> 16) << 12);
	Regs.X.SI = (UINT16) (UINTN) Dap;
	
//	DBG("Drive: %X, Dap=%p, Buffer=%p, d.size=%X, d.nsect=%d, d.buff=[%X:%X]\n",
//		DriveNum, Dap, Buffer, Dap->size, Dap->numSectors, Dap->buffSegment, Dap->buffOffset);
//	DBG("Dap: Reg.DS:SI = [%X:%X]\n", Regs.E.DS, Regs.X.SI);
	
	Status = EFI_SUCCESS;
	if (LegacyBiosInt86(0x13, &Regs)) {
		// TRUE = error
    Regs.H.AH = 0x01;		// INT 13h AH=01h: Get Status of Last Drive Operation
    LegacyBiosInt86(0x13, &Regs);
		Status = EFI_NOT_FOUND;
	}
	DBG("LegacyBiosInt86 status=%s, AH=%X\n", strerror(Status), Regs.H.AH);
	return Status;
}

/** Reads first 2 sectors from given DriveNum with bios and calculates DriveCRC32.
 *  Requires Dap and Buffer to be allocated in legacy memory region.
 */
EFI_STATUS GetBiosDriveCRC32(UINT8 DriveNum,
                             UINT32 *DriveCRC32,
                             BIOS_DISK_ADDRESS_PACKET *Dap,
                             VOID *Buffer)
{
	EFI_STATUS					Status;
	
	// read first 2 sectors
	Status = BiosReadSectorsFromDrive(DriveNum, 0, 2, Dap, Buffer);
	if (!EFI_ERROR(Status)) {
    *DriveCRC32 = GetCrc32((UINT8*)Buffer, 2 * 512);
		//gBS->CalculateCrc32(Buffer, 2 * 512, DriveCRC32);
		DBG("Bios drive CRC32 = 0x%X\n", *DriveCRC32);
	}
	return Status;
}

/** Scans bios drives 0x80 and up, calculates CRC32 of first 2 sectors and compares it with Volume->DriveCRC32.
 *  First 2 sectors whould be enough - covers MBR and GPT header with signatures.
 *  Requires mThunkContext to be initialiyzed already with InitializeBiosIntCaller().
 */
UINT8 GetBiosDriveNumForVolume(REFIT_VOLUME *Volume)
{
	EFI_STATUS					Status;
	UINT8						DriveNum, BestNum;
	UINT32						DriveCRC32;
	UINT8						*Buffer;
	BIOS_DISK_ADDRESS_PACKET	*Dap;
	UINTN						LegacyRegionPages;
	EFI_PHYSICAL_ADDRESS		LegacyRegion;
	
//	DBG("Expected volume CRC32 = %X\n", Volume->DriveCRC32);
	LegacyRegion = 0x0C0000;
	LegacyRegionPages = EFI_SIZE_TO_PAGES(sizeof(BIOS_DISK_ADDRESS_PACKET) + 2 * 512)+1 /* dap + 2 sectors */;
	Status = gBS->AllocatePages(AllocateMaxAddress,
								EfiBootServicesData,
								LegacyRegionPages,
								&LegacyRegion
								);
	if (EFI_ERROR(Status)) {
		return 0;
	}
	
	Dap = (BIOS_DISK_ADDRESS_PACKET *)(UINTN)LegacyRegion;
	Buffer = (UINT8 *)(UINTN)(LegacyRegion + 0x200);
//Slice - some CD has BIOS driveNum = 0	
	// scan drives from 0x80
  BestNum = 0;
	for (DriveNum = 0x80; DriveNum < 0x88; DriveNum++) {
    DriveCRC32 = 0;
		Status = GetBiosDriveCRC32(DriveNum, &DriveCRC32, Dap, Buffer);
		if (EFI_ERROR(Status)) {
			// error or no more disks
			//DriveNum = 0;
      DBG("Can't get drive 0x%X CRC32\n", DriveNum);
			continue;
		}
    BestNum = DriveNum;
    DBG("Calculated CRC=%X at drive 0x%X\n", Volume->DriveCRC32, BestNum);
		if (Volume->DriveCRC32 == DriveCRC32) {
			break;
		}
	}
	gBS->FreePages(LegacyRegion, LegacyRegionPages);
	DBG("Returning Bios drive %X\n", BestNum);
	return BestNum;
}

EFI_STATUS bootElTorito(REFIT_VOLUME*	volume)
{
	EFI_BLOCK_IO* pBlockIO = volume->BlockIO;
	Address      bootAddress = addrRealFromSegOfs(0x0000, 0x7C00);
	UINT8        *sectorBuffer; //[2048];
	EFI_STATUS   Status = EFI_NOT_FOUND;
	UINT64       lba;
	UINT16       bootLoadSegment;
	Address      bootLoadAddress;
	UINT32       bootSize;
	UINT32       bootSectors;
	IA32_REGISTER_SET           Regs;
   //UINTN         LogSize;
	
  sectorBuffer = (__typeof__(sectorBuffer))AllocateAlignedPages (EFI_SIZE_TO_PAGES (2048), 64);
	krnMemoryTop = addrRealFromSegOfs(0xA000, 0x0000);
	addrEnablePaging(0);
	
	// No device, no game
	if (!pBlockIO) {
		DBG("CDROMBoot: No CDROM to boot from\n");
		if (sectorBuffer) {
		  FreePages(sectorBuffer, EFI_SIZE_TO_PAGES (2048));
		}
		return Status;
	}
	
	// Load El Torito boot record volume descriptor
	Status = pBlockIO->ReadBlocks(pBlockIO, pBlockIO->Media->MediaId, 0x11, 2048, sectorBuffer);
	if (EFI_ERROR(Status)) {
		// Retry in case the CD was swapped out
		Status = gBS->HandleProtocol(volume->DeviceHandle, &gEfiBlockIoProtocolGuid, (VOID **) &pBlockIO);
		if (!EFI_ERROR(Status)) {
			//      pCDROMBlockIO = pBlockIO;
			Status = pBlockIO->ReadBlocks(pBlockIO, pBlockIO->Media->MediaId, 0x11, 2048, sectorBuffer);
		}
		if (EFI_ERROR(Status)) {
			DBG("CDROMBoot: Unable to read block %X: %s\n", 0x11, strerror(Status));
			return Status;
		}
	}
	
	if (AsciiStrCmp((CHAR8*)(sectorBuffer + 0x7), "EL TORITO SPECIFICATION")) {
		DBG("CDROMBoot: Not an El Torito Specification disk\n");
		return Status;
	}
	
	// Find the boot catalog
	lba = sectorBuffer[0x47] + sectorBuffer[0x48] * 256 + sectorBuffer[0x49] * 65536 + sectorBuffer[0x4A] * 16777216;
	Status = pBlockIO->ReadBlocks(pBlockIO, pBlockIO->Media->MediaId, lba, 2048, sectorBuffer);
	if (EFI_ERROR(Status)) {
		DBG("CDROMBoot: Unable to read block %llX: %s\n", lba, strerror(Status));
		return Status;
	}
	
	if (sectorBuffer[0x00] != 1 || sectorBuffer[0x1E] != 0x55 || sectorBuffer[0x1F] != 0xAA) {
		DBG("CDROMBoot: Invalid El Torito validation entry in boot catalog LBA %llX\n", lba);
		//    DumpHex(0, 0, 64, sectorBuffer);
		return Status;
	}
	
	if (sectorBuffer[0x01] != 0) {
		DBG("CDROMBoot: Platform mismatch: %d\n", sectorBuffer[0x01]);
		return Status;
	}
	
	if (sectorBuffer[0x20] != 0x88) {
		DBG("CDROMBoot: CD-ROM is not bootable\n");
		return Status;
	}
	
	if (sectorBuffer[0x21] != 0) {
		DBG("CDROMBoot: Currently only non-emulated CDROMs are supported");
		return Status;
	}
	
	bootLoadSegment = sectorBuffer[0x22] + sectorBuffer[0x23] * 256;
	if (!bootLoadSegment)
		bootLoadSegment = 0x7C0;
	bootSectors = sectorBuffer[0x26] + sectorBuffer[0x27] * 256;
	bootSize = bootSectors * pBlockIO->Media->BlockSize;
	bootLoadAddress = addrRealFromSegOfs(bootLoadSegment, 0);
	if (addrLT(bootLoadAddress, bootAddress) || addrGTE(bootLoadAddress, krnMemoryTop)) {
		DBG("CDROMBoot: Illegal boot load address %XL%X\n", addrToOffset(bootLoadAddress), bootSize);
		return Status;
	}
	
	lba = sectorBuffer[0x28] + sectorBuffer[0x29] * 256 + sectorBuffer[0x2A] * 65536 + sectorBuffer[0x2B] * 16777216;
	DBG("CDROMBoot: Booting LBA %llu @%X L%X\n", lba, addrToOffset(bootLoadAddress), bootSize);
	gBS->FreePages((EFI_PHYSICAL_ADDRESS)(UINTN)sectorBuffer, 1);
	// Read the boot sectors into the boot load address
	Status = pBlockIO->ReadBlocks(pBlockIO, pBlockIO->Media->MediaId, lba, bootSize, addrToPointer(bootLoadAddress));
	if (EFI_ERROR(Status)) {
		DBG("CDROMBoot: Unable to read block %llu: %s\n", lba, strerror(Status));
		return Status;
	}
  
   Status = SaveBooterLog(SelfRootDir, LEGBOOT_LOG);
  if (EFI_ERROR(Status)) {
    DBG("can't save legacy-boot.log\n");
    Status = SaveBooterLog(NULL, LEGBOOT_LOG);
  }
  /*LogSize = msgCursor - msgbuf;
  Status = egSaveFile(SelfRootDir, LEGBOOT_LOG, (UINT8*)msgbuf, LogSize);
  if (EFI_ERROR(Status)) {
    DBG("can't save legacy-boot.log\n");
    Status = egSaveFile(NULL, LEGBOOT_LOG, (UINT8*)msgbuf, LogSize);
  }
  */
    
	
	// Configure drive
	//  hd82 = pBlockIO;
	
	// Initialize Registers
	//  CONTEXT->edx = 0x82;
	
	// Boot it
	//  dbgStart(bootLoadAddress, enableDebugger);
  Status = gBS->LocateProtocol(&gEfiLegacy8259ProtocolGuid, NULL, (VOID**)&gLegacy8259);
	if (EFI_ERROR(Status)) {
		return Status;
	}
/*  mCpu = NULL;
  Status = gBS->LocateProtocol (&gEfiCpuArchProtocolGuid, NULL, (VOID **) &mCpu);
	if (EFI_ERROR(Status)) {
		return Status;
	}
*/
	
	Status = gBS->AllocatePool (EfiBootServicesData,sizeof(THUNK_CONTEXT),(VOID **)&mThunkContext);
	if (EFI_ERROR (Status)) {
		return Status;
	}
	Status = InitializeBiosIntCaller(); //mThunkContext);
	if (EFI_ERROR (Status)) {
		return Status;
	}
  //	InitializeInterruptRedirection(); //gLegacy8259);
  // Status = mCpu->EnableInterrupt(mCpu);  
  
	Regs.X.DX = 0; //0x82;
	//	Regs.X.SI = (UINT16)activePartition;
	//Regs.X.ES = EFI_SEGMENT((UINT32) pBootSector);
	//Regs.X.BX = EFI_OFFSET ((UINT32) pBootSector);
//  LegacyBiosFarCall86(0, 0x7c00, &Regs);
	LegacyBiosFarCall86(
						EFI_SEGMENT((UINT32) addrToOffset(bootLoadAddress)),
						EFI_OFFSET ((UINT32) addrToOffset(bootLoadAddress)),
						&Regs
						);
	
	// Success - Should never get here unless debugger aborts
	return Status;
}

EFI_STATUS bootMBR(REFIT_VOLUME* volume) 
{
	EFI_STATUS			Status;
	EFI_BLOCK_IO*		pDisk			= volume->BlockIO;
	//UINT8*				pMBR			= (void*)0x600;
	UINT8*				pMBR			= (UINT8*)(UINTN)0x7C00;
	//UINT8*				pBootSector		= (void*)0x7C00;
	//MBR_PARTITION_INFO*			activePartition = NULL;
	//UINTN				partitionIndex;
	IA32_REGISTER_SET           Regs;
    UINTN                       i, j;
    UINT8                       BiosDriveNum;
    //UINTN         LogSize;
    
	SetMem (&Regs, sizeof (Regs), 0);
	addrEnablePaging(0);
	
	Status = gBS->LocateProtocol(&gEfiLegacy8259ProtocolGuid, NULL, (VOID**)&gLegacy8259);
	if (EFI_ERROR(Status)) {
		return Status;
	}
  mCpu = NULL;
  Status = gBS->LocateProtocol (&gEfiCpuArchProtocolGuid, NULL, (VOID **) &mCpu);
	if (EFI_ERROR(Status)) {
		return Status;
	}
	
	Status = gBS->AllocatePool (EfiBootServicesData,sizeof(THUNK_CONTEXT),(VOID **)&mThunkContext);
	if (EFI_ERROR (Status)) {
		return Status;
	}
    
    DBG("boot from partition %ls\n", FileDevicePathToStr(volume->DevicePath));
    
	Status = InitializeBiosIntCaller(); //mThunkContext);
	if (EFI_ERROR (Status)) {
		return Status;
	}
	//InitializeInterruptRedirection(); //gLegacy8259);
  //Status = mCpu->EnableInterrupt(mCpu);
	
	// Read the MBR
	Status = pDisk->ReadBlocks(pDisk, pDisk->Media->MediaId, 0, 512, pMBR);
	if (EFI_ERROR(Status)) {
		DBG("HDBoot: Unable to read MBR: %s\n", strerror(Status));
		return Status;
	}
	
    for (i=0; i<16; i++) {
		DBG("%04llX: ", i*16);
        for (j=0; j<16; j++) {
            DBG("%02X ", pMBR[i*16+j]);
        }
        DBG("\n");
    }
  
  Status = SaveBooterLog(SelfRootDir, LEGBOOT_LOG);
  if (EFI_ERROR(Status)) {
    Status = SaveBooterLog(NULL, LEGBOOT_LOG);
  }
   /*
  LogSize = msgCursor - msgbuf;
  Status = egSaveFile(SelfRootDir, LEGBOOT_LOG, (UINT8*)msgbuf, LogSize);
  if (EFI_ERROR(Status)) {
    Status = egSaveFile(NULL, LEGBOOT_LOG, (UINT8*)msgbuf, LogSize);
  }
  */
    
	// Check validity of MBR
	if (pMBR[510] != 0x55 || pMBR[511] != 0xAA) {
		DBG("HDBoot: Invalid MBR signature 0x%02X%02X (not 0xAA55)\n", pMBR[511], pMBR[510]);
		Status = EFI_NOT_FOUND; 
		return Status;
	}
	
	BiosDriveNum = GetBiosDriveNumForVolume(volume);
	if (BiosDriveNum == 0) {
		// not found
		DBG("HDBoot: BIOS drive number not found\n");
		return EFI_NOT_FOUND;
	}
	
    /*
	
	// Traverse partitions
	for (partitionIndex = 0; partitionIndex < 4; ++partitionIndex) {
		MBR_PARTITION_INFO* partition = (MBR_PARTITION_INFO*)(pMBR + 0x1BE + sizeof(MBR_PARTITION_INFO) * partitionIndex);
		
		// Not the active partition?
		if (partition->Flags != 0x80)
			continue;
		
		// Is the partition valid?
		if (partition->StartLBA == 0 || partition->Size == 0) {
			DBG("HDBoot: Invalid active partition %d: (%08X L %08X)\n", partition->StartLBA, partition->Size);
			return Status;
		}
		
		activePartition = partition;
		break;
	}
	
	// No active partitions found?
	if (!activePartition) {
		DBG("HDBoot: No active partitions found.\n");
		Status = EFI_NOT_FOUND;
		return Status;
	}
	
	DBG("HDBoot: Found active partition #%d.\n", partitionIndex);
	
	// Read the boot sector
	Status = pDisk->ReadBlocks(pDisk, pDisk->Media->MediaId, activePartition->StartLBA, 512, pBootSector);
	if (EFI_ERROR(Status)) {
		DBG("HDBoot: Unable to read partition %d's boot sector: %s\n", partitionIndex, strerror(Status));
		Status = EFI_NOT_FOUND;
		return Status;
	}
	
	// Check boot sector
	if (pBootSector[0x1FE] != 0x55 || pBootSector[0x1FF] != 0xAA) {
		DBG("HDBoot: Invalid Boot Sector signature 0x%02X%02X (not 0xAA55)\n", pBootSector[0x1FF], pBootSector[0x1FE]);
		Status = EFI_NOT_FOUND;
		return Status;
	}
	
	DBG("HDBoot: Found valid boot sector on partition #%d. Booting...\n", partitionIndex);
    */
	DBG("HDBoot: Booting...\n");
		
	Regs.X.DX = BiosDriveNum;
	//Regs.X.SI = (UINT16)(UINTN)activePartition;
	LegacyBiosFarCall86(0, 0x7c00, &Regs);
		
	// Success - Should never get here 
	return EFI_SUCCESS;	
}

EFI_STATUS bootPBRtest(REFIT_VOLUME* volume)
{
	EFI_STATUS                  Status		= EFI_NOT_FOUND;
	EFI_BLOCK_IO*               pDisk     = volume->BlockIO;
	UINT8*                      pBootSector	= (UINT8*)(UINTN)0x7C00;
  UINT8*                      mBootSector;
  MBR_PARTITION_INFO*					pMBR      = (MBR_PARTITION_INFO*)(UINTN)0x7BE;
	UINT32                      LbaOffset	= 0;
	UINT32                      LbaSize		= 0;
	HARDDRIVE_DEVICE_PATH       *HdPath     = NULL; 
	EFI_DEVICE_PATH_PROTOCOL    *DevicePath = volume->DevicePath;
    UINT8                       BiosDriveNum;
//  UINT16                      OldMask;
//  UINT16                      NewMask;
  UINTN                       i, j;  //for debug dump
  UINT8                       *ptr;
 // UINT32                      MBRCRC32;
  //UINTN         LogSize;
  EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE		  *FadtPointer = NULL;
  EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE	*Facs = NULL;
	
	IA32_REGISTER_SET   Regs;
	SetMem (&Regs, sizeof (Regs), 0);
	addrEnablePaging(0);
	//
	// find the partition device path node
	//
	while (!IsDevicePathEnd (DevicePath)) {
		if ((DevicePathType (DevicePath) == MEDIA_DEVICE_PATH) &&
        (DevicePathSubType (DevicePath) == MEDIA_HARDDRIVE_DP)) {
			HdPath = (HARDDRIVE_DEVICE_PATH *)DevicePath;
			break;
		}		
		DevicePath = NextDevicePathNode (DevicePath);
	}
	
	if (HdPath != NULL) {
    DBG("boot from partition %ls\n", FileDevicePathToStr((EFI_DEVICE_PATH *)HdPath));
		LbaOffset	= (UINT32)HdPath->PartitionStart;
		LbaSize		= (UINT32)HdPath->PartitionSize;
        DBG("starting from 0x%X LBA \n", LbaOffset);
	} else {
		return Status;
	}
	
	Status = gBS->LocateProtocol(&gEfiLegacy8259ProtocolGuid, NULL, (VOID**)&gLegacy8259);
	if (EFI_ERROR(Status)) {
		return Status;
	}
  DBG("gEfiLegacy8259ProtocolGuid found\n");
	
	Status = gBS->AllocatePool (EfiBootServicesData,sizeof(THUNK_CONTEXT),(VOID **)&mThunkContext);
	if (EFI_ERROR (Status)) {
		return Status;
	}
	Status = InitializeBiosIntCaller(); //mThunkContext);
	if (EFI_ERROR (Status)) {
		return Status;
	}

  mBootSector = (__typeof__(mBootSector))AllocateAlignedPages(1, 16);  
	Status = pDisk->ReadBlocks(pDisk, pDisk->Media->MediaId, 0, 2*512, mBootSector);
  CopyMem(pBootSector, mBootSector, 1024);
  DBG("PBR after readDisk:\n");
  for (i=0; i<4; i++) {
	  DBG("%04llX: ", i*16);
    for (j=0; j<16; j++) {
      DBG("%02X ", pBootSector[i*16+j]);
    }
    DBG("\n");
  }
  DBG("Reset disk controller 0x80\n"); 
  Status = SaveBooterLog(SelfRootDir, LEGBOOT_LOG);
  if (EFI_ERROR(Status)) {
    DBG("can't save legacy-boot.log\n");
    Status = SaveBooterLog(NULL, LEGBOOT_LOG);
  }
  //after reset we can't save boot log
	Regs.H.AH = 0x0D;		// INT 13h AH=00h: Reset floppy disk controller; 0x0D - reset hard disk controller
	Regs.H.DL = 0x80;
	Status = EFI_SUCCESS;
	if (LegacyBiosInt86(0x13, &Regs)) {
		// TRUE = error
		Status = EFI_NOT_FOUND;
    DBG("reset controller 0x80 error\n");
    //		return Status;
	}
  
  BiosDriveNum = GetBiosDriveNumForVolume(volume);
	if (BiosDriveNum == 0) {
		// not found
		DBG("HDBoot: BIOS drive number not found\n");
//    BiosDriveNum = 0x80;
//		return EFI_NOT_FOUND;
	}
  
  //Now I want to start from VideoTest
  ptr = (UINT8*)(UINTN)0x7F00;
  CopyMem(ptr, &VideoTest[0], 30);
    
	CopyMem(pMBR, &tMBR, 16);
	pMBR->StartLBA = LbaOffset;
	pMBR->Size = LbaSize;
  
  FadtPointer = GetFadt();
  if (FadtPointer == NULL) {
    return EFI_NOT_FOUND;
  }
  Facs = (EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE*)(UINTN)(FadtPointer->FirmwareCtrl);
  Facs->FirmwareWakingVector = 0x7F00;
  
  gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
  
  
/*  
	Regs.X.DX = BiosDriveNum;
	Regs.X.SI = 0x07BE;
	//Regs.X.ES = EFI_SEGMENT((UINT32) pBootSector);
	//Regs.X.BX = EFI_OFFSET ((UINT32) pBootSector);
	LegacyBiosFarCall86(0, 0x7F00, &Regs); //0x7c00
*/ 
 
  //if not success then save legacyboot.log
  Status = SaveBooterLog(SelfRootDir, LEGBOOT_LOG);
  if (EFI_ERROR(Status)) {
    DBG("can't save legacy-boot.log\n");
    /*Status = */SaveBooterLog(NULL, LEGBOOT_LOG);
  }

	return EFI_SUCCESS;	
	
}

/*
EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE		*FadtPointer = GetFadt();
if (FadtPointer == NULL) {
  return;
}
Facs = (EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE*)(UINTN)(FadtPointer->FirmwareCtrl);
Facs->FirmwareWakingVector = 0x7F00;

gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
 */

#define EFI_CPU_EFLAGS_IF 0x200

/** For BIOS and some UEFI boots.
 * Loads partition boot record (PBR) and starts it.
 */
EFI_STATUS bootPBR(REFIT_VOLUME* volume, BOOLEAN SataReset)
{
  EFI_STATUS					Status;
  EFI_BLOCK_IO				*pDisk			= volume->BlockIO;
  UINT8               *pBootSector	= (UINT8*)(UINTN)0x7C00;
  UINT8               *mBootSector;
  UINT32                      LbaOffset		= 0;
  UINT32                      LbaSize			= 0;
  HARDDRIVE_DEVICE_PATH       *HdPath			= NULL;
  EFI_DEVICE_PATH_PROTOCOL    *DevicePath		= volume->DevicePath;
  UINT8                       BiosDriveNum;
  //UINT16                      OldMask;
  //UINT16                      NewMask;
  UINTN                       i, j;  //for debug dump
  IA32_REGISTER_SET           Regs;
  //UINTN						LogSize;

  EFI_LEGACY_BIOS_PROTOCOL    *LegacyBios;
  //UINT16                    HddCount;
  //HDD_INFO                  *HddInfo = NULL;
  UINT16                      BbsCount;
  BBS_TABLE                   *BbsTable = NULL;
  BBS_TABLE                   *BbsTableIt = NULL;
  CONST CHAR16                      *BbsPriorityTxt;
  CONST CHAR16                      *BbsDevTypeTxt;
  MBR_PARTITION_INFO          *pMBR = (MBR_PARTITION_INFO*)(UINTN)0x11BE; // typical location boot0 installs it, should be unused otherwise...

  //
  // get EfiLegacy8259Protocol - mandatory
  //
  Status = gBS->LocateProtocol(&gEfiLegacy8259ProtocolGuid, NULL, (VOID**)&gLegacy8259);
  DBG("EfiLegacy8259ProtocolGuid: %s\n", strerror(Status));
  if (EFI_ERROR(Status)) {
    return Status;
  }

  if (SataReset) {
    IoWrite8(0x3F2, 0x0C);
  }

  //
  // get EfiLegacyBiosProtocol - optional
  //
  Status = gBS->LocateProtocol(&gEfiLegacyBiosProtocolGuid, NULL, (VOID**)&LegacyBios);
  DBG("EfiLegacyBiosProtocolGuid: %s\n", strerror(Status));
  if (!EFI_ERROR(Status)) {
    //
    // call PrepareToBootEfi() to init BIOS drives
    //

    //Status = LegacyBios->GetBbsInfo(LegacyBios, &HddCount, &HddInfo, &BbsCount, &BbsTable);
    //DBG("GetBbsInfo = %s, HddCnt=%d, HddInfo=%p, BbsCount=%d, BbsTabl%p\n", strerror(Status), HddCount, HddInfo, BbsCount, BbsTable);
    Status = LegacyBios->PrepareToBootEfi(LegacyBios, &BbsCount, &BbsTable);
    DBG("PrepareToBootEfi = %s, BbsCount=%d, BbsTabl%p\n", strerror(Status), BbsCount, BbsTable);
    //PauseForKey(L"continue ...\n");

    //
    // debug: dump BbsTable
    //
    BbsTableIt = BbsTable;
    for (i=0; i<BbsCount; i++, BbsTableIt++) {

      BbsPriorityTxt = L"";
      switch (BbsTableIt->BootPriority) {
        case BBS_DO_NOT_BOOT_FROM:
          BbsPriorityTxt = L"NOT";
          break;

        case BBS_LOWEST_PRIORITY:
          BbsPriorityTxt = L"LOW";
          break;

        case BBS_UNPRIORITIZED_ENTRY:
          BbsPriorityTxt = L"UNP";
          break;

        case BBS_IGNORE_ENTRY:
          BbsPriorityTxt = L"IGN";
          break;
      }

      BbsDevTypeTxt = L"-";
      switch (BbsTableIt->DeviceType) {
        case BBS_FLOPPY:
          BbsDevTypeTxt = L"FLP";
          break;

        case BBS_HARDDISK:
          BbsDevTypeTxt = L"HDD";
          break;

        case BBS_CDROM:
          BbsDevTypeTxt = L"CDR";
          break;

        case BBS_PCMCIA:
          BbsDevTypeTxt = L"PCM";
          break;

        case BBS_USB:
          BbsDevTypeTxt = L"USB";
          break;

        case BBS_EMBED_NETWORK:
          BbsDevTypeTxt = L"NET";
          break;

        case BBS_BEV_DEVICE:
          BbsDevTypeTxt = L"BEV";
          break;
      }

		DBG("%llu: Drv: %X P: %X %ls PCI(%X,%X,%X), DT: %X %ls SF: %X Txt: '%s'\n",
          i, BbsTableIt->AssignedDriveNumber, BbsTableIt->BootPriority, BbsPriorityTxt,
          BbsTableIt->Bus, BbsTableIt->Device, BbsTableIt->Function,
          BbsTableIt->DeviceType, BbsDevTypeTxt, *(UINT32*)(&BbsTableIt->StatusFlags),
          (CHAR8*)(UINTN)((BbsTableIt->DescStringSegment << 4) + BbsTableIt->DescStringOffset)
          );
    }
    //PauseForKey(L"continue ...\n");
  }

  //
  // find the partition device path node
  //
  while (!IsDevicePathEnd (DevicePath)) {
    if ((DevicePathType (DevicePath) == MEDIA_DEVICE_PATH) &&
        (DevicePathSubType (DevicePath) == MEDIA_HARDDRIVE_DP)) {
      HdPath = (HARDDRIVE_DEVICE_PATH *)DevicePath;
      break;
    }
    DevicePath = NextDevicePathNode (DevicePath);
  }

  if (HdPath != NULL) {
    DBG("boot from partition %ls\n", FileDevicePathToStr((EFI_DEVICE_PATH *)HdPath));
    LbaOffset	= (UINT32)HdPath->PartitionStart;
    LbaSize		= (UINT32)HdPath->PartitionSize;
    DBG("starting from 0x%X LBA \n", LbaOffset);
  } else {
    return Status;
  }

  //
  // prepare ThunkContext for 16bit BIOS calls
  //
  if (mThunkContext == NULL) {
    Status = gBS->AllocatePool (EfiBootServicesData, sizeof(THUNK_CONTEXT), (VOID **)&mThunkContext);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    DBG("Thunk allocated\n");
    Status = InitializeBiosIntCaller(); //mThunkContext);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  //
  // read partition boot record and copy it to BIOS boot area 0000:07C00
  //
  mBootSector = (__typeof__(mBootSector))AllocatePages(1);
  Status = pDisk->ReadBlocks(pDisk, pDisk->Media->MediaId, 0, 1*512, mBootSector);
  CopyMem(pBootSector, mBootSector, 1*512);
  DBG("PBR:\n");
  for (i=0; i<4; i++) {
	  DBG("%04llX: ", i*16);
    for (j=0; j<16; j++) {
      DBG("%02X ", pBootSector[i*16+j]);
    }
    DBG("\n");
  }

  //
  // find parent disk volume and it's BIOS drive num
  // todo: if we managed to get BbsTable, then we may find
  // BIOS drive from there, by matching PCI bus, device, function
  //
  DBG("Looking for parent disk of %ls\n", FileDevicePathToStr(volume->DevicePath));
  BiosDriveNum = 0;
  for (i = 0; i < Volumes.size(); i++) {
    if (&Volumes[i] != volume && Volumes[i].BlockIO == volume->WholeDiskBlockIO)
    {
      BiosDriveNum = GetBiosDriveNumForVolume(&Volumes[i]);
      break;
    }
  }
  if (BiosDriveNum == 0) {
    // not found
    DBG("HDBoot: BIOS drive number not found, using default 0x80\n");
    BiosDriveNum = 0x80;
  }

  //
  // prepare 16bit regs:
  // DX = BIOS drive num
  //
  SetMem (&Regs, sizeof (Regs), 0);
  Regs.X.DX = BiosDriveNum;

  // set up SI to partition table entry, some boot1 boot code (such a boot1f32 and boot1h) depend on it
  if (volume->IsMbrPartition) {
    CopyMem(pMBR, volume->MbrPartitionTable, 4*16); // copy to lower memory, same location as boot0
    Regs.X.SI = (UINT16)(UINTN)&pMBR[volume->MbrPartitionIndex];
  }
  // apparently gpt without mbr, should this be legacy bootable?
  // boot0.s fakes an partition entry, so lets do the same...
  else {
    CopyMem(pMBR, &tMBR, 16);
    pMBR->StartLBA = LbaOffset;
    pMBR->Size = LbaSize;
    Regs.X.SI = (UINT16)(UINTN)pMBR;
  }

	DBG("mbr: %X index: %llX pointer: %llX dx: %X si: %X\n", volume->IsMbrPartition, volume->MbrPartitionIndex, (uintptr_t)volume->MbrPartitionTable, Regs.X.DX, Regs.X.SI);
	DBG("pmbr: %llX start: %X size: %X\n", (uintptr_t)&pMBR[volume->MbrPartitionIndex], pMBR[volume->MbrPartitionIndex].StartLBA, pMBR[volume->MbrPartitionIndex].Size);

  //
  // call 16bit partition boot code
  //
  //PauseForKey(L"Doing  LegacyBiosFarCall86 ...\n");
  LegacyBiosFarCall86(0, 0x7c00, &Regs);
  
  //Status = gLegacy8259->SetMask(gLegacy8259, &OldMask, NULL, NULL, NULL);
  PauseForKey(L"save legacy-boot.log ...\n");
  Status = SaveBooterLog(SelfRootDir, LEGBOOT_LOG);
  if (EFI_ERROR(Status)) {
    DBG("can't save legacy-boot.log\n");
    /*Status = */SaveBooterLog(NULL, LEGBOOT_LOG);
  }
  
  return EFI_SUCCESS;	
}


/** For DefaultLegacyBios (UEFI)
 * Patch BBS Table priorities to allow booting not only from first partition.
 */
static VOID PatchBbsTable(EFI_LEGACY_BIOS_PROTOCOL *LegacyBios, UINT16 BootEntry)
{
	UINT16		Idx;
	UINT16		IdxCount = 0;
	UINT16		Priority = 1;
	UINT16		OldPriority;
	UINT16		HddCount;
	UINT16		BbsCount;
	HDD_INFO	*LocalHddInfo;
	BBS_TABLE	*LocalBbsTable;

	LegacyBios->GetBbsInfo (
		LegacyBios,
		&HddCount,
		&LocalHddInfo,
		&BbsCount,
		&LocalBbsTable
		);

	DBG ("BBS Table of size %d, patching priorities Pold->Pnew:\n", BbsCount);
	DBG (" NO: BBS# Pold Pnew bb/dd/ff cl/sc Type Stat segm:offs\n");
	DBG (" =====================================================\n");

	for (Idx = 0; Idx < BbsCount; Idx++) {
		if ((LocalBbsTable[Idx].BootPriority == BBS_IGNORE_ENTRY) ||
		    (LocalBbsTable[Idx].BootPriority == BBS_DO_NOT_BOOT_FROM) ||
		    (LocalBbsTable[Idx].BootPriority == BBS_LOWEST_PRIORITY)
		    ) {
			continue;
		}

		OldPriority = LocalBbsTable[Idx].BootPriority;
		if (++IdxCount==BootEntry) { 
			LocalBbsTable[Idx].BootPriority = 0;
		} else {
			LocalBbsTable[Idx].BootPriority = Priority++;
		}

		DBG (" %02llu: 0x%02llX %04llX %04llX %02llX/%02llX/%02llX %02llX/%02llX %04llX %04llX %04llX:%04llX\n",
		    (UINTN) IdxCount,
		    (UINTN) Idx,
		    (UINTN) OldPriority,
		    (UINTN) LocalBbsTable[Idx].BootPriority,
		    (UINTN) LocalBbsTable[Idx].Bus,
		    (UINTN) LocalBbsTable[Idx].Device,
		    (UINTN) LocalBbsTable[Idx].Function,
		    (UINTN) LocalBbsTable[Idx].Class,
		    (UINTN) LocalBbsTable[Idx].SubClass,
		    (UINTN) LocalBbsTable[Idx].DeviceType,
		    (UINTN) * (UINT16 *) &LocalBbsTable[Idx].StatusFlags,
		    (UINTN) LocalBbsTable[Idx].BootHandlerSegment,
		    (UINTN) LocalBbsTable[Idx].BootHandlerOffset/*,
		    (UINTN) ((LocalBbsTable[Idx].MfgStringSegment << 4) + LocalBbsTable[Idx].MfgStringOffset),
		    (UINTN) ((LocalBbsTable[Idx].DescStringSegment << 4) + LocalBbsTable[Idx].DescStringOffset)*/
		    );

	}
}

/** For some UEFI boots that have EfiLegacyBiosProtocol.
 * Starts legacy boot from the first BIOS drive.
 */
EFI_STATUS bootLegacyBiosDefault(IN UINT16 LegacyBiosDefaultEntry) 
{
	EFI_STATUS					Status;
	EFI_LEGACY_BIOS_PROTOCOL	*LegacyBios;
	//BBS_BBS_DEVICE_PATH			*BbsDPN;
	BBS_BBS_DEVICE_PATH			*BbsDP = NULL;
	
	
	//
	// get EfiLegacyBiosProtocol - optional
	//
	Status = gBS->LocateProtocol(&gEfiLegacyBiosProtocolGuid, NULL, (VOID**)&LegacyBios);
	DBG("EfiLegacyBiosProtocolGuid: %s\n", strerror(Status));
	if (EFI_ERROR(Status)) {
		return Status;
	}
	
	// Patch BBS Table
	if (LegacyBiosDefaultEntry > 0) {
		PatchBbsTable(LegacyBios, LegacyBiosDefaultEntry);
		/*Status = SaveBooterLog(SelfRootDir, LEGBOOT_LOG);
		if (EFI_ERROR(Status)) {
			DBG("can't save legacy-boot.log\n");
			Status = SaveBooterLog(NULL, LEGBOOT_LOG);
		}*/
	}

	/* commented out - it seems it does not have any effect
	//
	// create BBS device path for HDD
	//
	
	// size - size of struct, no additional String
	BbsDPN = (BBS_BBS_DEVICE_PATH *) CreateDeviceNode(BBS_DEVICE_PATH, BBS_BBS_DP, sizeof(BBS_BBS_DEVICE_PATH));
	BbsDPN->DeviceType = BBS_TYPE_HARDDRIVE; // BBS_TYPE_CDROM;
	BbsDPN->StatusFlag = 0;
	BbsDPN->String[0] = '\0';
	
	// appends end-of-device-path node and returns complete DP
	BbsDP = (BBS_BBS_DEVICE_PATH *) AppendDevicePathNode(NULL, (EFI_DEVICE_PATH_PROTOCOL *) BbsDPN);
	FreePool(BbsDPN);
	*/
	
	//
	// do boot from default MBR hard disk
	//
	Status = LegacyBios->LegacyBoot(LegacyBios, BbsDP, 0, NULL);
	DBG("LegacyBios->LegacyBoot(): %s\n", strerror(Status));
	
	return Status;
}

VOID DumpBiosMemoryMap()
{
  EFI_STATUS                  Status;
  INT32                       i, Length;  //for debug dump
  UINT64                      Start, Size;
  IA32_REGISTER_SET           Regs;
  UINT8*                      BiosMap = (UINT8*)(UINTN)0x7C00;
  
	SetMem (&Regs, sizeof (Regs), 0);
	addrEnablePaging(0);
  
  Status = gBS->LocateProtocol(&gEfiLegacy8259ProtocolGuid, NULL, (VOID**)&gLegacy8259);
	if (EFI_ERROR(Status)) {
		return;
	}
  DBG("gEfiLegacy8259ProtocolGuid found\n");
	
	Status = gBS->AllocatePool (EfiBootServicesData,sizeof(THUNK_CONTEXT),(VOID **)&mThunkContext);
	if (EFI_ERROR (Status)) {
		return;
	}
	Status = InitializeBiosIntCaller(); //mThunkContext);
	if (EFI_ERROR (Status)) {
		return;
	}
  
  Regs.E.EBX  =  0;
  Regs.E.EDI   = 0x7C00;
  do {
    Regs.X.AX   = 0xE820;
    Regs.E.EDX  = 0x534d4150;
    Regs.X.CX   = 24;
    if (LegacyBiosInt86(0x15, &Regs)) {
      DBG("finished by bit C\n");
      break;
    }
    if (Regs.E.EBX == 0) {
      DBG("finished by ebx=0\n");
      break;
    }
    
    Regs.E.EDI += 24;
  } while (Regs.E.EDI < 0x8000);
  
  Length =  ((INT32)(Regs.E.EDI - 0x7c00)) / 24 + 1;
  DBG("BiosMemoryMap length=%d:\n               Start     End     Type  Ext\n", Length);
  
  for (i = 0; i < Length; i++) {
    Start = *(UINT64*)BiosMap;
    Size = *((UINT64*)BiosMap + 1);
	  DBG(" %08llx %08llx %X    %08X\n", Start, Start + Size - 1, *(UINT32*)(BiosMap + 16), *(UINT32*)(BiosMap + 20));
    BiosMap += 24;
  }
  
}
