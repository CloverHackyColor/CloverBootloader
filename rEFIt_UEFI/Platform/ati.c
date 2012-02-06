/*
 * ATI Graphics Card Enabler, part of the Chameleon Boot Loader Project
 *
 * Copyright 2010 by Islam M. Ahmed Zaid. All rights reserved.
 *
 */

#include "Platform.h"
#include "device_inject.h"
#include "ati_reg.h"

#define DEBUG_ATI 1

#if DEBUG_ATI == 2
#define DBG(x...) AsciiPrint(x)
#elif DEBUG_ATI == 1
#define DBG(x...) MsgLog(x)
#else
#define DBG(x...)
#endif


#define OFFSET_TO_GET_ATOMBIOS_STRINGS_START 0x6e

#define Reg32(reg)				(*(volatile UINT32 *)(card->mmio + reg))
#define RegRead32(reg)			(Reg32(reg))
#define RegWrite32(reg, value)	(Reg32(reg) = value)

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


typedef enum {
	kNul,
	kStr,
	kPtr,
	kCst
} type_t;

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
} chip_family_t;

static const CHAR8 *chip_family_name[] = {
	"UNKNOW",
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
	
	/* standard/default models */
	{ 0x9400,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 XT",			kNull		},
	{ 0x9405,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 GT",			kNull		},
	
	{ 0x9440,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",		kMotmot		},
	{ 0x9441,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4870 X2",			kMotmot		},
	{ 0x9442,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",		kMotmot		},
	{ 0x9443,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850 X2",			kMotmot		},
	{ 0x944C,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",		kMotmot		},
	{ 0x944E,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4700 Series",		kMotmot		},
	
	{ 0x9450,	0x00000000, CHIP_FAMILY_RV770,		"AMD FireStream 9270",				kMotmot		},
	{ 0x9452,	0x00000000, CHIP_FAMILY_RV770,		"AMD FireStream 9250",				kMotmot		},
	
	{ 0x9460,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",		kMotmot		},
	{ 0x9462,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",		kMotmot		},
	
	{ 0x9490,	0x00000000, CHIP_FAMILY_RV730,		"ATI Radeon HD 4600 Series",		kFlicker	},
	{ 0x9498,	0x00000000, CHIP_FAMILY_RV730,		"ATI Radeon HD 4600 Series",		kFlicker	},
	
	{ 0x94B3,	0x00000000, CHIP_FAMILY_RV740,		"ATI Radeon HD 4770",				kFlicker	},
	{ 0x94B4,	0x00000000, CHIP_FAMILY_RV740,		"ATI Radeon HD 4700 Series",		kFlicker	},
	{ 0x94B5,	0x00000000, CHIP_FAMILY_RV740,		"ATI Radeon HD 4770",				kFlicker	},
	
	{ 0x94C1,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",		kIago		},
	{ 0x94C3,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",		kIago		},
	{ 0x94C7,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2350",				kIago		},
	{ 0x94CC,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",		kIago		},
	
	{ 0x9501,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3800 Series",		kMegalodon	},
	{ 0x9505,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3800 Series",		kMegalodon	},
	{ 0x9507,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3830",				kMegalodon	},
	{ 0x950F,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3870 X2",			kMegalodon	},
	
	{ 0x9513,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3850 X2",			kMegalodon	},
	{ 0x9519,	0x00000000, CHIP_FAMILY_RV670,		"AMD FireStream 9170",				kMegalodon	},
	
	{ 0x9540,	0x00000000, CHIP_FAMILY_RV710,		"ATI Radeon HD 4550",				kNull		},
	{ 0x954F,	0x00000000, CHIP_FAMILY_RV710,		"ATI Radeon HD 4300/4500 Series",	kNull		},
	
	{ 0x9553,	0x00000000, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4500/5100 Series",	kShrike },
	
	{ 0x9588,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 XT",			kLamna		},
	{ 0x9589,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 PRO",			kLamna		},
	{ 0x958A,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 X2 Series",		kLamna		},
	
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
	
	{ 0x68F9,	0x00000000, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5400 Series",		kNull		},
	
	/* Northen Islands */
	{ 0x6718,	0x00000000, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970 Series",		kNull		},
	{ 0x6719,	0x00000000, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6950 Series",		kNull		},
	
	{ 0x6720,	0x00000000, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6900M Series",		kNull		},
	
	{ 0x6738,	0x00000000, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870 Series",		kDuckweed	},
	{ 0x6739,	0x00000000, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850 Series",		kDuckweed	},
	{ 0x673E,	0x00000000, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6790 Series",		kNull		},
	
	{ 0x6740,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6700M Series",		kNull		},
	{ 0x6741,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6600M/6700M Series",	kNull		},
	
	{ 0x6758,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670 Series",		kBulrushes	},
	{ 0x6759,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6500 Series",		kNull		},

	{ 0x6760,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6400M Series",		kNull		},
	{ 0x6761,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6430M Series",		kNull		},
		
	{ 0x6770,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6400 Series",		kNull		},
	{ 0x6779,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450 Series",		kBulrushes	},
	
	{ 0x0000,	0x00000000, CHIP_FAMILY_UNKNOW,		NULL,								kNull		}
};

typedef struct {
	struct DevPropDevice	*device;
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
card_t *card;

/* Flags */
#define MKFLAG(n)			(1 << n)
#define FLAGTRUE			MKFLAG(0)
#define EVERGREEN			MKFLAG(1)

//static UINT8 atN = 0;

typedef struct {
	type_t					type;
	UINT32				size;
	UINT8					*data;
} value_t;

static value_t aty_name;
static value_t aty_nameparent;
//static value_t aty_model;

#define DATVAL(x)			{kPtr, sizeof(x), (UINT8 *)x}
#define STRVAL(x)			{kStr, sizeof(x), (UINT8 *)x}
#define BYTVAL(x)			{kCst, 1, (UINT8 *)x}
#define WRDVAL(x)			{kCst, 2, (UINT8 *)x}
#define DWRVAL(x)			{kCst, 4, (UINT8 *)x}
#define QWRVAL(x)			{kCst, 8, (UINT8 *)x}
#define NULVAL				{kNul, 0, (UINT8 *)NULL}

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

typedef struct {
	UINT32				flags;
	BOOLEAN					all_ports;
	CHAR8					*name;
	BOOLEAN					(*get_value)(value_t *val);
	value_t					default_val;
} dev_prop_t;

dev_prop_t ati_devprop_list[] = {
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

BOOLEAN get_bootdisplay_val(value_t *val)
{
	static UINT32 v = 0;
	
	if (v)
		return FALSE;
	
	if (!card->posted)
		return FALSE;
	
	v = 1;
	val->type = kCst;
	val->size = 4;
	val->data = (UINT8 *)&v;
	
	return TRUE;
}

BOOLEAN get_vrammemory_val(value_t *val)
{
	return FALSE;
}

BOOLEAN get_name_val(value_t *val)
{
	val->type = aty_name.type;
	val->size = aty_name.size;
	val->data = aty_name.data;
	
	return TRUE;
}

BOOLEAN get_nameparent_val(value_t *val)
{
	val->type = aty_nameparent.type;
	val->size = aty_nameparent.size;
	val->data = aty_nameparent.data;
	
	return TRUE;
}

BOOLEAN get_model_val(value_t *val)
{
	if (!card->info->model_name)
		return FALSE;
	
	val->type = kStr;
	val->size = AsciiStrLen(card->info->model_name) + 1;
	val->data = (UINT8 *)card->info->model_name;
	
	return TRUE;
}

BOOLEAN get_conntype_val(value_t *val)
{
//Connector types:
//0x4 : DisplayPort
//0x400: DL DVI-I
//0x800: HDMI

	return FALSE;
}

BOOLEAN get_vrammemsize_val(value_t *val)
{
	static INTN idx = -1;
	static UINT64 memsize;
	
	idx++;
	memsize = ((UINT64)card->vram_size << 32);
	if (idx == 0)
		memsize = memsize | (UINT64)card->vram_size;
	
	val->type = kCst;
	val->size = 8;
	val->data = (UINT8 *)&memsize;
	
	return TRUE;
}

BOOLEAN get_binimage_val(value_t *val)
{
	if (!card->rom)
		return FALSE;
	
	val->type = kPtr;
	val->size = card->rom_size;
	val->data = card->rom;
	
	return TRUE;
}

BOOLEAN get_romrevision_val(value_t *val)
{
	UINT8 *rev;
	if (!card->rom)
		return FALSE;
	
	rev = card->rom + *(UINT8 *)(card->rom + OFFSET_TO_GET_ATOMBIOS_STRINGS_START);

	val->type = kPtr;
	val->size = AsciiStrLen((CHAR8 *)rev);
	val->data = AllocateZeroPool(val->size);
	
	if (!val->data)
		return FALSE;
	
	CopyMem(val->data, rev, val->size);
	
	return TRUE;
}

BOOLEAN get_deviceid_val(value_t *val)
{
	val->type = kCst;
	val->size = 2;
	val->data = (UINT8 *)&card->pci_dev->device_id;
	
	return TRUE;
}

BOOLEAN get_mclk_val(value_t *val)
{
	return FALSE;
}

BOOLEAN get_sclk_val(value_t *val)
{
	return FALSE;
}

BOOLEAN get_refclk_val(value_t *val)
{
	return FALSE;
}

BOOLEAN get_platforminfo_val(value_t *val)
{
	val->data = AllocateZeroPool(0x80);
	if (!val->data)
		return FALSE;
	
//	bzero(val->data, 0x80);
	
	val->type		= kPtr;
	val->size		= 0x80;
	val->data[0]	= 1;
	
	return TRUE;
}

BOOLEAN get_vramtotalsize_val(value_t *val)
{
	val->type = kCst;
	val->size = 4;
	val->data = (UINT8 *)&card->vram_size;
	
	return TRUE;
}

void free_val(value_t *val)
{
	if (val->type == kPtr)
		FreePool(val->data);
	
	ZeroMem(val, sizeof(value_t));
}

void devprop_add_list(dev_prop_t devprop_list[])
{
	value_t *val = AllocateZeroPool(sizeof(value_t));
	int i, pnum;
	
	for (i = 0; devprop_list[i].name != NULL; i++)
	{
		if ((devprop_list[i].flags == FLAGTRUE) || (devprop_list[i].flags | card->flags))
		{
			if (devprop_list[i].get_value && devprop_list[i].get_value(val))
			{
				devprop_add_value(card->device, devprop_list[i].name, val->data, val->size);
				free_val(val);
				
				if (devprop_list[i].all_ports)
				{
					for (pnum = 1; pnum < card->ports; pnum++)
					{
						if (devprop_list[i].get_value(val))
						{
							devprop_list[i].name[1] = 0x30 + pnum; // convert to ascii
							devprop_add_value(card->device, devprop_list[i].name, val->data, val->size);
							free_val(val);
						}
					}
					devprop_list[i].name[1] = 0x30; // write back our "@0," for a next possible card
				}
			}
			else
			{
				if (devprop_list[i].default_val.type != kNul)
				{
					devprop_add_value(card->device, devprop_list[i].name,
						devprop_list[i].default_val.type == kCst ?
						(UINT8 *)&(devprop_list[i].default_val.data) : devprop_list[i].default_val.data,
						devprop_list[i].default_val.size);
				}
				
				if (devprop_list[i].all_ports)
				{
					for (pnum = 1; pnum < card->ports; pnum++)
					{
						if (devprop_list[i].default_val.type != kNul)
						{
							devprop_list[i].name[1] = 0x30 + pnum; // convert to ascii
							devprop_add_value(card->device, devprop_list[i].name,
								devprop_list[i].default_val.type == kCst ?
								(UINT8 *)&(devprop_list[i].default_val.data) : devprop_list[i].default_val.data,
								devprop_list[i].default_val.size);
						}
					}
					devprop_list[i].name[1] = 0x30; // write back our "@0," for a next possible card
				}
			}
		}
	}

	FreePool(val);
}

BOOLEAN validate_rom(option_rom_header_t *rom_header, pci_dt_t *pci_dev)
{
	option_rom_pci_header_t *rom_pci_header;
	
	if (rom_header->signature != 0xaa55)
		return FALSE;
	
	rom_pci_header = (option_rom_pci_header_t *)((UINT8 *)rom_header + rom_header->pci_header_offset);
	
	if (rom_pci_header->signature != 0x52494350)
		return FALSE;
	
	if (rom_pci_header->vendor_id != pci_dev->vendor_id || rom_pci_header->device_id != pci_dev->device_id)
		return FALSE;
	
	return TRUE;
}

BOOLEAN load_vbios_file(const CHAR8 *key, UINT16 vendor_id, UINT16 device_id, UINT32 subsys_id)
{
	UINTN bufferLen;
	CHAR16 FileName[24];
//	BOOLEAN do_load = FALSE;
  UINT8*  buffer;
	
//	getBoolForKey(key, &do_load, &bootInfo->chameleonConfig);
	if (!gSettings.LoadVBios)
		return FALSE;
	
	UnicodeSPrint(FileName, 24, L"/EFI/device/%04x_%04x_%08x.rom", vendor_id, device_id, subsys_id);
	if (!FileExists(SelfRootDir, FileName))
		return FALSE;
//	Status = 
  egLoadFile(SelfRootDir, FileName, &buffer, &bufferLen);
  
	card->rom_size = bufferLen;
	card->rom = AllocateZeroPool(bufferLen);
	if (!card->rom)
		return FALSE;
	CopyMem(card->rom, buffer, bufferLen);
//	read(fd, (CHAR8 *)card->rom, card->rom_size);
	
	if (!validate_rom((option_rom_header_t *)card->rom, card->pci_dev))
	{
		card->rom_size = 0;
		card->rom = 0;
		return FALSE;
	}
	
	card->rom_size = ((option_rom_header_t *)card->rom)->rom_size * 512;
	
//	close(fd);
  FreePool(buffer);
	
	return TRUE;
}

void get_vram_size(void)
{
	chip_family_t chip_family = card->info->chip_family;
	
	card->vram_size = 0;
	
	if (chip_family >= CHIP_FAMILY_CEDAR)
		// size in MB on evergreen
		// XXX watch for overflow!!!
		card->vram_size = RegRead32(R600_CONFIG_MEMSIZE) * 1024 * 1024;
	else
		if (chip_family >= CHIP_FAMILY_R600)
			card->vram_size = RegRead32(R600_CONFIG_MEMSIZE);
  else {
    card->vram_size = RegRead32(RADEON_CONFIG_MEMSIZE);
    if (card->vram_size == 0) {
      card->vram_size = RegRead32(RADEON_CONFIG_APER_SIZE);
      //Slice - previously I successfully made Radeon9000 working
      //by writing this register
      RegWrite32(RADEON_CONFIG_MEMSIZE, 0x30000);
    }
  }
}

BOOLEAN read_vbios(BOOLEAN from_pci)
{
	option_rom_header_t *rom_addr;
	
	if (from_pci)
	{
		rom_addr = (option_rom_header_t *)(UINTN)(pci_config_read32(card->pci_dev->dev.addr, PCI_EXPANSION_ROM_BASE) & ~0x7ff);
		DBG(" @0x%x", rom_addr);
	}
	else
		rom_addr = (option_rom_header_t *)0xc0000;
	
	if (!validate_rom(rom_addr, card->pci_dev)){
    Print(L"There is no ROM @C0000\n");
    gBS->Stall(3000000);
		return FALSE;
  }
	
	card->rom_size = rom_addr->rom_size * 512;
	if (!card->rom_size)
		return FALSE;
	
	card->rom = AllocateZeroPool(card->rom_size);
	if (!card->rom)
		return FALSE;
	
	CopyMem(card->rom, (void *)rom_addr, card->rom_size);
	
	return TRUE;
}

BOOLEAN read_disabled_vbios(void)
{
	BOOLEAN ret = FALSE;
	chip_family_t chip_family = card->info->chip_family;
	
	if (chip_family >= CHIP_FAMILY_RV770)
	{
		UINT32 viph_control		= RegRead32(RADEON_VIPH_CONTROL);
		UINT32 bus_cntl			= RegRead32(RADEON_BUS_CNTL);
		UINT32 d1vga_control		= RegRead32(AVIVO_D1VGA_CONTROL);
		UINT32 d2vga_control		= RegRead32(AVIVO_D2VGA_CONTROL);
		UINT32 vga_render_control = RegRead32(AVIVO_VGA_RENDER_CONTROL);
		UINT32 rom_cntl			= RegRead32(R600_ROM_CNTL);
		UINT32 cg_spll_func_cntl	= 0;
		UINT32 cg_spll_status;
		
		// disable VIP
		RegWrite32(RADEON_VIPH_CONTROL, (viph_control & ~RADEON_VIPH_EN));
		
		// enable the rom
		RegWrite32(RADEON_BUS_CNTL, (bus_cntl & ~RADEON_BUS_BIOS_DIS_ROM));
		
		// Disable VGA mode
		RegWrite32(AVIVO_D1VGA_CONTROL, (d1vga_control & ~(AVIVO_DVGA_CONTROL_MODE_ENABLE | AVIVO_DVGA_CONTROL_TIMING_SELECT)));
		RegWrite32(AVIVO_D2VGA_CONTROL, (d2vga_control & ~(AVIVO_DVGA_CONTROL_MODE_ENABLE | AVIVO_DVGA_CONTROL_TIMING_SELECT)));
		RegWrite32(AVIVO_VGA_RENDER_CONTROL, (vga_render_control & ~AVIVO_VGA_VSTATUS_CNTL_MASK));
		
		if (chip_family == CHIP_FAMILY_RV730)
		{
			cg_spll_func_cntl = RegRead32(R600_CG_SPLL_FUNC_CNTL);
			
			// enable bypass mode
			RegWrite32(R600_CG_SPLL_FUNC_CNTL, (cg_spll_func_cntl | R600_SPLL_BYPASS_EN));
			
			// wait for SPLL_CHG_STATUS to change to 1
			cg_spll_status = 0;
			while (!(cg_spll_status & R600_SPLL_CHG_STATUS))
				cg_spll_status = RegRead32(R600_CG_SPLL_STATUS);
			
			RegWrite32(R600_ROM_CNTL, (rom_cntl & ~R600_SCK_OVERWRITE));
		}
		else
			RegWrite32(R600_ROM_CNTL, (rom_cntl | R600_SCK_OVERWRITE));
		
		ret = read_vbios(TRUE);
		
		// restore regs
		if (chip_family == CHIP_FAMILY_RV730)
		{
			RegWrite32(R600_CG_SPLL_FUNC_CNTL, cg_spll_func_cntl);
			
			// wait for SPLL_CHG_STATUS to change to 1
			cg_spll_status = 0;
			while (!(cg_spll_status & R600_SPLL_CHG_STATUS))
			cg_spll_status = RegRead32(R600_CG_SPLL_STATUS);
		}
		RegWrite32(RADEON_VIPH_CONTROL, viph_control);
		RegWrite32(RADEON_BUS_CNTL, bus_cntl);
		RegWrite32(AVIVO_D1VGA_CONTROL, d1vga_control);
		RegWrite32(AVIVO_D2VGA_CONTROL, d2vga_control);
		RegWrite32(AVIVO_VGA_RENDER_CONTROL, vga_render_control);
		RegWrite32(R600_ROM_CNTL, rom_cntl);
	}
	else
		if (chip_family >= CHIP_FAMILY_R600)
		{
			UINT32 viph_control				= RegRead32(RADEON_VIPH_CONTROL);
			UINT32 bus_cntl					= RegRead32(RADEON_BUS_CNTL);
			UINT32 d1vga_control				= RegRead32(AVIVO_D1VGA_CONTROL);
			UINT32 d2vga_control				= RegRead32(AVIVO_D2VGA_CONTROL);
			UINT32 vga_render_control			= RegRead32(AVIVO_VGA_RENDER_CONTROL);
			UINT32 rom_cntl					= RegRead32(R600_ROM_CNTL);
			UINT32 general_pwrmgt				= RegRead32(R600_GENERAL_PWRMGT);
			UINT32 low_vid_lower_gpio_cntl	= RegRead32(R600_LOW_VID_LOWER_GPIO_CNTL);
			UINT32 medium_vid_lower_gpio_cntl = RegRead32(R600_MEDIUM_VID_LOWER_GPIO_CNTL);
			UINT32 high_vid_lower_gpio_cntl	= RegRead32(R600_HIGH_VID_LOWER_GPIO_CNTL);
			UINT32 ctxsw_vid_lower_gpio_cntl	= RegRead32(R600_CTXSW_VID_LOWER_GPIO_CNTL);
			UINT32 lower_gpio_enable			= RegRead32(R600_LOWER_GPIO_ENABLE);
			
			// disable VIP
			RegWrite32(RADEON_VIPH_CONTROL, (viph_control & ~RADEON_VIPH_EN));
			
			// enable the rom
			RegWrite32(RADEON_BUS_CNTL, (bus_cntl & ~RADEON_BUS_BIOS_DIS_ROM));
			
			// Disable VGA mode
			RegWrite32(AVIVO_D1VGA_CONTROL, (d1vga_control & ~(AVIVO_DVGA_CONTROL_MODE_ENABLE | AVIVO_DVGA_CONTROL_TIMING_SELECT)));
			RegWrite32(AVIVO_D2VGA_CONTROL, (d2vga_control & ~(AVIVO_DVGA_CONTROL_MODE_ENABLE | AVIVO_DVGA_CONTROL_TIMING_SELECT)));
			RegWrite32(AVIVO_VGA_RENDER_CONTROL, (vga_render_control & ~AVIVO_VGA_VSTATUS_CNTL_MASK));
			RegWrite32(R600_ROM_CNTL, ((rom_cntl & ~R600_SCK_PRESCALE_CRYSTAL_CLK_MASK) | (1 << R600_SCK_PRESCALE_CRYSTAL_CLK_SHIFT) | R600_SCK_OVERWRITE));
			RegWrite32(R600_GENERAL_PWRMGT, (general_pwrmgt & ~R600_OPEN_DRAIN_PADS));
			RegWrite32(R600_LOW_VID_LOWER_GPIO_CNTL, (low_vid_lower_gpio_cntl & ~0x400));
			RegWrite32(R600_MEDIUM_VID_LOWER_GPIO_CNTL, (medium_vid_lower_gpio_cntl & ~0x400));
			RegWrite32(R600_HIGH_VID_LOWER_GPIO_CNTL, (high_vid_lower_gpio_cntl & ~0x400));
			RegWrite32(R600_CTXSW_VID_LOWER_GPIO_CNTL, (ctxsw_vid_lower_gpio_cntl & ~0x400));
			RegWrite32(R600_LOWER_GPIO_ENABLE, (lower_gpio_enable | 0x400));
			
			ret = read_vbios(TRUE);
			
			// restore regs
			RegWrite32(RADEON_VIPH_CONTROL, viph_control);
			RegWrite32(RADEON_BUS_CNTL, bus_cntl);
			RegWrite32(AVIVO_D1VGA_CONTROL, d1vga_control);
			RegWrite32(AVIVO_D2VGA_CONTROL, d2vga_control);
			RegWrite32(AVIVO_VGA_RENDER_CONTROL, vga_render_control);
			RegWrite32(R600_ROM_CNTL, rom_cntl);
			RegWrite32(R600_GENERAL_PWRMGT, general_pwrmgt);
			RegWrite32(R600_LOW_VID_LOWER_GPIO_CNTL, low_vid_lower_gpio_cntl);
			RegWrite32(R600_MEDIUM_VID_LOWER_GPIO_CNTL, medium_vid_lower_gpio_cntl);
			RegWrite32(R600_HIGH_VID_LOWER_GPIO_CNTL, high_vid_lower_gpio_cntl);
			RegWrite32(R600_CTXSW_VID_LOWER_GPIO_CNTL, ctxsw_vid_lower_gpio_cntl);
			RegWrite32(R600_LOWER_GPIO_ENABLE, lower_gpio_enable);
		}

	return ret;
}

BOOLEAN radeon_card_posted(void)
{
	UINT32 reg;
	
	// first check CRTCs
	reg = RegRead32(RADEON_CRTC_GEN_CNTL) | RegRead32(RADEON_CRTC2_GEN_CNTL);
	if (reg & RADEON_CRTC_EN)
		return TRUE;
	
	// then check MEM_SIZE, in case something turned the crtcs off
	reg = RegRead32(R600_CONFIG_MEMSIZE);
	if (reg)
		return TRUE;
	
	return FALSE;
}

#if 0
BOOLEAN devprop_add_pci_config_space(void)
{
	int offset;
	
	UINT8 *config_space = AllocateZeroPool(0x100);
	if (!config_space)
		return FALSE;
	
	for (offset = 0; offset < 0x100; offset += 4)
		config_space[offset / 4] = pci_config_read32(card->pci_dev->dev.addr, offset);
	
	devprop_add_value(card->device, "ATY,PCIConfigSpace", config_space, 0x100);
	FreePool(config_space);
	
	return TRUE;
}
#endif

static BOOLEAN init_card(pci_dt_t *pci_dev)
{
//	BOOLEAN	add_vbios = TRUE;
	CHAR8	name[24];
	CHAR8	name_parent[24];
	int		i;
//	int		n_ports = 0;
	
	card = AllocateZeroPool(sizeof(card_t));
	if (!card)
		return FALSE;
//	bzero(card, sizeof(card_t));
	
	card->pci_dev = pci_dev;
	
	for (i = 0; radeon_cards[i].device_id ; i++)
	{
		if (radeon_cards[i].device_id == pci_dev->device_id)
		{
			card->info = &radeon_cards[i];
			if ((radeon_cards[i].subsys_id == 0x00000000) ||
				(radeon_cards[i].subsys_id == pci_dev->subsys_id.subsys_id))
				break;
		}
	}
	
	if (!card->info->device_id || !card->info->cfg_name)
	{
		DBG("Unsupported ATI card! Device ID: [%04x:%04x] Subsystem ID: [%08x] \n", 
				pci_dev->vendor_id, pci_dev->device_id, pci_dev->subsys_id);
		return FALSE;
	}
	
	card->fb		= (UINT8 *)(UINTN)(pci_config_read32(pci_dev->dev.addr, PCI_BASE_ADDRESS_0) & ~0x0f);
	card->mmio		= (UINT8 *)(UINTN)(pci_config_read32(pci_dev->dev.addr, PCI_BASE_ADDRESS_2) & ~0x0f);
	card->io		= (UINT8 *)(UINTN)(pci_config_read32(pci_dev->dev.addr, PCI_BASE_ADDRESS_4) & ~0x03);
	
	DBG("Framebuffer @0x%08X  MMIO @0x%08X	I/O Port @0x%08X ROM Addr @0x%08X\n",
		card->fb, card->mmio, card->io, pci_config_read32(pci_dev->dev.addr, PCI_EXPANSION_ROM_BASE));
	
	card->posted = radeon_card_posted();
	DBG("ATI card %s, ", card->posted ? "POSTed" : "non-POSTed");
	
	get_vram_size();
	
//	getBoolForKey(kATYbinimage, &add_vbios, &bootInfo->chameleonConfig);
/*	
	if (gSettings.LoadVBios)
	{
		if (!load_vbios_file(kUseAtiROM, pci_dev->vendor_id, pci_dev->device_id, pci_dev->subsys_id.subsys_id))
		{
			DBG("reading VBIOS from %s", card->posted ? "legacy space" : "PCI ROM");
			if (card->posted)
				read_vbios(FALSE);
			else
				read_disabled_vbios();
			DBG("\n");
		}
	}
 */
	
//	card->ports = 2; // default - Azi: default is card_configs
	
	if (card->info->chip_family >= CHIP_FAMILY_CEDAR)
	{
		card->flags |= EVERGREEN;
//		card->ports = 3; //Azi: use the AtiPorts key if needed
	}
	if (gSettings.VideoPorts) {
    card->ports = gSettings.VideoPorts;
  }
	
//	atN = 0;
	
	// Check AtiConfig key for a framebuffer name,
  //	card->cfg_name = getStringForKey(kAtiConfig, &bootInfo->chameleonConfig);
  UnicodeStrToAsciiStr((CHAR16*)&gSettings.FBName[0],(CHAR8*)&card->cfg_name[0]);
	// if none,
	if (AsciiStrLen(card->cfg_name) == 0)
	{
		// use cfg_name on radeon_cards, to retrive the default name from card_configs,
		card->cfg_name = card_configs[card->info->cfg_name].name;
		// and leave ports alone!
//		card->ports = card_configs[card->info->cfg_name].ports;
		
		// which means one of the fb's or kNull
		DBG("Framebuffer set to device's default: %s\n", card->cfg_name);
	}
	else
	{
		// else, use the fb name returned by AtiConfig.
		DBG("(AtiConfig) Framebuffer set to: %s\n", card->cfg_name);
	}
	
	// Check AtiPorts key for nr of ports,
  //	card->ports = getIntForKey(kAtiPorts, &n_ports, &bootInfo->chameleonConfig);
  
	// if a value bigger than 0 ?? is found, (do we need >= 0 ?? that's null FB on card_configs)
	if (gSettings.VideoPorts > 0)
	{
		card->ports = gSettings.VideoPorts; // use it.
		DBG("(AtiPorts) Nr of ports set to: %d\n", card->ports);
    }
	else// if (card->cfg_name > 0) // do we want 0 ports if fb is kNull or mistyped ?
	{
		// else, match cfg_name with card_configs list and retrive default nr of ports.
		for (i = 0; i < kCfgEnd; i++)
			if (AsciiStrCmp(card->cfg_name, card_configs[i].name) == 0)
				card->ports = card_configs[i].ports; // default
		
		DBG("Nr of ports set to framebuffer's default: %d\n", card->ports);
	}
//	else
//		card->ports = 2/1 ?; // set a min if 0 ports ?
//		DBG("Nr of ports set to min: %d\n", card->ports);
	
	AsciiSPrint(name, 24, "ATY,%s", card->cfg_name);
	aty_name.type = kStr;
	aty_name.size = AsciiStrLen(name) + 1;
	aty_name.data = (UINT8 *)name;
	
	AsciiSPrint(name_parent, 24, "ATY,%sParent", card->cfg_name);
	aty_nameparent.type = kStr;
	aty_nameparent.size = AsciiStrLen(name_parent) + 1;
	aty_nameparent.data = (UINT8 *)name_parent;
	
	return TRUE;
}

BOOLEAN setup_ati_devprop(pci_dt_t *ati_dev)
{
	CHAR8 *devicepath;
	
	if (!init_card(ati_dev))
		return FALSE;
	
	// -------------------------------------------------
	// Find a better way to do this (in device_inject.c)
	if (!string)
		string = devprop_create_string();
	
	devicepath = get_pci_dev_path(ati_dev);
	card->device = devprop_add_device(string, devicepath);
	if (!card->device)
		return FALSE;
	// -------------------------------------------------
	
#if 0
	UINT64 fb		= (UINT32)card->fb;
	UINT64 mmio	= (UINT32)card->mmio;
	UINT64 io		= (UINT32)card->io;
	devprop_add_value(card->device, "ATY,FrameBufferOffset", &fb, 8);
	devprop_add_value(card->device, "ATY,RegisterSpaceOffset", &mmio, 8);
	devprop_add_value(card->device, "ATY,IOSpaceOffset", &io, 8);
#endif
	
	devprop_add_list(ati_devprop_list);
	
	// -------------------------------------------------
	// Find a better way to do this (in device_inject.c)
	//Azi: XXX tried to fix a malloc error in vain; this is related to XCode 4 compilation!
	stringdata = AllocateZeroPool(sizeof(UINT8) * string->length);
	CopyMem(stringdata, (UINT8*)devprop_generate_string(string), string->length);
	stringlength = string->length;
	// -------------------------------------------------
	
	DBG("ATI %s %s %dMB (%s) [%04x:%04x] (subsys [%04x:%04x]):: %s\n",
			chip_family_name[card->info->chip_family], card->info->model_name,
			(UINT32)(card->vram_size / (1024 * 1024)), card->cfg_name,
			ati_dev->vendor_id, ati_dev->device_id,
			ati_dev->subsys_id.subsys.vendor_id, ati_dev->subsys_id.subsys.device_id,
			devicepath);
	
	FreePool(card);
	
	return TRUE;
}
