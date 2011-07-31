/* $Id$ */
/** @file
 * IdeController.h
 */

/*
 * Copyright (C) 2009-2010 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 */

/** @file
  Header file for IDE controller driver.

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#ifndef _IDE_CONTROLLER_H
#define _IDE_CONTROLLER_H

#include <Uefi.h>
#include <Protocol/ComponentName.h>
#include <Protocol/DriverBinding.h>
#include <Protocol/PciIo.h>
#include <Protocol/IdeControllerInit.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <IndustryStandard/Pci.h>

//
// Global Variables definitions
//
extern EFI_DRIVER_BINDING_PROTOCOL  gIdeControllerDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL  gIdeControllerComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL gIdeControllerComponentName2;

//
// Supports 2 channel max
//
#define ICH_IDE_MAX_CHANNEL 0x02
//
// Supports 2 devices max
//
#define ICH_IDE_MAX_DEVICES 0x02
#define ICH_IDE_ENUMER_ALL  FALSE

//
// Driver binding functions declaration
//
EFI_STATUS
EFIAPI
IdeControllerSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL       *This,
  IN EFI_HANDLE                        Controller,
  IN EFI_DEVICE_PATH_PROTOCOL          *RemainingDevicePath
  )
/*++

  Routine Description:
  
  Register Driver Binding protocol for this driver.
  
  Arguments:
  
    This                 -- a pointer points to the Binding Protocol instance
    Controller           -- The handle of controller to be tested. 
    *RemainingDevicePath -- A pointer to the device path. Ignored by device
                            driver but used by bus driver

  Returns:

    EFI_SUCCESS          -- Driver loaded.
    other                -- Driver not loaded.
--*/
;

EFI_STATUS
EFIAPI
IdeControllerStart (
  IN EFI_DRIVER_BINDING_PROTOCOL        *This,
  IN EFI_HANDLE                         Controller,
  IN EFI_DEVICE_PATH_PROTOCOL           *RemainingDevicePath
  )
/*++

  Routine Description:
  
    This routine is called right after the .Supported() called and return 
    EFI_SUCCESS. Notes: The supported protocols are checked but the Protocols
    are closed.

  Arguments:
      
    This                 -- a pointer points to the Binding Protocol instance
    Controller           -- The handle of controller to be tested. Parameter
                            passed by the caller
    *RemainingDevicePath -- A pointer to the device path. Should be ignored by
                            device driver
--*/
;

EFI_STATUS
EFIAPI
IdeControllerStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL       *This,
  IN  EFI_HANDLE                        Controller,
  IN  UINTN                             NumberOfChildren,
  IN  EFI_HANDLE                        *ChildHandleBuffer
  )
/*++
  
  Routine Description:
    Stop this driver on Controller Handle. 

  Arguments:
    This              - Protocol instance pointer.
    Controller        - Handle of device to stop driver on 
    NumberOfChildren  - Not used
    ChildHandleBuffer - Not used

  Returns:
    EFI_SUCCESS       - This driver is removed DeviceHandle
    other             - This driver was not removed from this device
  
--*/
;

//
// IDE controller init functions declaration
//
EFI_STATUS
EFIAPI
IdeInitGetChannelInfo (
  IN   EFI_IDE_CONTROLLER_INIT_PROTOCOL *This,
  IN   UINT8                            Channel,
  OUT  BOOLEAN                          *Enabled,
  OUT  UINT8                            *MaxDevices
  )
/*++

Routine Description:

  @param This       the EFI_IDE_CONTROLLER_INIT_PROTOCOL instance.
  @param Phase      phase indicator defined by IDE_CONTROLLER_INIT protocol
  @param Channel    Channel number (0 based, either 0 or 1)

  @return EFI_SUCCESS Success operation.
**/
;

EFI_STATUS
EFIAPI
IdeInitNotifyPhase (
  IN  EFI_IDE_CONTROLLER_INIT_PROTOCOL  *This,
  IN  EFI_IDE_CONTROLLER_ENUM_PHASE     Phase,
  OUT UINT8                             Channel
  )
/**
  This function is called by IdeBus driver to submit EFI_IDENTIFY_DATA data structure
  obtained from IDE deivce. This structure is used to set IDE timing

  @param This           The EFI_IDE_CONTROLLER_INIT_PROTOCOL instance.
  @param Channel        IDE channel number (0 based, either 0 or 1)
  @param Device         IDE device number
  @param IdentifyData   A pointer to EFI_IDENTIFY_DATA data structure

  @return EFI_SUCCESS   Success operation.
**/
;

EFI_STATUS
EFIAPI
IdeInitSubmitData (
  IN  EFI_IDE_CONTROLLER_INIT_PROTOCOL  *This,
  IN  UINT8                             Channel,
  IN  UINT8                             Device,
  IN  EFI_IDENTIFY_DATA                 *IdentifyData
  )
/**
  This function is called by IdeBus driver to disqualify unsupported operation
  mode on specfic IDE device

  @param This       the EFI_IDE_CONTROLLER_INIT_PROTOCOL instance.
  @param Channel    IDE channel number (0 based, either 0 or 1)
  @param Device     IDE device number
  @param BadModes   Operation mode indicator

  @return EFI_SUCCESS Success operation.
**/
;

EFI_STATUS
EFIAPI
IdeInitSubmitFailingModes (
  IN  EFI_IDE_CONTROLLER_INIT_PROTOCOL  *This,
  IN  UINT8                             Channel,
  IN  UINT8                             Device
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This    - TODO: add argument description
  Channel - TODO: add argument description
  Device  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
IdeInitDisqualifyMode (
  IN  EFI_IDE_CONTROLLER_INIT_PROTOCOL  *This,
  IN  UINT8                             Channel,
  IN  UINT8                             Device,
  IN  EFI_ATA_COLLECTIVE_MODE           *BadModes
  );
/**
  This function is called by IdeBus driver to calculate the best operation mode
  supported by specific IDE device

  @param This               the EFI_IDE_CONTROLLER_INIT_PROTOCOL instance.
  @param Channel            IDE channel number (0 based, either 0 or 1)
  @param Device             IDE device number
  @param SupportedModes     Modes collection supported by IDE device

  @retval EFI_OUT_OF_RESOURCES  Fail to allocate pool.
  @retval EFI_INVALID_PARAMETER Invalid channel id and device id.
**/

EFI_STATUS
EFIAPI
IdeInitCalculateMode (
  IN  EFI_IDE_CONTROLLER_INIT_PROTOCOL  *This,
  IN  UINT8                             Channel,
  IN  UINT8                             Device,
  IN  EFI_ATA_COLLECTIVE_MODE           **SupportedModes
  )
/**
  This function is called by IdeBus driver to set appropriate timing on IDE
  controller according supported operation mode.

  @param This       the EFI_IDE_CONTROLLER_INIT_PROTOCOL instance.
  @param Channel    IDE channel number (0 based, either 0 or 1)
  @param Device     IDE device number
  @param Modes      IDE device modes

  @retval EFI_SUCCESS Sucess operation.
**/
;

EFI_STATUS
EFIAPI
IdeInitSetTiming (
  IN  EFI_IDE_CONTROLLER_INIT_PROTOCOL  *This,
  IN  UINT8                             Channel,
  IN  UINT8                             Device,
  IN  EFI_ATA_COLLECTIVE_MODE           *Modes
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This    - TODO: add argument description
  Channel - TODO: add argument description
  Device  - TODO: add argument description
  Modes   - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

//
// Forward reference declaration
//
EFI_STATUS
EFIAPI
IdeControllerComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME_PROTOCOL  *This,
  IN  CHAR8                        *Language,
  OUT CHAR16                       **DriverName
  )
/*++

  Routine Description:
    Retrieves a Unicode string that is the user readable name of the EFI Driver.

  Arguments:
    This       - A pointer to the EFI_COMPONENT_NAME_PROTOCOL instance.
    Language   - A pointer to a three character ISO 639-2 language identifier.
                 This is the language of the driver name that that the caller 
                 is requesting, and it must match one of the languages specified
                 in SupportedLanguages.  The number of languages supported by a 
                 driver is up to the driver writer.
    DriverName - A pointer to the Unicode string to return.  This Unicode string
                 is the name of the driver specified by This in the language 
                 specified by Language.

  Returns:
    EFI_SUCCESS           - The Unicode string for the Driver specified by This
                            and the language specified by Language was returned 
                            in DriverName.
    EFI_INVALID_PARAMETER - Language is NULL.
    EFI_INVALID_PARAMETER - DriverName is NULL.
    EFI_UNSUPPORTED       - The driver specified by This does not support the 
                            language specified by Language.

--*/
;

EFI_STATUS
EFIAPI
IdeControllerComponentNameGetControllerName (
  IN  EFI_COMPONENT_NAME_PROTOCOL                     *This,
  IN  EFI_HANDLE                                      ControllerHandle,
  IN  EFI_HANDLE                                      ChildHandle        OPTIONAL,
  IN  CHAR8                                           *Language,
  OUT CHAR16                                          **ControllerName
  )
/*++

  Routine Description:
    Retrieves a Unicode string that is the user readable name of the controller
    that is being managed by an EFI Driver.

  Arguments:
    This             - A pointer to the EFI_COMPONENT_NAME_PROTOCOL instance.
    ControllerHandle - The handle of a controller that the driver specified by 
                       This is managing.  This handle specifies the controller 
                       whose name is to be returned.
    ChildHandle      - The handle of the child controller to retrieve the name 
                       of.  This is an optional parameter that may be NULL.  It 
                       will be NULL for device drivers.  It will also be NULL 
                       for a bus drivers that wish to retrieve the name of the 
                       bus controller.  It will not be NULL for a bus driver 
                       that wishes to retrieve the name of a child controller.
    Language         - A pointer to a three character ISO 639-2 language 
                       identifier.  This is the language of the controller name 
                       that that the caller is requesting, and it must match one
                       of the languages specified in SupportedLanguages.  The 
                       number of languages supported by a driver is up to the 
                       driver writer.
    ControllerName   - A pointer to the Unicode string to return.  This Unicode
                       string is the name of the controller specified by 
                       ControllerHandle and ChildHandle in the language 
                       specified by Language from the point of view of the 
                       driver specified by This. 

  Returns:
    EFI_SUCCESS           - The Unicode string for the user readable name in the 
                            language specified by Language for the driver 
                            specified by This was returned in DriverName.
    EFI_INVALID_PARAMETER - ControllerHandle is not a valid EFI_HANDLE.
    EFI_INVALID_PARAMETER - ChildHandle is not NULL and it is not a valid 
                            EFI_HANDLE.
    EFI_INVALID_PARAMETER - Language is NULL.
    EFI_INVALID_PARAMETER - ControllerName is NULL.
    EFI_UNSUPPORTED       - The driver specified by This is not currently 
                            managing the controller specified by 
                            ControllerHandle and ChildHandle.
    EFI_UNSUPPORTED       - The driver specified by This does not support the 
                            language specified by Language.

--*/
;

#endif
