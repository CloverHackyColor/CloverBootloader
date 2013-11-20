/*
 * refit/scan/entry_scan.h
 *
 * Copyright (c) 2006-2010 Christoph Pfisterer
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

#include "Platform.h"

extern REFIT_MENU_ENTRY MenuEntryReturn;
extern REFIT_MENU_SCREEN MainMenu;

// common
EG_IMAGE *LoadBuiltinIcon(IN CHAR16 *IconName);
LOADER_ENTRY * DuplicateLoaderEntry(IN LOADER_ENTRY *Entry);
CHAR16 *AddLoadOption(IN CHAR16 *LoadOptions, IN CHAR16 *LoadOption);
CHAR16 *RemoveLoadOption(IN CHAR16 *LoadOptions, IN CHAR16 *LoadOption);
EG_IMAGE * ScanVolumeDefaultIcon(REFIT_VOLUME *Volume, IN UINT8 OSType);

// legacy
VOID ScanLegacy(VOID);
VOID AddCustomLegacy(VOID);

// loader
VOID ScanLoader(VOID);
VOID AddCustomEntries(VOID);

// tool
VOID ScanTool(VOID);
VOID AddCustomTool(VOID);

// secure boot
VOID AddSecureBootTool(VOID);
VOID InitializeSecureBoot(VOID);
EFI_STATUS InstallSecureBoot(VOID);
VOID UninstallSecureBoot(VOID);
VOID EnableSecureBoot(VOID);
VOID DisableSecureBoot(VOID);
