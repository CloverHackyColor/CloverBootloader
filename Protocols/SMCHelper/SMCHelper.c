/*
 *  SMCHelper.c
 *  
 *  Created by Slice on 03.10.2016.
 *
 */

#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/MemLogLib.h>

#include <Protocol/AppleSMC.h>

// DBG_TO: 0=no debug, 1=serial, 2=console 3=log
// serial requires
// [PcdsFixedAtBuild]
//  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x07
//  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0xFFFFFFFF
// in package DSC file

#define DBG_SMC 0

#if DBG_SMC == 3
#define DBG(...) MemLog(FALSE, 0, __VA_ARGS__)
#elif DBG_SMC == 2
#define DBG(...) AsciiPrint(__VA_ARGS__)
#elif DBG_SMC == 1
#define DBG(...) DebugPrint(1, __VA_ARGS__)
#else
#define DBG(...)
#endif


#define APPLE_SMC_SIGNATURE     SIGNATURE_64('A','P','P','L','E','S','M','C')
#define NON_APPLE_SMC_SIGNATURE SIGNATURE_64('S','M','C','H','E','L','P','E')

EFI_HANDLE              mHandle = NULL;
//EFI_BOOT_SERVICES*			gBS;

extern EFI_GUID gEfiAppleBootGuid;

typedef struct _SMC_STACK SMC_STACK;

struct _SMC_STACK {
  SMC_STACK           *Next;
  SMC_KEY             Id;
  SMC_KEY_TYPE        Type;
  SMC_DATA_SIZE       DataLen;
  SMC_KEY_ATTRIBUTES  Attributes;
  SMC_DATA            *Data;
};

SMC_STACK *SmcStack = NULL;

CHAR8 *StringId(UINT32 DataId)
{
  CHAR8 *Str;
  Str = AllocatePool(5);
  Str[4] = '\0';
  Str[3] = DataId & 0xFF;
  Str[2] = (DataId >> 8) & 0xFF;
  Str[1] = (DataId >> 16) & 0xFF;
  Str[0] = (DataId >> 24) & 0xFF;
  return Str;
}

/*

EFI_STATUS EFIAPI
ReadData (IN APPLE_SMC_PROTOCOL* This, IN UINT32 DataId, IN UINT32 DataLength, IN VOID* DataBuffer)
{
  SMC_STACK *TmpStack = SmcStack;
  INTN Len;
  CHAR8 *Str = StringId(DataId);
  DBG("asked for SMC=%x (%a) len=%d\n", DataId, Str, DataLength);
  FreePool(Str);
  while (TmpStack) {
    if (TmpStack->Id == DataId) {
      Len = MIN(TmpStack->DataLen, DataLength);
      CopyMem(DataBuffer, TmpStack->Data, Len);
      return EFI_SUCCESS;
    } else {
      TmpStack = TmpStack->Next;
    }
  }
  return EFI_NOT_FOUND;
}
 
EFI_STATUS EFIAPI
WriteData (IN APPLE_SMC_PROTOCOL* This, IN UINT32 DataId, IN UINT32 DataLength, IN VOID* DataBuffer)
{
  SMC_STACK *TmpStack = SmcStack;
  INTN Len;
  //First find existing key
  while (TmpStack) {
    if (TmpStack->Id == DataId) {
      Len = MIN(TmpStack->DataLen, DataLength);
      CopyMem(TmpStack->Data, DataBuffer, Len);
      return EFI_SUCCESS;
    } else {
      TmpStack = TmpStack->Next;
    }
  }
  //if not found then create new
  TmpStack = AllocatePool(sizeof(SMC_STACK));
  TmpStack->Next = SmcStack;
  TmpStack->Id = DataId;
  TmpStack->DataLen = DataLength;
  TmpStack->Data = AllocateCopyPool(DataLength, DataBuffer);
  SmcStack = TmpStack;
  return EFI_SUCCESS;
}

EFI_STATUS EFIAPI
DumpData (IN APPLE_SMC_PROTOCOL* This)
{
  INTN Index;
  SMC_STACK *TmpStack = SmcStack;
  
  while (TmpStack) {
    CHAR8 *Str = StringId(TmpStack->Id);
    DBG("found SMC=%x (%a) len=%d data:", TmpStack->Id, Str, TmpStack->DataLen);
    for (Index = 0; Index < TmpStack->DataLen; Index++) {
      DBG("%02x ", *((UINT8*)(TmpStack->Data) + Index));
    }
    DBG("\n");
    FreePool(Str);
    TmpStack = TmpStack->Next;
  }
  return EFI_SUCCESS;
}

APPLE_SMC_PROTOCOL SMCHelperProtocol = {
  APPLE_SMC_SIGNATURE,
  ReadData,
  WriteData,
  DumpData,
};
 
 */

// *************************************************************
// and then we switch to full protocol APPLE_SMC_IO_PROTOCOL

EFI_STATUS EFIAPI
SmcReadValueImpl (IN  APPLE_SMC_IO_PROTOCOL  *This,
                  IN  SMC_KEY                Key,
                  IN  SMC_DATA_SIZE          Size,
                  OUT SMC_DATA               *Value
                  )
{

  SMC_STACK *TmpStack = SmcStack;
  INTN Len;
  CHAR8 *Str;
  
  if (!Value || !Size) {
    return EFI_BUFFER_TOO_SMALL;
  }
  
  Str = StringId(Key);
  DBG("asked for SMC=%x (%a) len=%d\n", Key, Str, Size);
  FreePool(Str);
  while (TmpStack) {
    if (TmpStack->Id == Key) {
      Len = MIN(TmpStack->DataLen, Size);
      CopyMem(Value, TmpStack->Data, Len);
      return EFI_SUCCESS;
    } else {
      TmpStack = TmpStack->Next;
    }
  }
  return EFI_NOT_FOUND;
  
}


//fakesmc-key-CLKT-ui32: Size = 4, Data: 00 00 8C BE
EFI_STATUS EFIAPI SetNvramForTheKey(
                                    IN  SMC_KEY             Key,
                                    IN  SMC_KEY_TYPE        Type,
                                    IN  SMC_DATA_SIZE       Size,
                                    IN  SMC_DATA            *Value
                                    )
{
  EFI_STATUS Status;
  CHAR16 *Name = AllocateCopyPool(44, L"fakesmc-key-CLKT-ui32");
  Name[12] = (Key >> 24) & 0xFF;
  Name[13] = (Key >> 16) & 0xFF;
  Name[14] = (Key >> 8) & 0xFF;
  Name[15] = (Key >> 0) & 0xFF;
  Name[17] = (Type >> 24) & 0xFF;
  Name[18] = (Type >> 16) & 0xFF;
  Name[19] = (Type >> 8) & 0xFF;
  Name[20] = (Type >> 0) & 0xFF;
  if (Name[20] == 0x20) {
    Name[20] = 0;
  }
    
  Status = gRT->SetVariable(Name, &gEfiAppleBootGuid,
                            EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                            Size, (UINT8 *)Value);
  DBG("NVRAM Variable %s set\n", Name);
  FreePool(Name);
  return Status;
}

EFI_STATUS EFIAPI
SmcWriteValueImpl (IN  APPLE_SMC_IO_PROTOCOL  *This,
                   IN  SMC_KEY                Key,
                   IN  SMC_DATA_SIZE          Size,
                   OUT SMC_DATA               *Value
                   )
{  //IN UINT32 DataId, IN UINT32 DataLength, IN VOID* DataBuffer
  SMC_STACK *TmpStack = SmcStack;
  UINTN Len;
  //First find existing key
  while (TmpStack) {
    if (TmpStack->Id == Key) {
      Len = MIN(TmpStack->DataLen, Size);
      CopyMem(TmpStack->Data, Value, Len);
      SetNvramForTheKey(Key, TmpStack->Type, Size, Value);
      return EFI_SUCCESS;
    } else {
      TmpStack = TmpStack->Next;
    }
  }
  //if not found then create new. Not recommended!
  TmpStack = AllocatePool(sizeof(SMC_STACK));
  TmpStack->Next = SmcStack;
  TmpStack->Id = Key;
  TmpStack->DataLen = Size;
  TmpStack->Attributes = SMC_KEY_ATTRIBUTE_WRITE | SMC_KEY_ATTRIBUTE_READ;
  TmpStack->Data = AllocateCopyPool(Size, Value);
  TmpStack->Type = SmcKeyTypeFlag;
  SmcStack = TmpStack;
  return EFI_SUCCESS;
}

// SmcIoSmcGetKeyCountImpl
EFI_STATUS
EFIAPI
SmcGetKeyCountImpl (IN  APPLE_SMC_IO_PROTOCOL  *This,
                    OUT SMC_DATA               *Count
                    )
{
  UINT32 Index = 0;
  SMC_DATA *Big = Count;
  SMC_STACK *TmpStack = SmcStack;
  if (!Count) {
    return EFI_INVALID_PARAMETER;
  }
  while (TmpStack) {
    Index++;
    TmpStack = TmpStack->Next;
  }
  //take into account BigEndian
  *Big++ = (SMC_DATA)(Index >> 24);
  *Big++ = (SMC_DATA)(Index >> 16);
  *Big++ = (SMC_DATA)(Index >> 8);
  *Big++ = (SMC_DATA)Index;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SmcMakeKeyImpl (IN  APPLE_SMC_IO_PROTOCOL  *This,
                IN  CHAR8    *Name,
                OUT SMC_KEY  *Key
                )
{
  if (!Key || !Name) {
    return EFI_INVALID_PARAMETER;
  }
  *Key = SMC_MAKE_IDENTIFIER(Name[0], Name[1], Name[2], Name[3]);
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SmcAddKeyImpl (IN   APPLE_SMC_IO_PROTOCOL  *This,
               IN   SMC_KEY                Key,
               IN   SMC_DATA_SIZE          Size,
               IN   SMC_KEY_TYPE           Type,
               IN   SMC_KEY_ATTRIBUTES     Attributes
               )
{
  SMC_STACK *TmpStack;
  TmpStack = AllocatePool(sizeof(SMC_STACK));
  TmpStack->Next = SmcStack;
  TmpStack->Id = Key;
  TmpStack->DataLen = Size;
  TmpStack->Attributes = Attributes;
  TmpStack->Data = AllocatePool(Size);
  TmpStack->Type = Type;
  SmcStack = TmpStack;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SmcGetKeyFromIndexImpl(IN  APPLE_SMC_IO_PROTOCOL  *This,
                       IN  SMC_INDEX              Index,
                       OUT SMC_KEY                *Key
                       )
{
  UINT32 Num = 0;
  SMC_STACK *TmpStack = SmcStack;
  if (!Key) {
    return EFI_INVALID_PARAMETER;
  }
  while (TmpStack) {
    if (Num == Index) {
      *Key = TmpStack->Id;
      return EFI_SUCCESS;
    }
    Num++;
    TmpStack = TmpStack->Next;
  }
  return EFI_NOT_FOUND;
}

EFI_STATUS
EFIAPI
SmcGetKeyInfoImpl (IN  APPLE_SMC_IO_PROTOCOL  *This,
                   IN  SMC_KEY                Key,
                   OUT SMC_DATA_SIZE          *Size,
                   OUT SMC_KEY_TYPE           *Type,
                   OUT SMC_KEY_ATTRIBUTES     *Attributes
                   )
{
  UINT32 Num = 0;
  SMC_STACK *TmpStack = SmcStack;
  if (!Size || !Type || !Attributes) {
    return EFI_INVALID_PARAMETER;
  }
  while (TmpStack) {
    if (Key == TmpStack->Id) {
      *Size = TmpStack->DataLen;
      *Type = TmpStack->Type;
      *Attributes = TmpStack->Attributes;
      return EFI_SUCCESS;
    }
    Num++;
    TmpStack = TmpStack->Next;
  }
  return EFI_NOT_FOUND;  
}

EFI_STATUS
EFIAPI
SmcResetImpl (IN APPLE_SMC_IO_PROTOCOL  *This,
              IN UINT32                 Mode
              )
{
  SMC_STACK *TmpStack = SmcStack;
  if (Mode) {
    while (TmpStack) {
      FreePool(TmpStack->Data);
      SmcStack = TmpStack->Next;
      FreePool(TmpStack);
      TmpStack = SmcStack;
    }
  }
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SmcFlashTypeImpl(IN APPLE_SMC_IO_PROTOCOL  *This,
                 IN UINT32                 Type
                 )
{
  return EFI_SUCCESS; //EFI_NOT_IMPLEMENTED;
}

EFI_STATUS
EFIAPI
SmcFlashWriteImpl (
                        IN APPLE_SMC_IO_PROTOCOL  *This,
                        IN UINT32                 Unknown,
                        IN UINT32                 Size,
                        IN SMC_DATA               *Data
                        )
{
  return EFI_SUCCESS; //EFI_NOT_IMPLEMENTED;
}

EFI_STATUS
EFIAPI
SmcFlashAuthImpl (
                       IN APPLE_SMC_IO_PROTOCOL  *This,
                       IN UINT32                 Size,
                       IN SMC_DATA               *Data
                       )
{
  return EFI_SUCCESS; //EFI_NOT_IMPLEMENTED;
}

EFI_STATUS
EFIAPI
SmcUnsupportedImpl (IN APPLE_SMC_IO_PROTOCOL  *This
                    )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
SmcUnknown1Impl (IN APPLE_SMC_STATE_PROTOCOL  *This
                 )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SmcUnknown2Impl (IN APPLE_SMC_STATE_PROTOCOL  *This,
                 IN UINTN                  Ukn1,
                 IN UINTN                  Ukn2
                 )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SmcUnknown3Impl (IN APPLE_SMC_STATE_PROTOCOL  *This,
                 IN UINTN                  Ukn1,
                 IN UINTN                  Ukn2
                 )
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SmcUnknown4Impl (IN APPLE_SMC_STATE_PROTOCOL  *This,
                 OUT UINTN                  *Ukn1
                 )
{
  if (Ukn1) {
    *Ukn1 = 0;
  }
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SmcUnknown5Impl (IN APPLE_SMC_STATE_PROTOCOL  *This,
                 IN UINTN                  Ukn1
                 )
{
  return EFI_SUCCESS;
}



// gAppleSmcIoProtocolTemplate
APPLE_SMC_IO_PROTOCOL SMCHelperProtocol = {
  NON_APPLE_SMC_SIGNATURE, //APPLE_SMC_IO_PROTOCOL_REVISION,
  SmcReadValueImpl,
  SmcWriteValueImpl,
  SmcGetKeyCountImpl,
  SmcAddKeyImpl,  //SmcMakeKeyImpl
  SmcGetKeyFromIndexImpl,
  SmcGetKeyInfoImpl,
  SmcResetImpl,
  SmcFlashTypeImpl,
  SmcUnsupportedImpl,
  SmcFlashWriteImpl,
  SmcFlashAuthImpl,
  0,
  SMC_PORT_BASE,
  FALSE,
/*  SmcUnknown1Impl,
  SmcUnknown2Impl,
  SmcUnknown3Impl,
  SmcUnknown4Impl,
  SmcUnknown5Impl */
};

APPLE_SMC_STATE_PROTOCOL SMCStateProtocol = {
  1,            
  SmcUnknown1Impl,
  SmcUnknown2Impl,
  SmcUnknown3Impl,
  SmcUnknown4Impl,
  SmcUnknown5Impl
};




/****************************************************************
 * Entry point
 ***************************************************************/

/**
 * SMCHelper entry point. Installs AppleSMCProtocol.
 */
EFI_STATUS
EFIAPI
SMCHelperEntrypoint (
                    IN EFI_HANDLE           ImageHandle,
                    IN EFI_SYSTEM_TABLE		*SystemTable
                    )
{
  EFI_STATUS					Status; // = EFI_SUCCESS; 
  
 // gBS				= SystemTable->BootServices;
  
  Status = gBS->InstallMultipleProtocolInterfaces (
                &mHandle,
                &gAppleSMCProtocolGuid,
                &SMCHelperProtocol,
                &gAppleSMCStateProtocolGuid,
                &SMCStateProtocol,
                NULL
                );
  
  return Status;
}
