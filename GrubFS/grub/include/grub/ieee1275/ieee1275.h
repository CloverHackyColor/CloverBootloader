/* ieee1275.h - Access the Open Firmware client interface.  */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2003,2004,2005,2007,2008,2009  Free Software Foundation, Inc.
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

#ifndef GRUB_IEEE1275_HEADER
#define GRUB_IEEE1275_HEADER	1

#include <grub/err.h>
#include <grub/types.h>
#include <grub/machine/ieee1275.h>

struct grub_ieee1275_mem_region
{
  unsigned int start;
  unsigned int size;
};

#define IEEE1275_MAX_PROP_LEN	8192
#define IEEE1275_MAX_PATH_LEN	256

#ifndef IEEE1275_CALL_ENTRY_FN
#define IEEE1275_CALL_ENTRY_FN(args) (*grub_ieee1275_entry_fn) (args)
#endif

/* All backcalls to the firmware is done by calling an entry function
   which was passed to us from the bootloader.  When doing the backcall,
   a structure is passed which specifies what the firmware should do.
   NAME is the requested service.  NR_INS and NR_OUTS is the number of
   passed arguments and the expected number of return values, resp. */
struct grub_ieee1275_common_hdr
{
  grub_ieee1275_cell_t name;
  grub_ieee1275_cell_t nr_ins;
  grub_ieee1275_cell_t nr_outs;
};

#define INIT_IEEE1275_COMMON(p, xname, xins, xouts) \
  (p)->name = (grub_ieee1275_cell_t) xname; \
  (p)->nr_ins = (grub_ieee1275_cell_t) xins; \
  (p)->nr_outs = (grub_ieee1275_cell_t) xouts

typedef grub_uint32_t grub_ieee1275_ihandle_t;
typedef grub_uint32_t grub_ieee1275_phandle_t;

#define GRUB_IEEE1275_PHANDLE_INVALID  ((grub_ieee1275_phandle_t) -1)

struct grub_ieee1275_devalias
{
  char *name;
  char *path;
  char *type;
  char *parent_path;
  grub_ieee1275_phandle_t phandle;
  grub_ieee1275_phandle_t parent_dev;
};

extern void (*EXPORT_VAR(grub_ieee1275_net_config)) (const char *dev,
                                                     char **device,
                                                     char **path,
                                                     char *bootargs);

/* Maps a device alias to a pathname.  */
extern grub_ieee1275_phandle_t EXPORT_VAR(grub_ieee1275_chosen);
extern grub_ieee1275_ihandle_t EXPORT_VAR(grub_ieee1275_mmu);
#ifdef __i386__
#define GRUB_IEEE1275_ENTRY_FN_ATTRIBUTE  __attribute__ ((regparm(3)))
#else
#define GRUB_IEEE1275_ENTRY_FN_ATTRIBUTE
#endif

extern int (* EXPORT_VAR(grub_ieee1275_entry_fn)) (void *) GRUB_IEEE1275_ENTRY_FN_ATTRIBUTE;

/* Static heap, used only if FORCE_CLAIM is set,
   happens on Open Hack'Ware. Should be in platform-specific
   header but is used only on PPC anyway.
*/
#define GRUB_IEEE1275_STATIC_HEAP_START 0x1000000
#define GRUB_IEEE1275_STATIC_HEAP_LEN   0x1000000


enum grub_ieee1275_flag
{
  /* Old World Macintosh firmware fails seek when "dev:0" is opened.  */
  GRUB_IEEE1275_FLAG_NO_PARTITION_0,

  /* Apple firmware runs in translated mode and requires use of the "map"
     method.  Other firmware runs in untranslated mode and doesn't like "map"
     calls.  */
  GRUB_IEEE1275_FLAG_REAL_MODE,

  /* CHRP specifies partitions are numbered from 1 (partition 0 refers to the
     whole disk). However, CodeGen firmware numbers partitions from 0.  */
  GRUB_IEEE1275_FLAG_0_BASED_PARTITIONS,

  /* CodeGen firmware does not correctly implement "output-device output" */
  GRUB_IEEE1275_FLAG_BROKEN_OUTPUT,

  /* OLPC / XO firmware hangs when accessing USB devices.  */
  GRUB_IEEE1275_FLAG_OFDISK_SDCARD_ONLY,

  /* Open Hack'Ware stops when trying to set colors */
  GRUB_IEEE1275_FLAG_CANNOT_SET_COLORS,

  /* Open Hack'Ware stops when grub_ieee1275_interpret is used.  */
  GRUB_IEEE1275_FLAG_CANNOT_INTERPRET,

  /* Open Hack'Ware has no memory map, just claim what we need.  */
  GRUB_IEEE1275_FLAG_FORCE_CLAIM,

  /* Open Hack'Ware don't support the ANSI sequence.  */
  GRUB_IEEE1275_FLAG_NO_ANSI,

  /* OpenFirmware hangs on qemu if one requests any memory below 1.5 MiB.  */
  GRUB_IEEE1275_FLAG_NO_PRE1_5M_CLAIM,

  /* OLPC / XO firmware has the cursor ON/OFF routines.  */
  GRUB_IEEE1275_FLAG_HAS_CURSORONOFF,

  /* Some PowerMacs claim to use 2 address cells but in fact use only 1. 
     Other PowerMacs claim to use only 1 and really do so. Always assume
     1 address cell is used on PowerMacs.
   */
  GRUB_IEEE1275_FLAG_BROKEN_ADDRESS_CELLS,

  GRUB_IEEE1275_FLAG_NO_TREE_SCANNING_FOR_DISKS,

  GRUB_IEEE1275_FLAG_NO_OFNET_SUFFIX,

  GRUB_IEEE1275_FLAG_VIRT_TO_REAL_BROKEN,

  GRUB_IEEE1275_FLAG_BROKEN_REPEAT,

  GRUB_IEEE1275_FLAG_CURSORONOFF_ANSI_BROKEN,
};

extern int EXPORT_FUNC(grub_ieee1275_test_flag) (enum grub_ieee1275_flag flag);
extern void EXPORT_FUNC(grub_ieee1275_set_flag) (enum grub_ieee1275_flag flag);




void EXPORT_FUNC(grub_ieee1275_init) (void);
int EXPORT_FUNC(grub_ieee1275_finddevice) (const char *name,
					   grub_ieee1275_phandle_t *phandlep);
int EXPORT_FUNC(grub_ieee1275_get_property) (grub_ieee1275_phandle_t phandle,
					     const char *property, void *buf,
					     grub_size_t size,
					     grub_ssize_t *actual);
int EXPORT_FUNC(grub_ieee1275_get_integer_property) (grub_ieee1275_phandle_t phandle,
						     const char *property, grub_uint32_t *buf,
						     grub_size_t size,
						     grub_ssize_t *actual);
int EXPORT_FUNC(grub_ieee1275_next_property) (grub_ieee1275_phandle_t phandle,
					      char *prev_prop, char *prop);
int EXPORT_FUNC(grub_ieee1275_get_property_length)
     (grub_ieee1275_phandle_t phandle, const char *prop, grub_ssize_t *length);
int EXPORT_FUNC(grub_ieee1275_instance_to_package)
     (grub_ieee1275_ihandle_t ihandle, grub_ieee1275_phandle_t *phandlep);
int EXPORT_FUNC(grub_ieee1275_package_to_path) (grub_ieee1275_phandle_t phandle,
						char *path, grub_size_t len,
						grub_ssize_t *actual);
int EXPORT_FUNC(grub_ieee1275_instance_to_path)
     (grub_ieee1275_ihandle_t ihandle, char *path, grub_size_t len,
      grub_ssize_t *actual);
int EXPORT_FUNC(grub_ieee1275_write) (grub_ieee1275_ihandle_t ihandle,
				      const void *buffer, grub_size_t len,
				      grub_ssize_t *actualp);
int EXPORT_FUNC(grub_ieee1275_read) (grub_ieee1275_ihandle_t ihandle,
				     void *buffer, grub_size_t len,
				     grub_ssize_t *actualp);
int EXPORT_FUNC(grub_ieee1275_seek) (grub_ieee1275_ihandle_t ihandle,
				     grub_disk_addr_t pos,
				     grub_ssize_t *result);
int EXPORT_FUNC(grub_ieee1275_peer) (grub_ieee1275_phandle_t node,
				     grub_ieee1275_phandle_t *result);
int EXPORT_FUNC(grub_ieee1275_child) (grub_ieee1275_phandle_t node,
				      grub_ieee1275_phandle_t *result);
int EXPORT_FUNC(grub_ieee1275_parent) (grub_ieee1275_phandle_t node,
				       grub_ieee1275_phandle_t *result);
int EXPORT_FUNC(grub_ieee1275_interpret) (const char *command,
					  grub_ieee1275_cell_t *catch);
int EXPORT_FUNC(grub_ieee1275_enter) (void);
void EXPORT_FUNC(grub_ieee1275_exit) (void) __attribute__ ((noreturn));
int EXPORT_FUNC(grub_ieee1275_open) (const char *node,
				     grub_ieee1275_ihandle_t *result);
int EXPORT_FUNC(grub_ieee1275_close) (grub_ieee1275_ihandle_t ihandle);
int EXPORT_FUNC(grub_ieee1275_claim) (grub_addr_t addr, grub_size_t size,
				      unsigned int align, grub_addr_t *result);
int EXPORT_FUNC(grub_ieee1275_release) (grub_addr_t addr, grub_size_t size);
int EXPORT_FUNC(grub_ieee1275_set_property) (grub_ieee1275_phandle_t phandle,
					     const char *propname,
					     const void *buf,
					     grub_size_t size,
					     grub_ssize_t *actual);
int EXPORT_FUNC(grub_ieee1275_set_color) (grub_ieee1275_ihandle_t ihandle,
					  int index, int r, int g, int b);
int EXPORT_FUNC(grub_ieee1275_milliseconds) (grub_uint32_t *msecs);


grub_err_t EXPORT_FUNC(grub_claimmap) (grub_addr_t addr, grub_size_t size);

int
EXPORT_FUNC(grub_ieee1275_map) (grub_addr_t phys, grub_addr_t virt,
				grub_size_t size, grub_uint32_t mode);

char *EXPORT_FUNC(grub_ieee1275_encode_devname) (const char *path);
char *EXPORT_FUNC(grub_ieee1275_get_filename) (const char *path);
int EXPORT_FUNC(grub_ieee1275_devices_iterate) (int (*hook)
						(struct grub_ieee1275_devalias *
						 alias));
char *EXPORT_FUNC(grub_ieee1275_get_aliasdevname) (const char *path);
char *EXPORT_FUNC(grub_ieee1275_canonicalise_devname) (const char *path);
char *EXPORT_FUNC(grub_ieee1275_get_device_type) (const char *path);
char *EXPORT_FUNC(grub_ieee1275_get_devname) (const char *path);

void EXPORT_FUNC(grub_ieee1275_devalias_init_iterator) (struct grub_ieee1275_devalias *alias);
void EXPORT_FUNC(grub_ieee1275_devalias_free) (struct grub_ieee1275_devalias *alias);
int EXPORT_FUNC(grub_ieee1275_devalias_next) (struct grub_ieee1275_devalias *alias);
void EXPORT_FUNC(grub_ieee1275_children_peer) (struct grub_ieee1275_devalias *alias);
void EXPORT_FUNC(grub_ieee1275_children_first) (const char *devpath,
						struct grub_ieee1275_devalias *alias);

#define FOR_IEEE1275_DEVALIASES(alias) for (grub_ieee1275_devalias_init_iterator (&(alias)); grub_ieee1275_devalias_next (&(alias));)

#define FOR_IEEE1275_DEVCHILDREN(devpath, alias) for (grub_ieee1275_children_first ((devpath), &(alias)); \
						      (alias).name;	\
						      grub_ieee1275_children_peer (&(alias)))

#endif /* ! GRUB_IEEE1275_HEADER */
