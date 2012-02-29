/*
 * ATI Graphics Card Enabler, part of the Chameleon Boot Loader Project
 *
 * Copyright 2010 by Islam M. Ahmed Zaid. All rights reserved.
 *
 */

#include "device_inject.h"

#include "ati.h"

#define DEBUG_ATI 1

#if DEBUG_ATI == 2
#define DBG(x...) AsciiPrint(x)
#elif DEBUG_ATI == 1
#define DBG(x...) MsgLog(x)
#else
#define DBG(x...)
#endif


//#define Reg32(reg)				(*(volatile UINT32 *)(card->mmio + reg))
//#define RegRead32(reg)			(Reg32(reg))
//#define RegWrite32(reg, value)	(Reg32(reg) = value)



static value_t aty_name;
static value_t aty_nameparent;
card_t *card;
//static value_t aty_model;


BOOLEAN get_bootdisplay_val(value_t *val)
{
	static UINT32 v = 0;
	
	if (v)
		return FALSE;
	
	if (!card->posted)
		return FALSE;
	
	v = 1;
	val->type = kCst;
	val->size = 4;
	val->data = (UINT8 *)&v;
	
	return TRUE;
}

BOOLEAN get_vrammemory_val(value_t *val)
{
	return FALSE;
}

BOOLEAN get_name_val(value_t *val)
{
	val->type = aty_name.type;
	val->size = aty_name.size;
	val->data = aty_name.data;
	
	return TRUE;
}

BOOLEAN get_nameparent_val(value_t *val)
{
	val->type = aty_nameparent.type;
	val->size = aty_nameparent.size;
	val->data = aty_nameparent.data;
	
	return TRUE;
}

BOOLEAN get_model_val(value_t *val)
{
	if (!card->info->model_name)
		return FALSE;
	
	val->type = kStr;
	val->size = AsciiStrLen(card->info->model_name);
	val->data = (UINT8 *)card->info->model_name;
	
	return TRUE;
}

BOOLEAN get_conntype_val(value_t *val)
{
//Connector types:
//0x4 : DisplayPort
//0x400: DL DVI-I
//0x800: HDMI

	return FALSE;
}

BOOLEAN get_vrammemsize_val(value_t *val)
{
	static INTN idx = -1;
	static UINT64 memsize;
	
	idx++;
	memsize = ((UINT64)card->vram_size << 32);
	if (idx == 0)
		memsize = memsize | (UINT64)card->vram_size;
	
	val->type = kCst;
	val->size = 8;
	val->data = (UINT8 *)&memsize;
	
	return TRUE;
}

BOOLEAN get_binimage_val(value_t *val)
{
	if (!card->rom)
		return FALSE;
	
	val->type = kPtr;
	val->size = card->rom_size;
	val->data = card->rom;
	
	return TRUE;
}

BOOLEAN get_romrevision_val(value_t *val)
{
	UINT8 *rev;
	if (!card->rom)
		return FALSE;
	
	rev = card->rom + *(UINT8 *)(card->rom + OFFSET_TO_GET_ATOMBIOS_STRINGS_START);

	val->type = kPtr;
	val->size = AsciiStrLen((CHAR8 *)rev);
	val->data = AllocateZeroPool(val->size);
	
	if (!val->data)
		return FALSE;
	
	CopyMem(val->data, rev, val->size);
	
	return TRUE;
}

BOOLEAN get_deviceid_val(value_t *val)
{
	val->type = kCst;
	val->size = 2;
	val->data = (UINT8 *)&card->pci_dev->device_id;
	
	return TRUE;
}

BOOLEAN get_mclk_val(value_t *val)
{
	return FALSE;
}

BOOLEAN get_sclk_val(value_t *val)
{
	return FALSE;
}

BOOLEAN get_refclk_val(value_t *val)
{
	return FALSE;
}

BOOLEAN get_platforminfo_val(value_t *val)
{
	val->data = AllocateZeroPool(0x80);
	if (!val->data)
		return FALSE;
	
//	bzero(val->data, 0x80);
	
	val->type		= kPtr;
	val->size		= 0x80;
	val->data[0]	= 1;
	
	return TRUE;
}

BOOLEAN get_vramtotalsize_val(value_t *val)
{
  
	val->type = kCst;
	val->size = 4;
	val->data = (UINT8 *)&card->vram_size;
	
	return TRUE;
}

VOID free_val(value_t *val)
{
	if (val->type == kPtr)
		FreePool(val->data);
	
	ZeroMem(val, sizeof(value_t));
}

VOID devprop_add_list(AtiDevProp devprop_list[])
{
	value_t *val = AllocateZeroPool(sizeof(value_t));
	int i, pnum;
	
	for (i = 0; devprop_list[i].name != NULL; i++)
	{
		if ((devprop_list[i].flags == FLAGTRUE) || (devprop_list[i].flags & card->flags))
		{
			if (devprop_list[i].get_value && devprop_list[i].get_value(val))
			{
				devprop_add_value(card->device, devprop_list[i].name, val->data, val->size);
				free_val(val);
				
				if (devprop_list[i].all_ports)
				{
					for (pnum = 1; pnum < card->ports; pnum++)
					{
						if (devprop_list[i].get_value(val))
						{
							devprop_list[i].name[1] = 0x30 + pnum; // convert to ascii
							devprop_add_value(card->device, devprop_list[i].name, val->data, val->size);
							free_val(val);
						}
					}
					devprop_list[i].name[1] = 0x30; // write back our "@0," for a next possible card
				}
			}
			else
			{
				if (devprop_list[i].default_val.type != kNul)
				{
					devprop_add_value(card->device, devprop_list[i].name,
						devprop_list[i].default_val.type == kCst ?
						(UINT8 *)&(devprop_list[i].default_val.data) : devprop_list[i].default_val.data,
						devprop_list[i].default_val.size);
				}
				
				if (devprop_list[i].all_ports)
				{
					for (pnum = 1; pnum < card->ports; pnum++)
					{
						if (devprop_list[i].default_val.type != kNul)
						{
							devprop_list[i].name[1] = 0x30 + pnum; // convert to ascii
							devprop_add_value(card->device, devprop_list[i].name,
								devprop_list[i].default_val.type == kCst ?
								(UINT8 *)&(devprop_list[i].default_val.data) : devprop_list[i].default_val.data,
								devprop_list[i].default_val.size);
						}
					}
					devprop_list[i].name[1] = 0x30; // write back our "@0," for a next possible card
				}
			}
		}
	}

	FreePool(val);
}

BOOLEAN validate_rom(option_rom_header_t *rom_header, pci_dt_t *pci_dev)
{
	option_rom_pci_header_t *rom_pci_header;
	
	if (rom_header->signature != 0xaa55)
		return FALSE;
	
	rom_pci_header = (option_rom_pci_header_t *)((UINT8 *)rom_header + rom_header->pci_header_offset);
	
	if (rom_pci_header->signature != 0x52494350)
		return FALSE;
	
	if (rom_pci_header->vendor_id != pci_dev->vendor_id || rom_pci_header->device_id != pci_dev->device_id)
		return FALSE;
	
	return TRUE;
}

BOOLEAN load_vbios_file(UINT16 vendor_id, UINT16 device_id, UINT32 subsys_id)
{
  	EFI_STATUS            Status;
	UINTN bufferLen;
	CHAR16 FileName[24];
//	BOOLEAN do_load = FALSE;
  UINT8*  buffer;
	
//	getBoolForKey(key, &do_load, &bootInfo->chameleonConfig);
	if (!gSettings.LoadVBios)
		return FALSE;
	
	UnicodeSPrint(FileName, 24, L"\\EFI\\device\\%04x_%04x.rom", vendor_id, device_id, subsys_id);
	if (!FileExists(SelfRootDir, FileName)){
    DBG("ATI ROM not found \n");
		return FALSE;
  }
	Status = egLoadFile(SelfRootDir, FileName, &buffer, &bufferLen);
  if (EFI_ERROR(Status)) {
    return FALSE;
  }
  
	card->rom_size = bufferLen;
	card->rom = AllocateZeroPool(bufferLen);
	if (!card->rom)
		return FALSE;
	CopyMem(card->rom, buffer, bufferLen);
//	read(fd, (CHAR8 *)card->rom, card->rom_size);
	
	if (!validate_rom((option_rom_header_t *)card->rom, card->pci_dev))
	{
    DBG("validate_rom fails\n");
		card->rom_size = 0;
		card->rom = 0;
		return FALSE;
	}
	
	card->rom_size = ((option_rom_header_t *)card->rom)->rom_size * 512;
	
//	close(fd);
  FreePool(buffer);
	
	return TRUE;
}

void get_vram_size(void)
{
	chip_family_t chip_family = card->info->chip_family;
	
	card->vram_size = 128 << 20; //default 128Mb, this is minimum for OS
  if (gSettings.VRAM != 0) {
    card->vram_size = gSettings.VRAM;
  } else {
    if (chip_family >= CHIP_FAMILY_CEDAR) {
      // size in MB on evergreen
      // XXX watch for overflow!!!
      card->vram_size = REG32(card->mmio, R600_CONFIG_MEMSIZE) << 20;
    } else if (chip_family >= CHIP_FAMILY_R600) {
			card->vram_size = REG32(card->mmio, R600_CONFIG_MEMSIZE);
    } else {
      card->vram_size = REG32(card->mmio, RADEON_CONFIG_MEMSIZE);
      if (card->vram_size == 0) {
        card->vram_size = REG32(card->mmio, RADEON_CONFIG_APER_SIZE);
        //Slice - previously I successfully made Radeon9000 working
        //by writing this register
        WRITEREG32(card->mmio, RADEON_CONFIG_MEMSIZE, 0x30000);
      }
    }
  }	
}

BOOLEAN read_vbios(BOOLEAN from_pci)
{
	option_rom_header_t *rom_addr;
	
	if (from_pci)
	{
		rom_addr = (option_rom_header_t *)(UINTN)(pci_config_read32(card->pci_dev, PCI_EXPANSION_ROM_BASE) & ~0x7ff);
		DBG(" @0x%x", rom_addr);
	}
	else
		rom_addr = (option_rom_header_t *)0xc0000;
	
	if (!validate_rom(rom_addr, card->pci_dev)){
    DBG("There is no ROM @C0000\n");
 //   gBS->Stall(3000000);
		return FALSE;
  }
	
	card->rom_size = rom_addr->rom_size * 512;
	if (!card->rom_size)
		return FALSE;
	
	card->rom = AllocateZeroPool(card->rom_size);
	if (!card->rom)
		return FALSE;
	
	CopyMem(card->rom, (void *)rom_addr, card->rom_size);
	
	return TRUE;
}

BOOLEAN read_disabled_vbios(VOID)
{
	BOOLEAN ret = FALSE;
	chip_family_t chip_family = card->info->chip_family;
	
	if (chip_family >= CHIP_FAMILY_RV770)
	{
		UINT32 viph_control		= REG32(card->mmio, RADEON_VIPH_CONTROL);
		UINT32 bus_cntl			= REG32(card->mmio, RADEON_BUS_CNTL);
		UINT32 d1vga_control		= REG32(card->mmio, AVIVO_D1VGA_CONTROL);
		UINT32 d2vga_control		= REG32(card->mmio, AVIVO_D2VGA_CONTROL);
		UINT32 vga_render_control = REG32(card->mmio, AVIVO_VGA_RENDER_CONTROL);
		UINT32 rom_cntl			= REG32(card->mmio, R600_ROM_CNTL);
		UINT32 cg_spll_func_cntl	= 0;
		UINT32 cg_spll_status;
		
		// disable VIP
		WRITEREG32(card->mmio, RADEON_VIPH_CONTROL, (viph_control & ~RADEON_VIPH_EN));
		
		// enable the rom
		WRITEREG32(card->mmio, RADEON_BUS_CNTL, (bus_cntl & ~RADEON_BUS_BIOS_DIS_ROM));
		
		// Disable VGA mode
		WRITEREG32(card->mmio, AVIVO_D1VGA_CONTROL, (d1vga_control & ~(AVIVO_DVGA_CONTROL_MODE_ENABLE | AVIVO_DVGA_CONTROL_TIMING_SELECT)));
		WRITEREG32(card->mmio, AVIVO_D2VGA_CONTROL, (d2vga_control & ~(AVIVO_DVGA_CONTROL_MODE_ENABLE | AVIVO_DVGA_CONTROL_TIMING_SELECT)));
		WRITEREG32(card->mmio, AVIVO_VGA_RENDER_CONTROL, (vga_render_control & ~AVIVO_VGA_VSTATUS_CNTL_MASK));
		
		if (chip_family == CHIP_FAMILY_RV730)
		{
			cg_spll_func_cntl = REG32(card->mmio, R600_CG_SPLL_FUNC_CNTL);
			
			// enable bypass mode
			WRITEREG32(card->mmio, R600_CG_SPLL_FUNC_CNTL, (cg_spll_func_cntl | R600_SPLL_BYPASS_EN));
			
			// wait for SPLL_CHG_STATUS to change to 1
			cg_spll_status = 0;
			while (!(cg_spll_status & R600_SPLL_CHG_STATUS))
				cg_spll_status = REG32(card->mmio, R600_CG_SPLL_STATUS);
			
			WRITEREG32(card->mmio, R600_ROM_CNTL, (rom_cntl & ~R600_SCK_OVERWRITE));
		}
		else
			WRITEREG32(card->mmio, R600_ROM_CNTL, (rom_cntl | R600_SCK_OVERWRITE));
		
		ret = read_vbios(TRUE);
		
		// restore regs
		if (chip_family == CHIP_FAMILY_RV730)
		{
			WRITEREG32(card->mmio, R600_CG_SPLL_FUNC_CNTL, cg_spll_func_cntl);
			
			// wait for SPLL_CHG_STATUS to change to 1
			cg_spll_status = 0;
			while (!(cg_spll_status & R600_SPLL_CHG_STATUS))
			cg_spll_status = REG32(card->mmio, R600_CG_SPLL_STATUS);
		}
		WRITEREG32(card->mmio, RADEON_VIPH_CONTROL, viph_control);
		WRITEREG32(card->mmio, RADEON_BUS_CNTL, bus_cntl);
		WRITEREG32(card->mmio, AVIVO_D1VGA_CONTROL, d1vga_control);
		WRITEREG32(card->mmio, AVIVO_D2VGA_CONTROL, d2vga_control);
		WRITEREG32(card->mmio, AVIVO_VGA_RENDER_CONTROL, vga_render_control);
		WRITEREG32(card->mmio, R600_ROM_CNTL, rom_cntl);
	}
	else
		if (chip_family >= CHIP_FAMILY_R600)
		{
			UINT32 viph_control				= REG32(card->mmio, RADEON_VIPH_CONTROL);
			UINT32 bus_cntl					= REG32(card->mmio, RADEON_BUS_CNTL);
			UINT32 d1vga_control				= REG32(card->mmio, AVIVO_D1VGA_CONTROL);
			UINT32 d2vga_control				= REG32(card->mmio, AVIVO_D2VGA_CONTROL);
			UINT32 vga_render_control			= REG32(card->mmio, AVIVO_VGA_RENDER_CONTROL);
			UINT32 rom_cntl					= REG32(card->mmio, R600_ROM_CNTL);
			UINT32 general_pwrmgt				= REG32(card->mmio, R600_GENERAL_PWRMGT);
			UINT32 low_vid_lower_gpio_cntl	= REG32(card->mmio, R600_LOW_VID_LOWER_GPIO_CNTL);
			UINT32 medium_vid_lower_gpio_cntl = REG32(card->mmio, R600_MEDIUM_VID_LOWER_GPIO_CNTL);
			UINT32 high_vid_lower_gpio_cntl	= REG32(card->mmio, R600_HIGH_VID_LOWER_GPIO_CNTL);
			UINT32 ctxsw_vid_lower_gpio_cntl	= REG32(card->mmio, R600_CTXSW_VID_LOWER_GPIO_CNTL);
			UINT32 lower_gpio_enable			= REG32(card->mmio, R600_LOWER_GPIO_ENABLE);
			
			// disable VIP
			WRITEREG32(card->mmio, RADEON_VIPH_CONTROL, (viph_control & ~RADEON_VIPH_EN));
			
			// enable the rom
			WRITEREG32(card->mmio, RADEON_BUS_CNTL, (bus_cntl & ~RADEON_BUS_BIOS_DIS_ROM));
			
			// Disable VGA mode
			WRITEREG32(card->mmio, AVIVO_D1VGA_CONTROL, (d1vga_control & ~(AVIVO_DVGA_CONTROL_MODE_ENABLE | AVIVO_DVGA_CONTROL_TIMING_SELECT)));
			WRITEREG32(card->mmio, AVIVO_D2VGA_CONTROL, (d2vga_control & ~(AVIVO_DVGA_CONTROL_MODE_ENABLE | AVIVO_DVGA_CONTROL_TIMING_SELECT)));
			WRITEREG32(card->mmio, AVIVO_VGA_RENDER_CONTROL, (vga_render_control & ~AVIVO_VGA_VSTATUS_CNTL_MASK));
			WRITEREG32(card->mmio, R600_ROM_CNTL, ((rom_cntl & ~R600_SCK_PRESCALE_CRYSTAL_CLK_MASK) | (1 << R600_SCK_PRESCALE_CRYSTAL_CLK_SHIFT) | R600_SCK_OVERWRITE));
			WRITEREG32(card->mmio, R600_GENERAL_PWRMGT, (general_pwrmgt & ~R600_OPEN_DRAIN_PADS));
			WRITEREG32(card->mmio, R600_LOW_VID_LOWER_GPIO_CNTL, (low_vid_lower_gpio_cntl & ~0x400));
			WRITEREG32(card->mmio, R600_MEDIUM_VID_LOWER_GPIO_CNTL, (medium_vid_lower_gpio_cntl & ~0x400));
			WRITEREG32(card->mmio, R600_HIGH_VID_LOWER_GPIO_CNTL, (high_vid_lower_gpio_cntl & ~0x400));
			WRITEREG32(card->mmio, R600_CTXSW_VID_LOWER_GPIO_CNTL, (ctxsw_vid_lower_gpio_cntl & ~0x400));
			WRITEREG32(card->mmio, R600_LOWER_GPIO_ENABLE, (lower_gpio_enable | 0x400));
			
			ret = read_vbios(TRUE);
			
			// restore regs
			WRITEREG32(card->mmio, RADEON_VIPH_CONTROL, viph_control);
			WRITEREG32(card->mmio, RADEON_BUS_CNTL, bus_cntl);
			WRITEREG32(card->mmio, AVIVO_D1VGA_CONTROL, d1vga_control);
			WRITEREG32(card->mmio, AVIVO_D2VGA_CONTROL, d2vga_control);
			WRITEREG32(card->mmio, AVIVO_VGA_RENDER_CONTROL, vga_render_control);
			WRITEREG32(card->mmio, R600_ROM_CNTL, rom_cntl);
			WRITEREG32(card->mmio, R600_GENERAL_PWRMGT, general_pwrmgt);
			WRITEREG32(card->mmio, R600_LOW_VID_LOWER_GPIO_CNTL, low_vid_lower_gpio_cntl);
			WRITEREG32(card->mmio, R600_MEDIUM_VID_LOWER_GPIO_CNTL, medium_vid_lower_gpio_cntl);
			WRITEREG32(card->mmio, R600_HIGH_VID_LOWER_GPIO_CNTL, high_vid_lower_gpio_cntl);
			WRITEREG32(card->mmio, R600_CTXSW_VID_LOWER_GPIO_CNTL, ctxsw_vid_lower_gpio_cntl);
			WRITEREG32(card->mmio, R600_LOWER_GPIO_ENABLE, lower_gpio_enable);
		}

	return ret;
}

BOOLEAN radeon_card_posted(VOID)
{
	UINT32 reg;
	
	// first check CRTCs
	reg = REG32(card->mmio, RADEON_CRTC_GEN_CNTL) | REG32(card->mmio, RADEON_CRTC2_GEN_CNTL);
	if (reg & RADEON_CRTC_EN)
		return TRUE;
	
	// then check MEM_SIZE, in case something turned the crtcs off
	reg = REG32(card->mmio, R600_CONFIG_MEMSIZE);
	if (reg)
		return TRUE;
	
	return FALSE;
}

#if 0
BOOLEAN devprop_add_pci_config_space(VOID)
{
	int offset;
	
	UINT8 *config_space = AllocateZeroPool(0x100);
	if (!config_space)
		return FALSE;
	
	for (offset = 0; offset < 0x100; offset += 4)
		config_space[offset / 4] = pci_config_read32(card->pci_dev, offset);
	
	devprop_add_value(card->device, "ATY,PCIConfigSpace", config_space, 0x100);
	FreePool(config_space);
	
	return TRUE;
}
#endif

static BOOLEAN init_card(pci_dt_t *pci_dev)
{
//	BOOLEAN	add_vbios = TRUE;
	CHAR8	name[24];
	CHAR8	name_parent[24];
	int		i;
//	int		n_ports = 0;
	
	card = AllocateZeroPool(sizeof(card_t));
	if (!card)
		return FALSE;
//	bzero(card, sizeof(card_t));
	
	card->pci_dev = pci_dev;
	
	for (i = 0; radeon_cards[i].device_id ; i++)
	{
		if (radeon_cards[i].device_id == pci_dev->device_id)
		{
			card->info = &radeon_cards[i];
			if ((radeon_cards[i].subsys_id == 0x00000000) ||
				(radeon_cards[i].subsys_id == pci_dev->subsys_id.subsys_id))
				break;
		}
	}
	
	if (!card->info->device_id || !card->info->cfg_name)
	{
		DBG("Unsupported ATI card! Device ID: [%04x:%04x] Subsystem ID: [%08x] \n", 
				pci_dev->vendor_id, pci_dev->device_id, pci_dev->subsys_id);
		return FALSE;
	}
	
	card->fb		= (UINT8 *)(UINTN)(pci_config_read32(pci_dev, PCI_BASE_ADDRESS_0) & ~0x0f);
	card->mmio		= (UINT8 *)(UINTN)(pci_config_read32(pci_dev, PCI_BASE_ADDRESS_2) & ~0x0f);
	card->io		= (UINT8 *)(UINTN)(pci_config_read32(pci_dev, PCI_BASE_ADDRESS_4) & ~0x03);
  pci_dev->regs = card->mmio;
	
	DBG("Framebuffer @0x%08X  MMIO @0x%08X	I/O Port @0x%08X ROM Addr @0x%08X\n",
		card->fb, card->mmio, card->io, pci_config_read32(pci_dev, PCI_EXPANSION_ROM_BASE));
	
	card->posted = radeon_card_posted();
	DBG("ATI card %a, ", card->posted ? "POSTed" : "non-POSTed");
	
	get_vram_size();
	
//	getBoolForKey(kATYbinimage, &add_vbios, &bootInfo->chameleonConfig);

	if (gSettings.LoadVBios)
	{
		if (!load_vbios_file(pci_dev->vendor_id, pci_dev->device_id, pci_dev->subsys_id.subsys_id))
		{
			DBG("reading VBIOS from %a", card->posted ? "legacy space" : "PCI ROM");
			if (card->posted)
				read_vbios(FALSE);
			else
				read_disabled_vbios();
			DBG("\n");
		}
	}

	
//	card->ports = 2; // default - Azi: default is card_configs
	
	if (card->info->chip_family >= CHIP_FAMILY_CEDAR)
	{
		card->flags |= EVERGREEN;
//		card->ports = 3; //Azi: use the AtiPorts key if needed
	}
	if (gSettings.VideoPorts) {
    card->ports = gSettings.VideoPorts;
  }
	
//	atN = 0;
	
	// Check AtiConfig key for a framebuffer name,
  //	card->cfg_name = getStringForKey(kAtiConfig, &bootInfo->chameleonConfig);
  UnicodeStrToAsciiStr((CHAR16*)&gSettings.FBName[0],(CHAR8*)&card->cfg_name[0]);
	// if none,
	if (AsciiStrLen(card->cfg_name) == 0)
	{
		// use cfg_name on radeon_cards, to retrive the default name from card_configs,
		card->cfg_name = card_configs[card->info->cfg_name].name;
		// and leave ports alone!
//		card->ports = card_configs[card->info->cfg_name].ports;
		
		// which means one of the fb's or kNull
		DBG("Framebuffer set to device's default: %a\n", card->cfg_name);
	}
	else
	{
		// else, use the fb name returned by AtiConfig.
		DBG("(AtiConfig) Framebuffer set to: %a\n", card->cfg_name);
	}
	
	// Check AtiPorts key for nr of ports,
  //	card->ports = getIntForKey(kAtiPorts, &n_ports, &bootInfo->chameleonConfig);
  
	// if a value bigger than 0 ?? is found, (do we need >= 0 ?? that's null FB on card_configs)
	if (gSettings.VideoPorts > 0)
	{
		card->ports = gSettings.VideoPorts; // use it.
		DBG("(AtiPorts) Nr of ports set to: %d\n", card->ports);
    }
	else// if (card->cfg_name > 0) // do we want 0 ports if fb is kNull or mistyped ?
	{
		// else, match cfg_name with card_configs list and retrive default nr of ports.
		for (i = 0; i < kCfgEnd; i++)
			if (AsciiStrCmp(card->cfg_name, card_configs[i].name) == 0)
				card->ports = card_configs[i].ports; // default
		
		DBG("Nr of ports set to framebuffer's default: %d\n", card->ports);
	}
//	else
//		card->ports = 2/1 ?; // set a min if 0 ports ?
//		DBG("Nr of ports set to min: %d\n", card->ports);
	
	AsciiSPrint(name, 24, "ATY,%a", card->cfg_name);
	aty_name.type = kStr;
	aty_name.size = AsciiStrLen(name);
	aty_name.data = (UINT8 *)name;
	
	AsciiSPrint(name_parent, 24, "ATY,%aParent", card->cfg_name);
	aty_nameparent.type = kStr;
	aty_nameparent.size = AsciiStrLen(name_parent);
	aty_nameparent.data = (UINT8 *)name_parent;
	
	return TRUE;
}

BOOLEAN setup_ati_devprop(pci_dt_t *ati_dev)
{
	CHAR8 *devicepath;
	
	if (!init_card(ati_dev))
		return FALSE;
	
	// -------------------------------------------------
	// Find a better way to do this (in device_inject.c)
	if (!string)
		string = devprop_create_string();
	
	devicepath = get_pci_dev_path(ati_dev);
	card->device = devprop_add_device(string, devicepath);
	if (!card->device)
		return FALSE;
	// -------------------------------------------------
	
#if 0
	UINT64 fb		= (UINT32)card->fb;
	UINT64 mmio	= (UINT32)card->mmio;
	UINT64 io		= (UINT32)card->io;
	devprop_add_value(card->device, "ATY,FrameBufferOffset", &fb, 8);
	devprop_add_value(card->device, "ATY,RegisterSpaceOffset", &mmio, 8);
	devprop_add_value(card->device, "ATY,IOSpaceOffset", &io, 8);
#endif
	
	devprop_add_list(ati_devprop_list);
	
	// -------------------------------------------------
	// Find a better way to do this (in device_inject.c)
	//Azi: XXX tried to fix a malloc error in vain; this is related to XCode 4 compilation!
//	stringdata = AllocateZeroPool(sizeof(UINT8) * string->length);
//	CopyMem(stringdata, (UINT8*)devprop_generate_string(string), string->length);
	stringlength = string->length * 2;
	// -------------------------------------------------
  gDeviceProperties = AllocateAlignedPages(EFI_SIZE_TO_PAGES(stringlength + 1), 64);
	CopyMem(gDeviceProperties, (VOID*)devprop_generate_string(string), stringlength);
	DBG(gDeviceProperties);
  DBG("\n");
	
	DBG("ATI %a %a %dMB (%a) [%04x:%04x] (subsys [%04x:%04x]):: %a\n",
			chip_family_name[card->info->chip_family], card->info->model_name,
			(UINT32)(card->vram_size / (1024 * 1024)), card->cfg_name,
			ati_dev->vendor_id, ati_dev->device_id,
			ati_dev->subsys_id.subsys.vendor_id, ati_dev->subsys_id.subsys.device_id,
			devicepath);
	
	FreePool(card);
	
	return TRUE;
}
