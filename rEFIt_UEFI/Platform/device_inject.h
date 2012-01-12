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

extern struct DevPropString *string;
extern UINT8 *stringdata;
extern UINT32 stringlength;

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

struct DevPropString {
	UINT32 length;
	UINT32 WHAT2;			// 0x01000000 ?
	UINT16 numentries;
	UINT16 WHAT3;			// 0x0000     ?
	struct DevPropDevice **entries;
};

struct DevPropString	*devprop_create_string(void);
struct DevPropDevice	*devprop_add_device(struct DevPropString *string, char *path);
INT32			devprop_add_value(struct DevPropDevice *device, char *nm, UINT8 *vl, UINT32 len);
CHAR8			*devprop_generate_string(struct DevPropString *string);
VOID			devprop_free_string(struct DevPropString *string);

#endif /* !__LIBSAIO_DEVICE_INJECT_H */
