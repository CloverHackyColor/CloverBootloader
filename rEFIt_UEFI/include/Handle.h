/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Handle.h

Abstract:

  Infomation about the handle function.
 
Revision History

--*/
#ifndef _HANDLE_H
#define _HANDLE_H

#include "libeg.h"

#define EFI_HANDLE_TYPE_UNKNOWN                     0x000
#define EFI_HANDLE_TYPE_IMAGE_HANDLE                0x001
#define EFI_HANDLE_TYPE_DRIVER_BINDING_HANDLE       0x002
#define EFI_HANDLE_TYPE_DEVICE_DRIVER               0x004
#define EFI_HANDLE_TYPE_BUS_DRIVER                  0x008
#define EFI_HANDLE_TYPE_DRIVER_CONFIGURATION_HANDLE 0x010
#define EFI_HANDLE_TYPE_DRIVER_DIAGNOSTICS_HANDLE   0x020
#define EFI_HANDLE_TYPE_COMPONENT_NAME_HANDLE       0x040
#define EFI_HANDLE_TYPE_DEVICE_HANDLE               0x080
#define EFI_HANDLE_TYPE_PARENT_HANDLE               0x100
#define EFI_HANDLE_TYPE_CONTROLLER_HANDLE           0x200
#define EFI_HANDLE_TYPE_CHILD_HANDLE                0x400

EFI_FILE_SYSTEM_INFO              *
EfiLibFileSystemInfo (
  IN EFI_FILE_HANDLE                FHand
  );

/*EFI_FILE_SYSTEM_VOLUME_LABEL_INFO *
LibFileSystemVolumeLabelInfo (
  IN EFI_FILE_HANDLE                FHand
  );

EFI_STATUS
LibScanHandleDatabase (
  EFI_HANDLE  DriverBindingHandle, OPTIONAL
  UINT32      *DriverBindingHandleIndex, OPTIONAL
  EFI_HANDLE  ControllerHandle, OPTIONAL
  UINT32      *ControllerHandleIndex, OPTIONAL
  UINTN       *HandleCount,
  EFI_HANDLE  **HandleBuffer,
  UINT32      **HandleType
  );

EFI_STATUS
LibGetManagingDriverBindingHandles (
  EFI_HANDLE  ControllerHandle,
  UINTN       *DriverBindingHandleCount,
  EFI_HANDLE  **DriverBindingHandleBuffer
  );

EFI_STATUS
LibGetParentControllerHandles (
  EFI_HANDLE  ControllerHandle,
  UINTN       *ParentControllerHandleCount,
  EFI_HANDLE  **ParentControllerHandleBuffer
  );
*/
EFI_STATUS
LibGetManagedChildControllerHandles (
  EFI_HANDLE  DriverBindingHandle,
  EFI_HANDLE  ControllerHandle,
  UINTN       *ChildControllerHandleCount,
  EFI_HANDLE  **ChildControllerHandleBuffer
  );

EFI_STATUS
LibGetManagedControllerHandles (
  EFI_HANDLE  DriverBindingHandle,
  UINTN       *ControllerHandleCount,
  EFI_HANDLE  **ControllerHandleBuffer
  );

EFI_STATUS
LibGetChildControllerHandles (
  EFI_HANDLE  ControllerHandle,
  UINTN       *HandleCount,
  EFI_HANDLE  **HandleBuffer
  );

EFI_STATUS
LibInstallProtocolInterfaces (
  IN OUT EFI_HANDLE                   *Handle,
  ...
  );

VOID
LibUninstallProtocolInterfaces (
  IN EFI_HANDLE                       Handle,
  ...
  );

EFI_STATUS
LibReinstallProtocolInterfaces (
  IN OUT EFI_HANDLE                   *Handle,
  ...
  );

EFI_STATUS
LibLocateHandleByDiskSignature (
  IN UINT8                          MBRType,
  IN UINT8                          SignatureType,
  IN VOID                           *Signature,
  IN OUT UINTN                      *NoHandles,
  OUT EFI_HANDLE                    **Buffer
  );

EFI_STATUS
LibLocateHandle (
  IN EFI_LOCATE_SEARCH_TYPE         SearchType,
  IN EFI_GUID                       * Protocol OPTIONAL,
  IN VOID                           *SearchKey OPTIONAL,
  IN OUT UINTN                      *NoHandles,
  OUT EFI_HANDLE                    **Buffer
  );

EFI_STATUS
LibLocateProtocol (
  IN  EFI_GUID    *ProtocolGuid,
  OUT VOID        **Interface
  );

EFI_HANDLE
ShellHandleFromIndex (
  IN UINTN        Value
  );

UINTN
ShellHandleNoFromIndex (
  IN UINTN        Value
  );

UINTN
ShellHandleToIndex (
  IN  EFI_HANDLE  Handle
  );

UINTN
ShellHandleNoFromStr (
  IN CHAR16       *Str
  );

UINTN
ShellGetHandleNum (
  VOID
  );

#endif