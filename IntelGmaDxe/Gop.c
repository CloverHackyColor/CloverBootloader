/*++

  Copyright (c)  2006 - 2010 Intel Corporation. All rights reserved
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:

  Gop.c
    
Abstract:

  GOP Controller Driver.
  This driver is a sample implementation of the Graphics Output Protocol for the
  Intel GMCH family of PCI video controllers.  This driver is only
  usable in the EFI pre-boot environment.  This sample is intended to show
  how the Graphics Output Protocol is able to function.

Revision History:

--*/

#include "Gop.h"
#include "CpuSupport.h"

EFI_DRIVER_BINDING_PROTOCOL gDriverBinding = {
  ControllerDriverSupported,
  ControllerDriverStart,
  ControllerDriverStop,
  0x10,
  NULL,
  NULL
};

INTEL_PRIVATE_DATA      mGopPrivateDataTemplate = {
  INTEL_PRIVATE_DATA_SIGNATURE,               // Signature
  (EFI_HANDLE) NULL,                          // Handle
  (EFI_PCI_IO_PROTOCOL *) NULL,               // PciIo protocol
  (EFI_DEVICE_PATH_PROTOCOL *) NULL,          // DevicePath protocol
  {
    GraphicsOutputQueryMode,
    GraphicsOutputSetMode,
    GraphicsOutputBitBlt
  },                                          // Graphics Output Protocol
  {
    0, NULL                                   //Edid Discovered Protocol
  },
  {
    0, NULL                                   //Edid Active Protocol
  },
  {
    SIZE_TO_PAGE(GTT_ALLOC_SIZE),             // Number of pages of GTT buffer
    0                                         // GTT base address
  },
  {
    SIZE_TO_PAGE(FB_SIZE),                    // Number of pages of allocated memory
    0                                         // Frame buffer
  },
  0,                                          // Frame buffer offset
};

EFI_STATUS
ControllerChildHandleUninstall (
  EFI_DRIVER_BINDING_PROTOCOL    *This,
  EFI_HANDLE                     Controller,
  EFI_HANDLE                     Handle
  );

UINT32
GetUmaSize (
  EFI_PCI_IO_PROTOCOL                 *PciIo
  );

UINT32
GetUmaBase (
  EFI_PCI_IO_PROTOCOL                 *PciIo
  );

//
// GOP Driver Entry point
//
//EFI_DRIVER_ENTRY_POINT (GraphicsOutputDriverEntryPoint)

EFI_STATUS
EFIAPI
GraphicsOutputDriverEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/*++

Routine Description:

  Entrypoint of the GOP driver

Arguments:

  ImageHandle - Driver image handle
  SystemTable - Pointer to system table 

Returns:

  EFI_SUCCESS - GOP driver finished initialize

--*/
{
	EFI_STATUS              Status;
	
	Status = EfiLibInstallDriverBindingComponentName2 (
													   ImageHandle,
													   SystemTable,
													   &gDriverBinding,
													   ImageHandle,
													   &gComponentName,
													   &gComponentName2
													   );
	ASSERT_EFI_ERROR (Status);
	
	return Status;	
	
/*  return EfiLibInstallAllDriverProtocols2 (
           ImageHandle,
           SystemTable,
           &gDriverBinding,
           ImageHandle,
           &gComponentName,
           NULL,
           NULL
           );
 */
}

EFI_STATUS
EFIAPI
ControllerDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++
  
Routine Description:
  
  The function test whether or not GOP driver support the Controller.
    
Arguments:

  This                - Driver Binding Protocol instance 
  Controller          - Pointer to controller Handle
  RemainingDevicePath - Remaining Device Path
    
Returns:
  EFI_SUCCESS         - Successfully support this controller 
  
--*/
{
  EFI_STATUS          Status;
  EFI_PCI_IO_PROTOCOL *PciIo;
  PCI_TYPE00          Pci;

  //
  // Open the PCI I/O Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Read the PCI Configuration Header from the PCI Device
  //
  Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, 0, sizeof (Pci) / sizeof (UINT32), &Pci);
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  Status = EFI_UNSUPPORTED;

  //
  // See if this is a graphics controller
  //
  if (Pci.Hdr.VendorId == INTEL_VENDOR_ID && (
			(Pci.Hdr.DeviceId == INTEL_GMA_DEVICE_ID) ||
			(Pci.Hdr.DeviceId == INTEL_GOP_DEVICE_ID) ||
			(Pci.Hdr.DeviceId == INTEL_X3100_DEVICE_ID)	
			)) { //Slice
    Status = EFI_SUCCESS;
  }

Done:
  //
  // Close the PCI I/O Protocol
  //
  gBS->CloseProtocol (
        Controller,
        &gEfiPciIoProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  return Status;
}

EFI_STATUS
EFIAPI
ControllerDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++
  
Routine Description:

  GOP driver start entrypoint.
    
Arguments:

  This                - Driver Binding Protocol instance 
  Controller          - Pointer to controller Handle
  RemainingDevicePath - Remaining Device Path
    
Returns:

  EFI_SUCCESS         - Driver success to start
  
--*/
{
  EFI_STATUS                          Status;
  INTEL_PRIVATE_DATA                  *Private;
  EFI_PCI_IO_PROTOCOL                 *PciIo;
  ACPI_ADR_DEVICE_PATH                AcpiDevicePath;
  EFI_DEVICE_PATH_PROTOCOL            *ParentDevicePath;
  EFI_PHYSICAL_ADDRESS                AllocateMemoryBase;
  UINTN                               PreAllocatedMemory;

  //
  // Prepare parent device path
  //
  Status = gBS->HandleProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &ParentDevicePath
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Open PCI I/O Protocol and save pointer to open protocol
  // in private data area.
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Allocate Private context data for Graphics Output inteface.
  //
  Private = AllocateCopyPool (
              sizeof (INTEL_PRIVATE_DATA),
              &mGopPrivateDataTemplate
              );
  if (Private == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ErrorExit;
  }
  Private->PciIo = PciIo;

  //
  // Check UMA size
  //
  if (GetUmaSize(PciIo) < PREALLOCATED_SIZE) {
    //
    // if UMA == 1, or 0, then we allocate BootServices buffer
    // and not expose it
    //
    Private->PixelFormat = PixelBltOnly;
    //
    // Allocate the frame buffer 
    //
    AllocateMemoryBase = 0xFFFFFFFF;
    Status = gBS->AllocatePages (
                    AllocateMaxAddress,
                    EfiBootServicesData,
                    Private->AllocatedMemory.Pages,
                    &AllocateMemoryBase
                    );
    if (EFI_ERROR (Status)) {
      goto ErrorExit;
    }
    Private->AllocatedMemory.BaseAddress = (UINTN)AllocateMemoryBase;

    ZeroMem ((VOID *) Private->AllocatedMemory.BaseAddress, MEM_ALLOC_SIZE);

    FlushDataCache (Private->AllocatedMemory.BaseAddress, MEM_ALLOC_SIZE);
    //
    // Allocate GTT buffer
    //
    AllocateMemoryBase = 0xFFFFFFFF;
    Status = gBS->AllocatePages (
                    AllocateAnyPages,
                    EfiBootServicesData,
                    Private->GTTBaseAddress.Pages,
                    &AllocateMemoryBase
                    );
    if (EFI_ERROR (Status)) {
      goto ErrorExit;
    }
    Private->GTTBaseAddress.BaseAddress = (UINTN)AllocateMemoryBase;

    ZeroMem ((VOID *) Private->GTTBaseAddress.BaseAddress, GTT_ALLOC_SIZE);

    FlushDataCache (Private->GTTBaseAddress.BaseAddress, GTT_ALLOC_SIZE);

    //
    // Create ExitBootServiceEvent to clear Gfx Controller
    //
    Status = gBS->CreateEvent (
                      EVT_SIGNAL_EXIT_BOOT_SERVICES,
					  TPL_CALLBACK,
                      ClearGfxController,
                      Private,
                      &Private->ExitBootServiceEvent
                      );
  } else {
    //
    // if UMA == 8, then we use UMA and expose it
    //
    Private->PixelFormat = PixelBlueGreenRedReserved8BitPerColor;

    PreAllocatedMemory = (UINTN)GetUmaBase (PciIo);

    //
    // Use pre-allocated memory
    //
    Private->AllocatedMemory.BaseAddress = PreAllocatedMemory;
    Private->GTTBaseAddress.BaseAddress  = PreAllocatedMemory + MEM_ALLOC_SIZE;
    ZeroMem ((VOID *) PreAllocatedMemory, PREALLOCATED_SIZE);
    FlushDataCache (PreAllocatedMemory, PREALLOCATED_SIZE);

  }

  //
  // Start the Graphics Output software stack.
  // The Graphics Output constructor requires that the MMIO BAR be enabled.
  //
  Status = GraphicsOutputConstructor (Private);
  if (EFI_ERROR (Status)) {
    goto ErrorExit;
  }

  if (RemainingDevicePath == NULL) {
    //
    // If ACPI_ADR is not specified, use default
    //
    ZeroMem (&AcpiDevicePath, sizeof (ACPI_ADR_DEVICE_PATH));
    AcpiDevicePath.Header.Type = ACPI_DEVICE_PATH;
    AcpiDevicePath.Header.SubType = ACPI_ADR_DP;
    AcpiDevicePath.ADR = ACPI_DISPLAY_ADR (1, 0, 0, 1, 0, ACPI_ADR_DISPLAY_TYPE_VGA, 0, 0);
    SetDevicePathNodeLength (&AcpiDevicePath.Header, sizeof (ACPI_ADR_DEVICE_PATH));

    Private->DevicePath = AppendDevicePathNode (ParentDevicePath, (EFI_DEVICE_PATH_PROTOCOL *) &AcpiDevicePath);
  } else {
    Private->DevicePath = AppendDevicePathNode (ParentDevicePath, RemainingDevicePath);
  }

  //
  // Creat child handle and install Graphics Output Protocol,EDID Discovered/Active Protocol
  //
  Private->Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Private->Handle,
                  &gEfiDevicePathProtocolGuid,
                  Private->DevicePath,
                  &gEfiGraphicsOutputProtocolGuid,
                  &Private->GraphicsOutput,
                  &gEfiEdidDiscoveredProtocolGuid,
                  &Private->EdidDiscovered,
                  &gEfiEdidActiveProtocolGuid,
                  &Private->EdidActive,
                  NULL
                  );
  if (!EFI_ERROR (Status)) {
    //
    // Open the Parent Handle for the child
    //
    Status = gBS->OpenProtocol (
                    Controller,
                    &gEfiPciIoProtocolGuid,
                    (VOID **) &Private->PciIo,
                    This->DriverBindingHandle,
                    Private->Handle,
                    EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                    );
    if (EFI_ERROR (Status)) {
      goto ErrorExit;
    }
  }

ErrorExit:

  if (EFI_ERROR (Status)) {
    //
    // Free all allocated memory if there is any
    //
    if (Private != NULL) {
      if (Private->PixelFormat == PixelBltOnly) {
        if (Private->AllocatedMemory.BaseAddress) {
          gBS->FreePages (
                 (EFI_PHYSICAL_ADDRESS)Private->AllocatedMemory.BaseAddress,
                 Private->AllocatedMemory.Pages
                 );
        }

        if (Private->GTTBaseAddress.BaseAddress) {
          gBS->FreePages (
                 (EFI_PHYSICAL_ADDRESS)Private->GTTBaseAddress.BaseAddress,
                 Private->GTTBaseAddress.Pages
                 );
        }
      }
      Private->GTTBaseAddress.BaseAddress  = 0;
      Private->AllocatedMemory.BaseAddress = 0;

      if (Private->ExitBootServiceEvent != NULL) {
        gBS->CloseEvent (Private->ExitBootServiceEvent);
        Private->ExitBootServiceEvent = NULL;
      }

      gBS->FreePool (Private);
    }
    //
    // Close the PCI I/O Protocol
    //
    gBS->CloseProtocol (
           Controller,
           &gEfiPciIoProtocolGuid,
           This->DriverBindingHandle,
           Controller
           );
  }

  return Status;
}

EFI_STATUS
EFIAPI
ControllerDriverStop (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN UINTN                        NumberOfChildren,
  IN EFI_HANDLE                   *ChildHandleBuffer
  )
/*++

Routine Description:

  GOP driver stop entrypoint

Arguments:

  This              - Driver Binding Protocol instance 
  Controller        - Controller Handle
  NumberOfChildren  - Child number
  ChildHandleBuffer - A buffer that contain childern handles

Returns:

  EFI_SUCCESS       - GOP driver successfully Stop 

--*/
{
  EFI_STATUS  Status;
  BOOLEAN     AllChildrenStopped;
  UINTN       Index;

  if (NumberOfChildren == 0) {
    //
    // Close PCI I/O protocol on the controller handle
    //
    gBS->CloseProtocol (
          Controller,
          &gEfiPciIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );

    return EFI_SUCCESS;
  }

  AllChildrenStopped = TRUE;
  for (Index = 0; Index < NumberOfChildren; Index ++) {

    Status = ControllerChildHandleUninstall (This, Controller, ChildHandleBuffer[Index]);
    if (EFI_ERROR (Status)) {
      AllChildrenStopped = FALSE;
    }
  }

  if (!AllChildrenStopped) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}


EFI_STATUS
ControllerChildHandleUninstall (
  EFI_DRIVER_BINDING_PROTOCOL    *This,
  EFI_HANDLE                     Controller,
  EFI_HANDLE                     Handle
  )
/*++

Routine Description:

  Deregister an video child handle and free resources

Arguments:

  This            - Protocol instance pointer.
  Controller      - Video controller handle
  Handle          - Video child handle

Returns:

  EFI_STATUS

--*/
{
  EFI_STATUS                   Status;
  INTEL_PRIVATE_DATA           *Private;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *GraphicsOutput;

  //
  // There is only one child handle
  //
  Status = gBS->OpenProtocol (
                  Handle,
                  &gEfiGraphicsOutputProtocolGuid,
                  (VOID **) &GraphicsOutput,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Get our private context information
  //
  Private = INTEL_PRIVATE_DATA_FROM_GRAPHICS_OUTPUT_THIS (GraphicsOutput);

  //
  // Shutdown the hardware
  //
  GraphicsOutputDestructor (Private);

  if (Private->PixelFormat == PixelBltOnly) {
    //
    // Free our instance data
    //
    if (Private->AllocatedMemory.BaseAddress) {
      gBS->FreePages (
             (EFI_PHYSICAL_ADDRESS)Private->AllocatedMemory.BaseAddress,
             Private->AllocatedMemory.Pages
             );
    }

    if (Private->GTTBaseAddress.BaseAddress) {
      gBS->FreePages (
             (EFI_PHYSICAL_ADDRESS)Private->GTTBaseAddress.BaseAddress,
             Private->GTTBaseAddress.Pages
             );
    }
  }
  Private->GTTBaseAddress.BaseAddress  = 0;
  Private->AllocatedMemory.BaseAddress = 0;

  if (Private->ExitBootServiceEvent != NULL) {
    gBS->CloseEvent (Private->ExitBootServiceEvent);
    Private->ExitBootServiceEvent = NULL;
  }

  //
  // Close PCI I/O protocol that opened by child handle
  //
  gBS->CloseProtocol (
        Controller,
        &gEfiPciIoProtocolGuid,
        This->DriverBindingHandle,
        Handle
        );

  //
  // Remove the Graphics Output interface from the system
  //
  Status = gBS->UninstallMultipleProtocolInterfaces (
                  Private->Handle,
                  &gEfiDevicePathProtocolGuid,
                  Private->DevicePath,
                  &gEfiGraphicsOutputProtocolGuid,
                  &Private->GraphicsOutput,
                  &gEfiEdidDiscoveredProtocolGuid,
                  &Private->EdidDiscovered,
                  &gEfiEdidActiveProtocolGuid,
                  &Private->EdidActive,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    gBS->OpenProtocol (
           Controller,
           &gEfiPciIoProtocolGuid,
           (VOID **) &Private->PciIo,
           This->DriverBindingHandle,
           Handle,
           EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
           );
    return Status;
  }

  //
  // Free our instance data
  //
  gBS->FreePool (Private);

  return EFI_SUCCESS;
}

UINT32
GetUmaSize (
  EFI_PCI_IO_PROTOCOL                 *PciIo
  )
{
  EFI_STATUS Status;
  UINT16     GCC;
  UINT32     UmaSize;
  
  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint16,
                        IGD_GGC_OFFSET,
                        1,
                        &GCC
                        );
  if (EFI_ERROR(Status)) {
    return 0;
  }

  switch (GCC & B_GMS) {
    case V_GMS_8MB:
      UmaSize = 8 * 1024 * 1024;
      break;
    case V_GMS_1MB:
      UmaSize = 1 * 1024 * 1024;
      break;
    default:
      UmaSize = 0;
      break;
  };

  return UmaSize;
}

UINT32
GetUmaBase (
  EFI_PCI_IO_PROTOCOL                 *PciIo
  )
{
  EFI_STATUS Status;
  UINT32     BSM;
  
  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint32,
                        IGD_BSM_OFFSET,
                        1,
                        &BSM
                        );
  if (EFI_ERROR(Status)) {
    return 0;
  }

  return (BSM & 0xFFF00000);
}
