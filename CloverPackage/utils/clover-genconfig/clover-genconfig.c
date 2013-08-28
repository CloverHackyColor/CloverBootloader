/*
   This is secret service for Clover to obtain temporary setting that user made in Options menu.
   It will work with Clover rev 1680+.
   It will not work for other bootloaders.

    (c) Slice 2013
 
   Code portion from Apple's project nvram
*/
/*
 * Copyright (c) 2000-2005 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 *
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */
/*
cc -o genconfig genconfig.c -framework CoreFoundation -framework IOKit -Wall -Wno-unused-function
*/

// EDK2 includes
//#include <base.h>

#define __DEBUG_LIB_H__
#define  _STRUCT_X86_THREAD_STATE32
#define  _STRUCT_X86_THREAD_STATE64
#include "../../../rEFIt_UEFI/Platform/Platform.h"

#include <IOKit/IOKitLib.h>
#include <IOKit/IOKitKeys.h>
#include <CoreFoundation/CoreFoundation.h>

#include <err.h>
#include <mach/mach_error.h>

// Prototypes
static kern_return_t GetOFVariable(const char *name, CFTypeRef *valueRef);

// Global Variables
static io_registry_entry_t gPlatform;
static mach_port_t         masterPort;

CFMutableDictionaryRef addDict(CFMutableDictionaryRef dest, CFStringRef key) {
    assert(dest);
    CFMutableDictionaryRef dict = CFDictionaryCreateMutable (
                                                             kCFAllocatorDefault,
                                                             0,
                                                             &kCFTypeDictionaryKeyCallBacks,
                                                             &kCFTypeDictionaryValueCallBacks
                                                             );
    if (!dict)
        errx(1,"Error can't allocate dictionnary for key '%s'",
             CFStringGetCStringPtr( key, kCFStringEncodingMacRoman ));

    CFDictionaryAddValue( dest, key, dict );
    return dict;
}

void addString(CFMutableDictionaryRef dest, CFStringRef key, const char* value) {
    assert(dest);
    CFStringRef strValue = CFStringCreateWithCString(kCFAllocatorDefault,
                                                     value,
                                                     kCFStringEncodingMacRoman);
    assert(strValue);
    CFDictionaryAddValue( dest, key, strValue );
    CFRelease(strValue);
}

void addUString(CFMutableDictionaryRef dest, CFStringRef key, const UniChar* value) {
    assert(dest);
    CFStringRef strValue = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%S"), value);
    assert(strValue);
    CFDictionaryAddValue( dest, key, strValue );
    CFRelease(strValue);
}

void addHex(CFMutableDictionaryRef dest, CFStringRef key, UInt64 value) {
    assert(dest);
    CFStringRef strValue = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("0x%llx"), value);
    assert(strValue);
    CFDictionaryAddValue( dest, key, strValue );
    CFRelease(strValue);
}

void addBoolean(CFMutableDictionaryRef dest, CFStringRef key, Boolean value) {
    assert(dest);
    CFBooleanRef boolValue = value ? kCFBooleanTrue : kCFBooleanFalse;
    CFDictionaryAddValue( dest, key, boolValue );
}

void addInteger(CFMutableDictionaryRef dest, CFStringRef key, UInt64 value) {
    assert(dest);
    CFNumberRef valueRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &value);
    assert(valueRef);
    CFDictionaryAddValue( dest, key, valueRef );
    CFRelease(valueRef);
}

void addUUID(CFMutableDictionaryRef dest, CFStringRef key, EFI_GUID *uuid)
{
    assert(dest);
    CFStringRef strValue = CFStringCreateWithFormat(kCFAllocatorDefault, NULL,
                                                    CFSTR("%08tx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x"),
                                                    uuid->Data1, uuid->Data2, uuid->Data3,
                                                    uuid->Data4[0], uuid->Data4[1],
                                                    uuid->Data4[2], uuid->Data4[3], uuid->Data4[4], uuid->Data4[5], uuid->Data4[6], uuid->Data4[7]);
    assert(strValue);
    CFDictionaryAddValue( dest, key, strValue );
    CFRelease(strValue);
}

void addIntArray(CFMutableDictionaryRef dest, CFStringRef key, UInt8 *Value, SInt64 num)
{
    SInt64 i = 0;
    CFMutableStringRef strValue = CFStringCreateMutable (kCFAllocatorDefault, 0);

    for (i = 0; i < num; i++) {
        CFStringAppendFormat(strValue, NULL, CFSTR("%02x"), Value[i]);
    }
    CFDictionaryAddValue( dest, key, strValue );
    CFRelease(strValue);
}

void dump_plist(CFMutableDictionaryRef properties) {
    CFWriteStreamRef stdoutStream = NULL;
    CFURLRef devStdout = CFURLCreateWithFileSystemPath(
                                                       NULL,
                                                       CFSTR("/dev/stdout"),
                                                       kCFURLPOSIXPathStyle,
                                                       false
                                                       );
    stdoutStream = CFWriteStreamCreateWithFile(NULL, devStdout);
    CFRelease(devStdout);
    if (stdoutStream == NULL)
        errx(1,"cannot create CFWriteStream for /dev/stdout");
    if (!CFWriteStreamOpen(stdoutStream))
        errx(1,"cannot open CFWriteStream for /dev/stdout");

    CFPropertyListWrite(
                        properties,
                        stdoutStream,
                        kCFPropertyListXMLFormat_v1_0,
                        0,
                        NULL
                        );
    CFWriteStreamClose(stdoutStream);
    CFRelease(stdoutStream);
}


// GetOFVariable(name, valueRef)
//
//   Get the named firmware variable.
//   Return the value in valueRef.
//
static kern_return_t GetOFVariable(const char *name, CFTypeRef *valueRef)
{
    CFStringRef nameRef = CFStringCreateWithCString(kCFAllocatorDefault, name,
                                                    kCFStringEncodingUTF8);
    if (!nameRef) {
        errx(1, "Error creating CFString for key %s", name);
    }

    *valueRef = IORegistryEntryCreateCFProperty(gPlatform, nameRef, 0, 0);
    CFRelease(nameRef);
    if (!*valueRef) {
        printf("value not found\n");
        return kIOReturnNotFound;
    }

    return KERN_SUCCESS;
}

void PrintConfig(CFTypeRef data)
{
    const Byte *dataPtr = NULL;
    CFIndex    length = 0;
    CFTypeID   typeID;

    // Get the OF variable's type.
    typeID = CFGetTypeID(data);

    if (typeID == CFDataGetTypeID()) {
        length = CFDataGetLength(data);
        if (length == 0)
            return;
        else
            dataPtr = CFDataGetBytePtr(data);
    } else {
        printf("<INVALID> settings\n");
        return;
    }

    if (length != sizeof(SETTINGS_DATA)) {
        errx(1, "Error the version of clover-genconfig didn't match current booted clover version");
    }

    SETTINGS_DATA *s = (SETTINGS_DATA*)dataPtr;

    CFMutableDictionaryRef dict = CFDictionaryCreateMutable (
                                                             kCFAllocatorDefault,
                                                             0,
                                                             &kCFTypeDictionaryKeyCallBacks,
                                                             &kCFTypeDictionaryValueCallBacks
                                                             );

    // SystemParameters
    CFMutableDictionaryRef systemParametersDict = addDict(dict, CFSTR("SystemParameters"));
    addString(systemParametersDict, CFSTR("boot-args"), s->BootArgs);
    addString(systemParametersDict, CFSTR("prev-lang:kbd"), s->Language);
    addUString(systemParametersDict, CFSTR("CustomUUID"), s->CustomUuid);
    addBoolean(systemParametersDict, CFSTR("InjectSystemID"), s->InjectSystemID);
    addBoolean(systemParametersDict, CFSTR("NoCaches"), s->NoCaches);
    addBoolean(systemParametersDict, CFSTR("InjectKexts"), s->WithKexts);
    addHex(systemParametersDict, CFSTR("BacklightLevel"),s->BacklightLevel);
    addUString(systemParametersDict, CFSTR("LegacyBoot"), s->LegacyBoot);
    addUString(systemParametersDict, CFSTR("ConfigName"), s->ConfigName);
    addInteger(systemParametersDict, CFSTR("XMPDetection"), s->XMPDetection);

    // GUI
    CFMutableDictionaryRef guiDict = addDict(dict, CFSTR("GUI"));
    addUString(guiDict,CFSTR("DefaultBootVolume"), s->DefaultBoot);
    addBoolean(guiDict,CFSTR("DebugLog"), s->Debug);

    CFMutableDictionaryRef mouseDict = addDict(guiDict, CFSTR("Mouse"));
    addBoolean(mouseDict, CFSTR("Enabled"), s->PointerEnabled);
    addInteger(mouseDict, CFSTR("Speed"), s->PointerSpeed);
    addInteger(mouseDict, CFSTR("DoubleClick"), s->DoubleClickTime);
    addBoolean(mouseDict, CFSTR("Mirror"), s->PointerMirror);

    CFMutableDictionaryRef volumesDict = addDict(guiDict, CFSTR("Volumes"));
    addInteger(volumesDict, CFSTR("Hide Count"), s->HVCount);

    CFMutableDictionaryRef hideEntriesDict = addDict(guiDict, CFSTR("HideEntries"));
    addBoolean(hideEntriesDict, CFSTR("OSXInstall"), s->HVHideAllOSXInstall);
    addBoolean(hideEntriesDict, CFSTR("Recovery"), s->HVHideAllRecovery);
    addBoolean(hideEntriesDict, CFSTR("Duplicate"), s->HVHideDuplicatedBootTarget);
    addBoolean(hideEntriesDict, CFSTR("WindowsEFI"), s->HVHideAllWindowsEFI);
    addBoolean(hideEntriesDict, CFSTR("Ubuntu"), s->HVHideAllUbuntu);
    addBoolean(hideEntriesDict, CFSTR("Grub"), s->HVHideAllGrub);
    addBoolean(hideEntriesDict, CFSTR("Gentoo"), s->HVHideAllGentoo);
    addBoolean(hideEntriesDict, CFSTR("OpticalUEFI"), s->HVHideOpticalUEFI);
    addBoolean(hideEntriesDict, CFSTR("InternalUEFI"), s->HVHideInternalUEFI);
    addBoolean(hideEntriesDict, CFSTR("ExternalUEFI"), s->HVHideExternalUEFI);

    // SMBIOS
    CFMutableDictionaryRef smbiosDict = addDict(dict, CFSTR("SMBIOS"));
    // SMBIOS TYPE0
    addString(smbiosDict, CFSTR("BiosVendor"), s->VendorName);
    addString(smbiosDict, CFSTR("BiosVersion"), s->RomVersion);
    addString(smbiosDict, CFSTR("BiosReleaseDate"), s->ReleaseDate);
    // SMBIOS TYPE1
    addString(smbiosDict, CFSTR("Manufacturer"), s->ManufactureName);
    addString(smbiosDict, CFSTR("ProductName"), s->ProductName);
    addString(smbiosDict, CFSTR("Version"), s->VersionNr);
    addString(smbiosDict, CFSTR("SerialNumber"), s->SerialNr);

    addUUID(smbiosDict, CFSTR("SmUUID"), &s->SmUUID);
    addString(smbiosDict, CFSTR("Family"), s->FamilyName);
    // SMBIOS TYPE2
    addString(smbiosDict, CFSTR("BoardManufacturer"), s->BoardManufactureName);
    addString(smbiosDict, CFSTR("BoardSerialNumber"), s->BoardSerialNumber);
    addString(smbiosDict, CFSTR("Board-ID"), s->BoardNumber);
    addString(smbiosDict, CFSTR("BoardVersion"), s->BoardVersion);
    addInteger(smbiosDict, CFSTR("BoardType"), s->BoardType);
    addString(smbiosDict, CFSTR("LocationInChassis"), s->LocationInChassis);
    addString(smbiosDict, CFSTR("ChassisManufacturer"), s->ChassisManufacturer);
    addString(smbiosDict, CFSTR("ChassisAssetTag"), s->ChassisAssetTag);
    addHex(smbiosDict, CFSTR("ChassisType"), s->ChassisType);
    addBoolean(smbiosDict, CFSTR("Mobile"), s->Mobile);
    // SMBIOS TYPE17
    addBoolean(smbiosDict, CFSTR("Trust"), s->TrustSMBIOS);

    addString(smbiosDict, CFSTR("OEMProduct"), s->OEMProduct);
    addString(smbiosDict, CFSTR("OEMVendor"), s->OEMVendor);
    addString(smbiosDict, CFSTR("OEMBoard"), s->OEMBoard);

    if (s->InjectMemoryTables) {
        CFMutableDictionaryRef memoryDict = addDict(smbiosDict, CFSTR("Memory"));
        // there are wrong keys with some values
        addString(memoryDict, CFSTR("MemoryManufacturer"), s->MemoryManufacturer);
        addString(memoryDict, CFSTR("MemorySerialNumber"), s->MemorySerialNumber);
        addString(memoryDict, CFSTR("MemoryPartNumber"), s->MemoryPartNumber);
        addString(memoryDict, CFSTR("MemorySpeed"), s->MemorySpeed);
    }

    // CPU
    CFMutableDictionaryRef cpuDict = addDict(dict, CFSTR("CPU"));
    addBoolean(cpuDict, CFSTR("Turbo"), s->Turbo);
    addInteger(cpuDict, CFSTR("CpuFrequencyMHz"), s->CpuFreqMHz);
    addInteger(cpuDict, CFSTR("BusSpeedkHz"), s->BusSpeed);
    addInteger(cpuDict, CFSTR("QPI"), s->QPI);
    // these values read only
    addInteger(cpuDict, CFSTR("EnabledCores"), s->EnabledCores);

    // PCI
    CFMutableDictionaryRef pciDict = addDict(dict, CFSTR("PCI"));
    addBoolean(pciDict, CFSTR("StringInjector"), s->StringInjector);
    addString(pciDict, CFSTR("DeviceProperties"), "_NOT_SHOWN_");
    addInteger(pciDict, CFSTR("PCIRootUID"), s->PCIRootUID);
    if (s->HDAInjection)
        addInteger(pciDict, CFSTR("HDAInjection"), s->HDALayoutId);
    else
        addBoolean(pciDict, CFSTR("HDAInjection"), s->HDAInjection);
    addBoolean(pciDict, CFSTR("USBInjection"), s->USBInjection);
    addBoolean(pciDict, CFSTR("USBFixOwnership"), s->USBFixOwnership);
    addBoolean(pciDict, CFSTR("InjectClockID"), s->InjectClockID);
    addBoolean(pciDict, CFSTR("LpcTune"), s->LpcTune);
  CFMutableDictionaryRef fakeIDDict = addDict(pciDict, CFSTR("FakeID"));
  addHex(fakeIDDict, CFSTR("ATI"), s->FakeATI);
  addHex(fakeIDDict, CFSTR("NVidia"), s->FakeNVidia);
  addHex(fakeIDDict, CFSTR("IntelGFX"), s->FakeIntel);
  addHex(fakeIDDict, CFSTR("LAN"), s->FakeLAN);
  addHex(fakeIDDict, CFSTR("WIFI"), s->FakeWIFI);
  addHex(fakeIDDict, CFSTR("SATA"), s->FakeSATA);
  addHex(fakeIDDict, CFSTR("XHCI"), s->FakeXHCI);
  

    // Graphics
    CFMutableDictionaryRef graphicsDict = addDict(dict, CFSTR("Graphics"));
    addBoolean(graphicsDict, CFSTR("GraphicsInjector"), s->GraphicsInjector);
    addBoolean(graphicsDict, CFSTR("InjectATI"), s->InjectATI);
    addBoolean(graphicsDict, CFSTR("InjectNVidia"), s->InjectNVidia);
    addBoolean(graphicsDict, CFSTR("InjectIntel"), s->InjectIntel);
    addBoolean(graphicsDict, CFSTR("LoadVBios"), s->LoadVBios);
    addBoolean(graphicsDict, CFSTR("InjectEDID"), s->InjectEDID);
    addString(graphicsDict, CFSTR("CustomEDID"), "_NOT_SHOWN_");
    addBoolean(graphicsDict, CFSTR("PatchVBios"), s->PatchVBios);
    addInteger(graphicsDict, CFSTR("PatchVBios Manual Count"), s->PatchVBiosBytesCount);
    addInteger(graphicsDict, CFSTR("VideoPorts"), s->VideoPorts);
    addInteger(graphicsDict, CFSTR("VRAM"), s->VRAM);
    addInteger(graphicsDict, CFSTR("DualLink"), s->DualLink);
    // ATI specific"
    addUString(graphicsDict, CFSTR("FBName"), s->FBName);
    // NVIDIA specific
    addIntArray(graphicsDict, CFSTR("display-cfg"), &s->Dcfg[0], 8);
    addIntArray(graphicsDict, CFSTR("NVCAP"), &s->NVCAP[0], 20);
    // INTEL specific
    addHex(graphicsDict, CFSTR("ig-platform-id"), s->IgPlatform);

    //ACPI
    CFMutableDictionaryRef acpiDict = addDict(dict, CFSTR("ACPI"));
    addUString(acpiDict, CFSTR("DsdtName"), s->DsdtName);
    addHex(acpiDict, CFSTR("FixDsdtMask"), s->FixDsdt);
    addBoolean(acpiDict, CFSTR("DropOemSSDT"), s->DropSSDT);
  addInteger(acpiDict, CFSTR("Number_of_KeepSSDT"), s->KeepSsdtNum);
    addBoolean(acpiDict, CFSTR("DropAPIC"), s->bDropAPIC);
    addBoolean(acpiDict, CFSTR("PatchAPIC"), s->PatchNMI);
    addBoolean(acpiDict, CFSTR("DropMCFG"), s->bDropMCFG);
    addBoolean(acpiDict, CFSTR("DropHPET"), s->bDropHPET);
    addBoolean(acpiDict, CFSTR("DropECDT"), s->bDropECDT);
    addBoolean(acpiDict, CFSTR("DropDMAR"), s->bDropDMAR);
    addBoolean(acpiDict, CFSTR("DropBGRT"), s->bDropBGRT);
    addBoolean(acpiDict, CFSTR("GeneratePStates"), s->GeneratePStates);
    addBoolean(acpiDict, CFSTR("GenerateCStates"), s->GenerateCStates);
    addBoolean(acpiDict, CFSTR("DoubleFirstState"), s->DoubleFirstState);
    addBoolean(acpiDict, CFSTR("EnableC2"), s->EnableC2);
    addHex(acpiDict, CFSTR("C3Latency"), s->C3Latency);
    addBoolean(acpiDict, CFSTR("EnableC4"), s->EnableC4);
    addBoolean(acpiDict, CFSTR("EnableC6"), s->EnableC6);
    addBoolean(acpiDict, CFSTR("EnableISS"), s->EnableISS);
    addInteger(acpiDict, CFSTR("PLimitDict"), s->PLimitDict);
    addInteger(acpiDict, CFSTR("UnderVoltStep"), s->UnderVoltStep);
    addInteger(acpiDict, CFSTR("MinMultiplier"), s->MinMultiplier);
    addInteger(acpiDict, CFSTR("MaxMultiplier"), s->MaxMultiplier);
    addInteger(acpiDict, CFSTR("PluginType"), s->PluginType);
    addBoolean(acpiDict, CFSTR("smartUPS"), s->smartUPS);
    addHex(acpiDict, CFSTR("ResetAddress"), s->ResetAddr);
    addHex(acpiDict, CFSTR("ResetValue"), s->ResetVal);

    // KernelAndKextPatches
    CFMutableDictionaryRef KernelAndKextPatchesDict = addDict(dict, CFSTR("KernelAndKextPatches"));
    addBoolean(KernelAndKextPatchesDict, CFSTR("Debug"), s->KPDebug);
    addBoolean(KernelAndKextPatchesDict, CFSTR("KernelCpu"), s->KPKernelCpu);
    addBoolean(KernelAndKextPatchesDict, CFSTR("KernelLapic"), s->KPLapicPanic);
    addBoolean(KernelAndKextPatchesDict, CFSTR("AppleRTC"), s->KPAppleRTC);
    addBoolean(KernelAndKextPatchesDict, CFSTR("AsusAICPUPM"), s->KPAsusAICPUPM);
    addBoolean(KernelAndKextPatchesDict, CFSTR("KextPatchesAllowed"), s->KextPatchesAllowed);
    addInteger(KernelAndKextPatchesDict, CFSTR("Number_of_KextsToPatch"), s->NrKexts);

    //TODO
    // here we can get LogEveryBoot and MountEFI from
    //gPlatform = IORegistryEntryFromPath(masterPort, "IODeviceTree:/options");
    //GetOFVariable("MountEFI" ... and so on
    CFMutableDictionaryRef rtVariablesDict = addDict(dict, CFSTR("RtVariables"));
    addString(rtVariablesDict, CFSTR("MountEFI"), "_NOT_SHOWN_");
    addInteger(rtVariablesDict, CFSTR("LogLineCount"), s->LogLineCount);
    addString(rtVariablesDict, CFSTR("LogEveryBoot"), "_NOT_SHOWN_");

    dump_plist(dict);

}

int main(int argc, char **argv)
{
    kern_return_t       result;

    result = IOMasterPort(bootstrap_port, &masterPort);
    if (result != KERN_SUCCESS) {
        errx(1, "Error getting the IOMaster port: %s",
             mach_error_string(result));
    }

    gPlatform = IORegistryEntryFromPath(masterPort, "IODeviceTree:/efi/platform");
    if (gPlatform == 0) {
        errx(1, "EFI is not supported on this system");
    }

    CFTypeRef data;
    result = GetOFVariable("Settings", &data);
    if (result != KERN_SUCCESS) {
        errx(1, "Clover absent or too old : %s",
             mach_error_string(result));
    }

    PrintConfig(data);
    CFRelease(data);

    IOObjectRelease(gPlatform);

    return 0;
}