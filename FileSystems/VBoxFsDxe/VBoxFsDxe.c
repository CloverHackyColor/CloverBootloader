/* $Id: VBoxFsDxe.c 29125 2010-05-06 09:43:05Z vboxsync $ */
/** @file
 * VBoxFsDxe.c - VirtualBox FS wrapper
 */

/*
 * Copyright (C) 2010 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 */

/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#include <Protocol/ComponentName.h>
#include <Protocol/ComponentName2.h>
#include <Protocol/DriverBinding.h>
#include <Protocol/PciIo.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <IndustryStandard/Pci22.h>

/*******************************************************************************
*   Internal Functions                                                         *
*******************************************************************************/
static EFI_STATUS EFIAPI
VBoxFsDB_Supported(IN EFI_DRIVER_BINDING_PROTOCOL *This, IN EFI_HANDLE ControllerHandle,
                   IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath OPTIONAL);
static EFI_STATUS EFIAPI
VBoxFsDB_Start(IN EFI_DRIVER_BINDING_PROTOCOL *This, IN EFI_HANDLE ControllerHandle,
               IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath OPTIONAL);
static EFI_STATUS EFIAPI
VBoxFsDB_Stop(IN EFI_DRIVER_BINDING_PROTOCOL *This, IN EFI_HANDLE ControllerHandle,
              IN UINTN NumberOfChildren, IN EFI_HANDLE *ChildHandleBuffer OPTIONAL);

static EFI_STATUS EFIAPI
VBoxFsCN_GetDriverName(IN EFI_COMPONENT_NAME_PROTOCOL *This,
                       IN CHAR8 *Language, OUT CHAR16 **DriverName);
static EFI_STATUS EFIAPI
VBoxFsCN_GetControllerName(IN EFI_COMPONENT_NAME_PROTOCOL *This,
                           IN EFI_HANDLE ControllerHandle,
                           IN EFI_HANDLE ChildHandle OPTIONAL,
                           IN CHAR8 *Language,  OUT CHAR16 **ControllerName);

static EFI_STATUS EFIAPI
VBoxFsCN2_GetDriverName(IN EFI_COMPONENT_NAME2_PROTOCOL *This,
                        IN CHAR8 *Language, OUT CHAR16 **DriverName);
static EFI_STATUS EFIAPI
VBoxFsCN2_GetControllerName(IN EFI_COMPONENT_NAME2_PROTOCOL *This,
                            IN EFI_HANDLE ControllerHandle,
                            IN EFI_HANDLE ChildHandle OPTIONAL,
                            IN CHAR8 *Language,  OUT CHAR16 **ControllerName);


/** EFI Driver Binding Protocol. */
static EFI_DRIVER_BINDING_PROTOCOL          g_VBoxFsDB =
{
    VBoxFsDB_Supported,
    VBoxFsDB_Start,
    VBoxFsDB_Stop,
    /* .Version             = */    1,
    /* .ImageHandle         = */ NULL,
    /* .DriverBindingHandle = */ NULL
};

/** EFI Component Name Protocol. */
static const EFI_COMPONENT_NAME_PROTOCOL    g_VBoxFsCN =
{
    VBoxFsCN_GetDriverName,
    VBoxFsCN_GetControllerName,
    "eng"
};

/** EFI Component Name 2 Protocol. */
static const EFI_COMPONENT_NAME2_PROTOCOL   g_VBoxFsCN2 =
{
    VBoxFsCN2_GetDriverName,
    VBoxFsCN2_GetControllerName,
    "en"
};

/** Driver name translation table. */
static CONST EFI_UNICODE_STRING_TABLE       g_aVBoxFsDriverLangAndNames[] =
{
    {   "eng;en",   L"VBox Universal FS Wrapper Driver" },
    {   NULL,       NULL }
};



/**
 * VBoxFsDxe entry point.
 *
 * @returns EFI status code.
 *
 * @param   ImageHandle     The image handle.
 * @param   SystemTable     The system table pointer.
 */
EFI_STATUS EFIAPI
DxeInitializeVBoxFs(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS  rc;
    DEBUG((DEBUG_INFO, "DxeInitializeVBoxFsDxe\n"));

    rc = EfiLibInstallDriverBindingComponentName2(ImageHandle, SystemTable,
                                                  &g_VBoxFsDB, ImageHandle,
                                                  &g_VBoxFsCN, &g_VBoxFsCN2);
    ASSERT_EFI_ERROR(rc);
    return rc;
}

EFI_STATUS EFIAPI
DxeUninitializeVBoxFs(IN EFI_HANDLE         ImageHandle)
{
    return EFI_SUCCESS;
}


/**
 * @copydoc EFI_DRIVER_BINDING_SUPPORTED
 */
static EFI_STATUS EFIAPI
VBoxFsDB_Supported(IN EFI_DRIVER_BINDING_PROTOCOL *This, IN EFI_HANDLE ControllerHandle,
                   IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath OPTIONAL)
{
    EFI_STATUS              rcRet = EFI_UNSUPPORTED;
    /* EFI_STATUS              rc; */

    return rcRet;
}


/**
 * @copydoc EFI_DRIVER_BINDING_START
 */
static EFI_STATUS EFIAPI
VBoxFsDB_Start(IN EFI_DRIVER_BINDING_PROTOCOL *This, IN EFI_HANDLE ControllerHandle,
               IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath OPTIONAL)
{
    /* EFI_STATUS              rc; */

    return  EFI_UNSUPPORTED;
}


/**
 * @copydoc EFI_DRIVER_BINDING_STOP
 */
static EFI_STATUS EFIAPI
VBoxFsDB_Stop(IN EFI_DRIVER_BINDING_PROTOCOL *This, IN EFI_HANDLE ControllerHandle,
              IN UINTN NumberOfChildren, IN EFI_HANDLE *ChildHandleBuffer OPTIONAL)
{
    /* EFI_STATUS                  rc; */

    return  EFI_UNSUPPORTED;
}


/** @copydoc EFI_COMPONENT_NAME_GET_DRIVER_NAME */
static EFI_STATUS EFIAPI
VBoxFsCN_GetDriverName(IN EFI_COMPONENT_NAME_PROTOCOL *This,
                       IN CHAR8 *Language, OUT CHAR16 **DriverName)
{
    return LookupUnicodeString2(Language,
                                This->SupportedLanguages,
                                &g_aVBoxFsDriverLangAndNames[0],
                                DriverName,
                                TRUE);
}

/** @copydoc EFI_COMPONENT_NAME_GET_CONTROLLER_NAME */
static EFI_STATUS EFIAPI
VBoxFsCN_GetControllerName(IN EFI_COMPONENT_NAME_PROTOCOL *This,
                                    IN EFI_HANDLE ControllerHandle,
                                    IN EFI_HANDLE ChildHandle OPTIONAL,
                                    IN CHAR8 *Language, OUT CHAR16 **ControllerName)
{
    /** @todo try query the protocol from the controller and forward the query. */
    return EFI_UNSUPPORTED;
}

/** @copydoc EFI_COMPONENT_NAME2_GET_DRIVER_NAME */
static EFI_STATUS EFIAPI
VBoxFsCN2_GetDriverName(IN EFI_COMPONENT_NAME2_PROTOCOL *This,
                        IN CHAR8 *Language, OUT CHAR16 **DriverName)
{
    return LookupUnicodeString2(Language,
                                This->SupportedLanguages,
                                &g_aVBoxFsDriverLangAndNames[0],
                                DriverName,
                                FALSE);
}

/** @copydoc EFI_COMPONENT_NAME2_GET_CONTROLLER_NAME */
static EFI_STATUS EFIAPI
VBoxFsCN2_GetControllerName(IN EFI_COMPONENT_NAME2_PROTOCOL *This,
                            IN EFI_HANDLE ControllerHandle,
                            IN EFI_HANDLE ChildHandle OPTIONAL,
                            IN CHAR8 *Language, OUT CHAR16 **ControllerName)
{
    /** @todo try query the protocol from the controller and forward the query. */
    return EFI_UNSUPPORTED;
}
