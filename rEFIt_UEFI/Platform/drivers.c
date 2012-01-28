/*
 * Copyright (c) 1999-2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 2.0 (the 'License').  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 *  drivers.c - Driver Loading Functions.
 *
 *  Copyright (c) 2000 Apple Computer, Inc.
 *
 *  DRI: Josh de Cesare
 */

#include "Platform.h"


struct Module {  
  struct Module *nextModule;
  UINT32          willLoad;
  TagPtr        dict;
  CHAR8         *plistAddr;
  UINT32          plistLength;
  CHAR8         *executablePath;
  CHAR8         *bundlePath;
  UINT32          bundlePathLength;
};
typedef struct Module Module, *ModulePtr;

struct DriverInfo {
  CHAR8*plistAddr;
  UINT32 plistLength;
  VOID *executableAddr;
  UINT32 executableLength;
  VOID *bundlePathAddr;
  UINT32 bundlePathLength;
};
typedef struct DriverInfo DriverInfo, *DriverInfoPtr;

#define kDriverPackageSignature1 'MKXT'
#define kDriverPackageSignature2 'MOSX'

struct DriversPackage {
 UINT32 signature1;
 UINT32 signature2;
 UINT32 length;
 UINT32 adler32;
 UINT32 version;
 UINT32 numDrivers;
 UINT32 reserved1;
 UINT32 reserved2;
};
typedef struct DriversPackage DriversPackage;

enum {
  kCFBundleType2,
  kCFBundleType3
};

UINT32 (*LoadExtraDrivers_p)(FileLoadDrivers_t FileLoadDrivers_p);

/*static*/ UINT32 Adler32( UINT8 * buffer, UINT32 length );

UINT32 FileLoadDrivers(CHAR8*dirSpec, UINT32 plugin);
UINT32 NetLoadDrivers(CHAR8*dirSpec);
UINT32 LoadDriverMKext(CHAR8*fileSpec);
UINT32 LoadDriverPList(CHAR8*dirSpec, CHAR8*name, UINT32 bundleType);
UINT32 LoadMatchedModules(void);

static UINT32 MatchPersonalities(void);
static UINT32 MatchLibraries(void);
#ifdef NOTDEF
static ModulePtr FindModule(CHAR8*name);
static VOID ThinFatFile(VOID **loadAddrP, UINT32 *lengthP);
#endif
static UINT32 ParseXML(CHAR8*buffer, ModulePtr *module, TagPtr *personalities);
static UINT32 InitDriverSupport(void);

ModulePtr gModuleHead, gModuleTail;
static TagPtr    gPersonalityHead, gPersonalityTail;
static CHAR8*    gExtensionsSpec;
static CHAR8*    gDriverSpec;
static CHAR8*    gFileSpec;
static CHAR8*    gTempSpec;
static CHAR8*    gFileName;

/*static*/ UINT32
Adler32( UINT8 * buffer, UINT32 length )
{
    UINT32          cnt;
    UINT32 result, lowHalf, highHalf;
    
    lowHalf  = 1;
    highHalf = 0;
  
	for ( cnt = 0; cnt < length; cnt++ )
    {
        if ((cnt % 5000) == 0)
        {
            lowHalf  %= 65521L;
            highHalf %= 65521L;
        }
    
        lowHalf  += buffer[cnt];
        highHalf += lowHalf;
    }

	lowHalf  %= 65521L;
	highHalf %= 65521L;
  
	result = (highHalf << 16) | lowHalf;
  
	return result;
}

#define kPageSize     4096
#define RoundPage(x)  ((((unsigned)(x)) + kPageSize - 1) & ~(kPageSize - 1))


UINT32
AllocateMemoryRange(char * rangeName, UINT32 start, UINT32 length, UINT32 type)
{
  CHAR8 *nameBuf;
  UINT32 *buffer;
  
  nameBuf = malloc(strlen(rangeName) + 1);
  if (nameBuf == 0) return -1;
  strcpy(nameBuf, rangeName);
  
  buffer = malloc(2 * sizeof(UINT32));
  if (buffer == 0) return -1;
  
  buffer[0] = start;
  buffer[1] = length;
  
  DT__AddProperty(gMemoryMapNode, nameBuf, 2 * sizeof(UINT32), (CHAR8 *)buffer);
  
  return 0;
}


//==========================================================================
// InitDriverSupport

static UINT32
InitDriverSupport( VOID )
{
    gExtensionsSpec = malloc( 4096 );
    gDriverSpec     = malloc( 4096 );
    gFileSpec       = malloc( 4096 );
    gTempSpec       = malloc( 4096 );
    gFileName       = malloc( 4096 );

    if ( !gExtensionsSpec || !gDriverSpec || !gFileSpec || !gTempSpec || !gFileName )
        stop("InitDriverSupport error");

    return 0;
}

//==========================================================================
// LoadDrivers

UINT32 LoadDrivers( CHAR8* dirSpec )
{
    CHAR8 dirSpecExtra[1024];

    if ( InitDriverSupport() != 0 )
        return 0;

    // Load extra drivers if a hook has been installed.
    if (LoadExtraDrivers_p != NULL)
    {
        (*LoadExtraDrivers_p)(&FileLoadDrivers);
    }

    if ( gBootFileType == kNetworkDeviceType )
    {
        if (NetLoadDrivers(dirSpec) != 0) {
            error("Could not load drivers from the network\n");
            return -1;
        }
    }
    else if ( gBootFileType == kBlockDeviceType )
	{
        // First try to load Extra extensions from the ramdisk if isn't aliased as bt(0,0).
        if (gRAMDiskVolume && !gRAMDiskBTAliased)
        {
          strcpy(dirSpecExtra, "rd(0,0)/Extra/");
          FileLoadDrivers(dirSpecExtra, 0);
        }

        // Next try to load Extra extensions from the selected root partition.
        strcpy(dirSpecExtra, "/Extra/");
        if (FileLoadDrivers(dirSpecExtra, 0) != 0)
        {
            // If failed, then try to load Extra extensions from the boot partition
	        // in case we have a separate booter partition or a bt(0,0) aliased ramdisk.
	        if ( !(gBIOSBootVolume->biosdev == gBootVolume->biosdev  && gBIOSBootVolume->part_no == gBootVolume->part_no)
	             || (gRAMDiskVolume && gRAMDiskBTAliased) )
	        {
	            // Next try a specfic OS version folder ie 10.5
                sprintf(dirSpecExtra, "bt(0,0)/Extra/%s/", &gMacOSVersion);
	            if (FileLoadDrivers(dirSpecExtra, 0) != 0)
	            {	
	                // Next we'll try the base
	                strcpy(dirSpecExtra, "bt(0,0)/Extra/");
	                FileLoadDrivers(dirSpecExtra, 0);
	            }
	        }
        }
        if(!gHaveKernelCache)
        {
            // Don't load main driver (from /System/Library/Extentions) if gHaveKernelCache is set.
            // since these drivers will already be in the kernel cache.
            // NOTE: when gHaveKernelCache, xnu cannot (by default) load *any* extra kexts from the bootloader.
            // The /Extra code is not disabled in this case due to a kernel patch that allows for this to happen.
          
            //Slice - this algo is still wrong although I reported a bug years ago       
            // Also try to load Extensions from boot helper partitions.
            if (gBootVolume->flags & kBVFlagBooter)
            {
                strcpy(dirSpecExtra, "/com.apple.boot.P/System/Library/");
                if (FileLoadDrivers(dirSpecExtra, 0) != 0)
                {
                    strcpy(dirSpecExtra, "/com.apple.boot.R/System/Library/");
                    if (FileLoadDrivers(dirSpecExtra, 0) != 0)
                    {
                        strcpy(dirSpecExtra, "/com.apple.boot.S/System/Library/");
                        FileLoadDrivers(dirSpecExtra, 0);
                    }
                }
            }
            
            if (gMKextName[0] != '\0')
            {
                MsgLog("LoadDrivers: Loading from [%s]\n", gMKextName);
                if ( LoadDriverMKext(gMKextName) != 0 )
                {
                    error("Could not load %s\n", gMKextName);
                    return -1;
                }
            }
            else
            {
                strcpy(gExtensionsSpec, dirSpec);
                strcat(gExtensionsSpec, "System/Library/");
                FileLoadDrivers(gExtensionsSpec, 0);
            }

        }
    }
    else
    {
        return 0;
    }

    MatchPersonalities();

    MatchLibraries();

    LoadMatchedModules();

    return 0;
}

//==========================================================================
// FileLoadMKext

static UINT32
FileLoadMKext( const CHAR8 * dirSpec, const CHAR8 * extDirSpec )
{
	UINT32	ret, flags, time, time2;
	char	altDirSpec[512];
	
	sprintf (altDirSpec, "%s%s", dirSpec, extDirSpec);
	ret = GetFileInfo(altDirSpec, "Extensions.mkext", &flags, &time);
	
	if ((ret == 0) && ((flags & kFileTypeMask) == kFileTypeFlat))
	{
		ret = GetFileInfo(dirSpec, "Extensions", &flags, &time2);
		
		if ((ret != 0)
			|| ((flags & kFileTypeMask) != kFileTypeDirectory)
			|| (((gBootMode & kBootModeSafe) == 0) && (time == (time2 + 1))))
		{
			sprintf(gDriverSpec, "%sExtensions.mkext", altDirSpec);
			MsgLog("LoadDrivers: Loading from [%s]\n", gDriverSpec);
			
			if (LoadDriverMKext(gDriverSpec) == 0)
				return 0;
		}
	}
	return -1;
}

//==========================================================================
// FileLoadDrivers

UINT32
FileLoadDrivers( CHAR8 * dirSpec, UINT32 plugin )
{
    UINT32         ret, length, flags, time, bundleType;
    UINT64        index;
    UINT32         result = -1;
    const CHAR8 * name;
 
    if ( !plugin )
    {
        // First try 10.6's path for loading Extensions.mkext.
        if (FileLoadMKext(dirSpec, "Caches/com.apple.kext.caches/Startup/") == 0)
            return 0;

        // Next try the legacy path.
        else if (FileLoadMKext(dirSpec, "") == 0)
          return 0;

        strcat(dirSpec, "Extensions");
    }

    index = 0;
    while (1) {
        ret = GetDirEntry(dirSpec, &index, &name, &flags, &time);
        if (ret == -1) break;

        // Make sure this is a directory.
        if ((flags & kFileTypeMask) != kFileTypeDirectory) continue;
        
        // Make sure this is a kext.
        length = strlen(name);
        if (strcmp(name + length - 5, ".kext")) continue;

        // Save the file name.
        strcpy(gFileName, name);
    
        // Determine the bundle type.
        sprintf(gTempSpec, "%s/%s", dirSpec, gFileName);
        ret = GetFileInfo(gTempSpec, "Contents", &flags, &time);
        if (ret == 0) bundleType = kCFBundleType2;
        else bundleType = kCFBundleType3;

        if (!plugin)
            sprintf(gDriverSpec, "%s/%s/%sPlugIns", dirSpec, gFileName,
                    (bundleType == kCFBundleType2) ? "Contents/" : "");

        ret = LoadDriverPList(dirSpec, gFileName, bundleType);

        if (result != 0)
          result = ret;

        if (!plugin) 
          FileLoadDrivers(gDriverSpec, 1);
    }

    return result;
}


//==========================================================================
// 

UINT32
NetLoadDrivers( CHAR8 * dirSpec )
{
    UINT32 tries;

#if NODEF
    UINT32 cnt;

    // Get the name of the kernel
    cnt = strlen(gBootFile);
    while (cnt--) {
        if ((gBootFile[cnt] == '\\')  || (gBootFile[cnt] == ',')) {
        cnt++;
        break;
        }
    }
#endif

    // INTEL modification
    sprintf(gDriverSpec, "%s%s.mkext", dirSpec, bootInfo->bootFile);
    
    MsgLog("NetLoadDrivers: Loading from [%s]\n", gDriverSpec);
    
    tries = 3;
    while (tries--)
    {
        if (LoadDriverMKext(gDriverSpec) == 0) break;
    }
    if (tries == -1) return -1;

    return 0;
}

//==========================================================================
// loadDriverMKext

UINT32
LoadDriverMKext( CHAR8 * fileSpec )
{
    UINT32    driversAddr, driversLength;
    UINT32             length;
    CHAR8             segName[32];
    DriversPackage * package;

#define GetPackageElement(e)     OSSwapBigToHostInt32(package->e)

    // Load the MKext.
    length = LoadThinFatFile(fileSpec, (VOID **)&package);
    if (length < sizeof (DriversPackage)) return -1;

	
    // Verify the MKext.
    if (( GetPackageElement(signature1) != kDriverPackageSignature1) ||
        ( GetPackageElement(signature2) != kDriverPackageSignature2) ||
        ( GetPackageElement(length)      > kLoadSize )               ||
        ( GetPackageElement(adler32)    !=
          Adler32((UINT8 *)&package->version, GetPackageElement(length) - 0x10) ) )
    {
        return -1;
    }

    // Make space for the MKext.
    driversLength = GetPackageElement(length);
    driversAddr   = AllocateKernelMemory(driversLength);

    // Copy the MKext.
    memcpy((VOID *)driversAddr, (VOID *)package, driversLength);

    // Add the MKext to the memory map.
    sprintf(segName, "DriversPackage-%lx", driversAddr);
    AllocateMemoryRange(segName, driversAddr, driversLength,
                        kBootDriverTypeMKEXT);

    return 0;
}

//==========================================================================
// LoadDriverPList

UINT32
LoadDriverPList( CHAR8 * dirSpec, CHAR8 * name, UINT32 bundleType )
{
    UINT32      length, executablePathLength, bundlePathLength;
    ModulePtr module;
    TagPtr    personalities;
    CHAR8 *    buffer = 0;
    CHAR8 *    tmpExecutablePath = 0;
    CHAR8 *    tmpBundlePath = 0;
    UINT32      ret = -1;

    do {
        // Save the driver path.
        
        if(name) sprintf(gFileSpec, "%s/%s/%s", dirSpec, name,
                (bundleType == kCFBundleType2) ? "Contents/MacOS/" : "");
        else sprintf(gFileSpec, "%s/%s", dirSpec,
                     (bundleType == kCFBundleType2) ? "Contents/MacOS/" : "");
        executablePathLength = strlen(gFileSpec) + 1;

        tmpExecutablePath = malloc(executablePathLength);
        if (tmpExecutablePath == 0) break;
        strcpy(tmpExecutablePath, gFileSpec);
  
        if(name) sprintf(gFileSpec, "%s/%s", dirSpec, name);
        else sprintf(gFileSpec, "%s", dirSpec);
        bundlePathLength = strlen(gFileSpec) + 1;

        tmpBundlePath = malloc(bundlePathLength);
        if (tmpBundlePath == 0) break;

        strcpy(tmpBundlePath, gFileSpec);

        // Construct the file spec to the plist, then load it.

        if(name) sprintf(gFileSpec, "%s/%s/%sInfo.plist", dirSpec, name,
                (bundleType == kCFBundleType2) ? "Contents/" : "");
        else sprintf(gFileSpec, "%s/%sInfo.plist", dirSpec,
                     (bundleType == kCFBundleType2) ? "Contents/" : "");

        length = LoadFile(gFileSpec);
        if (length == -1) break;
        length = length + 1;
        buffer = malloc(length);
        if (buffer == 0) break;
        strlcpy(buffer, (CHAR8 *)kLoadAddr, length);

        // Parse the plist.

        ret = ParseXML(buffer, &module, &personalities);
        if (ret != 0) { break; }
        // Allocate memory for the driver path and the plist.

        module->executablePath = tmpExecutablePath;
        module->bundlePath = tmpBundlePath;
        module->bundlePathLength = bundlePathLength;
        module->plistAddr = malloc(length);
  
        if ((module->executablePath == 0) || (module->bundlePath == 0) || (module->plistAddr == 0))
            break;
        // Save the driver path in the module.
        //strcpy(module->driverPath, tmpDriverPath);
        tmpExecutablePath = 0;
        tmpBundlePath = 0;

        // Add the plist to the module.

        strlcpy(module->plistAddr, (CHAR8 *)kLoadAddr, length);
        module->plistLength = length;
  
        // Add the module to the end of the module list.
        
        if (gModuleHead == 0)
            gModuleHead = module;
        else
            gModuleTail->nextModule = module;
        gModuleTail = module;
  
        // Add the persionalities to the personality list.
    
        if (personalities) personalities = personalities->tag;
        while (personalities != 0)
        {
            if (gPersonalityHead == 0)
                gPersonalityHead = personalities->tag;
            else
                gPersonalityTail->tagNext = personalities->tag;
            
            gPersonalityTail = personalities->tag;
            personalities = personalities->tagNext;
        }
        
        ret = 0;
    }
    while (0);
    
    if ( buffer )        free( buffer );
    if ( tmpExecutablePath ) free( tmpExecutablePath );
    if ( tmpBundlePath ) free( tmpBundlePath );

    return ret;
}


//==========================================================================
// LoadMatchedModules

UINT32
LoadMatchedModules( VOID )
{
	TagPtr		  prop;
	ModulePtr	  module;
	char		  *fileName, segName[32];
	DriverInfoPtr driver;
	UINT32		  length, driverAddr, driverLength;
	void		  *executableAddr = 0;

  
	module = gModuleHead;

	while (module != 0)
	{
		if (module->willLoad)
		{
			prop = XMLGetProperty(module->dict, kPropCFBundleExecutable);

			if (prop != 0)
			{
				fileName = prop->string;
				sprintf(gFileSpec, "%s%s", module->executablePath, fileName);
				length = LoadThinFatFile(gFileSpec, &executableAddr);
				if (length == 0)
				{
					length = LoadFile(gFileSpec);
					executableAddr = (VOID *)kLoadAddr;
				}
//				printf("%s length = %d addr = 0x%x\n", gFileSpec, length, driverModuleAddr); getchar();
			}
			else
				length = 0;

			if (length != -1)
			{
//				driverModuleAddr = (VOID *)kLoadAddr;
//				if (length != 0)
//				{
//					ThinFatFile(&driverModuleAddr, &length);
//				}

				// Make make in the image area.
                
                execute_hook("LoadMatchedModules", module, &length, executableAddr, NULL);

				driverLength = sizeof(DriverInfo) + module->plistLength + length + module->bundlePathLength;
				driverAddr = AllocateKernelMemory(driverLength);

				// Set up the DriverInfo.
				driver = (DriverInfoPtr)driverAddr;
				driver->plistAddr = (CHAR8 *)(driverAddr + sizeof(DriverInfo));
				driver->plistLength = module->plistLength;
				if (length != 0)
				{
					driver->executableAddr = (VOID *)(driverAddr + sizeof(DriverInfo) +
										 module->plistLength);
					driver->executableLength = length;
				}
				else
				{
					driver->executableAddr	 = 0;
					driver->executableLength = 0;
				}
				driver->bundlePathAddr = (VOID *)(driverAddr + sizeof(DriverInfo) +
									 module->plistLength + driver->executableLength);
				driver->bundlePathLength = module->bundlePathLength;

				// Save the plist, module and bundle.
				strcpy(driver->plistAddr, module->plistAddr);
				if (length != 0)
				{
					memcpy(driver->executableAddr, executableAddr, length);
				}
				strcpy(driver->bundlePathAddr, module->bundlePath);

				// Add an entry to the memory map.
				sprintf(segName, "Driver-%lx", (UINT32)driver);
				AllocateMemoryRange(segName, driverAddr, driverLength,
									kBootDriverTypeKEXT);
			}
		}
		module = module->nextModule;
	}

	return 0;
}

//==========================================================================
// MatchPersonalities

static UINT32
MatchPersonalities( VOID )
{
    /* IONameMatch support not implemented */
    return 0;
}

//==========================================================================
// MatchLibraries

static UINT32
MatchLibraries( VOID )
{
    TagPtr     prop, prop2;
    ModulePtr  module, module2;
    UINT32       done;

    do {
        done = 1;
        module = gModuleHead;
        
        while (module != 0)
        {
            if (module->willLoad == 1)
            {
                prop = XMLGetProperty(module->dict, kPropOSBundleLibraries);
                if (prop != 0)
                {
                    prop = prop->tag;
                    while (prop != 0)
                    {
                        module2 = gModuleHead;
                        while (module2 != 0)
                        {
                            prop2 = XMLGetProperty(module2->dict, kPropCFBundleIdentifier);
                            if ((prop2 != 0) && (!strcmp(prop->string, prop2->string)))
                            {
                                if (module2->willLoad == 0) module2->willLoad = 1;
                                break;
                            }
                            module2 = module2->nextModule;
                        }
                        prop = prop->tagNext;
                    }
                }
                module->willLoad = 2;
                done = 0;
            }
            module = module->nextModule;
        }
    }
    while (!done);

    return 0;
}


//==========================================================================
// ParseXML

static UINT32
ParseXML( CHAR8 * buffer, ModulePtr * module, TagPtr * personalities )
{
	UINT32       length, pos;
	TagPtr     moduleDict, required;
	ModulePtr  tmpModule;
  
    pos = 0;
  
    while (1)
    {
        length = XMLParseNextTag(buffer + pos, &moduleDict);
        if (length == -1) break;
    
        pos += length;
    
        if (moduleDict == 0) continue;
        if (moduleDict->type == kTagTypeDict) break;
    
        XMLFreeTag(moduleDict);
    }
  
    if (length == -1) return -1;

	required = XMLGetProperty(moduleDict, kPropOSBundleRequired);
	if ( (required == 0) ||
		(required->type != kTagTypeString) ||
		!strcmp(required->string, "Safe Boot"))
	{
		XMLFreeTag(moduleDict);
		return -2;
	}

    tmpModule = malloc(sizeof(Module));
    if (tmpModule == 0)
    {
        XMLFreeTag(moduleDict);
        return -1;
    }
    tmpModule->dict = moduleDict;
  
    // For now, load any module that has OSBundleRequired != "Safe Boot".

    tmpModule->willLoad = 1;

    *module = tmpModule;
  
    // Get the personalities.

    *personalities = XMLGetProperty(moduleDict, kPropIOKitPersonalities);
  
    return 0;
}

