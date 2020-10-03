//
//  OcQuirksProtocol.h
//  Clover
//
//  Created by Sergey on 14/07/2020.
//  Copyright © 2020 Slice. All rights reserved.
//

#ifndef OcQuirksProtocol4Clover_h
#define OcQuirksProtocol4Clover_h

extern "C" {
#include <Library/OcTemplateLib.h>
#include <Library/OcAfterBootCompatLib4Clover.h>
}


#define OCQUIRKS_PROTOCOL_REVISION  23
/*
#define OC_MMIO_WL_STRUCT_FIELDS(_, __) \
_(BOOLEAN   , Enabled , , FALSE , ()) \
_(UINT64    , Address , , 0     , ()) \
_(OC_STRING , Comment , , OC_STRING_CONSTR ("", _, __), OC_DESTR (OC_STRING))
OC_DECLARE (OC_MMIO_WL_STRUCT)

#define OC_MMIO_WL_ARRAY_FIELDS(_, __) \
OC_ARRAY (OC_MMIO_WL_STRUCT, _, __)
OC_DECLARE (OC_MMIO_WL_ARRAY)

#define OC_QUIRKS_FIELDS(_, __) \
_(BOOLEAN , AvoidRuntimeDefrag      ,   , TRUE  ,()) \
_(BOOLEAN , DevirtualiseMmio        ,   , FALSE ,()) \
_(BOOLEAN , DisableSingleUser       ,   , FALSE ,()) \
_(BOOLEAN , DisableVariableWrite    ,   , FALSE ,()) \
_(BOOLEAN , DiscardHibernateMap     ,   , FALSE ,()) \
_(BOOLEAN , EnableSafeModeSlide     ,   , TRUE  ,()) \
_(BOOLEAN , EnableWriteUnprotector  ,   , TRUE  ,()) \
_(BOOLEAN , ForceExitBootServices   ,   , FALSE ,()) \
_(OC_MMIO_WL_ARRAY , MmioWhitelist  ,   , OC_CONSTR2 (OC_MMIO_WL_ARRAY, _, __) , OC_DESTR (OC_MMIO_WL_ARRAY)) \
_(BOOLEAN , ProtectMemoryRegions    ,   , FALSE ,()) \
_(BOOLEAN , ProtectSecureBoot       ,   , FALSE ,()) \
_(BOOLEAN , ProtectUefiServices     ,   , FALSE ,()) \
_(BOOLEAN , ProvideConsoleGopEnable ,   , TRUE  ,()) \
_(UINT8   , ProvideMaxSlide         ,   , 0     ,()) \
_(BOOLEAN , ProvideCustomSlide      ,   , TRUE  ,()) \
_(BOOLEAN , RebuildAppleMemoryMap   ,   , FALSE ,()) \
_(BOOLEAN , SetupVirtualMap         ,   , TRUE  ,()) \
_(BOOLEAN , SignalAppleOS           ,   , FALSE ,()) \
_(BOOLEAN , SyncRuntimePermissions  ,   , TRUE  ,())

OC_DECLARE (OC_QUIRKS)

OC_STRUCTORS        (OC_MMIO_WL_STRUCT, ())
OC_ARRAY_STRUCTORS  (OC_MMIO_WL_ARRAY)
OC_STRUCTORS        (OC_QUIRKS, ())
*/
#define OCQUIRKS_PROTOCOL_GUID \
{ \
  0x511CE020, 0x0020, 0x0714, {0x20, 0x12, 0x17, 0x38, 0x05, 0x01, 0x02, 0x04} \
}

typedef struct OCQUIRKS_PROTOCOL_ OCQUIRKS_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *OCQUIRKS_GET_CONFIG) (
                               IN  OCQUIRKS_PROTOCOL  *This,
                               OUT OC_ABC_SETTINGS_4CLOVER    *Buffer,
                               OUT BOOLEAN            *GopEnable
                               );


struct OCQUIRKS_PROTOCOL_ {
  UINT32                  Revision;     ///< The revision of the installed protocol.
  UINTN                   Reserved;     ///< Reserved for future extension.
  OCQUIRKS_GET_CONFIG     GetConfig;    
};

extern EFI_GUID gOcQuirksProtocolGuid;

#endif /* OcQuirksProtocol_h */
