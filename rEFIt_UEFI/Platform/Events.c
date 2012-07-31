/*
Copyright (c) 2010 - 2011, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED. 

*/

#include "Platform.h"
#include "device_tree.h"
#include "kernel_patcher.h"
#include "mkext.h"

#define PATCH_DEBUG 1

#if PATCH_DEBUG
#define DBG(x...)	Print(x);
#else
#define DBG(x...)
#endif

EFI_EVENT   mVirtualAddressChangeEvent = NULL;
EFI_EVENT   OnReadyToBootEvent = NULL;
EFI_EVENT   ExitBootServiceEvent = NULL;
EFI_EVENT   mSimpleFileSystemChangeEvent = NULL;



VOID CorrectMemoryMap(IN UINT32 memMap, 
                      IN UINT32 memDescriptorSize,
                      IN OUT UINT32 *memMapSize)
{
	EfiMemoryRange*		memDescriptor;
	UINT64              Bytes;
	UINT32				Index;
	//
	//step 1. Check for last empty descriptors
	//
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
  //
  //step 3. convert BootServiceData to conventional
  //
  memDescriptor = (EfiMemoryRange *)(UINTN)memMap;
  for (Index = 0; Index < *memMapSize / memDescriptorSize; Index ++) {
    switch (memDescriptor->Type) {
      case EfiLoaderData:
      case EfiBootServicesCode:
      case EfiBootServicesData:  
        memDescriptor->Type = EfiConventionalMemory;
        memDescriptor->Attribute = 0;
        break;
      default:
        break;
    }
  }  
  
	if(gSettings.Debug==TRUE) {
		
//		WaitForKeyPress("press any key to dump MemoryMap");
		memDescriptor = (EfiMemoryRange *)(UINTN)memMap;
		for (Index = 0; Index < *memMapSize / memDescriptorSize; Index ++) {
			Bytes = LShiftU64 (memDescriptor->NumberOfPages, 12);
			Print(L"%lX-%lX  %lX %lX %X\n",
                 memDescriptor->PhysicalStart, 
                 memDescriptor->PhysicalStart + Bytes - 1,
                 memDescriptor->NumberOfPages, 
                 memDescriptor->Attribute,
                 (UINTN)memDescriptor->Type);
			memDescriptor = (EfiMemoryRange *)((UINTN)memDescriptor + memDescriptorSize);
//			if (Index % 20 == 19) {
	//			WaitForKeyPress("press any key to next");
  //			}
		}
	}
	
}

VOID
EFIAPI
OnExitBootServices(IN EFI_EVENT Event, IN VOID *Context)
{
		
	//
	// Patch kernel and kexts if needed
	//
	KernelAndKextsPatcherStart();
	
	if (gSettings.MemoryFix) {
    BootArgs1*				bootArgs1;
    BootArgs2*				bootArgs2;
    UINT8*						ptr=(UINT8*)(UINTN)0x100000;
    //	DTEntry						efiPlatform;
    CHAR8*						dtRoot;
    UINTN						archMode = sizeof(UINTN) * 8;
    UINTN						Version = 0;
    
    while(TRUE)
    {
      bootArgs2 = (BootArgs2*)ptr;
      bootArgs1 = (BootArgs1*)ptr;
      
      /* search bootargs for 10.7 */
      if(((bootArgs2->Revision == 0) || (bootArgs2->Revision == 1)) && bootArgs2->Version==2)
      {
        if (((UINTN)bootArgs2->efiMode == 32) || ((UINTN)bootArgs2->efiMode == 64)){
          dtRoot = (CHAR8*)(UINTN)bootArgs2->deviceTreeP;
          bootArgs2->efiMode = archMode; //correct to EFI arch
          Version = 2;
          break;
        } 
        
        /* search bootargs for 10.4 - 10.6.x */
      } else if(((bootArgs1->Revision==6) ||
                 (bootArgs1->Revision==5) || 
                 (bootArgs1->Revision==4)) &&
                (bootArgs1->Version ==1)){
        
        if (((UINTN)bootArgs1->efiMode == 32) ||
            ((UINTN)bootArgs1->efiMode == 64)){
          dtRoot = (CHAR8*)(UINTN)bootArgs1->deviceTreeP;
          bootArgs1->efiMode = archMode;
          Version = 1;
          break;
        }
      }
      
      ptr+=0x1000;
      if((UINT32)(UINTN)ptr > 0x3000000)
      {
        Print(L"bootArgs not found!\n");
        //			gBS->Stall(5000000);
        //			return;
        break;
      }
    }
    if(Version==2) {
      CorrectMemoryMap(bootArgs2->MemoryMap,
                       bootArgs2->MemoryMapDescriptorSize,
                       &bootArgs2->MemoryMapSize);
 //     bootArgs2->efiSystemTable = (UINT32)(UINTN)gST;
      
    }else if(Version==1) {
      CorrectMemoryMap(bootArgs1->MemoryMap,
                       bootArgs1->MemoryMapDescriptorSize,
                       &bootArgs1->MemoryMapSize);
//      bootArgs1->efiSystemTable = (UINT32)(UINTN)gST;
    }
	}
	if (gFirmwareClover) {
		// Note: blocks on Aptio
		DisableUsbLegacySupport();
	}
}

VOID
EFIAPI
OnReadyToBoot (
               IN      EFI_EVENT   Event,
               IN      VOID        *Context
               )
{
//
}

VOID
EFIAPI
VirtualAddressChangeEvent (
                           IN EFI_EVENT  Event,
                           IN VOID       *Context
                           )
{
//  EfiConvertPointer (0x0, (VOID **) &mProperty);
//  EfiConvertPointer (0x0, (VOID **) &mSmmCommunication);
}

VOID
EFIAPI
OnSimpleFileSystem (
                    IN      EFI_EVENT  Event,
                    IN      VOID       *Context
                    )
{
	EFI_TPL		OldTpl;
	
	OldTpl = gBS->RaiseTPL (TPL_NOTIFY);
	gEvent = 1;
	//  ReinitRefitLib();
	//ScanVolumes();
	//enter GUI
	// DrawMenuText(L"OnSimpleFileSystem", 0, 0, UGAHeight-40, 1);
	MsgLog("OnSimpleFileSystem occured\n");
	
	gBS->RestoreTPL (OldTpl);
	
}  

EFI_STATUS
GuiEventsInitialize ()
{
	EFI_STATUS				Status;
	EFI_EVENT				Event;
	VOID*				  	RegSimpleFileSystem;
	
	
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

EFI_STATUS
EFIAPI
EventsInitialize ()
{
	EFI_STATUS			Status;
	//VOID*           Registration = NULL;
	
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
							   NULL,
							   &ExitBootServiceEvent);
  /*
	if(!EFI_ERROR(Status))
	{
		Status = gBS->RegisterProtocolNotify (
                 &gEfiStatusCodeRuntimeProtocolGuid,
                 ExitBootServiceEvent,
                 &Registration);
	}
   */
  
  
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
	
	return EFI_SUCCESS;
}

EFI_STATUS EjectVolume(IN REFIT_VOLUME *Volume)
{
	EFI_SCSI_IO_PROTOCOL            *ScsiIo = NULL;
	EFI_SCSI_IO_SCSI_REQUEST_PACKET CommandPacket;
	UINT64                          Lun = 0;
	UINT8                           *Target;
	UINT8                           TargetArray[EFI_SCSI_TARGET_MAX_BYTES];
	EFI_STATUS                      Status = EFI_UNSUPPORTED;
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
	ZeroMem (Cdb, sizeof (EFI_SCSI_OP_LENGTH_SIX));
	
	Status = gBS->HandleProtocol(Volume->DeviceHandle, &gEfiScsiIoProtocolGuid, (VOID **) &ScsiIo);
	if (ScsiIo) {
		Target = &TargetArray[0];
		ScsiIo->GetDeviceLocation (ScsiIo, &Target, &Lun);
		
		
		Cdb[0]  = EFI_SCSI_OP_START_STOP_UNIT;
		Cdb[1]  = (UINT8) (LShiftU64 (Lun, 5) & EFI_SCSI_LOGICAL_UNIT_NUMBER_MASK);
		Cdb[1] |= 0x01;
		Cdb[4]  = ATA_CMD_SUBOP_EJECT_DISC;  
		CommandPacket.Timeout = EFI_TIMER_PERIOD_SECONDS (3);
		CommandPacket.Cdb = Cdb;
		CommandPacket.CdbLength = (UINT8) sizeof (Cdb);
    
		Status = ScsiIo->ExecuteScsiCommand (ScsiIo, &CommandPacket, NULL);
	} else {
		Status = gBS->HandleProtocol(Volume->DeviceHandle, &gEfiBlockIoProtocolGuid, (VOID **) &BlkIo);
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
        //	if (EFI_ERROR (Status)) {
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
			Cdb[4] = 0x02; //eject command. NO DESCRIPTION IN HEADERS
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
