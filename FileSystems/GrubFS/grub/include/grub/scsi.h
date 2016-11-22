/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2008  Free Software Foundation, Inc.
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

#ifndef	GRUB_SCSI_H
#define	GRUB_SCSI_H	1

#include <grub/disk.h>

typedef struct grub_scsi_dev *grub_scsi_dev_t;

void grub_scsi_dev_register (grub_scsi_dev_t dev);
void grub_scsi_dev_unregister (grub_scsi_dev_t dev);

struct grub_scsi;

enum
  {
    GRUB_SCSI_SUBSYSTEM_USBMS,
    GRUB_SCSI_SUBSYSTEM_PATA,
    GRUB_SCSI_SUBSYSTEM_AHCI,
    GRUB_SCSI_NUM_SUBSYSTEMS
  };

extern const char grub_scsi_names[GRUB_SCSI_NUM_SUBSYSTEMS][5];

#define GRUB_SCSI_ID_SUBSYSTEM_SHIFT 24
#define GRUB_SCSI_ID_BUS_SHIFT 8
#define GRUB_SCSI_ID_LUN_SHIFT 0

static inline grub_uint32_t
grub_make_scsi_id (int subsystem, int bus, int lun)
{
  return (subsystem << GRUB_SCSI_ID_SUBSYSTEM_SHIFT)
    | (bus << GRUB_SCSI_ID_BUS_SHIFT) | (lun << GRUB_SCSI_ID_LUN_SHIFT);
}

typedef int (*grub_scsi_dev_iterate_hook_t) (int id, int bus, int luns,
					     void *data);

struct grub_scsi_dev
{
  /* Call HOOK with each device name, until HOOK returns non-zero.  */
  int (*iterate) (grub_scsi_dev_iterate_hook_t hook, void *hook_data,
		  grub_disk_pull_t pull);

  /* Open the device named NAME, and set up SCSI.  */
  grub_err_t (*open) (int id, int bus, struct grub_scsi *scsi);

  /* Close the scsi device SCSI.  */
  void (*close) (struct grub_scsi *scsi);

  /* Read SIZE bytes from the device SCSI into BUF after sending the
     command CMD of size CMDSIZE.  */
  grub_err_t (*read) (struct grub_scsi *scsi, grub_size_t cmdsize, char *cmd,
		      grub_size_t size, char *buf);

  /* Write SIZE  bytes from BUF to  the device SCSI  after sending the
     command CMD of size CMDSIZE.  */
  grub_err_t (*write) (struct grub_scsi *scsi, grub_size_t cmdsize, char *cmd,
		       grub_size_t size, const char *buf);

  /* The next scsi device.  */
  struct grub_scsi_dev *next;
};

struct grub_scsi
{
  /* The underlying scsi device.  */
  grub_scsi_dev_t dev;

  /* Type of SCSI device.  XXX: Make enum.  */
  grub_uint8_t devtype;

  int bus;

  /* Number of LUNs.  */
  int luns;

  /* LUN for this `struct grub_scsi'.  */
  int lun;

  /* Set to 0 when not removable, 1 when removable.  */
  int removable;

  /* Size of the device in blocks - 1.  */
  grub_uint64_t last_block;

  /* Size of one block.  */
  grub_uint32_t blocksize;

  /* Device-specific data.  */
  void *data;
};
typedef struct grub_scsi *grub_scsi_t;

#endif /* GRUB_SCSI_H */
