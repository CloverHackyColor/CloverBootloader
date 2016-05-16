/*
	Original patch by Nawcom
	http://forum.voodooprojects.org/index.php/topic,1029.0.html
*/

#include "Platform.h"
#include "gma.h"

#ifndef DEBUG_GMA
#ifndef DEBUG_ALL
#define DEBUG_GMA 1
#else
#define DEBUG_GMA DEBUG_ALL
#endif
#endif

#if DEBUG_GMA == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_GMA, __VA_ARGS__)
#endif

extern CHAR8*						gDeviceProperties;

//Slice - corrected all values, still not sure
UINT8 GMAX3100_vals[28][4] = {
	{ 0x01, 0x00, 0x00, 0x00 },	//0 "AAPL,HasPanel"
	{ 0x01, 0x00, 0x00, 0x00 },	//1 "AAPL,SelfRefreshSupported"
	{ 0x01, 0x00, 0x00, 0x00 },	//2 "AAPL,aux-power-connected"
	{ 0x01, 0x00, 0x00, 0x08 },	//3 "AAPL,backlight-control"
	{ 0x00, 0x00, 0x00, 0x00 },	//4 "AAPL00,blackscreen-preferences"
	{ 0x56, 0x00, 0x00, 0x08 },	//5 "AAPL01,BacklightIntensity"
	{ 0x00, 0x00, 0x00, 0x00 },	//6 "AAPL01,blackscreen-preferences"
	{ 0x01, 0x00, 0x00, 0x00 },	//7 "AAPL01,DataJustify"
	{ 0x20, 0x00, 0x00, 0x00 },	//8 "AAPL01,Depth"
	{ 0x01, 0x00, 0x00, 0x00 },	//9 "AAPL01,Dither"
	{ 0x20, 0x03, 0x00, 0x00 },	//10 "AAPL01,Height"
	{ 0x00, 0x00, 0x00, 0x00 },	//11 "AAPL01,Interlace"
	{ 0x00, 0x00, 0x00, 0x00 },	//12 "AAPL01,Inverter"
	{ 0x08, 0x52, 0x00, 0x00 },	//13 "AAPL01,InverterCurrent"
	{ 0x00, 0x00, 0x00, 0x00 },	//14 "AAPL01,LinkFormat"
	{ 0x00, 0x00, 0x00, 0x00 },	//15 "AAPL01,LinkType"
	{ 0x01, 0x00, 0x00, 0x00 },	//16 "AAPL01,Pipe"
	{ 0x01, 0x00, 0x00, 0x00 },	//17 "AAPL01,PixelFormat"
	{ 0x01, 0x00, 0x00, 0x00 },	//18 "AAPL01,Refresh"
	{ 0x3B, 0x00, 0x00, 0x00 },	//19 "AAPL01,Stretch"
	{ 0xc8, 0x95, 0x00, 0x00 },	//20 "AAPL01,InverterFrequency"
	{ 0x6B, 0x10, 0x00, 0x00 },	//21 "subsystem-vendor-id"
	{ 0xA2, 0x00, 0x00, 0x00 },	//22 "subsystem-id"
  { 0x05, 0x00, 0x62, 0x01 },    //23 "AAPL,ig-platform-id" HD4000 //STLVNUB
  { 0x06, 0x00, 0x62, 0x01 },    //24 "AAPL,ig-platform-id" HD4000 iMac
  { 0x09, 0x00, 0x66, 0x01 },    //25 "AAPL,ig-platform-id" HD4000
  { 0x03, 0x00, 0x22, 0x0d },    //26 "AAPL,ig-platform-id" HD4600 //bcc9 http://www.insanelymac.com/forum/topic/290783-intel-hd-graphics-4600-haswell-working-displayport/
  { 0x00, 0x00, 0x62, 0x01 }   //27 - automatic solution
};
// 5 - 32Mb 6 - 48Mb 9 - 64Mb, 0 - 96Mb

UINT8 FakeID_126[] = {0x26, 0x01, 0x00, 0x00 };
extern CHAR8 ClassFix[];

UINT8 reg_TRUE[]	= { 0x01, 0x00, 0x00, 0x00 };
UINT8 reg_FALSE[] = { 0x00, 0x00, 0x00, 0x00 };
UINT8 OsInfo[] = {0x30, 0x49, 0x01, 0x11, 0x01, 0x10, 0x08, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF};

static struct gma_gpu_t KnownGPUS[] = {
	{ 0x0000, "Intel Unknown"			},
	{ 0x2582, "GMA 915"	},
	{ 0x2592, "GMA 915"	},
	{ 0x27A2, "GMA 950"	},
	{ 0x27AE, "GMA 950"	},
//	{ 0x27A6, "Mobile GMA950"	}, //not a GPU
	{ 0xA011, "Mobile GMA3150"	},
	{ 0xA012, "Mobile GMA3150"	},
	{ 0x2772, "Desktop GMA950"	},
  { 0x29C2, "Desktop GMA3100"	},
//	{ 0x2776, "Desktop GMA950"	}, //not a GPU
//	{ 0xA001, "Desktop GMA3150" },
	{ 0xA001, "Mobile GMA3150"	},
	{ 0xA002, "Desktop GMA3150" },
	{ 0x2A02, "GMAX3100"		},
//	{ 0x2A03, "GMAX3100"		},//not a GPU
	{ 0x2A12, "GMAX3100"		},
//	{ 0x2A13, "GMAX3100"		},
	{ 0x2A42, "GMAX3100"		},
//	{ 0x2A43, "GMAX3100"		},
  { 0x0042, "Intel HD Graphics"  },
  { 0x0046, "Intel HD Graphics"  },
  { 0x0102, "Intel HD Graphics 2000"  },
  { 0x0106, "Intel HD Graphics 3000"  },
  { 0x010A, "Intel HD Graphics P3000" },  //Xeon E3 1200 v1, needs FakeID
  { 0x0112, "Intel HD Graphics 2000"  },
  { 0x0116, "Intel HD Graphics 3000"  },
  { 0x0122, "Intel HD Graphics 3000"  },
  { 0x0126, "Intel HD Graphics 3000"  },
  { 0x0162, "Intel HD Graphics 4000"  },  //Desktop
  { 0x0166, "Intel HD Graphics 4000"  }, // MacBookPro10,1 have this string as model name whatever chameleon team may say
  { 0x0152, "Intel HD Graphics 2500"  },  //iMac
  { 0x0156, "Intel HD Graphics 4000"  },  //MacBook
  { 0x016A, "Intel HD Graphics P4000" },  //Xeon E3-1245
  { 0x0412, "Intel HD Graphics 4600"  },  //Haswell
  { 0x0416, "Intel HD Graphics 4600"  },  //Haswell
  { 0x041E, "Intel HD Graphics 4400"  },  //Haswell
  { 0x0A0E, "Intel HD Graphics 4400"  },  //Haswell
  { 0x0A16, "Intel HD Graphics 4400"  },  //Haswell
  { 0x0A1E, "Intel HD Graphics 4200"  },  // Haswell *
  { 0x0A26, "Intel HD Graphics 5000"  },  //Haswell *
  { 0x0A2E, "Intel Iris"  },  // Haswell Intel Iris 5100 (i7-4558U, i7-4578U) *
  { 0x0D22, "Intel Iris Pro"  },  //Haswell *
  { 0x0D26, "Intel Iris Pro"  },  //Haswell i7 4860HQ *
                                  // Broadwell
    { 0x1602, "Intel HD Graphics"  }, // *
    { 0x1606, "Intel HD Graphics"  }, // *
    { 0x160A, "Intel HD Graphics"  }, // Broadwell-U Integrated
    //  { 0x160B, "Intel HD Graphics"  }, // Broadwell-U Integrated
    //  { 0x160D, "Intel HD Graphics"  }, // Broadwell-U Integrated
    { 0x1612, "Intel HD Graphics 5600"  }, // * Broadwell i7-5700HQ
    { 0x1616, "Intel HD Graphics 5500"  }, // *
    { 0x161A, "Intel HD Graphics"  }, // Broadwell-U Integrated
    //  { 0x161B, "Intel HD Graphics"  }, // Broadwell-U Integrated
    //  { 0x161D, "Intel HD Graphics"  }, // Broadwell-U Integrated
    { 0x161E, "Intel HD Graphics 5300"  }, // *
    { 0x1622, "Intel Iris Pro"  }, // *
    { 0x1626, "Intel HD Graphics 6000"  }, // *
    { 0x162A, "Intel Iris Pro Graphics P6300"  }, // Intel(R) Iris(TM) Pro Graphics
    //{ 0x162B, "Intel Iris"  }, // *
    //{ 0x162D, "Intel Iris Pro Graphics P6300"  }, // Intel(R) Iris(TM) Pro Graphics 6300P Drivers
    { 0x162E, "Intel HD Graphics"  }, // Broadwell-U Integrated Graphics
    { 0x1632, "Intel HD Graphics"  }, // Broadwell-U Integrated Graphics
    { 0x1636, "Intel HD Graphics"  }, // Broadwell-U Integrated Graphics
    { 0x163A, "Intel HD Graphics"  }, // Broadwell-U Integrated Graphics
//  { 0x163B, "Intel HD Graphics"  }, // Broadwell-U Integrated Graphics
//  { 0x163D, "Intel HD Graphics"  }, // Broadwell-U Integrated Graphics
    { 0x163E, "Intel HD Graphics"  }, // Broadwell-U Integrated
// Skylake
/*
    { 0x1902, "Intel HD Graphics 510"  }, // Intel(R) HD Graphics 510
    { 0x1906, "Intel HD Graphics 510"  }, // Intel(R) HD Graphics 510
    { 0x1912, "Intel HD Graphics 530"  }, // *
//  { 0x1913, "Intel HD Graphics 510"  }, // Intel(R) HD Graphics 510
    { 0x1916, "Intel HD Graphics 520"  }, // * or Intel® Iris™ Graphics 540
//  { 0x1917, "Intel HD Graphics 530"  }, // Intel(R) HD Graphics 530
    { 0x191A, "Intel HD Graphics 530"  }, // Intel(R) HD Graphics 530
//  { 0x191D, "Intel HD Graphics P530"  }, // Intel(R) HD Graphics P530
    { 0x191E, "Intel HD Graphics 515"  }, // Intel(R) HD Graphics 515
//  { 0x1921, "Intel HD Graphics 520"  }, // Intel(R) HD Graphics 520
    { 0x1922, "Intel Iris Graphics 535"  }, // Intel(R) HD Graphics 535
    { 0x1926, "Intel Iris Graphics 540"  }, // Intel(R) Iris(TM) Graphics 540
//  { 0x1927, "Intel Iris Graphics 550"  }, // Intel(R) Iris(TM) Graphics 550
    { 0x192A, "Intel Iris Pro Graphics P580"  }, // Intel(R) Iris(TM) Pro Graphics P580
*/

// https://github.com/anholt/mesa/blob/master/include/pci_ids/i965_pci_ids.h
//SKL1
    { 0x1902, "Intel(R) HD Graphics 510 (Skylake GT1)" },
    { 0x1906, "Intel(R) HD Graphics 510 (Skylake GT1)" },
    { 0x190A, "Intel(R) Skylake GT1" },
    { 0x190E, "Intel(R) Skylake GT1" },
//SKL2
    { 0x1912, "Intel(R) HD Graphics 530 (Skylake GT2)" },
    { 0x1913, "Intel(R) Skylake GT2f" },
    { 0x1915, "Intel(R) Skylake GT2f" },
    { 0x1916, "Intel(R) HD Graphics 520 (Skylake GT2)" },
    { 0x1917, "Intel(R) Skylake GT2f" },
    { 0x191A, "Intel(R) Skylake GT2" },
    { 0x191B, "Intel(R) HD Graphics 530 (Skylake GT2)" },
    { 0x191D, "Intel(R) HD Graphics P530 (Skylake GT2)" },
    { 0x191E, "Intel(R) HD Graphics 515 (Skylake GT2)" },
    { 0x1921, "Intel(R) Skylake GT2" },
//SKL3
    { 0x1923, "Intel(R) Iris Graphics 540 (Skylake GT3e)" },
    { 0x1926, "Intel(R) HD Graphics 535 (Skylake GT3)" },
    { 0x1927, "Intel(R) Iris Graphics 550 (Skylake GT3e)" },
    { 0x192A, "Intel(R) Skylake GT4" },
    { 0x192B, "Intel(R) Iris Graphics (Skylake GT3fe)" },
//SKL4
    //{ 0x1932, "Intel(R) Skylake GT4" },
    //{ 0x193A, "Intel(R) Skylake GT4" },
    { 0x193B, "Intel(R) Skylake GT4" },
    { 0x193D, "Intel(R) Skylake GT4" },

//  { 0x192B, "Intel Iris"  }, // Intel(R) Iris(TM) Graphics
    { 0x1932, "Intel Iris Pro Graphics 570/580"  }, // Intel(R) Iris(TM) Pro Graphics 570/580
    { 0x193A, "Intel Iris Pro Graphics P580"  }, // Intel(R) Iris(TM) Pro Graphics P580
//  { 0x193B, "Intel Iris Pro Graphics 580"  }, // Intel(R) Iris(TM) Pro Graphics 580
//  { 0x193D, "Intel Iris Pro Graphics P580"  }, // Intel(R) Iris(TM) Pro Graphics P580
  // 0x0e08 - Xeon E5-1620
};

CHAR8 *get_gma_model(UINT16 id)
{
	INT32 i;
	
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
  DevPropDevice *device;
//	UINT8         *regs;
  UINT32        DualLink = 0; //local variable must be inited
//	UINT32				bar[7];
	CHAR8					*model;
	UINT8         BuiltIn =		0x00;
  UINTN j;
  INT32 i;
  BOOLEAN Injected = FALSE;
  BOOLEAN SetSnb = FALSE;
//  UINT32 SnbId = 0;
//  MACHINE_TYPES MacModel;
//  UINT8 IG_ID[4] = { 0x00, 0x00, 0x62, 0x01 };
  
	devicepath = get_pci_dev_path(gma_dev);

	model = get_gma_model(gma_dev->device_id);
  for (j = 0; j < NGFX; j++) {    
    if ((gGraphics[j].Vendor == Intel) &&
        (gGraphics[j].DeviceID == gma_dev->device_id)) {
      model = gGraphics[j].Model; 
      break;
    }
  }
//	DBG("Finally model=%a\n", model);
	
	DBG("Intel %a [%04x:%04x] :: %a\n",
			model, gma_dev->vendor_id, gma_dev->device_id, devicepath);
	
	if (!string) {
		string = devprop_create_string();
  }
	
	//device = devprop_add_device(string, devicepath); //AllocatePool inside
	device = devprop_add_device_pci(string, gma_dev);
	
	if (!device) {
		DBG("Failed initializing dev-prop string dev-entry.\n");
		//pause();
		return FALSE;
	}

  if (gSettings.NrAddProperties != 0xFFFE) {
    for (i = 0; i < gSettings.NrAddProperties; i++) {
      if (gSettings.AddProperties[i].Device != DEV_INTEL) {
        continue;
      }
      Injected = TRUE;
      devprop_add_value(device,
                        gSettings.AddProperties[i].Key,
                        (UINT8*)gSettings.AddProperties[i].Value,
                        gSettings.AddProperties[i].ValueLen);
    }
  }

  if (Injected) {
    DBG("custom IntelGFX properties injected, continue\n");
  }

  if (gSettings.FakeIntel) {
    UINT32 FakeID = gSettings.FakeIntel >> 16;
    devprop_add_value(device, "device-id", (UINT8*)&FakeID, 4);
    FakeID = gSettings.FakeIntel & 0xFFFF;
    devprop_add_value(device, "vendor-id", (UINT8*)&FakeID, 4);
  }
    
  if (gSettings.UseIntelHDMI) {
    devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
  }

  if (gSettings.InjectEDID && gSettings.CustomEDID) {
    devprop_add_value(device, "AAPL00,override-no-connect", gSettings.CustomEDID, 128);
  }
  
  if (gSettings.IgPlatform != 0) {
    if (gma_dev->device_id < 0x130) {
      devprop_add_value(device, "AAPL,snb-platform-id",	(UINT8*)&gSettings.IgPlatform, 4);
    } else {
      devprop_add_value(device, "AAPL,ig-platform-id", (UINT8*)&gSettings.IgPlatform, 4);
    }
    SetSnb = TRUE;
  }
  
  if (gSettings.DualLink != 0) {
    devprop_add_value(device, "AAPL00,DualLink", (UINT8*)&gSettings.DualLink, 1);
  }

  if (gSettings.NoDefaultProperties) {
    DBG("Intel: no default properties\n");
    return TRUE;
  }

  devprop_add_value(device, "model", (UINT8*)model, (UINT32)AsciiStrLen(model));
	devprop_add_value(device, "device_type", (UINT8*)"display", 7);	
  devprop_add_value(device, "subsystem-vendor-id", GMAX3100_vals[21], 4);
  devprop_add_value(device, "class-code",	(UINT8*)ClassFix, 4);

//  DualLink = gSettings.DualLink;

//  MacModel = GetModelFromString(gSettings.ProductName);
  switch (gma_dev->device_id) {
    case 0x0102: 
      devprop_add_value(device, "class-code",	(UINT8*)ClassFix, 4);
    case 0x0106:
    case 0x0112:  
    case 0x0116:
    case 0x0122:
    case 0x0126:
 /*     switch (MacModel) {
        case MacBookPro81:
          SnbId = 0x00000100;
          break;
        case MacBookPro83:
          SnbId = 0x00000200;
          break;
        case MacMini51:
          SnbId = 0x10000300;
          break;
      case Macmini52:
          SnbId = 0x20000300;
          break;
        case MacBookAir41:
          SnbId = 0x00000400;
          break;

        default:
          break;
      }
  */
    case 0x0152:
    case 0x0156:
    case 0x0162:
    case 0x0166:
    case 0x016A:
    case 0x0412:
    case 0x0416:
    case 0x041E:
    case 0x0A0E:
    case 0x0A16:
    case 0x0A1E:
    case 0x0A26:
    case 0x0A2E:
    case 0x0D22:
    case 0x0D26:
    case 0x1602:
    case 0x1606:
    case 0x160A:
    case 0x1612:
    case 0x1616:
    case 0x161A:
    case 0x161E:
    case 0x1622:
    case 0x1626:
    case 0x162A:
    case 0x1632:
    case 0x1636:
    case 0x163A:
    case 0x163E:
    case 0x1902:
    case 0x1906:
    case 0x190A://#
    case 0x190E://#
    case 0x1912:
    case 0x1913://#
    case 0x1915://#
    case 0x1916:
    case 0x1917://#
    case 0x191A:
    case 0x191B://#
    case 0x191D://#
    case 0x191E:
    case 0x1921://#
    //0x1922://#
    case 0x1923://#
    case 0x1926:
    case 0x1927://#
    case 0x192A:
    case 0x192B://#
    case 0x1932:
    case 0x193A:
    case 0x193B://#
    case 0x193D://#
      if (!SetSnb) {
        switch (gma_dev->device_id) {
          case 0x162:
          case 0x16A:
            devprop_add_value(device, "AAPL,ig-platform-id", GMAX3100_vals[23], 4);
            break;
          case 0x152:
            devprop_add_value(device, "AAPL,ig-platform-id", GMAX3100_vals[24], 4);
            break;
          case 0x166:
          case 0x156:
            devprop_add_value(device, "AAPL,ig-platform-id", GMAX3100_vals[25], 4);
            break;
          case 0x0102:
          case 0x0106:
          case 0x0112:
          case 0x0116:
          case 0x0122:
          case 0x0126:
            break;
          default:
            devprop_add_value(device, "AAPL,ig-platform-id", GMAX3100_vals[26], 4);
            break;
        }
      }

    case 0xA011:
    case 0xA012:  
      if (DualLink != 0) {
        devprop_add_value(device, "AAPL00,DualLink", (UINT8*)&DualLink, 1);
      }
    case 0x2582:
    case 0x2592:
    case 0x27A2:
    case 0x27AE:
      devprop_add_value(device, "AAPL,HasPanel", reg_TRUE, 4);
      devprop_add_value(device, "built-in", &BuiltIn, 1);      
      break;
    case 0x2772:
    case 0x29C2:  
    case 0x0042:
    case 0x0046: 
    case 0xA002:  
      devprop_add_value(device, "built-in", &BuiltIn, 1);
      devprop_add_value(device, "AAPL00,DualLink", (UINT8*)&DualLink, 1);
      devprop_add_value(device, "AAPL,os-info", (UINT8*)&OsInfo, sizeof(OsInfo));
      break;
    case 0x2A02:
    case 0x2A12:
    case 0x2A42:
      devprop_add_value(device, "AAPL,HasPanel", GMAX3100_vals[0], 4);
      devprop_add_value(device, "AAPL,SelfRefreshSupported", GMAX3100_vals[1], 4);
      devprop_add_value(device, "AAPL,aux-power-connected", GMAX3100_vals[2], 4);
      devprop_add_value(device, "AAPL,backlight-control", GMAX3100_vals[3], 4);
      devprop_add_value(device, "AAPL00,blackscreen-preferences", GMAX3100_vals[4], 4);
      devprop_add_value(device, "AAPL01,BacklightIntensity", GMAX3100_vals[5], 4);
      devprop_add_value(device, "AAPL01,blackscreen-preferences", GMAX3100_vals[6], 4);
      devprop_add_value(device, "AAPL01,DataJustify", GMAX3100_vals[7], 4);
     // devprop_add_value(device, "AAPL01,Depth", GMAX3100_vals[8], 4);
      devprop_add_value(device, "AAPL01,Dither", GMAX3100_vals[9], 4);
      devprop_add_value(device, "AAPL01,DualLink", (UINT8 *)&DualLink, 1);
     // devprop_add_value(device, "AAPL01,Height", GMAX3100_vals[10], 4);
      devprop_add_value(device, "AAPL01,Interlace", GMAX3100_vals[11], 4);
      devprop_add_value(device, "AAPL01,Inverter", GMAX3100_vals[12], 4);
      devprop_add_value(device, "AAPL01,InverterCurrent", GMAX3100_vals[13], 4);
      //		devprop_add_value(device, "AAPL01,InverterCurrency", GMAX3100_vals[15], 4);
      devprop_add_value(device, "AAPL01,LinkFormat", GMAX3100_vals[14], 4);
      devprop_add_value(device, "AAPL01,LinkType", GMAX3100_vals[15], 4);
      devprop_add_value(device, "AAPL01,Pipe", GMAX3100_vals[16], 4);
     // devprop_add_value(device, "AAPL01,PixelFormat", GMAX3100_vals[17], 4);
      devprop_add_value(device, "AAPL01,Refresh", GMAX3100_vals[18], 4);
      devprop_add_value(device, "AAPL01,Stretch", GMAX3100_vals[19], 4);
      devprop_add_value(device, "AAPL01,InverterFrequency", GMAX3100_vals[20], 4);
      //		devprop_add_value(device, "class-code",		(UINT8*)ClassFix, 4);
//      devprop_add_value(device, "subsystem-vendor-id", GMAX3100_vals[21], 4);
      devprop_add_value(device, "subsystem-id", GMAX3100_vals[22], 4);
      devprop_add_value(device, "built-in", &BuiltIn, 1);
      break;
    default:
      DBG("Intel card id=%x unsupported, please report to the project home\n", gma_dev->device_id);
      return FALSE;
  }
#if DEBUG_GMA == 2  
	gBS->Stall(5000000);
#endif  

	
	return TRUE;
}
