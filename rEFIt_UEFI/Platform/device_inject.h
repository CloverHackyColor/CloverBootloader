/*
 *	Copyright 2009 Jasmin Fazlic All rights reserved.
 */
/*
 *	Cleaned and merged by iNDi
 */

#ifndef __LIBSAIO_DEVICE_INJECT_H
#define __LIBSAIO_DEVICE_INJECT_H

#define DP_ADD_TEMP_VAL(dev, val) devprop_add_value(dev, (CHAR8*)val[0], (UINT8*)val[1], AsciiStrLen(val[1]) + 1)
#define DP_ADD_TEMP_VAL_DATA(dev, val) devprop_add_value(dev, (CHAR8*)val.name, (UINT8*)val.data, val.size)
#define MAX_PCI_DEV_PATHS 4

//#define REG8(reg)  ((volatile UINT8 *)regs)[(reg)]
//#define REG16(reg)  ((volatile UINT16 *)regs)[(reg) >> 1]
//#define REG32(reg)  ((volatile UINT32 *)regs)[(reg) >> 2]
//UINT32 REG32(UINT32 reg);
//VOID WRITEREG32 (UINT32 reg, UINT32 value);


extern struct DevPropString *string;
extern UINT8 *stringdata;
extern UINT32 stringlength;

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
	UINT32	addr;
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
			UINT16	vendor_id;
			UINT16	device_id;
		} subsys;
		UINT32	subsys_id;
	}subsys_id;
	UINT8		revision;
	UINT8		subclass;
	UINT16				class_id;	
	
	struct pci_dt_t			*parent;
	struct pci_dt_t			*children;
	struct pci_dt_t			*next;
} pci_dt_t;
/* Option ROM header */
typedef struct {
	UINT16		signature;		// 0xAA55
	UINT8			rom_size;
	UINT32		entry_point;
	UINT8			reserved[15];
	UINT16		pci_header_offset;
	UINT16		expansion_header_offset;
} option_rom_header_t;

/* Option ROM PCI Data Structure */
typedef struct {
	UINT32		signature;		// ati - 0x52494350, nvidia - 0x50434952, 'PCIR'
	UINT16		vendor_id;
	UINT16		device_id;
	UINT16		vital_product_data_offset;
	UINT16		structure_length;
	UINT8			structure_revision;
	UINT8			class_code[3];
	UINT16		image_length;
	UINT16		image_revision;
	UINT8			code_type;
	UINT8			indicator;
	UINT16		reserved;
} option_rom_pci_header_t;


CHAR8  *get_pci_dev_path(pci_dt_t *pci_dt);
UINT32 pci_config_read32(pci_dt_t *pci_dt, UINT8 reg);
extern pci_dt_t* nvdevice;
VOID* PCIReadRom(pci_dt_t* device);

#if 0 //never do this
extern VOID setupDeviceProperties(Node *node);
#endif

struct ACPIDevPath {
	UINT8		type;		// = 2 ACPI device-path
	UINT8		subtype;	// = 1 ACPI Device-path
	UINT16	length;		// = 0x0c
	UINT32	_HID;		// = 0xD041030A ?
	UINT32	_UID;		// = 0x00000000 PCI ROOT
};

struct PCIDevPath {
	UINT8		type;		// = 1 Hardware device-path
	UINT8		subtype;	// = 1 PCI
	UINT16	length;		// = 6
	UINT8		function;	// pci func number
	UINT8		device;		// pci dev number
};

struct DevicePathEnd {
	UINT8		type;		// = 0x7f
	UINT8		subtype;	// = 0xff
	UINT16	length;		// = 4;
};

struct DevPropDevice {
	UINT32 length;
	UINT16 numentries;
	UINT16 WHAT2;										// 0x0000 ?
	struct ACPIDevPath acpi_dev_path;					// = 0x02010c00 0xd041030a
	struct PCIDevPath  pci_dev_path[MAX_PCI_DEV_PATHS]; // = 0x01010600 func dev
	struct DevicePathEnd path_end;						// = 0x7fff0400
	UINT8 *data;
	
	// ------------------------
	UINT8	 num_pci_devpaths;
	struct DevPropString *string;
	// ------------------------	
};

typedef struct DevPropDevice DevPropDevice;

struct DevPropString {
	UINT32 length;
	UINT32 WHAT2;			// 0x01000000 ?
	UINT16 numentries;
	UINT16 WHAT3;			// 0x0000     ?
	DevPropDevice **entries;
};

struct DevPropString	*devprop_create_string(void);
DevPropDevice	*devprop_add_device(struct DevPropString *string, char *path);
BOOLEAN		devprop_add_value(DevPropDevice *device, char *nm, UINT8 *vl, UINT32 len);
CHAR8			*devprop_generate_string(struct DevPropString *string);
VOID			devprop_free_string(struct DevPropString *string);

#endif /* !__LIBSAIO_DEVICE_INJECT_H */
