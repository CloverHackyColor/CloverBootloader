/** @file

  Implement all four UEFI runtime variable services and 
  install variable architeture protocol.
  
Copyright (c) 2006 - 2010, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Protocol/EmuVariableControl.h>

#include "Variable.h"

EFI_EVENT   mVirtualAddressChangeEvent = NULL;
EFI_EVENT   mExitBootServicesEvent = NULL;
BOOLEAN     mAtRuntime = FALSE;

/** Original runtime services. */
EFI_RUNTIME_SERVICES gOrgRT;

/** Pointer to runtime services. */
extern EFI_RUNTIME_SERVICES *gRT;

/** Apple Boot Guid - cars with this GUID are visible in OSX with nvram */
extern EFI_GUID gEfiAppleBootGuid;

EFI_GUID DellEventGuid = {0xFF2E9FC7, 0xD16F, 0x434A, {0xA2, 0x4E, 0xC9, 0x95, 0x19, 0xB7, 0xEB, 0x93}};



/**

  This code finds variable in storage blocks (Volatile or Non-Volatile).

  @param VariableName               Name of Variable to be found.
  @param VendorGuid                 Variable vendor GUID.
  @param Attributes                 Attribute value of the variable found.
  @param DataSize                   Size of Data found. If size is less than the
                                    data, this value contains the required size.
  @param Data                       Data pointer.
                      
  @return EFI_INVALID_PARAMETER     Invalid parameter
  @return EFI_SUCCESS               Find the specified variable
  @return EFI_NOT_FOUND             Not found
  @return EFI_BUFFER_TO_SMALL       DataSize is too small for the result

**/
EFI_STATUS
EFIAPI
RuntimeServiceGetVariable (
  IN CHAR16        *VariableName,
  IN EFI_GUID      *VendorGuid,
  OUT UINT32       *Attributes OPTIONAL,
  IN OUT UINTN     *DataSize,
  OUT VOID         *Data
  )
{
  return EmuGetVariable (
          VariableName,
          VendorGuid,
          Attributes OPTIONAL,
          DataSize,
          Data,
          &mVariableModuleGlobal->VariableGlobal[Physical]
          );
}

/**

  This code Finds the Next available variable.

  @param VariableNameSize           Size of the variable name
  @param VariableName               Pointer to variable name
  @param VendorGuid                 Variable Vendor Guid

  @return EFI_INVALID_PARAMETER     Invalid parameter
  @return EFI_SUCCESS               Find the specified variable
  @return EFI_NOT_FOUND             Not found
  @return EFI_BUFFER_TO_SMALL       DataSize is too small for the result

**/
EFI_STATUS
EFIAPI
RuntimeServiceGetNextVariableName (
  IN OUT UINTN     *VariableNameSize,
  IN OUT CHAR16    *VariableName,
  IN OUT EFI_GUID  *VendorGuid
  )
{
  return EmuGetNextVariableName (
          VariableNameSize,
          VariableName,
          VendorGuid,
          &mVariableModuleGlobal->VariableGlobal[Physical]
          );
}

/**

  This code sets variable in storage blocks (Volatile or Non-Volatile).

  @param VariableName                     Name of Variable to be found
  @param VendorGuid                       Variable vendor GUID
  @param Attributes                       Attribute value of the variable found
  @param DataSize                         Size of Data found. If size is less than the
                                          data, this value contains the required size.
  @param Data                             Data pointer

  @return EFI_INVALID_PARAMETER           Invalid parameter
  @return EFI_SUCCESS                     Set successfully
  @return EFI_OUT_OF_RESOURCES            Resource not enough to set variable
  @return EFI_NOT_FOUND                   Not found
  @return EFI_WRITE_PROTECTED             Variable is read-only

**/
EFI_STATUS
EFIAPI
RuntimeServiceSetVariable (
  IN CHAR16        *VariableName,
  IN EFI_GUID      *VendorGuid,
  IN UINT32        Attributes,
  IN UINTN         DataSize,
  IN VOID          *Data
  )
{
  if (CompareGuid(VendorGuid, &DellEventGuid)) {
    return EFI_SUCCESS;
  }
  return EmuSetVariable (
          VariableName,
          VendorGuid,
          Attributes,
          DataSize,
          Data,
          &mVariableModuleGlobal->VariableGlobal[Physical],
          &mVariableModuleGlobal->VolatileLastVariableOffset,
          &mVariableModuleGlobal->NonVolatileLastVariableOffset
          );
}

/**

  This code returns information about the EFI variables.

  @param Attributes                     Attributes bitmask to specify the type of variables
                                        on which to return information.
  @param MaximumVariableStorageSize     Pointer to the maximum size of the storage space available
                                        for the EFI variables associated with the attributes specified.
  @param RemainingVariableStorageSize   Pointer to the remaining size of the storage space available
                                        for EFI variables associated with the attributes specified.
  @param MaximumVariableSize            Pointer to the maximum size of an individual EFI variables
                                        associated with the attributes specified.

  @return EFI_INVALID_PARAMETER         An invalid combination of attribute bits was supplied.
  @return EFI_SUCCESS                   Query successfully.
  @return EFI_UNSUPPORTED               The attribute is not supported on this platform.

**/
EFI_STATUS
EFIAPI
RuntimeServiceQueryVariableInfo (
  IN  UINT32                 Attributes,
  OUT UINT64                 *MaximumVariableStorageSize,
  OUT UINT64                 *RemainingVariableStorageSize,
  OUT UINT64                 *MaximumVariableSize
  )
{
  return EmuQueryVariableInfo (
          Attributes,
          MaximumVariableStorageSize,
          RemainingVariableStorageSize,
          MaximumVariableSize,
          &mVariableModuleGlobal->VariableGlobal[Physical]
          );
}

/**
  Notification function of EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE.

  This is a notification function registered on EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE event.
  It convers pointer to new virtual address.

  @param  Event        Event whose notification function is being invoked.
  @param  Context      Pointer to the notification function's context.

**/
VOID
EFIAPI
VariableClassAddressChangeEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->PlatformLangCodes);
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->LangCodes);
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal->PlatformLang);
  gRT->ConvertPointer (
    0x0,
    (VOID **) &mVariableModuleGlobal->VariableGlobal[Physical].NonVolatileVariableBase
    );
  gRT->ConvertPointer (
    0x0,
    (VOID **) &mVariableModuleGlobal->VariableGlobal[Physical].VolatileVariableBase
    );
  gRT->ConvertPointer (0x0, (VOID **) &mVariableModuleGlobal);
}

/**
  Notification function of EVT_SIGNAL_EXIT_BOOT_SERVICES.

  This is a notification function registered on EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE event.
  It sets AtRuntime flag as TRUE after ExitBootServices.

  @param[in]  Event   The Event that is being processed.
  @param[in]  Context The Event Context.

**/
VOID
EFIAPI
VariableClassExitBootServicesEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  mAtRuntime = TRUE;
}

/**
  This function allows the caller to determine if UEFI ExitBootServices() has been called.

  This function returns TRUE after all the EVT_SIGNAL_EXIT_BOOT_SERVICES functions have
  executed as a result of the OS calling ExitBootServices().  Prior to this time FALSE
  is returned. This function is used by runtime code to decide it is legal to access
  services that go away after ExitBootServices().

  @retval  TRUE  The system has finished executing the EVT_SIGNAL_EXIT_BOOT_SERVICES event.
  @retval  FALSE The system has not finished executing the EVT_SIGNAL_EXIT_BOOT_SERVICES event.

**/
BOOLEAN
EFIAPI
VariableClassAtRuntime (
  VOID
  )
{
  return mAtRuntime;
}


////////////////////////////////////////
// Helper methods
//
//BOOLEAN CopyVarsDebugPrint = TRUE;
//#define DBG_COPY(...) if (CopyVarsDebugPrint) DBG(__VA_ARGS__)
#define DBG_COPY(...)

/** Copies vars from SrcRT into DestRT. */
VOID
EFIAPI
CopyRTVariables (
  EFI_RUNTIME_SERVICES    *SrcRT,
  EFI_RUNTIME_SERVICES    *DestRT
)
{
  EFI_STATUS              Status;
  CHAR16                  *Name;
  EFI_GUID                Guid;
  UINTN                   NameSize;
  UINTN                   NewNameSize;
  UINT32                  Attributes;
  VOID                    *Data;
  UINTN                   DataSize;
  UINTN                   NewDataSize;
  
  DBG_COPY("\nCopyRTVariables:\n");
  //
  // First call to GetNextVariableName is with L"\0"
  //
  NameSize    = 512;
  Name        = AllocateZeroPool(NameSize);
  
  //
  // Initial Data buffer
  //
  DataSize    = 512;
  Data        = AllocateZeroPool(DataSize);
  
  while (TRUE) {
    //
    // Get next variable name from SrcRT
    //
    NewNameSize = NameSize;
    //DBG("- NS: %d, NNS: %d\n", NameSize, NewNameSize);
    Status = SrcRT->GetNextVariableName (&NewNameSize, Name, &Guid);
    if (Status == EFI_BUFFER_TOO_SMALL) {
      //DBG("- EFI_BUFFER_TOO_SMALL: %d, %d\n", NameSize, NewNameSize);
      Name = ReallocatePool (NameSize, NewNameSize, Name);
      NameSize = NewNameSize;
      Status = SrcRT->GetNextVariableName (&NewNameSize, Name, &Guid);
    }
    //DBG("--- NS: %d, NNS: %d\n", NameSize, NewNameSize);
    
    if (Status == EFI_NOT_FOUND) {
      DBG_COPY("- EFI_NOT_FOUND\n");
      break;
    }
    if (EFI_ERROR(Status)) {
      DBG_COPY("- %r\n", Status);
      break;
    }
    DBG_COPY("- %g:'%s'", &Guid, Name);
    
    //
    // Read variable from SrcRT
    //
    NewDataSize = DataSize;
    Status = SrcRT->GetVariable (Name, &Guid, &Attributes, &NewDataSize, Data);
    if (Status == EFI_BUFFER_TOO_SMALL) {
      Data = ReallocatePool (DataSize, NewDataSize, Data);
      DataSize = NewDataSize;
      Status = SrcRT->GetVariable (Name, &Guid, &Attributes, &NewDataSize, Data);
    }
    if (EFI_ERROR(Status)) {
      DBG_COPY(" %r\n", Status);
      break;
    }
    DBG_COPY(" a: %04x, l: %d", Attributes, NewDataSize);
    
    //
    // Delete variable if exists in DestRT
    //
    Status = DestRT->SetVariable (Name, &Guid, 0, 0, NULL);
    if (Status == EFI_SUCCESS) {
      DBG_COPY(" -> del: %r", Status);
    }
    //
    // Write variable to DestRT
    //
    Status = DestRT->SetVariable (Name, &Guid, Attributes, NewDataSize, Data);
    DBG_COPY(" -> write: %r\n", Status);
  }
  
  // print just the first time
  //CopyVarsDebugPrint = FALSE;
  
  FreePool(Name);
  FreePool(Data);
}

////////////////////////////////////////
// 
// EMU_VARIABLE_CONTROL_PROTOCOL
//

/**
 Installs EmuVariable runtime var services.
 **/
EFI_STATUS
EFIAPI
EmuVariableControlProtocolInstallEmulation (
  IN EMU_VARIABLE_CONTROL_PROTOCOL  *This
  )
{
  EFI_STATUS            Status;
  EFI_RUNTIME_SERVICES  DestRT;
  
  DBG("EmuVariable InstallEmulation:");
  if (gRT->GetVariable == RuntimeServiceGetVariable) {
    DBG(" EFI_ALREADY_STARTED\n");
    return EFI_ALREADY_STARTED;
  }
  
  //
  // Copy rt vars to emulation store
  // (use temp DestRT structure to pass SetVariable pointer)
  //
  DestRT.SetVariable = RuntimeServiceSetVariable;
  CopyRTVariables (gRT, &DestRT);
  DBG(" orig vars copied");
  
  //
  // Install emulation services
  //
  gRT->GetVariable         = RuntimeServiceGetVariable;
  gRT->GetNextVariableName = RuntimeServiceGetNextVariableName;
  gRT->SetVariable         = RuntimeServiceSetVariable;
  gRT->QueryVariableInfo   = RuntimeServiceQueryVariableInfo;
  
  gRT->Hdr.CRC32 = 0;
  gBS->CalculateCrc32 (gRT, gRT->Hdr.HeaderSize, &gRT->Hdr.CRC32);
  DBG(", emu.var.services installed");
  
  
  /* original, new style CreateEventEx - fails on Phoenix UEFI and hangs on Apple EFI
   // Note: UefiRuntimeLib is also using CreateEventEx, so avoid using that lib as well
   
   Status = gBS->CreateEventEx (
                               EVT_NOTIFY_SIGNAL,
                               TPL_NOTIFY,
                               VariableClassAddressChangeEvent,
                               NULL,
                               &gEfiEventVirtualAddressChangeGuid,
                               &mVirtualAddressChangeEvent
                               );
   DBG(", CreateEventEx VirtualAddressChange = %r\n", Status);
   ASSERT_EFI_ERROR(Status);
   
   Status = gBS->CreateEventEx (
                              EVT_NOTIFY_SIGNAL,
                              TPL_NOTIFY,
                              VariableClassExitBootServicesEvent,
                              NULL,
                              &gEfiEventExitBootServicesGuid,
                              &mExitBootServicesEvent
                              );
   DBG(", CreateEventEx ExitBootServices = %r\n", Status);
   ASSERT_EFI_ERROR(Status); 
  */
  
  // old style CreateEvent
  
  //
  // Create a Set Virtual Address Map event.
  //
  Status = gBS->CreateEvent (
                             EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE,
                             TPL_NOTIFY,
                             VariableClassAddressChangeEvent,
                             NULL,
                             &mVirtualAddressChangeEvent
                             );
  
  DBG(", CreateEvent VirtualAddressChange = %r", Status);
//  ASSERT_EFI_ERROR(Status);
  
  //
  // Create an Exit Boot Services event.
  //
  Status = gBS->CreateEvent (
                             EVT_SIGNAL_EXIT_BOOT_SERVICES,
                             TPL_NOTIFY,
                             VariableClassExitBootServicesEvent,
                             NULL,
                             &mExitBootServicesEvent
                             );
  
  DBG(", CreateEvent ExitBootServices = %r", Status);
//  ASSERT_EFI_ERROR(Status);
  
  //
  // Add EmuVariableUefiPresent variable to allow /ect/rc* scripts to detect
  // that this driver is used.
  //
  Status = gRT->SetVariable (L"EmuVariableUefiPresent",
                             &gEfiAppleBootGuid,
                             EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                             3,
                             "Yes"
                             );
  
  DBG(", set Status=%r\n", Status);
  
  return Status; //EFI_SUCCESS;
}

/**
 Installs original runtime var services.
 **/
EFI_STATUS
EFIAPI
EmuVariableControlProtocolUninstallEmulation (
  IN EMU_VARIABLE_CONTROL_PROTOCOL  *This
  )
{
  EFI_STATUS  Status;
  
  DBG("EmuVariable UninstallEmulation:");
  //
  // Close mVirtualAddressChangeEvent
  //
  Status = gBS->CloseEvent (mVirtualAddressChangeEvent);
  mVirtualAddressChangeEvent = NULL;
  DBG(" CloseEvent = %r", Status);
  
  //
  // Return back original RT var services
  //
  gRT->GetVariable         = gOrgRT.GetVariable;
  gRT->GetNextVariableName = gOrgRT.GetNextVariableName;
  gRT->SetVariable         = gOrgRT.SetVariable;
  gRT->QueryVariableInfo   = gOrgRT.QueryVariableInfo;
  
  gRT->Hdr.CRC32 = 0;
  gBS->CalculateCrc32 (gRT, gRT->Hdr.HeaderSize, &gRT->Hdr.CRC32);
  
  
  DBG(", original var services restored\n");
  
  return EFI_SUCCESS;
}

/** EMU_VARIABLE_CONTROL_PROTOCOL */
EMU_VARIABLE_CONTROL_PROTOCOL mEmuVariableControlProtocol = {
  (EMU_VARIABLE_CONTROL_INSTALL_EMULATION)EmuVariableControlProtocolInstallEmulation,
  (EMU_VARIABLE_CONTROL_UNINSTALL_EMULATION)EmuVariableControlProtocolUninstallEmulation
};


/**
  EmuVariable Driver main entry point. The Variable driver places the 4 EFI
  runtime services in the EFI System Table and installs arch protocols 
  for variable read and write services being available. It also registers
  notification function for EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE event.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.  
  @param[in] SystemTable    A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS       Variable service successfully initialized.

**/
EFI_STATUS
EFIAPI
VariableServiceInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_HANDLE  NewHandle;
  EFI_STATUS  Status;

  DBG("EmuVariableUefi Initialize:");
  Status = VariableCommonInitialize (ImageHandle, SystemTable);
  DBG(" VariableCommonInitialize = %r", Status);
  ASSERT_EFI_ERROR(Status);
  
  //
  // Store orig RS var services
  //
  gRT = SystemTable->RuntimeServices;
  CopyMem (&gOrgRT, gRT, sizeof(EFI_RUNTIME_SERVICES));
  DBG(", orig services stored");

  
  //
  // Install EMU_VARIABLE_CONTROL_PROTOCOL on a new handle
  //
  NewHandle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                                                   &NewHandle,
                                                   &gEmuVariableControlProtocolGuid,
                                                   &mEmuVariableControlProtocol,
                                                   NULL
                                                   );
  DBG(", install gEmuVariableControlProtocolGuid = %r\n", Status);
  
  // For debugging purposes only - start immediately when loaded
   // mEmuVariableControlProtocol.InstallEmulation(&mEmuVariableControlProtocol);
  
  return EFI_SUCCESS;
}


