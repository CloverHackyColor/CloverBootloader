/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2003, 2007, 2008, 2009  Free Software Foundation, Inc.
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

#ifndef GRUB_UTIL_GETROOT_HEADER
#define GRUB_UTIL_GETROOT_HEADER	1

#include <grub/types.h>
#include <grub/device.h>

#include <sys/types.h>
#include <stdio.h>

enum grub_dev_abstraction_types {
  GRUB_DEV_ABSTRACTION_NONE,
  GRUB_DEV_ABSTRACTION_LVM,
  GRUB_DEV_ABSTRACTION_RAID,
  GRUB_DEV_ABSTRACTION_LUKS,
  GRUB_DEV_ABSTRACTION_GELI,
};

char *grub_find_device (const char *dir, dev_t dev);
void grub_util_pull_device (const char *osname);
char **grub_guess_root_devices (const char *dir);
int grub_util_get_dev_abstraction (const char *os_dev);
char *grub_make_system_path_relative_to_its_root (const char *path);
char *
grub_make_system_path_relative_to_its_root_os (const char *path);
char *grub_util_get_grub_dev (const char *os_dev);
#if defined (__FreeBSD__) || defined(__FreeBSD_kernel__)
void grub_util_follow_gpart_up (const char *name, grub_disk_addr_t *off_out,
				char **name_out);
#endif

#include <sys/stat.h>

#ifdef __linux__
char **
grub_find_root_devices_from_mountinfo (const char *dir, char **relroot);
#endif

/* Devmapper functions provided by getroot_devmapper.c.  */
void
grub_util_pull_devmapper (const char *os_dev);
int
grub_util_device_is_mapped_stat (struct stat *st);
void grub_util_devmapper_cleanup (void);
enum grub_dev_abstraction_types
grub_util_get_dm_abstraction (const char *os_dev);
char *
grub_util_get_vg_uuid (const char *os_dev);
char *
grub_util_devmapper_part_to_disk (struct stat *st,
				  int *is_part, const char *os_dev);
char *
grub_util_get_devmapper_grub_dev (const char *os_dev);

void
grub_util_pull_lvm_by_command (const char *os_dev);
char **
grub_util_find_root_devices_from_poolname (char *poolname);

grub_disk_addr_t
grub_util_find_partition_start (const char *dev);

/* OS-specific functions provided by getroot_*.c.  */
enum grub_dev_abstraction_types
grub_util_get_dev_abstraction_os (const char *os_dev);
char *
grub_util_part_to_disk (const char *os_dev, struct stat *st,
			int *is_part);
int
grub_util_pull_device_os (const char *osname,
			  enum grub_dev_abstraction_types ab);
char *
grub_util_get_grub_dev_os (const char *os_dev);
grub_disk_addr_t
grub_util_find_partition_start_os (const char *dev);

char *
grub_util_guess_bios_drive (const char *orig_path);
char *
grub_util_guess_efi_drive (const char *orig_path);
char *
grub_util_guess_baremetal_drive (const char *orig_path);
void
grub_util_fprint_full_disk_name (FILE *f,
				 const char *drive, grub_device_t dev);

#endif /* ! GRUB_UTIL_GETROOT_HEADER */
