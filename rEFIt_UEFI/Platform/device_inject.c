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
DevPropString *string = 0;
UINT8 *stringdata = 0;
UINT32 stringlength = 0;

//pci_dt_t* nvdevice;

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


DevPropString *devprop_create_string(VOID)
{
	DBG("Begin creating strings for devices:\n");
	string = (DevPropString*)AllocateZeroPool(sizeof(DevPropString));
	
	if(string == NULL)
		return NULL;
	
//	ZeroMem((VOID*)string, sizeof(DevPropString));
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
				i = buf[0]-'A'+10; //no error checking
			break;
		case 2:
			if (buf[0]>='0' && buf[0]<='9')
				i = buf[0]-'0';
			else
				i = buf[0]-'A'+10; //no error checking
			i *= 16;
			if (buf[1]>='0' && buf[1]<='9')
				i += buf[1]-'0';
			else
				i += buf[1]-'A'+10; //no error checking
			break;
	}
	return i;
}

CHAR8 *get_pci_dev_path(pci_dt_t *PciDt)
{
//	DBG("get_pci_dev_path");
	CHAR8*		tmp;
	CHAR16*		devpathstr = NULL;
	EFI_DEVICE_PATH_PROTOCOL*	DevicePath = NULL;
	
  DevicePath = DevicePathFromHandle (PciDt->DeviceHandle);
  if (!DevicePath)
    return NULL;
  devpathstr = DevicePathToStr(DevicePath);
  tmp = AllocateZeroPool((StrLen(devpathstr)+1)*sizeof(CHAR8));
  UnicodeStrToAsciiStr(devpathstr, tmp);		
  return tmp;
	
}

UINT32 pci_config_read32(pci_dt_t *PciDt, UINT8 reg)
{
//	DBG("pci_config_read32\n");
	EFI_STATUS Status;
	EFI_PCI_IO_PROTOCOL		*PciIo;
	PCI_TYPE00				Pci;
	UINT32					res;
	
  Status = gBS->OpenProtocol(PciDt->DeviceHandle, &gEfiPciIoProtocolGuid, (VOID**)&PciIo, gImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
  if (EFI_ERROR(Status)){
    DBG("pci_config_read cant open protocol\n");
    return 0;
  }
	Status = PciIo->Pci.Read(PciIo,EfiPciIoWidthUint32, 0, sizeof(Pci) / sizeof(UINT32), &Pci);
  if (EFI_ERROR(Status)) {
    DBG("pci_config_read cant read pci\n");
    return 0;
  }
  Status = PciIo->Pci.Read (
                            PciIo,
                            EfiPciIoWidthUint32,
                            (UINT64)(reg & ~3),
                            1,
                            &res
                            );
  if (EFI_ERROR(Status)) {
    DBG("pci_config_read32 failed %r\n", Status);
    return 0;
  }
  return res;										 
}

/*VOID WRITEREG32 (UINT32 reg, UINT32 value) {
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
/*
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
*/

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

 
DevPropDevice *devprop_add_device(DevPropString *string, CHAR8 *path)
{
	DevPropDevice	*device;
	const CHAR8		pciroot_string[] = "PciRoot(0x";
	const CHAR8		pcieroot_string[] = "PcieRoot(0x";
	const CHAR8		pci_device_string[] = "Pci(0x";

	if (string == NULL || path == NULL) {
		return NULL;
	}
// 	DBG("devprop_add_device %a\n", path);
 
	device = AllocateZeroPool(sizeof(DevPropDevice));

	if (AsciiStrnCmp(path, pciroot_string, AsciiStrLen(pciroot_string)) &&
		AsciiStrnCmp(path, pcieroot_string, AsciiStrLen(pcieroot_string))) {
		DBG("ERROR parsing device path\n");
		return NULL;
	}

	ZeroMem((VOID*)device, sizeof(DevPropDevice));
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
		if((string->entries = (DevPropDevice**)AllocatePool(sizeof(device)))== NULL)
			return 0;
	
	string->entries[string->numentries++] = (DevPropDevice*)AllocatePool(sizeof(device));
	string->entries[string->numentries-1] = device;
	
	return device;
}

BOOLEAN devprop_add_value(DevPropDevice *device, CHAR8 *nm, UINT8 *vl, UINT32 len)
{
  UINT32 offset;
  UINT32 off;
  UINT32 length;
  UINT8 *data;
  UINTN i, l;
  UINT32 *datalength;
  UINT8 *newdata;
  
	if(!device || !nm || !vl || !len)
		return FALSE;
/*	DBG("devprop_add_value %a=", nm);
  for (i=0; i<len; i++) {
    DBG("%02X", vl[i]);
  }
  DBG("\n"); */
	l = AsciiStrLen(nm);
	length = ((l * 2) + len + (2 * sizeof(UINT32)) + 2);
	data = (UINT8*)AllocateZeroPool(length);
  if(!data)
    return FALSE;
  
  off= 0;
  
  data[off+1] = ((l * 2) + 6) >> 8;
  data[off] =   ((l * 2) + 6) & 0x00FF;
  
  off += 4;
  
  for(i = 0 ; i < l ; i++, off += 2)
  {
    data[off] = *nm++;
  }
  
  off += 2;
  l = len;
  datalength = (UINT32*)&data[off];
  *datalength = l + 4;
  off += 4;
  for(i = 0 ; i < l ; i++, off++)
  {
    data[off] = *vl++;
  }
	
	offset = device->length - (24 + (6 * device->num_pci_devpaths));
	
	newdata = (UINT8*)AllocateZeroPool((length + offset));
	if(!newdata)
		return FALSE;
	if((device->data) && (offset > 1)){
			CopyMem((VOID*)newdata, (VOID*)device->data, offset);
  }

	CopyMem((VOID*)(newdata + offset), (VOID*)data, length);
	
	device->length += length;
	device->string->length += length;
	device->numentries++;		
	device->data = newdata;
  
	FreePool(data);	
	return TRUE;
}

CHAR8 *devprop_generate_string(DevPropString *string)
{
  UINTN len = string->length * 2;
	INT32 i = 0, x = 0;
  
	DBG("devprop_generate_string\n");
	CHAR8 *buffer = (CHAR8*)AllocatePool(len + 1);
	CHAR8 *ptr = buffer;
	
	if(!buffer)
		return NULL;

	AsciiSPrint(buffer, len, "%08x%08x%04x%04x", dp_swap32(string->length), string->WHAT2,
			dp_swap16(string->numentries), string->WHAT3);
	buffer += 24;
	
	while(i < string->numentries)
	{
		AsciiSPrint(buffer, len, "%08x%04x%04x", dp_swap32(string->entries[i]->length),
				dp_swap16(string->entries[i]->numentries), string->entries[i]->WHAT2); //FIXME: wrong buffer sizes!
		
		buffer += 16;
		AsciiSPrint(buffer, len, "%02x%02x%04x%08x%08x", string->entries[i]->acpi_dev_path.type,
				string->entries[i]->acpi_dev_path.subtype,
				dp_swap16(string->entries[i]->acpi_dev_path.length),
				string->entries[i]->acpi_dev_path._HID,
				dp_swap32(string->entries[i]->acpi_dev_path._UID));

		buffer += 24;
		for(x = 0; x < string->entries[i]->num_pci_devpaths; x++)
		{
			AsciiSPrint(buffer, len, "%02x%02x%04x%02x%02x", string->entries[i]->pci_dev_path[x].type,
					string->entries[i]->pci_dev_path[x].subtype,
					dp_swap16(string->entries[i]->pci_dev_path[x].length),
					string->entries[i]->pci_dev_path[x].function,
					string->entries[i]->pci_dev_path[x].device);
			buffer += 12;
		}
		
		AsciiSPrint(buffer, len, "%02x%02x%04x", string->entries[i]->path_end.type,
				string->entries[i]->path_end.subtype,
				dp_swap16(string->entries[i]->path_end.length));
		
		buffer += 8;
		UINT8 *dataptr = string->entries[i]->data;
		for(x = 0; x < (string->entries[i]->length) - (24 + (6 * string->entries[i]->num_pci_devpaths)); x++)
		{
			AsciiSPrint(buffer, len, "%02x", *dataptr++);
			buffer += 2;
		}
		i++;
	}
	return ptr;
}

VOID devprop_free_string(DevPropString *string)
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

// Ethernet built-in device injection
BOOLEAN set_eth_props(pci_dt_t *eth_dev)
{
	CHAR8           *devicepath;
  DevPropDevice   *device;
  UINT8           builtin = 0x0;
	
	if (!string)
    string = devprop_create_string();
    
  devicepath = get_pci_dev_path(eth_dev);
  device = devprop_add_device(string, devicepath);
  if (!device)
    return FALSE;
	// -------------------------------------------------
	DBG("LAN Controller [%04x:%04x] :: %a\n", eth_dev->vendor_id, eth_dev->device_id, devicepath);
	if (eth_dev->vendor_id != 0x168c && builtin_set == 0) 
  {
 		builtin_set = 1;
 		builtin = 0x01;
 	}
//  DBG("Setting dev.prop built-in=0x%x\n", builtin);
  devprop_add_value(device, "device_type", (UINT8*)"ethernet", 8);
	return devprop_add_value(device, "built-in", (UINT8*)&builtin, 1);
}

#define PCI_IF_XHCI 0x30

static UINT8 clock_id = 0;
BOOLEAN set_usb_props(pci_dt_t *usb_dev)
{
	CHAR8           *devicepath;
  DevPropDevice   *device;
  UINT8           builtin = 0x0;
  UINT16  current_available = 1200; //mA
  UINT16  current_extra     = 700;
  UINT16  current_in_sleep  = 1000;
  UINT32   fake_devid;
	
	if (!string)
    string = devprop_create_string();
  
  devicepath = get_pci_dev_path(usb_dev);
  device = devprop_add_device(string, devicepath);
  if (!device)
    return FALSE;
	// -------------------------------------------------
	DBG("USB Controller [%04x:%04x] :: %a\n", usb_dev->vendor_id, usb_dev->device_id, devicepath);
  //  DBG("Setting dev.prop built-in=0x%x\n", builtin);
  devprop_add_value(device, "AAPL,current-available", (UINT8*)&current_available, 2);
  devprop_add_value(device, "AAPL,current-extra",     (UINT8*)&current_extra, 2);
  devprop_add_value(device, "AAPL,current-in-sleep",  (UINT8*)&current_in_sleep, 2);
  devprop_add_value(device, "AAPL,clock-id", (UINT8*)&clock_id, 1);
  clock_id++;
  fake_devid = usb_dev->device_id && 0xFFFF;
  if ((fake_devid && 0xFF00) == 0x2900) {
    fake_devid &= 0xFEFF;
    devprop_add_value(device, "device-id", (UINT8*)&fake_devid, 4);
  }
  switch (usb_dev->subclass) {
    case PCI_IF_UHCI:
      devprop_add_value(device, "device_type", (UINT8*)"UHCI", 4);
      break;
    case PCI_IF_OHCI:
      devprop_add_value(device, "device_type", (UINT8*)"OHCI", 4);
      break;
    case PCI_IF_EHCI:
      devprop_add_value(device, "device_type", (UINT8*)"EHCI", 4);
      break;
    case PCI_IF_XHCI:
      devprop_add_value(device, "device_type", (UINT8*)"XHCI", 4);
      break;
    default:
      break;
  }
	return devprop_add_value(device, "built-in", (UINT8*)&builtin, 1);
}

// HDA layout-id device injection by dmazar

#define HDA_VMIN	0x02 // Minor, Major Version
#define HDA_GCTL	0x08 // Global Control Register
#define HDA_ICO		0x60 // Immediate Command Output Interface
#define HDA_IRI		0x64 // Immediate Response Input Interface
#define HDA_ICS		0x68 // Immediate Command Status

// executing HDA verb command using Immediate Command Input and Output Registers
UINT32 HDA_IC_sendVerb(EFI_PCI_IO_PROTOCOL *PciIo, UINT32 codecAdr, UINT32 nodeId, UINT32 verb)
{
	EFI_STATUS	Status;
	UINT16		ics = 0;
	UINT32		data32 = 0;
	UINT64		data64 = 0;
	
	// poll ICS[0] to become 0
	Status = PciIo->PollMem(PciIo, EfiPciIoWidthUint16, 0/*bar*/, HDA_ICS/*offset*/, 0x1/*mask*/, 0/*value*/, 10000000/*delay in 100ns*/, &data64);
	ics = (UINT16)(data64 & 0xFFFF);
	//DBG("poll ICS[0] == 0: Status=%r, ICS=%x, ICS[0]=%d\n", Status, ics, ics & 0x0001);
	if (EFI_ERROR(Status)) return 0;
	// prepare and write verb to ICO
	data32 = codecAdr << 28 | ((nodeId & 0xFF)<<20) | (verb & 0xFFFFF);
	Status = PciIo->Mem.Write(PciIo, EfiPciIoWidthUint32, 0, HDA_ICO, 1, &data32);
	//DBG("ICO write verb Codec=%x, Node=%x, verb=%x, command verb=%x: Status=%r\n", codecAdr, nodeId, verb, data32, Status);
	if (EFI_ERROR(Status)) return 0;
	// write 11b to ICS[1:0] to send command
	ics |= 0x3;
	Status = PciIo->Mem.Write(PciIo, EfiPciIoWidthUint16, 0, HDA_ICS, 1, &ics);
	//DBG("ICS[1:0] = 11b: Status=%r\n", Status);
	if (EFI_ERROR(Status)) return 0;
	// poll ICS[1:0] to become 10b
	Status = PciIo->PollMem(PciIo, EfiPciIoWidthUint16, 0/*bar*/, HDA_ICS/*offset*/, 0x3/*mask*/, 0x2/*value*/, 10000000/*delay in 100ns*/, &data64);
	//DBG("poll ICS[0] == 0: Status=%r\n", Status);
	if (EFI_ERROR(Status)) return 0;
	// read IRI for VendorId/DeviceId
	Status = PciIo->Mem.Read(PciIo, EfiPciIoWidthUint32, 0, HDA_IRI, 1, &data32);
	if (EFI_ERROR(Status)) return 0;
	return data32;
}

UINT32 HDA_getCodecVendorAndDeviceIds(EFI_PCI_IO_PROTOCOL *PciIo) {
	EFI_STATUS	Status;
	//UINT8		ver[2];
	UINT32		data32 = 0;
	
	// check HDA version - should be 1.0
	/*
   Status = PciIo->Mem.Read(PciIo, EfiPciIoWidthUint8, 0, HDA_VMIN, 2, &ver[0]);
   DBG("HDA Version: Status=%r, version=%d.%d\n", Status, ver[1], ver[0]);
   if (EFI_ERROR(Status)) {
   return 0;
   }
	 */
	
	// check if controller is out of reset - GCTL-08h[CRST-bit 0] == 1
	Status = PciIo->Mem.Read(PciIo, EfiPciIoWidthUint32, 0, HDA_GCTL, 1, &data32);
	//DBG("check CRST == 1: Status=%r, CRST=%d\n", Status, data32 & 0x1);
	if (EFI_ERROR(Status)) {
		return 0;
	}
	if ((data32 & 0x1) == 0) {
		// this controller is not inited yet - we can not read codec ids
		// if needed, we can init it by:
		// - set Control reset bit in Global Control reg 08h[0] to 1
		// - poll it to become 1 again
		// - wait at least 521 micro sec for codecs to init
		// - we can check STATESTS reg 0eh where each running codec will set one bit
		//   codec addr 0 bit 0, codec addr 1 bit 1 ...
		
		return 0;
	}
  
	// all ok - read Ids
	return HDA_IC_sendVerb(PciIo, 0/*codecAdr*/, 0/*nodeId*/, 0xF0000/*verb*/);
}

UINT32 getLayoutIdFromVendorAndDeviceId(UINT32 vendorDeviceId) {
	UINT32	layoutId = 0;
	UINT8	hexDigit = 0;
	// extract device id - 2 lower bytes,
	// convert it to decimal like this: 0x0887 => 887 decimal
	hexDigit = vendorDeviceId & 0xF;
	if (hexDigit > 9) return 0;
	layoutId = hexDigit;
	
	vendorDeviceId = vendorDeviceId >> 4;
	hexDigit = vendorDeviceId & 0xF;
	if (hexDigit > 9) return 0;
	layoutId += hexDigit * 10;
	
	vendorDeviceId = vendorDeviceId >> 4;
	hexDigit = vendorDeviceId & 0xF;
	if (hexDigit > 9) return 0;
	layoutId += hexDigit * 100;
	
	vendorDeviceId = vendorDeviceId >> 4;
	hexDigit = vendorDeviceId & 0xF;
	if (hexDigit > 9) return 0;
	layoutId += hexDigit * 1000;
	
	return layoutId;
}



BOOLEAN set_hda_props(EFI_PCI_IO_PROTOCOL *PciIo, pci_dt_t *hda_dev)
{
	CHAR8           *devicepath;
	DevPropDevice   *device;
	UINT32           layoutId = 0, codecId = 0;
	
	if (!gSettings.HDAInjection) {
		return FALSE;
	}
	
	if (!string)
		string = devprop_create_string();
  
	devicepath = get_pci_dev_path(hda_dev);
	device = devprop_add_device(string, devicepath);
	if (!device)
		return FALSE;
  
	if (gSettings.HDALayoutId > 0) {
		// layoutId is specified - use it
		layoutId = (UINT32)gSettings.HDALayoutId;
		DBG("HDA Controller [%04x:%04x] :: %a => specified layout-id=0x%x=%d\n", hda_dev->vendor_id, hda_dev->device_id, devicepath, layoutId, layoutId);
	} else {
		// use detection: layoutId=codec dviceId or use default 12
		codecId = HDA_getCodecVendorAndDeviceIds(PciIo);
		layoutId = getLayoutIdFromVendorAndDeviceId(codecId);
		if (layoutId == 0) {
			layoutId = 12;
		}
		DBG("HDA Controller [%04x:%04x] :: %a, detected codec: %04x:%04x => setting layout-id=0x%x=%d\n",
        hda_dev->vendor_id, hda_dev->device_id, devicepath, codecId >> 16, codecId & 0xFFFF, layoutId, layoutId);
	}
	
	devprop_add_value(device, "layout-id", (UINT8*)&layoutId, 4);
	layoutId = 0; // reuse variable
	devprop_add_value(device, "PinConfigurations", (UINT8*)&layoutId, 1);
	return TRUE;
}
