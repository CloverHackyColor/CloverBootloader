/*
   This is secret service for Clover to obtain temporary setting that user made in Options menu.
   It will work with Clover rev 1680+.
   It will not work for other bootloaders.
 
   To make it working with EFI32 it is needed to change lines 92, 93

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

#include <stdio.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOKitKeys.h>
#include <CoreFoundation/CoreFoundation.h>
#include <err.h>
#include <mach/mach_error.h>

///
/// 8-byte unsigned value.
///
typedef unsigned long long  UINT64;
///
/// 8-byte signed value.
///
typedef long long           INT64;
///
/// 4-byte unsigned value.
///
typedef unsigned int        UINT32;
///
/// 4-byte signed value.
///
typedef int                 INT32;
///
/// 2-byte unsigned value.
///
typedef unsigned short      UINT16;
///
/// 2-byte Character.  Unless otherwise specified all strings are stored in the
/// UTF-16 encoding format as defined by Unicode 2.1 and ISO/IEC 10646 standards.
///
typedef unsigned short      CHAR16;
///
/// 2-byte signed value.
///
typedef short               INT16;
///
/// Logical Boolean.  1-byte value containing 0 for FALSE or a 1 for TRUE.  Other
/// values are undefined.
///
typedef unsigned char       BOOLEAN;
///
/// 1-byte unsigned value.
///
typedef unsigned char       UINT8;
///
/// 1-byte Character
///
typedef char                CHAR8;
///
/// 1-byte signed value
///
typedef signed char         INT8;

//depending on arch of EFI but for now I will propose to use only EFI64
typedef long long                 INTN;
typedef unsigned long long        UINTN;

#define VERIFY_SIZE_OF(TYPE, Size) extern UINT8 _VerifySizeof##TYPE[(sizeof(TYPE) == (Size)) / (sizeof(TYPE) == (Size))]

//
// Verify that ProcessorBind.h produced UEFI Data Types that are compliant with
// Section 2.3.1 of the UEFI 2.3 Specification.
//
VERIFY_SIZE_OF (BOOLEAN, 1);
VERIFY_SIZE_OF (INT8, 1);
VERIFY_SIZE_OF (UINT8, 1);
VERIFY_SIZE_OF (INT16, 2);
VERIFY_SIZE_OF (UINT16, 2);
VERIFY_SIZE_OF (INT32, 4);
VERIFY_SIZE_OF (UINT32, 4);
VERIFY_SIZE_OF (INT64, 8);
VERIFY_SIZE_OF (UINT64, 8);
VERIFY_SIZE_OF (CHAR8, 1);
VERIFY_SIZE_OF (CHAR16, 2);

#pragma pack(push)
#pragma pack(1)

typedef struct {
  UINT32  Data1;
  UINT16  Data2;
  UINT16  Data3;
  UINT8   Data4[8];
} EFI_GUID;

typedef struct {

	// SMBIOS TYPE0
	CHAR8	VendorName[64];
	CHAR8	RomVersion[64];
	CHAR8	ReleaseDate[64];
	// SMBIOS TYPE1
	CHAR8	ManufactureName[64];
	CHAR8	ProductName[64];
	CHAR8	VersionNr[64];
	CHAR8	SerialNr[64];
  EFI_GUID SmUUID;
  //	CHAR8	Uuid[64];
  //	CHAR8	SKUNumber[64];
	CHAR8	FamilyName[64];
  CHAR8 OEMProduct[64];
  CHAR8 OEMVendor[64];
	// SMBIOS TYPE2
	CHAR8	BoardManufactureName[64];
	CHAR8	BoardSerialNumber[64];
	CHAR8	BoardNumber[64]; //Board-ID
	CHAR8	LocationInChassis[64];
  CHAR8 BoardVersion[64];
  CHAR8 OEMBoard[64];
  UINT8 BoardType;
  UINT8 Pad1;
	// SMBIOS TYPE3
  BOOLEAN Mobile;
  UINT8 ChassisType;
	CHAR8	ChassisManufacturer[64];
	CHAR8	ChassisAssetTag[64];
	// SMBIOS TYPE4
	UINT32	CpuFreqMHz;
	UINT32	BusSpeed; //in kHz
  BOOLEAN Turbo;
  UINT8   EnabledCores;
  UINT8   Pad2[2];
	// SMBIOS TYPE17
	CHAR8	MemoryManufacturer[64];
	CHAR8	MemorySerialNumber[64];
	CHAR8	MemoryPartNumber[64];
	CHAR8	MemorySpeed[64];
	// SMBIOS TYPE131
	UINT16	CpuType;
  // SMBIOS TYPE132
  UINT16	QPI;
  BOOLEAN TrustSMBIOS;

	// OS parameters
	CHAR8 	Language[16];
	CHAR8   BootArgs[256];
	CHAR16	CustomUuid[40];
  CHAR16  DefaultBoot[40];
  UINT16  BacklightLevel;
  BOOLEAN MemoryFix;

	// GUI parameters
	BOOLEAN	Debug;

	//ACPI
	UINT64	ResetAddr;
	UINT8 	ResetVal;
	BOOLEAN	UseDSDTmini;
	BOOLEAN	DropSSDT;
	BOOLEAN	GeneratePStates;
  BOOLEAN	GenerateCStates;
  UINT8   PLimitDict;
  UINT8   UnderVoltStep;
  BOOLEAN DoubleFirstState;
  BOOLEAN LpcTune;
  BOOLEAN EnableC2;
  BOOLEAN EnableC4;
  BOOLEAN EnableC6;
  BOOLEAN EnableISS;
  UINT16  C3Latency;
	BOOLEAN	smartUPS;
  BOOLEAN PatchNMI;
	CHAR16	DsdtName[60];
  UINT32  FixDsdt;
  BOOLEAN bDropAPIC;
  BOOLEAN bDropMCFG;
  BOOLEAN bDropHPET;
  BOOLEAN bDropECDT;
  BOOLEAN bDropDMAR;
  BOOLEAN bDropBGRT;
  //  BOOLEAN RememberBIOS;
  UINT8   MinMultiplier;
  UINT8   MaxMultiplier;
  UINT8   PluginType;


  //Injections
  BOOLEAN StringInjector;
  BOOLEAN InjectSystemID;

  //Graphics
  UINT16  PCIRootUID;
  BOOLEAN GraphicsInjector;
  BOOLEAN LoadVBios;
  BOOLEAN PatchVBios;
  void   *PatchVBiosBytes; //VBIOS_PATCH_BYTES
  UINTN   PatchVBiosBytesCount;
  BOOLEAN InjectEDID;
  UINT8   *CustomEDID;
  CHAR16  FBName[16];
  UINT16  VideoPorts;
  UINT64  VRAM;
  UINT8   Dcfg[8];
  UINT8   NVCAP[20];
  UINT32  DualLink;
  UINT32  IgPlatform;

  // HDA
  BOOLEAN HDAInjection;
  UINTN   HDALayoutId;

  // USB DeviceTree injection
  BOOLEAN USBInjection;
  // USB ownership fix
  BOOLEAN USBFixOwnership;
  BOOLEAN InjectClockID;

  // LegacyBoot
  CHAR16  LegacyBoot[32];

  // KernelAndKextPatches
  BOOLEAN KPDebug;
  BOOLEAN KPKernelCpu;
  BOOLEAN KPKextPatchesNeeded;
  BOOLEAN KPAsusAICPUPM;
  BOOLEAN KPAppleRTC;
  BOOLEAN KextPatchesAllowed;
  CHAR16  *KPATIConnectorsController;
  UINT8   *KPATIConnectorsData;
  UINTN   KPATIConnectorsDataLen;
  UINT8   *KPATIConnectorsPatch;
  INT32   NrKexts;
  void *KextPatches; //KEXT_PATCH
  //Volumes hiding
  BOOLEAN HVHideAllOSX;
  BOOLEAN HVHideAllOSXInstall;
  BOOLEAN HVHideAllRecovery;
  BOOLEAN HVHideDuplicatedBootTarget;
  BOOLEAN HVHideAllWindowsEFI;
  BOOLEAN HVHideAllGrub;
  BOOLEAN HVHideAllGentoo;
  BOOLEAN HVHideAllRedHat;
  BOOLEAN HVHideAllUbuntu;
  BOOLEAN HVHideAllLinuxMint;
  BOOLEAN HVHideAllFedora;
  BOOLEAN HVHideAllSuSe;
  BOOLEAN HVHideAllArch;
  //BOOLEAN HVHideAllUEFI;
  BOOLEAN HVHideOpticalUEFI;
  BOOLEAN HVHideInternalUEFI;
  BOOLEAN HVHideExternalUEFI;
  CHAR16 **HVHideStrings;
  INTN    HVCount;

  //Pointer
  BOOLEAN PointerEnabled;
  INTN    PointerSpeed;
  UINT64  DoubleClickTime;
  BOOLEAN PointerMirror;

  // RtVariables
  CHAR8   *RtMLB;
  UINT8   *RtROM;
  UINTN   RtROMLen;
  CHAR8   *MountEFI;
  UINT32  LogLineCount;
  CHAR8   *LogEveryBoot;

  // Multi-config
  CHAR16  ConfigName[64];
  //Drivers
  INTN     BlackListCount;
  CHAR16 **BlackList;

  //SMC keys
  CHAR8  RPlt[8];
  CHAR8  RBr[8];
  UINT8  EPCI[4];
  UINT8  REV[6];

} SETTINGS_DATA;

#pragma pack(pop)

// Prototypes
//static void UsageMessage(char *message);
//static void ParseFile(char *fileName);
static kern_return_t GetOFVariable(char *name, CFStringRef *nameRef,
                                   CFTypeRef *valueRef);

static void PrintConfig(CFTypeRef message, CFTypeRef valueRef);
static void printOpenString();
static void printCloseString();
static void printDict(char *Name);
static void printCloseDict();
static void printSubDict(char *Name);
static void printCloseSubDict();
static void printString(char *Name, char *Value);
static void printUString(char *Name, CHAR16 *Value);
static void printInteger(char *Name, int Value);
static void printHex(char *Name, int Value);
static void printBoolean(char *Name, BOOLEAN Value);
static void printUUID(char *Name, EFI_GUID *g);
static void printIntArray(char *Name, UINT8 *Value, int num);


// Global Variables
//static char                *gToolName;
static io_registry_entry_t gPlatform;
//static bool                gUseXML;
//static SETTINGS_DATA       gSettings;

int main(int argc, char **argv)
{
  mach_port_t         masterPort;
  CFStringRef         nameRef;
  CFTypeRef           valueRef;
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

  result = GetOFVariable("Settings", &nameRef, &valueRef);
  if (result != KERN_SUCCESS) {
    errx(1, "Clover absent or too old : %s",
         mach_error_string(result));
  }
  
  PrintConfig(nameRef, valueRef);
  CFRelease(nameRef);
  CFRelease(valueRef);

  IOObjectRelease(gPlatform);
  
  return 0;
}


// GetOFVariable(name, nameRef, valueRef)
//
//   Get the named firmware variable.
//   Return it and it's symbol in valueRef and nameRef.
//
static kern_return_t GetOFVariable(char *name, CFStringRef *nameRef,
				   CFTypeRef *valueRef)
{
  *nameRef = CFStringCreateWithCString(kCFAllocatorDefault, name,
				       kCFStringEncodingUTF8);
  if (*nameRef == 0) {
    errx(1, "Error creating CFString for key %s", name);
  }
  
  *valueRef = IORegistryEntryCreateCFProperty(gPlatform, *nameRef, 0, 0);
  if (*valueRef == 0){
    printf("value not found\n");
    return kIOReturnNotFound;
  }
  
  return KERN_SUCCESS;
}

static void PrintConfig(const void *key, const void *value)
{
//  long          cnt, cnt2;
  CFIndex       nameLen;
  char          *nameBuffer = 0;
  const char    *nameString;
//  char          numberBuffer[10];
  const uint8_t *dataPtr = NULL;
//  uint8_t       dataChar;
//  char          *dataBuffer = 0;
//  CFIndex       valueLen;
//  char          *valueBuffer = 0;
  const char    *valueString = 0;
  uint32_t      length = 0;
  CFTypeID      typeID;

  // Get the OF variable's name.
  nameLen = CFStringGetLength(key) + 1;
  nameBuffer = malloc(nameLen);
  if( nameBuffer && CFStringGetCString(key, nameBuffer, nameLen, kCFStringEncodingUTF8) )
    nameString = nameBuffer;
  else {
    warnx("Unable to convert property name to C string");
    nameString = "<UNPRINTABLE>";
  }

  // Get the OF variable's type.
  typeID = CFGetTypeID(value);

  if (typeID == CFDataGetTypeID()) {
    length = CFDataGetLength(value);
    if (length == 0) valueString = "";
    else {
//      dataBuffer = malloc(length * 3 + 1);
//      if (dataBuffer != 0) {
        dataPtr = CFDataGetBytePtr(value);
 /*       for (cnt = cnt2 = 0; cnt < length; cnt++) {
          dataChar = dataPtr[cnt];
          if (isprint(dataChar)) dataBuffer[cnt2++] = dataChar;
          else {
            sprintf(dataBuffer + cnt2, "%%%02x", dataChar);
            cnt2 += 3;
          }
        }
        dataBuffer[cnt2] = '\0';
        valueString = dataBuffer; */
//      }
    }
  } else {
    printf("<INVALID> settings\n");
    return;
  }
  //dump to debug
  /*
   int i,j;
   UINT8 *p = (UINT8 *)s;
   for (i=0; i<16; i++){
     for (j=0; j<16; j++) {
       printf("%02x ", p[i*16 +j]);
     }
     printf("\n");
   } */

//  printf("%s\n", nameString);
//  printf("typesize=%ld valuelength=%ld\n", sizeof(SETTINGS_DATA), (long)length);

  SETTINGS_DATA *s = (SETTINGS_DATA*)dataPtr;
  
  printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
         "<plist version=\"1.0\">\n"
         "<dict>\n");
  printDict("SystemParameters");
  printString("boot-args", s->BootArgs);
  printString("prev-lang:kbd", s->Language);
  printUString("CustomUUID", s->CustomUuid);
  printBoolean("InjectSystemID", s->InjectSystemID);
  printHex("BacklightLevel", s->BacklightLevel);
  printUString("LegacyBoot", s->LegacyBoot);
  printUString("ConfigName", s->ConfigName);
  printCloseDict();

  printDict("GUI");
  printUString("DefaultBootVolume", s->DefaultBoot);
  printBoolean("DebugLog", s->Debug);
  printSubDict("Mouse");
  printf("\t"); printBoolean("Enabled", s->PointerEnabled);
  printf("\t"); printInteger("Speed", s->PointerSpeed);
  printf("\t"); printInteger("DoubleClick", s->DoubleClickTime);
  printf("\t"); printBoolean("Mirror", s->PointerMirror);
  printCloseSubDict();
  printSubDict("Volume");
  printf("\t"); printInteger("Hide Count", s->HVCount);
  printCloseSubDict();
  printSubDict("HideEntries");
  printf("\t"); printBoolean("OSXInstall", s->HVHideAllOSXInstall);
  printf("\t"); printBoolean("Recovery", s->HVHideAllRecovery);
  printf("\t"); printBoolean("Duplicate", s->HVHideDuplicatedBootTarget);
  printf("\t"); printBoolean("WindowsEFI", s->HVHideAllWindowsEFI);
  printf("\t"); printBoolean("Ubuntu", s->HVHideAllUbuntu);
  printf("\t"); printBoolean("Grub", s->HVHideAllGrub);
  printf("\t"); printBoolean("Gentoo", s->HVHideAllGentoo);
  printf("\t"); printBoolean("OpticalUEFI", s->HVHideOpticalUEFI);
  printf("\t"); printBoolean("InternalUEFI", s->HVHideInternalUEFI);
  printf("\t"); printBoolean("ExternalUEFI", s->HVHideExternalUEFI);
  printCloseSubDict();
  printCloseDict();

  printDict("SMBIOS");
  printString("Comment", "SMBIOS TYPE0");
  printString("BiosVendor", s->VendorName);
  printString("BiosVersion", s->RomVersion);
  printString("BiosReleaseDate", s->ReleaseDate);
  printString("Comment", "SMBIOS TYPE1");
  printString("Manufacturer", s->ManufactureName);
  printString("ProductName", s->ProductName);
  printString("Version", s->VersionNr);
  printString("SerialNumber", s->SerialNr);
  printUUID("SmUUID", &s->SmUUID);
  printString("Family", s->FamilyName);
  printString("Comment", "SMBIOS TYPE2");
  printString("BoardManufacturer", s->BoardManufactureName);
  printString("BoardSerialNumber", s->BoardSerialNumber);
  printString("Board-ID", s->BoardNumber);
  printString("BoardVersion", s->BoardVersion);
  printInteger("BoardType", s->BoardType);
  printString("LocationInChassis", s->LocationInChassis);
  printString("Comment", "SMBIOS TYPE3");
  printString("ChassisManufacturer", s->ChassisManufacturer);
  printString("ChassisAssetTag", s->ChassisAssetTag);
  printHex("ChassisType", s->ChassisType);
  printBoolean("Mobile", s->Mobile);
  printString("Comment", "SMBIOS TYPE17");
  printBoolean("Trust", s->TrustSMBIOS);
  printString("Comment", "these values read only");
  printString("MemoryManufacturer", s->MemoryManufacturer);
  printString("MemorySerialNumber", s->MemorySerialNumber);
  printString("MemoryPartNumber", s->MemoryPartNumber);
  printString("MemorySpeed", s->MemorySpeed);
  printString("OEMProduct", s->OEMProduct);
  printString("OEMVendor", s->OEMVendor);
  printString("OEMBoard", s->OEMBoard);
  printCloseDict();

  printDict("CPU");
  printBoolean("Turbo", s->Turbo);
  printInteger("CpuFrequencyMHz", s->CpuFreqMHz);
  printInteger("BusSpeedkHz", s->BusSpeed);
  printInteger("QPI", s->QPI);
  printString("Comment", "these values read only");
  printHex("EnabledCores", s->EnabledCores);
  printCloseDict();

  printDict("PCI");
  printBoolean("StringInjector", s->StringInjector);
  printString("DeviceProperties", "_NOT_SHOWN_");
  printInteger("PCIRootUID", s->PCIRootUID);
  if (s->HDAInjection) {
    printInteger("HDAInjection", s->HDALayoutId);
  } else printBoolean("HDAInjection", s->HDAInjection);
  printBoolean("USBInjection", s->USBInjection);
  printBoolean("USBFixOwnership", s->USBFixOwnership);
  printBoolean("InjectClockID", s->InjectClockID);
  printBoolean("LpcTune", s->LpcTune);
  printCloseDict();

  printDict("Graphics");
  printBoolean("GraphicsInjector", s->GraphicsInjector);
  printBoolean("LoadVBios", s->LoadVBios);
  printBoolean("InjectEDID", s->InjectEDID);
  printString("CustomEDID", "_NOT_SHOWN_");
  printBoolean("PatchVBios", s->PatchVBios);
  printInteger("PatchVBios Manual Count", s->PatchVBiosBytesCount);
  printInteger("VideoPorts", s->VideoPorts);
  printInteger("VRAM", s->VRAM);
  printBoolean("DualLink", s->DualLink);
  printString("Comment", "ATI specific");
  printUString("FBName", s->FBName);
  printString("Comment", "NVIDIA specific");
  printIntArray("display-cfg", &s->Dcfg[0], 8);
  printIntArray("NVCAP", &s->NVCAP[0], 20);
  printString("Comment", "INTEL specific");
  printHex("ig-platform-id", s->IgPlatform);
  printCloseDict();

  printDict("ACPI");
  printUString("DsdtName", s->DsdtName);
  printHex("FixDsdtMask", s->FixDsdt);
  printBoolean("DropOemSSDT", s->DropSSDT);
  printBoolean("DropAPIC", s->bDropAPIC);
  printBoolean("PatchAPIC", s->PatchNMI);
  printBoolean("DropMCFG", s->bDropMCFG);
  printBoolean("DropHPET", s->bDropHPET);
  printBoolean("DropECDT", s->bDropECDT);
  printBoolean("DropDMAR", s->bDropDMAR);
  printBoolean("DropBGRT", s->bDropBGRT);
  printBoolean("GeneratePStates", s->GeneratePStates);
  printBoolean("GenerateCStates", s->GenerateCStates);
  printBoolean("DoubleFirstState", s->DoubleFirstState);
  printBoolean("EnableC2", s->EnableC2);
  printHex("C3Latency", s->C3Latency);
  printBoolean("EnableC4", s->EnableC4);
  printBoolean("EnableC6", s->EnableC6);
  printBoolean("EnableISS", s->EnableISS);
  printInteger("PLimitDict", s->PLimitDict);
  printInteger("UnderVoltStep", s->UnderVoltStep);
  printInteger("MinMultiplier", s->MinMultiplier);
  printInteger("MaxMultiplier", s->MaxMultiplier);
  printInteger("PluginType", s->PluginType);
  printBoolean("smartUPS", s->smartUPS);
  printHex("ResetAddress", s->ResetAddr);
  printHex("ResetValue", s->ResetVal);
  printCloseDict();

  printDict("KernelAndKextPatches");
  printBoolean("Debug", s->KPDebug);
  printBoolean("KernelCpu", s->KPKernelCpu);
  printBoolean("AppleRTC", s->KPAppleRTC);
  printBoolean("AsusAICPUPM", s->KPAsusAICPUPM);
  printBoolean("KextPatchesAllowed", s->KextPatchesAllowed);
  printInteger("Number_of_KextsToPatch", s->NrKexts);
  printCloseDict();

  printDict("RtVariables");
  printString("MountEFI", "_NOT_SHOWN_");
  printInteger("LogLineCount", s->LogLineCount);
  printCloseDict();

  printf("</dict>\n"
         "</plist>\n");
}

static void printOpenString()
{
  printf("\t\t<string>");
}

static void printCloseString()
{
  printf("</string>\n");
}

static void printDict(char *Name)
{
  printf("\t<key>%s</key>\n\t<dict>\n", Name);
}

static void printCloseDict()
{
  printf("\t</dict>\n");
}

static void printSubDict(char *Name)
{
  printf("\t\t<key>%s</key>\n\t\t<dict>\n", Name);
}

static void printCloseSubDict()
{
  printf("\t\t</dict>\n");
}

static void printString(char *Name, char *Value)
{
  printf("\t\t<key>%s</key>\n\t\t<string>%s</string>\n", Name, Value);
}

static void printUString(char *Name, CHAR16 *Value)
{
  int i = 0;
  printf("\t\t<key>%s</key>\n\t\t<string>", Name);
  while (Value[i]) {
    printf("%c", (char)(Value[i++] & 0xFF));
  }
  printCloseString();
}

static void printIntArray(char *Name, UINT8 *Value, int num)
{
  int i = 0;
  printf("\t\t<key>%s</key>\n\t\t<string>", Name);
  for (i=0; i<num; i++) {
    printf("%02x", Value[i]);
  }
  printCloseString();
}


static void printInteger(char *Name, int Value)
{
  printf("\t\t<key>%s</key>\n\t\t<integer>%d</integer>\n", Name, Value);
}

static void printHex(char *Name, int Value)
{
  printf("\t\t<key>%s</key>\n\t\t<string>0x%x</string>\n", Name, Value);
}

static void printBoolean(char *Name, BOOLEAN Value)
{
  if (Value) {
    printf("\t\t<key>%s</key>\n\t\t<true/>\n", Name);
  } else {
    printf("\t\t<key>%s</key>\n\t\t<false/>\n", Name);
  }
}

static void printUUID(char *Name, EFI_GUID *g)
{
  printf("\t\t<key>%s</key>\n\t\t<string>%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x</string>\n", Name,
         g->Data1, g->Data2, g->Data3,
         g->Data4[0], g->Data4[1], g->Data4[2], g->Data4[3], g->Data4[4], g->Data4[5], g->Data4[6], g->Data4[7]);
}

