/*
 *
 * Copyright 2008 by Islam M. Ahmed Zaid. All rights reserved.
 *
 */

#include "libsaio.h"
#include "bootstruct.h"
#include "boot.h"
#include "pci.h"
#include "pci_root.h"

#ifndef DEBUG_PCI
#ifndef DEBUG_ALL
#define DEBUG_PCI 0
#else
#define DEBUG_PCI DEBUG_ALL
#endif
#endif

#if DEBUG_PCI==0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_PCI, __VA_ARGS__)
#endif

pci_dt_t	*root_pci_dev;


uint8_t pci_config_read8(uint32_t pci_addr, uint8_t reg)
{
	pci_addr |= reg & ~3;
	outl(PCI_ADDR_REG, pci_addr);
	return inb(PCI_DATA_REG + (reg & 3));
}

uint16_t pci_config_read16(uint32_t pci_addr, uint8_t reg)
{
	pci_addr |= reg & ~3;
	outl(PCI_ADDR_REG, pci_addr);
	return inw(PCI_DATA_REG + (reg & 2));
}

uint32_t pci_config_read32(uint32_t pci_addr, uint8_t reg)
{
	pci_addr |= reg & ~3;
	outl(PCI_ADDR_REG, pci_addr);
	return inl(PCI_DATA_REG);
}

void pci_config_write8(uint32_t pci_addr, uint8_t reg, uint8_t data)
{
	pci_addr |= reg & ~3;
	outl(PCI_ADDR_REG, pci_addr);
	outb(PCI_DATA_REG + (reg & 3), data);
}

void pci_config_write16(uint32_t pci_addr, uint8_t reg, uint16_t data)
{
	pci_addr |= reg & ~3;
	outl(PCI_ADDR_REG, pci_addr);
	outw(PCI_DATA_REG + (reg & 2), data);
}

void pci_config_write32(uint32_t pci_addr, uint8_t reg, uint32_t data)
{
	pci_addr |= reg & ~3;
	outl(PCI_ADDR_REG, pci_addr);
	outl(PCI_DATA_REG, data);
}

void scan_pci_bus(pci_dt_t *start, uint8_t bus)
{
	pci_dt_t	*new;
	pci_dt_t	**current = &start->children;
	uint32_t	id;
	uint32_t	pci_addr;
	uint8_t		dev;
	uint8_t		func;
	uint8_t		secondary_bus;
	uint8_t		header_type;

	for (dev = 0; dev < 32; dev++)
	{
		for (func = 0; func < 8; func++)
		{
			pci_addr = PCIADDR(bus, dev, func);
			id = pci_config_read32(pci_addr, PCI_VENDOR_ID);
			if (!id || id == 0xffffffff) {
				continue;
			}
			new = (pci_dt_t*)malloc(sizeof(pci_dt_t));
			if (!new)
				return;
			memset(new, 0, sizeof(pci_dt_t));
			
			new->dev.addr				= pci_addr;
			new->vendor_id				= id & 0xffff;
			new->device_id				= (id >> 16) & 0xffff;
			new->subsys_id.subsys_id	= pci_config_read32(pci_addr, PCI_SUBSYSTEM_VENDOR_ID);
			new->subclass				= pci_config_read8(pci_addr, PCI_CLASS_PROG);
			new->class_id				= pci_config_read16(pci_addr, PCI_CLASS_DEVICE);
			new->parent	= start;

			header_type = pci_config_read8(pci_addr, PCI_HEADER_TYPE);
			switch (header_type & 0x7f)
			{
			case PCI_HEADER_TYPE_BRIDGE:
			case PCI_HEADER_TYPE_CARDBUS:
				secondary_bus = pci_config_read8(pci_addr, PCI_SECONDARY_BUS);
				if (secondary_bus != 0) {
					scan_pci_bus(new, secondary_bus);
				}
				break;
			}
			*current = new;
			current = &new->next;

			if ((func == 0) && ((header_type & 0x80) == 0))
			{
				break;
			}
		}
	}
}

void enable_pci_devs(void)
{
	uint16_t id;
	uint32_t rcba, *fd;

	id = pci_config_read16(PCIADDR(0, 0x00, 0), 0x00);
	/* make sure we're on Intel chipset */
	if (id != 0x8086)
		return;
	rcba = pci_config_read32(PCIADDR(0, 0x1f, 0), 0xf0) & ~1; //this is LPC host
	fd = (uint32_t *)(rcba + 0x3418);
	/* set SMBus Disable (SD) to 0 */
	*fd &= ~0x8;
	/* and all devices? */
	//*fd = 0x1;
}


void build_pci_dt(void)
{
	root_pci_dev = malloc(sizeof(pci_dt_t));
	
	if (!root_pci_dev)
		return;
	gRootPCIDev = (void*)root_pci_dev;
	bzero(root_pci_dev, sizeof(pci_dt_t));
	enable_pci_devs();
	scan_pci_bus(root_pci_dev, 0);
#if 1 //DEBUG_PCI
	dump_pci_dt(root_pci_dev->children);
	//pause();
#endif
}

static char dev_path[256];
char *get_pci_dev_path(pci_dt_t *pci_dt)
{
	pci_dt_t	*current;
	pci_dt_t	*end;
	char		tmp[64];

	dev_path[0] = 0;
	end = root_pci_dev;
	
	int uid = getPciRootUID();
	while (end != pci_dt)
	{
		current = pci_dt;
		while (current->parent != end)
			current = current->parent;			
		end = current;
		if (current->parent == root_pci_dev)
		{
			sprintf(tmp, "PciRoot(0x%x)/Pci(0x%x,0x%x)", uid, 
				current->dev.bits.dev, current->dev.bits.func);
		} else {
			sprintf(tmp, "/Pci(0x%x,0x%x)", 
				current->dev.bits.dev, current->dev.bits.func);
		}
		strcat(dev_path, tmp);
	}
	return dev_path;
}

void dump_pci_dt(pci_dt_t *pci_dt)
{
	pci_dt_t	*current;

	current = pci_dt;
	while (current) {
		msglog("%02x:%02x.%x [%04x%02x] [%04x:%04x] (subsys [%04x:%04x]):: %s\n", 
			current->dev.bits.bus, current->dev.bits.dev, current->dev.bits.func, 
			   current->class_id, current->subclass, current->vendor_id, current->device_id, 
			current->subsys_id.subsys.vendor_id, current->subsys_id.subsys.device_id, 
			get_pci_dev_path(current));
		dump_pci_dt(current->children);
		current = current->next;
	}
}
