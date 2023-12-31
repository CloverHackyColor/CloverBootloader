/** @file
  Driver entry point

  Copyright (c) 2021 - 2023 Pedro Falcato All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "Ext4Dxe.h"

GLOBAL_REMOVE_IF_UNREFERENCED EFI_UNICODE_STRING_TABLE  mExt4DriverNameTable[] = {
  {
    "eng;en",
    L"Ext4 File System Driver"
  },
  {
    NULL,
    NULL
  }
};

GLOBAL_REMOVE_IF_UNREFERENCED EFI_UNICODE_STRING_TABLE  mExt4ControllerNameTable[] = {
  {
    "eng;en",
    L"Ext4 File System"
  },
  {
    NULL,
    NULL
  }
};

// Needed by gExt4ComponentName*

/**
  Retrieves a Unicode string that is the user-readable name of the EFI Driver.

  @param[in]  This       A pointer to the EFI_COMPONENT_NAME_PROTOCOL instance.
  @param[in]  Language   A pointer to a three-character ISO 639-2 language identifier.
                     This is the language of the driver name that that the caller
                     is requesting, and it must match one of the languages specified
                     in SupportedLanguages.  The number of languages supported by a
                     driver is up to the driver writer.
  @param[out]  DriverName A pointer to the Unicode string to return.  This Unicode string
                     is the name of the driver specified by This in the language
                     specified by Language.

  @retval EFI_SUCCESS           The Unicode string for the Driver specified by This
                                and the language specified by Language was returned
                                in DriverName.
  @retval EFI_INVALID_PARAMETER Language is NULL.
  @retval EFI_INVALID_PARAMETER DriverName is NULL.
  @retval EFI_UNSUPPORTED       The driver specified by This does not support the
                                language specified by Language.

**/
EFI_STATUS
EFIAPI
Ext4ComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME_PROTOCOL  *This,
  IN  CHAR8                        *Language,
  OUT CHAR16                       **DriverName
  );

/**
  Retrieves a Unicode string that is the user readable name of the controller
  that is being managed by an EFI Driver.

  @param[in]  This             A pointer to the EFI_COMPONENT_NAME_PROTOCOL instance.
  @param[in]  ControllerHandle The handle of a controller that the driver specified by
                               This is managing.  This handle specifies the controller
                               whose name is to be returned.
  @param[in]  ChildHandle      The handle of the child controller to retrieve the name
                               of.  This is an optional parameter that may be NULL.  It
                               will be NULL for device drivers.  It will also be NULL
                               for a bus drivers that wish to retrieve the name of the
                               bus controller.  It will not be NULL for a bus driver
                               that wishes to retrieve the name of a child controller.
  @param[in]  Language         A pointer to a three character ISO 639-2 language
                               identifier.  This is the language of the controller name
                               that the caller is requesting, and it must match one
                               of the languages specified in SupportedLanguages.  The
                               number of languages supported by a driver is up to the
                               driver writer.
  @param[out]  ControllerName  A pointer to the Unicode string to return.  This Unicode
                               string is the name of the controller specified by
                               ControllerHandle and ChildHandle in the language specified
                               by Language, from the point of view of the driver specified
                               by This.

  @retval EFI_SUCCESS           The Unicode string for the user-readable name in the
                                language specified by Language for the driver
                                specified by This was returned in DriverName.
  @retval EFI_INVALID_PARAMETER ControllerHandle is NULL.
  @retval EFI_INVALID_PARAMETER ChildHandle is not NULL and it is not a valid EFI_HANDLE.
  @retval EFI_INVALID_PARAMETER Language is NULL.
  @retval EFI_INVALID_PARAMETER ControllerName is NULL.
  @retval EFI_UNSUPPORTED       The driver specified by This is not currently managing
                                the controller specified by ControllerHandle and
                                ChildHandle.
  @retval EFI_UNSUPPORTED       The driver specified by This does not support the
                                language specified by Language.

**/
EFI_STATUS
EFIAPI
Ext4ComponentNameGetControllerName (
  IN  EFI_COMPONENT_NAME_PROTOCOL  *This,
  IN  EFI_HANDLE                   ControllerHandle,
  IN  EFI_HANDLE                   ChildHandle  OPTIONAL,
  IN  CHAR8                        *Language,
  OUT CHAR16                       **ControllerName
  );

extern EFI_COMPONENT_NAME_PROTOCOL  gExt4ComponentName;

GLOBAL_REMOVE_IF_UNREFERENCED EFI_COMPONENT_NAME_PROTOCOL  gExt4ComponentName = {
  Ext4ComponentNameGetDriverName,
  Ext4ComponentNameGetControllerName,
  "eng"
};

//
// EFI Component Name 2 Protocol
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_COMPONENT_NAME2_PROTOCOL  gExt4ComponentName2 = {
  (EFI_COMPONENT_NAME2_GET_DRIVER_NAME)Ext4ComponentNameGetDriverName,
  (EFI_COMPONENT_NAME2_GET_CONTROLLER_NAME)Ext4ComponentNameGetControllerName,
  "en"
};

// Needed by gExt4BindingProtocol

/**
  Tests to see if this driver supports a given controller. If a child device is provided,
  it further tests to see if this driver supports creating a handle for the specified child device.

  This function checks to see if the driver specified by This supports the device specified by
  ControllerHandle. Drivers will typically use the device path attached to
  ControllerHandle and/or the services from the bus I/O abstraction attached to
  ControllerHandle to determine if the driver supports ControllerHandle. This function
  may be called many times during platform initialization. In order to reduce boot times, the tests
  performed by this function must be very small, and take as little time as possible to execute. This
  function must not change the state of any hardware devices, and this function must be aware that the
  device specified by ControllerHandle may already be managed by the same driver or a
  different driver. This function must match its calls to AllocatePages() with FreePages(),
  AllocatePool() with FreePool(), and OpenProtocol() with CloseProtocol().
  Because ControllerHandle may have been previously started by the same driver, if a protocol is
  already in the opened state, then it must not be closed with CloseProtocol(). This is required
  to guarantee the state of ControllerHandle is not modified by this function.

  @param[in]  This                 A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.
  @param[in]  ControllerHandle     The handle of the controller to test. This handle
                                   must support a protocol interface that supplies
                                   an I/O abstraction to the driver.
  @param[in]  RemainingDevicePath  A pointer to the remaining portion of a device path.  This
                                   parameter is ignored by device drivers, and is optional for bus
                                   drivers. For bus drivers, if this parameter is not NULL, then
                                   the bus driver must determine if the bus controller specified
                                   by ControllerHandle and the child controller specified
                                   by RemainingDevicePath are both supported by this
                                   bus driver.

  @retval EFI_SUCCESS              The device specified by ControllerHandle and
                                   RemainingDevicePath is supported by the driver specified by This.
  @retval EFI_ALREADY_STARTED      The device specified by ControllerHandle and
                                   RemainingDevicePath is already being managed by the driver
                                   specified by This.
  @retval EFI_ACCESS_DENIED        The device specified by ControllerHandle and
                                   RemainingDevicePath is already being managed by a different
                                   driver or an application that requires exclusive access.
                                   Currently not implemented.
  @retval EFI_UNSUPPORTED          The device specified by ControllerHandle and
                                   RemainingDevicePath is not supported by the driver specified by This.
**/
EFI_STATUS
EFIAPI
Ext4IsBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *BindingProtocol,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH              *RemainingDevicePath  OPTIONAL
  );

/**
  Starts a device controller or a bus controller.

  The Start() function is designed to be invoked from the EFI boot service ConnectController().
  As a result, much of the error checking on the parameters to Start() has been moved into this
  common boot service. It is legal to call Start() from other locations,
  but the following calling restrictions must be followed, or the system behavior will not be deterministic.
  1. ControllerHandle must be a valid EFI_HANDLE.
  2. If RemainingDevicePath is not NULL, then it must be a pointer to a naturally aligned
     EFI_DEVICE_PATH_PROTOCOL.
  3. Prior to calling Start(), the Supported() function for the driver specified by This must
     have been called with the same calling parameters, and Supported() must have returned EFI_SUCCESS.

  @param[in]  This                 A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.
  @param[in]  ControllerHandle     The handle of the controller to start. This handle
                                   must support a protocol interface that supplies
                                   an I/O abstraction to the driver.
  @param[in]  RemainingDevicePath  A pointer to the remaining portion of a device path.  This
                                   parameter is ignored by device drivers, and is optional for bus
                                   drivers. For a bus driver, if this parameter is NULL, then handles
                                   for all the children of Controller are created by this driver.
                                   If this parameter is not NULL and the first Device Path Node is
                                   not the End of Device Path Node, then only the handle for the
                                   child device specified by the first Device Path Node of
                                   RemainingDevicePath is created by this driver.
                                   If the first Device Path Node of RemainingDevicePath is
                                   the End of Device Path Node, no child handle is created by this
                                   driver.

  @retval EFI_SUCCESS              The device was started.
  @retval EFI_DEVICE_ERROR         The device could not be started due to a device error.Currently not implemented.
  @retval EFI_OUT_OF_RESOURCES     The request could not be completed due to a lack of resources.
  @retval Others                   The driver failded to start the device.

**/
EFI_STATUS
EFIAPI
Ext4Bind (
  IN EFI_DRIVER_BINDING_PROTOCOL  *BindingProtocol,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH              *RemainingDevicePath  OPTIONAL
  );

/**
  Stops a device controller or a bus controller.

  The Stop() function is designed to be invoked from the EFI boot service DisconnectController().
  As a result, much of the error checking on the parameters to Stop() has been moved
  into this common boot service. It is legal to call Stop() from other locations,
  but the following calling restrictions must be followed, or the system behavior will not be deterministic.
  1. ControllerHandle must be a valid EFI_HANDLE that was used on a previous call to this
     same driver's Start() function.
  2. The first NumberOfChildren handles of ChildHandleBuffer must all be a valid
     EFI_HANDLE. In addition, all of these handles must have been created in this driver's
     Start() function, and the Start() function must have called OpenProtocol() on
     ControllerHandle with an Attribute of EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER.

  @param[in]  This              A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.
  @param[in]  ControllerHandle  A handle to the device being stopped. The handle must
                                support a bus specific I/O protocol for the driver
                                to use to stop the device.
  @param[in]  NumberOfChildren  The number of child device handles in ChildHandleBuffer.
  @param[in]  ChildHandleBuffer An array of child handles to be freed. May be NULL
                                if NumberOfChildren is 0.

  @retval EFI_SUCCESS           The device was stopped.
  @retval EFI_DEVICE_ERROR      The device could not be stopped due to a device error.

**/
EFI_STATUS
EFIAPI
Ext4Stop (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN UINTN                        NumberOfChildren,
  IN EFI_HANDLE                   *ChildHandleBuffer  OPTIONAL
  );

EFI_DRIVER_BINDING_PROTOCOL  gExt4BindingProtocol =
{
  .Supported           = Ext4IsBindingSupported,
  .Start               = Ext4Bind,
  .Stop                = Ext4Stop,
  .Version             = EXT4_DRIVER_VERSION,
  .ImageHandle         = NULL,
  .DriverBindingHandle = NULL
};

/**
  Retrieves a Unicode string that is the user readable name of the controller
  that is being managed by an EFI Driver.

  @param[in]  This             A pointer to the EFI_COMPONENT_NAME_PROTOCOL instance.
  @param[in]  ControllerHandle The handle of a controller that the driver specified by
                               This is managing.  This handle specifies the controller
                               whose name is to be returned.
  @param[in]  ChildHandle      The handle of the child controller to retrieve the name
                               of.  This is an optional parameter that may be NULL.  It
                               will be NULL for device drivers.  It will also be NULL
                               for a bus drivers that wish to retrieve the name of the
                               bus controller.  It will not be NULL for a bus driver
                               that wishes to retrieve the name of a child controller.
  @param[in]  Language         A pointer to a three character ISO 639-2 language
                               identifier.  This is the language of the controller name
                               that the caller is requesting, and it must match one
                               of the languages specified in SupportedLanguages.  The
                               number of languages supported by a driver is up to the
                               driver writer.
  @param[out]  ControllerName   A pointer to the Unicode string to return.  This Unicode
                                string is the name of the controller specified by
                                ControllerHandle and ChildHandle in the language specified
                                by Language, from the point of view of the driver specified
                                by This.

  @retval EFI_SUCCESS           The Unicode string for the user-readable name in the
                                language specified by Language for the driver
                                specified by This was returned in DriverName.
  @retval EFI_INVALID_PARAMETER ControllerHandle is NULL.
  @retval EFI_INVALID_PARAMETER ChildHandle is not NULL and it is not a valid EFI_HANDLE.
  @retval EFI_INVALID_PARAMETER Language is NULL.
  @retval EFI_INVALID_PARAMETER ControllerName is NULL.
  @retval EFI_UNSUPPORTED       The driver specified by This is not currently managing
                                the controller specified by ControllerHandle and
                                ChildHandle.
  @retval EFI_UNSUPPORTED       The driver specified by This does not support the
                                language specified by Language.

**/
EFI_STATUS
EFIAPI
Ext4ComponentNameGetControllerName (
  IN  EFI_COMPONENT_NAME_PROTOCOL  *This,
  IN  EFI_HANDLE                   ControllerHandle,
  IN  EFI_HANDLE                   ChildHandle  OPTIONAL,
  IN  CHAR8                        *Language,
  OUT CHAR16                       **ControllerName
  )
{
  EFI_STATUS  Status;

  if (ChildHandle != NULL) {
    return EFI_UNSUPPORTED;
  }

  // Test if the driver manages ControllHandle
  Status = EfiTestManagedDevice (
             ControllerHandle,
             gExt4BindingProtocol.DriverBindingHandle,
             &gEfiDiskIoProtocolGuid
             );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  return LookupUnicodeString2 (
           Language,
           This->SupportedLanguages,
           mExt4ControllerNameTable,
           ControllerName,
           (BOOLEAN)(This == &gExt4ComponentName)
           );
}

/**
  Retrieves a Unicode string that is the user-readable name of the EFI Driver.

  @param[in]  This        A pointer to the EFI_COMPONENT_NAME_PROTOCOL instance.
  @param[in]  Language    A pointer to a three-character ISO 639-2 language identifier.
                          This is the language of the driver name that that the caller
                          is requesting, and it must match one of the languages specified
                          in SupportedLanguages.  The number of languages supported by a
                          driver is up to the driver writer.
  @param[out]  DriverName A pointer to the Unicode string to return.  This Unicode string
                          is the name of the driver specified by This in the language
                          specified by Language.

  @retval EFI_SUCCESS           The Unicode string for the Driver specified by This
                                and the language specified by Language was returned
                                in DriverName.
  @retval EFI_INVALID_PARAMETER Language is NULL.
  @retval EFI_INVALID_PARAMETER DriverName is NULL.
  @retval EFI_UNSUPPORTED       The driver specified by This does not support the
                                language specified by Language.

**/
EFI_STATUS
EFIAPI
Ext4ComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME_PROTOCOL  *This,
  IN  CHAR8                        *Language,
  OUT CHAR16                       **DriverName
  )
{
  return LookupUnicodeString2 (
           Language,
           This->SupportedLanguages,
           mExt4DriverNameTable,
           DriverName,
           (BOOLEAN)(This == &gExt4ComponentName)
           );
}

/**
  Stops a device controller or a bus controller.

  The Stop() function is designed to be invoked from the EFI boot service DisconnectController().
  As a result, much of the error checking on the parameters to Stop() has been moved
  into this common boot service. It is legal to call Stop() from other locations,
  but the following calling restrictions must be followed, or the system behavior will not be deterministic.
  1. ControllerHandle must be a valid EFI_HANDLE that was used on a previous call to this
     same driver's Start() function.
  2. The first NumberOfChildren handles of ChildHandleBuffer must all be a valid
     EFI_HANDLE. In addition, all of these handles must have been created in this driver's
     Start() function, and the Start() function must have called OpenProtocol() on
     ControllerHandle with an Attribute of EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER.

  @param[in]  This              A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.
  @param[in]  ControllerHandle  A handle to the device being stopped. The handle must
                                support a bus specific I/O protocol for the driver
                                to use to stop the device.
  @param[in]  NumberOfChildren  The number of child device handles in ChildHandleBuffer.
  @param[in]  ChildHandleBuffer An array of child handles to be freed. May be NULL
                                if NumberOfChildren is 0.

  @retval EFI_SUCCESS           The device was stopped.
  @retval EFI_DEVICE_ERROR      The device could not be stopped due to a device error.

**/
EFI_STATUS
EFIAPI
Ext4Stop (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN UINTN                        NumberOfChildren,
  IN EFI_HANDLE                   *ChildHandleBuffer  OPTIONAL
  )
{
  EFI_STATUS                       Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *Sfs;
  EXT4_PARTITION                   *Partition;
  BOOLEAN                          HasDiskIo2;

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiSimpleFileSystemProtocolGuid,
                  (VOID **)&Sfs,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Partition = (EXT4_PARTITION *)Sfs;

  HasDiskIo2 = EXT4_DISK_IO2 (Partition) != NULL;

  Status = Ext4UnmountAndFreePartition (Partition);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->UninstallMultipleProtocolInterfaces (
                  ControllerHandle,
                  &gEfiSimpleFileSystemProtocolGuid,
                  &Partition->Interface,
                  NULL
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Close all open protocols (DiskIo, DiskIo2, BlockIo)

  Status = gBS->CloseProtocol (
                  ControllerHandle,
                  &gEfiDiskIoProtocolGuid,
                  This->DriverBindingHandle,
                  ControllerHandle
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->CloseProtocol (
                  ControllerHandle,
                  &gEfiBlockIoProtocolGuid,
                  This->DriverBindingHandle,
                  ControllerHandle
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (HasDiskIo2) {
    Status = gBS->CloseProtocol (
                    ControllerHandle,
                    &gEfiDiskIo2ProtocolGuid,
                    This->DriverBindingHandle,
                    ControllerHandle
                    );

    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  return Status;
}

/**
   Ext4Dxe Driver's entry point.

   Called at load time.

   @param[in]    ImageHandle   Handle to the image.
   @param[in]    SystemTable   Pointer to the EFI_SYSTEM_TABLE.
   @return Result of the load.
**/
EFI_STATUS
EFIAPI
Ext4EntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return EfiLibInstallAllDriverProtocols2 (
           ImageHandle,
           SystemTable,
           &gExt4BindingProtocol,
           ImageHandle,
           &gExt4ComponentName,
           &gExt4ComponentName2,
           NULL,
           NULL,
           NULL,
           NULL
           );
}

/**
   Ext4Dxe Driver's unload callback.

   Called at unload time.

   @param[in]    ImageHandle   Handle to the image.
   @return Result of the unload.
**/
EFI_STATUS
EFIAPI
Ext4Unload (
  IN EFI_HANDLE  ImageHandle
  )
{
  EFI_STATUS  Status;
  EFI_HANDLE  *DeviceHandleBuffer;
  UINTN       DeviceHandleCount;
  UINTN       Index;
  EFI_HANDLE  Handle;

  Status = gBS->LocateHandleBuffer (
                  AllHandles,
                  NULL,
                  NULL,
                  &DeviceHandleCount,
                  &DeviceHandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Index = 0; Index < DeviceHandleCount; Index++) {
    Handle = DeviceHandleBuffer[Index];

    Status = EfiTestManagedDevice (Handle, ImageHandle, &gEfiDiskIoProtocolGuid);

    if (Status == EFI_SUCCESS) {
      Status = gBS->DisconnectController (Handle, ImageHandle, NULL);

      if (EFI_ERROR (Status)) {
        break;
      }
    }
  }

  FreePool (DeviceHandleBuffer);

  Status = EfiLibUninstallAllDriverProtocols2 (
             &gExt4BindingProtocol,
             &gExt4ComponentName,
             &gExt4ComponentName2,
             NULL,
             NULL,
             NULL,
             NULL
             );

  return Status;
}

/**
  Tests to see if this driver supports a given controller. If a child device is provided,
  it further tests to see if this driver supports creating a handle for the specified child device.

  This function checks to see if the driver specified by This supports the device specified by
  ControllerHandle. Drivers will typically use the device path attached to
  ControllerHandle and/or the services from the bus I/O abstraction attached to
  ControllerHandle to determine if the driver supports ControllerHandle. This function
  may be called many times during platform initialization. In order to reduce boot times, the tests
  performed by this function must be very small, and take as little time as possible to execute. This
  function must not change the state of any hardware devices, and this function must be aware that the
  device specified by ControllerHandle may already be managed by the same driver or a
  different driver. This function must match its calls to AllocatePages() with FreePages(),
  AllocatePool() with FreePool(), and OpenProtocol() with CloseProtocol().
  Because ControllerHandle may have been previously started by the same driver, if a protocol is
  already in the opened state, then it must not be closed with CloseProtocol(). This is required
  to guarantee the state of ControllerHandle is not modified by this function.

  @param[in]  This                 A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.
  @param[in]  ControllerHandle     The handle of the controller to test. This handle
                                   must support a protocol interface that supplies
                                   an I/O abstraction to the driver.
  @param[in]  RemainingDevicePath  A pointer to the remaining portion of a device path.  This
                                   parameter is ignored by device drivers, and is optional for bus
                                   drivers. For bus drivers, if this parameter is not NULL, then
                                   the bus driver must determine if the bus controller specified
                                   by ControllerHandle and the child controller specified
                                   by RemainingDevicePath are both supported by this
                                   bus driver.

  @retval EFI_SUCCESS              The device specified by ControllerHandle and
                                   RemainingDevicePath is supported by the driver specified by This.
  @retval EFI_ALREADY_STARTED      The device specified by ControllerHandle and
                                   RemainingDevicePath is already being managed by the driver
                                   specified by This.
  @retval EFI_ACCESS_DENIED        The device specified by ControllerHandle and
                                   RemainingDevicePath is already being managed by a different
                                   driver or an application that requires exclusive access.
  @retval EFI_UNSUPPORTED          The device specified by ControllerHandle and
                                   RemainingDevicePath is not supported by the driver specified by This.
**/
EFI_STATUS
EFIAPI
Ext4IsBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *BindingProtocol,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH              *RemainingDevicePath  OPTIONAL
  )
{
  EFI_STATUS             Status;
  EFI_DISK_IO_PROTOCOL   *DiskIo;
  EFI_BLOCK_IO_PROTOCOL  *BlockIo;

  DiskIo  = NULL;
  BlockIo = NULL;

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiDiskIoProtocolGuid,
                  (VOID **)&DiskIo,
                  BindingProtocol->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiBlockIoProtocolGuid,
                  (VOID **)&BlockIo,
                  BindingProtocol->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (!EFI_ERROR (Status)) {
    if (!Ext4SuperblockCheckMagic (DiskIo, BlockIo)) {
      Status = EFI_UNSUPPORTED;
    }
  }

  //
  // Close the I/O Abstraction(s) used to perform the supported test
  //
  if (DiskIo != NULL) {
    gBS->CloseProtocol (
           ControllerHandle,
           &gEfiDiskIoProtocolGuid,
           BindingProtocol->DriverBindingHandle,
           ControllerHandle
           );
  }

  if (BlockIo != NULL) {
    gBS->CloseProtocol (
           ControllerHandle,
           &gEfiBlockIoProtocolGuid,
           BindingProtocol->DriverBindingHandle,
           ControllerHandle
           );
  }

  return Status;
}

/**
  Starts a device controller or a bus controller.

  The Start() function is designed to be invoked from the EFI boot service ConnectController().
  As a result, much of the error checking on the parameters to Start() has been moved into this
  common boot service. It is legal to call Start() from other locations,
  but the following calling restrictions must be followed, or the system behavior will not be deterministic.
  1. ControllerHandle must be a valid EFI_HANDLE.
  2. If RemainingDevicePath is not NULL, then it must be a pointer to a naturally aligned
     EFI_DEVICE_PATH_PROTOCOL.
  3. Prior to calling Start(), the Supported() function for the driver specified by This must
     have been called with the same calling parameters, and Supported() must have returned EFI_SUCCESS.

  @param[in]  This                 A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.
  @param[in]  ControllerHandle     The handle of the controller to start. This handle
                                   must support a protocol interface that supplies
                                   an I/O abstraction to the driver.
  @param[in]  RemainingDevicePath  A pointer to the remaining portion of a device path.  This
                                   parameter is ignored by device drivers, and is optional for bus
                                   drivers. For a bus driver, if this parameter is NULL, then handles
                                   for all the children of Controller are created by this driver.
                                   If this parameter is not NULL and the first Device Path Node is
                                   not the End of Device Path Node, then only the handle for the
                                   child device specified by the first Device Path Node of
                                   RemainingDevicePath is created by this driver.
                                   If the first Device Path Node of RemainingDevicePath is
                                   the End of Device Path Node, no child handle is created by this
                                   driver.

  @retval EFI_SUCCESS              The device was started.
  @retval EFI_DEVICE_ERROR         The device could not be started due to a device error.Currently not implemented.
  @retval EFI_OUT_OF_RESOURCES     The request could not be completed due to a lack of resources.
  @retval Others                   The driver failded to start the device.

**/
EFI_STATUS
EFIAPI
Ext4Bind (
  IN EFI_DRIVER_BINDING_PROTOCOL  *BindingProtocol,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH              *RemainingDevicePath  OPTIONAL
  )
{
  EFI_DISK_IO_PROTOCOL   *DiskIo;
  EFI_DISK_IO2_PROTOCOL  *DiskIo2;
  EFI_BLOCK_IO_PROTOCOL  *BlockIo;
  EFI_STATUS             Status;

  DiskIo2 = NULL;
  BlockIo = NULL;
  DiskIo  = NULL;

  // Note: We initialize collation here since this is called in BDS, when we are likely
  // to have the Unicode Collation protocols available.
  Status = Ext4InitialiseUnicodeCollation (BindingProtocol->ImageHandle);
  if (EFI_ERROR (Status)) {
    // Lets throw a loud error into the log
    // It is very unlikely something like this may fire out of the blue. Chances are either
    // the platform configuration is wrong, or we are.
    DEBUG ((DEBUG_ERROR, "[ext4] Error: Unicode Collation not available - failure to Start() - error %r\n", Status));
    goto Error;
  }

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiDiskIoProtocolGuid,
                  (VOID **)&DiskIo,
                  BindingProtocol->ImageHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR (Status)) {
    goto Error;
  }

  DEBUG ((DEBUG_INFO, "[ext4] Controller supports DISK_IO\n"));

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiDiskIo2ProtocolGuid,
                  (VOID **)&DiskIo2,
                  BindingProtocol->ImageHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  // It's okay to not support DISK_IO2

  if (DiskIo2 != NULL) {
    DEBUG ((DEBUG_INFO, "[ext4] Controller supports DISK_IO2\n"));
  }

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiBlockIoProtocolGuid,
                  (VOID **)&BlockIo,
                  BindingProtocol->ImageHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    goto Error;
  }

  Status = Ext4OpenPartition (ControllerHandle, DiskIo, DiskIo2, BlockIo);

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[ext4] Error mounting: %r\n", Status));
    // Falls through to Error
  } else {
    return Status;
  }

Error:
  if (DiskIo) {
    gBS->CloseProtocol (
           ControllerHandle,
           &gEfiDiskIoProtocolGuid,
           BindingProtocol->ImageHandle,
           ControllerHandle
           );
  }

  if (DiskIo2) {
    gBS->CloseProtocol (
           ControllerHandle,
           &gEfiDiskIo2ProtocolGuid,
           BindingProtocol->ImageHandle,
           ControllerHandle
           );
  }

  if (BlockIo) {
    gBS->CloseProtocol (
           ControllerHandle,
           &gEfiBlockIoProtocolGuid,
           BindingProtocol->ImageHandle,
           ControllerHandle
           );
  }

  return Status;
}
