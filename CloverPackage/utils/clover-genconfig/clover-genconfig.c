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
cc -o genconfig clover-genconfig.c gfxutil.c -framework CoreFoundation -framework IOKit -Wall -Wno-unused-function
*/

// EDK2 includes
//#include <base.h>

#define __DEBUG_LIB_H__
#define  _STRUCT_X86_THREAD_STATE32
#define  _STRUCT_X86_THREAD_STATE64

#include "../../../rEFIt_UEFI/Platform/Platform.h"

#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>

#include <err.h>
#include <mach/mach_error.h>
#define GFX 1
#if GFX
  #include "gfxutil.h"
#endif

/* from efidevp.c */
extern CHAR8 *ConvertDevicePathToAscii (const struct _EFI_DEVICE_PATH_P_TAG  *DeviceNode, BOOLEAN DisplayOnly, BOOLEAN AllowShortcuts);

/*
#define offsetof(st, m) \
((UINTN) ( (UINT8 *)&((st *)(0))->m - (UINT8 *)0 ))
*/

// Prototypes
//static kern_return_t GetOFVariable(const char *name, CFTypeRef *valueRef);

// Global Variables
static io_registry_entry_t gEFI;
static io_registry_entry_t gPlatform;
static mach_port_t         masterPort;

#if 0
static CFMutableDictionaryRef patchDict[100];
#endif

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


void addUUID(CFMutableDictionaryRef dest, CFStringRef key, UInt8 *uuid)
{
  SInt64 i = 0;
  CFMutableStringRef strValue = CFStringCreateMutable (kCFAllocatorDefault, 0);

  for (i = 0; i < 4; i++) {
    CFStringAppendFormat(strValue, NULL, CFSTR("%02x"), *uuid++);
  }
  for (i = 0; i < 3; i++) {
    CFStringAppendFormat(strValue, NULL, CFSTR("-%02x"), *uuid++);
    CFStringAppendFormat(strValue, NULL, CFSTR("%02x"), *uuid++);
  }
  CFStringAppendFormat(strValue, NULL, CFSTR("-"));
  for (i = 0; i < 6; i++) {
    CFStringAppendFormat(strValue, NULL, CFSTR("%02x"), *uuid++);
  }
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
static kern_return_t GetOFVariable(io_registry_entry_t entry, const char *name, CFTypeRef *valueRef)
{
    CFStringRef nameRef = CFStringCreateWithCString(kCFAllocatorDefault, name,
                                                    kCFStringEncodingUTF8);
    if (!nameRef) {
        errx(1, "Error creating CFString for key %s", name);
    }

    *valueRef = IORegistryEntryCreateCFProperty(entry, nameRef, 0, 0);
    CFRelease(nameRef);
    if (!*valueRef) {
        printf("value not found\n");
        return kIOReturnNotFound;
    }

    return KERN_SUCCESS;
}

void addGFXDictionary(CFMutableDictionaryRef dict, GFX_HEADER * gfx)
{
  CFMutableDictionaryRef items;
  CFDataRef data = NULL;
  //CFNumberRef number = NULL;
  CFStringRef string = NULL;
  CFStringRef key = NULL;
  GFX_BLOCKHEADER *gfx_blockheader_tmp;
  GFX_ENTRY *gfx_entry_tmp;
//  uint64_t bigint;
//  char hexstr[32];
  char *dpath;
  
  // Create dictionary that will hold gfx data
//  dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0 ,&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  
  gfx_blockheader_tmp = gfx->blocks;
  while(gfx_blockheader_tmp)
  {
    items = CFDictionaryCreateMutable(kCFAllocatorDefault, 0 ,&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    gfx_entry_tmp = gfx_blockheader_tmp->entries;
    while(gfx_entry_tmp)
    {
      key = CFStringCreateWithCString(kCFAllocatorDefault, gfx_entry_tmp->key, kCFStringEncodingUTF8);
      switch(gfx_entry_tmp->val_type)
      {
        case DATA_STRING:
          string = CFStringCreateWithBytes(kCFAllocatorDefault,gfx_entry_tmp->val, gfx_entry_tmp->val_len-1, kCFStringEncodingASCII, false);
          CFDictionarySetValue(items, key, string);
          CFRelease(string);
          CFRelease(key);
          break;
          /*
        case DATA_INT8:
          bigint = READ_UINT8(gfx_entry_tmp->val);
          sprintf(hexstr,"0x%02llx",bigint);
          string = CFStringCreateWithCString(kCFAllocatorDefault,hexstr, kCFStringEncodingASCII);
          CFDictionarySetValue(items, key, string);
          CFRelease(string);
          CFRelease(key);
          break;
        case DATA_INT16:
          bigint = READ_UINT16(gfx_entry_tmp->val);
          sprintf(hexstr,"0x%04llx",bigint);
          string = CFStringCreateWithCString(kCFAllocatorDefault,hexstr, kCFStringEncodingASCII);
          CFDictionarySetValue(items, key, string);
          CFRelease(string);
          CFRelease(key);
          break;
        case DATA_INT32:
          bigint = READ_UINT32(gfx_entry_tmp->val);
          sprintf(hexstr,"0x%08llx",bigint);
          string = CFStringCreateWithCString(kCFAllocatorDefault,hexstr, kCFStringEncodingASCII);
          CFDictionarySetValue(items, key, string);
          CFRelease(string);
          CFRelease(key);
          break;
           */
        default:
        case DATA_BINARY:
          data = CFDataCreate(kCFAllocatorDefault,gfx_entry_tmp->val, gfx_entry_tmp->val_len);
          CFDictionarySetValue(items, key, data);
          CFRelease(data);
          CFRelease(key);
          break;
      }
      gfx_entry_tmp = gfx_entry_tmp->next;
    }
    
    dpath = ConvertDevicePathToAscii (gfx_blockheader_tmp->devpath, 1, 1);
    if(dpath != NULL)
    {
      key = CFStringCreateWithCString(kCFAllocatorDefault, dpath, kCFStringEncodingUTF8);
    }
    else
    {
      printf("CreateGFXDictionary: error converting device path to text shorthand notation\n");
      return;
    }
    
    CFDictionarySetValue(dict, key, items);
    
    free(dpath);
    CFRelease(key);
    CFRelease(items);
    gfx_blockheader_tmp = gfx_blockheader_tmp->next;
  }
  
  return;
}


void PrintConfig(CFTypeRef data, GFX_HEADER * gfx)
{
  const Byte *dataPtr = NULL;
  CFIndex    length = 0;
  CFTypeID   typeID;
//  int i;

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
    printf("Error the version of clover-genconfig didn't match current booted clover version\n");
    printf("len=%d sizeof=%d\n", (int)length, (int)sizeof(SETTINGS_DATA));
#if defined(MDE_CPU_IA32)
    printf("32 bit generator\n");
#elif defined(MDE_CPU_X64)
    printf("64 bit generator\n");
#else
    printf("xxx bit generator\n");
#endif
    return;
  }

  SETTINGS_DATA *s = (SETTINGS_DATA*)dataPtr;

  CFMutableDictionaryRef dict = CFDictionaryCreateMutable (
                                                           kCFAllocatorDefault,
                                                           0,
                                                           &kCFTypeDictionaryKeyCallBacks,
                                                           &kCFTypeDictionaryValueCallBacks
                                                           );
/*
  if (s->ConfigName != NULL) {
    //ConfigName allocated in boot memory, impossible to show
//    addUString(dict, CFSTR("ConfigName"), (const UniChar *)&s->ConfigName);
    addString(dict, CFSTR("ConfigName"), "config1");
  } else {
    addString(dict, CFSTR("ConfigName"), "config");
  }
*/
  //This is possible since Clover 4511
  addUString(dict, CFSTR("ConfigName"), (const UniChar *)&s->ConfigName);
  //Boot
  CFMutableDictionaryRef bootDict = addDict(dict, CFSTR("Boot"));
  addString(bootDict, CFSTR("Arguments"), s->BootArgs);
  addUString(bootDict, CFSTR("Legacy"), (const UniChar *)&s->LegacyBoot);
 // addUString(bootDict, CFSTR("LegacyEntry"), s->LegacyBiosCustomEntry);
  addInteger(bootDict, CFSTR("XMPDetection"), s->XMPDetection);
  //impossible
//  addUString(bootDict, CFSTR("DefaultVolume"), (const UniChar *)&s->DefaultVolume);
//  addUString(bootDict, CFSTR("DefaultLoader"), (const UniChar *)&s->DefaultLoader);
  addBoolean(bootDict, CFSTR("Debug"), s->Debug);
  addString(bootDict, CFSTR("#Timeout"), "_NOT_SHOWN_");
  addBoolean(bootDict, CFSTR("Fast"), 0);
  addString(bootDict, CFSTR("#CustomLogo"), "_NOT_SHOWN_");
  addBoolean(bootDict, CFSTR("#NeverHibernate"), 0);
  addBoolean(bootDict, CFSTR("#StrictHibernate"), 0);
  addBoolean(bootDict, CFSTR("RtcHibernateAware"), 0);
  addBoolean(bootDict, CFSTR("NeverDoRecovery"), s->NeverDoRecovery);
  addBoolean(bootDict, CFSTR("SkipHibernateTimeout"), s->SkipHibernateTimeout);
  addBoolean(bootDict, CFSTR("DisableCloverHotkeys"), s->DisableCloverHotkeys);
  addInteger(bootDict, CFSTR("#LegacyBiosDefaultEntry"), s->LegacyBiosDefaultEntry);

  // SystemParameters
  CFMutableDictionaryRef systemParametersDict = addDict(dict, CFSTR("SystemParameters"));
  addUString(systemParametersDict, CFSTR("CustomUUID"), (const UniChar *)&s->CustomUuid);
  addBoolean(systemParametersDict, CFSTR("InjectSystemID"), s->InjectSystemID);
  addHex(systemParametersDict, CFSTR("BacklightLevel"),s->BacklightLevel);
//  addBoolean(systemParametersDict, CFSTR("InjectKexts"), 0);
  addString(systemParametersDict, CFSTR("#InjectKexts"), "Detect");
  addBoolean(systemParametersDict, CFSTR("NvidiaWeb"), s->NvidiaWeb);

  // GUI
  CFMutableDictionaryRef guiDict = addDict(dict, CFSTR("GUI"));
  addString(guiDict, CFSTR("#Language"), s->Language);
  addString(guiDict, CFSTR("#Theme"), "embedded");
  addBoolean(guiDict, CFSTR("TextOnly"), 0);
  addBoolean(guiDict, CFSTR("CustomIcons"), 0);

  CFMutableDictionaryRef mouseDict = addDict(guiDict, CFSTR("Mouse"));
  addBoolean(mouseDict, CFSTR("Enabled"), s->PointerEnabled);
  addInteger(mouseDict, CFSTR("Speed"), s->PointerSpeed);
//  addInteger(mouseDict, CFSTR("DoubleClick"), s->DoubleClickTime);
  addBoolean(mouseDict, CFSTR("Mirror"), s->PointerMirror);

  CFMutableArrayRef hideArray = addArray(guiDict, CFSTR("#Hide"));
  addStringToArray(hideArray, "VolumeName_NOT_SHOWN");
  addStringToArray(hideArray, "VolumeUUID_NOT_SHOWN");
  addStringToArray(hideArray, "EntryPath_NOT_SHOWN");

  CFMutableDictionaryRef scanDict = addDict(guiDict, CFSTR("Scan"));
  addString(scanDict, CFSTR("Comment"), "These values wrong, they present for sample");
  addBoolean(scanDict, CFSTR("#Entries"), 1);
  addBoolean(scanDict, CFSTR("#Tool"), 1);
  addBoolean(scanDict, CFSTR("#Legacy"), 1);

  CFMutableDictionaryRef customDict = addDict(guiDict, CFSTR("Custom"));
  addString(customDict, CFSTR("Comment"), "These values wrong, they present for sample");
    CFMutableArrayRef entriesArray = addArray(customDict, CFSTR("Entries"));
      CFMutableDictionaryRef entries1Dict = addDictToArray(entriesArray);
      addString(entries1Dict, CFSTR("Comment"), "These values wrong, they present for sample");
      addString(entries1Dict, CFSTR("#Volume"), "VolumeUUID_NOT_SHOWN");
      addString(entries1Dict, CFSTR("#Path"), "_NOT_SHOWN_");
      addString(entries1Dict, CFSTR("#Type"), "_NOT_SHOWN_");
      addString(entries1Dict, CFSTR("#Arguments"), "_NOT_SHOWN_");
      addString(entries1Dict, CFSTR("#AddArguments"), "-v");
      addString(entries1Dict, CFSTR("#Title"), "_NOT_SHOWN_");
      addString(entries1Dict, CFSTR("#FullTitle"), "_NOT_SHOWN_");
      addString(entries1Dict, CFSTR("#Image"), "_NOT_SHOWN_");
      addString(entries1Dict, CFSTR("#Hotkey"), "_NOT_SHOWN_");
      addBoolean(entries1Dict, CFSTR("#Disabled"), 1);
      addBoolean(entries1Dict, CFSTR("#InjectKexts"), 1);
      addBoolean(entries1Dict, CFSTR("#NoCaches"), 0);
      addBoolean(entries1Dict, CFSTR("#Hidden"), 1);
      CFMutableArrayRef subEntriesArray = addArray(entries1Dict, CFSTR("SubEntries"));
        CFMutableDictionaryRef subEntries1Dict = addDictToArray(subEntriesArray);
        addString(subEntries1Dict, CFSTR("#Title"), "_NOT_SHOWN_");
        addString(subEntries1Dict, CFSTR("#AddArguments"), "_NOT_SHOWN_");
    CFMutableArrayRef legacyArray = addArray(customDict, CFSTR("Legacy"));
      CFMutableDictionaryRef legacy1Dict = addDictToArray(legacyArray);
      addString(legacy1Dict, CFSTR("#Volume"), "VolumeUUID_NOT_SHOWN");
      addString(legacy1Dict, CFSTR("#Type"), "_NOT_SHOWN_");
      addString(legacy1Dict, CFSTR("#Title"), "_NOT_SHOWN_");
      addString(legacy1Dict, CFSTR("#Hotkey"), "_NOT_SHOWN_");
      addBoolean(legacy1Dict, CFSTR("#Disabled"), 1);
      addBoolean(legacy1Dict, CFSTR("#Hidden"), 1);
    CFMutableArrayRef toolArray = addArray(customDict, CFSTR("Tool"));
      CFMutableDictionaryRef tool1Dict = addDictToArray(toolArray);
      addString(tool1Dict, CFSTR("#Volume"), "VolumeUUID_NOT_SHOWN");
      addString(tool1Dict, CFSTR("#Path"), "_NOT_SHOWN_");
      addString(tool1Dict, CFSTR("#Type"), "_NOT_SHOWN_");
      addString(tool1Dict, CFSTR("#Title"), "_NOT_SHOWN_");
      addString(tool1Dict, CFSTR("#Arguments"), "_NOT_SHOWN_");
      addString(tool1Dict, CFSTR("#Hotkey"), "_NOT_SHOWN_");
      addBoolean(tool1Dict, CFSTR("#Disabled"), 1);
      addBoolean(tool1Dict, CFSTR("#Hidden"), 1);

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

  addUUID(smbiosDict,   CFSTR("SmUUID"), (UInt8 *)&s->SmUUID);
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
  if (s->PlatformFeature != 0xFFFF) {
    addHex(smbiosDict, CFSTR("PlatformFeature"), s->PlatformFeature);
  }

  if (s->InjectMemoryTables) {
    CFMutableDictionaryRef memoryDict = addDict(smbiosDict, CFSTR("Memory"));

    addString(memoryDict, CFSTR("Comment"), "there are no real data here");
    addInteger(memoryDict, CFSTR("#SlotCount"), 0);
    addInteger(memoryDict, CFSTR("#Channels"), 0);

    CFMutableArrayRef modulesArray = addArray(memoryDict, CFSTR("Modules"));
    CFMutableDictionaryRef moduleDict = addDictToArray(modulesArray);
    addInteger(moduleDict, CFSTR("#Slot"), 0);
    addInteger(moduleDict, CFSTR("#Size"), 0);
    addString(moduleDict, CFSTR("#Vendor"), s->MemoryManufacturer);
    addString(moduleDict, CFSTR("#Serial"), s->MemorySerialNumber);
    addString(moduleDict, CFSTR("#Part"), s->MemoryPartNumber);
    addString(moduleDict, CFSTR("#Frequency"), s->MemorySpeed);
    addString(moduleDict, CFSTR("#Type"), "DDRx");
  }
  CFMutableArrayRef slotsArray = addArray(smbiosDict, CFSTR("#Slots"));
  CFMutableDictionaryRef slotsDict = addDictToArray(slotsArray);
  addString(slotsDict, CFSTR("Comment"), "there is a sample");
  addString(slotsDict, CFSTR("Device"), "WIFI");
  addInteger(slotsDict, CFSTR("ID"), 5);
  addInteger(slotsDict, CFSTR("Type"), 1);
  addString(slotsDict, CFSTR("Name"), "Airport");


  // CPU
  CFMutableDictionaryRef cpuDict = addDict(dict, CFSTR("CPU"));
  addHex(cpuDict, CFSTR("Type"), s->CpuType);
  addInteger(cpuDict, CFSTR("FrequencyMHz"), s->CpuFreqMHz);
  addInteger(cpuDict, CFSTR("#BusSpeedkHz"), s->BusSpeed);
  addInteger(cpuDict, CFSTR("QPI"), s->QPI);
  addInteger(cpuDict, CFSTR("SavingMode"), s->SavingMode);
  addBoolean(cpuDict, CFSTR("#UseARTFrequency"), s->UseARTFreq);
  addBoolean(cpuDict, CFSTR("#TurboDisable"), (s->Turbo == 0));
  addBoolean(cpuDict, CFSTR("#HWPEnable"), s->HWP);
  addInteger(cpuDict, CFSTR("#HWPValue"), s->HWPValue);
  addInteger(cpuDict, CFSTR("EnabledCores"), s->EnabledCores);
  addBoolean(cpuDict, CFSTR("#TDP"), s->TDP);
  addBoolean(cpuDict, CFSTR("#QEMU"), s->QEMU);


  // Devices
  CFMutableDictionaryRef pciDict = addDict(dict, CFSTR("Devices"));
  addBoolean(pciDict, CFSTR("#Inject"), s->StringInjector);
  addString(pciDict, CFSTR("#Properties"), "_NOT_SHOWN_");
//  addInteger(pciDict, CFSTR("PCIRootUID"), s->PCIRootUID);
  addBoolean(pciDict, CFSTR("#NoDefaultProperties"), s->NoDefaultProperties);
  CFMutableArrayRef appPropArray = addArray(pciDict, CFSTR("#AddProperties"));
  CFMutableDictionaryRef appPropDict = addDictToArray(appPropArray);
  addString(appPropDict, CFSTR("#Device"), "XXX");
  addBoolean(appPropDict, CFSTR("#Disabled"), 1);
  addString(appPropDict, CFSTR("#Key"), "AAPL,XXX");
  addHex(appPropDict, CFSTR("#Value"), 0xFFFF);
  
  CFMutableDictionaryRef propDict = addDict(pciDict, CFSTR("Properties"));
  addGFXDictionary(propDict, gfx);
  


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
    addInteger(audioDict, CFSTR("#Inject"), s->HDALayoutId);
  else
    addBoolean(audioDict, CFSTR("#Inject"), s->HDAInjection);
  addBoolean(audioDict, CFSTR("#ResetHDA"), s->ResetHDA);

  addBoolean(pciDict, CFSTR("UseIntelHDMI"), s->UseIntelHDMI);
  addBoolean(pciDict, CFSTR("ForceHPET"), s->ForceHPET);
  addBoolean(pciDict, CFSTR("#SetIntelBacklight"), s->IntelBacklight);
  addBoolean(pciDict, CFSTR("#SetIntelMaxBacklight"), s->IntelMaxBacklight);
  addInteger(pciDict, CFSTR("#IntelMaxValue"), s->IntelMaxValue);


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
//  addBoolean(graphicsDict, CFSTR("InjectEDID"), s->InjectEDID);
//  addString(graphicsDict, CFSTR("#CustomEDID"), "_NOT_SHOWN_");
  addBoolean(graphicsDict, CFSTR("PatchVBios"), s->PatchVBios);
  addInteger(graphicsDict, CFSTR("VideoPorts"), s->VideoPorts);
  addInteger(graphicsDict, CFSTR("VRAM"), s->VRAM);
  addInteger(graphicsDict, CFSTR("DualLink"), s->DualLink);
  // ATI specific"
  addUString(graphicsDict, CFSTR("FBName"), (const UniChar *)&s->FBName);
  addBoolean(graphicsDict, CFSTR("RadeonDeInit"), s->DeInit);

  // NVIDIA specific
  addIntArray(graphicsDict, CFSTR("display-cfg"), &s->Dcfg[0], 8);
  addIntArray(graphicsDict, CFSTR("NVCAP"), &s->NVCAP[0], 20);
  addBoolean(graphicsDict, CFSTR("NvidiaGeneric"), s->NvidiaGeneric);
  addBoolean(graphicsDict, CFSTR("NvidiaNoEFI"), s->NvidiaNoEFI);
  addBoolean(graphicsDict, CFSTR("NvidiaSingle"), s->NvidiaSingle);
  // INTEL specific
  addHex(graphicsDict, CFSTR("ig-platform-id"), s->IgPlatform);
  addInteger(graphicsDict, CFSTR("#PatchVBiosBytes Count"), s->PatchVBiosBytesCount);
  CFMutableArrayRef vbiosArray = addArray(graphicsDict, CFSTR("#PatchVBiosBytes"));
  CFMutableDictionaryRef vbiosDict = addDictToArray(vbiosArray);
  addString(vbiosDict, CFSTR("#Find"), "_NOT_SHOWN_");
  addString(vbiosDict, CFSTR("#Replace"), "_NOT_SHOWN_");
  //EDID
   CFMutableDictionaryRef edidDict = addDict(graphicsDict, CFSTR("EDID"));
  addBoolean(edidDict, CFSTR("Inject"), s->InjectEDID);
  addString(edidDict, CFSTR("#Custom"), "_NOT_SHOWN_");
  addHex(edidDict, CFSTR("#VendorID"), s->VendorEDID);
  addHex(edidDict, CFSTR("#ProductID"), s->ProductEDID);

  //ACPI
  CFMutableDictionaryRef acpiDict = addDict(dict, CFSTR("ACPI"));
  addHex(acpiDict, CFSTR("ResetAddress"), s->ResetAddr);
  addHex(acpiDict, CFSTR("ResetValue"), s->ResetVal);
  addBoolean(acpiDict, CFSTR("HaltEnabler"), s->SlpSmiEnable);
  addBoolean(acpiDict, CFSTR("PatchAPIC"), s->PatchNMI);
  addBoolean(acpiDict, CFSTR("smartUPS"), s->smartUPS);
  addBoolean(acpiDict, CFSTR("AutoMerge"), s->AutoMerge);
  addBoolean(acpiDict, CFSTR("DisableASPM"), s->NoASPM);
  addBoolean(acpiDict, CFSTR("FixHeaders"), s->FixHeaders);
  addBoolean(acpiDict, CFSTR("FixMCFG"), s->FixMCFG);

  CFMutableDictionaryRef dsdtDict = addDict(acpiDict, CFSTR("DSDT"));
  addUString(dsdtDict, CFSTR("Name"), (const UniChar *)&s->DsdtName);
  addBoolean(dsdtDict, CFSTR("Debug"), s->DebugDSDT);
  addBoolean(dsdtDict, CFSTR("ReuseFFFF"), s->ReuseFFFF);
  addBoolean(dsdtDict, CFSTR("SuspendOverride"), s->SuspendOverride);
  addBoolean(dsdtDict, CFSTR("Rtc8Allowed"), s->Rtc8Allowed);
//  addBoolean(dsdtDict, CFSTR("SlpSmiAtWake"), s->SlpWak);
  addInteger(dsdtDict, CFSTR("#Patches count"), s->PatchDsdtNum);

  CFMutableDictionaryRef fixDict = addDict(dsdtDict, CFSTR("Fixes"));
  addBoolean(fixDict, CFSTR("AddDTGP"),        !!(s->FixDsdt & FIX_DTGP));
  addBoolean(fixDict, CFSTR("FixDarwin"),      !!(s->FixDsdt & FIX_WARNING));
  addBoolean(fixDict, CFSTR("FixShutdown"),    !!(s->FixDsdt & FIX_SHUTDOWN));
  addBoolean(fixDict, CFSTR("AddMCHC"),        !!(s->FixDsdt & FIX_MCHC));
  addBoolean(fixDict, CFSTR("FixHPET"),        !!(s->FixDsdt & FIX_HPET));
  addBoolean(fixDict, CFSTR("FakeLPC"),        !!(s->FixDsdt & FIX_LPC));
  addBoolean(fixDict, CFSTR("FixIPIC"),        !!(s->FixDsdt & FIX_IPIC));
  addBoolean(fixDict, CFSTR("FixSBUS"),        !!(s->FixDsdt & FIX_SBUS));
  addBoolean(fixDict, CFSTR("FixDisplay"),     !!(s->FixDsdt & FIX_DISPLAY));
  addBoolean(fixDict, CFSTR("FixIDE"),         !!(s->FixDsdt & FIX_IDE));
  addBoolean(fixDict, CFSTR("FixSATA"),        !!(s->FixDsdt & FIX_SATA));
  addBoolean(fixDict, CFSTR("FixFirewire"),    !!(s->FixDsdt & FIX_FIREWIRE));
  addBoolean(fixDict, CFSTR("FixUSB"),         !!(s->FixDsdt & FIX_USB));
  addBoolean(fixDict, CFSTR("FixLAN"),         !!(s->FixDsdt & FIX_LAN));
  addBoolean(fixDict, CFSTR("FixAirport"),     !!(s->FixDsdt & FIX_WIFI));
  addBoolean(fixDict, CFSTR("FixHDA"),         !!(s->FixDsdt & FIX_HDA));
  addBoolean(fixDict, CFSTR("FixDarwin7"),     !!(s->FixDsdt & FIX_DARWIN));
  addBoolean(fixDict, CFSTR("FixRTC"),         !!(s->FixDsdt & FIX_RTC));
  addBoolean(fixDict, CFSTR("FixTMR"),         !!(s->FixDsdt & FIX_TMR));
  addBoolean(fixDict, CFSTR("AddIMEI"),        !!(s->FixDsdt & FIX_IMEI));
  addBoolean(fixDict, CFSTR("FixIntelGfx"),    !!(s->FixDsdt & FIX_INTELGFX));
  addBoolean(fixDict, CFSTR("FixWAK"),         !!(s->FixDsdt & FIX_WAK));
  addBoolean(fixDict, CFSTR("DeleteUnused"),   !!(s->FixDsdt & FIX_UNUSED));
  addBoolean(fixDict, CFSTR("FixADP1"),        !!(s->FixDsdt & FIX_ADP1));
  addBoolean(fixDict, CFSTR("AddPNLF"),        !!(s->FixDsdt & FIX_PNLF));
  addBoolean(fixDict, CFSTR("FixS3D"),         !!(s->FixDsdt & FIX_S3D));
  addBoolean(fixDict, CFSTR("FixACST"),        !!(s->FixDsdt & FIX_ACST));
  addBoolean(fixDict, CFSTR("AddHDMI"),        !!(s->FixDsdt & FIX_HDMI));
  addBoolean(fixDict, CFSTR("FixRegions"),     !!(s->FixDsdt & FIX_REGIONS));
  addBoolean(fixDict, CFSTR("FixHeaders"),     !!(s->FixDsdt & FIX_HEADERS));

  CFMutableArrayRef dsdtPatchArray = addArray(dsdtDict, CFSTR("Patches"));
    CFMutableDictionaryRef dsdtPatchDict = addDictToArray(dsdtPatchArray);
    addString(dsdtPatchDict, CFSTR("Comment"), "This is for sample");
    addBoolean(dsdtPatchDict, CFSTR("Disabled"), TRUE);
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
    addBoolean(genDict, CFSTR("APSN"), s->GenerateAPSN);
    addBoolean(genDict, CFSTR("APLF"), s->GenerateAPLF);
    addBoolean(genDict, CFSTR("PluginType"), s->GeneratePluginType);
  addBoolean(ssdtDict, CFSTR("DropOem"), s->DropSSDT);
  addBoolean(ssdtDict, CFSTR("#DoubleFirstState"), s->DoubleFirstState);
  addInteger(ssdtDict, CFSTR("#MinMultiplier"), s->MinMultiplier);
  addInteger(ssdtDict, CFSTR("#MaxMultiplier"), s->MaxMultiplier);
  addInteger(ssdtDict, CFSTR("#PLimitDict"), s->PLimitDict);
  addInteger(ssdtDict, CFSTR("#UnderVoltStep"), s->UnderVoltStep);
  addInteger(ssdtDict, CFSTR("#PluginType"), s->PluginType);
  addBoolean(ssdtDict, CFSTR("#UseSystemIO"), s->EnableISS);
  addBoolean(ssdtDict, CFSTR("#EnableC2"), s->EnableC2);
  addBoolean(ssdtDict, CFSTR("#EnableC4"), s->EnableC4);
  addBoolean(ssdtDict, CFSTR("#EnableC6"), s->EnableC6);
  addBoolean(ssdtDict, CFSTR("#EnableC7"), s->EnableC7);
  addInteger(ssdtDict, CFSTR("#C3Latency"), s->C3Latency);
  addBoolean(ssdtDict, CFSTR("NoDynamicExtract"), s->NoDynamicExtract);

  CFMutableArrayRef dropArray = addArray(acpiDict, CFSTR("DropTables"));
  CFMutableDictionaryRef drop1Dict = addDictToArray(dropArray);
  addString(drop1Dict, CFSTR("#Signature"), "_NOT_SHOWN_");
  addString(drop1Dict, CFSTR("#TableId"), "_NOT_SHOWN_");
  addInteger(drop1Dict, CFSTR("#Length"), 0);

  CFMutableArrayRef sortedArray = addArray(acpiDict, CFSTR("#SortedOrder"));
  addStringToArray(sortedArray, "SSDT-1.aml");
  addInteger(acpiDict, CFSTR("#Sorted ACPI tables Count"), s->SortedACPICount);
  
  CFMutableDictionaryRef renameDict = addDict(acpiDict, CFSTR("#RenameDevices"));
  addString(renameDict, CFSTR("#_SB.PCI0.RP01.PXSX"), "ARPT");
  addString(renameDict, CFSTR("_SB.PCI0.RP02.PXSX"), "XHC2");
  

  // KernelAndKextPatches
  CFMutableDictionaryRef KernelAndKextPatchesDict = addDict(dict, CFSTR("KernelAndKextPatches"));
  addBoolean(KernelAndKextPatchesDict, CFSTR("#Debug"), s->KernelAndKextPatches.KPDebug);
  addBoolean(KernelAndKextPatchesDict, CFSTR("KernelCpu"), s->KernelAndKextPatches.KPKernelCpu);
  addBoolean(KernelAndKextPatchesDict, CFSTR("KernelLapic"), s->KernelAndKextPatches.KPKernelLapic);
  addBoolean(KernelAndKextPatchesDict, CFSTR("KernelXCPM"), s->KernelAndKextPatches.KPKernelXCPM);
  addBoolean(KernelAndKextPatchesDict, CFSTR("KernelPm"), s->KernelAndKextPatches.KPKernelPm);
  addBoolean(KernelAndKextPatchesDict, CFSTR("AppleIntelCPUPM"), s->KernelAndKextPatches.KPAppleIntelCPUPM);
  addBoolean(KernelAndKextPatchesDict, CFSTR("AppleRTC"), s->KernelAndKextPatches.KPAppleRTC);
  addBoolean(KernelAndKextPatchesDict, CFSTR("DellSMBIOSPatch"), s->KernelAndKextPatches.KPDELLSMBIOS);
  //addBoolean(KernelAndKextPatchesDict, CFSTR("KextPatchesAllowed"), s->KextPatchesAllowed);
  addInteger(KernelAndKextPatchesDict, CFSTR("#Number of KextsToPatch"), s->KernelAndKextPatches.NrKexts);
  addInteger(KernelAndKextPatchesDict, CFSTR("#Number of Patchs To Kernel"), s->KernelAndKextPatches.NrKernels);
  addHex(KernelAndKextPatchesDict, CFSTR("#FakeCPUID"), s->KernelAndKextPatches.FakeCPUID);

  CFMutableArrayRef KKPatchArray = addArray(KernelAndKextPatchesDict, CFSTR("#KextsToPatch"));
  CFMutableDictionaryRef patchDict1 = addDictToArray(KKPatchArray);
  addString(patchDict1, CFSTR("Comment"), "this is a sample");
  addString(patchDict1, CFSTR("#Name"), "AppleUSBXHCIPCI");
  addString(patchDict1, CFSTR("#Find"), "_NOT_SHOWN_");
  addString(patchDict1, CFSTR("#Replace"), "_NOT_SHOWN_");
  addBoolean(patchDict1, CFSTR("Disabled"), 1);
  addString(patchDict1, CFSTR("#MatchOS"), "10.11.6,10.12.x");
  addString(patchDict1, CFSTR("#MatchBuild"), "16D1111");

  CFMutableDictionaryRef rtVariablesDict = addDict(dict, CFSTR("RtVariables"));
  addString(rtVariablesDict, CFSTR("#ROM"), "UseMacAddr0");
  addString(rtVariablesDict, CFSTR("#MLB"), s->BoardSerialNumber);
  addHex(rtVariablesDict, CFSTR("CsrActiveConfig"), s->CsrActiveConfig);
  addHex(rtVariablesDict, CFSTR("BooterConfig"), s->BooterConfig);

  CFMutableArrayRef disArray = addArray(dict, CFSTR("#DisableDrivers"));
  addStringToArray(disArray, "_NOT_SHOWN_");

  CFMutableDictionaryRef bootGraphicsDict = addDict(dict, CFSTR("BootGraphics"));
  addHex(bootGraphicsDict, CFSTR("DefaultBackgroundColor"), s->DefaultBackgroundColor);
  addInteger(bootGraphicsDict, CFSTR("UIScale"), s->UIScale);
  addInteger(bootGraphicsDict, CFSTR("EFILoginHiDPI"), s->EFILoginHiDPI);
  addInteger(bootGraphicsDict, CFSTR("flagstate"), s->flagstate[0]);


  dump_plist(dict);

  printf("\nDsdtFix=%x\n", s->FixDsdt);
  printf("DsdtFix offset=%d\n", (int)offsetof(SETTINGS_DATA, FixDsdt));
  printf("HDALayoutId offset=%d\n", (int)offsetof(SETTINGS_DATA, HDALayoutId));
#if defined(MDE_CPU_IA32)
  printf("32 bit generator\n");
#elif defined(MDE_CPU_X64)
  printf("64 bit generator\n");
#else
  printf("xxx bit generator\n");
#endif


}

int main(int argc, char **argv)
{
  kern_return_t       result;
#if GFX
  CFTypeRef devProp = NULL;
//  SETTINGS settings;
  GFX_HEADER * gfx;
  const unsigned char *dataPtr = NULL;
  CFIndex    length = 0;
  CFTypeID   typeID;
#endif

  result = IOMasterPort(bootstrap_port, &masterPort);
  if (result != KERN_SUCCESS) {
    errx(1, "Error getting the IOMaster port: %s",
         mach_error_string(result));
  }
  
#if GFX
  gEFI = IORegistryEntryFromPath(masterPort, "IODeviceTree:/efi");
  if (gEFI == 0) {
    errx(1, "EFI is not supported on this system");
  }
  
  (void) GetOFVariable(gEFI, "device-properties", &devProp);
  
  // Get the OF variable's type.
  typeID = CFGetTypeID(devProp);
  
  if (typeID == CFDataGetTypeID()) {
    length = CFDataGetLength(devProp);
      if (length > 0) {
        dataPtr = CFDataGetBytePtr(devProp);
        gfx =  parse_binary(dataPtr);
      } else {
          warnx("<INVALID> Length of device-properties");
      }
  } else {
    warnx("<INVALID> Type of device-properties");
  }
#endif
  
  gPlatform = IORegistryEntryFromPath(masterPort, "IODeviceTree:/efi/platform");
  if (gPlatform == 0) {
    errx(1, "EFI is not supported on this system");
  }
  CFTypeRef data = NULL;
  result = GetOFVariable(gPlatform, "Settings", &data);
  if (result != KERN_SUCCESS) {
    errx(1, "Can not get Clover settings: %s",
         mach_error_string(result));
  }

  PrintConfig(data, gfx);
  CFRelease(data);

  IOObjectRelease(gPlatform);

  return 0;
}
