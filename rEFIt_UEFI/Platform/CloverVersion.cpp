/*
 * CloverVersion.cpp
 *
 *  Created on: Apr 26, 2021
 *      Author: jief
 */

#include "CloverVersion.h"
#include "../../Version.h"


#ifdef FIRMWARE_REVISION
CONST CHAR16 *gFirmwareRevision = FIRMWARE_REVISION;
CONST CHAR8* gRevisionStr = REVISION_STR;
CONST CHAR8* gFirmwareBuildDate = FIRMWARE_BUILDDATE;
CONST CHAR8* gBuildInfo = BUILDINFOS_STR;
#else
CONST CHAR16 *gFirmwareRevision = L"unknown";
CONST CHAR8* gRevisionStr = "unknown";
CONST CHAR8* gFirmwareBuildDate = "unknown";
CONST CHAR8* gBuildInfo = NULL;
#endif
#ifdef BUILD_ID
const LString8 gBuildId __attribute__((used)) = LString8(BUILD_ID);
const LString8 gBuildIdGrepTag __attribute__((used)) = "CloverBuildIdGrepTag: " BUILD_ID;
#else
const LString8 gBuildId __attribute__((used)) = "unknown";
const LString8 gBuildIdGrepTag __attribute__((used)) = "CloverBuildIdGrepTag: " "unknown";
#endif

// __attribute__((used)) seems to not always work. So, in AboutRefit(), there is a trick to let the compiler thinks it's used.
const LString8 path_independant __attribute__((used)) = "path_independant";

