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

static CFMutableDictionaryRef patchDict[100];

CFMutableDictionaryRef addDict(CFMutableDictionaryRef dest, CFStringRef key)
{
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

CFMutableArrayRef addArray(CFMutableDictionaryRef dest, CFStringRef key)
{
  assert(dest);
  CFMutableArrayRef array = CFArrayCreateMutable (
                                                   kCFAllocatorDefault,
                                                   0,
                                                   &kCFTypeArrayCallBacks
                                                  );
  if (!array) {
    errx(1,"Error can't allocate array for key '%s'",
         CFStringGetCStringPtr( key, kCFStringEncodingMacRoman ));
  }
  CFDictionaryAddValue(dest, key, array );
  return array;
}

CFMutableDictionaryRef addDictToArray(CFMutableArrayRef dest)
{
  assert(dest);
  CFMutableDictionaryRef dict = CFDictionaryCreateMutable (
                                                             kCFAllocatorDefault,
                                                             0,
                                                             &kCFTypeDictionaryKeyCallBacks,
                                                             &kCFTypeDictionaryValueCallBacks
                                                             ); 
  if (!dict) {
    errx(1,"Error can't allocate dictionnary for array");
  }
  CFArrayAppendValue(dest, dict);
  return dict;
}

void addStringToArray(CFMutableArrayRef dest, const char* value)
{
  assert(dest);
  CFStringRef strValue = CFStringCreateWithCString(kCFAllocatorDefault,
                                                   value,
                                                   kCFStringEncodingMacRoman);
  assert(strValue);
  CFArrayAppendValue(dest, strValue);
  CFRelease(strValue);
}

void addString(CFMutableDictionaryRef dest, CFStringRef key, const char* value)
{
    assert(dest);
    CFStringRef strValue = CFStringCreateWithCString(kCFAllocatorDefault,
                                                     value,
                                                     kCFStringEncodingMacRoman);
    assert(strValue);
    CFDictionaryAddValue( dest, key, strValue );
    CFRelease(strValue);
}

void addUString(CFMutableDictionaryRef dest, CFStringRef key, const UniChar* value)
{
  if (!value) {
    return;
  }
    assert(dest);
    CFStringRef strValue = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%S"), value);
    assert(strValue);
    CFDictionaryAddValue( dest, key, strValue );
    CFRelease(strValue);
}

void addHex(CFMutableDictionaryRef dest, CFStringRef key, UInt64 value)
{
  CFStringRef strValue = NULL;

  assert(dest);
  if (value > 0xFFFF) {
    strValue = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("0x%08llx"), value);
  } else {
    strValue = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("0x%04llx"), value);
  }
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
  int i;
  
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
//    errx(1, "Error the version of clover-genconfig didn't match current booted clover version");
    printf("Error the version of clover-genconfig didn't match current booted clover version");
  }
  
  SETTINGS_DATA *s = (SETTINGS_DATA*)dataPtr;
  
  CFMutableDictionaryRef dict = CFDictionaryCreateMutable (
                                                           kCFAllocatorDefault,
                                                           0,
                                                           &kCFTypeDictionaryKeyCallBacks,
                                                           &kCFTypeDictionaryValueCallBacks
                                                           );
  if (s->ConfigName != NULL) {
    addUString(dict, CFSTR("ConfigName"), (const UniChar *)&s->ConfigName);
  } else {
    addString(dict, CFSTR("ConfigName"), "config");
  }

  //Boot
  CFMutableDictionaryRef bootDict = addDict(dict, CFSTR("Boot"));
  addString(bootDict, CFSTR("Arguments"), s->BootArgs);
  addUString(bootDict, CFSTR("Legacy"), (const UniChar *)&s->LegacyBoot);
 // addUString(bootDict, CFSTR("LegacyEntry"), s->LegacyBiosCustomEntry);
  addInteger(bootDict, CFSTR("XMPDetection"), s->XMPDetection);
  addUString(bootDict, CFSTR("DefaultVolume"), (const UniChar *)&s->DefaultVolume);
  addUString(bootDict, CFSTR("DefaultLoader"), (const UniChar *)&s->DefaultLoader);
  addBoolean(bootDict, CFSTR("Log"), s->Debug);
  addString(bootDict, CFSTR("Timeout"), "_NOT_SHOWN_");
  addBoolean(bootDict, CFSTR("Fast"), 0);
  
  
  // SystemParameters
  CFMutableDictionaryRef systemParametersDict = addDict(dict, CFSTR("SystemParameters"));
  addUString(systemParametersDict, CFSTR("CustomUUID"), (const UniChar *)&s->CustomUuid);
  addBoolean(systemParametersDict, CFSTR("InjectSystemID"), s->InjectSystemID);
  addHex(systemParametersDict, CFSTR("BacklightLevel"),s->BacklightLevel);
//  addBoolean(systemParametersDict, CFSTR("InjectKexts"), 0);
  addString(systemParametersDict, CFSTR("InjectKexts"), "Detect");

  // GUI
  CFMutableDictionaryRef guiDict = addDict(dict, CFSTR("GUI"));
  addString(guiDict, CFSTR("Language"), s->Language);
  addString(guiDict, CFSTR("Theme"), "BGM");
  addBoolean(guiDict, CFSTR("TextOnly"), 0);
  addBoolean(guiDict, CFSTR("CustomIcons"), 0);
    
  CFMutableDictionaryRef mouseDict = addDict(guiDict, CFSTR("Mouse"));
  addBoolean(mouseDict, CFSTR("Enabled"), s->PointerEnabled);
  addInteger(mouseDict, CFSTR("Speed"), s->PointerSpeed);
  addInteger(mouseDict, CFSTR("DoubleClick"), s->DoubleClickTime);
  addBoolean(mouseDict, CFSTR("Mirror"), s->PointerMirror);
  
  CFMutableArrayRef hideArray = addArray(guiDict, CFSTR("Hide"));
  addStringToArray(hideArray, "VolumeName_NOT_SHOWN");
  addStringToArray(hideArray, "VolumeUUID_NOT_SHOWN");
  addStringToArray(hideArray, "EntryPath_NOT_SHOWN");
  
  CFMutableDictionaryRef scanDict = addDict(guiDict, CFSTR("Scan"));
  addBoolean(scanDict, CFSTR("Entries"), 1);
  addBoolean(scanDict, CFSTR("Tool"), 1);
  addBoolean(scanDict, CFSTR("Legacy"), 1);
  
  CFMutableDictionaryRef customDict = addDict(guiDict, CFSTR("Custom"));
    CFMutableArrayRef entriesArray = addArray(customDict, CFSTR("Entries"));
      CFMutableDictionaryRef entries1Dict = addDictToArray(entriesArray);
      addString(entries1Dict, CFSTR("Volume"), "VolumeUUID_NOT_SHOWN");
      addString(entries1Dict, CFSTR("Path"), "_NOT_SHOWN_");
      addString(entries1Dict, CFSTR("Type"), "_NOT_SHOWN_");
      addString(entries1Dict, CFSTR("Arguments"), "_NOT_SHOWN_");
      addString(entries1Dict, CFSTR("Title"), "_NOT_SHOWN_");
      addString(entries1Dict, CFSTR("FullTitle"), "_NOT_SHOWN_");
      addString(entries1Dict, CFSTR("Image"), "_NOT_SHOWN_");
      addString(entries1Dict, CFSTR("Hotkey"), "_NOT_SHOWN_");
      addBoolean(entries1Dict, CFSTR("Disabled"), 1);
      addBoolean(entries1Dict, CFSTR("InjectKexts"), 1);
      addBoolean(entries1Dict, CFSTR("NoCaches"), 0);
      addBoolean(entries1Dict, CFSTR("Hidden"), 1);
      CFMutableArrayRef subEntriesArray = addArray(entries1Dict, CFSTR("SubEntries"));
        CFMutableDictionaryRef subEntries1Dict = addDictToArray(subEntriesArray);
        addString(subEntries1Dict, CFSTR("Title"), "_NOT_SHOWN_");
        addString(subEntries1Dict, CFSTR("AddArguments"), "_NOT_SHOWN_");
    CFMutableArrayRef legacyArray = addArray(customDict, CFSTR("Legacy"));
      CFMutableDictionaryRef legacy1Dict = addDictToArray(legacyArray);
      addString(legacy1Dict, CFSTR("Volume"), "VolumeUUID_NOT_SHOWN");
      addString(legacy1Dict, CFSTR("Type"), "_NOT_SHOWN_");
      addString(legacy1Dict, CFSTR("Title"), "_NOT_SHOWN_");
      addString(legacy1Dict, CFSTR("Hotkey"), "_NOT_SHOWN_");
      addBoolean(legacy1Dict, CFSTR("Disabled"), 1);
      addBoolean(legacy1Dict, CFSTR("Hidden"), 1);
    CFMutableArrayRef toolArray = addArray(customDict, CFSTR("Tool"));
      CFMutableDictionaryRef tool1Dict = addDictToArray(toolArray);
      addString(tool1Dict, CFSTR("Volume"), "VolumeUUID_NOT_SHOWN");
      addString(tool1Dict, CFSTR("Path"), "_NOT_SHOWN_");
      addString(tool1Dict, CFSTR("Type"), "_NOT_SHOWN_");
      addString(tool1Dict, CFSTR("Title"), "_NOT_SHOWN_");
      addString(tool1Dict, CFSTR("Arguments"), "_NOT_SHOWN_");
      addString(tool1Dict, CFSTR("Hotkey"), "_NOT_SHOWN_");
      addBoolean(tool1Dict, CFSTR("Disabled"), 1);
      addBoolean(tool1Dict, CFSTR("Hidden"), 1);
  
/*  
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
  addBoolean(hideEntriesDict, CFSTR("UEFIBootOptions"), s->HVHideUEFIBootOptions);
*/  
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
  
  addUUID(smbiosDict,   CFSTR("SmUUID"), &s->SmUUID);
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
    
    addString(memoryDict, CFSTR("Comment"), "there are no real data here");
    addInteger(memoryDict, CFSTR("SlotCount"), 0);
    addInteger(memoryDict, CFSTR("Channels"), 0);
    
    CFMutableArrayRef modulesArray = addArray(memoryDict, CFSTR("Modules"));
    CFMutableDictionaryRef moduleDict = addDictToArray(modulesArray);
    addInteger(moduleDict, CFSTR("Slot"), 0);
    addInteger(moduleDict, CFSTR("Size"), 0);
    addString(moduleDict, CFSTR("Vendor"), s->MemoryManufacturer);
    addString(moduleDict, CFSTR("Serial"), s->MemorySerialNumber);
    addString(moduleDict, CFSTR("Part"), s->MemoryPartNumber);
    addString(moduleDict, CFSTR("Frequency"), s->MemorySpeed);
    addString(moduleDict, CFSTR("Type"), "DDRx");
  }
  
  // CPU
  CFMutableDictionaryRef cpuDict = addDict(dict, CFSTR("CPU"));
  addInteger(cpuDict, CFSTR("Type"), s->CpuType);
  addInteger(cpuDict, CFSTR("FrequencyMHz"), s->CpuFreqMHz);
  addInteger(cpuDict, CFSTR("BusSpeedkHz"), s->BusSpeed);
  addInteger(cpuDict, CFSTR("QPI"), s->QPI);
  // these values read only
  addInteger(cpuDict, CFSTR("EnabledCores"), s->EnabledCores);
  addBoolean(cpuDict, CFSTR("C2"), s->EnableC2);  
  addBoolean(cpuDict, CFSTR("C4"), s->EnableC4); 
  addBoolean(cpuDict, CFSTR("C6"), s->EnableC6); 
  addInteger(cpuDict, CFSTR("Latency"), s->C3Latency); 
  
  // Devices
  CFMutableDictionaryRef pciDict = addDict(dict, CFSTR("Devices"));
  addBoolean(pciDict, CFSTR("Inject"), s->StringInjector);
  addString(pciDict, CFSTR("Properties"), "_NOT_SHOWN_");
//  addInteger(pciDict, CFSTR("PCIRootUID"), s->PCIRootUID);
  addBoolean(pciDict, CFSTR("NoDefaultProperties"), s->NoDefaultProperties);
  CFMutableArrayRef appPropArray = addArray(pciDict, CFSTR("AddProperties"));
  CFMutableDictionaryRef appPropDict = addDictToArray(appPropArray);
  addString(appPropDict, CFSTR("Device"), "XXX");
  addString(appPropDict, CFSTR("Key"), "AAPL,XXX");
  addHex(appPropDict, CFSTR("Value"), 0xFFFF);
  
  CFMutableDictionaryRef fakeIDDict = addDict(pciDict, CFSTR("FakeID"));
  addHex(fakeIDDict, CFSTR("ATI"), s->FakeATI);
  addHex(fakeIDDict, CFSTR("NVidia"), s->FakeNVidia);
  addHex(fakeIDDict, CFSTR("IntelGFX"), s->FakeIntel);
  addHex(fakeIDDict, CFSTR("LAN"), s->FakeLAN);
  addHex(fakeIDDict, CFSTR("WIFI"), s->FakeWIFI);
  addHex(fakeIDDict, CFSTR("SATA"), s->FakeSATA);
  addHex(fakeIDDict, CFSTR("XHCI"), s->FakeXHCI);
  addHex(fakeIDDict, CFSTR("IMEI"), s->FakeIMEI);
  
  CFMutableDictionaryRef audioDict = addDict(pciDict, CFSTR("Audio"));
  if (s->HDAInjection)
    addInteger(audioDict, CFSTR("Inject"), s->HDALayoutId);
  else
    addBoolean(audioDict, CFSTR("Inject"), s->HDAInjection);
  
  CFMutableDictionaryRef usbDict = addDict(pciDict, CFSTR("USB"));
  addBoolean(usbDict, CFSTR("Inject"), s->USBInjection);
  addBoolean(usbDict, CFSTR("FixOwnership"), s->USBFixOwnership);
  addBoolean(usbDict, CFSTR("AddClockID"), s->InjectClockID);
  addBoolean(usbDict, CFSTR("HighCurrent"), s->HighCurrent);
  
  // Graphics
  CFMutableDictionaryRef graphicsDict = addDict(dict, CFSTR("Graphics"));
  CFMutableDictionaryRef injectDict = addDict(graphicsDict, CFSTR("Inject"));
  addBoolean(injectDict, CFSTR("ATI"), s->InjectATI);
  addBoolean(injectDict, CFSTR("NVidia"), s->InjectNVidia);
  addBoolean(injectDict, CFSTR("Intel"), s->InjectIntel);
  addBoolean(graphicsDict, CFSTR("LoadVBios"), s->LoadVBios);
  addBoolean(graphicsDict, CFSTR("InjectEDID"), s->InjectEDID);
  addString(graphicsDict, CFSTR("CustomEDID"), "_NOT_SHOWN_");
  addBoolean(graphicsDict, CFSTR("PatchVBios"), s->PatchVBios);
  addInteger(graphicsDict, CFSTR("VideoPorts"), s->VideoPorts);
  addInteger(graphicsDict, CFSTR("VRAM"), s->VRAM);
  addInteger(graphicsDict, CFSTR("DualLink"), s->DualLink);
  // ATI specific"
  addUString(graphicsDict, CFSTR("FBName"), (const UniChar *)&s->FBName);
  // NVIDIA specific
  addIntArray(graphicsDict, CFSTR("display-cfg"), &s->Dcfg[0], 8);
  addIntArray(graphicsDict, CFSTR("NVCAP"), &s->NVCAP[0], 20);
  // INTEL specific
  addHex(graphicsDict, CFSTR("ig-platform-id"), s->IgPlatform);
  addInteger(graphicsDict, CFSTR("PatchVBiosBytes Count"), s->PatchVBiosBytesCount);
  CFMutableArrayRef vbiosArray = addArray(graphicsDict, CFSTR("PatchVBiosBytes"));
  CFMutableDictionaryRef vbiosDict = addDictToArray(vbiosArray);
  addString(vbiosDict, CFSTR("Find"), "_NOT_SHOWN_");
  addString(vbiosDict, CFSTR("Replace"), "_NOT_SHOWN_");
  
  //ACPI
  CFMutableDictionaryRef acpiDict = addDict(dict, CFSTR("ACPI"));
  addHex(acpiDict, CFSTR("ResetAddress"), s->ResetAddr);
  addHex(acpiDict, CFSTR("ResetValue"), s->ResetVal);
  addBoolean(acpiDict, CFSTR("HaltEnabler"), s->SlpSmiEnable);
  addBoolean(acpiDict, CFSTR("PatchAPIC"), s->PatchNMI);
  addBoolean(acpiDict, CFSTR("smartUPS"), s->smartUPS);

  CFMutableDictionaryRef dsdtDict = addDict(acpiDict, CFSTR("DSDT"));
  addUString(dsdtDict, CFSTR("Name"), (const UniChar *)&s->DsdtName);
//  addHex(dsdtDict, CFSTR("FixMask"), s->FixDsdt);
  addBoolean(dsdtDict, CFSTR("Debug"), s->DebugDSDT);
  addBoolean(dsdtDict, CFSTR("ReuseFFFF"), s->ReuseFFFF);
  addBoolean(dsdtDict, CFSTR("SuspendOverride"), s->SuspendOverride);
  addBoolean(dsdtDict, CFSTR("SlpSmiAtWake"), s->SlpWak);
  addInteger(dsdtDict, CFSTR("Patches count"), s->PatchDsdtNum);

  CFMutableDictionaryRef fixDict = addDict(dsdtDict, CFSTR("Fixes"));
  addBoolean(fixDict, CFSTR("AddDTGP_0001"),       !!(s->FixDsdt & FIX_DTGP));
  addBoolean(fixDict, CFSTR("FixDarwin_0002"),     !!(s->FixDsdt & FIX_WARNING));
  addBoolean(fixDict, CFSTR("FixShutdown_0004"),   !!(s->FixDsdt & FIX_SHUTDOWN));
  addBoolean(fixDict, CFSTR("AddMCHC_0008"),       !!(s->FixDsdt & FIX_MCHC));
  addBoolean(fixDict, CFSTR("FixHPET_0010"),       !!(s->FixDsdt & FIX_HPET));
  addBoolean(fixDict, CFSTR("FakeLPC_0020"),       !!(s->FixDsdt & FIX_LPC));
  addBoolean(fixDict, CFSTR("FixIPIC_0040"),       !!(s->FixDsdt & FIX_IPIC));
  addBoolean(fixDict, CFSTR("FixSBUS_0080"),       !!(s->FixDsdt & FIX_SBUS));
  addBoolean(fixDict, CFSTR("FixDisplay_0100"),    !!(s->FixDsdt & FIX_DISPLAY));
  addBoolean(fixDict, CFSTR("FixIDE_0200"),        !!(s->FixDsdt & FIX_IDE));
  addBoolean(fixDict, CFSTR("FixSATA_0400"),       !!(s->FixDsdt & FIX_SATA));
  addBoolean(fixDict, CFSTR("FixFirewire_0800"),   !!(s->FixDsdt & FIX_FIREWIRE));
  addBoolean(fixDict, CFSTR("FixUSB_1000"),        !!(s->FixDsdt & FIX_USB));
  addBoolean(fixDict, CFSTR("FixLAN_2000"),        !!(s->FixDsdt & FIX_LAN));
  addBoolean(fixDict, CFSTR("FixAirport_4000"),    !!(s->FixDsdt & FIX_WIFI));
  addBoolean(fixDict, CFSTR("FixHDA_8000"),        !!(s->FixDsdt & FIX_HDA));



  CFMutableArrayRef dsdtPatchArray = addArray(dsdtDict, CFSTR("Patches"));
    CFMutableDictionaryRef dsdtPatchDict = addDictToArray(dsdtPatchArray);
    addString(dsdtPatchDict, CFSTR("Comment"), "This is for sample");
    addString(dsdtPatchDict, CFSTR("Find"), "_NOT_SHOWN_");
    addString(dsdtPatchDict, CFSTR("Replace"), "_NOT_SHOWN_");

  CFMutableDictionaryRef dsmDict = addDict(dsdtDict, CFSTR("DropOEM_DSM"));
  addBoolean(dsmDict, CFSTR("ATI"),       !!(s->DropOEM_DSM & DEV_ATI));
  addBoolean(dsmDict, CFSTR("IntelGFX"),  !!(s->DropOEM_DSM & DEV_INTEL));
  addBoolean(dsmDict, CFSTR("NVidia"),    !!(s->DropOEM_DSM & DEV_NVIDIA));
  addBoolean(dsmDict, CFSTR("LAN"),       !!(s->DropOEM_DSM & DEV_LAN));
  addBoolean(dsmDict, CFSTR("WIFI"),      !!(s->DropOEM_DSM & DEV_WIFI));
  addBoolean(dsmDict, CFSTR("HDA"),       !!(s->DropOEM_DSM & DEV_HDA));
  addBoolean(dsmDict, CFSTR("HDMI"),      !!(s->DropOEM_DSM & DEV_HDMI));
  addBoolean(dsmDict, CFSTR("LPC"),       !!(s->DropOEM_DSM & DEV_LPC));
  addBoolean(dsmDict, CFSTR("SmBUS"),     !!(s->DropOEM_DSM & DEV_SMBUS));
  addBoolean(dsmDict, CFSTR("Firewire"),  !!(s->DropOEM_DSM & DEV_FIREWIRE));
  addBoolean(dsmDict, CFSTR("USB"),       !!(s->DropOEM_DSM & DEV_USB));
  addBoolean(dsmDict, CFSTR("IDE"),       !!(s->DropOEM_DSM & DEV_IDE));
  addBoolean(dsmDict, CFSTR("SATA"),      !!(s->DropOEM_DSM & DEV_SATA));
  
  CFMutableDictionaryRef ssdtDict = addDict(acpiDict, CFSTR("SSDT"));
    CFMutableDictionaryRef genDict = addDict(ssdtDict, CFSTR("Generate"));
    addBoolean(genDict, CFSTR("PStates"), s->GeneratePStates);
    addBoolean(genDict, CFSTR("CStates"), s->GenerateCStates);
  addBoolean(ssdtDict, CFSTR("DropOem"), s->DropSSDT);
  addBoolean(ssdtDict, CFSTR("DoubleFirstState"), s->DoubleFirstState);
  addInteger(ssdtDict, CFSTR("MinMultiplier"), s->MinMultiplier);
  addInteger(ssdtDict, CFSTR("MaxMultiplier"), s->MaxMultiplier);
  addInteger(ssdtDict, CFSTR("PLimitDict"), s->PLimitDict);
  addInteger(ssdtDict, CFSTR("UnderVoltStep"), s->UnderVoltStep);
  addInteger(ssdtDict, CFSTR("PluginType"), s->PluginType);
  addBoolean(ssdtDict, CFSTR("UseSystemIO"), s->EnableISS);

  CFMutableArrayRef dropArray = addArray(acpiDict, CFSTR("DropTables"));
  CFMutableDictionaryRef drop1Dict = addDictToArray(dropArray);
  addString(drop1Dict, CFSTR("Signature"), "_NOT_SHOWN_");
  addString(drop1Dict, CFSTR("TableId"), "_NOT_SHOWN_");
  addInteger(drop1Dict, CFSTR("Length"), 0);
  
  
  // KernelAndKextPatches
  CFMutableDictionaryRef KernelAndKextPatchesDict = addDict(dict, CFSTR("KernelAndKextPatches"));
  addBoolean(KernelAndKextPatchesDict, CFSTR("Debug"), s->KPDebug);
  addBoolean(KernelAndKextPatchesDict, CFSTR("KernelCpu"), s->KPKernelCpu);
  addBoolean(KernelAndKextPatchesDict, CFSTR("KernelLapic"), s->KPLapicPanic);
  addBoolean(KernelAndKextPatchesDict, CFSTR("AppleRTC"), s->KPAppleRTC);
  addBoolean(KernelAndKextPatchesDict, CFSTR("AsusAICPUPM"), s->KPAsusAICPUPM);
  addBoolean(KernelAndKextPatchesDict, CFSTR("KextPatchesAllowed"), s->KextPatchesAllowed);
  addInteger(KernelAndKextPatchesDict, CFSTR("Number_of_KextsToPatch"), s->NrKexts);
    
  CFMutableArrayRef KKPatchArray = addArray(KernelAndKextPatchesDict, CFSTR("KextsToPatch"));
  for (i = 0; i < s->NrKexts; i++) {
    patchDict[i] = addDictToArray(KKPatchArray);
    addString(patchDict[i], CFSTR("Name"), "_NOT_SHOWN_");
    addString(patchDict[i], CFSTR("Find"), "_NOT_SHOWN_");
    addString(patchDict[i], CFSTR("Replace"), "_NOT_SHOWN_");
  }
  
  
  //TODO
  // here we can get LogEveryBoot and MountEFI from
  //gPlatform = IORegistryEntryFromPath(masterPort, "IODeviceTree:/options");
  //GetOFVariable("MountEFI" ... and so on
  CFMutableDictionaryRef rtVariablesDict = addDict(dict, CFSTR("RtVariables"));
  addString(rtVariablesDict, CFSTR("ROM"), "_NOT_SHOWN_" /*s->RtROM*/);
  addString(rtVariablesDict, CFSTR("MLB"), s->BoardSerialNumber);
  addString(rtVariablesDict, CFSTR("MountEFI"), "_NOT_SHOWN_");
  addInteger(rtVariablesDict, CFSTR("LogLineCount"), s->LogLineCount);
  addString(rtVariablesDict, CFSTR("LogEveryBoot"), "_NOT_SHOWN_");
  
  CFMutableArrayRef disArray = addArray(dict, CFSTR("DisableDrivers"));
  addStringToArray(disArray, "_NOT_SHOWN_");
  
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
