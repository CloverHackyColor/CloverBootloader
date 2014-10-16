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

#include <grub/cpu/io.h>
#include <grub/misc.h>
#include <grub/acpi.h>
#include <grub/i18n.h>
#include <grub/pci.h>
#include <grub/mm.h>

const char bochs_shutdown[] = "Shutdown";

/*
 *  This call is special...  it never returns...  in fact it should simply
 *  hang at this point!
 */
static inline void  __attribute__ ((noreturn))
stop (void)
{
  asm volatile ("cli");
  while (1)
    {
      asm volatile ("hlt");
    }
}

static int
grub_shutdown_pci_iter (grub_pci_device_t dev, grub_pci_id_t pciid,
			void *data __attribute__ ((unused)))
{
  /* QEMU.  */
  if (pciid == 0x71138086)
    {
      grub_pci_address_t addr;
      addr = grub_pci_make_address (dev, 0x40);
      grub_pci_write (addr, 0x7001);
      addr = grub_pci_make_address (dev, 0x80);
      grub_pci_write (addr, grub_pci_read (addr) | 1);
      grub_outw (0x2000, 0x7004);
    }
  return 0;
}

void
grub_halt (void)
{
  unsigned int i;

#if defined (GRUB_MACHINE_COREBOOT) || defined (GRUB_MACHINE_MULTIBOOT)
  grub_acpi_halt ();
#endif

  /* Disable interrupts.  */
  __asm__ __volatile__ ("cli");

  /* Bochs, QEMU, etc. Removed in newer QEMU releases.  */
  for (i = 0; i < sizeof (bochs_shutdown) - 1; i++)
    grub_outb (bochs_shutdown[i], 0x8900);

  grub_pci_iterate (grub_shutdown_pci_iter, NULL);

  grub_puts_ (N_("GRUB doesn't know how to halt this machine yet!"));

  /* In order to return we'd have to check what the previous status of IF
     flag was.  But user most likely doesn't want to return anyway ...  */
  stop ();
}
