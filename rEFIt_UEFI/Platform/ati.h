/*
 *  ati.h
 *  
 *  Created by Slice on 19.02.12.
 *  
 *  the code ported from Chameleon project as well as from RadeonFB by Joblo and RadeonHD by dong
 *  bis thank to Islam M. Ahmed Zaid for the updating the collection
 */

#include "Platform.h"  //this include needed for Uefi types
#include "ati_reg.h"

#define OFFSET_TO_GET_ATOMBIOS_STRINGS_START 0x6e
#define DATVAL(x)			{kPtr, sizeof(x), (UINT8 *)x}
#define STRVAL(x)			{kStr, sizeof(x)-1, (UINT8 *)x}
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
//X1000 0x71871002 0x72101002 0x71DE1002 0x71461002 0x71421002 0x71091002 0x71C51002
//      0x71C01002 0x72401002 0x72491002 0x72911002  
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
  { 0x7140, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",		kCaretta		 },
  { 0x7141, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",		kCaretta		 },
  { 0x7142, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",		kCaretta		 },
  { 0x7143, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",		kCaretta		 },
  { 0x7144, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",			kCaretta		 },
  { 0x7145, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",			kCaretta		 },
  //7146, 7187 - Caretta
  { 0x7146, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",		kCaretta		 },
  { 0x7147, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",		kCaretta		 },
  { 0x7149, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",			kCaretta		 },
  { 0x714A, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",			kCaretta		 },
  { 0x714B, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",			kCaretta		 },
  { 0x714C, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",			kCaretta		 },
  { 0x714D, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",		kCaretta		 },
  { 0x714E, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",		kCaretta		 },
  { 0x714F, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",		kCaretta		 },
  { 0x7151, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",		kCaretta		 },
  { 0x7152, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",		kCaretta		 },
  { 0x7153, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",		kCaretta		 },
  { 0x715E, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",		kCaretta		 },
  { 0x715F, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",		kCaretta		 },
  { 0x7180, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",		kCaretta		 },
  { 0x7181, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",		kCaretta		 },
  { 0x7183, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",		kCaretta		 },
  { 0x7186, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",			kCaretta		 },
  { 0x7187, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",		kCaretta		 },
  { 0x7188, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",			kCaretta		 },
  { 0x718A, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",			kCaretta		 },
  { 0x718B, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",			kCaretta		 },
  { 0x718C, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",			kCaretta		 },
  { 0x718D, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",			kCaretta		 },
  { 0x718F, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",		kCaretta		 },
  { 0x7193, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",		kCaretta		 },
  { 0x7196, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",			kCaretta		 },
  { 0x719B, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",		kCaretta		 },
  { 0x719F, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",		kCaretta		 },
  { 0x71C0, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",		kWormy		 },
  { 0x71C1, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",		kWormy		 },
  { 0x71C2, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",		kWormy		 },
  { 0x71C3, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",		kWormy		 },
  { 0x71C4, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Mobile ",			kWormy		 },
  //71c5 -Wormy
  { 0x71C5, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Mobile ",			kWormy		 },
  { 0x71C6, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",		kWormy		 },
  { 0x71C7, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",		kWormy		 },
  { 0x71CD, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",		kWormy		 },
  { 0x71CE, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",		kWormy		 },
  { 0x71D2, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",		kWormy		 },
  { 0x71D4, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Mobile ",			kWormy		 },
  { 0x71D5, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Mobile ",			kWormy		 },
  { 0x71D6, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Mobile ",			kWormy		 },
  { 0x71DA, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",		kWormy		 },
  { 0x71DE, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Mobile ",			kWormy		 },
  { 0x7200, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",		kWormy		 },
  { 0x7210, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Mobile ",			kWormy		 },
  { 0x7211, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Mobile ",			kWormy		 },
  { 0x7240, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kAlopias		 },
  { 0x7243, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kAlopias		 },
  { 0x7244, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kAlopias		 },
  { 0x7245, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kAlopias		 },
  { 0x7246, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kAlopias		 },
  { 0x7247, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kAlopias		 },
  { 0x7248, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kAlopias		 },
  //7249 -Alopias
  { 0x7249, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kAlopias		 },
  { 0x724A, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kAlopias		 },
  { 0x724B, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kAlopias		 },
  { 0x724C, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kAlopias		 },
  { 0x724D, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kAlopias		 },
  { 0x724E, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kAlopias		 },
  { 0x724F, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Desktop ",			kAlopias		 },
  { 0x7280, 0x00000000, CHIP_FAMILY_RV570, "ATI Radeon HD Desktop ",		kAlopias		 },
  { 0x7281, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",		kAlopias		 },
  { 0x7283, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",		kAlopias		 },
  { 0x7284, 0x00000000, CHIP_FAMILY_R580, "ATI Radeon HD Mobile ",			kAlopias		 },
  { 0x7287, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",		kAlopias		 },
  { 0x7288, 0x00000000, CHIP_FAMILY_RV570, "ATI Radeon HD Desktop ",		kAlopias		 },
  { 0x7289, 0x00000000, CHIP_FAMILY_RV570, "ATI Radeon HD Desktop ",		kAlopias		 },
  { 0x728B, 0x00000000, CHIP_FAMILY_RV570, "ATI Radeon HD Desktop ",		kAlopias		 },
  { 0x728C, 0x00000000, CHIP_FAMILY_RV570, "ATI Radeon HD Desktop ",		kAlopias		 },
  { 0x7290, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",		kAlopias		 },
  { 0x7291, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",		kAlopias		 },
  { 0x7293, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",		kAlopias		 },
  { 0x7297, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",		kAlopias		 },
//IGP  
  { 0x791E, 0x00000000, CHIP_FAMILY_RS690, "ATI Radeon IGP ",			kNull		 },
  { 0x791F, 0x00000000, CHIP_FAMILY_RS690, "ATI Radeon IGP ",			kNull		},
  { 0x796C, 0x00000000, CHIP_FAMILY_RS740, "ATI Radeon IGP ",			kNull		 },
  { 0x796D, 0x00000000, CHIP_FAMILY_RS740, "ATI Radeon IGP ",			kNull		},
  { 0x796E, 0x00000000, CHIP_FAMILY_RS740, "ATI Radeon IGP ",			kNull		},
  { 0x796F, 0x00000000, CHIP_FAMILY_RS740, "ATI Radeon HD ",			kNull		 },
  
	
//X2000 0x94001002 0x94011002 0x94021002 0x94031002 0x95811002 0x95831002 0x95881002 0x94c81002 0x94c91002 
//      0x95001002 0x95011002 0x95051002 0x95071002 0x95041002 0x95061002 0x95981002 0x94881002 0x95991002
//      0x95911002 0x95931002 0x94401002 0x94421002 0x944A1002 0x945A1002 0x94901002 0x949E1002 0x94801002
//      0x95401002 0x95411002 0x954E1002 0x954F1002 0x95521002 0x95531002 0x94a01002  
	/* standard/default models */
	{ 0x9400,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 XT",			kNull		},
	{ 0x9401,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 GT",			kNull		},
	{ 0x9402,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 GT",			kNull		},
	{ 0x9403,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 GT",			kNull		},
	{ 0x9405,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 GT",			kNull		},
	{ 0x940A,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 GT",			kNull		},
	{ 0x940B,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 GT",			kNull		},
	{ 0x940F,	0x00000000, CHIP_FAMILY_R600,		"ATI Radeon HD 2900 GT",			kNull		},
//9440, 944A - Cardinal	
	{ 0x9440,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4870 ",		kMotmot		},
	{ 0x9441,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4870 X2",			kMotmot		},
	{ 0x9442,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850 Series",		kMotmot		},
	{ 0x9443,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4850 X2",			kMotmot		},
	{ 0x944A,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",		kMotmot		},
	{ 0x944C,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4830 Series",		kMotmot		},
	{ 0x944E,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4810 Series",		kMotmot		},
	
	{ 0x9450,	0x00000000, CHIP_FAMILY_RV770,		"AMD FireStream 9270",				kMotmot		},
	{ 0x9452,	0x00000000, CHIP_FAMILY_RV770,		"AMD FireStream 9250",				kMotmot		},
	
	{ 0x9460,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",		kMotmot		},
	{ 0x9462,	0x00000000, CHIP_FAMILY_RV770,		"ATI Radeon HD 4800 Series",		kMotmot		},
//9488, 9490 - Gliff
	{ 0x9480,	0x00000000, CHIP_FAMILY_RV730,		"ATI Radeon HD 4650 Series",		kGliff	},
	{ 0x9488,	0x00000000, CHIP_FAMILY_RV730,		"ATI Radeon HD 4650 Series",		kGliff	},
	{ 0x9490,	0x00000000, CHIP_FAMILY_RV730,		"ATI Radeon HD 4710 Series",		kGliff	},
	{ 0x9498,	0x00000000, CHIP_FAMILY_RV730,		"ATI Radeon HD 4710 Series",		kGliff	},
	
	{ 0x94B3,	0x00000000, CHIP_FAMILY_RV740,		"ATI Radeon HD 4770",           kFlicker	},
	{ 0x94B4,	0x00000000, CHIP_FAMILY_RV740,		"ATI Radeon HD 4700 Series",		kFlicker	},
	{ 0x94B5,	0x00000000, CHIP_FAMILY_RV740,		"ATI Radeon HD 4770",           kFlicker	},
//94C8 -Iago	
	{ 0x94C1,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",		kIago		},
	{ 0x94C3,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2350 Series",		kIago		},
	{ 0x94C4,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",		kIago		},
	{ 0x94C5,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",		kIago		},
	{ 0x94C6,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",		kIago		},
	{ 0x94C7,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2350",           kIago		},
	{ 0x94C8,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",		kIago		},
	{ 0x94C9,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",		kIago		},
	{ 0x94CB,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",		kIago		},
	{ 0x94CC,	0x00000000, CHIP_FAMILY_RV610,		"ATI Radeon HD 2400 Series",		kIago		},
//9501 - Megalodon, Triakis HD3800
	{ 0x9500,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3800 Series",		kMegalodon	},
	{ 0x9501,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3690 Series",		kMegalodon	},
	{ 0x9505,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3800 Series",		kMegalodon	},
	{ 0x9507,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3830",           kMegalodon	},
	{ 0x950F,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3870 X2",        kMegalodon	},
	{ 0x9511,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3850 X2",        kMegalodon	},
	
	{ 0x9513,	0x00000000, CHIP_FAMILY_RV670,		"ATI Radeon HD 3850 X2",        kMegalodon	},
	{ 0x9519,	0x00000000, CHIP_FAMILY_RV670,		"AMD FireStream 9170",          kMegalodon	},
	
	{ 0x9540,	0x00000000, CHIP_FAMILY_RV710,		"ATI Radeon HD 4550",           kShrike		},
	{ 0x954F,	0x00000000, CHIP_FAMILY_RV710,		"ATI Radeon HD 4570 Series",    kShrike		},
	{ 0x9552,	0x00000000, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4300 Series",	kShrike },
	{ 0x9553,	0x00000000, CHIP_FAMILY_RV710,		"ATI Mobility Radeon HD 4330 Series",	kShrike },
  //9583, 9588 - Lamna, Hypoprion HD2600
  { 0x9581,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 3600 XT",          kLamna		},
  { 0x9583,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 XT",          kLamna		},	
	{ 0x9588,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 XT",          kLamna		},
	{ 0x9589,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 PRO",         kLamna		},
	{ 0x958A,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 X2 Series",		kLamna		},
	{ 0x958B,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 X2 Series",		kLamna		},
	{ 0x958C,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 X2 Series",		kLamna		},
	{ 0x958D,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 X2 Series",		kLamna		},
	{ 0x958E,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 X2 Series",		kLamna		},
	
	{ 0x9591,	0x00000000, CHIP_FAMILY_RV635,		"ATI Radeon HD 3600 Series",		kMegalodon	},
	{ 0x9598,	0x00000000, CHIP_FAMILY_RV635,		"ATI Radeon HD 3600 Series",		kMegalodon	},
	
	{ 0x95C0,	0x00000000, CHIP_FAMILY_RV620,		"ATI Radeon HD 3550 Series",		kIago		},
	{ 0x95C5,	0x00000000, CHIP_FAMILY_RV620,		"ATI Radeon HD 4250 Series",		kIago		},
	
	/* IGP */
	{ 0x9610,	0x00000000, CHIP_FAMILY_RS780,		"ATI Radeon HD 3200 Graphics",		kNull		},
	{ 0x9611,	0x00000000, CHIP_FAMILY_RS780,		"ATI Radeon 3100 Graphics",       kNull		},
	{ 0x9614,	0x00000000, CHIP_FAMILY_RS780,		"ATI Radeon HD 3300 Graphics",		kNull		},
	{ 0x9616,	0x00000000, CHIP_FAMILY_RS780,		"AMD 760G",                       kNull		},
	
	{ 0x9710,	0x00000000, CHIP_FAMILY_RS880,		"ATI Radeon HD 4200",				kNull		},
	{ 0x9715,	0x00000000, CHIP_FAMILY_RS880,		"ATI Radeon HD 4250",				kNull		},
	{ 0x9714,	0x00000000, CHIP_FAMILY_RS880,		"ATI Radeon HD 4290",				kNull		},
//X3000 - 0x68881002 0x68891002 0x68981002 0x68991002 0x689C1002 0x689D1002 0x68801002 0x68901002 0x68A81002
//0x68A91002 0x68B81002 0x68B91002  0x68BE1002 0x68A01002 0x68A11002 0x68B01002 0x68B11002  0x68C81002
//0x68C91002 0x68D81002 0x68D91002 0x68DE1002 0x68C01002 0x68C11002 0x68D01002 0x68D11002 0x68E81002
//0x68E91002 0x68F81002 0x68F91002 0x68FE1002 0x68E01002 0x68E11002 0x68F01002 0x68F11002  0x67011002
//0x67021002 0x67031002 0x67041002 0x67051002 0x67061002 0x67071002 0x67081002 0x67091002 0x67181002
//0x67191002 0x671C1002 0x671D1002 0x67221002 0x67231002 0x67261002 0x67271002 0x67281002 0x67291002
//0x67381002 0x67391002 0x67201002 0x67211002 0x67241002 0x67251002 0x67421002 0x67431002 0x67461002
//0x67471002 0x67481002 0x67491002 0x67501002 0x67581002 0x67591002 0x67401002 0x67411002 0x67441002
//0x67451002 0x67621002 0x67631002 0x67661002 0x67671002 0x67681002 0x67701002 0x67791002 0x67601002
//0x67611002 0x67641002 0x67651002	
	/* Evergreen */
//0x68981002 0x68991002 0x68E01002 0x68E11002 0x68D81002 0x68C01002 0x68C11002
//0x68D91002 0x68B81002 0x68B01002 0x68B11002 0x68A01002 0x68A11002 
//Hoolock, Langur, Orangutan, Zonalis  
	{ 0x688D,	0x00000000, CHIP_FAMILY_CYPRESS,	"AMD FireStream 9350",			  	kZonalis	},
	
	{ 0x6898,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5870 Series",		kZonalis	},
	{ 0x6899,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5850 Series",		kZonalis	},
	{ 0x689C,	0x00000000, CHIP_FAMILY_HEMLOCK,	"ATI Radeon HD 5900 Series",		kUakari		},
	{ 0x689E,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5800 Series",		kZonalis	},
  
  { 0x68A0,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770 Series",		kHoolock	},
  { 0x68A1,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770 Series",		kHoolock	},
  { 0x68A8,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770 Series",		kHoolock	},
  
  { 0x68B0,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770 Series",		kVervet		},
  { 0x68B1,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770 Series",		kVervet		},
	{ 0x68B8,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770 Series",		kVervet		},
	{ 0x68B9,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5700 Series",		kVervet		},
	{ 0x68BE,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5700 Series",		kVervet		},
	{ 0x68BF,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5700 Series",		kVervet		},
	
  { 0x68C0,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5730 Series",		kVervet		},
  { 0x68C1,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5650 Series",    kVervet		},	
  { 0x68C8,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5650 Series",    kVervet		},	
  { 0x68D8,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5670 Series",		kVervet		},
	{ 0x68D9,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5570 Series",		kVervet		},
	{ 0x68DA,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5500 Series",		kVervet		},
	{ 0x68E0,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5470 Series",		kVervet		},	
  
	{ 0x68F9,	0x00000000, CHIP_FAMILY_CEDAR,		"ATI Radeon HD 5400 Series",		kEulemur	},
	
	/* Northen Islands */
//0x67681002 0x67701002 0x67791002 0x67601002 0x67611002 0x67501002 0x67581002 0x67591002
//0x67401002 0x67411002 0x67451002 0x67381002 0x67391002 0x67201002 0x67221002 0x67181002  
//Gibba, Lotus, Muskgrass  
	{ 0x6718,	0x00000000, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970 Series",		kLotus		},
	{ 0x6719,	0x00000000, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6950 Series",		kLotus		},
	{ 0x671C,	0x00000000, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6970 Series",		kLotus		},
	{ 0x671D,	0x00000000, CHIP_FAMILY_CAYMAN,		"AMD Radeon HD 6950 Series",		kLotus		},
	
	{ 0x6720,	0x00000000, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6900M Series",		kDuckweed	},	
	{ 0x6738,	0x00000000, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870 Series",		kDuckweed  },
	{ 0x6739,	0x00000000, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850 Series",		kDuckweed  },
	{ 0x673E,	0x00000000, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6790 Series",		kDuckweed	},
	
	{ 0x6740,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6770M Series",		kDuckweed		},
	{ 0x6741,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6650M Series",	  kDuckweed		}, // kOsmunda },
	{ 0x6758,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670 Series",		kDuckweed   },
	{ 0x6759,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6500 Series",		kDuckweed   },
  
	{ 0x6760,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6400M Series",		kBulrushes	},
	{ 0x6761,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6430M Series",		kBulrushes	},
	{ 0x6770,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6400 Series",		kBulrushes	},
	{ 0x6779,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450 Series",		kBulrushes	},
	
	{ 0x0000,	0x00000000, CHIP_FAMILY_UNKNOW,		NULL,								kNull		}
};

typedef struct {
	DevPropDevice         *device;
	radeon_card_info_t		*info;
	pci_dt_t              *pci_dev;
	UINT8                 *fb;
	UINT8                 *mmio;
	UINT8                 *io;
	UINT8                 *rom;
	UINT32                rom_size;
	UINT64                vram_size;
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
	
  //{FLAGTRUE,	TRUE,	"@0,AAPL,vram-memory",		get_vrammemory_val,		NULVAL							},
  {FLAGTRUE,	TRUE,	"@0,compatible",		get_name_val,			NULVAL							},
  {FLAGTRUE,	TRUE,	"@0,connector-type",		get_conntype_val,		NULVAL							},
  {FLAGTRUE,	TRUE,	"@0,device_type",			NULL,					STRVAL("display")				},
  //	{FLAGTRUE,	FALSE,	"@0,display-connect-flags", NULL,					DWRVAL((UINT32)0)				},
  {FLAGTRUE,	TRUE,	"@0,display-type",			NULL,					STRVAL("NONE")					},
	{FLAGTRUE,	TRUE,	"@0,name",					get_name_val,			NULVAL							},
  {FLAGTRUE,	TRUE,	"@0,VRAM,memsize",			get_vrammemsize_val,	NULVAL							},
	
  {FLAGTRUE,	FALSE,	"AAPL,aux-power-connected", NULL,					DWRVAL((UINT32)1)				},
  {FLAGTRUE,	FALSE,	"AAPL,backlight-control",	NULL,					DWRVAL((UINT32)0)				},
	{FLAGTRUE,	FALSE,	"ATY,bin_image",	get_binimage_val,		NULVAL							},
	{FLAGTRUE,	FALSE,	"ATY,Copyright",	NULL,	STRVAL("Copyright AMD Inc. All Rights Reserved. 2005-2011") },
	{FLAGTRUE,	FALSE,	"ATY,Card#",			get_romrevision_val,	NULVAL							},
	{FLAGTRUE,	FALSE,	"ATY,VendorID",		NULL,					WRDVAL((UINT16)0x1002)		},
	{FLAGTRUE,	FALSE,	"ATY,DeviceID",		get_deviceid_val,		NULVAL							},
	
  //	{FLAGTRUE,	FALSE,	"ATY,MCLK",					get_mclk_val,			NULVAL							},
  //	{FLAGTRUE,	FALSE,	"ATY,SCLK",					get_sclk_val,			NULVAL							},
  //	{FLAGTRUE,	FALSE,	"ATY,RefCLK",				get_refclk_val,			DWRVAL((UINT32)0x0a8c)		},
	
  {FLAGTRUE,	FALSE,	"ATY,PlatformInfo",			get_platforminfo_val,	NULVAL							},
	
	{FLAGTRUE,	FALSE,	"name",						get_nameparent_val,		NULVAL							},
	{FLAGTRUE,	FALSE,	"device_type",		get_nameparent_val,		NULVAL							},
	{FLAGTRUE,	FALSE,	"model",					get_model_val,			STRVAL("ATI Radeon")			},
  {FLAGTRUE,	FALSE,	"VRAM,totalsize",			get_vramtotalsize_val,	NULVAL							},
	
	{FLAGTRUE,	FALSE,	NULL,						NULL,					NULVAL							}
};

