/*
 * Pci.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef INCLUDE_PCI_H_
#define INCLUDE_PCI_H_


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
	UINT32		:2;
	UINT32	reg :6;
	UINT32	func:3;
	UINT32	dev :5;
	UINT32	bus :8;
	UINT32		:7;
	UINT32	eb	:1;
} pci_addr_t;

typedef union {
	pci_addr_t	bits;
	UINT32	    addr;
} pci_dev_t;

typedef struct pci_dt_t {
//  EFI_PCI_IO_PROTOCOL		*PciIo;
//  PCI_TYPE00            Pci;
  EFI_HANDLE    DeviceHandle;
  UINT8*        regs;
	pci_dev_t			dev;

	UINT16				vendor_id;
	UINT16				device_id;

	union {
		struct {
			UINT16	  vendor_id;
			UINT16	  device_id;
		} subsys;
		UINT32	    subsys_id;
	} subsys_id;
	UINT8		      revision;
	UINT8		      subclass;
	UINT16				class_id;

	struct pci_dt_t			*parent;
	struct pci_dt_t			*children;
	struct pci_dt_t			*next;
  BOOLEAN             used;
} pci_dt_t;


#endif /* INCLUDE_PCI_H_ */
