/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2011  Free Software Foundation, Inc.
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

#include <grub/machine/ec.h>
#include <grub/machine/kernel.h>
#include <grub/machine/memory.h>
#include <grub/misc.h>
#include <grub/pci.h>
#include <grub/cs5536.h>
#include <grub/time.h>
#include <grub/term.h>
#include <grub/i18n.h>

void
grub_reboot (void)
{
  switch (grub_arch_machine)
    {
    case GRUB_ARCH_MACHINE_FULOONG2E:
      grub_outl (grub_inl (0xbfe00104) & ~4, 0xbfe00104);
      grub_outl (grub_inl (0xbfe00104) | 4, 0xbfe00104);
      break;
    case GRUB_ARCH_MACHINE_FULOONG2F:
      {
	grub_pci_device_t dev;
	if (!grub_cs5536_find (&dev))
	  break;
	grub_cs5536_write_msr (dev, GRUB_CS5536_MSR_DIVIL_RESET,
			       grub_cs5536_read_msr (dev,
						     GRUB_CS5536_MSR_DIVIL_RESET) 
			       | 1);
	break;
      }
    case GRUB_ARCH_MACHINE_YEELOONG:
      grub_write_ec (GRUB_MACHINE_EC_COMMAND_REBOOT);
      break;
    case GRUB_ARCH_MACHINE_YEELOONG_3A:
      grub_millisleep (1);
      grub_outb (0x4e, GRUB_MACHINE_PCI_IO_BASE_3A | 0x66);
      grub_millisleep (1);
      grub_outb (1, GRUB_MACHINE_PCI_IO_BASE_3A | 0x62);
      grub_millisleep (5000);
    }
  grub_millisleep (1500);

  grub_puts_ (N_("Reboot failed"));
  grub_refresh ();
  while (1);
}
