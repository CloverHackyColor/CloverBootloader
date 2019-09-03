/* ata.h - ATA disk access.  */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifndef GRUB_ATA_HEADER
#define GRUB_ATA_HEADER 1

#include <grub/misc.h>
#include <grub/symbol.h>
#include <grub/disk.h>
/* XXX: For now this only works on i386.  */
#include <grub/cpu/io.h>

typedef enum
  {
    GRUB_ATA_CHS,
    GRUB_ATA_LBA,
    GRUB_ATA_LBA48
  } grub_ata_addressing_t;

#define GRUB_ATA_CH0_PORT1 0x1f0
#define GRUB_ATA_CH1_PORT1 0x170

#define GRUB_ATA_CH0_PORT2 0x3f6
#define GRUB_ATA_CH1_PORT2 0x376

#define GRUB_ATA_REG_DATA	0
#define GRUB_ATA_REG_ERROR	1
#define GRUB_ATA_REG_FEATURES	1
#define GRUB_ATA_REG_SECTORS	2
#define GRUB_ATAPI_REG_IREASON	2
#define GRUB_ATA_REG_SECTNUM	3
#define GRUB_ATA_REG_CYLLSB	4
#define GRUB_ATA_REG_CYLMSB	5
#define GRUB_ATA_REG_LBALOW	3
#define GRUB_ATA_REG_LBAMID	4
#define GRUB_ATAPI_REG_CNTLOW	4
#define GRUB_ATA_REG_LBAHIGH	5
#define GRUB_ATAPI_REG_CNTHIGH	5
#define GRUB_ATA_REG_DISK	6
#define GRUB_ATA_REG_CMD	7
#define GRUB_ATA_REG_STATUS	7

#define GRUB_ATA_REG2_CONTROL	0

#define GRUB_ATA_STATUS_ERR	0x01
#define GRUB_ATA_STATUS_INDEX	0x02
#define GRUB_ATA_STATUS_ECC	0x04
#define GRUB_ATA_STATUS_DRQ	0x08
#define GRUB_ATA_STATUS_SEEK	0x10
#define GRUB_ATA_STATUS_WRERR	0x20
#define GRUB_ATA_STATUS_READY	0x40
#define GRUB_ATA_STATUS_BUSY	0x80

/* ATAPI interrupt reason values (I/O, D/C bits).  */
#define GRUB_ATAPI_IREASON_MASK     0x3
#define GRUB_ATAPI_IREASON_DATA_OUT 0x0
#define GRUB_ATAPI_IREASON_CMD_OUT  0x1
#define GRUB_ATAPI_IREASON_DATA_IN  0x2
#define GRUB_ATAPI_IREASON_ERROR    0x3

enum grub_ata_commands
  {
    GRUB_ATA_CMD_CHECK_POWER_MODE	= 0xe5,
    GRUB_ATA_CMD_IDENTIFY_DEVICE	= 0xec,
    GRUB_ATA_CMD_IDENTIFY_PACKET_DEVICE	= 0xa1,
    GRUB_ATA_CMD_IDLE			= 0xe3,
    GRUB_ATA_CMD_PACKET			= 0xa0,
    GRUB_ATA_CMD_READ_SECTORS		= 0x20,
    GRUB_ATA_CMD_READ_SECTORS_EXT	= 0x24,
    GRUB_ATA_CMD_READ_SECTORS_DMA	= 0xc8,
    GRUB_ATA_CMD_READ_SECTORS_DMA_EXT	= 0x25,

    GRUB_ATA_CMD_SECURITY_FREEZE_LOCK	= 0xf5,
    GRUB_ATA_CMD_SET_FEATURES		= 0xef,
    GRUB_ATA_CMD_SLEEP			= 0xe6,
    GRUB_ATA_CMD_SMART			= 0xb0,
    GRUB_ATA_CMD_STANDBY_IMMEDIATE	= 0xe0,
    GRUB_ATA_CMD_WRITE_SECTORS		= 0x30,
    GRUB_ATA_CMD_WRITE_SECTORS_EXT	= 0x34,
    GRUB_ATA_CMD_WRITE_SECTORS_DMA_EXT	= 0x35,
    GRUB_ATA_CMD_WRITE_SECTORS_DMA	= 0xca,
  };

enum grub_ata_timeout_milliseconds
  {
    GRUB_ATA_TOUT_STD  =  1000,  /* 1s standard timeout.  */
    GRUB_ATA_TOUT_DATA = 10000,   /* 10s DATA I/O timeout.  */
    GRUB_ATA_TOUT_SPINUP  =  10000,  /* Give the device 10s on first try to spinon.  */
  };

typedef union
{
  grub_uint8_t raw[11];
  struct
  {
    union
    {
      grub_uint8_t features;
      grub_uint8_t error;
    };
    union
    {
      grub_uint8_t sectors;
      grub_uint8_t atapi_ireason;
    };
    union
    {
      grub_uint8_t lba_low;
      grub_uint8_t sectnum;
    };
    union
    {
      grub_uint8_t lba_mid;
      grub_uint8_t cyllsb;
      grub_uint8_t atapi_cntlow;
    };
    union
    {
      grub_uint8_t lba_high;
      grub_uint8_t cylmsb;
      grub_uint8_t atapi_cnthigh;
    };
    grub_uint8_t disk;
    union
    {
      grub_uint8_t cmd;
      grub_uint8_t status;
    };
    grub_uint8_t sectors48;
    grub_uint8_t lba48_low;
    grub_uint8_t lba48_mid;
    grub_uint8_t lba48_high;
  };
} grub_ata_regs_t;

/* ATA pass through parameters and function.  */
struct grub_disk_ata_pass_through_parms
{
  grub_ata_regs_t taskfile;
  void * buffer;
  grub_size_t size;
  int write;
  void *cmd;
  int cmdsize;
  int dma;
};

struct grub_ata
{
  /* Addressing methods available for accessing this device.  If CHS
     is only available, use that.  Otherwise use LBA, except for the
     high sectors.  In that case use LBA48.  */
  grub_ata_addressing_t addr;

  /* Sector count.  */
  grub_uint64_t size;
  grub_uint32_t log_sector_size;

  /* CHS maximums.  */
  grub_uint16_t cylinders;
  grub_uint16_t heads;
  grub_uint16_t sectors_per_track;

  /* Set to 0 for ATA, set to 1 for ATAPI.  */
  int atapi;

  int dma;

  grub_size_t maxbuffer;

  int *present;

  void *data;

  struct grub_ata_dev *dev;
};

typedef struct grub_ata *grub_ata_t;

typedef int (*grub_ata_dev_iterate_hook_t) (int id, int bus, void *data);

struct grub_ata_dev
{
  /* Call HOOK with each device name, until HOOK returns non-zero.  */
  int (*iterate) (grub_ata_dev_iterate_hook_t hook, void *hook_data,
		  grub_disk_pull_t pull);

  /* Open the device named NAME, and set up SCSI.  */
  grub_err_t (*open) (int id, int bus, struct grub_ata *scsi);

  /* Close the scsi device SCSI.  */
  void (*close) (struct grub_ata *ata);

  /* Read SIZE bytes from the device SCSI into BUF after sending the
     command CMD of size CMDSIZE.  */
  grub_err_t (*readwrite) (struct grub_ata *ata,
			   struct grub_disk_ata_pass_through_parms *parms,
			   int spinup);

  /* The next scsi device.  */
  struct grub_ata_dev *next;
};

typedef struct grub_ata_dev *grub_ata_dev_t;

void grub_ata_dev_register (grub_ata_dev_t dev);
void grub_ata_dev_unregister (grub_ata_dev_t dev);

#endif /* ! GRUB_ATA_HEADER */
