/** @file
  BDS Lib functions which relate with connect the device

Copyright (c) 2004 - 2008, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Platform.h"
//#include "InternalBdsLib.h"


/**
  This function will connect all the system driver to controller
  first, and then special connect the default console, this make
  sure all the system controller available and the platform default
  console connected.

**/
/*
VOID
EFIAPI
BdsLibConnectAll (
  VOID
  )
{
  //
  // Connect the platform console first
  //
  BdsLibConnectAllDefaultConsoles ();

  //
  // Generic way to connect all the drivers
  //
  BdsLibConnectAllDriversToAllControllers ();

  //
  // Here we have the assumption that we have already had
  // platform default console
  //
  BdsLibConnectAllDefaultConsoles ();
}
*/

/**
  This function will connect all the system drivers to all controllers
  first, and then connect all the console devices the system current
  have. After this we should get all the device work and console available
  if the system have console device.

**/
/*
VOID
BdsLibGenericConnectAll (
  VOID
  )
{
  //
  // Most generic way to connect all the drivers
  //
  BdsLibConnectAllDriversToAllControllers ();
  BdsLibConnectAllConsoles ();
}
*/

/**
  This function will create all handles associate with every device
  path node. If the handle associate with one device path node can not
  be created success, then still give one chance to do the dispatch,
  which load the missing drivers if possible.

  @param  DevicePathToConnect   The device path which will be connected, it can be
                                a multi-instance device path

  @retval EFI_SUCCESS           All handles associate with every device path  node
                                have been created
  @retval EFI_OUT_OF_RESOURCES  There is no resource to create new handles
  @retval EFI_NOT_FOUND         Create the handle associate with one device  path
                                node failed

**/
EFI_STATUS
EFIAPI
BdsLibConnectDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePathToConnect
  )
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *CopyOfDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *Instance;
  EFI_DEVICE_PATH_PROTOCOL  *RemainingDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *Next;
  EFI_HANDLE                Handle;
  EFI_HANDLE                PreviousHandle;
  UINTN                     Size;

  if (DevicePathToConnect == NULL) {
    return EFI_SUCCESS;
  }

  DevicePath        = DuplicateDevicePath (DevicePathToConnect);
  if (DevicePath == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  CopyOfDevicePath  = DevicePath;
  
  do {
    //
    // The outer loop handles multi instance device paths.
    // Only console variables contain multiple instance device paths.
    //
    // After this call DevicePath points to the next Instance
    //
    Instance  = GetNextDevicePathInstance (&DevicePath, &Size);
    if (Instance == NULL) {
      FreePool (CopyOfDevicePath);
      return EFI_OUT_OF_RESOURCES;
    }
    
    Next      = Instance;
    while (!IsDevicePathEndType (Next)) {
      Next = NextDevicePathNode (Next);
    }

    SetDevicePathEndNode (Next);

    //
    // Start the real work of connect with RemainingDevicePath
    //
    PreviousHandle = NULL;
    do {
      //
      // Find the handle that best matches the Device Path. If it is only a
      // partial match the remaining part of the device path is returned in
      // RemainingDevicePath.
      //
      RemainingDevicePath = Instance;
      Status              = gBS->LocateDevicePath (&gEfiDevicePathProtocolGuid, &RemainingDevicePath, &Handle);

      if (!EFI_ERROR (Status)) {
        if (Handle == PreviousHandle) {
          //
          // If no forward progress is made try invoking the Dispatcher.
          // A new FV may have been added to the system an new drivers
          // may now be found.
          // Status == EFI_SUCCESS means a driver was dispatched
          // Status == EFI_NOT_FOUND means no new drivers were dispatched
          //
          Status = gDS->Dispatch ();
        }

        if (!EFI_ERROR (Status)) {
          PreviousHandle = Handle;
          //
          // Connect all drivers that apply to Handle and RemainingDevicePath,
          // the Recursive flag is FALSE so only one level will be expanded.
          //
          // Do not check the connect status here, if the connect controller fail,
          // then still give the chance to do dispatch, because partial
          // RemainingDevicepath may be in the new FV
          //
          // 1. If the connect fail, RemainingDevicepath and handle will not
          //    change, so next time will do the dispatch, then dispatch's status
          //    will take effect
          // 2. If the connect success, the RemainingDevicepath and handle will
          //    change, then avoid the dispatch, we have chance to continue the
          //    next connection
          //
          gBS->ConnectController (Handle, NULL, RemainingDevicePath, FALSE);
        }
      }
      //
      // Loop until RemainingDevicePath is an empty device path
      //
    } while (!EFI_ERROR (Status) && !IsDevicePathEnd (RemainingDevicePath));

  } while (DevicePath != NULL);

  if (CopyOfDevicePath != NULL) {
    FreePool (CopyOfDevicePath);
  }
  //
  // All handle with DevicePath exists in the handle database
  //
  return Status;
}


/**
  This function will connect all current system handles recursively. 
  
  gBS->ConnectController() service is invoked for each handle exist in system handler buffer.
  If the handle is bus type handler, all childrens also will be connected recursively
  by gBS->ConnectController().

  @retval EFI_SUCCESS           All handles and it's child handle have been connected
  @retval EFI_STATUS            Error status returned by of gBS->LocateHandleBuffer().

**/
EFI_STATUS
EFIAPI
BdsLibConnectAllEfi (
  VOID
  )
{
  EFI_STATUS  Status;
  UINTN       HandleCount;
  EFI_HANDLE  *HandleBuffer;
  UINTN       Index;

  Status = gBS->LocateHandleBuffer (
                  AllHandles,
                  NULL,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    //Status = 
    gBS->ConnectController (HandleBuffer[Index], NULL, NULL, TRUE);
  }

  if (HandleBuffer != NULL) {
    FreePool (HandleBuffer);
  }

  return EFI_SUCCESS;
}

/**
  This function will disconnect all current system handles. 
  
  gBS->DisconnectController() is invoked for each handle exists in system handle buffer.
  If handle is a bus type handle, all childrens also are disconnected recursively by
  gBS->DisconnectController().

  @retval EFI_SUCCESS           All handles have been disconnected
  @retval EFI_STATUS            Error status returned by of gBS->LocateHandleBuffer().

**/
EFI_STATUS
EFIAPI
BdsLibDisconnectAllEfi (
  VOID
  )
{
  EFI_STATUS  Status;
  UINTN       HandleCount;
  EFI_HANDLE  *HandleBuffer;
  UINTN       Index;

  //
  // Disconnect all
  //
  Status = gBS->LocateHandleBuffer (
                  AllHandles,
                  NULL,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    //Status = 
    gBS->DisconnectController (HandleBuffer[Index], NULL, NULL);
  }

  if (HandleBuffer != NULL) {
    FreePool (HandleBuffer);
  }

  return EFI_SUCCESS;
}

EFI_STATUS ScanDeviceHandles(EFI_HANDLE ControllerHandle,
                             UINTN *HandleCount,
                             EFI_HANDLE **HandleBuffer,
                             UINT32 **HandleType)
{
  EFI_STATUS                          Status;
  UINTN                               HandleIndex;
  EFI_GUID                            **ProtocolGuidArray;
  UINTN                               ArrayCount;
  UINTN                               ProtocolIndex;
  EFI_OPEN_PROTOCOL_INFORMATION_ENTRY *OpenInfo;
  UINTN                               OpenInfoCount;
  UINTN                               OpenInfoIndex;
  UINTN                               ChildIndex;
  
  *HandleCount  = 0;
  *HandleBuffer = NULL;
  *HandleType   = NULL;
  
  //
  // Retrieve the list of all handles from the handle database
  //
  Status = gBS->LocateHandleBuffer (AllHandles, NULL, NULL, HandleCount, HandleBuffer);
  if (EFI_ERROR (Status)) goto Error;
  
  *HandleType = (__typeof_am__(*HandleType))AllocatePool (*HandleCount * sizeof (**HandleType));
  if (*HandleType == NULL) goto Error;
    
  for (HandleIndex = 0; HandleIndex < *HandleCount; HandleIndex++) {
    (*HandleType)[HandleIndex] = EFI_HANDLE_TYPE_UNKNOWN;
    //
    // Retrieve the list of all the protocols on each handle
    //
    Status = gBS->ProtocolsPerHandle (
                  (*HandleBuffer)[HandleIndex],
                  &ProtocolGuidArray,
                  &ArrayCount
                  );
    if (!EFI_ERROR (Status)) {      
      for (ProtocolIndex = 0; ProtocolIndex < ArrayCount; ProtocolIndex++) {
        
        if (CompareGuid (ProtocolGuidArray[ProtocolIndex], &gEfiLoadedImageProtocolGuid)) {
          (*HandleType)[HandleIndex] |= EFI_HANDLE_TYPE_IMAGE_HANDLE;
        }
        
        if (CompareGuid (ProtocolGuidArray[ProtocolIndex], &gEfiDriverBindingProtocolGuid)) {
          (*HandleType)[HandleIndex] |= EFI_HANDLE_TYPE_DRIVER_BINDING_HANDLE;
        }
        
        if (CompareGuid (ProtocolGuidArray[ProtocolIndex], &gEfiDriverConfigurationProtocolGuid)) {
          (*HandleType)[HandleIndex] |= EFI_HANDLE_TYPE_DRIVER_CONFIGURATION_HANDLE;
        }
        
        if (CompareGuid (ProtocolGuidArray[ProtocolIndex], &gEfiDriverDiagnosticsProtocolGuid)) {
          (*HandleType)[HandleIndex] |= EFI_HANDLE_TYPE_DRIVER_DIAGNOSTICS_HANDLE;
        }
        
        if (CompareGuid (ProtocolGuidArray[ProtocolIndex], &gEfiComponentName2ProtocolGuid)) {
          (*HandleType)[HandleIndex] |= EFI_HANDLE_TYPE_COMPONENT_NAME_HANDLE;
        }
        
        if (CompareGuid (ProtocolGuidArray[ProtocolIndex], &gEfiComponentNameProtocolGuid) ) {
          (*HandleType)[HandleIndex] |= EFI_HANDLE_TYPE_COMPONENT_NAME_HANDLE;
        }
        
        if (CompareGuid (ProtocolGuidArray[ProtocolIndex], &gEfiDevicePathProtocolGuid)) {
          (*HandleType)[HandleIndex] |= EFI_HANDLE_TYPE_DEVICE_HANDLE;
        }
        
        //
        // Retrieve the list of agents that have opened each protocol
        //
        Status = gBS->OpenProtocolInformation (
                                               (*HandleBuffer)[HandleIndex],
                                               ProtocolGuidArray[ProtocolIndex],
                                               &OpenInfo,
                                               &OpenInfoCount
                                               );
        if (!EFI_ERROR (Status)) {
          
          for (OpenInfoIndex = 0; OpenInfoIndex < OpenInfoCount; OpenInfoIndex++) {
            
            if (OpenInfo[OpenInfoIndex].ControllerHandle == ControllerHandle) {
              if ((OpenInfo[OpenInfoIndex].Attributes & EFI_OPEN_PROTOCOL_BY_DRIVER) == EFI_OPEN_PROTOCOL_BY_DRIVER) {
                for (ChildIndex = 0; ChildIndex < *HandleCount; ChildIndex++) {
                  if ((*HandleBuffer)[ChildIndex] == OpenInfo[OpenInfoIndex].AgentHandle) {
                    (*HandleType)[ChildIndex] |= EFI_HANDLE_TYPE_DEVICE_DRIVER;
                  }
                }
              }
              
              if ((OpenInfo[OpenInfoIndex].Attributes & EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER) == EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER) {
                (*HandleType)[HandleIndex] |= EFI_HANDLE_TYPE_PARENT_HANDLE;
                for (ChildIndex = 0; ChildIndex < *HandleCount; ChildIndex++) 
                {
                  if ((*HandleBuffer)[ChildIndex] == OpenInfo[OpenInfoIndex].AgentHandle) {
                    (*HandleType)[ChildIndex] |= EFI_HANDLE_TYPE_BUS_DRIVER;
                  }
                }
              }
            }
          }
          
          FreePool (OpenInfo);
        }
      }
      
      FreePool (ProtocolGuidArray);
    }
  }
  
  return EFI_SUCCESS;
  
Error:
  if (*HandleType != NULL) {
    FreePool (*HandleType);
  }
  
  if (*HandleBuffer != NULL) {
    FreePool (*HandleBuffer);
  }
  
  *HandleCount  = 0;
  *HandleBuffer = NULL;
  *HandleType   = NULL;
  
  return Status;
}



EFI_STATUS BdsLibConnectMostlyAllEfi()
{
	EFI_STATUS				Status;
	UINTN             AllHandleCount = 0;
	EFI_HANDLE				*AllHandleBuffer = NULL;
	UINTN             Index;
	UINTN             HandleCount = 0;
	EFI_HANDLE				*HandleBuffer = NULL;
	UINT32            *HandleType = NULL;
	UINTN             HandleIndex;
	BOOLEAN           Parent;
	BOOLEAN           Device;
	EFI_PCI_IO_PROTOCOL*	PciIo = NULL;
	PCI_TYPE00				Pci;
  
  
	Status = gBS->LocateHandleBuffer (AllHandles, NULL, NULL, &AllHandleCount, &AllHandleBuffer);
	if (EFI_ERROR (Status)) 
		return Status;
  
	for (Index = 0; Index < AllHandleCount; Index++) {
		Status = ScanDeviceHandles(AllHandleBuffer[Index], &HandleCount, &HandleBuffer, &HandleType);
    
		if (EFI_ERROR (Status))
			goto Done;
    
		Device = TRUE;
		
		if (HandleType[Index] & EFI_HANDLE_TYPE_DRIVER_BINDING_HANDLE)
			Device = FALSE;
		if (HandleType[Index] & EFI_HANDLE_TYPE_IMAGE_HANDLE)
			Device = FALSE;
    
		if (Device) {					
			Parent = FALSE;
			for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
				if (HandleType[HandleIndex] & EFI_HANDLE_TYPE_PARENT_HANDLE)
					Parent = TRUE;
			}
      
			if (!Parent) {
				if (HandleType[Index] & EFI_HANDLE_TYPE_DEVICE_HANDLE) {
					Status = gBS->HandleProtocol (AllHandleBuffer[Index], &gEfiPciIoProtocolGuid, (VOID**)&PciIo);
					if (!EFI_ERROR (Status)) {
						Status = PciIo->Pci.Read (PciIo,EfiPciIoWidthUint32, 0, sizeof (Pci) / sizeof (UINT32), &Pci);
						if (!EFI_ERROR (Status)) {
							if(IS_PCI_VGA(&Pci)==TRUE) {
								gBS->DisconnectController(AllHandleBuffer[Index], NULL, NULL);
							}
						}
					}
					Status = gBS->ConnectController(AllHandleBuffer[Index], NULL, NULL, TRUE);
				}
			}
		}
    
		FreePool (HandleBuffer);
		FreePool (HandleType);
	}
  
Done:
	FreePool (AllHandleBuffer);
	return Status;
}



/**
  Connects all drivers to all controllers.
  This function make sure all the current system driver will manage
  the corresponding controllers if have. And at the same time, make
  sure all the system controllers have driver to manage it if have.

**/
VOID
EFIAPI
BdsLibConnectAllDriversToAllControllers (
  VOID
  )
{
  EFI_STATUS  Status;

  do {
    //
    // Connect All EFI 1.10 drivers following EFI 1.10 algorithm
    //
    //BdsLibConnectAllEfi ();
    BdsLibConnectMostlyAllEfi ();
    //
    // Check to see if it's possible to dispatch an more DXE drivers.
    // The BdsLibConnectAllEfi () may have made new DXE drivers show up.
    // If anything is Dispatched Status == EFI_SUCCESS and we will try
    // the connect again.
    //
    Status = gDS->Dispatch ();

  } while (!EFI_ERROR (Status));

}


/**
  Connect the specific Usb device which match the short form device path,
  and whose bus is determined by Host Controller (Uhci or Ehci).

  @param  HostControllerPI      Uhci (0x00) or Ehci (0x20) or Both uhci and ehci
                                (0xFF)
  @param  RemainingDevicePath   a short-form device path that starts with the first
                                element  being a USB WWID or a USB Class device
                                path

  @return EFI_INVALID_PARAMETER  RemainingDevicePath is NULL pointer.
                                 RemainingDevicePath is not a USB device path.
                                 Invalid HostControllerPI type.
  @return EFI_SUCCESS            Success to connect USB device
  @return EFI_NOT_FOUND          Fail to find handle for USB controller to connect.

**/
EFI_STATUS
EFIAPI
BdsLibConnectUsbDevByShortFormDP(
  IN UINT8                      HostControllerPI,
  IN EFI_DEVICE_PATH_PROTOCOL   *RemainingDevicePath
  )
{
  EFI_STATUS                            Status;
  EFI_HANDLE                            *HandleArray;
  UINTN                                 HandleArrayCount;
  UINTN                                 Index;
  EFI_PCI_IO_PROTOCOL                   *PciIo;
  UINT8                                 Class[3];
  BOOLEAN                               AtLeastOneConnected;

  //
  // Check the passed in parameters
  //
  if (RemainingDevicePath == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if ((DevicePathType (RemainingDevicePath) != MESSAGING_DEVICE_PATH) ||
      ((DevicePathSubType (RemainingDevicePath) != MSG_USB_CLASS_DP)
      && (DevicePathSubType (RemainingDevicePath) != MSG_USB_WWID_DP)
      )) {
    return EFI_INVALID_PARAMETER;
  }

  if (HostControllerPI != 0xFF &&
      HostControllerPI != 0x00 &&
      HostControllerPI != 0x10 &&
      HostControllerPI != 0x20 &&
      HostControllerPI != 0x30) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Find the usb host controller firstly, then connect with the remaining device path
  //
  AtLeastOneConnected = FALSE;
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &HandleArrayCount,
                  &HandleArray
                  );
  if (!EFI_ERROR (Status)) {
    for (Index = 0; Index < HandleArrayCount; Index++) {
      Status = gBS->HandleProtocol (
                      HandleArray[Index],
                      &gEfiPciIoProtocolGuid,
                      (VOID **)&PciIo
                      );
      if (!EFI_ERROR (Status)) {
        //
        // Check whether the Pci device is the wanted usb host controller
        //
        Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint8, 0x09, 3, &Class);
        if (!EFI_ERROR (Status)) {
          if ((PCI_CLASS_SERIAL == Class[2]) &&
              (PCI_CLASS_SERIAL_USB == Class[1])) {
            if (HostControllerPI == Class[0] || HostControllerPI == 0xFF) {
              Status = gBS->ConnectController (
                              HandleArray[Index],
                              NULL,
                              RemainingDevicePath,
                              FALSE
                              );
              if (!EFI_ERROR(Status)) {
                AtLeastOneConnected = TRUE;
              }
            }
          }
        }
      }
    }

    if (HandleArray != NULL) {
      FreePool (HandleArray);
    }

    if (AtLeastOneConnected) {
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}
