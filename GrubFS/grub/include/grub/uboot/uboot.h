/* uboot.h - declare variables and functions for U-Boot support */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2013 Free Software Foundation, Inc.
 *
 *  GRUB is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GRUB is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GRUB.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GRUB_UBOOT_UBOOT_HEADER
#define GRUB_UBOOT_UBOOT_HEADER	1

#include <grub/types.h>
#include <grub/dl.h>

/* Functions.  */
void grub_uboot_mm_init (void);
void grub_uboot_init (void);
void grub_uboot_fini (void);

void grub_uboot_return (int) __attribute__ ((noreturn));

grub_addr_t grub_uboot_get_real_bss_start (void);

grub_err_t grub_uboot_probe_hardware (void);

extern grub_addr_t EXPORT_VAR (start_of_ram);

grub_uint32_t EXPORT_FUNC (grub_uboot_get_machine_type) (void);
grub_addr_t EXPORT_FUNC (grub_uboot_get_boot_data) (void);


/*
 * The U-Boot API operates through a "syscall" interface, consisting of an
 * entry point address and a set of syscall numbers. The location of this
 * entry point is described in a structure allocated on the U-Boot heap.
 * We scan through a defined region around the hint address passed to us
 * from U-Boot.
 */

#define UBOOT_API_SEARCH_LEN (3 * 1024 * 1024)
int grub_uboot_api_init (void);

/*
 * All functions below are wrappers around the uboot_syscall() function,
 * implemented in grub-core/kern/uboot/uboot.c
*/

int  grub_uboot_getc (void);
int  grub_uboot_tstc (void);
void grub_uboot_putc (int c);
void grub_uboot_puts (const char *s);

void EXPORT_FUNC (grub_uboot_reset) (void);

struct sys_info *grub_uboot_get_sys_info (void);

void grub_uboot_udelay (grub_uint32_t usec);
grub_uint32_t grub_uboot_get_timer (grub_uint32_t base);

int EXPORT_FUNC (grub_uboot_dev_enum) (void);
struct device_info * EXPORT_FUNC (grub_uboot_dev_get) (int index);
int EXPORT_FUNC (grub_uboot_dev_open) (struct device_info *dev);
int EXPORT_FUNC (grub_uboot_dev_close) (struct device_info *dev);
int grub_uboot_dev_write (struct device_info *dev, void *buf, int *len);
int grub_uboot_dev_read (struct device_info *dev, void *buf, grub_size_t blocks,
			 grub_uint32_t start, grub_size_t * real_blocks);
int EXPORT_FUNC (grub_uboot_dev_recv) (struct device_info *dev, void *buf,
				       int size, int *real_size);
int EXPORT_FUNC (grub_uboot_dev_send) (struct device_info *dev, void *buf,
				       int size);

char *grub_uboot_env_get (const char *name);
void grub_uboot_env_set (const char *name, const char *value);

#endif /* ! GRUB_UBOOT_UBOOT_HEADER */
