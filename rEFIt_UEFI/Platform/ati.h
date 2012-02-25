/*
 *  ati.h
 *  
 *  Created by Slice on 19.02.12.
 *  
 *  the code ported from Chameleon project as well as from RadeonFB by Joblo and RadeonHD by dong
 */

#include "Platform.h"  //this include needed for Uefi types
#include "ati_reg.h"

#define OFFSET_TO_GET_ATOMBIOS_STRINGS_START 0x6e
#define DATVAL(x)			{kPtr, sizeof(x), (UINT8 *)x}
#define STRVAL(x)			{kStr, sizeof(x), (UINT8 *)x}
#define BYTVAL(x)			{kCst, 1, (UINT8 *)x}
#define WRDVAL(x)			{kCst, 2, (UINT8 *)x}
#define DWRVAL(x)			{kCst, 4, (UINT8 *)x}
#define QWRVAL(x)			{kCst, 8, (UINT8 *)x}
#define NULVAL				{kNul, 0, (UINT8 *)NULL}


typedef enum {
	kNul,
	kStr,
	kPtr,
	kCst
} type_t;

typedef enum {
	CHIP_FAMILY_UNKNOW,
  /* Old */
  CHIP_FAMILY_R420,
  CHIP_FAMILY_RV410,
  CHIP_FAMILY_RV515,
  CHIP_FAMILY_R520,
  CHIP_FAMILY_RV530,
  CHIP_FAMILY_RV560,
  CHIP_FAMILY_RV570,
  CHIP_FAMILY_R580,
  
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
} chip_family_t;

static const CHAR8 *chip_family_name[] = {
	"UNKNOW",
  "R420",
  "RV410",
  "RV515",
  "R520",
  "RV530",
  "RV560",
  "RV570",
  "R580",
	/* IGP */
	"RS600",
	"RS690",
	"RS740",
	"RS780",
	"RS880",
	/* R600 */
	"R600",
	"RV610",
	"RV620",
	"RV630",
	"RV635",
	"RV670",
	/* R700 */
	"RV710",
	"RV730",
	"RV740",
	"RV770",
	/* Evergreen */
	"Cedar",
	"Cypress",
	"Hemlock",
	"Juniper",
	"Redwood",
	/* Northern Islands */
	"Barts",
	"Caicos",
	"Cayman",
	"Turks",
	""
};

typedef struct {
	const CHAR8		*name;
	UINT8			ports;
} card_config_t;

static card_config_t card_configs[] = {
	{NULL,			0},
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
	{"Hoolock",		3},
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
} config_name_t;

typedef struct {
	UINT16            device_id;
	UINT32            subsys_id;
	chip_family_t			chip_family;
	const CHAR8				*model_name;
	config_name_t			cfg_name;
} radeon_card_info_t;

static radeon_card_info_t radeon_cards[] = {
	
	// Earlier cards are not supported
	//
	// Layout is device_id, subsys_id (subsystem id plus vendor id), chip_family_name, display name, frame buffer
	// Cards are grouped by device id and vendor id then sorted by subsystem id to make it easier to add new cards
	//  
	{ 0x9400,	0x25521002, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 XT",			kNull		},
	{ 0x9400,	0x30001002, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 PRO",			kNull		},
	
	{ 0x9440,	0x24401682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4870",				kMotmot		},
	{ 0x9440,	0x24411682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4870",				kMotmot		},
	{ 0x9440,	0x24441682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4870",				kMotmot		},
	{ 0x9440,	0x24451682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4870",				kMotmot		},
	
	{ 0x9441,	0x24401682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4870 X2",		kMotmot		},
	
	{ 0x9442,	0x080110B0, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850",				kMotmot		},
	
	{ 0x9442,	0x24701682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850",				kMotmot		},
	{ 0x9442,	0x24711682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850",				kMotmot		},
	
	{ 0x9442,	0xE104174B, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850",				kMotmot		},
	
	{ 0x944A,	0x30001043, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",		kMotmot		},
	
	{ 0x944A,	0x30001458, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",		kMotmot		},
	
	{ 0x944A,	0x30001462, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",		kMotmot		},
	
	{ 0x944A,	0x30001545, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",		kMotmot		},
	
	{ 0x944A,	0x30001682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",		kMotmot		},
	
	{ 0x944A,	0x3000174B, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",		kMotmot		},
	
	{ 0x944A,	0x30001787, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",		kMotmot		},
	
	{ 0x944A,	0x300017AF, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",		kMotmot		},
	
	{ 0x944C,	0x24801682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4830",				kMotmot		},
	{ 0x944C,	0x24811682, CHIP_FAMILY_RV770,		"ATI Radeon HD 4830",				kMotmot		},
	
	{ 0x944E,	0x3260174B, CHIP_FAMILY_RV770,		"ATI Radeon HD 4810 Series",		kMotmot		},
	{ 0x944E,	0x3261174B, CHIP_FAMILY_RV770,		"ATI Radeon HD 4810 series",		kMotmot		},
	
	{ 0x944E,	0x30001787, CHIP_FAMILY_RV770,		"ATI Radeon HD 4730 Series",		kMotmot		},
	{ 0x944E,	0x30101787, CHIP_FAMILY_RV770,		"ATI Radeon HD 4810 Series",		kMotmot		},
	{ 0x944E,	0x31001787, CHIP_FAMILY_RV770,		"ATI Radeon HD 4820",				kMotmot		},
	
	{ 0x9480,	0x3628103C, CHIP_FAMILY_RV730,		"ATI Radeon HD 4650M",				kGliff		},
	
	{ 0x9480,	0x9035104D, CHIP_FAMILY_RV730,		"ATI Radeon HD 4650M",				kGliff		},
	
	{ 0x9490,	0x4710174B, CHIP_FAMILY_RV730,		"ATI Radeon HD 4710",				kNull		},
	
	{ 0x9490,	0x20031787, CHIP_FAMILY_RV730,		"ATI Radeon HD 4670",				kFlicker	},
	{ 0x9490,	0x30501787, CHIP_FAMILY_RV730,		"ATI Radeon HD 4710",				kNull		},
	
	{ 0x9490,	0x300017AF, CHIP_FAMILY_RV730,		"ATI Radeon HD 4710",				kNull		},
	
	{ 0x9498,	0x21CF1458, CHIP_FAMILY_RV730,		"ATI Radeon HD 4600 Series",		kNull		},
	
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
	
	{ 0x94C3,	0x30001025, CHIP_FAMILY_RV610,		"ATI Radeon HD 2350 Series",		kNull		},
	
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
	{ 0x94C3,	0x3000148C, CHIP_FAMILY_RV610,		"ATI Radeon HD 2350 Series",		kNull		},
	
	{ 0x94C3,	0x30001642, CHIP_FAMILY_RV610,		"ATI Radeon HD 3410",				kNull		},
	{ 0x94C3,	0x37161642, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 PRO",			kNull		},
	
	{ 0x94C3,	0x3000174B, CHIP_FAMILY_RV610,		"ATI Radeon HD 2350 Series",		kNull		},
	{ 0x94C3,	0xE370174B, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 PRO",			kNull		},
	{ 0x94C3,	0xE400174B, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 PRO",			kNull		},
	
	{ 0x94C3,	0x203817AF, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400",				kNull		},
	
	{ 0x94C3,	0x22471787, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 LE",			kNull		},
	{ 0x94C3,	0x30001787, CHIP_FAMILY_RV610,		"ATI Radeon HD 2350 Series",		kNull		},
	
	{ 0x94C3,	0x01011A93, CHIP_FAMILY_RV610,		"Qimonda Radeon HD 2400 PRO",		kNull		},
	
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
	
	{ 0x9552,	0x308B103C, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4300 Series", kShrike	},
	
	{ 0x9552,	0x3000148C, CHIP_FAMILY_RV710,		"ATI Radeon HD 4300/4500 Series",	kNull		},
	
	{ 0x9552,	0x3000174B, CHIP_FAMILY_RV710,		"ATI Radeon HD 4300/4500 Series",	kNull		},
	
	{ 0x9552,	0x30001787, CHIP_FAMILY_RV710,		"ATI Radeon HD 4300/4500 Series",	kNull		},
	
	{ 0x9552,	0x300017AF, CHIP_FAMILY_RV710,		"ATI Radeon HD 4300/4500 Series",	kNull		},
	
	{ 0x9553,	0x18751043, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4570",		kShrike		},
	{ 0x9553,	0x1B321043, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4570",		kShrike		},
	
	{ 0x9581,	0x95811002, CHIP_FAMILY_RV630,		"ATI Radeon HD 3600 Series",		kNull		},
	
	{ 0x9581,	0x3000148C, CHIP_FAMILY_RV630,		"ATI Radeon HD 3600 Series",		kNull		},
	
	{ 0x9583,	0x3000148C, CHIP_FAMILY_RV630,		"ATI Radeon HD 3600 Series",		kNull		},
	
	{ 0x9588,	0x01021A93, CHIP_FAMILY_RV630,		"Qimonda Radeon HD 2600 XT",		kNull		},
	
	{ 0x9589,	0x30001462, CHIP_FAMILY_RV630,		"ATI Radeon HD 3610",				kNull		},
	
	{ 0x9589,	0x0E41174B, CHIP_FAMILY_RV630,		"ATI Radeon HD 3600 Series",		kNull		},
	
	{ 0x9589,	0x30001787, CHIP_FAMILY_RV630,		"ATI Radeon HD 3600 Series",		kNull		},
	
	{ 0x9589,	0x01001A93, CHIP_FAMILY_RV630,		"Qimonda Radeon HD 2600 PRO",		kNull		},
	
	{ 0x9591,	0x2303148C, CHIP_FAMILY_RV635,		"ATI Radeon HD 3600 Series",		kNull		},
	
	{ 0x9598,	0xB3831002, CHIP_FAMILY_RV635,		"ATI All-in-Wonder HD",				kNull		},
	
	{ 0x9598,	0x30001043, CHIP_FAMILY_RV635,		"ATI Radeon HD 3730",				kNull		},
	{ 0x9598,	0x30011043, CHIP_FAMILY_RV635,		"ATI Radeon HD 4570",				kNull		},
	
	{ 0x9598,	0x3000148C, CHIP_FAMILY_RV635,		"ATI Radeon HD 3730",				kNull		},
	{ 0x9598,	0x3001148C, CHIP_FAMILY_RV635,		"ATI Radeon HD 4580",				kNull		},
	{ 0x9598,	0x3031148C, CHIP_FAMILY_RV635,		"ATI Radeon HD 4570",				kNull		},
	
	{ 0x9598,	0x30001545, CHIP_FAMILY_RV635,		"VisionTek Radeon HD 2600 XT",		kNull		},
	{ 0x9598,	0x30011545, CHIP_FAMILY_RV635,		"VisionTek Radeon HD 2600 Pro",		kNull		},
	
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
	
	{ 0x6899,	0x200A1787, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5850",				kUakari		},
	{ 0x6899,	0x22901787, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5850",				kUakari		},
	
	{ 0x689C,	0x03521043, CHIP_FAMILY_HEMLOCK,	"ASUS ARES",						kUakari		},
	{ 0x689C,	0x039E1043, CHIP_FAMILY_HEMLOCK,	"ASUS EAH5870 Series",				kUakari		},
	
	{ 0x689C,	0x30201682, CHIP_FAMILY_HEMLOCK,	"ATI Radeon HD 5970",				kUakari		},
	
	{ 0x68A1,	0x144D103C,	CHIP_FAMILY_CYPRESS,	"ATI Mobility Radeon HD 5850",		kNomascus	},
	{ 0x68A1,	0x1522103C, CHIP_FAMILY_CYPRESS,	"ATI Mobility Radeon HD 5850",		kHoolock	},
	
	{ 0x68A8,	0x050E1025, CHIP_FAMILY_CYPRESS,	"AMD Radeon HD 6850M",				kUakari		},
	
	{ 0x68B8,	0x00CF106B, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",				kHoolock	},
	
	{ 0x68B8,	0x29901682, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",				kVervet		},
	{ 0x68B8,	0x29911682, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",				kVervet		},
	
	{ 0x68B8,	0x1482174B, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",				kVervet		},
	{ 0x68B8,	0xE147174B, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",				kVervet		},
	
	{ 0x68B8,	0x21D71458, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",				kVervet		},
	
	{ 0x68B8,	0x200B1787, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",				kVervet		},
	{ 0x68B8,	0x22881787, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770",				kVervet		},
	
	{ 0x68BF,	0x220E1458, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 6750",				kVervet		},
	
	{ 0x68C0,	0x1594103C, CHIP_FAMILY_REDWOOD,	"AMD Radeon HD 6570M",				kNull		},
	
	{ 0x68C0,	0x392717AA, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5730",		kNull		},
	
	{ 0x68C1,	0x033E1025, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5650",		kNull		},
	
	{ 0x68C1,	0x9071104D,	CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5650",		kEulemur	},
	
	{ 0x68C8,	0x2306103C, CHIP_FAMILY_REDWOOD,	"ATI FirePro V4800 (FireGL)",		kNull		},
	
	{ 0x68D8,	0x03561043, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5670",				kBaboon		},
  
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
	
	{ 0x68F9,	0x5470174B, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470",				kNull		},
	{ 0x68F9,	0x5490174B, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5490",				kNull		},
	{ 0x68F9,	0x5530174B, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5530",				kNull		},
  
	{ 0x68F9,	0x20091787, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",				kEulemur	},
	{ 0x68F9,	0x22911787, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5450",				kEulemur	},
	{ 0x68F9,	0x30001787, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470",				kNull		},
	{ 0x68F9,	0x30011787, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5530",				kNull		},
	{ 0x68F9,	0x30021787, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5490",				kNull		},
	
	{ 0x68F9,	0x301117AF, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470",				kNull		},
	{ 0x68F9,	0x301217AF, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5490",				kNull		},
	{ 0x68F9,	0x301317AF, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5470",				kNull		},
	
	/* Northen Islands */
	{ 0x6718,	0x0B001002, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970",				kNull		},
	{ 0x6718,	0x67181002, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970",				kNull		},
	
	{ 0x6718,	0x31301682, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970",				kNull		},
  
	{ 0x6738,	0x00D01002,	CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",				kDuckweed	},
	{ 0x6738,	0x21FA1002,	CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",				kDuckweed	},
	{ 0x6738,	0x67381002, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",				kDuckweed	},
  
	{ 0x6738,	0x21FA1458,	CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",				kDuckweed	},
	
	{ 0x6738,	0x31031682,	CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",				kDuckweed	},
	{ 0x6738,	0x31041682,	CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",				kDuckweed	},
	
	{ 0x6738,	0xE178174B,	CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",				kDuckweed	},
	
	{ 0x6738,	0x20101787,	CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",				kDuckweed	},
	{ 0x6738,	0x23051787,	CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870",				kDuckweed	},
	
	{ 0x6739,	0x67391002, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",				kDuckweed	},
	
	{ 0x6739,	0x21F81458, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",				kDuckweed	},
  
	{ 0x6739,	0x24411462, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",				kDuckweed	},
	
	{ 0x6739,	0x31101682, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",				kDuckweed	},   
	
	{ 0x6739,	0xE177174B,	CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850",				kDuckweed	},
  
	{ 0x6740,	0x1657103C, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6770M",				kNull		},
	{ 0x6740,	0x165A103C, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6770M",				kNull		},
	
	{ 0x6741,	0x050E1025, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6650M",				kNull		},
	{ 0x6741,	0x05131025, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6650M",				kNull		},
	
	{ 0x6741,	0x1646103C, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6750M",				kNull		},
	
	{ 0x6741,	0x9080104D,	CHIP_FAMILY_TURKS,		"AMD Radeon HD 6630M",				kNull		},
  
	{ 0x6758,	0x67581002,	CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670",				kBulrushes	},
	
	{ 0x6758,	0x22051458,	CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670",				kBulrushes	},
	
	{ 0x6758,	0xE194174B,	CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670",				kBulrushes	},
	
	{ 0x6758,	0x31811682,	CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670",				kBulrushes	},
	{ 0x6758,	0x31831682,	CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670",				kBulrushes	},
	
	{ 0x6758,	0xE1941746,	CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670",				kBulrushes	},
	
	{ 0x6759,	0xE193174B, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6570",				kNull		},
	
	{ 0x6760,	0x04CC1028,	CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6490M",				kNull		},
	
	{ 0x6760,	0x1CB21043, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6470M",				kNull		},
	
	{ 0x6779,	0x64501092,	CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450",				kBulrushes	},
	
	{ 0x6779,	0xE164174B,	CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450",				kBulrushes	},	
/*old series*/
  { 0x5D48, 0x00000000, CHIP_FAMILY_R420, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x5D49, 0x00000000, CHIP_FAMILY_R420, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x5D4A, 0x00000000, CHIP_FAMILY_R420, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x5D4C, 0x00000000, CHIP_FAMILY_R420, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x5D4D, 0x00000000, CHIP_FAMILY_R420, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x5D4E, 0x00000000, CHIP_FAMILY_R420, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x5D4F, 0x00000000, CHIP_FAMILY_R420, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x5D50, 0x00000000, CHIP_FAMILY_R420, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x5D52, 0x00000000, CHIP_FAMILY_R420, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x5D57, 0x00000000, CHIP_FAMILY_R420, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x5E48, 0x00000000, CHIP_FAMILY_RV410, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x5E4A, 0x00000000, CHIP_FAMILY_RV410, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x5E4B, 0x00000000, CHIP_FAMILY_RV410, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x5E4C, 0x00000000, CHIP_FAMILY_RV410, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x5E4D, 0x00000000, CHIP_FAMILY_RV410, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x5E4F, 0x00000000, CHIP_FAMILY_RV410, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7100, 0x00000000, CHIP_FAMILY_R520, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7101, 0x00000000, CHIP_FAMILY_R520, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x7102, 0x00000000, CHIP_FAMILY_R520, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x7103, 0x00000000, CHIP_FAMILY_R520, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x7104, 0x00000000, CHIP_FAMILY_R520, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7105, 0x00000000, CHIP_FAMILY_R520, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7106, 0x00000000, CHIP_FAMILY_R520, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x7108, 0x00000000, CHIP_FAMILY_R520, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7109, 0x00000000, CHIP_FAMILY_R520, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x710A, 0x00000000, CHIP_FAMILY_R520, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x710B, 0x00000000, CHIP_FAMILY_R520, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x710C, 0x00000000, CHIP_FAMILY_R520, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x710E, 0x00000000, CHIP_FAMILY_R520, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x710F, 0x00000000, CHIP_FAMILY_R520, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7140, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7141, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7142, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7143, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7144, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x7145, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x7146, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7147, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7149, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x714A, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x714B, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x714C, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x714D, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x714E, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x714F, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7151, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7152, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7153, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x715E, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x715F, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7180, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7181, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7183, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7186, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x7187, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7188, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x718A, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x718B, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x718C, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x718D, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x718F, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7193, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7196, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x719B, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x719F, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x71C0, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x71C1, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x71C2, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x71C3, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x71C4, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x71C5, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x71C6, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x71C7, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x71CD, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x71CE, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x71D2, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x71D4, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x71D5, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x71D6, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x71DA, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x71DE, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x7200, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7210, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x7211, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x7240, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7243, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7244, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7245, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7246, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7247, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7248, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7249, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x724A, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x724B, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x724C, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x724D, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x724E, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x724F, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7280, 0x00000000, CHIP_FAMILY_RV570, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7281, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7283, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7284, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Mobile ",			kNull		 },
  { 0x7287, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7288, 0x00000000, CHIP_FAMILY_RV570, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7289, 0x00000000, CHIP_FAMILY_RV570, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x728B, 0x00000000, CHIP_FAMILY_RV570, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x728C, 0x00000000, CHIP_FAMILY_RV570, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7290, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7291, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7293, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x7297, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",			kNull		 },
  { 0x791E, 0x00000000, CHIP_FAMILY_RS690, "ATI Radeon IGP ",			kNull		 },
  { 0x791F, 0x00000000, CHIP_FAMILY_RS690, "ATI Radeon IGP ",			kNull		},
  { 0x796C, 0x00000000, CHIP_FAMILY_RS740, "ATI Radeon IGP ",			kNull		 },
  { 0x796D, 0x00000000, CHIP_FAMILY_RS740, "ATI Radeon IGP ",			kNull		},
  { 0x796E, 0x00000000, CHIP_FAMILY_RS740, "ATI Radeon IGP ",			kNull		},
  { 0x796F, 0x00000000, CHIP_FAMILY_RS740, "ATI Radeon HD ",			kNull		 },
  
	
  
	/* standard/default models */
	{ 0x9400,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 XT",			kNull		},
	{ 0x9401,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 GT",			kNull		},
	{ 0x9402,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 GT",			kNull		},
	{ 0x9403,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 GT",			kNull		},
	{ 0x9405,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 GT",			kNull		},
	{ 0x940A,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 GT",			kNull		},
	{ 0x940B,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 GT",			kNull		},
	{ 0x940F,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 GT",			kNull		},
	
	{ 0x9440,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",		kMotmot		},
	{ 0x9441,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4870 X2",			kMotmot		},
	{ 0x9442,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",		kMotmot		},
	{ 0x9443,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850 X2",			kMotmot		},
	{ 0x944A,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",		kMotmot		},
	{ 0x944C,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",		kMotmot		},
	{ 0x944E,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4700 Series",		kMotmot		},
	
	{ 0x9450,	0x00000000, CHIP_FAMILY_RV770,		"AMD FireStream 9270",				kMotmot		},
	{ 0x9452,	0x00000000, CHIP_FAMILY_RV770,		"AMD FireStream 9250",				kMotmot		},
	
	{ 0x9460,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",		kMotmot		},
	{ 0x9462,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",		kMotmot		},

	{ 0x9480,	0x00000000, CHIP_FAMILY_RV730,		"ATI Radeon HD 4600 Series",		kFlicker	},
	{ 0x9490,	0x00000000, CHIP_FAMILY_RV730,		"ATI Radeon HD 4600 Series",		kFlicker	},
	{ 0x9498,	0x00000000, CHIP_FAMILY_RV730,		"ATI Radeon HD 4600 Series",		kFlicker	},
	
	{ 0x94B3,	0x00000000, CHIP_FAMILY_RV740,		"ATI Radeon HD 4770",				kFlicker	},
	{ 0x94B4,	0x00000000, CHIP_FAMILY_RV740,		"ATI Radeon HD 4700 Series",		kFlicker	},
	{ 0x94B5,	0x00000000, CHIP_FAMILY_RV740,		"ATI Radeon HD 4770",				kFlicker	},
	
	{ 0x94C1,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",		kIago		},
	{ 0x94C3,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",		kIago		},
	{ 0x94C4,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",		kIago		},
	{ 0x94C5,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",		kIago		},
	{ 0x94C6,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",		kIago		},
	{ 0x94C7,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2350",				kIago		},
	{ 0x94C8,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",		kIago		},
	{ 0x94C9,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",		kIago		},
	{ 0x94CB,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",		kIago		},
	{ 0x94CC,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",		kIago		},

	{ 0x9500,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3800 Series",		kMegalodon	},
	{ 0x9501,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3800 Series",		kMegalodon	},
	{ 0x9505,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3800 Series",		kMegalodon	},
	{ 0x9507,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3830",				kMegalodon	},
	{ 0x950F,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3870 X2",			kMegalodon	},
	{ 0x9511,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3850 X2",			kMegalodon	},
	
	{ 0x9513,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3850 X2",			kMegalodon	},
	{ 0x9519,	0x00000000, CHIP_FAMILY_RV670,		"AMD FireStream 9170",				kMegalodon	},
	
	{ 0x9540,	0x00000000, CHIP_FAMILY_RV710,		"ATI Radeon HD 4550",				kNull		},
	{ 0x954F,	0x00000000, CHIP_FAMILY_RV710,		"ATI Radeon HD 4300/4500 Series",	kNull		},
	
	{ 0x9553,	0x00000000, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4500/5100 Series",	kShrike },
	
	{ 0x9588,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 XT",			kLamna		},
	{ 0x9589,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 PRO",			kLamna		},
	{ 0x958A,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 X2 Series",		kLamna		},
	{ 0x958B,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 X2 Series",		kLamna		},
	{ 0x958C,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 X2 Series",		kLamna		},
	{ 0x958D,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 X2 Series",		kLamna		},
	{ 0x958E,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 X2 Series",		kLamna		},
	
	{ 0x9598,	0x00000000, CHIP_FAMILY_RV635,		"ATI Radeon HD 3600 Series",		kMegalodon	},
	
	{ 0x95C0,	0x00000000, CHIP_FAMILY_RV620,		"ATI Radeon HD 3400 Series",		kIago		},
	{ 0x95C5,	0x00000000, CHIP_FAMILY_RV620,		"ATI Radeon HD 3400 Series",		kIago		},
	
	/* IGP */
	{ 0x9610,	0x00000000, CHIP_FAMILY_RS780,		"ATI Radeon HD 3200 Graphics",		kNull		},
	{ 0x9611,	0x00000000, CHIP_FAMILY_RS780,		"ATI Radeon 3100 Graphics",			kNull		},
	{ 0x9614,	0x00000000, CHIP_FAMILY_RS780,		"ATI Radeon HD 3300 Graphics",		kNull		},
	{ 0x9616,	0x00000000, CHIP_FAMILY_RS780,		"AMD 760G",							kNull		},
	
	{ 0x9710,	0x00000000, CHIP_FAMILY_RS880,		"ATI Radeon HD 4200",				kNull		},
	{ 0x9715,	0x00000000, CHIP_FAMILY_RS880,		"ATI Radeon HD 4250",				kNull		},
	{ 0x9714,	0x00000000, CHIP_FAMILY_RS880,		"ATI Radeon HD 4290",				kNull		},
	
	/* Evergreen */
	{ 0x688D,	0x00000000, CHIP_FAMILY_CYPRESS,	"AMD FireStream 9350",				kUakari		},
	
	{ 0x6898,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5800 Series",		kUakari		},
	{ 0x6899,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5800 Series",		kUakari		},
	{ 0x689C,	0x00000000, CHIP_FAMILY_HEMLOCK,	"ATI Radeon HD 5900 Series",		kUakari		},
	{ 0x689E,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5800 Series",		kUakari		},
	
	{ 0x68B8,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5700 Series",		kVervet		},
	{ 0x68B9,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5600 Series",		kVervet		},
	{ 0x68BE,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5700 Series",		kVervet		},
	
	{ 0x68D8,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5600 Series",		kBaboon		},
	{ 0x68D9,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5500 Series",		kBaboon		},
	{ 0x68DA,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5500 Series",		kBaboon		},
	{ 0x68E0,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5500 Series",		kBaboon		},	
	{ 0x68F9,	0x00000000, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5400 Series",		kNull		},
	
	/* Northen Islands */
	{ 0x6718,	0x00000000, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970 Series",		kNull		},
	{ 0x6719,	0x00000000, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6950 Series",		kNull		},
	
	{ 0x6720,	0x00000000, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6900M Series",		kNull		},
	
	{ 0x6738,	0x00000000, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870 Series",		kDuckweed	},
	{ 0x6739,	0x00000000, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850 Series",		kDuckweed	},
	{ 0x673E,	0x00000000, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6790 Series",		kNull		},
	
	{ 0x6740,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6700M Series",		kNull		},
	{ 0x6741,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6650M Series",	 kNull		}, // kOsmunda },
	
	{ 0x6758,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670 Series",		kBulrushes	},
	{ 0x6759,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6500 Series",		kNull		},
  
	{ 0x6760,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6400M Series",		kNull		},
	{ 0x6761,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6430M Series",		kNull		},
  
	{ 0x6770,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6400 Series",		kNull		},
	{ 0x6779,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450 Series",		kBulrushes	},
	
	{ 0x0000,	0x00000000, CHIP_FAMILY_UNKNOW,		NULL,								kNull		}
};

typedef struct {
	DevPropDevice	*device;
	radeon_card_info_t		*info;
	pci_dt_t              *pci_dev;
	UINT8                 *fb;
	UINT8                 *mmio;
	UINT8                 *io;
	UINT8                 *rom;
	UINT32                rom_size;
	UINT32                vram_size;
	const CHAR8           *cfg_name;
	UINT8                 ports;
	UINT32                flags;
	BOOLEAN               posted;
} card_t;


/* Flags */
#define MKFLAG(n)			(1 << n)
#define FLAGTRUE			MKFLAG(0)
#define EVERGREEN			MKFLAG(1)

//static UINT8 atN = 0;

typedef struct {
	type_t				type;
	UINT32				size;
	UINT8					*data;
} value_t;

typedef struct {
	UINT32				flags;
	BOOLEAN					all_ports;
	CHAR8					*name;
	BOOLEAN					(*get_value)(value_t *val);
	value_t					default_val;
} AtiDevProp;

BOOLEAN get_bootdisplay_val(value_t *val);
BOOLEAN get_vrammemory_val(value_t *val);
BOOLEAN get_name_val(value_t *val);
BOOLEAN get_nameparent_val(value_t *val);
BOOLEAN get_model_val(value_t *val);
BOOLEAN get_conntype_val(value_t *val);
BOOLEAN get_vrammemsize_val(value_t *val);
BOOLEAN get_binimage_val(value_t *val);
BOOLEAN get_romrevision_val(value_t *val);
BOOLEAN get_deviceid_val(value_t *val);
BOOLEAN get_mclk_val(value_t *val);
BOOLEAN get_sclk_val(value_t *val);
BOOLEAN get_refclk_val(value_t *val);
BOOLEAN get_platforminfo_val(value_t *val);
BOOLEAN get_vramtotalsize_val(value_t *val);


AtiDevProp ati_devprop_list[] = {
	{FLAGTRUE,	FALSE,	"@0,AAPL,boot-display",		get_bootdisplay_val,	NULVAL							},
  //	{FLAGTRUE,	FALSE,	"@0,ATY,EFIDisplay",		NULL,					STRVAL("TMDSA")					},
	
  //	{FLAGTRUE,	TRUE,	"@0,AAPL,vram-memory",		get_vrammemory_val,		NULVAL							},
  //	{FLAGTRUE,	TRUE,	"@0,compatible",			get_name_val,			NULVAL							},
  //	{FLAGTRUE,	TRUE,	"@0,connector-type",		get_conntype_val,		NULVAL							},
  //	{FLAGTRUE,	TRUE,	"@0,device_type",			NULL,					STRVAL("display")				},
  //	{FLAGTRUE,	FALSE,	"@0,display-connect-flags", NULL,					DWRVAL((UINT32)0)				},
  //	{FLAGTRUE,	TRUE,	"@0,display-type",			NULL,					STRVAL("NONE")					},
	{FLAGTRUE,	TRUE,	"@0,name",					get_name_val,			NULVAL							},
  //	{FLAGTRUE,	TRUE,	"@0,VRAM,memsize",			get_vrammemsize_val,	NULVAL							},
	
  //	{FLAGTRUE,	FALSE,	"AAPL,aux-power-connected", NULL,					DWRVAL((UINT32)1)				},
  //	{FLAGTRUE,	FALSE,	"AAPL,backlight-control",	NULL,					DWRVAL((UINT32)0)				},
	{FLAGTRUE,	FALSE,	"ATY,bin_image",			get_binimage_val,		NULVAL							},
	{FLAGTRUE,	FALSE,	"ATY,Copyright",			NULL,	STRVAL("Copyright AMD Inc. All Rights Reserved. 2005-2010") },
	{FLAGTRUE,	FALSE,	"ATY,Card#",				get_romrevision_val,	NULVAL							},
	{FLAGTRUE,	FALSE,	"ATY,VendorID",				NULL,					WRDVAL((UINT16)0x1002)		},
	{FLAGTRUE,	FALSE,	"ATY,DeviceID",				get_deviceid_val,		NULVAL							},
	
  //	{FLAGTRUE,	FALSE,	"ATY,MCLK",					get_mclk_val,			NULVAL							},
  //	{FLAGTRUE,	FALSE,	"ATY,SCLK",					get_sclk_val,			NULVAL							},
  //	{FLAGTRUE,	FALSE,	"ATY,RefCLK",				get_refclk_val,			DWRVAL((UINT32)0x0a8c)		},
	
  //	{FLAGTRUE,	FALSE,	"ATY,PlatformInfo",			get_platforminfo_val,	NULVAL							},
	
	{FLAGTRUE,	FALSE,	"name",						get_nameparent_val,		NULVAL							},
	{FLAGTRUE,	FALSE,	"device_type",				get_nameparent_val,		NULVAL							},
	{FLAGTRUE,	FALSE,	"model",					get_model_val,			STRVAL("ATI Radeon")			},
  //	{FLAGTRUE,	FALSE,	"VRAM,totalsize",			get_vramtotalsize_val,	NULVAL							},
	
	{FLAGTRUE,	FALSE,	NULL,						NULL,					NULVAL							}
};

