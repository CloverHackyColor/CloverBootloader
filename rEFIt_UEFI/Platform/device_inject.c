/*
 *	Copyright 2009 Jasmin Fazlic All rights reserved.
 *
 *
 *	Cleaned and merged by iNDi
 */
// UEFI adaptation by usr-sse2, then slice, dmazar



#include "Platform.h"

#ifndef DEBUG_INJECT
#ifndef DEBUG_ALL
#define DEBUG_INJECT 1
#else
#define DEBUG_INJECT DEBUG_ALL
#endif
#endif

#if DEBUG_INJECT == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_INJECT, __VA_ARGS__)
#endif

UINT32 devices_number = 1;
UINT32 builtin_set    = 0;
DevPropString *string = NULL;
UINT8  *stringdata    = NULL;
UINT32 stringlength   = 0;

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
//	DBG("Begin creating strings for devices:\n");
	string = (DevPropString*)AllocateZeroPool(sizeof(DevPropString));
	
	if(string == NULL)
		return NULL;
	
	string->length = 12;
	string->WHAT2 = 0x01000000;
	return string;
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

//dmazar: replaced by devprop_add_device_pci

DevPropDevice *devprop_add_device_pci(DevPropString *StringBuf, pci_dt_t *PciDt)
{
	EFI_DEVICE_PATH_PROTOCOL		*DevicePath;
	DevPropDevice               *device;
	UINT32                        NumPaths;
  
	
	if (StringBuf == NULL || PciDt == NULL) {
		return NULL;
	}
	
	DevicePath = DevicePathFromHandle(PciDt->DeviceHandle);
// 	DBG("devprop_add_device_pci %s ", DevicePathToStr(DevicePath));
	if (!DevicePath)
		return NULL;
	
	device = AllocateZeroPool(sizeof(DevPropDevice));
	
	device->numentries = 0x00;
	
	//
	// check and copy ACPI_DEVICE_PATH
	//
	if (DevicePath->Type == ACPI_DEVICE_PATH && DevicePath->SubType == ACPI_DP) {
//		CopyMem(&device->acpi_dev_path, DevicePath, sizeof(struct ACPIDevPath));
		device->acpi_dev_path.length = 0x0c;
		device->acpi_dev_path.type = 0x02;
		device->acpi_dev_path.subtype = 0x01;
		device->acpi_dev_path._HID = 0x0a0341d0;
		device->acpi_dev_path._UID = gSettings.PCIRootUID; 
				
//		DBG("ACPI HID=%x, UID=%x ", device->acpi_dev_path._HID, device->acpi_dev_path._UID);
	} else {
//		DBG("not ACPI\n");
		return NULL;
	}
	
	//
	// copy PCI paths
	//
	for (NumPaths = 0; NumPaths < MAX_PCI_DEV_PATHS; NumPaths++) {
		DevicePath = NextDevicePathNode(DevicePath);
		if (DevicePath->Type == HARDWARE_DEVICE_PATH && DevicePath->SubType == HW_PCI_DP) {
			CopyMem(&device->pci_dev_path[NumPaths], DevicePath, sizeof(struct PCIDevPath));
//			DBG("PCI[%d] f=%x, d=%x ", NumPaths, device->pci_dev_path[NumPaths].function, device->pci_dev_path[NumPaths].device);
		} else {
			// not PCI path - break the loop
//			DBG("not PCI ");
			break;
		}
	}
	
	if (NumPaths == 0) {
		DBG("NumPaths == 0 \n");
		return NULL;
	}
	
//	DBG("-> NumPaths=%d\n", NumPaths);
	device->num_pci_devpaths = (UINT8)NumPaths;
	device->length = (UINT32)(24U + (6U * NumPaths));
	
	device->path_end.length = 0x04;
	device->path_end.type = 0x7f;
	device->path_end.subtype = 0xff;
	
	device->string = StringBuf;
	device->data = NULL;
	StringBuf->length += device->length;
	
	if(!StringBuf->entries) {
    StringBuf->entries = (DevPropDevice**)AllocateZeroPool(MAX_NUM_DEVICES * sizeof(device));
		if(!StringBuf->entries)
			return 0;
  }
	
	StringBuf->entries[StringBuf->numentries++] = device;
	
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
	length = (UINT32)((l * 2) + len + (2 * sizeof(UINT32)) + 2);
	data = (UINT8*)AllocateZeroPool(length);
  if(!data)
    return FALSE;
  
  off= 0;
  
  data[off+1] = (UINT8)(((l * 2) + 6) >> 8);
  data[off] =   ((l * 2) + 6) & 0x00FF;
  
  off += 4;
  
  for(i = 0 ; i < l ; i++, off += 2)
  {
    data[off] = *nm++;
  }
  
  off += 2;
  l = len;
  datalength = (UINT32*)&data[off];
  *datalength = (UINT32)(l + 4);
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

CHAR8 *devprop_generate_string(DevPropString *StringBuf)
{
  UINTN len = StringBuf->length * 2;
	INT32 i = 0;
  UINT32 x = 0;
	CHAR8 *buffer = (CHAR8*)AllocatePool(len + 1);
	CHAR8 *ptr = buffer;

  //   DBG("devprop_generate_string\n");
	if(!buffer)
		return NULL;

	AsciiSPrint(buffer, len, "%08x%08x%04x%04x", dp_swap32(StringBuf->length), StringBuf->WHAT2,
              dp_swap16(StringBuf->numentries), StringBuf->WHAT3);
	buffer += 24;

	while(i < StringBuf->numentries)
	{
    UINT8 *dataptr = StringBuf->entries[i]->data;
		AsciiSPrint(buffer, len, "%08x%04x%04x", dp_swap32(StringBuf->entries[i]->length),
                dp_swap16(StringBuf->entries[i]->numentries), StringBuf->entries[i]->WHAT2); //FIXME: wrong buffer sizes!

		buffer += 16;
		AsciiSPrint(buffer, len, "%02x%02x%04x%08x%08x", StringBuf->entries[i]->acpi_dev_path.type,
                StringBuf->entries[i]->acpi_dev_path.subtype,
                dp_swap16(StringBuf->entries[i]->acpi_dev_path.length),
                dp_swap32(StringBuf->entries[i]->acpi_dev_path._HID),
                dp_swap32(StringBuf->entries[i]->acpi_dev_path._UID));

		buffer += 24;
		for(x = 0; x < StringBuf->entries[i]->num_pci_devpaths; x++)
		{
			AsciiSPrint(buffer, len, "%02x%02x%04x%02x%02x", StringBuf->entries[i]->pci_dev_path[x].type,
                  StringBuf->entries[i]->pci_dev_path[x].subtype,
                  dp_swap16(StringBuf->entries[i]->pci_dev_path[x].length),
                  StringBuf->entries[i]->pci_dev_path[x].function,
                  StringBuf->entries[i]->pci_dev_path[x].device);
			buffer += 12;
		}

		AsciiSPrint(buffer, len, "%02x%02x%04x", StringBuf->entries[i]->path_end.type,
                StringBuf->entries[i]->path_end.subtype,
                dp_swap16(StringBuf->entries[i]->path_end.length));

		buffer += 8;
		for(x = 0; x < (StringBuf->entries[i]->length) - (24 + (6 * StringBuf->entries[i]->num_pci_devpaths)); x++)
		{
			AsciiSPrint(buffer, len, "%02x", *dataptr++);
			buffer += 2;
		}
		i++;
	}
	return ptr;
}

VOID devprop_free_string(DevPropString *StringBuf)
{
   INT32 i;
	//DBG("devprop_free_string\n");
	if(!StringBuf)
		return;
	
	for(i = 0; i < StringBuf->numentries; i++)
	{
		if(StringBuf->entries[i])
		{
			if(StringBuf->entries[i]->data)
			{
				FreePool(StringBuf->entries[i]->data);
		//		StringBuf->entries[i]->data = NULL;
			}
		//	FreePool(StringBuf->entries[i]);
		//	StringBuf->entries[i] = NULL;
		}
	}
	FreePool(StringBuf->entries);
	FreePool(StringBuf);
	StringBuf = NULL;
}

// Ethernet built-in device injection
BOOLEAN set_eth_props(pci_dt_t *eth_dev)
{
	CHAR8           *devicepath;
  DevPropDevice   *device;
  UINT8           builtin = 0x0;
  BOOLEAN         Injected;
  INT32           i;
	
	if (!string)
    string = devprop_create_string();
    
  devicepath = get_pci_dev_path(eth_dev);
  //device = devprop_add_device(string, devicepath);
  device = devprop_add_device_pci(string, eth_dev);
  if (!device)
    return FALSE;
	// -------------------------------------------------
	DBG("LAN Controller [%04x:%04x] :: %a\n", eth_dev->vendor_id, eth_dev->device_id, devicepath);
	if (eth_dev->vendor_id != 0x168c && builtin_set == 0) 
  {
 		builtin_set = 1;
 		builtin = 0x01;
 	}

  for (i = 0; i < gSettings.NrAddProperties; i++) {
    if (gSettings.AddProperties[i].Device != DEV_LAN) {
      continue;
    }
    Injected = TRUE;
    devprop_add_value(device,
                      gSettings.AddProperties[i].Key,
                      (UINT8*)gSettings.AddProperties[i].Value,
                      gSettings.AddProperties[i].ValueLen);
  }
  if (Injected) {
    DBG("custom LAN properties injected, continue\n");
    //    return TRUE;
  }

//  DBG("Setting dev.prop built-in=0x%x\n", builtin);
  devprop_add_value(device, "device_type", (UINT8*)"ethernet", 8);
  if (gSettings.FakeLAN) {
    UINT32 FakeID = gSettings.FakeLAN >> 16;
    devprop_add_value(device, "device-id", (UINT8*)&FakeID, 4);
    FakeID = gSettings.FakeLAN & 0xFFFF;
    devprop_add_value(device, "vendor-id", (UINT8*)&FakeID, 4);
  }
  
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
  BOOLEAN         Injected;
  INT32           i;

	if (!string)
    string = devprop_create_string();
  
  devicepath = get_pci_dev_path(usb_dev);
  //device = devprop_add_device(string, devicepath);
  device = devprop_add_device_pci(string, usb_dev);
  if (!device)
    return FALSE;
	// -------------------------------------------------
	DBG("USB Controller [%04x:%04x] :: %a\n", usb_dev->vendor_id, usb_dev->device_id, devicepath);
  //  DBG("Setting dev.prop built-in=0x%x\n", builtin);

  for (i = 0; i < gSettings.NrAddProperties; i++) {
    if (gSettings.AddProperties[i].Device != DEV_USB) {
      continue;
    }
    Injected = TRUE;
    devprop_add_value(device,
                      gSettings.AddProperties[i].Key,
                      (UINT8*)gSettings.AddProperties[i].Value,
                      gSettings.AddProperties[i].ValueLen);
  }
  if (Injected) {
    DBG("custom USB properties injected, continue\n");
    //    return TRUE;
  }

  if (gSettings.InjectClockID) {
    devprop_add_value(device, "AAPL,clock-id", (UINT8*)&clock_id, 1);
    clock_id++;
  }
  
  fake_devid = usb_dev->device_id & 0xFFFF;
  if ((fake_devid & 0xFF00) == 0x2900) {
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
      devprop_add_value(device, "AAPL,current-available", (UINT8*)&current_available, 2);
      devprop_add_value(device, "AAPL,current-extra",     (UINT8*)&current_extra, 2);
      devprop_add_value(device, "AAPL,current-in-sleep",  (UINT8*)&current_in_sleep, 2);      
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
	
  // about that polling below ...
  // spec says that delay is in 100ns units. value 1.000.000.0
  // should then be 1 second, but users of newer Aptio boards were reporting
  // delays of 10-20 secs when this value was used. maybe this polling timeout
  // value does not mean the same on all implementations?
  // anyway, delay is lowered now to 10.000.0 (10 millis).
  
	// poll ICS[0] to become 0
	Status = PciIo->PollMem(PciIo, EfiPciIoWidthUint16, 0/*bar*/, HDA_ICS/*offset*/, 0x1/*mask*/, 0/*value*/, 100000/*delay in 100ns*/, &data64);
	ics = (UINT16)(data64 & 0xFFFF);
	//DBG("poll ICS[0] == 0: Status=%r, ICS=%x, ICS[0]=%d\n", Status, ics, (ics & 0x0001));
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
	Status = PciIo->PollMem(PciIo, EfiPciIoWidthUint16, 0/*bar*/, HDA_ICS/*offset*/, 0x3/*mask*/, 0x2/*value*/, 100000/*delay in 100ns*/, &data64);
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
	//DBG("check CRST == 1: Status=%r, CRST=%d\n", Status, (data32 & 0x1));
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
  //Slice - TODO check codecAdr=2 - it is my Dell 1525.
	// all ok - read Ids
	return HDA_IC_sendVerb(PciIo, 0/*codecAdr*/, 0/*nodeId*/, 0xF0000/*verb*/);
}

UINT32 getLayoutIdFromVendorAndDeviceId(UINT32 vendorDeviceId) 
{
	UINT32	layoutId = 0;
	UINT8	  hexDigit = 0;
  
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

BOOLEAN IsHDMIAudio(EFI_HANDLE PciDevHandle)
{
  EFI_STATUS          Status;
  EFI_PCI_IO_PROTOCOL *PciIo;
  UINTN               Segment;
	UINTN               Bus;
	UINTN               Device;
	UINTN               Function;
	UINTN               Index;

  // get device PciIo protocol
  Status = gBS->OpenProtocol(PciDevHandle, &gEfiPciIoProtocolGuid, (VOID **)&PciIo, gImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
  if (EFI_ERROR(Status)) {
    return FALSE;
  }
  
  // get device location
  Status = PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
  if (EFI_ERROR(Status)) {
    return FALSE;
  }
  
  // iterate over all GFX devices and check for sibling
  for (Index = 0; Index < NGFX; Index++) {
    if (gGraphics[Index].Segment == Segment
        && gGraphics[Index].Bus == Bus
        && gGraphics[Index].Device == Device
        )
    {
      return TRUE;
    }
  }
  
  return FALSE;
}

BOOLEAN set_hda_props(EFI_PCI_IO_PROTOCOL *PciIo, pci_dt_t *hda_dev)
{
	CHAR8           *devicepath;
	DevPropDevice   *device;
	UINT32           layoutId = 0, codecId = 0;
  BOOLEAN         Injected = FALSE;
  INT32           i;

	if (!gSettings.HDAInjection) {
		return FALSE;
	}
	
  if (!string)
		string = devprop_create_string();
    
  devicepath = get_pci_dev_path(hda_dev);
  //device = devprop_add_device(string, devicepath);
  device = devprop_add_device_pci(string, hda_dev);
  if (!device)
    return FALSE;
  
  DBG("HDA Controller [%04x:%04x] :: %a =>", hda_dev->vendor_id, hda_dev->device_id, devicepath);
  
  if (IsHDMIAudio(hda_dev->DeviceHandle)) {
    for (i = 0; i < gSettings.NrAddProperties; i++) {
      if (gSettings.AddProperties[i].Device != DEV_HDMI) {
        continue;
      }
      Injected = TRUE;
      devprop_add_value(device,
                        gSettings.AddProperties[i].Key,
                        (UINT8*)gSettings.AddProperties[i].Value,
                        gSettings.AddProperties[i].ValueLen);
    }
    if (Injected) {
      DBG("custom USB properties injected, continue\n");
      //    return TRUE;
    } else {
      DBG(" HDMI Audio, setting hda-gfx=onboard-1\n");
      devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
    }    
  } else {
    
    // HDA - determine layout-id
    if (gSettings.HDALayoutId > 0) {
      // layoutId is specified - use it
      layoutId = (UINT32)gSettings.HDALayoutId;
      DBG(" setting specified layout-id=%d (0x%x)\n", layoutId, layoutId);
    } else {
      // use detection: layoutId=codec dviceId or use default 12
      codecId = HDA_getCodecVendorAndDeviceIds(PciIo);
      if (codecId != 0) {
        DBG(" detected codec: %04x:%04x", (codecId >> 16), (codecId & 0xFFFF));
        layoutId = getLayoutIdFromVendorAndDeviceId(codecId);
      } else {
        DBG(" codec not detected");
      }
      // if not detected - use 12 as default
      if (layoutId == 0) {
        layoutId = 12;
      }
      DBG(", setting layout-id=%d (0x%x)\n", layoutId, layoutId);
    }
    for (i = 0; i < gSettings.NrAddProperties; i++) {
      if (gSettings.AddProperties[i].Device != DEV_HDA) {
        continue;
      }
      Injected = TRUE;
      devprop_add_value(device,
                        gSettings.AddProperties[i].Key,
                        (UINT8*)gSettings.AddProperties[i].Value,
                        gSettings.AddProperties[i].ValueLen);
    }
    if (!Injected) {

    // inject layout and pin config
      devprop_add_value(device, "layout-id", (UINT8*)&layoutId, 4);
      layoutId = 0; // reuse variable
      devprop_add_value(device, "PinConfigurations", (UINT8*)&layoutId, 1);
    }
    
  }
  
	return TRUE;
}
