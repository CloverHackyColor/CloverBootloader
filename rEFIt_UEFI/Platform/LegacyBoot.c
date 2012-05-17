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

#define DEBUG_LBOOT 1

#if DEBUG_LBOOT == 2
#define DBG(x...) AsciiPrint(x)
#elif DEBUG_LBOOT == 1
#define DBG(x...) MsgLog(x)
#else
#define DBG(x...)
#endif

#pragma pack(1)

//template
CONST MBR_PARTITION_INFO tMBR = {0x80, {0xFE, 0xFF, 0xFF}, 0x06, {0xFE, 0xFF, 0xFF}, 0, 0};

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


#pragma pack(0)

#define ADDRT_REAL   0   // Segment:Offset (16:16)
#define ADDRT_FLAT   1   // Segment is 0, Offset is 32 bit flat address

EFI_CPU_ARCH_PROTOCOL      *mCpu;
EFI_LEGACY_8259_PROTOCOL   *gLegacy8259;
THUNK_CONTEXT              *mThunkContext;

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
	EFI_STATUS			Status;
	IA32_REGISTER_SET   Regs;
	
	// init disk access packet
	Dap->size = sizeof(BIOS_DISK_ADDRESS_PACKET);
	Dap->unused = 0;
	Dap->numSectors = NumSectors;
	Dap->buffSegment = (UINT16) (((UINTN) Buffer >> 16) << 12);
	Dap->buffOffset = (UINT16) (UINTN) Buffer;
	Dap->lba = Lba;
	
	// set registers
	gBS->SetMem (&Regs, sizeof (Regs), 0);
	Regs.H.AH = 0x42;		// INT 13h AH=42h: Extended Read Sectors From Drive
	Regs.H.DL = DriveNum;
	Regs.E.DS = (UINT16) (((UINTN) Dap >> 16) << 12);
	Regs.X.SI = (UINT16) (UINTN) Dap;
	
	DBG("Drive: %x, Dap=%p, Buffer=%p, d.size=%X, d.nsect=%d, d.buff=[%X:%X]\n",
		  DriveNum, Dap, Buffer, Dap->size, Dap->numSectors, Dap->buffSegment, Dap->buffOffset);
	DBG("Dap: Reg.DS:SI = [%X:%X]\n", Regs.E.DS, Regs.X.SI);
	
	Status = EFI_SUCCESS;
	if (LegacyBiosInt86(0x13, &Regs)) {
		// TRUE = error
		Status = EFI_NOT_FOUND;
	}
	DBG("LegacyBiosInt86=%r, AH=%x\n", Status, Regs.H.AH);
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
    *DriveCRC32 = GetCrc32(Buffer, 2 * 512);
		//gBS->CalculateCrc32(Buffer, 2 * 512, DriveCRC32);
		DBG("Bios drive CRC32 = 0x%x\n", *DriveCRC32);
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
	UINT16						DriveNum, BestNum;
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
			continue;
		}
    BestNum = DriveNum;
    DBG("Calculated CRC=%X at drive 0x%x\n", DriveCRC32, BestNum);
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
	UINT32       bootLoadSegment;
	Address      bootLoadAddress;
	UINT32       bootSize;
	UINT32       bootSectors;
	IA32_REGISTER_SET           Regs;
	
  sectorBuffer = AllocateAlignedPages (EFI_SIZE_TO_PAGES (2048), 64);
	krnMemoryTop = addrRealFromSegOfs(0xA000, 0x0000);
	addrEnablePaging(0);
	
	// No device, no game
	if (!pBlockIO) {
		Print(L"CDROMBoot: No CDROM to boot from\n");
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
			Print(L"CDROMBoot: Unable to read block %X: %r\n", 0x11, Status);
			return Status;
		}
	}
	
	if (AsciiStrCmp((CHAR8*)(sectorBuffer + 0x7), "EL TORITO SPECIFICATION")) {
		Print(L"CDROMBoot: Not an El Torito Specification disk\n");
		return Status;
	}
	
	// Find the boot catalog
	lba = sectorBuffer[0x47] + sectorBuffer[0x48] * 256 + sectorBuffer[0x49] * 65536 + sectorBuffer[0x4A] * 16777216;
	Status = pBlockIO->ReadBlocks(pBlockIO, pBlockIO->Media->MediaId, lba, 2048, sectorBuffer);
	if (EFI_ERROR(Status)) {
		Print(L"CDROMBoot: Unable to read block %X: %r\n", lba, Status);
		return Status;
	}
	
	if (sectorBuffer[0x00] != 1 || sectorBuffer[0x1E] != 0x55 || sectorBuffer[0x1F] != 0xAA) {
		Print(L"CDROMBoot: Invalid El Torito validation entry in boot catalog LBA %X\n", lba);
		//    DumpHex(0, 0, 64, sectorBuffer);
		return Status;
	}
	
	if (sectorBuffer[0x01] != 0) {
		Print(L"CDROMBoot: Platform mismatch: %d\n", sectorBuffer[0x01]);
		return Status;
	}
	
	if (sectorBuffer[0x20] != 0x88) {
		Print(L"CDROMBoot: CD-ROM is not bootable\n");
		return Status;
	}
	
	if (sectorBuffer[0x21] != 0) {
		Print(L"CDROMBoot: Currently only non-emulated CDROMs are supported");
		return Status;
	}
	
	bootLoadSegment = sectorBuffer[0x22] + sectorBuffer[0x23] * 256;
	if (!bootLoadSegment)
		bootLoadSegment = 0x7C0;
	bootSectors = sectorBuffer[0x26] + sectorBuffer[0x27] * 256;
	bootSize = bootSectors * pBlockIO->Media->BlockSize;
	bootLoadAddress = addrRealFromSegOfs(bootLoadSegment, 0);
	if (addrLT(bootLoadAddress, bootAddress) || addrGTE(bootLoadAddress, krnMemoryTop)) {
		Print(L"CDROMBoot: Illegal boot load address %xL%x\n", addrToOffset(bootLoadAddress), bootSize);
		return Status;
	}
	
	lba = sectorBuffer[0x28] + sectorBuffer[0x29] * 256 + sectorBuffer[0x2A] * 65536 + sectorBuffer[0x2B] * 16777216;
	DBG("CDROMBoot: Booting LBA %ld @%x L%x\n", lba, addrToOffset(bootLoadAddress), bootSize);
	gBS->FreePages((EFI_PHYSICAL_ADDRESS)(UINTN)sectorBuffer, 1);
	// Read the boot sectors into the boot load address
	Status = pBlockIO->ReadBlocks(pBlockIO, pBlockIO->Media->MediaId, lba, bootSize, addrToPointer(bootLoadAddress));
	if (EFI_ERROR(Status)) {
		Print(L"CDROMBoot: Unable to read block %ld: %r\n", lba, Status);
		return Status;
	}
  
  UINTN         LogSize;  
  LogSize = msgCursor - msgbuf;
  Status = egSaveFile(SelfRootDir, L"EFI\\misc\\legacy_boot.log", (UINT8*)msgbuf, LogSize);
  if (EFI_ERROR(Status)) {
    Print(L"can't save legacy-boot.log\n");
    Status = egSaveFile(NULL, L"EFI\\misc\\legacy_boot.log", (UINT8*)msgbuf, LogSize);
  }
    
	
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
	InitializeBiosIntCaller(); //mThunkContext);
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
	EFI_STATUS			Status			= EFI_NOT_FOUND;
	EFI_BLOCK_IO*		pDisk			= volume->BlockIO;
	//UINT8*				pMBR			= (void*)0x600;
	UINT8*				pMBR			= (void*)0x7C00;
	//UINT8*				pBootSector		= (void*)0x7C00;
	//MBR_PARTITION_INFO*			activePartition = NULL;
	//UINTN				partitionIndex;
	IA32_REGISTER_SET           Regs;
    UINTN                       i, j;
    UINT8                       BiosDriveNum;
    
	gBS->SetMem (&Regs, sizeof (Regs), 0);
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
    
    DBG("boot from partition %s\n", DevicePathToStr(volume->DevicePath));
    
	InitializeBiosIntCaller(); //mThunkContext);
	//InitializeInterruptRedirection(); //gLegacy8259);
  //Status = mCpu->EnableInterrupt(mCpu);
	
	// Read the MBR
	Status = pDisk->ReadBlocks(pDisk, pDisk->Media->MediaId, 0, 512, pMBR);
	if (EFI_ERROR(Status)) {
		Print(L"HDBoot: Unable to read MBR: %r\n", Status);
		return Status;
	}
	
    for (i=0; i<16; i++) {
        DBG("%04x: ", i*16);
        for (j=0; j<16; j++) {
            DBG("%02x ", pMBR[i*16+j]);
        }
        DBG("\n");
    }
  
  UINTN         LogSize;  
  LogSize = msgCursor - msgbuf;
  Status = egSaveFile(SelfRootDir, L"EFI\\misc\\legacy_boot.log", (UINT8*)msgbuf, LogSize);
  if (EFI_ERROR(Status)) {
    Status = egSaveFile(NULL, L"EFI\\misc\\legacy_boot.log", (UINT8*)msgbuf, LogSize);
  }
    
	// Check validity of MBR
	if (pMBR[510] != 0x55 || pMBR[511] != 0xAA) {
		Print(L"HDBoot: Invalid MBR signature 0x%02X%02X (not 0xAA55)\n", pMBR[511], pMBR[510]);
		Status = EFI_NOT_FOUND; 
		return Status;
	}
	
	BiosDriveNum = GetBiosDriveNumForVolume(volume);
	if (BiosDriveNum == 0) {
		// not found
		Print(L"HDBoot: BIOS drive number not found\n");
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
			Print(L"HDBoot: Invalid active partition %d: (%08X L %08X)\n", partition->StartLBA, partition->Size);
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
		Print(L"HDBoot: Unable to read partition %d's boot sector: %r\n", partitionIndex, Status);
		Status = EFI_NOT_FOUND;
		return Status;
	}
	
	// Check boot sector
	if (pBootSector[0x1FE] != 0x55 || pBootSector[0x1FF] != 0xAA) {
		Print(L"HDBoot: Invalid Boot Sector signature 0x%02X%02X (not 0xAA55)\n", pBootSector[0x1FF], pBootSector[0x1FE]);
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

EFI_STATUS bootPBR(REFIT_VOLUME* volume) 
{
	EFI_STATUS                  Status		= EFI_NOT_FOUND;
	EFI_BLOCK_IO*               pDisk     = volume->BlockIO;
	UINT8*                      pBootSector	= (UINT8*)0x7C00;
  UINT8*                      mBootSector;
  MBR_PARTITION_INFO*					pMBR      = (MBR_PARTITION_INFO*)0x7BE;
	UINT32                      LbaOffset	= 0;
	UINT32                      LbaSize		= 0;
	HARDDRIVE_DEVICE_PATH       *HdPath     = NULL; 
	EFI_DEVICE_PATH_PROTOCOL    *DevicePath = volume->DevicePath;
    UINT8                       BiosDriveNum;
  UINT16                      OldMask;
  UINT16                      NewMask;
  UINTN                       i, i2, j;  //for debug dump
  UINT8                       *ptr;
  UINT32                      MBRCRC32;
  
	
	IA32_REGISTER_SET   Regs;
	gBS->SetMem (&Regs, sizeof (Regs), 0);
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
    DBG("boot from partition %s\n", DevicePathToStr((EFI_DEVICE_PATH *)HdPath));
		LbaOffset	= HdPath->PartitionStart;
		LbaSize		= HdPath->PartitionSize;
        DBG("starting from 0x%x LBA \n", LbaOffset);
	} else {
		return Status;
	}
	
	Status = gBS->LocateProtocol(&gEfiLegacy8259ProtocolGuid, NULL, (VOID**)&gLegacy8259);
	if (EFI_ERROR(Status)) {
		return Status;
	}
  DBG("gEfiLegacy8259ProtocolGuid found\n");
	/*
  mCpu = NULL;
  Status = gBS->LocateProtocol (&gEfiCpuArchProtocolGuid, NULL, (VOID **) &mCpu);
	if (EFI_ERROR(Status)) {
		return Status;
	}
  Status = mCpu->EnableInterrupt(mCpu);
  DBG("gEfiCpuArchProtocolGuid found\n");
	 */
  Status = gLegacy8259->GetMask(gLegacy8259, &OldMask, NULL, NULL, NULL);
	
	Status = gBS->AllocatePool (EfiBootServicesData,sizeof(THUNK_CONTEXT),(VOID **)&mThunkContext);
	if (EFI_ERROR (Status)) {
		return Status;
	}
  DBG("Thunk allocated\n");
	InitializeBiosIntCaller(); //mThunkContext);
	//InitializeInterruptRedirection(); //gLegacy8259);
  mBootSector = AllocateAlignedPages(1, 16);  
	Status = pDisk->ReadBlocks(pDisk, pDisk->Media->MediaId, 0, 2*512, mBootSector);
  DBG("PBR:\n");
    for (i=0; i<4; i++) {
        DBG("%04x: ", i*16);
        for (j=0; j<16; j++) {
            DBG("%02x ", pBootSector[i*16+j]);
        }
        DBG("\n");
    }
  CopyMem(pBootSector, mBootSector, 1024);
//  mBootSector = AllocateAlignedPages(1, 16);
    // find parent disk volume and it's bios drive num
    DBG("Looking for parent disk of %s\n", DevicePathToStr(volume->DevicePath));
    BiosDriveNum = 0;
    for (i = 0; i < VolumesCount; i++) {
        if (Volumes[i] != volume && Volumes[i]->BlockIO == volume->WholeDiskBlockIO)
        {
        	MBRCRC32 = 0;
            DBG("Found parent volume: %s\n", DevicePathToStr(Volumes[i]->DevicePath));
          Status = volume->WholeDiskBlockIO->ReadBlocks(volume->WholeDiskBlockIO, 
                              volume->WholeDiskBlockIO->Media->MediaId, 0, 2*512, 
                              mBootSector);
          DBG("MBR:\n");
          for (i2=0; i<4; i2++) {
            DBG("%04x: ", i2*16);
            for (j=0; j<16; j++) {
              DBG("%02x ", mBootSector[i2*16+j]);
            }
            DBG("\n");
          }
          //gBS->CalculateCrc32(mBootSector, 2 * 512, &MBRCRC32);
          MBRCRC32 = GetCrc32(mBootSector, 2 * 512);
          DBG("MBR drive CRC32 = 0x%x\n", MBRCRC32);
            BiosDriveNum = GetBiosDriveNumForVolume(Volumes[i]);
          break;
        }
    }
  gBS->FreePages((EFI_PHYSICAL_ADDRESS)(UINTN)mBootSector, 1);
    if (BiosDriveNum == 0) {
        // not found
      DBG("HDBoot: BIOS drive number not found, using default 0x80\n");
      BiosDriveNum = 0x80;
//        return EFI_NOT_FOUND;
    }
  
  UINTN         LogSize;  
  LogSize = msgCursor - msgbuf;
  Status = egSaveFile(SelfRootDir, L"EFI\\misc\\legacy_boot.log", (UINT8*)msgbuf, LogSize);
  if (EFI_ERROR(Status)) {
    Print(L"can't save legacy-boot.log\n");
    Status = egSaveFile(NULL, L"EFI\\misc\\legacy_boot.log", (UINT8*)msgbuf, LogSize);
  }
    
  NewMask = 0x0;
  Status = gLegacy8259->SetMask(gLegacy8259, &NewMask, NULL, NULL, NULL);
	//Status = mCpu->EnableInterrupt(mCpu);
  //Now I want to start from VideoTest
  ptr = (UINT8*)0x7F00;
  CopyMem(ptr, &VideoTest[0], 30);
  
  
	CopyMem(pMBR, &tMBR, 16);
	pMBR->StartLBA = LbaOffset;
	pMBR->Size = LbaSize;
  Print(L"Ready to start from LBA=%x\n", LbaOffset); //log is closed
//  Status = gLegacy8259->SetMask(gLegacy8259, &OldMask, NULL, NULL, NULL);
//  return EFI_NOT_FOUND;
  
	Regs.X.DX = BiosDriveNum;
	Regs.X.SI = 0x07BE;
	//Regs.X.ES = EFI_SEGMENT((UINT32) pBootSector);
	//Regs.X.BX = EFI_OFFSET ((UINT32) pBootSector);
	LegacyBiosFarCall86(0, 0x7F00, &Regs); //0x7c00
	//LegacyBiosFarCall86(
	//					EFI_SEGMENT((UINT32)(UINTN) pBootSector),
	//					EFI_OFFSET ((UINT32)(UINTN) pBootSector),
//						&Regs
//						);
	
	// Success - Should never get here 
  //else
  Status = gLegacy8259->SetMask(gLegacy8259, &OldMask, NULL, NULL, NULL);

	return EFI_SUCCESS;	
	
}	
