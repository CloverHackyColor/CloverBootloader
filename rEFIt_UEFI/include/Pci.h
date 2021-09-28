/*
 * Pci.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef INCLUDE_PCI_H_
#define INCLUDE_PCI_H_

#include <stdint.h>
#include "../cpp_foundation/XBool.h"

/* PCI */
#define PCI_BASE_ADDRESS_0          0x10    /* 32 bits */
#define PCI_BASE_ADDRESS_1          0x14    /* 32 bits [htype 0,1 only] */
#define PCI_BASE_ADDRESS_2          0x18    /* 32 bits [htype 0 only] */
#define PCI_BASE_ADDRESS_3          0x1c    /* 32 bits */
#define PCI_BASE_ADDRESS_4          0x20    /* 32 bits */
#define PCI_BASE_ADDRESS_5          0x24    /* 32 bits */

#define PCI_CLASS_MEDIA_HDA         0x03


#define PCIADDR(bus, dev, func)      ((1 << 31) | ((bus) << 16) | ((dev) << 11) | ((func) << 8))


typedef struct {
	uint32_t		:2;
	uint32_t	reg :6;
	uint32_t	func:3;
	uint32_t	dev :5;
	uint32_t	bus :8;
	uint32_t		:7;
	uint32_t	eb	:1;
} pci_addr_t;

typedef union {
  uint32_t    addr = 0;
	pci_addr_t	bits;
} pci_dev_t;

typedef struct pci_dt_t {
//  EFI_PCI_IO_PROTOCOL		*PciIo;
//  PCI_TYPE00            Pci;
  EFI_HANDLE    DeviceHandle = NULL;
  UINT8*        regs         = NULL;
	pci_dev_t			dev          = pci_dev_t();

	UINT16				vendor_id    = 0;
	UINT16				device_id    = 0;

	union {
		struct {
			UINT16	  vendor_id;
			UINT16	  device_id;
		} subsys;
		UINT32	    subsys_id;
  } subsys_id_union = { {0, 0} };
	UINT8		      revision      = 0;
	UINT8		      subclass      = 0;
	UINT16				class_id      = 0;

	struct pci_dt_t	*parent     = NULL;
	struct pci_dt_t	*children   = NULL;
	struct pci_dt_t	*next       = NULL;
  XBool            used       = false;
} pci_dt_t;


#endif /* INCLUDE_PCI_H_ */
