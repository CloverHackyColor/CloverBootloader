/* cmain.c - Startup code for the PowerPC.  */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2003,2004,2005,2006,2007,2008  Free Software Foundation, Inc.
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

#include <grub/kernel.h>
#include <grub/misc.h>
#include <grub/types.h>
#include <grub/ieee1275/ieee1275.h>

int (*grub_ieee1275_entry_fn) (void *) GRUB_IEEE1275_ENTRY_FN_ATTRIBUTE;

grub_ieee1275_phandle_t grub_ieee1275_chosen;
grub_ieee1275_ihandle_t grub_ieee1275_mmu;

static grub_uint32_t grub_ieee1275_flags;



int
grub_ieee1275_test_flag (enum grub_ieee1275_flag flag)
{
  return (grub_ieee1275_flags & (1 << flag));
}

void
grub_ieee1275_set_flag (enum grub_ieee1275_flag flag)
{
  grub_ieee1275_flags |= (1 << flag);
}

static void
grub_ieee1275_find_options (void)
{
  grub_ieee1275_phandle_t root;
  grub_ieee1275_phandle_t options;
  grub_ieee1275_phandle_t openprom;
  grub_ieee1275_phandle_t bootrom;
  int rc;
  grub_uint32_t realmode = 0;
  char tmp[256];
  int is_smartfirmware = 0;
  int is_olpc = 0;
  int is_qemu = 0;
  grub_ssize_t actual;

#ifdef __sparc__
  grub_ieee1275_set_flag (GRUB_IEEE1275_FLAG_NO_PARTITION_0);
#endif

  grub_ieee1275_finddevice ("/", &root);
  grub_ieee1275_finddevice ("/options", &options);
  grub_ieee1275_finddevice ("/openprom", &openprom);

  rc = grub_ieee1275_get_integer_property (options, "real-mode?", &realmode,
					   sizeof realmode, 0);
  if (((rc >= 0) && realmode) || (grub_ieee1275_mmu == 0))
    grub_ieee1275_set_flag (GRUB_IEEE1275_FLAG_REAL_MODE);

  rc = grub_ieee1275_get_property (openprom, "CodeGen-copyright",
				   tmp,	sizeof (tmp), 0);
  if (rc >= 0 && !grub_strncmp (tmp, "SmartFirmware(tm)",
				sizeof ("SmartFirmware(tm)") - 1))
    is_smartfirmware = 1;

  rc = grub_ieee1275_get_property (root, "architecture",
				   tmp,	sizeof (tmp), 0);
  if (rc >= 0 && !grub_strcmp (tmp, "OLPC"))
    is_olpc = 1;

  rc = grub_ieee1275_get_property (root, "model",
				   tmp,	sizeof (tmp), 0);
  if (rc >= 0 && (!grub_strcmp (tmp, "Emulated PC")
		  || !grub_strcmp (tmp, "IBM pSeries (emulated by qemu)"))) {
    is_qemu = 1;
  }

  if (rc >= 0 && grub_strncmp (tmp, "IBM", 3) == 0)
    grub_ieee1275_set_flag (GRUB_IEEE1275_FLAG_NO_TREE_SCANNING_FOR_DISKS);

  /* Old Macs have no key repeat, newer ones have fully working one.
     The ones inbetween when repeated key generates an escaoe sequence
     only the escape is repeated. With this workaround however a fast
     e.g. down arrow-ESC is perceived as down arrow-down arrow which is
     also annoying but is less so than the original bug of exiting from
     the current window on arrow repeat. To avoid unaffected users suffering
     from this workaround match only exact models known to have this bug.
   */
  if (rc >= 0 && grub_strcmp (tmp, "PowerBook3,3") == 0)
    grub_ieee1275_set_flag (GRUB_IEEE1275_FLAG_BROKEN_REPEAT);

  rc = grub_ieee1275_get_property (root, "compatible",
				   tmp,	sizeof (tmp), &actual);
  if (rc >= 0)
    {
      char *ptr;
      for (ptr = tmp; ptr - tmp < actual; ptr += grub_strlen (ptr) + 1)
	{
	  if (grub_memcmp (ptr, "MacRISC", sizeof ("MacRISC") - 1) == 0
	      && (ptr[sizeof ("MacRISC") - 1] == 0
		  || grub_isdigit (ptr[sizeof ("MacRISC") - 1])))
	    {
	      grub_ieee1275_set_flag (GRUB_IEEE1275_FLAG_BROKEN_ADDRESS_CELLS);
	      grub_ieee1275_set_flag (GRUB_IEEE1275_FLAG_NO_OFNET_SUFFIX);
	      grub_ieee1275_set_flag (GRUB_IEEE1275_FLAG_VIRT_TO_REAL_BROKEN);
	      grub_ieee1275_set_flag (GRUB_IEEE1275_FLAG_CURSORONOFF_ANSI_BROKEN);
	      break;
	    }
	}
    }

  if (is_smartfirmware)
    {
      /* Broken in all versions */
      grub_ieee1275_set_flag (GRUB_IEEE1275_FLAG_BROKEN_OUTPUT);

      /* There are two incompatible ways of checking the version number.  Try
         both. */
      rc = grub_ieee1275_get_property (openprom, "SmartFirmware-version",
				       tmp, sizeof (tmp), 0);
      if (rc < 0)
	rc = grub_ieee1275_get_property (openprom, "firmware-version",
					 tmp, sizeof (tmp), 0);
      if (rc >= 0)
	{
	  /* It is tempting to implement a version parser to set the flags for
	     e.g. 1.3 and below.  However, there's a special situation here.
	     3rd party updates which fix the partition bugs are common, and for
	     some reason their fixes aren't being merged into trunk.  So for
	     example we know that 1.2 and 1.3 are broken, but there's 1.2.99
	     and 1.3.99 which are known good (and applying this workaround
	     would cause breakage). */
	  if (!grub_strcmp (tmp, "1.0")
	      || !grub_strcmp (tmp, "1.1")
	      || !grub_strcmp (tmp, "1.2")
	      || !grub_strcmp (tmp, "1.3"))
	    {
	      grub_ieee1275_set_flag (GRUB_IEEE1275_FLAG_NO_PARTITION_0);
	      grub_ieee1275_set_flag (GRUB_IEEE1275_FLAG_0_BASED_PARTITIONS);
	    }
	}
    }

  if (is_olpc)
    {
      /* OLPC / XO laptops have three kinds of storage devices:

	 - NAND flash.  These are accessible via OFW callbacks, but:
	   - Follow strange semantics, imposed by hardware constraints.
	   - Its ABI is undocumented, and not stable.
	   They lack "device_type" property, which conveniently makes GRUB
	   skip them.

	 - USB drives.  Not accessible, because OFW shuts down the controller
	   in order to prevent collisions with applications accessing it
	   directly.  Even worse, attempts to access it will NOT return
	   control to the caller, so we have to avoid probing them.

	 - SD cards.  These work fine.

	 To avoid breakage, we only need to skip USB probing.  However,
	 since detecting SD cards is more reliable, we do that instead.
      */

      grub_ieee1275_set_flag (GRUB_IEEE1275_FLAG_OFDISK_SDCARD_ONLY);
      grub_ieee1275_set_flag (GRUB_IEEE1275_FLAG_HAS_CURSORONOFF);
    }

  if (is_qemu)
    {
      /* OpenFirmware hangs on qemu if one requests any memory below 1.5 MiB.  */
      grub_ieee1275_set_flag (GRUB_IEEE1275_FLAG_NO_PRE1_5M_CLAIM);

      grub_ieee1275_set_flag (GRUB_IEEE1275_FLAG_HAS_CURSORONOFF);
    }

  if (! grub_ieee1275_finddevice ("/rom/boot-rom", &bootrom)
      || ! grub_ieee1275_finddevice ("/boot-rom", &bootrom))
    {
      rc = grub_ieee1275_get_property (bootrom, "model", tmp, sizeof (tmp), 0);
      if (rc >= 0 && !grub_strncmp (tmp, "PPC Open Hack'Ware",
				    sizeof ("PPC Open Hack'Ware") - 1))
	{
	  grub_ieee1275_set_flag (GRUB_IEEE1275_FLAG_BROKEN_OUTPUT);
	  grub_ieee1275_set_flag (GRUB_IEEE1275_FLAG_CANNOT_SET_COLORS);
	  grub_ieee1275_set_flag (GRUB_IEEE1275_FLAG_CANNOT_INTERPRET);
	  grub_ieee1275_set_flag (GRUB_IEEE1275_FLAG_FORCE_CLAIM);
	  grub_ieee1275_set_flag (GRUB_IEEE1275_FLAG_NO_ANSI);
	}
    }
}

void
grub_ieee1275_init (void)
{
  grub_ieee1275_finddevice ("/chosen", &grub_ieee1275_chosen);

  if (grub_ieee1275_get_integer_property (grub_ieee1275_chosen, "mmu", &grub_ieee1275_mmu,
					  sizeof grub_ieee1275_mmu, 0) < 0)
    grub_ieee1275_mmu = 0;

  grub_ieee1275_find_options ();
}
