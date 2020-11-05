/*
Copyright (c) 2010 - 2011, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED. 

*/

#include <Platform.h>
#include "../entry_scan/entry_scan.h"
//#include "device_tree.h"
#include "kernel_patcher.h"
#include "usbfix.h"
#include "../Platform/cpu.h"
#include "../Platform/Settings.h"

#ifndef DEBUG_ALL
#define PATCH_DEBUG 0
#define MEM_DEB 0
#else
#define PATCH_DEBUG DEBUG_ALL
#define MEM_DEB DEBUG_ALL
#endif

#if PATCH_DEBUG
#define DBG(...)	printf(__VA_ARGS__);
#else
#define DBG(...)	
#endif

////
//// Max bytes needed to represent ID of a SCSI device
////
//#define EFI_SCSI_TARGET_MAX_BYTES (0x10)
////
//// bit5..7 are for Logical unit number
//// 11100000b (0xe0)
////
//#define EFI_SCSI_LOGICAL_UNIT_NUMBER_MASK 0xe0


//
// Scsi Command Length
//
#define EFI_SCSI_OP_LENGTH_SIX      0x6
//#define EFI_SCSI_OP_LENGTH_TEN      0xa
//#define EFI_SCSI_OP_LENGTH_SIXTEEN  0x10



EFI_EVENT   mVirtualAddressChangeEvent = NULL;
EFI_EVENT   OnReadyToBootEvent = NULL;
EFI_EVENT   ExitBootServiceEvent = NULL;
EFI_EVENT   mSimpleFileSystemChangeEvent = NULL;
EFI_HANDLE  mHandle = NULL;

extern EFI_RUNTIME_SERVICES gOrgRS;

/*
void WaitForCR()
{
  EFI_STATUS    Status;
  EFI_INPUT_KEY key;
  UINTN         ind;

  while (TRUE) {
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &key);
    if (Status == EFI_NOT_READY) {
      gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &ind);
      continue;
    }
    if (key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
      break;
    }
  }
}
*/
#if 0
//this procedure was developed for 10.5. Seems no more needed
void CorrectMemoryMap(IN UINT32 memMap, 
                      IN UINT32 memDescriptorSize,
                      IN OUT UINT32 *memMapSize)
{
	EfiMemoryRange*		memDescriptor;
	UINT64              Bytes;
	UINT32				Index;
  CHAR16        tmp[80];
  EFI_INPUT_KEY Key;
//  UINTN         ind;
	//
	//step 1. Check for last empty descriptors
	//
 // PauseForKey(L"Check for last empty descriptors");
//  gST->ConOut->OutputString (gST->ConOut, L"Check for last empty descriptors\n\r");
//  gBS->Stall(2000000);
	memDescriptor = (EfiMemoryRange *)(UINTN)(memMap + *memMapSize - memDescriptorSize);
	while ((memDescriptor->NumberOfPages == 0) || (memDescriptor->NumberOfPages > (1<<25)))
	{
		memDescriptor = (EfiMemoryRange *)((UINTN)memDescriptor - memDescriptorSize);
		*memMapSize -= memDescriptorSize;
	}
	//
	//step 2. Add last desc about MEM4GB
	//
  /*	if (gTotalMemory > MEM4GB) {
   //next descriptor
   memDescriptor = (EfiMemoryRange *)((UINTN)memDescriptor + memDescriptorSize);
   memDescriptor->Type = EfiConventionalMemory;
   memDescriptor->PhysicalStart = MEM4GB;
   memDescriptor->VirtualStart = MEM4GB;
   memDescriptor->NumberOfPages = LShiftU64(gTotalMemory - MEM4GB, EFI_PAGE_SHIFT);
   memDescriptor->Attribute = 0;
   *memMapSize += memDescriptorSize;
   }
   */	
  memDescriptor = (EfiMemoryRange *)(UINTN)memMap;
  for (Index = 0; Index < *memMapSize / memDescriptorSize; Index ++) {
    //
    //step 3. convert BootServiceData to conventional
    // not needed as performed by mach_kernel
/*    switch (memDescriptor->Type) {
      case EfiLoaderData:
      case EfiBootServicesCode:
      case EfiBootServicesData:  
        memDescriptor->Type = EfiConventionalMemory;
        memDescriptor->Attribute = 0;
    //    DBG(L"Range BS %hhX corrected to conventional\n", memDescriptor->PhysicalStart);
        if(MEM_DEB) {
          snwprintf(tmp, 160, "Range BS %hhX corrected to conventional\n\r", memDescriptor->PhysicalStart);
          gST->ConOut->OutputString (gST->ConOut, tmp);
         // gBS->Stall(2000000);
          WaitForCR();
        }
        break;
      default:
        break;
    }
 */
    //
    //step 4. free reserved memory if cachable
    if ((memDescriptor->Type == EfiReservedMemoryType) &&
        (memDescriptor->Attribute == EFI_MEMORY_WB)) {
      memDescriptor->Type = EfiConventionalMemory;
      memDescriptor->Attribute = 0xF;
//      DBG(L"Range WB %hhX corrected to conventional\n", memDescriptor->PhysicalStart);
      if(MEM_DEB) {
        snwprintf(tmp, 160, "Range WB %hhX corrected to conventional\n\r", memDescriptor->PhysicalStart);
        gST->ConOut->OutputString (gST->ConOut, tmp);
        //gBS->Stall(2000000);
 //       WaitForCR();
      }
    }
    //
    //step 5. free reserved memory if base >= 20000 & <= 60000
    //xxx
    if ((memDescriptor->Type == EfiReservedMemoryType) &&
        (memDescriptor->PhysicalStart >= 0x20000000) &&
        (memDescriptor->PhysicalStart <= 0x60000000)) {
      memDescriptor->Type = EfiConventionalMemory;
      memDescriptor->Attribute = 0xF;
    }
    //
  }
    //step 6. Reserve for 9E
  memDescriptor = (EfiMemoryRange *)((UINTN)memDescriptor + memDescriptorSize);
  memDescriptor->Type = EfiReservedMemoryType;
  memDescriptor->PhysicalStart = 0x9e000;
  memDescriptor->VirtualStart = 0x9e000;
  memDescriptor->NumberOfPages = 2;
  memDescriptor->Attribute = 0;
  *memMapSize += memDescriptorSize;


	if(MEM_DEB) {
    gST->ConOut->OutputString (gST->ConOut, L"press any key to dump MemoryMap\r\n");
//    gBS->Stall(2000000);
    WaitForSingleEvent (gST->ConIn->WaitForKey, 0);
    
    gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
		
//		PauseForKey(L"press any key to dump MemoryMap");
		memDescriptor = (EfiMemoryRange *)(UINTN)memMap;
		for (Index = 0; Index < *memMapSize / memDescriptorSize; Index ++) {
			Bytes = LShiftU64 (memDescriptor->NumberOfPages, 12);
                 snwprintf(tmp, 160, "%lX-%lX pages %lX type %lX attr %hhX \r\n\r\t",
                 memDescriptor->PhysicalStart, 
                 memDescriptor->PhysicalStart + Bytes - 1,
                 memDescriptor->NumberOfPages,
                 (UINTN)memDescriptor->Type,
                 memDescriptor->Attribute
                 );
      gST->ConOut->OutputString (gST->ConOut, tmp);
//      gBS->Stall(2000000);
      
			memDescriptor = (EfiMemoryRange *)((UINTN)memDescriptor + memDescriptorSize);
			if (Index % 20 == 19) {
				gST->ConOut->OutputString (gST->ConOut, L"press any key\r\n");
        WaitForSingleEvent (gST->ConIn->WaitForKey, 0);
//        gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
/*        if (ReadAllKeyStrokes()) {  // remove buffered key strokes
          gBS->Stall(5000000);     // 5 seconds delay
          ReadAllKeyStrokes();    // empty the buffer again
        }
        
        gBS->WaitForEvent(1, &gST->ConIn->WaitForKey, &ind);
        ReadAllKeyStrokes();        // empty the buffer to protect the menu
        WaitForCR();
 */
  		}
		}
	}
	
}
#endif

void
EFIAPI
OnExitBootServices(IN EFI_EVENT Event, IN void *Context)
{
  /*
  if (gCPUStructure.Vendor == CPU_VENDOR_INTEL &&
      (gCPUStructure.Family == 0x06 && gCPUStructure.Model >= CPU_MODEL_SANDY_BRIDGE)
       ) {
    UINT64 msr = 0;

    msr = AsmReadMsr64(MSR_PKG_CST_CONFIG_CONTROL); //0xE2
    //  printf("MSR 0xE2 on Exit BS %08x\n", msr);

  } */
/*
//  EFI_STATUS Status;
  {
    //    UINT32                    machineSignature    = 0;
    EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE     *FadtPointer = NULL;
    EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE  *Facs = NULL;

    //    DBG("---dump hibernations data---\n");
    FadtPointer = GetFadt();
    if (FadtPointer != NULL) {
      Facs = (EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE*)(UINTN)(FadtPointer->FirmwareCtrl);

      printf("  Firmware wake address=%08lx\n", Facs->FirmwareWakingVector);
      printf("  Firmware wake 64 addr=%16llx\n",  Facs->XFirmwareWakingVector);
      printf("  Hardware signature   =%08lx\n", Facs->HardwareSignature);
      printf("  GlobalLock           =%08lx\n", Facs->GlobalLock);
      printf("  Flags                =%08lx\n", Facs->Flags);
      printf(" HS at offset 0x%08x\n", OFFSET_OF(EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE, HardwareSignature));
 //     machineSignature = Facs->HardwareSignature;
    }
  }
*/  

  gST->ConOut->OutputString (gST->ConOut, L"+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	//
	// Patch kernel and kexts if needed
	//
//  Jief : OpenCore is doing the kernel patching for all versions.
//  LOADER_ENTRY *Entry = ((REFIT_ABSTRACT_MENU_ENTRY*)Context)->getLOADER_ENTRY();
//	if ( Entry && Entry->OSVersion.startWith("10") ) {
//    Entry->KernelAndKextsPatcherStart();
//  }
	
#if 0  //it will be as a sample of possible patches in future
//    gBS->Stall(2000000);
	//PauseForKey(L"press any key to MemoryFix");
	if (gSettings.MemoryFix) {
    BootArgs1*				bootArgs1v;
    BootArgs2*				bootArgs2v;
    UINT8*						ptr=(UINT8*)(UINTN)0x100000;
    //	DTEntry						efiPlatform;
//    CHAR8*						dtreeRoot;
    UINTN						archMode = sizeof(UINTN) * 8;
    UINTN						Version = 0;
    
    while(TRUE)
    {
      bootArgs2v = (BootArgs2*)ptr;
      bootArgs1v = (BootArgs1*)ptr;
      
      /* search bootargs for 10.7 */
      if(((bootArgs2v->Revision == 0) || (bootArgs2v->Revision == 1)) && bootArgs2v->Version==2)
      {
        if (((UINTN)bootArgs2v->efiMode == 32) || ((UINTN)bootArgs2v->efiMode == 64)){
//          dtreeRoot = (CHAR8*)(UINTN)bootArgs2v->deviceTreeP;
          bootArgs2v->efiMode = (UINT8)archMode; //correct to EFI arch
          Version = 2;
     //     DBG(L"found bootarg v2");
          gST->ConOut->OutputString (gST->ConOut, L"found bootarg v2");
          break;
        } 
        
        /* search bootargs for 10.4 - 10.6.x */
      } else if(((bootArgs1v->Revision==6) ||
                 (bootArgs1v->Revision==5) || 
                 (bootArgs1v->Revision==4)) &&
                (bootArgs1v->Version ==1)){
        
        if (((UINTN)bootArgs1v->efiMode == 32) ||
            ((UINTN)bootArgs1v->efiMode == 64)){
//          dtreeRoot = (CHAR8*)(UINTN)bootArgs1v->deviceTreeP;
          bootArgs1v->efiMode = (UINT8)archMode;
          Version = 1;
   //       DBG(L"found bootarg v1");
          gST->ConOut->OutputString (gST->ConOut, L"found bootarg v1");
          break;
        }
      }
      
      ptr+=0x1000;
      if((UINT32)(UINTN)ptr > 0x3000000)
      {
 //       DBG("bootArgs not found!\n");
        gST->ConOut->OutputString (gST->ConOut, L"bootArgs not found!");
        gBS->Stall(5000000);
        //			return;
        break;
      }
    }
    if(Version==2) {
      CorrectMemoryMap(bootArgs2v->MemoryMap,
                       bootArgs2v->MemoryMapDescriptorSize,
                       &bootArgs2v->MemoryMapSize);
 //     bootArgs2v->efiSystemTable = (UINT32)(UINTN)gST;
      
    }else if(Version==1) {
      CorrectMemoryMap(bootArgs1v->MemoryMap,
                       bootArgs1v->MemoryMapDescriptorSize,
                       &bootArgs1v->MemoryMapSize);
//      bootArgs1v->efiSystemTable = (UINT32)(UINTN)gST;
    }
	}
#endif
  
	if (gSettings.USBFixOwnership) {
    FixOwnership();
	}
}

void
EFIAPI
OnReadyToBoot (
               IN      EFI_EVENT   Event,
               IN      void        *Context
               )
{
/*
  if ((gCPUStructure.Vendor == CPU_VENDOR_INTEL &&
       (gCPUStructure.Family == 0x06 && gCPUStructure.Model >= CPU_MODEL_SANDY_BRIDGE)
       )) {
    UINT64 msr = 0;

    msr = AsmReadMsr64(MSR_PKG_CST_CONFIG_CONTROL); //0xE2

  }
//  printf("MSR 0xE2 on ReadyToBoot %08x\n", msr);
*/
  gST->ConOut->OutputString (gST->ConOut, L"-- ReadyToBoot --\n");
}

void
EFIAPI
VirtualAddressChangeEvent (
                           IN EFI_EVENT  Event,
                           IN void       *Context
                           )
{
//  EfiConvertPointer (0x0, (void **) &mProperty);
//  EfiConvertPointer (0x0, (void **) &mSmmCommunication);
}

void
EFIAPI
OnSimpleFileSystem (
                    IN      EFI_EVENT  Event,
                    IN      void       *Context
                    )
{
	EFI_TPL		OldTpl;
	
	OldTpl = gBS->RaiseTPL (TPL_NOTIFY);
	gEvent = 1;
	//  ReinitRefitLib();
	//ScanVolumes();
	//enter GUI
	// DrawMenuText(L"OnSimpleFileSystem", 0, 0, UGAHeight-40, 1);
	// MsgLog("OnSimpleFileSystem occured\n");
	
	gBS->RestoreTPL (OldTpl);
	
}  

EFI_STATUS
GuiEventsInitialize ()
{
	EFI_STATUS				Status;
	EFI_EVENT				Event;
	void*				  	RegSimpleFileSystem = NULL;
	
	gEvent = 0;
	Status = gBS->CreateEvent (
							   EVT_NOTIFY_SIGNAL,
							   TPL_NOTIFY,
							   OnSimpleFileSystem,
							   NULL,
							   &Event);
	if(!EFI_ERROR(Status))
	{
		Status = gBS->RegisterProtocolNotify (
											  &gEfiSimpleFileSystemProtocolGuid,
											  Event,
											  &RegSimpleFileSystem);
	}
	
	
	return Status;
}  

//EFI_STATUS
//WaitForSingleEvent (
//          IN EFI_EVENT        Event,
//          IN UINT64           Timeout OPTIONAL
//          )
//{
//  EFI_STATUS          Status;
//  UINTN            Index;
//
//  EFI_EVENT          WaitList[3];
//  EFI_EVENT          TimerEvent;
//
//  if (Timeout != 0)
//  {
//    //
//    // Create a timer event
//    //
//    Status = gBS->CreateEvent(EVT_TIMER, 0, NULL, NULL, &TimerEvent);
//    if (!EFI_ERROR(Status))
//    {
//      //
//      // Set the timer event
//      //
//      gBS->SetTimer(TimerEvent, TimerRelative, Timeout);
//
//      //
//      // Wait for the original event or the timer
//      //
//      WaitList[0] = Event;
//      WaitList[1] = TimerEvent;
//
//      Status = gBS->WaitForEvent(2, WaitList, &Index);
//      gBS->CloseEvent (TimerEvent);
//      if (!EFI_ERROR(Status) && Index == 1)
//      {
//        Status = EFI_TIMEOUT;
//      }
//    }
//  }
//  else
//  {
//    WaitList[0] = Event;
//    Status = gBS->WaitForEvent (1, WaitList, &Index);
//  }
//  return Status;
//}
//

//set Timeout in ms
EFI_STATUS
WaitFor2EventWithTsc (
                     IN  EFI_EVENT        Event1,
                     IN  EFI_EVENT        Event2,
                     IN  UINT64           Timeout OPTIONAL
                    )
{
  EFI_STATUS        Status;
  UINTN             Index;
  EFI_EVENT         WaitList[2];
  UINT64            t0, t1;
  //all arguments are UINT64, we can use native divide and multiply
  UINT64            Delta = DivU64x64Remainder(MultU64x64(Timeout, gCPUStructure.TSCFrequency), 1000, NULL);

  if (Timeout != 0)
  {
    t0 = AsmReadTsc();
    do {
      Status = gBS->CheckEvent(Event1);
      if (!EFI_ERROR(Status)) {
        break;
      }
      if (Event2) {
        Status = gBS->CheckEvent(Event2);
        if (!EFI_ERROR(Status)) {
          break;
        }
      }
      // Let's try to relax processor a bit
      CpuPause();
      Status = EFI_TIMEOUT;
      t1 = AsmReadTsc();
    } while ((t1 - t0) < Delta);
  }
  else
  {
    WaitList[0] = Event1;
    WaitList[1] = Event2;
    Status = gBS->WaitForEvent (2, WaitList, &Index);
  }
  return Status;
}


EFI_STATUS
EventsInitialize (IN LOADER_ENTRY *Entry)
{
	EFI_STATUS			Status;
	void*           Registration = NULL;
	
	//
	// Register the event to reclaim variable for OS usage.
	//
	//EfiCreateEventReadyToBoot(&OnReadyToBootEvent);
	/*  EfiCreateEventReadyToBootEx (
	 TPL_NOTIFY, 
	 OnReadyToBoot, 
	 NULL, 
	 &OnReadyToBootEvent
	 );  */           
	
	//
	// Register notify for exit boot services
	//
	Status = gBS->CreateEvent (EVT_SIGNAL_EXIT_BOOT_SERVICES,
							   TPL_CALLBACK,
							   OnExitBootServices, 
							   Entry,
							   &ExitBootServiceEvent);
  
	if(!EFI_ERROR(Status))
	{
		/*Status = */gBS->RegisterProtocolNotify (
                 &gEfiStatusCodeRuntimeProtocolGuid,
                 ExitBootServiceEvent,
                 &Registration);
	}
   
  
  
	//
	// Register the event to convert the pointer for runtime.
	//
    /*
	 gBS->CreateEventEx (
	 EVT_NOTIFY_SIGNAL,
	 TPL_NOTIFY,
	 VirtualAddressChangeEvent,
	 NULL,
	 &gEfiEventVirtualAddressChangeGuid,
	 &mVirtualAddressChangeEvent
	 );
     */
	// and what if EFI_ERROR?
	return Status;
}

EFI_STATUS EjectVolume(IN REFIT_VOLUME *Volume)
{
	EFI_SCSI_IO_PROTOCOL            *ScsiIo = NULL;
	EFI_SCSI_IO_SCSI_REQUEST_PACKET CommandPacket;
//	UINT64                          Lun = 0;
//	UINT8                           *Target;
//	UINT8                           TargetArray[EFI_SCSI_TARGET_MAX_BYTES];
	EFI_STATUS                      Status; // = EFI_UNSUPPORTED;
	UINT8                           Cdb[EFI_SCSI_OP_LENGTH_SIX];
	USB_MASS_DEVICE                 *UsbMass = NULL;
	EFI_BLOCK_IO_PROTOCOL           *BlkIo	= NULL;
	EFI_BLOCK_IO_MEDIA              *Media;
	UINT32                          Timeout;
	UINT32                          CmdResult;
  
	//
	// Initialize SCSI REQUEST_PACKET and 6-byte Cdb
	//
	ZeroMem (&CommandPacket, sizeof (EFI_SCSI_IO_SCSI_REQUEST_PACKET));
	ZeroMem (Cdb, EFI_SCSI_OP_LENGTH_SIX);
	
	Status = gBS->HandleProtocol(Volume->DeviceHandle, &gEfiScsiIoProtocolGuid, (void **) &ScsiIo);
	if (ScsiIo) {
//		Target = &TargetArray[0];
//		ScsiIo->GetDeviceLocation (ScsiIo, &Target, &Lun);
		
		
		Cdb[0]  = EFI_SCSI_OP_START_STOP_UNIT;
//		Cdb[1]  = (UINT8) (LShiftU64 (Lun, 5) & EFI_SCSI_LOGICAL_UNIT_NUMBER_MASK);
//		Cdb[1] |= 0x01;
    Cdb[1]  = 0x01;
		Cdb[4]  = ATA_CMD_SUBOP_EJECT_DISC;  
		CommandPacket.Timeout = EFI_TIMER_PERIOD_SECONDS (3);
		CommandPacket.Cdb = Cdb;
		CommandPacket.CdbLength = (UINT8) sizeof (Cdb);
    
		Status = ScsiIo->ExecuteScsiCommand (ScsiIo, &CommandPacket, NULL);
	} else {
		Status = gBS->HandleProtocol(Volume->DeviceHandle, &gEfiBlockIoProtocolGuid, (void **) &BlkIo);
		if (BlkIo) {
			UsbMass = USB_MASS_DEVICE_FROM_BLOCK_IO (BlkIo);
			if (!UsbMass) {
				MsgLog("no UsbMass\n");
				Status = EFI_NOT_FOUND;
				goto ON_EXIT;
			}
			Media   = &UsbMass->BlockIoMedia;
			if (!Media) {
				MsgLog("no BlockIoMedia\n");
				Status = EFI_NO_MEDIA;
				goto ON_EXIT;
			}
			
			//
			// If it is a removable media, such as CD-Rom or Usb-Floppy,
			// need to detect the media before each read/write. While some of
			// Usb-Flash is marked as removable media.
			//
      //TODO - DetectMedia will appear automatically. Do nothing?
			if (!Media->RemovableMedia) {
				//Status = UsbBootDetectMedia (UsbMass);
        //	if (EFI_ERROR(Status)) {
				Status = EFI_UNSUPPORTED;
        goto ON_EXIT;
        //	}
			} 
			
			if (!(Media->MediaPresent)) {
				Status = EFI_NO_MEDIA;
				goto ON_EXIT;
			}
      //TODO - remember previous state		
      /*		if (MediaId != Media->MediaId) {
       Status = EFI_MEDIA_CHANGED;
       goto ON_EXIT;
       }*/
			
			Timeout = USB_BOOT_GENERAL_CMD_TIMEOUT;
			Cdb[0]  = EFI_SCSI_OP_START_STOP_UNIT;
	//		Cdb[1]  = (UINT8) (USB_BOOT_LUN(UsbMass->Lun) & EFI_SCSI_LOGICAL_UNIT_NUMBER_MASK);
	//		Cdb[1] |= 0x01;
      Cdb[1] = 0x01;
			Cdb[4] = ATA_CMD_SUBOP_EJECT_DISC; //eject command. 
		//	Status = EFI_UNSUPPORTED;
			Status    = UsbMass->Transport->ExecCommand (
                                                   UsbMass->Context,
                                                   &Cdb,
                                                   sizeof(Cdb),
                                                   EfiUsbDataOut,
                                                   NULL, 0,
                                                   UsbMass->Lun,
                                                   Timeout,
                                                   &CmdResult
                                                   );
      
      //ON_EXIT:			
      //			gBS->RestoreTPL (OldTpl);
		}
	}
ON_EXIT:
  return Status;
}
