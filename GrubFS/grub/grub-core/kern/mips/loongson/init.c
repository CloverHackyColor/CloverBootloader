/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2009,2010  Free Software Foundation, Inc.
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
#include <grub/env.h>
#include <grub/time.h>
#include <grub/types.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/machine/time.h>
#include <grub/machine/kernel.h>
#include <grub/machine/memory.h>
#include <grub/memory.h>
#include <grub/mips/loongson.h>
#include <grub/cs5536.h>
#include <grub/term.h>
#include <grub/cpu/memory.h>
#include <grub/i18n.h>
#include <grub/video.h>
#include <grub/terminfo.h>
#include <grub/keyboard_layouts.h>
#include <grub/serial.h>
#include <grub/loader.h>
#include <grub/at_keyboard.h>

grub_err_t
grub_machine_mmap_iterate (grub_memory_hook_t hook, void *hook_data)
{
  hook (GRUB_ARCH_LOWMEMPSTART, grub_arch_memsize << 20,
	GRUB_MEMORY_AVAILABLE, hook_data);
  hook (GRUB_ARCH_HIGHMEMPSTART, grub_arch_highmemsize << 20,
	GRUB_MEMORY_AVAILABLE, hook_data);
  return GRUB_ERR_NONE;
}

/* Helper for init_pci.  */
static int
set_card (grub_pci_device_t dev, grub_pci_id_t pciid,
	  void *data __attribute__ ((unused)))
{
  grub_pci_address_t addr;
  /* We could use grub_pci_assign_addresses for this but we prefer to
     have exactly same memory map as on pmon.  */
  switch (pciid)
    {
    case GRUB_LOONGSON_OHCI_PCIID:
      addr = grub_pci_make_address (dev, GRUB_PCI_REG_ADDRESS_REG0);
      grub_pci_write (addr, 0x5025000);
      addr = grub_pci_make_address (dev, GRUB_PCI_REG_COMMAND);
      grub_pci_write_word (addr, GRUB_PCI_COMMAND_SERR_ENABLE
			   | GRUB_PCI_COMMAND_PARITY_ERROR
			   | GRUB_PCI_COMMAND_BUS_MASTER
			   | GRUB_PCI_COMMAND_MEM_ENABLED);

      addr = grub_pci_make_address (dev, GRUB_PCI_REG_STATUS);
      grub_pci_write_word (addr, 0x0200 | GRUB_PCI_STATUS_CAPABILITIES);
      break;
    case GRUB_LOONGSON_EHCI_PCIID:
      addr = grub_pci_make_address (dev, GRUB_PCI_REG_ADDRESS_REG0);
      grub_pci_write (addr, 0x5026000);
      addr = grub_pci_make_address (dev, GRUB_PCI_REG_COMMAND);
      grub_pci_write_word (addr, GRUB_PCI_COMMAND_SERR_ENABLE
			   | GRUB_PCI_COMMAND_PARITY_ERROR
			   | GRUB_PCI_COMMAND_BUS_MASTER
			   | GRUB_PCI_COMMAND_MEM_ENABLED);

      addr = grub_pci_make_address (dev, GRUB_PCI_REG_STATUS);
      grub_pci_write_word (addr, (1 << GRUB_PCI_STATUS_DEVSEL_TIMING_SHIFT)
			   | GRUB_PCI_STATUS_CAPABILITIES);
      break;
    }
  return 0;
}

static void
init_pci (void)
{
  *((volatile grub_uint32_t *) GRUB_CPU_LOONGSON_PCI_HIT1_SEL_LO) = 0x8000000c;
  *((volatile grub_uint32_t *) GRUB_CPU_LOONGSON_PCI_HIT1_SEL_HI) = 0xffffffff;

  /* Setup PCI controller.  */
  *((volatile grub_uint16_t *) (GRUB_MACHINE_PCI_CONTROLLER_HEADER
				+ GRUB_PCI_REG_COMMAND))
    = GRUB_PCI_COMMAND_PARITY_ERROR | GRUB_PCI_COMMAND_BUS_MASTER
    | GRUB_PCI_COMMAND_MEM_ENABLED;
  *((volatile grub_uint16_t *) (GRUB_MACHINE_PCI_CONTROLLER_HEADER
				+ GRUB_PCI_REG_STATUS))
    = (1 << GRUB_PCI_STATUS_DEVSEL_TIMING_SHIFT)
    | GRUB_PCI_STATUS_FAST_B2B_CAPABLE | GRUB_PCI_STATUS_66MHZ_CAPABLE
    | GRUB_PCI_STATUS_CAPABILITIES;

  *((volatile grub_uint32_t *) (GRUB_MACHINE_PCI_CONTROLLER_HEADER
				+ GRUB_PCI_REG_CACHELINE)) = 0xff;
  *((volatile grub_uint32_t *) (GRUB_MACHINE_PCI_CONTROLLER_HEADER 
				+ GRUB_PCI_REG_ADDRESS_REG0))
    = 0x80000000 | GRUB_PCI_ADDR_MEM_TYPE_64 | GRUB_PCI_ADDR_MEM_PREFETCH;
  *((volatile grub_uint32_t *) (GRUB_MACHINE_PCI_CONTROLLER_HEADER 
				+ GRUB_PCI_REG_ADDRESS_REG1)) = 0;

  grub_pci_iterate (set_card, NULL);
}

void
grub_machine_init (void)
{
  grub_addr_t modend;
  grub_uint32_t prid;

  asm volatile ("mfc0 %0, " GRUB_CPU_LOONGSON_COP0_PRID : "=r" (prid));

  switch (prid)
    {
      /* Loongson 2E.  */
    case 0x6302:
      grub_arch_machine = GRUB_ARCH_MACHINE_FULOONG2E;
      grub_bonito_type = GRUB_BONITO_2F;
      break;
      /* Loongson 2F.  */
    case 0x6303:
      if (grub_arch_machine != GRUB_ARCH_MACHINE_FULOONG2F
	  && grub_arch_machine != GRUB_ARCH_MACHINE_YEELOONG)
	grub_arch_machine = GRUB_ARCH_MACHINE_YEELOONG;
      grub_bonito_type = GRUB_BONITO_2F;
      break;
      /* Loongson 3A. */
    case 0x6305:
      grub_arch_machine = GRUB_ARCH_MACHINE_YEELOONG_3A;
      grub_bonito_type = GRUB_BONITO_3A;
      break;
    }

  /* FIXME: measure this.  */
  if (grub_arch_busclock == 0)
    {
      grub_arch_busclock = 66000000;
      grub_arch_cpuclock = 797000000;
    }

  grub_install_get_time_ms (grub_rtc_get_time_ms);

  if (grub_arch_memsize == 0)
    {
      grub_port_t smbbase;
      grub_err_t err;
      grub_pci_device_t dev;
      struct grub_smbus_spd spd;
      unsigned totalmem;
      int i;

      if (!grub_cs5536_find (&dev))
	grub_fatal ("No CS5536 found\n");

      err = grub_cs5536_init_smbus (dev, 0x7ff, &smbbase);
      if (err)
	grub_fatal ("Couldn't init SMBus: %s\n", grub_errmsg);

      /* Yeeloong and Fuloong have only one memory slot.  */
      err = grub_cs5536_read_spd (smbbase, GRUB_SMB_RAM_START_ADDR, &spd);
      if (err)
	grub_fatal ("Couldn't read SPD: %s\n", grub_errmsg);
      for (i = 5; i < 13; i++)
	if (spd.ddr2.rank_capacity & (1 << (i & 7)))
	  break;
      /* Something is wrong.  */
      if (i == 13)
	totalmem = 256;
      else
	totalmem = ((spd.ddr2.num_of_ranks
		     & GRUB_SMBUS_SPD_MEMORY_NUM_OF_RANKS_MASK) + 1) << (i + 2);
      
      if (totalmem >= 256)
	{
	  grub_arch_memsize = 256;
	  grub_arch_highmemsize = totalmem - 256;
	}
      else
	{
	  grub_arch_memsize = totalmem;
	  grub_arch_highmemsize = 0;
	}

      grub_cs5536_init_geode (dev);

      init_pci ();
    }

  modend = grub_modules_get_end ();
  grub_mm_init_region ((void *) modend, (grub_arch_memsize << 20)
		       - (modend - GRUB_ARCH_LOWMEMVSTART));
  /* FIXME: use upper memory as well.  */

  /* Initialize output terminal (can't be done earlier, as gfxterm
     relies on a working heap.  */
  grub_video_sm712_init ();
  grub_video_sis315pro_init ();
  grub_video_radeon_fuloong2e_init ();
  grub_video_radeon_yeeloong3a_init ();
  grub_font_init ();
  grub_gfxterm_init ();

  grub_keylayouts_init ();
  if (grub_arch_machine == GRUB_ARCH_MACHINE_YEELOONG
      || grub_arch_machine == GRUB_ARCH_MACHINE_YEELOONG_3A)
    grub_at_keyboard_init ();

  grub_terminfo_init ();
  grub_serial_init ();

  grub_boot_init ();
}

void
grub_machine_fini (int flags __attribute__ ((unused)))
{
}

static int
halt_via (grub_pci_device_t dev, grub_pci_id_t pciid,
	  void *data __attribute__ ((unused)))
{
  grub_uint16_t pm;
  grub_pci_address_t addr;

  if (pciid != 0x30571106)
    return 0;

  addr = grub_pci_make_address (dev, 0x40);
  pm = grub_pci_read (addr) & ~1;

  if (pm == 0)
    {
      grub_pci_write (addr, 0x1801);
      pm = 0x1800;
    }

  addr = grub_pci_make_address (dev, 0x80);
  grub_pci_write_byte (addr, 0xff);

  addr = grub_pci_make_address (dev, GRUB_PCI_REG_COMMAND);
  grub_pci_write_word (addr, grub_pci_read_word (addr) | GRUB_PCI_COMMAND_IO_ENABLED);

  /* FIXME: This one is derived from qemu. Check on real hardware.  */
  grub_outw (0x2000, pm + 4 + GRUB_MACHINE_PCI_IO_BASE);
  grub_millisleep (5000);

  return 0;
}

void
grub_halt (void)
{
  switch (grub_arch_machine)
    {
    case GRUB_ARCH_MACHINE_FULOONG2E:
      grub_pci_iterate (halt_via, NULL);
      break;
    case GRUB_ARCH_MACHINE_FULOONG2F:
      {
	grub_pci_device_t dev;
	grub_port_t p;
	if (grub_cs5536_find (&dev))
	  {
	    p = (grub_cs5536_read_msr (dev, GRUB_CS5536_MSR_GPIO_BAR)
		 & GRUB_CS5536_LBAR_ADDR_MASK) + GRUB_MACHINE_PCI_IO_BASE;
	    grub_outl ((1 << 13), p + 4);
	    grub_outl ((1 << 29), p);
	    grub_millisleep (5000);
	  }
      }
      break;
    case GRUB_ARCH_MACHINE_YEELOONG:
      grub_outb (grub_inb (GRUB_CPU_LOONGSON_GPIOCFG)
		 & ~GRUB_CPU_YEELOONG_SHUTDOWN_GPIO, GRUB_CPU_LOONGSON_GPIOCFG);
      grub_millisleep (1500);
      break;
    case GRUB_ARCH_MACHINE_YEELOONG_3A:
      grub_millisleep (1);
      grub_outb (0x4e, GRUB_MACHINE_PCI_IO_BASE_3A | 0x66);
      grub_millisleep (1);
      grub_outb (2, GRUB_MACHINE_PCI_IO_BASE_3A | 0x62);
      grub_millisleep (5000);
      break;
    }

  grub_puts_ (N_("Shutdown failed"));
  grub_refresh ();
  while (1);
}

void
grub_exit (void)
{
  grub_halt ();
}

void
grub_machine_get_bootlocation (char **device __attribute__ ((unused)),
			       char **path __attribute__ ((unused)))
{
}

extern char _end[];
grub_addr_t grub_modbase = (grub_addr_t) _end;

