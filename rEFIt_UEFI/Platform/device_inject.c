/*
 *	Copyright 2009 Jasmin Fazlic All rights reserved.
 */
/*
 *	Cleaned and merged by iNDi
 */
// UEFI adaptation by usr-sse2



#include "Platform.h"

#ifndef DEBUG_INJECT
#define DEBUG_INJECT 1
#endif

#if DEBUG_INJECT == 2
#define DBG(x...)	AsciiPrint(x)
#elif DEBUG_INJECT == 1
#define DBG(x...) MsgLog(x)
#else
#define DBG(x...)
#endif

UINT32 devices_number = 1;
UINT32 builtin_set = 0;
struct DevPropString *string = 0;
UINT8 *stringdata = 0;
UINT32 stringlength = 0;

pci_dt_t* nvdevice;

static UINT16 dp_swap16(UINT16 toswap)
{
    return (((toswap & 0x00FF) << 8) | ((toswap & 0xFF00) >> 8));
}

static UINT32 dp_swap32(UINT32 toswap)
{
    return  ((toswap & 0x000000FF) << 24) |
	((toswap & 0x0000FF00) << 8 ) |
	((toswap & 0x00FF0000) >> 8 ) |
	((toswap & 0xFF000000) >> 24);
}	


struct DevPropString *devprop_create_string(VOID)
{
	DBG("devprop_create_string\n");
	string = (struct DevPropString*)AllocateZeroPool(sizeof(struct DevPropString));
	
	if(string == NULL)
		return NULL;
	
//	ZeroMem((VOID*)string, sizeof(struct DevPropString));
	string->length = 12;
	string->WHAT2 = 0x01000000;
	return string;
}

UINT8 ascii_hex_to_int (CHAR8* buf)
{
	INT8 i = 0;
	switch (AsciiStrLen(buf)) {
		case 1:
			if (buf[0]>='0' && buf[0]<='9')
				i = buf[0]-'0';
			else
				i = buf[0]-'A'; //no error checking
			break;
		case 2:
			if (buf[0]>='0' && buf[0]<='9')
				i = buf[0]-'0';
			else
				i = buf[0]-'A'; //no error checking
			i *= 16;
			if (buf[1]>='0' && buf[1]<='9')
				i += buf[1]-'0';
			else
				i += buf[1]-'A'; //no error checking
			break;
	}
	return i;
}

CHAR8 *get_pci_dev_path(pci_dt_t *pci_dt)
{
//	DBG("get_pci_dev_path");
	CHAR8*		tmp;
	CHAR16*		devpathstr;
	EFI_DEVICE_PATH_PROTOCOL*	DevicePath;
	
	
	EFI_STATUS Status;
	EFI_PCI_IO_PROTOCOL		*PciIo;
	PCI_TYPE00				Pci;
	UINTN					HandleCount;
	UINTN					ArrayCount;
	UINTN					HandleIndex;
	UINTN					ProtocolIndex;
	EFI_HANDLE				*HandleBuffer;
	EFI_GUID				**ProtocolGuidArray;
	
	
	Status = gBS->LocateHandleBuffer(AllHandles,NULL,NULL,&HandleCount,&HandleBuffer);
	if (EFI_ERROR(Status)) return 0;
	for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
		Status = gBS->ProtocolsPerHandle(HandleBuffer[HandleIndex],&ProtocolGuidArray,&ArrayCount);
		if (EFI_ERROR(Status)) continue;
		for (ProtocolIndex = 0; ProtocolIndex < ArrayCount; ProtocolIndex++) {
			if (CompareGuid(&gEfiPciIoProtocolGuid, ProtocolGuidArray[ProtocolIndex])) {
				Status = gBS->OpenProtocol(HandleBuffer[HandleIndex], &gEfiPciIoProtocolGuid, (VOID**)&PciIo, gImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
				if (EFI_ERROR(Status)) continue;
				Status = PciIo->Pci.Read(PciIo,EfiPciIoWidthUint32, 0, sizeof(Pci) / sizeof(UINT32), &Pci);
				if (EFI_ERROR(Status)) continue;
				if ((Pci.Hdr.VendorId == pci_dt->vendor_id) && (Pci.Hdr.DeviceId == pci_dt->device_id)) {
					DevicePath = DevicePathFromHandle (HandleBuffer[HandleIndex]);
					if (!DevicePath) continue;
					devpathstr = DevicePathToStr(DevicePath);
					tmp = AllocatePool((StrLen(devpathstr)+1)*sizeof(CHAR8));
					UnicodeStrToAsciiStr(devpathstr, tmp);		
					return tmp;
				}
			}
		}
	}
	return NULL;
}

UINT32 pci_config_read32(UINT32 pci_addr, UINT8 reg)
{
//	DBG("pci_config_read32\n");
	EFI_STATUS Status;
	EFI_PCI_IO_PROTOCOL		*PciIo;
	PCI_TYPE00				Pci;
	UINTN					HandleCount;
	UINTN					ArrayCount;
	UINTN					HandleIndex;
	UINTN					ProtocolIndex;
	EFI_HANDLE				*HandleBuffer;
	EFI_GUID				**ProtocolGuidArray;
	UINT32					res;
	
	
	Status = gBS->LocateHandleBuffer(AllHandles,NULL,NULL,&HandleCount,&HandleBuffer);
	if (EFI_ERROR(Status)) return 0;
	for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
		Status = gBS->ProtocolsPerHandle(HandleBuffer[HandleIndex],&ProtocolGuidArray,&ArrayCount);
		if (EFI_ERROR(Status)) continue;
		for (ProtocolIndex = 0; ProtocolIndex < ArrayCount; ProtocolIndex++) {
			if (CompareGuid(&gEfiPciIoProtocolGuid, ProtocolGuidArray[ProtocolIndex])) {
				Status = gBS->OpenProtocol(HandleBuffer[HandleIndex], &gEfiPciIoProtocolGuid, (VOID**)&PciIo, gImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
				if (EFI_ERROR(Status)) continue;
				Status = PciIo->Pci.Read(PciIo,EfiPciIoWidthUint32, 0, sizeof(Pci) / sizeof(UINT32), &Pci);
				if (EFI_ERROR(Status)) continue;
				if ((Pci.Hdr.VendorId != nvdevice->vendor_id) || (Pci.Hdr.DeviceId != nvdevice->device_id)) continue;
				//return ((UINT32*)&Pci)[reg];	
				Status = PciIo->Pci.Read(
										 PciIo,
										 EfiPciIoWidthUint32,
										 reg & ~3,
										 1,
										 &res
										 );
				if (EFI_ERROR(Status)) {
					DBG("pci_config_read32 failed\n");
					return 0;
				}
				return res;										 
			}
		}
	}
	return 0;
}

/*VOID WRITETEG32 (UINT32 reg, UINT32 value) {
	AsciiPrint("WRITEREG32\n");
	EFI_STATUS Status;
	EFI_PCI_IO_PROTOCOL		*PciIo;
	PCI_TYPE00				Pci;
	UINTN					HandleCount;
	UINTN					ArrayCount;
	UINTN					HandleIndex;
	UINTN					ProtocolIndex;
	EFI_HANDLE				*HandleBuffer;
	EFI_GUID				**ProtocolGuidArray;	
	
	Status = gBS->LocateHandleBuffer(AllHandles,NULL,NULL,&HandleCount,&HandleBuffer);
	if (EFI_ERROR(Status)) return 0;
	for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
		Status = gBS->ProtocolsPerHandle(HandleBuffer[HandleIndex],&ProtocolGuidArray,&ArrayCount);
		if (EFI_ERROR(Status)) continue;
		for (ProtocolIndex = 0; ProtocolIndex < ArrayCount; ProtocolIndex++) {
			if (CompareGuid(&gEfiPciIoProtocolGuid, ProtocolGuidArray[ProtocolIndex])) {
				Status = gBS->OpenProtocol(HandleBuffer[HandleIndex], &gEfiPciIoProtocolGuid, (VOID**)&PciIo, gImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
				if (EFI_ERROR(Status)) continue;
				Status = PciIo->Pci.Read(PciIo,EfiPciIoWidthUint32, 0, sizeof(Pci) / sizeof(UINT32), &Pci);
				if (EFI_ERROR(Status)) continue;
				if ((Pci.Hdr.VendorId != nvdevice->vendor_id) || (Pci.Hdr.DeviceId != nvdevice->device_id)) continue;
				//return ((UINT32*)&Pci)[reg];	
				Status = PciIo->Mem.Write(
										 PciIo,
										 EfiPciIoWidthUint32,
										 0, //BAR 0
										 reg,
										 1,
										 &value
										 );
				if (EFI_ERROR(Status))
					AsciiPrint("WRITEREG32 failed\n");
				return;										 
			}
		}
	}
}*/

UINT32 REG32(UINT32 reg)
{
	//AsciiPrint("REG32\n");
	EFI_STATUS Status;
	EFI_PCI_IO_PROTOCOL		*PciIo;
	PCI_TYPE00				Pci;
	UINTN					HandleCount;
	UINTN					ArrayCount;
	UINTN					HandleIndex;
	UINTN					ProtocolIndex;
	EFI_HANDLE				*HandleBuffer;
	EFI_GUID				**ProtocolGuidArray;
	UINT32					res;
	
	
	Status = gBS->LocateHandleBuffer(AllHandles,NULL,NULL,&HandleCount,&HandleBuffer);
	if (EFI_ERROR(Status)) return 0;
	for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
		Status = gBS->ProtocolsPerHandle(HandleBuffer[HandleIndex],&ProtocolGuidArray,&ArrayCount);
		if (EFI_ERROR(Status)) continue;
		for (ProtocolIndex = 0; ProtocolIndex < ArrayCount; ProtocolIndex++) {
			if (CompareGuid(&gEfiPciIoProtocolGuid, ProtocolGuidArray[ProtocolIndex])) {
				Status = gBS->OpenProtocol(HandleBuffer[HandleIndex], &gEfiPciIoProtocolGuid, (VOID**)&PciIo, gImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
				if (EFI_ERROR(Status)) continue;
				Status = PciIo->Pci.Read(PciIo,EfiPciIoWidthUint32, 0, sizeof(Pci) / sizeof(UINT32), &Pci);
				if (EFI_ERROR(Status)) continue;
				if ((Pci.Hdr.VendorId != nvdevice->vendor_id) || (Pci.Hdr.DeviceId != nvdevice->device_id)) continue;
				//return ((UINT32*)&Pci)[reg];	
				Status = PciIo->Mem.Read(
										 PciIo,
										 EfiPciIoWidthUint32,
										 0, //BAR 0
										 reg,
										 1,
										 &res
										 );
				if (EFI_ERROR(Status)) {
					AsciiPrint("REG32 failed\n");
					return 0;
				}
				return (UINT32)res;										 
			}
		}
	}
	return 0;
}


/* This is only for EFI ROMs, not BIOS.
VOID* PCIReadRom(pci_dt_t* device)
{
	AsciiPrint("PCIReadRom\n");
	EFI_STATUS Status;
	EFI_PCI_IO_PROTOCOL		*PciIo;
	PCI_TYPE00				Pci;
	UINTN					HandleCount;
	UINTN					ArrayCount;
	UINTN					HandleIndex;
	UINTN					ProtocolIndex;
	EFI_HANDLE				*HandleBuffer;
	EFI_GUID				**ProtocolGuidArray;
	UINT64 ii;
	//VOID* rom;
	
	
	Status = gBS->LocateHandleBuffer(AllHandles,NULL,NULL,&HandleCount,&HandleBuffer);
	if (EFI_ERROR(Status)) return 0;
	for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
		Status = gBS->ProtocolsPerHandle(HandleBuffer[HandleIndex],&ProtocolGuidArray,&ArrayCount);
		if (EFI_ERROR(Status)) continue;
		for (ProtocolIndex = 0; ProtocolIndex < ArrayCount; ProtocolIndex++) {
			if (CompareGuid(&gEfiPciIoProtocolGuid, ProtocolGuidArray[ProtocolIndex])) {
				Status = gBS->OpenProtocol(HandleBuffer[HandleIndex], &gEfiPciIoProtocolGuid, (VOID**)&PciIo, gImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
				if (EFI_ERROR(Status)) continue;
				Status = PciIo->Pci.Read(PciIo,EfiPciIoWidthUint32, 0, sizeof(Pci) / sizeof(UINT32), &Pci);
				if (EFI_ERROR(Status)) continue;
				if ((Pci.Hdr.VendorId != device->vendor_id) || (Pci.Hdr.DeviceId != device->device_id)) continue;
				//return ((UINT32*)&Pci)[reg];	
				AsciiPrint("%d\n",PciIo->RomSize);
				for (ii=0; ii<PciIo->RomSize; ii++) {
					AsciiPrint("%0x ", ((UINT8*)(PciIo->RomImage))[ii]); 
				}
				return PciIo->RomImage;								 
			}
		}
	}
	return 0;
}*/



/*UINT32 pci_config_read32(UINT32 pci_addr, UINT8 reg)
{
	pci_addr |= reg & ~3;
	outl(PCI_ADDR_REG, pci_addr);
	return inl(PCI_DATA_REG);
}*/

 
struct DevPropDevice *devprop_add_device(struct DevPropString *string, CHAR8 *path)
{
	//DBG("devprop_add_device\n");
	struct DevPropDevice	*device;
	const CHAR8		pciroot_string[] = "PciRoot(0x";
	const CHAR8		pcieroot_string[] = "PcieRoot(0x";
	const CHAR8		pci_device_string[] = "Pci(0x";

	if (string == NULL || path == NULL) {
		return NULL;
	}
	device = AllocateZeroPool(sizeof(struct DevPropDevice));

	if (AsciiStrnCmp(path, pciroot_string, AsciiStrLen(pciroot_string)) &&
		AsciiStrnCmp(path, pcieroot_string, AsciiStrLen(pcieroot_string))) {
		DBG("ERROR parsing device path\n");
		return NULL;
	}

	ZeroMem((VOID*)device, sizeof(struct DevPropDevice));
	device->acpi_dev_path._UID = /*getPciRootUID();*/0; //FIXME: what if 0?

	INT32 numpaths = 0;
	INT32		x, curr = 0;
	CHAR8	buff[] = "00";

	for (x = 0; x < AsciiStrLen(path); x++) {
		if (!AsciiStrnCmp(&path[x], pci_device_string, AsciiStrLen(pci_device_string))) {
			x+=AsciiStrLen(pci_device_string);
			curr=x;
			while(path[++x] != ',');
			if(x-curr == 2)
				AsciiSPrint(buff, 3, "%c%c", path[curr], path[curr+1]);
			else if(x-curr == 1)
				AsciiSPrint(buff, 3, "%c", path[curr]);
			else 
			{
				DBG("ERROR parsing device path\n");
				numpaths = 0;
				break;
			}
			device->pci_dev_path[numpaths].device =	ascii_hex_to_int(buff);
			
			x += 3; // 0x
			curr = x;
			while(path[++x] != ')');
			if(x-curr == 2)
				AsciiSPrint(buff, 3, "%c%c", path[curr], path[curr+1]);
			else if(x-curr == 1)
				AsciiSPrint(buff, 3, "%c", path[curr]);
			else
			{
				DBG("ERROR parsing device path\n");
				numpaths = 0;
				break;
			}
			device->pci_dev_path[numpaths].function = ascii_hex_to_int(buff); // TODO: find dev from CHAR8 *path
			
			numpaths++;
		}
	}
	
	if(!numpaths)
		return NULL;
	
	device->numentries = 0x00;
	
	device->acpi_dev_path.length = 0x0c;
	device->acpi_dev_path.type = 0x02;
	device->acpi_dev_path.subtype = 0x01;
	device->acpi_dev_path._HID = 0xd041030a;
	
	device->num_pci_devpaths = numpaths;
	device->length = 24 + (6*numpaths);
	
	INT32		i; 
	
	for(i = 0; i < numpaths; i++)
	{
		device->pci_dev_path[i].length = 0x06;
		device->pci_dev_path[i].type = 0x01;
		device->pci_dev_path[i].subtype = 0x01;
	}
	
	device->path_end.length = 0x04;
	device->path_end.type = 0x7f;
	device->path_end.subtype = 0xff;
	
	device->string = string;
	device->data = NULL;
	string->length += device->length;
	
	if(!string->entries)
		if((string->entries = (struct DevPropDevice**)AllocatePool(sizeof(device)))== NULL)
			return 0;
	
	string->entries[string->numentries++] = (struct DevPropDevice*)AllocatePool(sizeof(device));
	string->entries[string->numentries-1] = device;
	
	return device;
}

INT32 devprop_add_value(struct DevPropDevice *device, CHAR8 *nm, UINT8 *vl, UINT32 len)
{
	DBG("devprop_add_value\n");
	if(!nm || !vl || !len)
		return 0;
	
	UINT32 length = ((AsciiStrLen(nm) * 2) + len + (2 * sizeof(UINT32)) + 2);
	UINT8 *data = (UINT8*)AllocatePool(length);
	{
		if(!data)
			return 0;
		
		ZeroMem((VOID*)data, length);
		UINT32 off= 0;
		data[off+1] = ((AsciiStrLen(nm) * 2) + 6) >> 8;
		data[off] =   ((AsciiStrLen(nm) * 2) + 6) & 0x00FF;
		
		off += 4;
		UINT32 i=0, l = AsciiStrLen(nm);
		for(i = 0 ; i < l ; i++, off += 2)
		{
			data[off] = *nm++;
		}
		
		off += 2;
		l = len;
		UINT32 *datalength = (UINT32*)&data[off];
		*datalength = l + 4;
		off += 4;
		for(i = 0 ; i < l ; i++, off++)
		{
			data[off] = *vl++;
		}
	}	
	
	UINT32 offset = device->length - (24 + (6 * device->num_pci_devpaths));
	
	UINT8 *newdata = (UINT8*)AllocatePool((length + offset));
	if(!newdata)
		return 0;
	if(device->data)
		if(offset > 1)
			CopyMem((VOID*)newdata, (VOID*)device->data, offset);

	CopyMem((VOID*)(newdata + offset), (VOID*)data, length);
	
	device->length += length;
	device->string->length += length;
	device->numentries++;
	
	if(!device->data)
		device->data = (UINT8*)AllocatePool(sizeof(UINT8));
	else
		FreePool(device->data);
	
	FreePool(data);
	device->data = newdata;
	
	return 1;
}

CHAR8 *devprop_generate_string(struct DevPropString *string)
{
	DBG("devprop_generate_string\n");
	CHAR8 *buffer = (CHAR8*)AllocatePool(string->length * 2);
	CHAR8 *ptr = buffer;
	
	if(!buffer)
		return NULL;

	AsciiSPrint(buffer, string->length * 2, "%08x%08x%04x%04x", dp_swap32(string->length), string->WHAT2,
			dp_swap16(string->numentries), string->WHAT3);
	buffer += 24;
	INT32 i = 0, x = 0;
	
	while(i < string->numentries)
	{
		AsciiSPrint(buffer, string->length * 2, "%08x%04x%04x", dp_swap32(string->entries[i]->length),
				dp_swap16(string->entries[i]->numentries), string->entries[i]->WHAT2); //FIXME: wrong buffer sizes!
		
		buffer += 16;
		AsciiSPrint(buffer, string->length * 2, "%02x%02x%04x%08x%08x", string->entries[i]->acpi_dev_path.type,
				string->entries[i]->acpi_dev_path.subtype,
				dp_swap16(string->entries[i]->acpi_dev_path.length),
				string->entries[i]->acpi_dev_path._HID,
				dp_swap32(string->entries[i]->acpi_dev_path._UID));

		buffer += 24;
		for(x=0;x < string->entries[i]->num_pci_devpaths; x++)
		{
			AsciiSPrint(buffer, string->length * 2, "%02x%02x%04x%02x%02x", string->entries[i]->pci_dev_path[x].type,
					string->entries[i]->pci_dev_path[x].subtype,
					dp_swap16(string->entries[i]->pci_dev_path[x].length),
					string->entries[i]->pci_dev_path[x].function,
					string->entries[i]->pci_dev_path[x].device);
			buffer += 12;
		}
		
		AsciiSPrint(buffer, string->length * 2, "%02x%02x%04x", string->entries[i]->path_end.type,
				string->entries[i]->path_end.subtype,
				dp_swap16(string->entries[i]->path_end.length));
		
		buffer += 8;
		UINT8 *dataptr = string->entries[i]->data;
		for(x = 0; x < (string->entries[i]->length) - (24 + (6 * string->entries[i]->num_pci_devpaths)) ; x++)
		{
			AsciiSPrint(buffer, string->length * 2, "%02x", *dataptr++);
			buffer += 2;
		}
		i++;
	}
	return ptr;
}

VOID devprop_free_string(struct DevPropString *string)
{
	//DBG("devprop_free_string\n");
	if(!string)
		return;
	
	INT32 i;
	for(i = 0; i < string->numentries; i++)
	{
		if(string->entries[i])
		{
			if(string->entries[i]->data)
			{
				FreePool(string->entries[i]->data);
				string->entries[i]->data = NULL;
			}
			FreePool(string->entries[i]);
			string->entries[i] = NULL;
		}
	}
	
	FreePool(string);
	string = NULL;
}