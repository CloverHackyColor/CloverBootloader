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


EFI_EVENT                        mVirtualAddressChangeEvent = NULL;

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
OnExitBootServices (
                    IN      EFI_EVENT  Event,
                    IN      VOID       *Context
                    )
{
  //
  BootArgs1*				bootArgs1;
	BootArgs2*				bootArgs2;
	UINT8*						ptr=(UINT8*)(UINTN)0x100000;
	DTEntry						efiPlatform;
	CHAR8*						dtRoot;
	UINTN						archMode = sizeof(UINTN) * 8;
	UINTN						Version = 0;
  
  while(TRUE)
	{
		bootArgs2 = (BootArgs2*)ptr;
		bootArgs1 = (BootArgs1*)ptr;
    
		/* search bootargs for 10.7 */
		if(((bootArgs2->Revision == 0) || (bootArgs2->Revision == 0)) && bootArgs2->Version==2)
		{
			if (((UINTN)bootArgs2->efiMode == 32) || ((UINTN)bootArgs2->efiMode == 64)){
				dtRoot = (CHAR8*)bootArgs2->deviceTreeP;
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
				dtRoot = (CHAR8*)bootArgs1->deviceTreeP;
				bootArgs1->efiMode = archMode;
				Version = 1;
				break;
			}
		}
    
		ptr+=0x1000;
		if((UINT32)ptr > 0x3000000)
		{
			Print(L"bootArgs not found!\n");
			gBS->Stall(5000000);
			return;
		}
	}
  
  if(Version==2) {
		CorrectMemoryMap(bootArgs2->MemoryMap,
                     bootArgs2->MemoryMapDescriptorSize,
                     &bootArgs2->MemoryMapSize);
		bootArgs2->efiSystemTable = (UINT32)(UINTN)gST;
		
	}else if(Version==1) {
		CorrectMemoryMap(bootArgs1->MemoryMap,
                     bootArgs1->MemoryMapDescriptorSize,
                     &bootArgs1->MemoryMapSize);
		bootArgs1->efiSystemTable = (UINT32)(UINTN)gST;
	}
  
  DisableUsbLegacySupport();
  
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

EFI_STATUS
EFIAPI
EventsInitialize (
  IN EFI_HANDLE                             ImageHandle,
  IN EFI_SYSTEM_TABLE                       *SystemTable
  )
{
//  EFI_EVENT   OnReadyToBootEvent = NULL;
  EFI_EVENT   ExitBootServiceEvent = NULL;
  EFI_EVENT   mVirtualAddressChangeEvent = NULL;

  //
  // Register the event to reclaim variable for OS usage.
  //
/*  EfiCreateEventReadyToBootEx (
    TPL_NOTIFY, 
    OnReadyToBoot, 
    NULL, 
    &OnReadyToBootEvent
    );  */           

  gBS->CreateEventEx (
         EVT_NOTIFY_SIGNAL,
         TPL_NOTIFY,
         OnExitBootServices,
         NULL,
         &gEfiEventExitBootServicesGuid,
         &ExitBootServiceEvent
         ); 

  //
  // Register the event to convert the pointer for runtime.
  //
  gBS->CreateEventEx (
         EVT_NOTIFY_SIGNAL,
         TPL_NOTIFY,
         VirtualAddressChangeEvent,
         NULL,
         &gEfiEventVirtualAddressChangeGuid,
         &mVirtualAddressChangeEvent
         );
  
  return EFI_SUCCESS;
}

