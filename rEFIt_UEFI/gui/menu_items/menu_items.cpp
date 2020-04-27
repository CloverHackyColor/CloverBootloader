/*
 *
 * Copyright (c) 2020 Jief
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *  * Neither the name of Christoph Pfisterer nor the names of the
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "../../Platform/Platform.h"
#include "menu_items.h"

#include "../../libeg/libeg.h"
#include "../../refit/lib.h"
#ifdef __cplusplus
#include "../../cpp_foundation/XObjArray.h"
#include "../../cpp_foundation/XString.h"
#include "../../cpp_foundation/XStringArray.h"
#include "../../libeg/XPointer.h"
#endif

REFIT_MENU_ENTRY_CLOVER* REFIT_MENU_ENTRY_CLOVER::getPartiallyDuplicatedEntry() const
{
	REFIT_MENU_ENTRY_CLOVER* DuplicateEntry = new REFIT_MENU_ENTRY_CLOVER();

    DuplicateEntry->AtClick      = ActionEnter;
    DuplicateEntry->Volume          = Volume;
    DuplicateEntry->DevicePathString= EfiStrDuplicate(DevicePathString);
    DuplicateEntry->LoadOptions     = LoadOptions;
    DuplicateEntry->LoaderPath      = LoaderPath;
	DuplicateEntry->VolName         = EfiStrDuplicate(VolName);
	DuplicateEntry->DevicePath      = DevicePath;
	DuplicateEntry->Flags           = Flags;
	return DuplicateEntry;
}

LOADER_ENTRY* LOADER_ENTRY::getPartiallyDuplicatedEntry() const
{
	LOADER_ENTRY* DuplicateEntry = new LOADER_ENTRY();

    DuplicateEntry->AtClick      = ActionEnter;
    DuplicateEntry->Volume          = Volume;
    DuplicateEntry->DevicePathString= EfiStrDuplicate(DevicePathString);
    DuplicateEntry->LoadOptions     = LoadOptions;
    DuplicateEntry->LoaderPath      = LoaderPath;
	DuplicateEntry->VolName         = EfiStrDuplicate(VolName);
	DuplicateEntry->DevicePath      = DevicePath;
	DuplicateEntry->Flags           = Flags;
	DuplicateEntry->LoaderType      = LoaderType;
	DuplicateEntry->OSVersion       = OSVersion;
	DuplicateEntry->BuildVersion    = BuildVersion;
	DuplicateEntry->KernelAndKextPatches = KernelAndKextPatches;
	return DuplicateEntry;
}

