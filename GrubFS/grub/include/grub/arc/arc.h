/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2011 Free Software Foundation, Inc.
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

#ifndef GRUB_ARC_HEADER
#define GRUB_ARC_HEADER	1

#include <grub/types.h>
#include <grub/symbol.h>

typedef unsigned grub_arc_enum_t;
typedef grub_uint64_t grub_arc_ularge_t;
typedef unsigned long grub_arc_ulong_t;
typedef long grub_arc_long_t;
typedef unsigned short grub_arc_ushort_t;
typedef unsigned char grub_arc_uchar_t;

typedef grub_arc_long_t grub_arc_err_t;
typedef grub_arc_ulong_t grub_arc_fileno_t;

struct grub_arc_memory_descriptor
{
  grub_arc_enum_t type;
  grub_arc_ulong_t start_page;
  grub_arc_ulong_t num_pages;
};

enum grub_arc_memory_type
  {
    GRUB_ARC_MEMORY_EXCEPTION_BLOCK,
    GRUB_ARC_MEMORY_SYSTEM_PARAMETER_BLOCK,
#ifdef GRUB_CPU_WORDS_BIGENDIAN
    GRUB_ARC_MEMORY_FREE_CONTIGUOUS,
#endif
    GRUB_ARC_MEMORY_FREE,
    GRUB_ARC_MEMORY_BADRAM,
    GRUB_ARC_MEMORY_LOADED,    GRUB_ARC_MEMORY_FW_TEMPORARY,
    GRUB_ARC_MEMORY_FW_PERMANENT,
#ifndef GRUB_CPU_WORDS_BIGENDIAN
    GRUB_ARC_MEMORY_FREE_CONTIGUOUS,
#endif
  } grub_arc_memory_type_t;

struct grub_arc_timeinfo
{
  grub_arc_ushort_t y;
  grub_arc_ushort_t m;
  grub_arc_ushort_t d;
  grub_arc_ushort_t h;
  grub_arc_ushort_t min;
  grub_arc_ushort_t s;
  grub_arc_ushort_t ms;
};

struct grub_arc_display_status
{
  grub_arc_ushort_t x;
  grub_arc_ushort_t y;
  grub_arc_ushort_t w;
  grub_arc_ushort_t h;
  grub_arc_uchar_t fgcolor;
  grub_arc_uchar_t bgcolor;
  grub_arc_uchar_t high_intensity;
  grub_arc_uchar_t underscored;
  grub_arc_uchar_t reverse_video;
};

enum
  {
    GRUB_ARC_COMPONENT_FLAG_OUT = 0x40,
    GRUB_ARC_COMPONENT_FLAG_IN = 0x20,
  };

struct grub_arc_component
{
  grub_arc_enum_t class;
  grub_arc_enum_t type;
  grub_arc_enum_t flags;
  grub_arc_ushort_t version;
  grub_arc_ushort_t rev;
  grub_arc_ulong_t key;
  grub_arc_ulong_t affinity;
  grub_arc_ulong_t configdatasize;
  grub_arc_ulong_t idlen;
  const char *idstr;
};

enum
  {
#ifdef GRUB_CPU_WORDS_BIGENDIAN
    GRUB_ARC_COMPONENT_TYPE_ARC = 1,
#else
    GRUB_ARC_COMPONENT_TYPE_ARC,
#endif
    GRUB_ARC_COMPONENT_TYPE_CPU,
    GRUB_ARC_COMPONENT_TYPE_FPU,
    GRUB_ARC_COMPONENT_TYPE_PRI_I_CACHE,
    GRUB_ARC_COMPONENT_TYPE_PRI_D_CACHE,
    GRUB_ARC_COMPONENT_TYPE_SEC_I_CACHE,
    GRUB_ARC_COMPONENT_TYPE_SEC_D_CACHE,
    GRUB_ARC_COMPONENT_TYPE_SEC_CACHE,
    GRUB_ARC_COMPONENT_TYPE_EISA,
    GRUB_ARC_COMPONENT_TYPE_TCA,
    GRUB_ARC_COMPONENT_TYPE_SCSI,
    GRUB_ARC_COMPONENT_TYPE_DTIA,
    GRUB_ARC_COMPONENT_TYPE_MULTIFUNC,
    GRUB_ARC_COMPONENT_TYPE_DISK_CONTROLLER,
    GRUB_ARC_COMPONENT_TYPE_TAPE_CONTROLLER,
    GRUB_ARC_COMPONENT_TYPE_CDROM_CONTROLLER,
    GRUB_ARC_COMPONENT_TYPE_WORM_CONTROLLER,
    GRUB_ARC_COMPONENT_TYPE_SERIAL_CONTROLLER,
    GRUB_ARC_COMPONENT_TYPE_NET_CONTROLLER,
    GRUB_ARC_COMPONENT_TYPE_DISPLAY_CONTROLLER,
    GRUB_ARC_COMPONENT_TYPE_PARALLEL_CONTROLLER,
    GRUB_ARC_COMPONENT_TYPE_POINTER_CONTROLLER,
    GRUB_ARC_COMPONENT_TYPE_KBD_CONTROLLER,
    GRUB_ARC_COMPONENT_TYPE_AUDIO_CONTROLLER,
    GRUB_ARC_COMPONENT_TYPE_OTHER_CONTROLLER,
    GRUB_ARC_COMPONENT_TYPE_DISK,
    GRUB_ARC_COMPONENT_TYPE_FLOPPY,
    GRUB_ARC_COMPONENT_TYPE_TAPE,
    GRUB_ARC_COMPONENT_TYPE_MODEM,
    GRUB_ARC_COMPONENT_TYPE_MONITOR,
    GRUB_ARC_COMPONENT_TYPE_PRINTER,
    GRUB_ARC_COMPONENT_TYPE_POINTER,
    GRUB_ARC_COMPONENT_TYPE_KBD,
    GRUB_ARC_COMPONENT_TYPE_TERMINAL,
#ifndef GRUB_CPU_WORDS_BIGENDIAN
    GRUB_ARC_COMPONENT_TYPE_OTHER_PERIPHERAL,
#endif
    GRUB_ARC_COMPONENT_TYPE_LINE,
    GRUB_ARC_COMPONENT_TYPE_NET,
    GRUB_ARC_COMPONENT_TYPE_MEMORY_UNIT,
  };

enum grub_arc_file_access
  {
    GRUB_ARC_FILE_ACCESS_OPEN_RO,
    GRUB_ARC_FILE_ACCESS_OPEN_WO,
    GRUB_ARC_FILE_ACCESS_OPEN_RW,
  };

struct grub_arc_fileinfo
{
  grub_arc_ularge_t start;
  grub_arc_ularge_t end;
  grub_arc_ularge_t current;
  grub_arc_enum_t type;
  grub_arc_ulong_t fnamelength;
  grub_arc_uchar_t attr;
  char filename[32];
};

struct grub_arc_firmware_vector
{
  /* 0x00. */
  void *load;
  void *invoke;
  void *execute;
  void *halt;

  /* 0x10. */
  void (*powerdown) (void);
  void (*restart) (void);
  void (*reboot) (void);
  void (*exit) (void);

  /* 0x20. */
  void *reserved1;
  const struct grub_arc_component * (*getpeer) (const struct grub_arc_component *comp);
  const struct grub_arc_component * (*getchild) (const struct grub_arc_component *comp);
  void *getparent;
  
  /* 0x30. */
  void *getconfigurationdata;
  void *addchild;
  void *deletecomponent;
  void *getcomponent;

  /* 0x40. */
  void *saveconfiguration;
  void *getsystemid;
  struct grub_arc_memory_descriptor *(*getmemorydescriptor) (struct grub_arc_memory_descriptor *current);
  void *reserved2;

  /* 0x50. */
  struct grub_arc_timeinfo *(*gettime) (void);
  void *getrelativetime;
  void *getdirectoryentry;
  grub_arc_err_t (*open) (const char *path, grub_arc_enum_t mode,
			  grub_arc_fileno_t *fileno);

  /* 0x60. */
  grub_arc_err_t (*close) (grub_arc_fileno_t fileno);
  grub_arc_err_t (*read) (grub_arc_fileno_t fileno, void *buf,
			  grub_arc_ulong_t n,
			  grub_arc_ulong_t *count);
  grub_arc_err_t (*get_read_status) (grub_arc_fileno_t fileno);
  grub_arc_err_t (*write) (grub_arc_fileno_t fileno, const void *buf,
			   grub_arc_ulong_t n,
			   grub_arc_ulong_t *count);

  /* 0x70. */
  grub_arc_err_t (*seek) (grub_arc_fileno_t fileno,
			  grub_arc_ularge_t *pos, grub_arc_enum_t mode);
  void *mount;
  const char * (*getenvironmentvariable) (const char *name);
  void *setenvironmentvariable;

  /* 0x80. */
  grub_arc_err_t (*getfileinformation) (grub_arc_fileno_t fileno,
					struct grub_arc_fileinfo *info);
  void *setfileinformation;
  void *flushallcaches;
  void *testunicodecharacter;
  
  /* 0x90. */
  struct grub_arc_display_status * (*getdisplaystatus) (grub_arc_fileno_t fileno);
};

struct grub_arc_adapter
{
  grub_arc_ulong_t adapter_type;
  grub_arc_ulong_t adapter_vector_length;
  void *adapter_vector;
};

struct grub_arc_system_parameter_block
{
  grub_arc_ulong_t signature;
  grub_arc_ulong_t length;
  grub_arc_ushort_t version;
  grub_arc_ushort_t revision;
  void *restartblock;
  void *debugblock;
  void *gevector;
  void *utlbmissvector;
  grub_arc_ulong_t firmware_vector_length;
  struct grub_arc_firmware_vector *firmwarevector;
  grub_arc_ulong_t private_vector_length;
  void *private_vector;
  grub_arc_ulong_t adapter_count;
  struct grub_arc_adapter adapters[0];
};


#define GRUB_ARC_SYSTEM_PARAMETER_BLOCK ((struct grub_arc_system_parameter_block *) 0xa0001000)
#define GRUB_ARC_FIRMWARE_VECTOR (GRUB_ARC_SYSTEM_PARAMETER_BLOCK->firmwarevector)
#define GRUB_ARC_STDIN 0
#define GRUB_ARC_STDOUT 1

typedef int (*grub_arc_iterate_devs_hook_t)
  (const char *name, const struct grub_arc_component *comp, void *data);

int EXPORT_FUNC (grub_arc_iterate_devs) (grub_arc_iterate_devs_hook_t hook,
					 void *hook_data,
					 int alt_names);

char *EXPORT_FUNC (grub_arc_alt_name_to_norm) (const char *name, const char *suffix);

int EXPORT_FUNC (grub_arc_is_device_serial) (const char *name, int alt_names);


#define FOR_ARC_CHILDREN(comp, parent) for (comp = GRUB_ARC_FIRMWARE_VECTOR->getchild (parent); comp; comp = GRUB_ARC_FIRMWARE_VECTOR->getpeer (comp))

extern void grub_arcdisk_init (void);
extern void grub_arcdisk_fini (void);


#endif
