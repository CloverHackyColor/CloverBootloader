/*
 *	Copyright 2009 Jasmin Fazlic All rights reserved.
 */
/*
 *	Cleaned and merged by iNDi
 */


#include "device_inject.h"
#include "iBoot.h"
#ifndef DEBUG_INJECT
#define DEBUG_INJECT 0
#endif

#if DEBUG_INJECT
#define DBG(x...)	AsciiPrint(x)
#else
#define DBG(x...)
#endif

UINT32 devices_number = 1;
UINT32 builtin_set = 0;
struct DevPropString *string = 0;
UINT8 *stringdata = 0;
UINT32 stringlength = 0;

#if 0 //never do this
CHAR8 *efi_inject_get_devprop_string(UINT32 *len)
{
	if(string) {
		*len = string->length;
		return devprop_generate_string(string);
	}
//	verbose("efi_inject_get_devprop_string NULL trying stringdata\n");
	return NULL;
}

VOID setupDeviceProperties(Node *node)
{
  const CHAR8 *val;
  UINT8 *binStr;
  INT32 cnt, cnt2;

  static CHAR8 DEVICE_PROPERTIES_PROP[] = "device-properties";

  /* Generate devprop string.
   */
//  UINT32 AsciiStrLength;
  CHAR8 *string = efi_inject_get_devprop_string(&stringlength);

  /* Use the static "device-properties" boot config key contents if available,
   * otheriwse use the generated one.
   */  
  if (!getValueForKey(kDeviceProperties, &val, &cnt, &bootInfo->chameleonConfig) && string)
  {
    val = (const CHAR8*)string;
    cnt = stringlength * 2;
  } 
    
  if (cnt > 1)
  {
    binStr = convertHexStr2Binary(val, &cnt2);
    if (cnt2 > 0) DT__AddProperty(node, DEVICE_PROPERTIES_PROP, cnt2, binStr);
  }
}
#endif

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
	string = (struct DevPropString*)AllocatePool(sizeof(struct DevPropString));
	
	if(string == NULL)
		return NULL;
	
	ZeroMem((VOID*)string, sizeof(struct DevPropString));
	string->length = 12;
	string->WHAT2 = 0x01000000;
	return string;
}

UINT8 ascii_hex_to_int (CHAR8* buf) {
	INT8 i;
	if (buf[0]>='0' && buf[0]<='9')
		i = buf[0]-'0';
	else
		i = buf[0]-'A'; //no error checking
	i *= 16;
	if (buf[1]>='0' && buf[1]<='9')
		i += buf[1]-'0';
	else
		i += buf[1]-'A'; //no error checking
	return i;
}
 
struct DevPropDevice *devprop_add_device(struct DevPropString *string, CHAR8 *path)
{
	struct DevPropDevice	*device;
	const CHAR8		pciroot_string[] = "PciRoot(0x";
	const CHAR8		pci_device_string[] = "Pci(0x";

	if (string == NULL || path == NULL) {
		return NULL;
	}
	device = AllocatePool(sizeof(struct DevPropDevice));

	if (AsciiStrnCmp(path, pciroot_string, AsciiStrLen(pciroot_string))) {
		AsciiPrint("ERROR parsing device path\n");
		return NULL;
	}

	ZeroMem((VOID*)device, sizeof(struct DevPropDevice));
	device->acpi_dev_path._UID = /*getPciRootUID();*/0; //FIXME: what if 1?

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
				AsciiPrint("ERROR parsing device path\n");
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
				AsciiPrint("ERROR parsing device path\n");
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

VOID devprop_FreePool_string(struct DevPropString *string)
{
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