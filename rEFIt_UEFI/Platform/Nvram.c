/**
 *  Module for work with runtime (RT, NVRAM) vars,
 *  determining default boot volume (Startup disk)
 *  and (kid of) persistent RT support with nvram.plist on CloverEFI.
 */

#include "Platform.h"

#ifndef DEBUG_ALL
#define DEBUG_SET 1
#else
#define DEBUG_SET DEBUG_ALL
#endif

#if DEBUG_SET == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_SET, __VA_ARGS__)
#endif



// for saving nvram.plist and it's data
TagPtr                          gNvramDict;
EFI_GUID                        *gEfiBootDeviceGuid = NULL;



/** returns given time as miliseconds.
 *  assumes 31 days per month, so it's not correct,
 *  but is enough for basic checks.
 */
UINT64 GetEfiTimeInMs(IN EFI_TIME *T)
{
    UINT64              TimeMs;
    
    TimeMs = T->Year - 1900;
    // is 64bit multiply workign in 32 bit?
    TimeMs = MultU64x32(TimeMs, 12) + T->Month;
    TimeMs = MultU64x32(TimeMs, 31) + T->Day; // counting with 31 day
    TimeMs = MultU64x32(TimeMs, 24) + T->Hour;
    TimeMs = MultU64x32(TimeMs, 60) + T->Minute;
    TimeMs = MultU64x32(TimeMs, 60) + T->Second;
    TimeMs = MultU64x32(TimeMs, 1000) + DivU64x32(T->Nanosecond, 1000000);
    
    return TimeMs;
}


/** Reads and returns value of NVRAM variable. */
VOID *GetNvramVariable(IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid, OUT UINTN *DataSize OPTIONAL)
{
    EFI_STATUS      Status;
    VOID            *Data = NULL;
    UINTN           IntDataSize = 0;
    
    Status = gRT->GetVariable(VariableName, VendorGuid, NULL, &IntDataSize, NULL);
    if (Status == EFI_BUFFER_TOO_SMALL) {
        Data = AllocateZeroPool(IntDataSize);
        if (Data) {
            Status = gRT->GetVariable(VariableName, VendorGuid, NULL, &IntDataSize, Data);
            if (EFI_ERROR(Status)) {
                FreePool(Data);
                Data = NULL;
            } else {
        if (DataSize != NULL) {
          *DataSize = IntDataSize;
        }
      }
        }
    }
    return Data;
}


/** Searches for GPT HDD dev path node and return pointer to partition GUID or NULL. */
EFI_GUID *FindGPTPartitionGuidInDevicePath(IN EFI_DEVICE_PATH_PROTOCOL *DevicePath)
{
    HARDDRIVE_DEVICE_PATH   *HDDDevPath;
    EFI_GUID                *Guid = NULL;
    
    if (DevicePath == NULL) {
        return NULL;
    }
    
    while (!IsDevicePathEndType(DevicePath)
           &&
           !(DevicePathType(DevicePath) == MEDIA_DEVICE_PATH && DevicePathSubType(DevicePath) == MEDIA_HARDDRIVE_DP)
           )
    {
        DevicePath = NextDevicePathNode(DevicePath);
    }
    
    if (DevicePathType(DevicePath) == MEDIA_DEVICE_PATH
        && DevicePathSubType(DevicePath) == MEDIA_HARDDRIVE_DP)
    {
        HDDDevPath = (HARDDRIVE_DEVICE_PATH*)DevicePath;
        if (HDDDevPath->SignatureType == SIGNATURE_TYPE_GUID) {
            Guid = (EFI_GUID*)HDDDevPath->Signature;
        }
    }
    return Guid;
}


/** Reads gEfiAppleBootGuid:efi-boot-device-data NVRAM variable and extracts volume UUID gEfiBootDeviceGuid. */
EFI_STATUS GetEfiBootDeviceFromNvram(VOID)
{
    EFI_STATUS      Status;
    UINTN           Size;
    EFI_DEVICE_PATH_PROTOCOL    *EfiBootDeviceData;
    EFI_GUID        *Guid;

    Status = EFI_NOT_FOUND;
    
    DBG("NVRAM efi-boot-device-data: ");
    EfiBootDeviceData = GetNvramVariable(L"efi-boot-device-data", &gEfiAppleBootGuid, &Size);
    if (EfiBootDeviceData != NULL) {
        DBG("%s\n", DevicePathToStr(EfiBootDeviceData));
        Guid = FindGPTPartitionGuidInDevicePath(EfiBootDeviceData);
        if (Guid != NULL) {
            gEfiBootDeviceGuid = AllocatePool(sizeof(EFI_GUID));
            if (gEfiBootDeviceGuid != NULL) {
                CopyMem(gEfiBootDeviceGuid, Guid, sizeof(EFI_GUID));
            }
        }
        if (gEfiBootDeviceGuid != NULL) {
            DBG(" Guid = %g\n", gEfiBootDeviceGuid);
            Status = EFI_SUCCESS;
        } else {
            DBG(" Guid not found\n");
        }
    } else {
        DBG("not found\n");
    }
    
    return Status;
}


/** Loads and parses nvram.plist into gNvramDict. */
EFI_STATUS LoadNvramPlist(IN EFI_FILE *RootDir, IN CHAR16* NVRAMPlistPath)
{
    EFI_STATUS      Status;
    CHAR8           *NvramPtr;
    UINTN           Size;
    
    
    //
    // skip loading if already loaded
    //
    if (gNvramDict != NULL) {
        return EFI_SUCCESS;
    }
    
    //
    // load nvram.plist
    //
    Status = egLoadFile(RootDir, NVRAMPlistPath, (UINT8**)&NvramPtr, &Size);
    if(EFI_ERROR(Status)) {
        DBG(" not present\n");
        return Status;
    }
    DBG(" loaded, size=%d\n", Size);
    
    //
    // parse it into gNvramDict 
    //
    Status = ParseXML((const CHAR8*)NvramPtr, &gNvramDict);
    if(Status != EFI_SUCCESS) {
        DBG(" parsing error\n");
    }
    
    FreePool(NvramPtr);
    // we will leave nvram.plist loaded and parsed for later processing
    //FreeTag(gNvramDict);
    
    return Status;
}


/** Searches all volumes for the most recent nvram.plist and loads it into gNvramDict. */
EFI_STATUS LoadLatestNvramPlist(VOID)
{
    EFI_STATUS          Status;
    INTN               Index;
    REFIT_VOLUME        *Volume;
    EFI_GUID            *Guid;
    EFI_FILE_HANDLE     FileHandle;
    EFI_FILE_INFO       *FileInfo;
    UINT64              LastModifTimeMs;
    UINT64              ModifTimeMs;
    REFIT_VOLUME        *VolumeWithLatestNvramPlist;
    
    
    DBG("Searching volumes for latest nvram.plist ...");

    //
    // skip loading if already loaded
    //
    if (gNvramDict != NULL) {
        DBG(" already loaded\n");
        return EFI_SUCCESS;
    }
    DBG("\n");
    
    //
    // find latest nvram.plist
    //
    
    LastModifTimeMs = 0;
    VolumeWithLatestNvramPlist = NULL;
    
    // search all volumes
    for (Index = 0; Index < VolumesCount; Index++) {
        Volume = Volumes[Index];
        
        if (!Volume->RootDir) {
            continue;
        }
        
        Guid = FindGPTPartitionGuidInDevicePath(Volume->DevicePath);
        
        DBG(" %2d. Volume '%s', GUID = %g", Index, Volume->VolName, Guid);
        if (Guid == NULL) {
            // not a GUID partition
            DBG(" - not GPT");
        }
        
        // check if nvram.plist exists
        Status = Volume->RootDir->Open(Volume->RootDir, &FileHandle, L"nvram.plist", EFI_FILE_MODE_READ, 0);
        if (EFI_ERROR(Status)) {
            DBG(" - no nvram.plist - skipping!\n");
            continue;
        }
        
        // get nvram.plist modification date
        FileInfo = EfiLibFileInfo(FileHandle);
        if (FileInfo == NULL) {
            DBG(" - no nvram.plist file info - skipping!\n");
            FileHandle->Close(FileHandle);
            continue;
        }
        
        DBG(" Modified = ");
        ModifTimeMs = GetEfiTimeInMs(&FileInfo->ModificationTime);
        DBG("%d-%d-%d %d:%d:%d (%ld ms)",
            FileInfo->ModificationTime.Year, FileInfo->ModificationTime.Month, FileInfo->ModificationTime.Day,
            FileInfo->ModificationTime.Hour, FileInfo->ModificationTime.Minute, FileInfo->ModificationTime.Second,
            ModifTimeMs);
        FreePool(FileInfo);
        FileHandle->Close(FileHandle);
        
        // check if newer
        if (LastModifTimeMs < ModifTimeMs) {
            
            DBG(" - newer - will use this one\n");
            VolumeWithLatestNvramPlist = Volume;
            LastModifTimeMs = ModifTimeMs;
            
        } else {
            DBG(" - older - skipping!\n");
        }
    }
    
    Status = EFI_NOT_FOUND;
    
    //
    // if we have nvram.plist - load it
    //
    if (VolumeWithLatestNvramPlist != NULL) {
        
        DBG("Loading nvram.plist from Vol '%s' -", VolumeWithLatestNvramPlist->VolName);
        Status = LoadNvramPlist(VolumeWithLatestNvramPlist->RootDir, L"nvram.plist");
        
    } else {
        DBG(" nvram.plist not found!\n");
    }
    
    return Status;
}


/** Puts all vars from nvram.plist to RT vars. Should be used in CloverEFI only
 *  or if some UEFI boot uses EmuRuntimeDxe driver.
 */
VOID PutNvramPlistToRtVars(VOID)
{
    EFI_STATUS      Status;
    TagPtr          Tag;
    TagPtr          ValTag;
    INTN            Size;
    CHAR16          KeyBuf[128];
    VOID            *Value;
    

    if (gNvramDict == NULL) {
        Status = LoadLatestNvramPlist();
        if (gNvramDict == NULL) {
            DBG("PutNvramPlistToRtVars: nvram.plist not found\n");
            return;
        }
    }
    
    DBG("PutNvramPlistToRtVars ...\n");
    // iterate over dict elements
    for (Tag = gNvramDict->tag; Tag != NULL; Tag = Tag->tagNext) {
        
        Value = NULL;
        ValTag = (TagPtr)Tag->tag;
        
        // process only valid <key> tags
        if (Tag->type != kTagTypeKey || ValTag == NULL) {
            DBG(" ERROR: Tag is not <key>, type = %d\n", Tag->type);
            continue;
        }
        
        // key to unicode; check if key buffer is large enough
        if (AsciiStrLen(Tag->string) > (sizeof(KeyBuf) / 2 - 1)) {
            DBG(" ERROR: Skipping too large key %s\n", Tag->string);
            continue;
        }
        AsciiStrToUnicodeStr(Tag->string, KeyBuf);
        DBG(" Adding Key: %s: ", KeyBuf);
        
        // process value tag
        
        if (ValTag->type == kTagTypeString) {
            
            // <string> element
            Value = ValTag->string;
            Size = AsciiStrLen(Value);
            DBG("String: Size = %d, Val = '%a'", Size, Value);
            
        } else if (ValTag->type == kTagTypeData) {
            
            // <data> element
            Size = ValTag->dataLen;
            Value = ValTag->data;
            DBG("Data: Size = %d", Size);
            
        } else {
            DBG("ERROR: Unsupported tag type: %d\n", ValTag->type);
            continue;
        }
        
        // set RT var: all vars visible in nvram.plist are gEfiAppleBootGuid
        Status = gRS->SetVariable(KeyBuf,
                                  &gEfiAppleBootGuid,
                                  /*    EFI_VARIABLE_NON_VOLATILE | */EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                                  Size,
                                  Value
                                  );
        DBG(": %r\n", Status);
    }
}


/** Searches for Startup Disk boot volume by looking for gEfiAppleBootGuid:efi-boot-device-data RT var.
 *  Returns matching volume or NULL.
 */
REFIT_VOLUME* FindStartupDiskVolume(VOID)
{
    EFI_STATUS          Status;
    INTN                Index;
    REFIT_VOLUME        *Volume;
    EFI_GUID            *Guid;
    
    
    DBG("FindStartupDiskVolume ...\n");
    
    //
    // search RT vars for efi-boot-device
    // and get volume GUID into gEfiBootDeviceGuid
    //
    // TODO: add Win volume detection
    //
    Status = GetEfiBootDeviceFromNvram();
    
    if (gEfiBootDeviceGuid != NULL) {
        
        DBG(" searching for volume %g\n", gEfiBootDeviceGuid);
        // find menu entry with GPT volume with gEfiBootDeviceGuid
        for (Index = 0; Index < VolumesCount; Index++) {
            Volume = Volumes[Index];
            
            Guid = FindGPTPartitionGuidInDevicePath(Volume->DevicePath);
            
            if (Guid && CompareGuid(Guid, gEfiBootDeviceGuid)) {
                // that's the one
                DBG(" found volume %d. '%s', GUID = %g\n", Index, Volume->VolName, Guid);
                return Volume;
            }
            
        }
    }
    DBG(" not found\n");
    return NULL;
}
