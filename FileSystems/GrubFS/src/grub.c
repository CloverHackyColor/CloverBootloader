/* grub.c - The elastic binding between grub and standalone EFI */
/*
 *  Copyright © 2014 Pete Batard <pete@akeo.ie>
 *  Based on GRUB, glibc and additional software:
 *  Copyright © 2001-2014 Free Software Foundation, Inc.
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
#include <grub/misc.h>

#include "driver.h"

void
grub_exit(void)
{
	ST->BootServices->Exit(EfiImageHandle, EFI_SUCCESS, 0, NULL);
	for (;;) ;
}

/* Screen I/O */
int grub_term_inputs = 0;

void
grub_refresh(void) { }

int
grub_getkey(void)
{
	EFI_INPUT_KEY Key;

	while (ST->ConIn->ReadKeyStroke(ST->ConIn, &Key) == EFI_NOT_READY);
	return (int) Key.UnicodeChar;
}

static void
grub_xputs_dumb(const char *str)
{
	APrint((CHAR8 *)str);
}

void (*grub_xputs)(const char *str) = grub_xputs_dumb;

/* Read an EFI shell variable */
const char *
grub_env_get(const char *var)
{
	EFI_STATUS Status;
	CHAR16 Var[64], Val[128];
	UINTN ValSize = sizeof(Val);
	static char val[128] = { 0 };

	Status = Utf8ToUtf16NoAlloc((CHAR8 *) var, Var, ARRAYSIZE(Var));
	if (EFI_ERROR(Status)) {
		PrintStatusError(Status, L"Could not convert variable name to UTF-16");
		return NULL;
	}

	Status = RT->GetVariable(Var, &ShellVariable, NULL, &ValSize, Val);
	if (EFI_ERROR(Status))
		return NULL;

	Status = Utf16ToUtf8NoAlloc(Val, val, sizeof(val));
	if (EFI_ERROR(Status)) {
		PrintStatusError(Status, L"Could not convert value '%s' to UTF-8", Val);
		return NULL;
	}

	return val;
}

/* Memory management
 * NB: We must keep track of the size allocated for grub_realloc
 */
void *
grub_malloc(grub_size_t size)
{
	grub_size_t *ptr;

	ptr = (grub_size_t *) AllocatePool(size + sizeof(grub_size_t));

	if (ptr != NULL)
		*ptr++ = size;

	return (void *) ptr;
}

void *
grub_zalloc(grub_size_t size)
{
	grub_size_t *ptr;

	ptr = (grub_size_t *) AllocateZeroPool(size + sizeof(grub_size_t));

	if (ptr != NULL)
		*ptr++ = size;

	return (void *) ptr;
}

void 
grub_free(void *p)
{
	grub_size_t *ptr = (grub_size_t *) p;

	if (ptr != NULL) {
		ptr = &ptr[-1];
        if (ptr != NULL)
        {
            FreePool(ptr);
            ptr = NULL;
        }
	}
}

void *
grub_realloc(void *p, grub_size_t new_size)
{
	grub_size_t *ptr = (grub_size_t *) p;

	if (ptr != NULL) {
		ptr = &ptr[-1];
		ptr = ReallocatePool(*ptr, new_size + sizeof(grub_size_t), ptr);
		if (ptr != NULL)
			*ptr++ = new_size;
	}
	return ptr;
}

/* Convert a grub_err_t to EFI_STATUS */
EFI_STATUS 
GrubErrToEFIStatus(grub_err_t err)
{
	// The following are defined in EFI but unused here:
	// EFI_BAD_BUFFER_SIZE
	// EFI_WRITE_PROTECTED
	// EFI_VOLUME_FULL
	// EFI_MEDIA_CHANGED
	// EFI_NO_MEDIA
	// EFI_NOT_STARTED
	// EFI_ALREADY_STARTED
	// EFI_ABORTED
	// EFI_END_OF_MEDIA
	// EFI_NO_RESPONSE
	// EFI_PROTOCOL_ERROR
	// EFI_INCOMPATIBLE_VERSION

	if ((grub_errno != 0) && (LogLevel > FS_LOGLEVEL_INFO))
		/* NB: Calling grub_print_error() will reset grub_errno */
		grub_print_error();

	switch (err) {
	case GRUB_ERR_NONE:
		return EFI_SUCCESS;

	case GRUB_ERR_BAD_MODULE:
		return EFI_LOAD_ERROR;

	case GRUB_ERR_OUT_OF_RANGE:
		return EFI_BUFFER_TOO_SMALL;

	case GRUB_ERR_OUT_OF_MEMORY:
	case GRUB_ERR_SYMLINK_LOOP:
		return EFI_OUT_OF_RESOURCES;

	case GRUB_ERR_BAD_FILE_TYPE:
		return EFI_NO_MAPPING;

	case GRUB_ERR_FILE_NOT_FOUND:
	case GRUB_ERR_UNKNOWN_DEVICE:
	case GRUB_ERR_UNKNOWN_FS:
		return EFI_NOT_FOUND;

	case GRUB_ERR_FILE_READ_ERROR:
	case GRUB_ERR_BAD_DEVICE:
	case GRUB_ERR_READ_ERROR:
	case GRUB_ERR_WRITE_ERROR:
	case GRUB_ERR_IO:
		return EFI_DEVICE_ERROR;

	case GRUB_ERR_BAD_PART_TABLE:
	case GRUB_ERR_BAD_FS:
		return EFI_VOLUME_CORRUPTED;

	case GRUB_ERR_BAD_FILENAME:
	case GRUB_ERR_BAD_ARGUMENT:
	case GRUB_ERR_BAD_NUMBER:
	case GRUB_ERR_UNKNOWN_COMMAND:
	case GRUB_ERR_INVALID_COMMAND:
		return EFI_INVALID_PARAMETER;

	case GRUB_ERR_NOT_IMPLEMENTED_YET:
		return EFI_UNSUPPORTED;

	case GRUB_ERR_TIMEOUT:
		return EFI_TIMEOUT;

	case GRUB_ERR_ACCESS_DENIED:
		return EFI_ACCESS_DENIED;

	case GRUB_ERR_WAIT:
		return EFI_NOT_READY;

	case GRUB_ERR_EXTRACTOR:
	case GRUB_ERR_BAD_COMPRESSED_DATA:
		return EFI_CRC_ERROR;

	case GRUB_ERR_EOF:
		return EFI_END_OF_FILE;

	case GRUB_ERR_BAD_SIGNATURE:
		return EFI_SECURITY_VIOLATION;

	default:
		return EFI_NOT_FOUND;
	}
}

/* The following is adapted from glibc's (offtime.c, etc.)
 */

/* How many days come before each month (0-12). */
static const unsigned short int __mon_yday[2][13] = {
	/* Normal years.  */
	{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
	/* Leap years.  */
	{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};

/* Nonzero if YEAR is a leap year (every 4 years,
   except every 100th isn't, and every 400th is). */
#define __isleap(year) \
	((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))

#define SECS_PER_HOUR         (60 * 60)
#define SECS_PER_DAY          (SECS_PER_HOUR * 24)
#define DIV(a, b)             ((a) / (b) - ((a) % (b) < 0))
#define LEAPS_THRU_END_OF(y)  (DIV (y, 4) - DIV (y, 100) + DIV (y, 400))

/* Compute an EFI_TIME representation of a GRUB's mtime_t */
VOID
GrubTimeToEfiTime(const INT32 t, EFI_TIME *tp)
{
	INT32 days, rem, y;
	const unsigned short int *ip;

	days = t / SECS_PER_DAY;
	rem = t % SECS_PER_DAY;
	while (rem < 0) {
		rem += SECS_PER_DAY;
		--days;
	}
	while (rem >= SECS_PER_DAY) {
		rem -= SECS_PER_DAY;
		++days;
	}
	tp->Hour = rem / SECS_PER_HOUR;
	rem %= SECS_PER_HOUR;
	tp->Minute = rem / 60;
	tp->Second = rem % 60;
	y = 1970;

	while (days < 0 || days >= (__isleap (y) ? 366 : 365)) {
		/* Guess a corrected year, assuming 365 days per year. */
		INT32 yg = y + days / 365 - (days % 365 < 0);

		/* Adjust DAYS and Y to match the guessed year. */
		days -= ((yg - y) * 365
			+ LEAPS_THRU_END_OF (yg - 1)
			- LEAPS_THRU_END_OF (y - 1));
		y = yg;
	}
	tp->Year = y;
	ip = __mon_yday[__isleap(y)];
	for (y = 11; days < (long int) ip[y]; --y)
		continue;
	days -= ip[y];
	tp->Month = y + 1;
	tp->Day = days + 1;
}
