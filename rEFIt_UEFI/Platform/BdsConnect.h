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

#ifndef PLATFORM_BDSCONNECT_H_
#define PLATFORM_BDSCONNECT_H_


#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
//#include "../include/Handle.h"
//#include "../Platform/Settings.h"


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
  );

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
  void
  );

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
  void
  );

EFI_STATUS ScanDeviceHandles(EFI_HANDLE ControllerHandle,
                             UINTN *HandleCount,
                             EFI_HANDLE **HandleBuffer,
                             UINT32 **HandleType);



EFI_STATUS BdsLibConnectMostlyAllEfi();


/**
  Connects all drivers to all controllers.
  This function make sure all the current system driver will manage
  the corresponding controllers if have. And at the same time, make
  sure all the system controllers have driver to manage it if have.

**/
void
EFIAPI
BdsLibConnectAllDriversToAllControllers (
  void
  );


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
  );
