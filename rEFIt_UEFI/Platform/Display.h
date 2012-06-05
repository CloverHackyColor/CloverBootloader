/*
 * Copyright (c) 2011-2012 Frank Peng. All rights reserved.
 *
 */

#ifndef _DISPLAY_H_
#define _DISPLAY_H_
/*
UINT8 hexstrtouint8 (CHAR8* buf)
{
	INT8 i;
	if (IS_DIGIT(buf[0]))
		i = buf[0]-'0';
	else if (IS_HEX(buf[0])) 
    i = buf[0]-'a' + 10; 
  else
		i = buf[0]-'A' + 10;
  if (AsciiStrLen(buf) == 1) {
    return i;
  }
	i <<= 4;
	if (IS_DIGIT(buf[1]))
		i += buf[1]-'0';
	else if (IS_HEX(buf[1])) 
    i += buf[1]-'a' + 10; 
	else
		i += buf[1]-'A'+ 10; //no error checking
	return i;
}

BOOLEAN IsHexDigit (CHAR8 c) {
	return (IS_DIGIT(c) || (c>='A'&&c<='F') || (c>='a'&&c<='f'))?TRUE:FALSE;
}


BOOLEAN hex2bin(IN CHAR8 *hex, OUT UINT8 *bin, INT32 len)
{
	CHAR8	*p;
	INT32	i;
	CHAR8	buf[3];
	
	if (hex == NULL || bin == NULL || len <= 0 || AsciiStrLen(hex) != len * 2) {
		DBG("[ERROR] bin2hex input error\n");
		return FALSE;
	}
	
	buf[2] = '\0';
	p = (CHAR8 *) hex;
	
	for (i = 0; i < len; i++)
	{
		if (!IsHexDigit(p[0]) || !IsHexDigit(p[1])) {
			DBG("[ERROR] bin2hex '%a' syntax error\n", hex);
			return -2;
		}
		buf[0] = *p++;
		buf[1] = *p++;
		bin[i] = hexstrtouint8(buf);
	}
	return TRUE;
}
*/
// ATI Card
typedef struct {
	CHAR8		*name;
	UINT8			ports;
} ati_card_config_t;

ati_card_config_t ati_card_configs[] = {
	{NULL,			0},
    {"Wormy",   2},
	{"Alopias",		2},
	{"Alouatta",	4},
	{"Baboon",		3},
	{"Cardinal",	2},
	{"Caretta",		1},
	{"Colobus",		2},
	{"Douc",		2},
	{"Eulemur",		3},
	{"Flicker",		3},
	{"Galago",		2},
	{"Gliff",		3},
	{"Hoolock",		1},  //{"Hoolock",		3},
	{"Hypoprion",	2},
	{"Iago",		2},
	{"Kakapo",		3},
	{"Kipunji",		4},
	{"Lamna",		2},
	{"Langur",		3},
	{"Megalodon",	3},
	{"Motmot",		2},
	{"Nomascus",	5},
	{"Orangutan",	2},
	{"Peregrine",	2},
	{"Quail",		3},
	{"Raven",		3},
	{"Shrike",		3},
	{"Sphyrna",		1},
	{"Triakis",		2},
	{"Uakari",		4},
	{"Vervet",		4},
	{"Zonalis",		6},
	{"Pithecia",	3},
	{"Bulrushes",	6},
	{"Cattail",		4},
	{"Hydrilla",	5},
	{"Duckweed",	4},
	{"Fanwort",		4},
	{"Elodea",		5},
	{"Kudzu",		2},
	{"Gibba",		5},
	{"Lotus",		3},
	{"Ipomoea",		3},
	{"Mangabey",	2},
	{"Muskgrass",	4},
	{"Juncus",		4}
};

typedef enum {
	kNull,
    kWormy,
	kAlopias,
	kAlouatta,
	kBaboon,
	kCardinal,
	kCaretta,
	kColobus,
	kDouc,
	kEulemur,
	kFlicker,
	kGalago,
	kGliff,
	kHoolock,
	kHypoprion,
	kIago,
	kKakapo,
	kKipunji,
	kLamna,
	kLangur,
	kMegalodon,
	kMotmot,
	kNomascus,
	kOrangutan,
	kPeregrine,
	kQuail,
	kRaven,
	kShrike,
	kSphyrna,
	kTriakis,
	kUakari,
	kVervet,
	kZonalis,
	kPithecia,
	kBulrushes,
	kCattail,
	kHydrilla,
	kDuckweed,
	kFanwort,
	kElodea,
	kKudzu,
	kGibba,
	kLotus,
	kIpomoea,
	kMangabey,
	kMuskgrass,
	kJuncus,
//  kOsmunda,
	kCfgEnd
} ati_config_name_t;

typedef enum {
	CHIP_FAMILY_UNKNOW,
	/* IGP */
	CHIP_FAMILY_RS600,
	CHIP_FAMILY_RS690,
	CHIP_FAMILY_RS740,
	CHIP_FAMILY_RS780,
	CHIP_FAMILY_RS880,
	/* R600 */
	CHIP_FAMILY_R600,
	CHIP_FAMILY_RV610,
	CHIP_FAMILY_RV620,
	CHIP_FAMILY_RV630,
	CHIP_FAMILY_RV635,
	CHIP_FAMILY_RV670,
	/* R700 */
	CHIP_FAMILY_RV710,
	CHIP_FAMILY_RV730,
	CHIP_FAMILY_RV740,
	CHIP_FAMILY_RV770,
	/* Evergreen */
	CHIP_FAMILY_CEDAR,
	CHIP_FAMILY_CYPRESS,
	CHIP_FAMILY_HEMLOCK,
	CHIP_FAMILY_JUNIPER,
	CHIP_FAMILY_REDWOOD,
	/* Northern Islands */
	CHIP_FAMILY_BARTS,
	CHIP_FAMILY_CAICOS,
	CHIP_FAMILY_CAYMAN,
	CHIP_FAMILY_TURKS,
	CHIP_FAMILY_LAST
} ati_chip_family_t;

typedef struct {

  UINT16        deviceid;
  UINT32        subid;
  ati_chip_family_t			ati_chip_family;
  CHAR8*         model;
  ati_config_name_t	cfg_name;
} ati_card_t;

static ati_card_t radeon_cards[] = {
	
	{ 0x0000,	0x00000000, CHIP_FAMILY_UNKNOW,		NULL,						kNull		},

	// Earlier cards are not supported
	//
	// Layout is device_id, subsys_id (subsystem id plus vendor id), chip_family_name, display name, frame buffer
	// Cards are grouped by device id and vendor id then sorted by subsystem id to make it easier to add new cards
	//  
	{ 0x9400,	0x25521002, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 XT",			kNull		},
	{ 0x9400,	0x30001002, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 PRO",			kNull		},

	{ 0x9440,	0x0851174B, CHIP_FAMILY_RV770,		"ATI Radeon HD 4870",				kMotmot		}, 
	{ 0x9440,	0x114A174B, CHIP_FAMILY_RV770,		"Sapphire Radeon HD4870 Vapor-X",		kCardinal	}, 
	{ 0x9440,	0x24401682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4870",				kMotmot		},
	{ 0x9440,	0x24411682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4870",				kMotmot		},
	{ 0x9440,	0x24441682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4870",				kMotmot		},
	{ 0x9440,	0x24451682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4870",				kMotmot		},

	{ 0x9441,	0x24401682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4870 X2",			kMotmot		},

	{ 0x9442,	0x080110B0, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850",				kMotmot		},

	{ 0x9442,	0x24701682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850",				kMotmot		},
	{ 0x9442,	0x24711682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850",				kMotmot		},

	{ 0x9442,	0xE104174B, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850",				kMotmot		},

	{ 0x944A,	0x30001043, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",			kMotmot		},

	{ 0x944A,	0x30001458, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",			kMotmot		},

	{ 0x944A,	0x30001462, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",			kMotmot		},

	{ 0x944A,	0x30001545, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",			kMotmot		},

	{ 0x944A,	0x30001682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",			kMotmot		},

	{ 0x944A,	0x3000174B, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",			kMotmot		},

	{ 0x944A,	0x30001787, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",			kMotmot		},

	{ 0x944A,	0x300017AF, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",			kMotmot		},

	{ 0x944C,	0x24801682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4830",				kMotmot		},
	{ 0x944C,	0x24811682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4830",				kMotmot		},

	{ 0x944E,	0x3260174B, CHIP_FAMILY_RV770,		"ATI Radeon HD 4810 Series",			kMotmot		},
	{ 0x944E,	0x3261174B, CHIP_FAMILY_RV770,		"ATI Radeon HD 4810 series",			kMotmot		},

	{ 0x944E,	0x30001787, CHIP_FAMILY_RV770,		"ATI Radeon HD 4730 Series",			kMotmot		},
	{ 0x944E,	0x30101787, CHIP_FAMILY_RV770,		"ATI Radeon HD 4810 Series",			kMotmot		},
	{ 0x944E,	0x31001787, CHIP_FAMILY_RV770,		"ATI Radeon HD 4820",				kMotmot		},
	
	{ 0x9480,	0x3628103C, CHIP_FAMILY_RV730,		"ATI Radeon HD 4650M",				kGliff		},
	
	{ 0x9480,	0x9035104D, CHIP_FAMILY_RV730,		"ATI Radeon HD 4650M",				kGliff		},
	
	{ 0x9490,	0x4710174B, CHIP_FAMILY_RV730,		"ATI Radeon HD 4710",				kNull		},
	
	{ 0x9490,	0x20031787, CHIP_FAMILY_RV730,		"ATI Radeon HD 4670",				kFlicker	},
	{ 0x9490,	0x30501787, CHIP_FAMILY_RV730,		"ATI Radeon HD 4710",				kNull		},
	
	{ 0x9490,	0x300017AF, CHIP_FAMILY_RV730,		"ATI Radeon HD 4710",				kNull		},
	
	{ 0x9498,	0x21CF1458, CHIP_FAMILY_RV730,		"ATI Radeon HD 4600 Series",			kNull		},
	
	{ 0x9498,	0x24511682, CHIP_FAMILY_RV730,		"ATI Radeon HD 4650",				kNull		},
	{ 0x9498,	0x24521682, CHIP_FAMILY_RV730,		"ATI Radeon HD 4650",				kNull		},
	{ 0x9498,	0x24541682, CHIP_FAMILY_RV730,		"ATI Radeon HD 4650",				kNull		},
	{ 0x9498,	0x29331682, CHIP_FAMILY_RV730,		"ATI Radeon HD 4670",				kNull		},
	{ 0x9498,	0x29341682, CHIP_FAMILY_RV730,		"ATI Radeon HD 4670",				kNull		},
	
	{ 0x9498,	0x30501787, CHIP_FAMILY_RV730,		"ATI Radeon HD 4700",				kNull		},
	{ 0x9498,	0x31001787, CHIP_FAMILY_RV730,		"ATI Radeon HD 4720",				kNull		},
	
	{ 0x94B3,	0x0D001002, CHIP_FAMILY_RV740,		"ATI Radeon HD 4770",				kFlicker	},
	
	{ 0x94B3,	0x29001682, CHIP_FAMILY_RV740,		"ATI Radeon HD 4770",				kFlicker	},
	
	{ 0x94B3,	0x1170174B, CHIP_FAMILY_RV740,		"ATI Radeon HD 4770",				kFlicker	},
	
	{ 0x94C1,	0x0D021002, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 XT",			kNull		},
	{ 0x94C1,	0x10021002, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Pro",			kNull		},
	
	{ 0x94C1,	0x0D021028, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 XT",			kNull		},
	
	{ 0x94C1,	0x21741458, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 XT",			kNull		},
	
	{ 0x94C1,	0x10331462, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 XT",			kNull		},
	{ 0x94C1,	0x10401462, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 XT",			kNull		},
	{ 0x94C1,	0x11101462, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 XT",			kNull		},
	
	{ 0x94C3,	0x03421002, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 PRO",			kNull		},
	
	{ 0x94C3,	0x30001025, CHIP_FAMILY_RV610,		"ATI Radeon HD 2350 Series",			kNull		},
	
	{ 0x94C3,	0x03021028, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 PRO",			kNull		},
	{ 0x94C3,	0x04021028, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 PRO",			kNull		},
	
	{ 0x94C3,	0x216A1458, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 PRO",			kNull		},
	{ 0x94C3,	0x21721458, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 PRO",			kNull		},
	{ 0x94C3,	0x30001458, CHIP_FAMILY_RV610,		"ATI Radeon HD 3410",				kNull		},
	
	{ 0x94C3,	0x10321462, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 PRO",			kNull		},
	{ 0x94C3,	0x10411462, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400",				kNull		},
	{ 0x94C3,	0x11041462, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400",				kNull		},
	{ 0x94C3,	0x11051462, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400",				kNull		},
	{ 0x94C3,	0x30001462, CHIP_FAMILY_RV610,		"ATI Radeon HD 3410",				kNull		},
	
	{ 0x94C3,	0x2247148C, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 LE",			kNull		},
	{ 0x94C3,	0x3000148C, CHIP_FAMILY_RV610,		"ATI Radeon HD 2350 Series",			kNull		},
	
	{ 0x94C3,	0x30001642, CHIP_FAMILY_RV610,		"ATI Radeon HD 3410",				kNull		},
	{ 0x94C3,	0x37161642, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 PRO",			kNull		},
	
	{ 0x94C3,	0x3000174B, CHIP_FAMILY_RV610,		"ATI Radeon HD 2350 Series",			kNull		},
	{ 0x94C3,	0xE370174B, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 PRO",			kNull		},
	{ 0x94C3,	0xE400174B, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 PRO",			kNull		},
	
	{ 0x94C3,	0x203817AF, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400",				kNull		},
	
	{ 0x94C3,	0x22471787, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 LE",			kNull		},
	{ 0x94C3,	0x30001787, CHIP_FAMILY_RV610,		"ATI Radeon HD 2350 Series",			kNull		},
	
	{ 0x94C3,	0x01011A93, CHIP_FAMILY_RV610,		"Qimonda Radeon HD 2400 PRO",			kNull		},
	
	{ 0x9501,	0x25421002, CHIP_FAMILY_RV670,		"ATI Radeon HD 3870",				kNull		},
	{ 0x9501,	0x30001002, CHIP_FAMILY_RV670,		"ATI Radeon HD 3690",				kNull		},
	
	{ 0x9501,	0x3000174B, CHIP_FAMILY_RV670,		"Sapphire Radeon HD 3690",			kNull		},
	{ 0x9501,	0x4750174B, CHIP_FAMILY_RV670,		"ATI Radeon HD 4750",				kNull		},
	
	{ 0x9501,	0x30001787, CHIP_FAMILY_RV670,		"ATI Radeon HD 3690",				kNull		},
	
	{ 0x9505,	0x25421002, CHIP_FAMILY_RV670,		"ATI Radeon HD 3850",				kNull		},
	{ 0x9505,	0x30001002, CHIP_FAMILY_RV670,		"ATI Radeon HD 3690",				kNull		},
	
	{ 0x9505,	0x30011043, CHIP_FAMILY_RV670,		"ATI Radeon HD 4730",				kNull		},
	
	{ 0x9505,	0x3000148C, CHIP_FAMILY_RV670,		"ATI Radeon HD 3850",				kNull		},
	{ 0x9505,	0x3001148C, CHIP_FAMILY_RV670,		"ATI Radeon HD 4730",				kNull		},
	{ 0x9505,	0x3002148C, CHIP_FAMILY_RV670,		"ATI Radeon HD 4730",				kNull		},
	{ 0x9505,	0x3003148C, CHIP_FAMILY_RV670,		"ATI Radeon HD 4750",				kNull		},
	{ 0x9505,	0x3004148C, CHIP_FAMILY_RV670,		"ATI Radeon HD 4750",				kNull		},
	
	{ 0x9505,	0x3000174B, CHIP_FAMILY_RV670,		"Sapphire Radeon HD 3690",			kNull		},
	{ 0x9505,	0x3001174B, CHIP_FAMILY_RV670,		"ATI Radeon HD 4750",				kNull		},
	{ 0x9505,	0x3010174B, CHIP_FAMILY_RV670,		"ATI Radeon HD 4750",				kNull		},
	{ 0x9505,	0x4730174B, CHIP_FAMILY_RV670,		"ATI Radeon HD 4730",				kNull		},
	
	{ 0x9505,	0x30001787, CHIP_FAMILY_RV670,		"ATI Radeon HD 3690",				kNull		},
	{ 0x9505,	0x301017AF, CHIP_FAMILY_RV670,		"ATI Radeon HD 4750",				kNull		},
	
	{ 0x9540,	0x4590174B, CHIP_FAMILY_RV710,		"ATI Radeon HD 4590",				kNull		},
	
	{ 0x9540,	0x30501787, CHIP_FAMILY_RV710,		"ATI Radeon HD 4590",				kNull		},

	{ 0x954F,	0x16131462, CHIP_FAMILY_RV710,		"ATI Radeon HD 4550",				kNull		}, 
	{ 0x954F,	0x20081787, CHIP_FAMILY_RV710,		"ATI Radeon HD 4350",				kNull		}, 
	{ 0x954F,	0x29201682, CHIP_FAMILY_RV710,		"ATI Radeon HD 4550",				kNull		},
	{ 0x954F,	0x29211682, CHIP_FAMILY_RV710,		"ATI Radeon HD 4550",				kNull		},
	{ 0x954F,	0x30901682, CHIP_FAMILY_RV710,		"XFX Radeon HD 4570",				kNull		},

	{ 0x954F,	0x30501787, CHIP_FAMILY_RV710,		"ATI Radeon HD 4450",				kNull		},
	{ 0x954F,	0x31001787, CHIP_FAMILY_RV710,		"ATI Radeon HD 4520",				kNull		},

	{ 0x954F,	0x3000174B, CHIP_FAMILY_RV710,		"ATI Radeon HD 4520",				kNull		},
	{ 0x954F,	0x4450174B, CHIP_FAMILY_RV710,		"ATI Radeon HD 4450",				kNull		},
	{ 0x954F,	0x4570174B, CHIP_FAMILY_RV710,		"Sapphire Radeon HD 4570",			kNull		},
	{ 0x954F,	0xE990174B, CHIP_FAMILY_RV710,		"Sapphire Radeon HD 4350",			kNull		},

	{ 0x954F,	0x301017AF, CHIP_FAMILY_RV710,		"ATI Radeon HD 4450",				kNull		},

	{ 0x9552,	0x04341028, CHIP_FAMILY_RV710,		"ATI Mobility Radeon 4330",			kShrike		},

	{ 0x9552,	0x308B103C, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4300 Series",		kShrike		},

	{ 0x9552,	0x3000148C, CHIP_FAMILY_RV710,		"ATI Radeon HD 4300/4500 Series",		kNull		},

	{ 0x9552,	0x3000174B, CHIP_FAMILY_RV710,		"ATI Radeon HD 4300/4500 Series",		kNull		},

	{ 0x9552,	0x30001787, CHIP_FAMILY_RV710,		"ATI Radeon HD 4300/4500 Series",		kNull		},

	{ 0x9552,	0x300017AF, CHIP_FAMILY_RV710,		"ATI Radeon HD 4300/4500 Series",		kNull		},

	{ 0x9553,	0x18751043, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4570",			kShrike		},
	{ 0x9553,	0x1B321043, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4570",			kShrike		},

	{ 0x9581,	0x95811002, CHIP_FAMILY_RV630,		"ATI Radeon HD 3600 Series",			kNull		},

	{ 0x9581,	0x3000148C, CHIP_FAMILY_RV630,		"ATI Radeon HD 3600 Series",			kNull		},

	{ 0x9583,	0x3000148C, CHIP_FAMILY_RV630,		"ATI Radeon HD 3600 Series",			kNull		},

	{ 0x9588,	0x01021A93, CHIP_FAMILY_RV630,		"Qimonda Radeon HD 2600 XT",			kNull		},

	{ 0x9589,	0x30001462, CHIP_FAMILY_RV630,		"ATI Radeon HD 3610",				kNull		},

	{ 0x9589,	0x0E41174B, CHIP_FAMILY_RV630,		"ATI Radeon HD 3600 Series",			kNull		},

	{ 0x9589,	0x30001787, CHIP_FAMILY_RV630,		"ATI Radeon HD 3600 Series",			kNull		},

	{ 0x9589,	0x01001A93, CHIP_FAMILY_RV630,		"Qimonda Radeon HD 2600 PRO",			kNull		},

	{ 0x9591,	0x2303148C, CHIP_FAMILY_RV635,		"ATI Radeon HD 3600 Series",			kNull		},

	{ 0x9598,	0xB3831002, CHIP_FAMILY_RV635,		"ATI All-in-Wonder HD",				kNull		},

	{ 0x9598,	0x30001043, CHIP_FAMILY_RV635,		"ATI Radeon HD 3730",				kNull		},
	{ 0x9598,	0x30011043, CHIP_FAMILY_RV635,		"ATI Radeon HD 4570",				kNull		},

	{ 0x9598,	0x3000148C, CHIP_FAMILY_RV635,		"ATI Radeon HD 3730",				kNull		},
	{ 0x9598,	0x3001148C, CHIP_FAMILY_RV635,		"ATI Radeon HD 4580",				kNull		},
	{ 0x9598,	0x3031148C, CHIP_FAMILY_RV635,		"ATI Radeon HD 4570",				kNull		},

	{ 0x9598,	0x30001545, CHIP_FAMILY_RV635,		"VisionTek Radeon HD 2600 XT",			kNull		},
	{ 0x9598,	0x30011545, CHIP_FAMILY_RV635,		"VisionTek Radeon HD 2600 Pro",			kNull		},

	{ 0x9598,	0x3000174B, CHIP_FAMILY_RV635,		"Sapphire Radeon HD 3730",			kNull		},
	{ 0x9598,	0x3001174B, CHIP_FAMILY_RV635,		"Sapphire Radeon HD 3750",			kNull		},
	{ 0x9598,	0x4570174B, CHIP_FAMILY_RV635,		"ATI Radeon HD 4570",				kNull		},
	{ 0x9598,	0x4580174B, CHIP_FAMILY_RV635,		"ATI Radeon HD 4580",				kNull		},
	{ 0x9598,	0x4610174B, CHIP_FAMILY_RV635,		"ATI Radeon HD 4610",				kNull		},

	{ 0x9598,	0x300117AF, CHIP_FAMILY_RV635,		"ATI Radeon HD 3750",				kNull		},
	{ 0x9598,	0x301017AF, CHIP_FAMILY_RV635,		"ATI Radeon HD 4570",				kNull		},
	{ 0x9598,	0x301117AF, CHIP_FAMILY_RV635,		"ATI Radeon HD 4580",				kNull		},

	{ 0x9598,	0x30501787, CHIP_FAMILY_RV635,		"ATI Radeon HD 4610",				kNull		},

	{ 0x95C0,	0x3000148C, CHIP_FAMILY_RV620,		"ATI Radeon HD 3550",				kNull		},

	{ 0x95C0,	0xE3901745, CHIP_FAMILY_RV620,		"ATI Radeon HD 3550",				kNull		},

	{ 0x95C0,	0x3000174B, CHIP_FAMILY_RV620,		"Sapphire Radeon HD 3550",			kNull		},
	{ 0x95C0,	0x3002174B, CHIP_FAMILY_RV620,		"ATI Radeon HD 3570",				kNull		},
	{ 0x95C0,	0x3020174B, CHIP_FAMILY_RV620,		"ATI Radeon HD 4250",				kNull		},

	{ 0x95C5,	0x3000148C, CHIP_FAMILY_RV620,		"ATI Radeon HD 3450",				kNull		},
	{ 0x95C5,	0x3001148C, CHIP_FAMILY_RV620,		"ATI Radeon HD 3550",				kNull		},
	{ 0x95C5,	0x3002148C, CHIP_FAMILY_RV620,		"ATI Radeon HD 4230",				kNull		},
	{ 0x95C5,	0x3003148C, CHIP_FAMILY_RV620,		"ATI Radeon HD 4250",				kNull		},
	{ 0x95C5,	0x3032148C, CHIP_FAMILY_RV620,		"ATI Radeon HD 4250",				kNull		},
	{ 0x95C5,	0x3033148C, CHIP_FAMILY_RV620,		"ATI Radeon HD 4230",				kNull		},

	{ 0x95C5,	0x3010174B, CHIP_FAMILY_RV620,		"ATI Radeon HD 4250",				kNull		},
	{ 0x95C5,	0x4250174B, CHIP_FAMILY_RV620,		"ATI Radeon HD 4250",				kNull		},

	{ 0x95C5,	0x30501787, CHIP_FAMILY_RV620,		"ATI Radeon HD 4250",				kNull		},

	{ 0x95C5,	0x301017AF, CHIP_FAMILY_RV620,		"ATI Radeon HD 4230",				kNull		},

	{ 0x95C5,	0x01041A93, CHIP_FAMILY_RV620,		"Qimonda Radeon HD 3450",			kNull		},
	{ 0x95C5,	0x01051A93, CHIP_FAMILY_RV620,		"Qimonda Radeon HD 3450",			kNull		},

	/* Evergreen */
	{ 0x6898,	0x0B001002, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5870",				kZonalis	},

	{ 0x6898,	0x032E1043, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5870",				kUakari		},

	{ 0x6898,	0x00D0106B, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5870",				kLangur		},

	{ 0x6898,	0xE140174B, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5870",				kUakari		},

	{ 0x6898,	0x29611682, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5870",				kUakari		},

	{ 0x6899,	0x21E41458, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5850",				kUakari		},

	{ 0x6899,	0xE140174B, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5850",				kUakari		},
	{ 0x6899,	0xE174174B, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5850",				kUakari		},
	
	{ 0x6899,	0x200A1787, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5850",				kUakari		},
	{ 0x6899,	0x22901787, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5850",				kUakari		},
	
	{ 0x689C,	0x03521043, CHIP_FAMILY_HEMLOCK,	"ASUS ARES",						kUakari		},
	{ 0x689C,	0x039E1043, CHIP_FAMILY_HEMLOCK,	"ASUS EAH5870 Series",				kUakari		},
	
	{ 0x689C,	0x30201682, CHIP_FAMILY_HEMLOCK,	"ATI Radeon HD 5970",				kUakari		},

	{ 0x68A0,	0x043A1028, CHIP_FAMILY_CYPRESS,	"ATI Mobility Radeon HD 5870",		kNomascus	},
	
	{ 0x68A1,	0x144D103C, CHIP_FAMILY_CYPRESS,	"ATI Mobility Radeon HD 5850",		kNomascus	},
	{ 0x68A1,	0x1522103C, CHIP_FAMILY_CYPRESS,	"ATI Mobility Radeon HD 5850",		kHoolock	},
	
	{ 0x68A8,	0x050E1025, CHIP_FAMILY_CYPRESS,	"AMD Radeon HD 6850M",				kUakari		},

	{ 0x68BA,	0x174B1482, CHIP_FAMILY_JUNIPER,	"ATI Sapphire Radeon HD 6770",			kVervet		}, 

	{ 0x68B8,	0x00CF106B, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",				kHoolock	},

	{ 0x68B8,	0x21D71458, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",				kVervet		},
	{ 0x68B8,	0x21F61458, CHIP_FAMILY_JUNIPER,	"GigaByte HD5770 R577SL-1GD",			kVervet		}, 

	{ 0x68B8,	0x29901682, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",				kVervet		},
	{ 0x68B8,	0x29911682, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",				kVervet		},

	{ 0x68B8,	0x1482174B, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",				kVervet		},
	{ 0x68B8,	0xE144174B, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",				kHoolock	}, 
	{ 0x68B8,	0xE147174B, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",				kVervet		},

	{ 0x68B8,	0x200A1787, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",				kVervet		}, 
	{ 0x68B8,	0x200B1787, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",				kVervet		},
	{ 0x68B8,	0x22881787, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",				kVervet		},

	{ 0x68BE,	0x22881787, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5750",			kVervet		}, 

	{ 0x68BF,	0x220E1458, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 6750",				kVervet		},

	{ 0x68C0,	0x1594103C, CHIP_FAMILY_REDWOOD,	"AMD Radeon HD 6570M",				kNull		},

	{ 0x68C0,	0x392717AA, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5730",			kNull		},

	{ 0x68C0,	0x395217AA, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5730",			kNull		}, 

	{ 0x68C1,	0x02051025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		}, 
	{ 0x68C1,	0x02961025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		}, 
	{ 0x68C1,	0x030A1025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		}, 
	{ 0x68C1,	0x033D1025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		}, 
	{ 0x68C1,	0x033E1025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		}, 
	{ 0x68C1,	0x03471025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		}, 
	{ 0x68C1,	0x03561025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		}, 
	{ 0x68C1,	0x03581025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		}, 
	{ 0x68C1,	0x035A1025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		}, 
	{ 0x68C1,	0x035C1025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		}, 
	{ 0x68C1,	0x03641025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		}, 
	{ 0x68C1,	0x03791025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		}, 
	{ 0x68C1,	0x037E1025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		}, 
	{ 0x68C1,	0x03821025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5750",			kNull		}, 

	{ 0x68C1,	0x033E1025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5650",			kNull		},

	{ 0x68C1,	0x9071104D, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5650",			kEulemur	},

	{ 0x68C8,	0x2306103C, CHIP_FAMILY_REDWOOD,	"ATI FirePro V4800 (FireGL)",			kNull		},

	{ 0x68D8,	0x03561043, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5670",				kBaboon		},
	{ 0x68D8,	0x68e01028, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5670", 				kBaboon		},
	{ 0x68D8,	0x21D91458, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5670",				kBaboon		},

	{ 0x68D8,	0x5690174B, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5690",				kNull		},
	{ 0x68D8,	0x5730174B, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5730",				kNull		},
	{ 0x68D8,	0xE151174B, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5670",				kBaboon		},

	{ 0x68D8,	0x30001787, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5730",				kNull		},

	{ 0x68D8,	0x301017AF, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5730",				kNull		},
	{ 0x68D8,	0x301117AF, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5690",				kNull		},

	{ 0x68D9,	0x301017AF, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5630",				kNull		},

	{ 0x68DA,	0x5630174B, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5630",				kNull		},

	{ 0x68DA,	0x30001787, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5630",				kNull		},

	{ 0x68DA,	0x301017AF, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5630",				kNull		},

	{ 0x68E0,	0x04561028, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470M",				kEulemur	},

	{ 0x68E0,	0x1433103C, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470M",				kEulemur	},

	{ 0x68E1,	0x1426103C, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5430M",				kEulemur	},

	{ 0x68F9,	0x010E1002, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",				kEulemur		}, 

	{ 0x68F9,	0x03741043, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",     	                kEulemur		}, 

	{ 0x68F9,	0x5470174B, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470",				kNull		},
	{ 0x68F9,	0x5490174B, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5490",				kNull		},
	{ 0x68F9,	0x5530174B, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5530",				kNull		},

	{ 0x68F9,	0x20091787, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",				kEulemur	},
	{ 0x68F9,	0x22911787, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",				kHoolock	}, // my card
	{ 0x68F9,	0x23401462, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",				kEulemur	}, 
	{ 0x68F9,	0x30001787, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470",				kNull		},
	{ 0x68F9,	0x30011787, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5530",				kNull		},
	{ 0x68F9,	0x30021787, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5490",				kNull		},

	{ 0x68F9,	0x301117AF, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470",				kNull		},
	{ 0x68F9,	0x301217AF, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5490",				kNull		},
	{ 0x68F9,	0x301317AF, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470",				kNull		},

	{ 0x68F9,	0xE145174B, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",				kEulemur	}, 
	{ 0x68F9,	0xE153174B, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",				kEulemur	},
	{ 0x68F9,	0x30321682, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",				kEulemur	},
	{ 0x68F9,	0x303A1682, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",				kEulemur	},
	{ 0x68E1,	0x3000174B, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",				kEulemur	},

	/* Northen Islands */
	{ 0x6718,	0x0B001002, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970",				kNull		},
	{ 0x6718,	0x67181002, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970",				kNull		},
	{ 0x6718,	0x31301682, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970",				kNull		},

	{ 0x6719,	0x0B001002, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6950",				kGibba		}, 
	{ 0x6719,	0x186B174B, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6950",				kNull		}, 
	{ 0x6719,	0x20101787, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6950",				kGibba		}, 

	{ 0x6720,	0x04BA1028, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6970M",				kElodea		}, 
	{ 0x6720,	0x51041558, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6990M",				kElodea		},

	{ 0x6738,	0x00D01002, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",				kDuckweed	},
	{ 0x6738,	0x174B174B, CHIP_FAMILY_BARTS,		"Sapphire Radeon HD6870",			kDuckweed	}, // ?? kBulrushes ??
	{ 0x6738,	0x21FA1002, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",				kDuckweed	},
	{ 0x6738,	0x67381002, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",				kDuckweed	},

	{ 0x6738,	0x21FA1458, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",				kDuckweed	},

	{ 0x6738,	0x31031682, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",				kDuckweed	},
	{ 0x6738,	0x31041682, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",				kDuckweed	},
	{ 0x6738,	0x31071682, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",				kDuckweed	}, 
	{ 0x6738,	0x31081682, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",				kDuckweed	},  // ?? kJuncus ??

	{ 0x6738,	0xE178174B, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",				kDuckweed	},

	{ 0x6738,	0x20101787, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",				kDuckweed	},
	{ 0x6738,	0x23051787, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",				kDuckweed	},

	{ 0x6739,	0xAA881002, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",				kDuckweed	},
	{ 0x6739,	0x03B41043, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",				kDuckweed	}, 
	{ 0x6739,	0x21F81458, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",				kBulrushes	}, 
	{ 0x6739,	0x24411462, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",				kDuckweed	},
	{ 0x6739,	0x31101682, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",				kDuckweed	},   
	{ 0x6739,	0x67391002, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",				kDuckweed	},
	{ 0x6739,	0xE177174B, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",				kDuckweed	},

	{ 0x6740,	0x1D121043, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6730M",				kNull		}, 
	{ 0x6740,	0x1657103C, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6770M",				kNull		},
	{ 0x6740,	0x165A103C, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6770M",				kNull		},

	{ 0x6741,	0x050E1025, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6650M",				kNull		},
	{ 0x6741,	0x05131025, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6650M",				kNull		},

	{ 0x6741,	0x1646103C, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6750M",				kNull		},

	{ 0x6741,	0x9080104D, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6630M",				kNull		},

	{ 0x6758,	0x67581002, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670",				kBulrushes	},

	{ 0x6758,	0x22051458, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670",				kBulrushes	},

	{ 0x6758,	0xE194174B, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670",				kBulrushes	},

	{ 0x6758,	0x31811682, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670",				kBulrushes	},
	{ 0x6758,	0x31831682, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670",				kBulrushes	},

	{ 0x6758,	0xE1941746, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670",				kBulrushes	},

	{ 0x6759,	0x20121787, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6570",				kPithecia	}, // ErmaC 
	{ 0x6759,	0xE193174B, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6570",				kNull		},

	{ 0x6760,	0x04CC1028, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6490M",				kNull		},

	{ 0x6760,	0x165A103C, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6470M",				kNull		},
	{ 0x6760,	0x167D103C, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6470M",				kNull		}, 

	{ 0x6760,	0x1CB21043, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6470M",				kNull		},

	{ 0x6779,	0x64501092, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450",				kBulrushes	},
	{ 0x6779,	0xE164174B, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450",				kBulrushes	},
	{ 0x6779,	0xE180174B, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450",				kPithecia	}, // ErmaC 

	/* standard/default models */
	{ 0x9400,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 XT",			kNull		},
	{ 0x9405,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 GT",			kNull		},

	{ 0x9440,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",			kMotmot		},
	{ 0x9441,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4870 X2",			kMotmot		},
	{ 0x9442,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",			kMotmot		},
	{ 0x9443,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850 X2",			kMotmot		},
	{ 0x944C,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",			kMotmot		},
	{ 0x944E,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4700 Series",			kMotmot		},

	{ 0x9450,	0x00000000, CHIP_FAMILY_RV770,		"AMD FireStream 9270",				kMotmot		},
	{ 0x9452,	0x00000000, CHIP_FAMILY_RV770,		"AMD FireStream 9250",				kMotmot		},

	{ 0x9460,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",			kMotmot		},
	{ 0x9462,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",			kMotmot		},

	{ 0x9490,	0x00000000, CHIP_FAMILY_RV730,		"ATI Radeon HD 4600 Series",			kFlicker	},
	{ 0x9498,	0x00000000, CHIP_FAMILY_RV730,		"ATI Radeon HD 4600 Series",			kFlicker	},

	{ 0x94B3,	0x00000000, CHIP_FAMILY_RV740,		"ATI Radeon HD 4770",				kFlicker	},
	{ 0x94B4,	0x00000000, CHIP_FAMILY_RV740,		"ATI Radeon HD 4700 Series",			kFlicker	},
	{ 0x94B5,	0x00000000, CHIP_FAMILY_RV740,		"ATI Radeon HD 4770",				kFlicker	},

	{ 0x94C1,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",			kIago		},
	{ 0x94C3,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",			kIago		},
	{ 0x94C7,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2350",				kIago		},
	{ 0x94CC,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",			kIago		},

	{ 0x9501,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3800 Series",			kMegalodon	},
	{ 0x9505,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3800 Series",			kMegalodon	},
	{ 0x9507,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3830",				kMegalodon	},
	{ 0x950F,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3870 X2",			kMegalodon	},

	{ 0x9513,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3850 X2",			kMegalodon	},
	{ 0x9519,	0x00000000, CHIP_FAMILY_RV670,		"AMD FireStream 9170",				kMegalodon	},

	{ 0x9540,	0x00000000, CHIP_FAMILY_RV710,		"ATI Radeon HD 4550",				kNull		},
	{ 0x954F,	0x00000000, CHIP_FAMILY_RV710,		"ATI Radeon HD 4300/4500 Series",		kNull		},

	{ 0x9553,	0x00000000, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4500/5100 Series",	kShrike 	},

	{ 0x9588,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 XT",			kLamna		},
	{ 0x9589,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 PRO",			kLamna		},
	{ 0x958A,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 X2 Series",			kLamna		},

	{ 0x9598,	0x00000000, CHIP_FAMILY_RV635,		"ATI Radeon HD 3600 Series",			kMegalodon	},

	{ 0x95C0,	0x00000000, CHIP_FAMILY_RV620,		"ATI Radeon HD 3400 Series",			kIago		},
	{ 0x95C5,	0x00000000, CHIP_FAMILY_RV620,		"ATI Radeon HD 3400 Series",			kIago		},

	/* IGP */
	{ 0x9610,	0x00000000, CHIP_FAMILY_RS780,		"ATI Radeon HD 3200 Graphics",			kNull		},
	{ 0x9611,	0x00000000, CHIP_FAMILY_RS780,		"ATI Radeon 3100 Graphics",			kNull		},
	{ 0x9614,	0x00000000, CHIP_FAMILY_RS780,		"ATI Radeon HD 3300 Graphics",			kNull		},
	{ 0x9616,	0x00000000, CHIP_FAMILY_RS780,		"AMD 760G",					kNull		},

	{ 0x9710,	0x00000000, CHIP_FAMILY_RS880,		"ATI Radeon HD 4200",				kNull		},
	{ 0x9715,	0x00000000, CHIP_FAMILY_RS880,		"ATI Radeon HD 4250",				kNull		},
	{ 0x9714,	0x00000000, CHIP_FAMILY_RS880,		"ATI Radeon HD 4290",				kNull		},

	/* Evergreen */
	{ 0x688D,	0x00000000, CHIP_FAMILY_CYPRESS,	"AMD FireStream 9350",				kUakari		},

	{ 0x6898,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5800 Series",			kUakari		},
	{ 0x6899,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5800 Series",			kUakari		},
	{ 0x689C,	0x00000000, CHIP_FAMILY_HEMLOCK,	"ATI Radeon HD 5900 Series",			kUakari		},
	{ 0x689E,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5800 Series",			kUakari		},

	{ 0x68A0,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Mobility Radeon HD 5800 Series",	kNomascus	}, 

	{ 0x68B8,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5700 Series",			kVervet		},
	{ 0x68B9,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5600 Series",			kVervet		},
	{ 0x68BA,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 6770 Series",			kVervet		},
	{ 0x68BE,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5700 Series",			kVervet		},

	{ 0x68D8,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5600 Series",			kBaboon		},
	{ 0x68D9,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5500 Series",			kBaboon		},
	{ 0x68DA,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5500 Series",			kBaboon		},

	{ 0x68F9,	0x00000000, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5400 Series",			kNull		},

	/* Northen Islands */
	{ 0x6718,	0x00000000, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970 Series",			kNull		},
	{ 0x6719,	0x00000000, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6950 Series",			kGibba		},

	{ 0x6720,	0x00000000, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6900M Series",			kNull		},

	{ 0x6738,	0x00000000, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870 Series",			kDuckweed	},
	{ 0x6739,	0x00000000, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850 Series",			kDuckweed	},
	{ 0x673E,	0x00000000, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6790 Series",			kNull		},

	{ 0x6740,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6700M Series",			kNull		},
	{ 0x6741,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6600M/6700M Series",		kNull		},

	{ 0x6758,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670 Series",			kBulrushes	},
	{ 0x6759,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6500 Series",			kNull		},
	{ 0x675F,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 5570 Series",			kNull		},

	{ 0x6760,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6400M Series",			kNull		},
	{ 0x6761,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6430M Series",			kNull		},
	
	{ 0x6770,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6400 Series",			kNull		},
	{ 0x6779,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450 Series",			kBulrushes	},

	/* Southen Islands */

};

CHAR8 *ati_name(UINT16 device_id, UINT32 sub_id)
{
    INT32	i;

	for (i=1; i< (sizeof(radeon_cards) / sizeof(radeon_cards[0])); i++) {
		if (radeon_cards[i].deviceid == device_id && radeon_cards[i].subid == sub_id) {
			return radeon_cards[i].model;
		}
	}
	return radeon_cards[0].model;
}

CHAR8 *ati_cfg_name(UINT16 device_id, UINT32 sub_id)
{
    INT32	i;

	for (i=1; i< (sizeof(radeon_cards) / sizeof(radeon_cards[0])); i++) 
	{
		if (radeon_cards[i].deviceid == device_id && radeon_cards[i].subid == sub_id) 
		{
			 return ati_card_configs[radeon_cards[i].cfg_name].name;
		}
	}
	return radeon_cards[0].model;
}

UINT8 ati_port(UINT16 device_id, UINT32 sub_id)
{
    INT32	i;

	for (i=1; i< (sizeof(radeon_cards) / sizeof(radeon_cards[0])); i++) 
	{
		if (radeon_cards[i].deviceid == device_id && radeon_cards[i].subid == sub_id) 
		{
            return ati_card_configs[radeon_cards[i].cfg_name].ports;
		}
	}
	return ati_card_configs[0].ports;
}

UINT32 ATI_vram_size(pci_dt_t *ati_dev)
{

    INT32	i;
    
    ati_chip_family_t chip_family=0;

	for (i=1; i< (sizeof(radeon_cards) / sizeof(radeon_cards[0])); i++) 
	{
		if (radeon_cards[i].deviceid == ati_dev->device_id && radeon_cards[i].subid == ati_dev->subsys_id.subsys_id) 
		{
			 chip_family = radeon_cards[i].ati_chip_family;
			 break;
		}
	}
	
	UINT8 *mmio = (UINT8*)(UINTN)(pci_config_read32(ati_dev, PCI_BASE_ADDRESS_2) & ~0x0f);
	
	UINT64 vram_size = 128 << 20; //default 128Mb, this is minimum for OS

    if (chip_family >= CHIP_FAMILY_CEDAR) 
    {
      // size in MB on evergreen
      // XXX watch for overflow!!!
      vram_size = REG32(mmio, 0x5428) << 20;
    } else if (chip_family >= CHIP_FAMILY_R600) 
    {
			vram_size = REG32(mmio, 0x5428);
    }
    else 
    {
      vram_size = REG32(mmio, 0x00f8);
      if (vram_size == 0) 
      {
          vram_size = REG32(mmio, 0x0108);
        //Slice - previously I successfully made Radeon9000 working
        //by writing this register
    //    WRITEREG32(card->mmio, RADEON_CONFIG_MEMSIZE, 0x30000);
      }
    }
  
    return vram_size;	
}

CHAR8* ATI_romrevision(pci_dt_t *ati_dev)
{
    CHAR8* cRev="109-B77101-11";
    
	option_rom_header_t *rom_addr;
	
	//rom_addr = (option_rom_header_t *)(UINTN)(pci_config_read32(ati_dev, 0x30) & ~0x7ff);
	rom_addr = (option_rom_header_t *)0xc0000;
	
	UINT8 *rev;
	
	if (!rom_addr)
		return cRev;
	
	rev = (UINT8*)rom_addr + *(UINT8 *)((UINT8*)rom_addr + 0x6e);
	
	CopyMem(cRev, rev, AsciiStrLen((CHAR8 *)rev));

	return cRev;
}

// NVIDA Card

struct nv_chipset_t {
	UINT32 device;
	CHAR8 *name;
};

static struct nv_chipset_t NVKnowns[] = {
	{ 0x00000000, "Unknown" },
//========================================
	// 0040 - 004F	
	{ 0x10DE0040, "GeForce 6800 Ultra" },
	{ 0x10DE0041, "GeForce 6800" },
	{ 0x10DE0042, "GeForce 6800 LE" },
	{ 0x10DE0043, "GeForce 6800 XE" },
	{ 0x10DE0044, "GeForce 6800 XT" },
	{ 0x10DE0045, "GeForce 6800 GT" },
	{ 0x10DE0046, "GeForce 6800 GT" },
	{ 0x10DE0047, "GeForce 6800 GS" },
	{ 0x10DE0048, "GeForce 6800 XT" },
	{ 0x10DE004D, "Quadro FX 3400" },
	{ 0x10DE004E, "Quadro FX 4000" },
	// 0050 - 005F
	// 0060 - 006F	
	// 0070 - 007F	
	// 0080 - 008F	
	// 0090 - 009F	
	{ 0x10DE0090, "GeForce 7800 GTX" },
	{ 0x10DE0091, "GeForce 7800 GTX" },
	{ 0x10DE0092, "GeForce 7800 GT" },
	{ 0x10DE0093, "GeForce 7800 GS" },
	{ 0x10DE0095, "GeForce 7800 SLI" },
	{ 0x10DE0098, "GeForce Go 7800" },
	{ 0x10DE0099, "GeForce Go 7800 GTX" },
	{ 0x10DE009D, "Quadro FX 4500" },
	// 00A0 - 00AF	
	// 00B0 - 00BF	
	// 00C0 - 00CF	
	{ 0x10DE00C0, "GeForce 6800 GS" },
	{ 0x10DE00C1, "GeForce 6800" },
	{ 0x10DE00C2, "GeForce 6800 LE" },
	{ 0x10DE00C3, "GeForce 6800 XT" },
	{ 0x10DE00C8, "GeForce Go 6800" },
	{ 0x10DE00C9, "GeForce Go 6800 Ultra" },
	{ 0x10DE00CC, "Quadro FX Go1400" },
	{ 0x10DE00CD, "Quadro FX 3450/4000 SDI" },
	{ 0x10DE00CE, "Quadro FX 1400" },
	// 00D0 - 00DF	
	// 00E0 - 00EF	
	// 00F0 - 00FF	
	{ 0x10DE00F1, "GeForce 6600 GT" },
	{ 0x10DE00F2, "GeForce 6600" },
	{ 0x10DE00F3, "GeForce 6200" },
	{ 0x10DE00F4, "GeForce 6600 LE" },
	{ 0x10DE00F5, "GeForce 7800 GS" },
	{ 0x10DE00F6, "GeForce 6800 GS/XT" },
	{ 0x10DE00F8, "Quadro FX 3400/4400" },
	{ 0x10DE00F9, "GeForce 6800 Series GPU" },
	// 0100 - 010F	
	// 0110 - 011F	
	// 0120 - 012F	
	// 0130 - 013F	
	// 0140 - 014F	
	{ 0x10DE0140, "GeForce 6600 GT" },
	{ 0x10DE0141, "GeForce 6600" },
	{ 0x10DE0142, "GeForce 6600 LE" },
	{ 0x10DE0143, "GeForce 6600 VE" },
	{ 0x10DE0144, "GeForce Go 6600" },
	{ 0x10DE0145, "GeForce 6610 XL" },
	{ 0x10DE0146, "GeForce Go 6600 TE/6200 TE" },
	{ 0x10DE0147, "GeForce 6700 XL" },
	{ 0x10DE0148, "GeForce Go 6600" },
	{ 0x10DE0149, "GeForce Go 6600 GT" },
	{ 0x10DE014A, "Quadro NVS 440" },
	{ 0x10DE014C, "Quadro FX 550" },
	{ 0x10DE014D, "Quadro FX 550" },
	{ 0x10DE014E, "Quadro FX 540" },
	{ 0x10DE014F, "GeForce 6200" },
	// 0150 - 015F	
	// 0160 - 016F	
	{ 0x10DE0160, "GeForce 6500" },
	{ 0x10DE0161, "GeForce 6200 TurboCache(TM)" },
	{ 0x10DE0162, "GeForce 6200SE TurboCache(TM)" },
	{ 0x10DE0163, "GeForce 6200 LE" },
	{ 0x10DE0164, "GeForce Go 6200" },
	{ 0x10DE0165, "Quadro NVS 285" },
	{ 0x10DE0166, "GeForce Go 6400" },
	{ 0x10DE0167, "GeForce Go 6200" },
	{ 0x10DE0168, "GeForce Go 6400" },
	{ 0x10DE0169, "GeForce 6250" },
	{ 0x10DE016A, "GeForce 7100 GS" },
	// 0170 - 017F	
	// 0180 - 018F	
	// 0190 - 019F		
	{ 0x10DE0191, "GeForce 8800 GTX" },
	{ 0x10DE0193, "GeForce 8800 GTS" },
	{ 0x10DE0194, "GeForce 8800 Ultra" },
	{ 0x10DE0197, "Tesla C870" },
	{ 0x10DE019D, "Quadro FX 5600" },
	{ 0x10DE019E, "Quadro FX 4600" },
	// 01A0 - 01AF	
	// 01B0 - 01BF	
	// 01C0 - 01CF	
	// 01D0 - 01DF		
	{ 0x10DE01D0, "GeForce 7350 LE" },
	{ 0x10DE01D1, "GeForce 7300 LE" },
	{ 0x10DE01D2, "GeForce 7550 LE" },
	{ 0x10DE01D3, "GeForce 7300 SE/7200 GS" },
	{ 0x10DE01D6, "GeForce Go 7200" },
	{ 0x10DE01D7, "GeForce Go 7300" },
	{ 0x10DE01D8, "GeForce Go 7400" },
	{ 0x10DE01D9, "GeForce Go 7400 GS" },
	{ 0x10DE01DA, "Quadro NVS 110M" },
	{ 0x10DE01DB, "Quadro NVS 120M" },
	{ 0x10DE01DC, "Quadro FX 350M" },
	{ 0x10DE01DD, "GeForce 7500 LE" },
	{ 0x10DE01DE, "Quadro FX 350" },
	{ 0x10DE01DF, "GeForce 7300 GS" },
	// 01E0 - 01EF	
	// 01F0 - 01FF
    { 0x10DE01F0,	"GeForce4 MX" },
	// 0200 - 020F	
	// 0210 - 021F	
	{ 0x10DE0211, "GeForce 6800" },
	{ 0x10DE0212, "GeForce 6800 LE" },
	{ 0x10DE0215, "GeForce 6800 GT" },
	{ 0x10DE0218, "GeForce 6800 XT" },
	// 0220 - 022F	
	{ 0x10DE0221, "GeForce 6200" },
	{ 0x10DE0222, "GeForce 6200 A-LE" },
    { 0x10DE0228,	"NVIDIA NV44M" },
	// 0230 - 023F	
	// 0240 - 024F	
	{ 0x10DE0240, "GeForce 6150" },
	{ 0x10DE0241, "GeForce 6150 LE" },
	{ 0x10DE0242, "GeForce 6100" },
   	{ 0x10DE0243,	"NVIDIA C51" },
	{ 0x10DE0244, "GeForce Go 6150" },
	{ 0x10DE0245, "Quadro NVS 210S / GeForce 6150LE" },
	{ 0x10DE0247, "GeForce Go 6100" },
	// 0250 - 025F	
	// 0260 - 026F	
	// 0270 - 027F	
	// 0280 - 028F		
	// 0290 - 029F	
	{ 0x10DE0290, "GeForce 7900 GTX" },
	{ 0x10DE0291, "GeForce 7900 GT/GTO" },
	{ 0x10DE0292, "GeForce 7900 GS" },
	{ 0x10DE0293, "GeForce 7950 GX2" },
	{ 0x10DE0294, "GeForce 7950 GX2" },
	{ 0x10DE0295, "GeForce 7950 GT" },
	{ 0x10DE0298, "GeForce Go 7900 GS" },
	{ 0x10DE0299, "GeForce Go 7900 GTX" },
	{ 0x10DE029A, "Quadro FX 2500M" },
	{ 0x10DE029B, "Quadro FX 1500M" },
	{ 0x10DE029C, "Quadro FX 5500" },
	{ 0x10DE029D, "Quadro FX 3500" },
	{ 0x10DE029E, "Quadro FX 1500" },
	{ 0x10DE029F, "Quadro FX 4500 X2" },
	// 02A0 - 02AF	
	// 02B0 - 02BF	
	// 02C0 - 02CF	
	// 02D0 - 02DF		
	// 02E0 - 02EF		
	{ 0x10DE02E0, "GeForce 7600 GT" },
	{ 0x10DE02E1, "GeForce 7600 GS" },
	{ 0x10DE02E2, "GeForce 7300 GT" },
	{ 0x10DE02E3, "GeForce 7900 GS" },
	{ 0x10DE02E4, "GeForce 7950 GT" },
	// 02F0 - 02FF		
	// 0300 - 030F		
	{ 0x10DE0301, "GeForce FX 5800 Ultra" },
	{ 0x10DE0302, "GeForce FX 5800" },
	{ 0x10DE0308, "Quadro FX 2000" },
	{ 0x10DE0309, "Quadro FX 1000" },
	// 0310 - 031F		
	{ 0x10DE0311, "GeForce FX 5600 Ultra" },
	{ 0x10DE0312, "GeForce FX 5600" },
	{ 0x10DE0314, "GeForce FX 5600XT" },
	{ 0x10DE031A, "GeForce FX Go5600" },
	{ 0x10DE031B, "GeForce FX Go5650" },
	{ 0x10DE031C, "Quadro FX Go700" },
	// 0320 - 032F
    { 0x10DE0320, "GeForce FX 5200" },
	{ 0x10DE0321, "GeForce FX 5200 Ultra" },
	{ 0x10DE0322, "GeForce FX 5200" },
	{ 0x10DE0323, "GeForce FX 5200 LE" },
	{ 0x10DE0324, "GeForce FX Go5200" },
	{ 0x10DE0325, "GeForce FX Go5250" },
	{ 0x10DE0326, "GeForce FX 5500" },
	{ 0x10DE0328, "GeForce FX Go5200 32M/64M" },
   	{ 0x10DE0329, "GeForce FX Go5200" },
	{ 0x10DE032A, "Quadro NVS 55/280 PCI" },
	{ 0x10DE032B, "Quadro FX 500/600 PCI" },
	{ 0x10DE032C, "GeForce FX Go53xx Series" },
	{ 0x10DE032D, "GeForce FX Go5100" },
	// 0330 - 033F
	{ 0x10DE0330, "GeForce FX 5900 Ultra" },
	{ 0x10DE0331, "GeForce FX 5900" },
	{ 0x10DE0332, "GeForce FX 5900XT" },
	{ 0x10DE0333, "GeForce FX 5950 Ultra" },
	{ 0x10DE0334, "GeForce FX 5900ZT" },
	{ 0x10DE0338, "Quadro FX 3000" },
	{ 0x10DE033F, "Quadro FX 700" },
	// 0340 - 034F
	{ 0x10DE0341, "GeForce FX 5700 Ultra" },
	{ 0x10DE0342, "GeForce FX 5700" },
	{ 0x10DE0343, "GeForce FX 5700LE" },
	{ 0x10DE0344, "GeForce FX 5700VE" },
	{ 0x10DE0347, "GeForce FX Go5700" },
	{ 0x10DE0348, "GeForce FX Go5700" },
	{ 0x10DE034C, "Quadro FX Go1000" },
	{ 0x10DE034E, "Quadro FX 1100" },
	// 0350 - 035F	
	// 0360 - 036F	
	// 0370 - 037F	
	// 0380 - 038F			
	{ 0x10DE038B, "GeForce 7650 GS" },
	// 0390 - 039F
	{ 0x10DE0390, "GeForce 7650 GS" },
	{ 0x10DE0391, "GeForce 7600 GT" },
	{ 0x10DE0392, "GeForce 7600 GS" },
	{ 0x10DE0393, "GeForce 7300 GT" },
	{ 0x10DE0394, "GeForce 7600 LE" },
	{ 0x10DE0395, "GeForce 7300 GT" },
	{ 0x10DE0397, "GeForce Go 7700" },
	{ 0x10DE0398, "GeForce Go 7600" },
	{ 0x10DE0399, "GeForce Go 7600 GT"},
	{ 0x10DE039A, "Quadro NVS 300M" },
	{ 0x10DE039B, "GeForce Go 7900 SE" },
	{ 0x10DE039C, "Quadro FX 550M" },
	{ 0x10DE039E, "Quadro FX 560" },
	// 03A0 - 03AF	
	// 03B0 - 03BF	
	// 03C0 - 03CF	
	// 03D0 - 03DF			
	{ 0x10DE03D0, "GeForce 6150SE nForce 430" },
	{ 0x10DE03D1, "GeForce 6100 nForce 405" },
	{ 0x10DE03D2, "GeForce 6100 nForce 400" },
	{ 0x10DE03D5, "GeForce 6100 nForce 420" },
	{ 0x10DE03D6, "GeForce 7025 / nForce 630a" },
	// 03E0 - 03EF
	// 03F0 - 03FF
	// 0400 - 040F		
	{ 0x10DE0400, "GeForce 8600 GTS" },
	{ 0x10DE0401, "GeForce 8600 GT" },
	{ 0x10DE0402, "GeForce 8600 GT" },
	{ 0x10DE0403, "GeForce 8600 GS" },
	{ 0x10DE0404, "GeForce 8400 GS" },
	{ 0x10DE0405, "GeForce 9500M GS" },
	{ 0x10DE0406, "GeForce 8300 GS" },
	{ 0x10DE0407, "GeForce 8600M GT" },
	{ 0x10DE0408, "GeForce 9650M GS" },
	{ 0x10DE0409, "GeForce 8700M GT" },
	{ 0x10DE040A, "Quadro FX 370" },
	{ 0x10DE040B, "Quadro NVS 320M" },
	{ 0x10DE040C, "Quadro FX 570M" },
	{ 0x10DE040D, "Quadro FX 1600M" },
	{ 0x10DE040E, "Quadro FX 570" },
	{ 0x10DE040F, "Quadro FX 1700" },
	// 0410 - 041F	
	{ 0x10DE0410, "GeForce GT 330" },
	// 0420 - 042F	
	{ 0x10DE0420, "GeForce 8400 SE" },
	{ 0x10DE0421, "GeForce 8500 GT" },
	{ 0x10DE0422, "GeForce 8400 GS" },
	{ 0x10DE0423, "GeForce 8300 GS" },
	{ 0x10DE0424, "GeForce 8400 GS" },
	{ 0x10DE0425, "GeForce 8600M GS" },
	{ 0x10DE0426, "GeForce 8400M GT" },
	{ 0x10DE0427, "GeForce 8400M GS" },
	{ 0x10DE0428, "GeForce 8400M G" },
	{ 0x10DE0429, "Quadro NVS 140M" },
	{ 0x10DE042A, "Quadro NVS 130M" },
	{ 0x10DE042B, "Quadro NVS 135M" },
	{ 0x10DE042C, "GeForce 9400 GT" },
	{ 0x10DE042D, "Quadro FX 360M" },
	{ 0x10DE042E, "GeForce 9300M G" },
	{ 0x10DE042F, "Quadro NVS 290" },
	// 0430 - 043F
	// 0440 - 044F
	// 0450 - 045F
	// 0460 - 046F
	// 0470 - 047F
	// 0480 - 048F
	// 0490 - 049F
	// 04A0 - 04AF
	// 04B0 - 04BF
	// 04C0 - 04CF
    { 0x10DE04C0,	"NVIDIA G78" },
	{ 0x10DE04C1,	"NVIDIA G78" },
	{ 0x10DE04C2,	"NVIDIA G78" },
	{ 0x10DE04C3,	"NVIDIA G78" },
	{ 0x10DE04C4,	"NVIDIA G78" },
	{ 0x10DE04C5,	"NVIDIA G78" },
	{ 0x10DE04C6,	"NVIDIA G78" },
	{ 0x10DE04C7,	"NVIDIA G78" },
	{ 0x10DE04C8,	"NVIDIA G78" },
	{ 0x10DE04C9,	"NVIDIA G78" },
	{ 0x10DE04CA,	"NVIDIA G78" },
	{ 0x10DE04CB,	"NVIDIA G78" },
	{ 0x10DE04CC,	"NVIDIA G78" },
	{ 0x10DE04CD,	"NVIDIA G78" },
	{ 0x10DE04CE,	"NVIDIA G78" },
	{ 0x10DE04CF,	"NVIDIA G78" },
	// 04D0 - 04DF
	// 04E0 - 04EF
	// 04F0 - 04FF
	// 0500 - 050F	
	// 0510 - 051F	
	// 0520 - 052F	
	// 0530 - 053F	
	{ 0x10DE0530,	"GeForce 7190M / nForce 650M" },
	{ 0x10DE0531,	"GeForce 7150M / nForce 630M" },
	{ 0x10DE0533,	"GeForce 7000M / nForce 610M" },
	{ 0x10DE053A,	"GeForce 7050 PV / nForce 630a" },
	{ 0x10DE053B,	"GeForce 7050 PV / nForce 630a" },
	{ 0x10DE053E,	"GeForce 7025 / nForce 630a" },
	// 0540 - 054F
	// 0550 - 055F
	// 0560 - 056F
	// 0570 - 057F
	// 0580 - 058F
	// 0590 - 059F
	// 05A0 - 05AF
	// 05B0 - 05BF
	// 05C0 - 05CF
	// 05D0 - 05DF
	// 05E0 - 05EF	
	{ 0x10DE05E0,	"GeForce GTX 295" },
	{ 0x10DE05E1,	"GeForce GTX 280" },
	{ 0x10DE05E2,	"GeForce GTX 260" },
	{ 0x10DE05E3,	"GeForce GTX 285" },
	{ 0x10DE05E4,	"NVIDIA GT200" },
	{ 0x10DE05E5,	"NVIDIA GT200" },
	{ 0x10DE05E6,	"GeForce GTX 275" },
	{ 0x10DE05E7,	"Tesla C1060" },
	{ 0x10DE05E8,	"NVIDIA GT200" },
	{ 0x10DE05E9,	"NVIDIA GT200" },
	{ 0x10DE05EA,	"GeForce GTX 260" },
	{ 0x10DE05EB,	"GeForce GTX 295" },
	{ 0x10DE05EC,	"NVIDIA GT200" },
	{ 0x10DE05ED,	"Quadroplex 2200 D2" },
	{ 0x10DE05EE,	"NVIDIA GT200" },
	{ 0x10DE05EF,	"NVIDIA GT200" },
	// 05F0 - 05FF		
	{ 0x10DE05F0,	"NVIDIA GT200" },
	{ 0x10DE05F1,	"NVIDIA GT200" },
	{ 0x10DE05F2,	"NVIDIA GT200" },
	{ 0x10DE05F3,	"NVIDIA GT200" },
	{ 0x10DE05F4,	"NVIDIA GT200" },
	{ 0x10DE05F5,	"NVIDIA GT200" },
	{ 0x10DE05F6,	"NVIDIA GT200" },
	{ 0x10DE05F7,	"NVIDIA GT200" },
	{ 0x10DE05F8,	"Quadroplex 2200 S4" },
	{ 0x10DE05F9,	"Quadro CX" },
	{ 0x10DE05FA,	"NVIDIA GT200" },
	{ 0x10DE05FB,	"NVIDIA GT200" },
	{ 0x10DE05FC,	"NVIDIA GT200" },
	{ 0x10DE05FD,	"Quadro FX 5800" },
	{ 0x10DE05FE,	"Quadro FX 4800" },
	{ 0x10DE05FF,	"Quadro FX 3800" },
	// 0600 - 060F	
	{ 0x10DE0600, "GeForce 8800 GTS 512" },
	{ 0x10DE0601, "GeForce 9800 GT" },
	{ 0x10DE0602, "GeForce 8800 GT" },
	{ 0x10DE0603, "GeForce GT 230" },
	{ 0x10DE0604, "GeForce 9800 GX2" },
	{ 0x10DE0605, "GeForce 9800 GT" },
	{ 0x10DE0606, "GeForce 8800 GS" },
	{ 0x10DE0607, "GeForce GTS 240" },
	{ 0x10DE0608, "GeForce 9800M GTX" },
	{ 0x10DE0609, "GeForce 8800M GTS" },
	{ 0x10DE060A, "GeForce GTX 280M" },
	{ 0x10DE060B, "GeForce 9800M GT" },
	{ 0x10DE060C, "GeForce 8800M GTX" },
	{ 0x10DE060D, "GeForce 8800 GS" },
	{ 0x10DE060F, "GeForce GTX 285M" },
	// 0610 - 061F	
	{ 0x10DE0610, "GeForce 9600 GSO" },
	{ 0x10DE0611, "GeForce 8800 GT" },
	{ 0x10DE0612, "GeForce 9800 GTX" },
	{ 0x10DE0613, "GeForce 9800 GTX+" },
	{ 0x10DE0614, "GeForce 9800 GT" },
	{ 0x10DE0615, "GeForce GTS 250" },
	{ 0x10DE0617, "GeForce 9800M GTX" },
	{ 0x10DE0618, "GeForce GTX 260M" },
	{ 0x10DE0619, "Quadro FX 4700 X2" },
	{ 0x10DE061A, "Quadro FX 3700" },
	{ 0x10DE061B, "Quadro VX 200" },
	{ 0x10DE061C, "Quadro FX 3600M" },
	{ 0x10DE061D, "Quadro FX 2800M" },
	{ 0x10DE061E, "Quadro FX 3700M" }, 
	{ 0x10DE061F, "Quadro FX 3800M" },
	// 0620 - 062F	
	{ 0x10DE0620,	"NVIDIA G94" }, // GeForce 8100/8200/8300
	{ 0x10DE0621,	"GeForce GT 230" },
	{ 0x10DE0622,	"GeForce 9600 GT" },
	{ 0x10DE0623,	"GeForce 9600 GS" },
	{ 0x10DE0624,	"NVIDIA G94" },
	{ 0x10DE0625,	"GeForce 9600 GSO 512"},
	{ 0x10DE0626,	"GeForce GT 130" },
	{ 0x10DE0627,	"GeForce GT 140" },
	{ 0x10DE0628,	"GeForce 9800M GTS" },
	{ 0x10DE0629,	"NVIDIA G94" },
	{ 0x10DE062A,	"GeForce 9700M GTS" },
	{ 0x10DE062B,	"GeForce 9800M GS" },
	{ 0x10DE062C,	"GeForce 9800M GTS" },
	{ 0x10DE062D,	"GeForce 9600 GT" },
	{ 0x10DE062E,	"GeForce 9600 GT" },
	{ 0x10DE062F,	"GeForce 9800 S" },
	// 0630 - 063F	
	{ 0x10DE0630,	"NVIDIA G94" },
	{ 0x10DE0631,	"GeForce GTS 160M" },
	{ 0x10DE0632,	"GeForce GTS 150M" },
	{ 0x10DE0633,	"NVIDIA G94" },
	{ 0x10DE0634,	"NVIDIA G94" },
	{ 0x10DE0635,	"GeForce 9600 GSO" },
	{ 0x10DE0636,	"NVIDIA G94" },
	{ 0x10DE0637,	"GeForce 9600 GT" },
	{ 0x10DE0638,	"Quadro FX 1800" },
	{ 0x10DE0639,	"NVIDIA G94" },
	{ 0x10DE063A,	"Quadro FX 2700M" },
	{ 0x10DE063B,	"NVIDIA G94" },
	{ 0x10DE063C,	"NVIDIA G94" },
	{ 0x10DE063D,	"NVIDIA G94" },
	{ 0x10DE063E,	"NVIDIA G94" },
	{ 0x10DE063F,	"NVIDIA G94" },
	// 0640 - 064F	
	{ 0x10DE0640, "GeForce 9500 GT" },
	{ 0x10DE0641, "GeForce 9400 GT" },
	{ 0x10DE0642, "GeForce 8400 GS" },
	{ 0x10DE0643, "GeForce 9500 GT" },
	{ 0x10DE0644, "GeForce 9500 GS" },
	{ 0x10DE0645, "GeForce 9500 GS" },
	{ 0x10DE0646, "GeForce GT 120" },
	{ 0x10DE0647, "GeForce 9600M GT" },
	{ 0x10DE0648, "GeForce 9600M GS" },
	{ 0x10DE0649, "GeForce 9600M GT" },
	{ 0x10DE064A, "GeForce 9700M GT" },
	{ 0x10DE064B, "GeForce 9500M G" },
	{ 0x10DE064C, "GeForce 9650M GT" },
	// 0650 - 065F		
	{ 0x10DE0650,	"NVIDIA G96-825" },
	{ 0x10DE0651,	"GeForce G 110M" },
	{ 0x10DE0652,	"GeForce GT 130M" },
	{ 0x10DE0653,	"GeForce GT 120M" },
	{ 0x10DE0654,	"GeForce GT 220M" },
	{ 0x10DE0655,	"GeForce GT 120" },
	{ 0x10DE0656,	"GeForce 9650 S" },
	{ 0x10DE0657,	"NVIDIA G96" },
	{ 0x10DE0658,	"Quadro FX 380" },
	{ 0x10DE0659,	"Quadro FX 580" },
	{ 0x10DE065A,	"Quadro FX 1700M" },
	{ 0x10DE065B,	"GeForce 9400 GT" },
	{ 0x10DE065C,	"Quadro FX 770M" },
	{ 0x10DE065D,	"NVIDIA G96" },
	{ 0x10DE065E,	"NVIDIA G96" },
	{ 0x10DE065F,	"GeForce G210" },
	// 0660 - 066F
	// 0670 - 067F
	// 0680 - 068F
	// 0690 - 069F
	// 06A0 - 06AF
	{ 0x10DE06A0,	"NVIDIA GT214" },
	// 06B0 - 06BF
	{ 0x10DE06B0,	"NVIDIA GT214" },
	// 06C0 - 06CF
	{ 0x10DE06C0, "GeForce GTX 480" },
	{ 0x10DE06C3, "GeForce GTX D12U" },
	{ 0x10DE06C4, "GeForce GTX 465" },
	{ 0x10DE06CA, "GeForce GTX 480M" },
	{ 0x10DE06CD, "GeForce GTX 470" },
	// 06D0 - 06DF
	{ 0x10DE06D1,	"Tesla C2050 / C2070" },
	// { 0x10DE06D1,	0x10DE0771,	"Tesla C2050" },
	// { 0x10DE06D1,	0x10DE0772,	"Tesla C2070" },
	{ 0x10DE06D2,	"Tesla M2070 / X2070" },
	{ 0x10DE06D8, "Quadro 6000" },
	{ 0x10DE06D9, "Quadro 5000" },
	{ 0x10DE06DA, "Quadro 5000M" },
	{ 0x10DE06DC, "Quadro 6000" },
	{ 0x10DE06DD, "Quadro 4000" },
	{ 0x10DE06DE, "Tesla M2050" },	// TODO: sub-device id: 0x0846
	{ 0x10DE06DE, "Tesla M2070" },	// TODO: sub-device id: ?	
	{ 0x10DE06DF, "Tesla M2070-Q" }, 
	// 0x10DE06DE also applies to misc S2050, X2070, M2050, M2070
	// 06E0 - 06EF	
	{ 0x10DE06E0, "GeForce 9300 GE" },
	{ 0x10DE06E1, "GeForce 9300 GS" },
	{ 0x10DE06E2, "GeForce 8400" },
	{ 0x10DE06E3, "GeForce 8400 SE" },
	{ 0x10DE06E4, "GeForce 8400 GS" },
	{ 0x10DE06E5, "GeForce 9300M GS" },
	{ 0x10DE06E6, "GeForce G100" },
	{ 0x10DE06E7, "GeForce 9300 SE" },
	{ 0x10DE06E8, "GeForce 9200M GS" },
	{ 0x10DE06E9, "GeForce 9300M GS" },
	{ 0x10DE06EA, "Quadro NVS 150M" },
	{ 0x10DE06EB, "Quadro NVS 160M" },
	{ 0x10DE06EC, "GeForce G 105M" },
	{ 0x10DE06EF, "GeForce G 103M" },
	// 06F0 - 06FF	
	{ 0x10DE06F0,	"NVIDIA G98" },
	{ 0x10DE06F1,	"GeForce G105M" },
	{ 0x10DE06F2,	"NVIDIA G98" },
	{ 0x10DE06F3,	"NVIDIA G98" },
	{ 0x10DE06F4,	"NVIDIA G98" },
	{ 0x10DE06F5,	"NVIDIA G98" },
	{ 0x10DE06F6,	"NVIDIA G98" },
	{ 0x10DE06F7,	"NVIDIA G98" },
	{ 0x10DE06F8,	"Quadro NVS 420" },
	{ 0x10DE06F9,	"Quadro FX 370 LP" },
	{ 0x10DE06FA,	"Quadro NVS 450" },
	{ 0x10DE06FB,	"Quadro FX 370M" },
	{ 0x10DE06FC,	"NVIDIA G98" },
	{ 0x10DE06FD,	"Quadro NVS 295" },
	{ 0x10DE06FE,	"NVIDIA G98" },
	{ 0x10DE06FF,	"HICx16 + Graphics" },
	// 0700 - 070F
	// 0710 - 071F
	// 0720 - 072F
	// 0730 - 073F
	// 0740 - 074F
	// 0750 - 075F
	// 0760 - 076F
	// 0770 - 077F
	// 0780 - 078F
	// 0790 - 079F
	// 07A0 - 07AF
	// 07B0 - 07BF
	// 07C0 - 07CF
	// 07D0 - 07DF	
	// 07E0 - 07EF		
	{ 0x10DE07E0, "GeForce 7150 / nForce 630i" },
	{ 0x10DE07E1, "GeForce 7100 / nForce 630i" },
	{ 0x10DE07E2, "GeForce 7050 / nForce 630i" },
	{ 0x10DE07E3, "GeForce 7050 / nForce 610i" },
	{ 0x10DE07E5, "GeForce 7050 / nForce 620i" },
	// 07F0 - 07FF	
	// 0800 - 080F
	// 0810 - 081F
	// 0820 - 082F
	// 0830 - 083F
	// 0840 - 084F
	{ 0x10DE0840, "GeForce 8200M" }, 
	{ 0x10DE0844, "GeForce 9100M G" },
	{ 0x10DE0845, "GeForce 8200M G" },
	{ 0x10DE0846, "GeForce 9200" },
	{ 0x10DE0847, "GeForce 9100" },
	{ 0x10DE0848, "GeForce 8300" },
	{ 0x10DE0849, "GeForce 8200" },
	{ 0x10DE084A, "nForce 730a" },
	{ 0x10DE084B, "GeForce 9200" },
	{ 0x10DE084C, "nForce 980a/780a SLI" },
	{ 0x10DE084D, "nForce 750a SLI" },
	{ 0x10DE084F, "GeForce 8100 / nForce 720a" },
	// 0850 - 085F
	// 0860 - 086F	
	{ 0x10DE0860, "GeForce 9400" },
	{ 0x10DE0861, "GeForce 9400" },
	{ 0x10DE0862, "GeForce 9400M G" },
	{ 0x10DE0863, "GeForce 9400M" },
	{ 0x10DE0864, "GeForce 9300" },
	{ 0x10DE0865, "ION" },
	{ 0x10DE0866, "GeForce 9400M G" },
	{ 0x10DE0867, "GeForce 9400" },
	{ 0x10DE0868, "nForce 760i SLI" },
	{ 0x10DE0869, "GeForce 9400" }, 
	{ 0x10DE086A, "GeForce 9400" },
	{ 0x10DE086C, "GeForce 9300 / nForce 730i" },
	{ 0x10DE086D, "GeForce 9200" },
	{ 0x10DE086E, "GeForce 9100M G" },
	{ 0x10DE086F, "GeForce 8200M G" },
	// 0870 - 087F	
	{ 0x10DE0870, "GeForce 9400M" },
	{ 0x10DE0871, "GeForce 9200" },
	{ 0x10DE0872, "GeForce G102M" },
	{ 0x10DE0873, "GeForce G102M" },
	{ 0x10DE0874, "ION 9300M" },	
	{ 0x10DE0876, "ION" },
	{ 0x10DE087A, "GeForce 9400" },
	{ 0x10DE087D, "ION 9400M" },
	{ 0x10DE087E, "ION LE" },
	{ 0x10DE087F, "ION LE" },
	// 0880 - 088F
	// 0890 - 089F
	// 08A0 - 08AF
	{ 0x10DE08A0,	"GeForce 320M" },
	{ 0x10DE08A3,	"GeForce 320M" },
	{ 0x10DE08A4,	"GeForce 320M" },
	{ 0x10DE08A5,	"GeForce 320M" },
	// 08B0 - 08BF
	{ 0x10DE08B1,	"GeForce 300M" },
	// 08C0 - 08CF
	// 08D0 - 08DF	
	// 08E0 - 08EF			
	// 08F0 - 08FF
	// 0900 - 090F
	// 0910 - 091F
	// 0920 - 092F
	// 0930 - 093F
	// 0940 - 094F
	// 0950 - 095F
	// 0960 - 096F
	// 0970 - 097F
	// 0980 - 098F
	// 0990 - 099F
	// 09A0 - 09AF
	// 09B0 - 09BF
	// 09C0 - 09CF
	// 09D0 - 09DF	
	// 09E0 - 09EF
	// 09F0 - 09FF
	// 0A00 - 0A0F
	// 0A10 - 0A1F
	// 0A20 - 0A2F
	{ 0x10DE0A20, "GeForce GT220" },
	{ 0x10DE0A22, "GeForce 315" },
	{ 0x10DE0A23, "GeForce 210" },
	{ 0x10DE0A26, "GeForce 405" }, 
	{ 0x10DE0A27, "GeForce 405" }, 
	{ 0x10DE0A28, "GeForce GT 230M" },
	{ 0x10DE0A29, "GeForce GT 330M" },
	{ 0x10DE0A2A, "GeForce GT 230M" },
	{ 0x10DE0A2B, "GeForce GT 330M" },
	{ 0x10DE0A2C, "NVS 5100M" },
	{ 0x10DE0A2D, "GeForce GT 320M" },	
	// 0A30 - 0A3F	
	{ 0x10DE0A32,	"GeForce GT 415" },
	{ 0x10DE0A34,	"GeForce GT 240M" },
	{ 0x10DE0A35,	"GeForce GT 325M" },
	{ 0x10DE0A38,	"Quadro 400" },
	{ 0x10DE0A3C,	"Quadro FX 880M" },
	// 0A40 - 0A4F
	// 0A50 - 0A5F
	// 0A60 - 0A6F
	{ 0x10DE0A60, "GeForce G210" },
	{ 0x10DE0A62, "GeForce 205" },
	{ 0x10DE0A63, "GeForce 310" },
	{ 0x10DE0A64, "ION" },
	{ 0x10DE0A65, "GeForce 210" },
	{ 0x10DE0A66, "GeForce 310" },
	{ 0x10DE0A67, "GeForce 315" },
	{ 0x10DE0A68, "GeForce G105M" },
	{ 0x10DE0A69, "GeForce G105M" },
	{ 0x10DE0A6A, "NVS 2100M" },
	{ 0x10DE0A6C, "NVS 3100M" },
	{ 0x10DE0A6E, "GeForce 305M" },
	{ 0x10DE0A6F, "ION" },	
	// 0A70 - 0A7F
	{ 0x10DE0A70, "GeForce 310M" },
	{ 0x10DE0A71, "GeForce 305M" },
	{ 0x10DE0A72, "GeForce 310M" },
	{ 0x10DE0A73, "GeForce 305M" },
	{ 0x10DE0A74, "GeForce G210M" },
	{ 0x10DE0A75, "GeForce G310M" },
	{ 0x10DE0A76, "ION" }, 
	{ 0x10DE0A78, "Quadro FX 380 LP" },
	{ 0x10DE0A7A, "GeForce 315M" },
   	{ 0x10DE0A7B,	"GeForce 505" },
	{ 0x10DE0A7C, "Quadro FX 380M" },
	// 0A80 - 0A8F
	// 0A90 - 0A9F
	// 0AA0 - 0AAF
	// 0AB0 - 0ABF
	// 0AC0 - 0ACF
	// 0AD0 - 0ADF	
	// 0AE0 - 0AEF
	// 0AF0 - 0AFF
	// 0B00 - 0B0F
	// 0B10 - 0B1F
	// 0B20 - 0B2F
	// 0B30 - 0B3F
	// 0B40 - 0B4F
	// 0B50 - 0B5F
	// 0B60 - 0B6F
	// 0B70 - 0B7F
	// 0B80 - 0B8F
	// 0B90 - 0B9F
	// 0BA0 - 0BAF
	// 0BB0 - 0BBF
	// 0BC0 - 0BCF
	// 0BD0 - 0BDF	
	// 0BE0 - 0BEF
	// 0BF0 - 0BFF
	// 0C00 - 0C0F
	// 0C10 - 0C1F
	// 0C20 - 0C2F
	// 0C30 - 0C3F
	// 0C40 - 0C4F
	// 0C50 - 0C5F
	// 0C60 - 0C6F
	// 0C70 - 0C7F
	// 0C80 - 0C8F
	// 0C90 - 0C9F
	// 0CA0 - 0CAF
	{ 0x10DE0CA0, "GeForce GT 330 " },
	{ 0x10DE0CA2, "GeForce GT 320" },
	{ 0x10DE0CA3, "GeForce GT 240" },
	{ 0x10DE0CA4, "GeForce GT 340" },
	{ 0x10DE0CA5, "GeForce GT 220" }, 
	{ 0x10DE0CA7, "GeForce GT 330" },
	{ 0x10DE0CA8, "GeForce GTS 260M" },
	{ 0x10DE0CA9, "GeForce GTS 250M" },
	{ 0x10DE0CAC, "GeForce GT 220" }, 
	{ 0x10DE0CAF, "GeForce GT 335M" },
	// 0CB0 - 0CBF	
	{ 0x10DE0CB0, "GeForce GTS 350M" },
	{ 0x10DE0CB1, "GeForce GTS 360M" },
	{ 0x10DE0CBC, "Quadro FX 1800M" },
	// 0CC0 - 0CCF
	// 0CD0 - 0CDF	
	// 0CE0 - 0CEF
	// 0CF0 - 0CFF
	// 0D00 - 0D0F
	// 0D10 - 0D1F
	// 0D20 - 0D2F
	// 0D30 - 0D3F
	// 0D40 - 0D4F
	// 0D50 - 0D5F
	// 0D60 - 0D6F
	// 0D70 - 0D7F
	// 0D80 - 0D8F
	// 0D90 - 0D9F
	// 0DA0 - 0DAF	
	// 0DB0 - 0DBF
	// 0DC0 - 0DCF
	{ 0x10DE0DC0, "GeForce GT 440" },
	{ 0x10DE0DC1, "D12-P1-35" },
	{ 0x10DE0DC2, "D12-P1-35" },
	{ 0x10DE0DC4, "GeForce GTS 450" },
	{ 0x10DE0DC5, "GeForce GTS 450" },
	{ 0x10DE0DC6, "GeForce GTS 450" },
	{ 0x10DE0DCA, "GF10x" },
	{ 0x10DE0DCD, "GeForce GT 555M" }, 
	{ 0x10DE0DCE, "GeForce GT 555M" }, 
	// 0DD0 - 0DDF	
	{ 0x10DE0DD1, "GeForce GTX 460M" },
	{ 0x10DE0DD2, "GeForce GT 445M" },
	{ 0x10DE0DD3, "GeForce GT 435M" },
	{ 0x10DE0DD6, "GeForce GT 550M" }, 
	{ 0x10DE0DD8, "Quadro 2000" },
	{ 0x10DE0DDA, "Quadro 2000M" }, 
	{ 0x10DE0DDE, "GF106-ES" },
	{ 0x10DE0DDF, "GF106-INT" },
	// 0DE0 - 0DEF	
	{ 0x10DE0DE0,	"GeForce GT 440" },
	{ 0x10DE0DE1,	"GeForce GT 430" },
	{ 0x10DE0DE2,	"GeForce GT 420" },
	{ 0x10DE0DE4,	"GeForce GT 520" },
	{ 0x10DE0DE5,	"GeForce GT 530" },
	{ 0x10DE0DE8,	"GeForce GT 620M" },
	{ 0x10DE0DE9,	"GeForce GT 630M" },
	{ 0x10DE0DEA,	"GeForce GT 610M" },
	{ 0x10DE0DEB,	"GeForce GT 555M" },
	{ 0x10DE0DEC,	"GeForce GT 525M" },
	{ 0x10DE0DED,	"GeForce GT 520M" },
	{ 0x10DE0DEE,	"GeForce GT 415M" },

	// 0DF0 - 0DFF	
	{ 0x10DE0DF0, "GeForce GT 425M" },
	{ 0x10DE0DF1, "GeForce GT 420M" },
	{ 0x10DE0DF2, "GeForce GT 435M" },
	{ 0x10DE0DF3, "GeForce GT 420M" },
	{ 0x10DE0DF4, "GeForce GT 540M" }, 
	{ 0x10DE0DF5, "GeForce GT 525M" }, 
	{ 0x10DE0DF6, "GeForce GT 550M" }, 
	{ 0x10DE0DF7, "GeForce GT 520M" }, 
	{ 0x10DE0DF8, "Quadro 600" },
   	{ 0x10DE0DF9,	"Quadro 500M" },
	{ 0x10DE0DFA, "Quadro 1000M" }, 
	{ 0x10DE0DFE, "GF108 ES" },
	{ 0x10DE0DFF, "GF108 INT" },
	// 0E00 - 0E0F
	// 0E10 - 0E1F
	// 0E20 - 0E2F
	{ 0x10DE0E21, "D12U-25" },
	{ 0x10DE0E22, "GeForce GTX 460" },
	{ 0x10DE0E23, "GeForce GTX 460 SE" },
	{ 0x10DE0E24, "GeForce GTX 460" },
	{ 0x10DE0E25, "D12U-50" },
   	{ 0x10DE0E28,	"GeForce GTX 460" },
	// 0E30 - 0E3F	
	{ 0x10DE0E30, "GeForce GTX 470M" },
	{ 0x10DE0E31, "GeForce GTX 485M" }, 
	{ 0x10DE0E38, "GF104GL" },
	{ 0x10DE0E3A, "Quadro 3000M" }, 
	{ 0x10DE0E3B, "Quadro 4000M" }, 
	{ 0x10DE0E3E, "GF104-ES" },
	{ 0x10DE0E3F, "GF104-INT" },
	// 0E40 - 0E4F	
	// 0E50 - 0E5F
	// 0E60 - 0E6F
	// 0E70 - 0E7F
	// 0E80 - 0E8F
	// 0E90 - 0E9F
	// 0EA0 - 0EAF
	// 0EB0 - 0EBF
	// 0EC0 - 0ECF
	// 0ED0 - 0EDF	
	// 0EE0 - 0EEF
	// 0EF0 - 0EFF
	// 0F00 - 0F0F
	// 0F10 - 0F1F
	// 0F20 - 0F2F
	// 0F30 - 0F3F
	// 0F40 - 0F4F
	// 0F50 - 0F5F
	// 0F60 - 0F6F
	// 0F70 - 0F7F
	// 0F80 - 0F8F
	// 0F90 - 0F9F
	// 0FA0 - 0FAF
	// 0FB0 - 0FBF
	// 0FC0 - 0FCF
	// 0FD0 - 0FDF
    { 0x10DE0FD1,	"GeForce GT 650M" },
	{ 0x10DE0FD2,	"GeForce GT 640M" },
	{ 0x10DE0FD4,	"GeForce GTX 660M" },
	// 0FE0 - 0FEF
	// 0FF0 - 0FFF
	// 1000 - 100F
	// 1010 - 101F
	// 1020 - 102F
	// 1030 - 103F
	// 1040 - 104F
	{ 0x10DE1040, "GeForce GT 520" },
    { 0x10DE1042,	"GeForce 510" },
	{ 0x10DE1048,	"GeForce 605" },
	{ 0x10DE1049,	"GeForce GT 620" },
	// 1050 - 105F
	{ 0x10DE1050,	"GeForce GT 520M" },
	{ 0x10DE1051,	"GeForce GT 520MX" },
	{ 0x10DE1052,	"GeForce GT 520M" },
	{ 0x10DE1054,	"GeForce GT 410M" },
	{ 0x10DE1055,	"GeForce 410M" },
	{ 0x10DE1056,	"Quadro NVS 4200M" },
	{ 0x10DE1057,	"Quadro NVS 4200M" },
	{ 0x10DE1058,	"GeForce 610M" },
	{ 0x10DE1059,	"GeForce 610M" },
	{ 0x10DE105A,	"GeForce 610M" },
	// 1060 - 106F
	// 1070 - 107F
	{ 0x10DE107F, "NVIDIA GF119-ES" }, 
	// 1080 - 108F
	{ 0x10DE1080, "GeForce GTX 580" },
	{ 0x10DE1081, "GeForce GTX 570" },
	{ 0x10DE1082, "GeForce GTX 560 Ti" },
	{ 0x10DE1083, "D13U" },
	{ 0x10DE1084, "GeForce GTX 560" }, 
	{ 0x10DE1086, "GeForce GTX 570" }, 
	{ 0x10DE1087, "GeForce GTX 560 Ti-448" }, 
	{ 0x10DE1088, "GeForce GTX 590" },
	{ 0x10DE1089, "GeForce GTX 580" }, 
	{ 0x10DE108B, "GeForce GTX 590" },
   	{ 0x10DE108E,	"Tesla C2090" },
	// 1090 - 109F	
	{ 0x10DE1091, "Tesla M2090" },
    { 0x10DE1094,	"Tesla M2075 Dual-Slot Computing Processor Module" },
	{ 0x10DE1096,	"Tesla C2075" },
	{ 0x10DE1098, "D13U" },
	{ 0x10DE109A, "Quadro 5010M" }, 
	{ 0x10DE109B, "Quadro 7000" }, 
	// 10A0 - 10AF
	// 10B0 - 10BF
	// 10C0 - 10CF
	{ 0x10DE10C0, "GeForce 9300 GS" }, 
	{ 0x10DE10C3, "GeForce 8400 GS" },
   	{ 0x10DE10C4,	"NVIDIA ION" },
	{ 0x10DE10C5, "GeForce 405" }, 
	// 10D0 - 10DF
	{ 0x10DE10D8, "NVS 300" },
    // 10E0 - 10EF
	// 10F0 - 10FF
	// 1100 - 110F
	// 1110 - 111F
	// 1120 - 112F
	// 1130 - 113F
	// 1140 - 114F
	//  { 0x10DE1140,	"GF117" },
	{ 0x10DE1141,	"GeForce 610M" },
	{ 0x10DE1142,	"GeForce 620M" },
	//  { 0x10DE1143,	"N13P-GV" },
	//  { 0x10DE1144,	"GF117" },
	//  { 0x10DE1145,	"GF117" },
	//  { 0x10DE1146,	"GF117" },
	//  { 0x10DE1147,	"GF117" },
	//  { 0x10DE1149,	"GF117-ES" },
	//  { 0x10DE114A,	"GF117-INT" },
	//  { 0x10DE114B,	"PCI-GEN3-B" },
	// 1150 - 115F
	// 1160 - 116F
	// 1170 - 117F
	// 1180 - 118F
	{ 0x10DE1180,	"GeForce GTX 680" },
	{ 0x10DE1188,	"GeForce GTX 690" },
	{ 0x10DE1189,	"GeForce GTX 670" },
	// 1190 - 119F
	// 11A0 - 11AF
	// 11B0 - 11BF
	// 11C0 - 11CF
	// 11D0 - 11DF
	// 11E0 - 11EF
	// 11F0 - 11FF
	// 1200 - 120F
	{ 0x10DE1200,	"GeForce GTX 560 Ti" },
	{ 0x10DE1201,	"GeForce GTX 560" },
	{ 0x10DE1202,	"GeForce GTX 560 Ti" },
	{ 0x10DE1203,	"GeForce GTX 460 SE v2" },
	{ 0x10DE1205,	"GeForce GTX 460 v2" },
	{ 0x10DE1206,	"GeForce GTX 555" },
	{ 0x10DE1208,	"GeForce GTX 560 SE" },
	{ 0x10DE1210,	"GeForce GTX 570M" },
	{ 0x10DE1211,	"GeForce GTX 580M" },
	{ 0x10DE1212,	"GeForce GTX 675M" },
	{ 0x10DE1213,	"GeForce GTX 670M" },
	{ 0x10DE1240,	"GeForce GT 620M" },
	{ 0x10DE1241,	"GeForce GT 545" },
	{ 0x10DE1243,	"GeForce GT 545" },
	{ 0x10DE1244,	"GeForce GTX 550 Ti" },
	{ 0x10DE1245,	"GeForce GTS 450" },
	{ 0x10DE1246,	"GeForce GTX 550M" },
	{ 0x10DE1247,	"GeForce GT 635M" }, // Subsystem Id: 1043 212C Asus GeForce GT 635M
	{ 0x10DE1248,	"GeForce GTX 555M" },
	{ 0x10DE124B,	"GeForce GT 640" },
	{ 0x10DE124D,	"GeForce GTX 555M" },
	//  { 0x10DE1250,	"GF116-INT" },
	{ 0x10DE1251,	"GeForce GTX 560M" },
	// 1260 - 126F
	// 1270 - 127F
	// 1280 - 128F
	// 1290 - 129F
	// 12A0 - 12AF
	// 12B0 - 12BF
	// 12C0 - 12CF
	// 12D0 - 12DF
	// 12E0 - 12EF
	// 12F0 - 12FF

};

CHAR8 NVCAP[]= {  // Work
	0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07,
	0x00, 0x00, 0x00, 0x00
};

CHAR8 NVPM[]= {
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
};

CHAR8 *nv_name(UINT16 vendor_id, UINT16 device_id)
{
    INT32	i;

	for (i=1; i< (sizeof(NVKnowns) / sizeof(NVKnowns[0])); i++) 
	{
		if (NVKnowns[i].device == (vendor_id << 16| device_id << 0)) 
		{
			return NVKnowns[i].name;
		}
	}
	return NVKnowns[0].name;
}

UINT32 nv_mem_detect(pci_dt_t *nvda_dev)
{
	UINT32				bar[7];
    bar[0] = pci_config_read32(nvda_dev, PCI_BASE_ADDRESS_0);
    nvda_dev->regs = (UINT8 *)(UINTN)(bar[0] & ~0x0f);
		
	// get card type
	UINT8 nvCardType = (REG32(nvda_dev->regs, 0) >> 20) & 0x1ff;
	
	UINT64 vram_size = 0;
	
	// Workaround for GT 420/430 & 9600M GT
	switch (nvda_dev->device_id)
	{
		case 0x0DE1: vram_size = 1024*1024*1024; return vram_size; // GT 430
		case 0x0DE2: vram_size = 1024*1024*1024; return vram_size; // GT 420
		case 0x0649: vram_size = 512*1024*1024; return vram_size;	// 9600M GT
		case 0x0DF4: vram_size = (UINT32)(2048*1024)*(UINT32)1024; return vram_size;	// GT540M
		default: break;
	}
	
	if (nvCardType < 0x50)
	{
		vram_size  = REG32(nvda_dev->regs, 0x0010020c);
		vram_size &= 0xfff00000;
	}
	else if (nvCardType < 0xC0)
	{
		vram_size = REG32(nvda_dev->regs, 0x0010020c);
		vram_size |= (vram_size & 0xff) << 32;
		vram_size &= 0xffffffff00ll;
	}
	else // >= NV_ARCH_C0
	{
		vram_size = REG32(nvda_dev->regs, 0x0010f20c) << 20;
		vram_size *= REG32(nvda_dev->regs, 0x00121c74);
	}
	
	//DBG("mem_detected %ld\n", vram_size);
	return vram_size;
}


// Intel GMA .. HDXXXX

CHAR8 GMAX3100_vals_bad[21][4] = {
	{ 0x01,0x00,0x00,0x00 },
	{ 0x01,0x00,0x00,0x00 },
	{ 0x01,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x08 },
	{ 0x64,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x08 },
	{ 0x01,0x00,0x00,0x00 },
	{ 0x20,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x01,0x00,0x00,0x00 },
	{ 0x20,0x03,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x08,0x52,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x01,0x00,0x00,0x00 },
	{ 0x01,0x00,0x00,0x00 },
	{ 0x3B,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 }
};

CHAR8 HD2000_vals[16][4] = {
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x14,0x00,0x00,0x00 },
	{ 0xfa,0x00,0x00,0x00 },
	{ 0x2c,0x01,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x14,0x00,0x00,0x00 },
	{ 0xf4,0x01,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x01,0x00,0x00,0x00 },
};

CHAR8 HD3000_vals[17][4] = {
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x14,0x00,0x00,0x00 },
	{ 0xfa,0x00,0x00,0x00 },
	{ 0x2c,0x01,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x14,0x00,0x00,0x00 },
	{ 0xf4,0x01,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x00,0x00,0x00,0x00 },
	{ 0x01,0x00,0x00,0x00 },
	{ 0x00,0x00,0x01,0x00 },
};
//Brainless Hackintoshing
CHAR8 HD2000_tbl_info[18] = {
	0x30,0x44,0x02,0x02,0x02,0x02,0x00,0x00,0x00,
	0x00,0x01,0x02,0x02,0x02,0x00,0x01,0x02,0x02
};
CHAR8 HD2000_os_info[20] = {
	0x30,0x49,0x01,0x11,0x11,0x11,0x08,0x00,0x00,0x01,
	0xf0,0x1f,0x01,0x00,0x00,0x00,0x10,0x07,0x00,0x00
};

// The following values came from a Sandy Bridge MacBook Air
CHAR8 HD3000_tbl_info[18] = {
	0x30,0x44,0x02,0x02,0x02,0x02,0x00,0x00,0x00,
	0x00,0x02,0x02,0x02,0x02,0x01,0x01,0x01,0x01
};

// The following values came from a Sandy Bridge MacBook Air
CHAR8 HD3000_os_info[20] = {
	0x30,0x49,0x01,0x12,0x12,0x12,0x08,0x00,0x00,0x01,
	0xf0,0x1f,0x01,0x00,0x00,0x00,0x10,0x07,0x00,0x00
};
/*
struct gma_gpu_t {
	unsigned device;
	char *name;
};

static struct gma_gpu_t KnownGPUS[] = {
	{ 0x00000000, "Unknown"			},
	{ 0x808627A2, "Mobile GMA950"	},
	{ 0x808627AE, "Mobile GMA950"	},
	{ 0x808627A6, "Mobile GMA950"	},
	{ 0x8086A011, "Mobile GMA3150"	},
	{ 0x8086A012, "Mobile GMA3150"	},
	{ 0x80862772, "Desktop GMA950"	},
	{ 0x80862776, "Desktop GMA950"	},
//	{ 0x8086A001, "Desktop GMA3150" },
	{ 0x8086A001, "Mobile GMA3150"	},
	{ 0x8086A002, "Desktop GMA3150" },
	{ 0x80862A02, "GMAX3100"		},
	{ 0x80862A03, "GMAX3100"		},
	{ 0x80862A12, "GMAX3100"		},
	{ 0x80862A13, "GMAX3100"		},
	{ 0x80862A42, "GMAX3100"		},
	{ 0x80862A43, "GMAX3100"		},
	{ 0x80860102, "Intel HD Graphics 2000"			},
	{ 0x80860106, "Intel HD Graphics 2000 Mobile"	},
	{ 0x80860112, "Intel HD Graphics 3000"			},
	{ 0x80860116, "Intel HD Graphics 3000 Mobile"	},
	{ 0x80860122, "Intel HD Graphics 3000"			},
	{ 0x80860126, "Intel HD Graphics 3000 Mobile"	},
};

CHAR8 *get_gma_name(UINT32 id) 
{
	UINT32 i = 0;
	
	for (i = 0; i < (sizeof(KnownGPUS) / sizeof(KnownGPUS[0])); i++)
	{
		if (KnownGPUS[i].device == id)
			return KnownGPUS[i].name;
	}
	return KnownGPUS[0].name;
}
*/
#endif
