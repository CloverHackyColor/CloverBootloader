/* grub_driver.c - Data that needs to be altered for each driver */
/*
 *  Copyright © 2014 Pete Batard <pete@akeo.ie>
 *  Based on GRUB  --  GRand Unified Bootloader
 *  Copyright © 2001-2014 Free Software Foundation, Inc.
 *  Path sanitation code by Ludwig Nussel <ludwig.nussel@suse.de>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <grub/err.h>

#include "driver.h"

/* Setup generic function calls for grub_<fs>_init and grub_<fs>_exit */
#define MAKE_FN_NAME(drivername, suffix) grub_ ## drivername ## _ ## suffix
#define GRUB_FS_CALL(drivername, suffix) MAKE_FN_NAME(drivername, suffix)
extern void GRUB_FS_CALL(DRIVERNAME, init)(void);
extern void GRUB_FS_CALL(DRIVERNAME, fini)(void);
#if defined(EXTRAMODULE)
extern void GRUB_FS_CALL(EXTRAMODULE, init)(void);
extern void GRUB_FS_CALL(EXTRAMODULE, fini)(void);
#endif
#if defined(EXTRAMODULE2)
extern void GRUB_FS_CALL(EXTRAMODULE2, init)(void);
extern void GRUB_FS_CALL(EXTRAMODULE2, fini)(void);
#endif

extern LIST_ENTRY FsListHead;

CHAR16 *DriverNameString = L"efifs " WIDEN(STRINGIFY(FS_DRIVER_VERSION_MAJOR)) L"."
		WIDEN(STRINGIFY(FS_DRIVER_VERSION_MINOR)) L"." WIDEN(STRINGIFY(FS_DRIVER_VERSION_MICRO))
		L" " WIDEN(STRINGIFY(DRIVERNAME)) L" driver (" WIDEN(PACKAGE_STRING) L")";

/**
 * Set a filesystem GUID according to the filesystem name
 * We use a static ID for the first 8 bytes, and then roll the lowercase name
 * for the last 8 bytes (eg. exfat => {'e', 'x', 'f', 'a', 't', 'e', 'x', 'f'})
 */
EFI_GUID *
GetFSGuid(VOID)
{
	INTN i, k, Len = StrLen(WIDEN(STRINGIFY(DRIVERNAME)));
  UINTN j;
	static EFI_GUID Guid = { 0xEF1F5EF1, 0xF17E, 0x5857, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
	CHAR16 *FsName = WIDEN(STRINGIFY(DRIVERNAME));
	const CHAR16 *PlusName = L"plus";
    EFI_UNICODE_COLLATION_PROTOCOL *UnicodeCollation;
	UINT8 Data4[12];	/* +4 so that we can also reduce something like "1234567plus" into "1234567+" */
    EFI_STATUS Status = EFI_SUCCESS;

    Status = gBS->LocateProtocol(&gEfiUnicodeCollation2ProtocolGuid, NULL, (VOID**)&UnicodeCollation);
    if (EFI_ERROR(Status) || (UnicodeCollation == NULL))
    {
      Status = gBS->LocateProtocol(&gEfiUnicodeCollationProtocolGuid, NULL, (VOID**)&UnicodeCollation);
    }

    if (EFI_ERROR(Status) || (UnicodeCollation == NULL))
    {
      Print(L"ERROR: Could not open unicode collation protocol\n");

      return NULL;
    }

	UnicodeCollation->StrLwr(UnicodeCollation, FsName);

	for (i = 0, j = 0, k = 0; j < ARRAYSIZE(Data4); i = (i+1)%Len, j++) {
		/* Convert any 'plus' that is part of the name to '+' */
		if (FsName[i] == PlusName[k]) {
			if (++k == 4) {
				k = 0;
				j -= 3;
				Data4[j] = (UINT8) '+';
			} else {
				Data4[j] = FsName[i];
			}
		} else {
			k = 0;
			Data4[j] = FsName[i];
		}
	}

    if (FsName != NULL)
    {
        FreePool(FsName);
        FsName = NULL;
    }

        
    CopyMem(Guid.Data4, Data4, 8);

	return &Guid;
}

VOID
GrubDriverInit(VOID)
{
	/* Register the relevant GRUB filesystem module */
	GRUB_FS_CALL(DRIVERNAME, init)();
	/* The GRUB compression routines are registered as extra module(s) */
	// TODO: Eventually, we could try to turn each GRUB module into their
	// own EFI driver, have them register their interface and consume that.
#if defined(EXTRAMODULE)
	GRUB_FS_CALL(EXTRAMODULE, init)();
#endif
#if defined(EXTRAMODULE2)
	GRUB_FS_CALL(EXTRAMODULE2, init)();
#endif

	InitializeListHead(&FsListHead);
}

VOID
GrubDriverExit(VOID)
{
	/* Unregister the relevant grub module */
	GRUB_FS_CALL(DRIVERNAME, fini)();
#if defined(EXTRAMODULE)
	GRUB_FS_CALL(EXTRAMODULE, fini)();
#endif
#if defined(EXTRAMODULE2)
	GRUB_FS_CALL(EXTRAMODULE2, fini)();
#endif
}
