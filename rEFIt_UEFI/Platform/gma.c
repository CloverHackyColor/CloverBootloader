/*
	Original patch by Nawcom
	http://forum.voodooprojects.org/index.php/topic,1029.0.html
*/

//#include "libsa.h"
//#include "saio_internal.h"
//#include "bootstruct.h"
//#include "pci.h"
//#include "platform.h"
#include "device_inject.h"
#include "gma.h"
#include "iBoot.h"
//#include "graphics.h"

#ifndef DEBUG_GMA
#define DEBUG_GMA 1
#endif

#if DEBUG_GMA == 2
#define DBG(x...)	AsciiPrint(x)
#elif DEBUG_GMA == 1
#define DBG(x...) MsgLog(x)
#else
#define DBG(x...)
#endif


//Slice - correct all values, still not sure
UINT8 GMAX3100_vals[23][4] = {
	{ 0x01,0x00,0x00,0x00 },	//0 "AAPL,HasPanel"
	{ 0x01,0x00,0x00,0x00 },	//1 "AAPL,SelfRefreshSupported"
	{ 0x01,0x00,0x00,0x00 },	//2 "AAPL,aux-power-connected"
	{ 0x01,0x00,0x00,0x08 },	//3 "AAPL,backlight-control"
	{ 0x00,0x00,0x00,0x00 },	//4 "AAPL00,blackscreen-preferences"
	{ 0x56,0x00,0x00,0x08 },	//5 "AAPL01,BacklightIntensity"
	{ 0x00,0x00,0x00,0x00 },	//6 "AAPL01,blackscreen-preferences"
	{ 0x01,0x00,0x00,0x00 },	//7 "AAPL01,DataJustify"
	{ 0x20,0x00,0x00,0x00 },	//8 "AAPL01,Depth"
	{ 0x01,0x00,0x00,0x00 },	//9 "AAPL01,Dither"
	{ 0x20,0x03,0x00,0x00 },	//10 "AAPL01,Height"
	{ 0x00,0x00,0x00,0x00 },	//11 "AAPL01,Interlace"
	{ 0x00,0x00,0x00,0x00 },	//12 "AAPL01,Inverter"
	{ 0x08,0x52,0x00,0x00 },	//13 "AAPL01,InverterCurrent"
	{ 0x00,0x00,0x00,0x00 },	//14 "AAPL01,LinkFormat"
	{ 0x00,0x00,0x00,0x00 },	//15 "AAPL01,LinkType"
	{ 0x01,0x00,0x00,0x00 },	//16 "AAPL01,Pipe"
	{ 0x01,0x00,0x00,0x00 },	//17 "AAPL01,PixelFormat"
	{ 0x01,0x00,0x00,0x00 },	//18 "AAPL01,Refresh"
	{ 0x3B,0x00,0x00,0x00 },	//19 "AAPL01,Stretch"
	{ 0xc8,0x95,0x00,0x00 },	//20 "AAPL01,InverterFrequency"
	{ 0x6B,0x10,0x00,0x00 },	//21 "subsystem-vendor-id"
	{ 0xA2,0x00,0x00,0x00 }		//22 "subsystem-id"
};

UINT8 reg_TRUE[]	= { 0x01, 0x00, 0x00, 0x00 };
UINT8 reg_FALSE[] = { 0x00, 0x00, 0x00, 0x00 };

static struct gma_gpu_t KnownGPUS[] = {
	{ 0x00000000, "Unknown"			},
	{ 0x808627A2, "GMA 950"	},
	{ 0x808627AE, "GMA 950"	},
//	{ 0x808627A6, "Mobile GMA950"	}, //not a GPU
	{ 0x8086A011, "Mobile GMA3150"	},
	{ 0x8086A012, "Mobile GMA3150"	},
	{ 0x80862772, "Desktop GMA950"	},
//	{ 0x80862776, "Desktop GMA950"	}, //not a GPU
//	{ 0x8086A001, "Desktop GMA3150" },
	{ 0x8086A001, "Mobile GMA3150"	},
	{ 0x8086A002, "Desktop GMA3150" },
	{ 0x80862A02, "GMAX3100"		},
	{ 0x80862A03, "GMAX3100"		},
	{ 0x80862A12, "GMAX3100"		},
	{ 0x80862A13, "GMAX3100"		},
	{ 0x80862A42, "GMAX3100"		},
	{ 0x80862A43, "GMAX3100"		},
};

CHAR8 *get_gma_model(UINT32 id) {
	INT32 i = 0;
	
	for (i = 0; i < (sizeof(KnownGPUS) / sizeof(KnownGPUS[0])); i++)
	{
		if (KnownGPUS[i].device == id)
			return KnownGPUS[i].name;
	}
	return KnownGPUS[0].name;
}

BOOLEAN setup_gma_devprop(pci_dt_t *gma_dev)
{
	CHAR8					*devicepath;
	UINT8         *regs;
	UINT32				bar[7];
	CHAR8					*model;
	UINT8 BuiltIn =		0x00;
	UINT8 ClassFix[4] =	{ 0x00, 0x00, 0x03, 0x00 };
	
	devicepath = get_pci_dev_path(gma_dev);
	
	bar[0] = pci_config_read32(gma_dev->dev.addr, 0x10);
	regs = (UINT8 *) (bar[0] & ~0x0f);
	
	model = get_gma_model((gma_dev->vendor_id << 16) | gma_dev->device_id);
	DBG(model);
	
	//DBG("Intel %s [%04x:%04x] :: %s\n",
			//model, gma_dev->vendor_id, gma_dev->device_id, devicepath);
	
	if (!string)
		string = devprop_create_string();
	
	struct DevPropDevice *device = AllocatePool(sizeof(struct DevPropDevice));
	device = devprop_add_device(string, devicepath);
	
	if (!device)
	{
		DBG("Failed initializing dev-prop string dev-entry.\n");
		//pause();
		return FALSE;
	}
	
	devprop_add_value(device, "model", (UINT8*)model, (AsciiStrLen(model) + 1));
	devprop_add_value(device, "device_type", (UINT8*)"display", 8);	
	
	if ((model == (CHAR8 *)"GMA 950")
		|| (model == (CHAR8 *)"Mobile GMA3150"))
	{
		devprop_add_value(device, "AAPL,HasPanel", reg_TRUE, 4);
		devprop_add_value(device, "built-in", &BuiltIn, 1);
		devprop_add_value(device, "class-code", ClassFix, 4);
	}
	else if ((model == (CHAR8 *)"Desktop GMA950")
			|| (model == (CHAR8 *)"Desktop GMA3150"))
	{
		BuiltIn = 0x01;
		devprop_add_value(device, "built-in", &BuiltIn, 1);
		devprop_add_value(device, "class-code", ClassFix, 4);
	}
	else if (model == (CHAR8 *)"GMAX3100")
	{
		//BuiltIn = gDualLink;
		devprop_add_value(device, "AAPL,HasPanel",GMAX3100_vals[0], 4);
		devprop_add_value(device, "AAPL,SelfRefreshSupported",GMAX3100_vals[1], 4);
		devprop_add_value(device, "AAPL,aux-power-connected",GMAX3100_vals[2], 4);
		devprop_add_value(device, "AAPL,backlight-control",GMAX3100_vals[3], 4);
		devprop_add_value(device, "AAPL00,blackscreen-preferences",GMAX3100_vals[4], 4);
		devprop_add_value(device, "AAPL01,BacklightIntensity",GMAX3100_vals[5], 4);
		devprop_add_value(device, "AAPL01,blackscreen-preferences",GMAX3100_vals[6], 4);
		devprop_add_value(device, "AAPL01,DataJustify",GMAX3100_vals[7], 4);
		devprop_add_value(device, "AAPL01,Depth",GMAX3100_vals[8], 4);
		devprop_add_value(device, "AAPL01,Dither",GMAX3100_vals[9], 4);
		devprop_add_value(device, "AAPL01,DualLink", &BuiltIn, 1);		//GMAX3100_vals[10]
		devprop_add_value(device, "AAPL01,Height",GMAX3100_vals[10], 4);
		devprop_add_value(device, "AAPL01,Interlace",GMAX3100_vals[11], 4);
		devprop_add_value(device, "AAPL01,Inverter",GMAX3100_vals[12], 4);
		devprop_add_value(device, "AAPL01,InverterCurrent",GMAX3100_vals[13], 4);
//		devprop_add_value(device, "AAPL01,InverterCurrency",GMAX3100_vals[15], 4);
		devprop_add_value(device, "AAPL01,LinkFormat",GMAX3100_vals[14], 4);
		devprop_add_value(device, "AAPL01,LinkType",GMAX3100_vals[15], 4);
		devprop_add_value(device, "AAPL01,Pipe",GMAX3100_vals[16], 4);
		devprop_add_value(device, "AAPL01,PixelFormat",GMAX3100_vals[17], 4);
		devprop_add_value(device, "AAPL01,Refresh",GMAX3100_vals[18], 4);
		devprop_add_value(device, "AAPL01,Stretch",GMAX3100_vals[19], 4);
		devprop_add_value(device, "AAPL01,InverterFrequency",GMAX3100_vals[20], 4);
		devprop_add_value(device, "class-code",						ClassFix, 4);
		devprop_add_value(device, "subsystem-vendor-id", GMAX3100_vals[21], 4);
		devprop_add_value(device, "subsystem-id", GMAX3100_vals[22], 4);
	}
	else if (model == (CHAR8*)"Unknown")
	{
		return FALSE;
	}
	
	/*stringdata = AllocatePool(sizeof(UINT8) * string->length);
	if (!stringdata)
	{
		DBG("No stringdata.\n");
		//pause();
		return FALSE;
	}*/
	
	extern CHAR8*						gDeviceProperties;
	gDeviceProperties = AllocatePool(string->length * 2);
	CopyMem(gDeviceProperties, (VOID*)devprop_generate_string(string), string->length * 2);
	DBG(gDeviceProperties);
#if DEBUG_GMA == 2  
	gBootServices->Stall(5000000);
#endif  

	
	return TRUE;
}
