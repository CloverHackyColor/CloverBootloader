/*
 * ATI Graphics Card Enabler, part of the Chameleon Boot Loader Project
 *
 * Copyright 2010 by Islam M. Ahmed Zaid. All rights reserved.
 *
 */

#include "Platform.h"

#include "ati.h"

#define DEBUG_ATI 1

#if DEBUG_ATI == 2
#define DBG(x...) AsciiPrint(x)
#elif DEBUG_ATI == 1
#define DBG(x...) MsgLog(x)
#else
#define DBG(x...)
#endif


static value_t aty_name;
static value_t aty_nameparent;
card_t *card;
//static value_t aty_model;

card_config_t card_configs[] = {
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

radeon_card_info_t radeon_cards[] = {
	
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
  { 0x9581,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 XT",          kHypoprion		},
  { 0x9583,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 XT",          kHypoprion		},	
	{ 0x9588,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 XT",          kHypoprion		},
	{ 0x9589,	0x00000000, CHIP_FAMILY_RV630,		"ATI Radeon HD 2600 PRO",         kHypoprion		},
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
	
	{ 0x6898,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5870 Series",		kUakari	},
	{ 0x6899,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5850 Series",		kUakari	},
	{ 0x689C,	0x00000000, CHIP_FAMILY_HEMLOCK,	"ATI Radeon HD 5900 Series",		kUakari		},
	{ 0x689E,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5800 Series",		kUakari	},
  
  { 0x68A0,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770 Series",		kHoolock	},
  { 0x68A1,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770 Series",		kHoolock	},
  { 0x68A8,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770 Series",		kHoolock	},
  
  { 0x68B0,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770 Series",		kVervet		},
  { 0x68B1,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770 Series",		kVervet		},
	{ 0x68B8,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770 Series",		kVervet		},
	{ 0x68B9,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5700 Series",		kVervet		},
	{ 0x68BE,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5700 Series",		kVervet		},
	{ 0x68BF,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5700 Series",		kVervet		},
	
  { 0x68C0,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5730 Series",		kBaboon		},
  { 0x68C1,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5650 Series",    kBaboon		},	
  { 0x68C8,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5650 Series",    kBaboon		},	
  { 0x68D8,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5670 Series",		kBaboon		},
	{ 0x68D9,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5570 Series",		kBaboon		},
	{ 0x68DA,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5500 Series",		kBaboon		},
	{ 0x68E0,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5470 Series",		kBaboon		},	
  
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
	{ 0x6722,	0x00000000, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6900M Series",		kDuckweed	},	
	{ 0x6738,	0x00000000, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6870 Series",		kDuckweed },
	{ 0x6739,	0x00000000, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6850 Series",		kDuckweed },
	{ 0x673E,	0x00000000, CHIP_FAMILY_BARTS,		"AMD Radeon HD 6790 Series",		kDuckweed	},
	
	{ 0x6740,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6770M Series",		kPithecia		},
	{ 0x6741,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6650M Series",	  kPithecia		}, // kOsmunda },
	{ 0x6745,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6600M Series",	  kPithecia		}, // kOsmunda },
	{ 0x6750,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670 Series",		kPithecia   },
	{ 0x6758,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6670 Series",		kPithecia   },
	{ 0x6759,	0x00000000, CHIP_FAMILY_TURKS,		"AMD Radeon HD 6570 Series",		kPithecia   },
  
	{ 0x6760,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6400M Series",		kBulrushes	},
	{ 0x6761,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6430M Series",		kBulrushes	},
	{ 0x6768,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6400M Series",		kBulrushes	},
	{ 0x6770,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6400 Series",		kBulrushes	},
	{ 0x6779,	0x00000000, CHIP_FAMILY_CAICOS,		"AMD Radeon HD 6450 Series",		kBulrushes	},
	
	{ 0x0000,	0x00000000, CHIP_FAMILY_UNKNOW,		NULL,								kNull		}
};

const CHAR8 *chip_family_name[] = {
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
#define TEST_CHAM 1
AtiDevProp ati_devprop_list[] = {
	{FLAGTRUE,	FALSE,	"@0,AAPL,boot-display",		get_bootdisplay_val,	NULVAL				},
  //	{FLAGTRUE,	FALSE,	"@0,ATY,EFIDisplay",		NULL,					STRVAL("TMDSA")			},
#if TEST_CHAM	
  //{FLAGTRUE,	TRUE,	"@0,AAPL,vram-memory",		get_vrammemory_val,		NULVAL				},
  {FLAGTRUE,	TRUE,	"@0,override-no-connector",		get_edid_val,       NULVAL        },
  {FLAGTRUE,	TRUE,	"@0,compatible",              get_name_val,       NULVAL				},
  {FLAGTRUE,	TRUE,	"@0,connector-type",          get_conntype_val,		NULVAL        },
  {FLAGTRUE,	TRUE,	"@0,device_type",             NULL,					STRVAL("display")   },
//	{FLAGTRUE,	FALSE,	"@0,display-connect-flags", NULL,				DWRVAL((UINT32)0)   },
  {FLAGMOBILE,	FALSE,	"@0,display-link-component-bits", NULL,		DWRVAL((UINT32)6)	},
  {FLAGMOBILE,	TRUE,	"@0,display-type",          get_display_type,		NULVAL  			},
#endif  
	{FLAGTRUE,	TRUE,	"@0,name",                    get_name_val,			NULVAL          },
//  {FLAGTRUE,	TRUE,	"@0,VRAM,memsize",			get_vrammemsize_val,	NULVAL          },
	
  {FLAGTRUE,	FALSE,	"AAPL,aux-power-connected", NULL,					DWRVAL((UINT32)1)		},
  {FLAGTRUE,	FALSE,	"AAPL00,DualLink",          NULL,					DWRVAL((UINT32)1)		},
  {FLAGMOBILE,	FALSE,	"AAPL,HasPanel",          NULL,					DWRVAL((UINT32)1)   },
  {FLAGMOBILE,	FALSE,	"AAPL,HasLid",            NULL,					DWRVAL((UINT32)1)   },
  {FLAGMOBILE,	FALSE,	"AAPL,backlight-control", NULL,					DWRVAL((UINT32)0)   },
	{FLAGTRUE,	FALSE,	"AAPL,overwrite_binimage",	get_binimage_owr,		NULVAL				},
	{FLAGTRUE,	FALSE,	"ATY,bin_image",            get_binimage_val,		NULVAL				},
	{FLAGTRUE,	FALSE,	"ATY,Copyright",	NULL,	STRVAL("Copyright AMD Inc. All Rights Reserved. 2005-2011") },
  {FLAGTRUE,	FALSE,	"ATY,EFIVersion",	NULL,	STRVAL("01.00.3180")                  },
	{FLAGTRUE,	FALSE,	"ATY,Card#",			get_romrevision_val,	NULVAL                },
	{FLAGTRUE,	FALSE,	"ATY,VendorID",		NULL,					WRDVAL((UINT16)0x1002)        },
	{FLAGTRUE,	FALSE,	"ATY,DeviceID",		get_deviceid_val,		NULVAL                  },
	
  //	{FLAGTRUE,	FALSE,	"ATY,MCLK",					get_mclk_val,			NULVAL							},
  //	{FLAGTRUE,	FALSE,	"ATY,SCLK",					get_sclk_val,			NULVAL							},
  //	{FLAGTRUE,	FALSE,	"ATY,RefCLK",				get_refclk_val,			DWRVAL((UINT32)0x0a8c)		},
	
  {FLAGTRUE,	FALSE,	"ATY,PlatformInfo",			get_platforminfo_val,	NULVAL					},
	
	{FLAGTRUE,	FALSE,	"name",						get_nameparent_val,     NULVAL							},
	{FLAGTRUE,	FALSE,	"device_type",		get_nameparent_val,     NULVAL							},
	{FLAGTRUE,	FALSE,	"model",					get_model_val,          STRVAL("ATI Radeon")},
#if TEST_CHAM  
  {FLAGTRUE,	FALSE,	"VRAM,totalsize",	get_vramtotalsize_val,	NULVAL              },
#endif	
	{FLAGTRUE,	FALSE,	NULL,	NULL,	NULVAL}
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

BOOLEAN get_edid_val(value_t *val)
{
  static UINT32 v = 0;
	
	if (v)
		return FALSE;
	  
  if (!gEDID) {
    return FALSE;
  }
  v = 1;
  val->type = kPtr;
  val->size = 128;
  val->data = (UINT8 *)gEDID;
	return TRUE;
}

static CONST CHAR8* dtyp[] = {"LCD", "CRT", "DVI", "NONE"};
static UINT32 dti = 0;

BOOLEAN get_display_type(value_t *val)
{

  dti++;
  if (dti > 3) {
    dti = 0;
  }
	val->type = kStr;
	val->size = 4;
	val->data = (UINT8 *)dtyp[dti];
	
	return TRUE;
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
	val->size = AsciiStrLen(card->info->model_name);
	val->data = (UINT8 *)card->info->model_name;
	
	return TRUE;
}

static CONST UINT32 ct[] = {0x02, 0x200, 0x400, /*0x800,*/ 0x4};

BOOLEAN get_conntype_val(value_t *val)
{
//Connector types:
//0x200: VGA
//0x400: DL DVI-I
//0x800: HDMI
//0x4 : DisplayPort
  static UINT32 cti = 0;
  
  val->type = kCst;
	val->size = 4;
	val->data = (UINT8 *)&ct[cti];
  
  cti++;
  if(cti > 3) cti = 0;
  
	return TRUE;
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

BOOLEAN get_binimage_owr(value_t *val)
{
	static UINT32 v = 0;
  
	if (!gSettings.LoadVBios)
		return FALSE;
		
	v = 1;
	val->type = kCst;
	val->size = 4;
	val->data = (UINT8 *)&v;
	
	
	return TRUE;
}



BOOLEAN get_romrevision_val(value_t *val)
{
  CHAR8* cRev="109-B77101-00";
	UINT8 *rev;
	if (!card->rom){
    val->type = kPtr;
    val->size = 13;
    val->data = AllocateZeroPool(val->size);
    if (!val->data)
      return FALSE;
    
    CopyMem(val->data, cRev, val->size);
    
		return TRUE;
  }
	
	rev = card->rom + *(UINT8 *)(card->rom + OFFSET_TO_GET_ATOMBIOS_STRINGS_START);

	val->type = kPtr;
	val->size = AsciiStrLen((CHAR8 *)rev);
  if ((val->size < 3) || (val->size > 30)) { //fool proof. Real value 13
    rev = (UINT8 *)cRev;
    val->size = 13;
  }
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

VOID free_val(value_t *val)
{
	if (val->type == kPtr)
		FreePool(val->data);
	
	ZeroMem(val, sizeof(value_t));
}

//  {FLAGTRUE,	TRUE,	"@0,compatible",    get_name_val,       NULVAL				},
// 	{FLAGTRUE,	FALSE,	"ATY,VendorID",		NULL,					WRDVAL((UINT16)0x1002)        },
/*typedef struct {
	UINT32				flags;
	BOOLEAN				all_ports;
	CHAR8					*name;
	BOOLEAN				(*get_value)(value_t *val);
	value_t				default_val;
} AtiDevProp;
*/
VOID devprop_add_list(AtiDevProp devprop_list[])
{
	INTN i, pnum;
	value_t *val = AllocateZeroPool(sizeof(value_t));
	
	for (i = 0; devprop_list[i].name != NULL; i++)
	{
		if ((devprop_list[i].flags == FLAGTRUE) || (devprop_list[i].flags & card->flags))
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
	
	if (rom_header->signature != 0xaa55){
    DBG("invalid ROM signature %x\n", rom_header->signature);
		return FALSE;
  }
	
	rom_pci_header = (option_rom_pci_header_t *)((UINT8 *)rom_header + rom_header->pci_header_offset);
	
	if (rom_pci_header->signature != 0x52494350){
    DBG("invalid ROM header %x\n", rom_pci_header->signature);
		return FALSE;
  }
	
	if (rom_pci_header->vendor_id != pci_dev->vendor_id || rom_pci_header->device_id != pci_dev->device_id){
    DBG("invalid ROM vendor=%x deviceID=%d\n", rom_pci_header->vendor_id, rom_pci_header->device_id);
		return FALSE;
  }
	
	return TRUE;
}

BOOLEAN load_vbios_file(UINT16 vendor_id, UINT16 device_id)
{
  EFI_STATUS            Status = EFI_NOT_FOUND;
	UINTN bufferLen;
	CHAR16 FileName[24];
  UINT8*  buffer;
	
//	getBoolForKey(key, &do_load, &bootInfo->chameleonConfig);
	if (!gSettings.LoadVBios)
		return FALSE;
	
	UnicodeSPrint(FileName, 24, L"\\ROM\\%04x_%04x.rom", vendor_id, device_id);
  if (FileExists(OEMDir, FileName)){
    Status = egLoadFile(OEMDir, FileName, &buffer, &bufferLen);
  }
  if (EFI_ERROR(Status)) {
    UnicodeSPrint(FileName, 24, L"\\EFI\\ROM\\%04x_%04x.rom", vendor_id, device_id);
    if (FileExists(SelfRootDir, FileName)){
      Status = egLoadFile(SelfRootDir, FileName, &buffer, &bufferLen);
    }
  }

	if (EFI_ERROR(Status)){
 	    DBG("ATI ROM not found \n");
	    card->rom_size = 0;
		card->rom = 0;
		return FALSE;
  }
  
	card->rom_size = bufferLen;
	card->rom = AllocateZeroPool(bufferLen);
	if (!card->rom)
		return FALSE;
	CopyMem(card->rom, buffer, bufferLen);
//	read(fd, (CHAR8 *)card->rom, card->rom_size);
	
	if (!validate_rom((option_rom_header_t *)card->rom, card->pci_dev))
	{
    DBG("validate_rom fails\n");
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
	
	card->vram_size = 128 << 20; //default 128Mb, this is minimum for OS
  if (gSettings.VRAM != 0) {
    card->vram_size = gSettings.VRAM;
  } else {
    if (chip_family >= CHIP_FAMILY_CEDAR) {
      // size in MB on evergreen
      // XXX watch for overflow!!!
      card->vram_size = REG32(card->mmio, R600_CONFIG_MEMSIZE) << 20;
    } else if (chip_family >= CHIP_FAMILY_R600) {
			card->vram_size = REG32(card->mmio, R600_CONFIG_MEMSIZE);
    } else {
      card->vram_size = REG32(card->mmio, RADEON_CONFIG_MEMSIZE);
      if (card->vram_size == 0) {
        card->vram_size = REG32(card->mmio, RADEON_CONFIG_APER_SIZE);
        //Slice - previously I successfully made Radeon9000 working
        //by writing this register
    //    WRITEREG32(card->mmio, RADEON_CONFIG_MEMSIZE, 0x30000);
      }
    }
  }	
}

BOOLEAN read_vbios(BOOLEAN from_pci)
{
	option_rom_header_t *rom_addr;
	
	if (from_pci)
	{
		rom_addr = (option_rom_header_t *)(UINTN)(pci_config_read32(card->pci_dev, PCI_EXPANSION_ROM_BASE) & ~0x7ff);
		DBG(" @0x%x", rom_addr);
	}
	else
		rom_addr = (option_rom_header_t *)0xc0000;
	
	if (!validate_rom(rom_addr, card->pci_dev)){
    DBG("There is no ROM @C0000\n");
 //   gBS->Stall(3000000);
		return FALSE;
  }
	
	card->rom_size = rom_addr->rom_size * 512;
	if (!card->rom_size){
    DBG("invalid ROM size =0\n");
		return FALSE;
  }
	
	card->rom = AllocateZeroPool(card->rom_size);
	if (!card->rom)
		return FALSE;
	
	CopyMem(card->rom, (void *)rom_addr, card->rom_size);
	
	return TRUE;
}

BOOLEAN read_disabled_vbios(VOID)
{
	BOOLEAN ret = FALSE;
	chip_family_t chip_family = card->info->chip_family;
	
	if (chip_family >= CHIP_FAMILY_RV770)
	{
		UINT32 viph_control		= REG32(card->mmio, RADEON_VIPH_CONTROL);
		UINT32 bus_cntl			= REG32(card->mmio, RADEON_BUS_CNTL);
		UINT32 d1vga_control		= REG32(card->mmio, AVIVO_D1VGA_CONTROL);
		UINT32 d2vga_control		= REG32(card->mmio, AVIVO_D2VGA_CONTROL);
		UINT32 vga_render_control = REG32(card->mmio, AVIVO_VGA_RENDER_CONTROL);
		UINT32 rom_cntl			= REG32(card->mmio, R600_ROM_CNTL);
		UINT32 cg_spll_func_cntl	= 0;
		UINT32 cg_spll_status;
		
		// disable VIP
		WRITEREG32(card->mmio, RADEON_VIPH_CONTROL, (viph_control & ~RADEON_VIPH_EN));
		
		// enable the rom
		WRITEREG32(card->mmio, RADEON_BUS_CNTL, (bus_cntl & ~RADEON_BUS_BIOS_DIS_ROM));
		
		// Disable VGA mode
		WRITEREG32(card->mmio, AVIVO_D1VGA_CONTROL, (d1vga_control & ~(AVIVO_DVGA_CONTROL_MODE_ENABLE | AVIVO_DVGA_CONTROL_TIMING_SELECT)));
		WRITEREG32(card->mmio, AVIVO_D2VGA_CONTROL, (d2vga_control & ~(AVIVO_DVGA_CONTROL_MODE_ENABLE | AVIVO_DVGA_CONTROL_TIMING_SELECT)));
		WRITEREG32(card->mmio, AVIVO_VGA_RENDER_CONTROL, (vga_render_control & ~AVIVO_VGA_VSTATUS_CNTL_MASK));
		
		if (chip_family == CHIP_FAMILY_RV730)
		{
			cg_spll_func_cntl = REG32(card->mmio, R600_CG_SPLL_FUNC_CNTL);
			
			// enable bypass mode
			WRITEREG32(card->mmio, R600_CG_SPLL_FUNC_CNTL, (cg_spll_func_cntl | R600_SPLL_BYPASS_EN));
			
			// wait for SPLL_CHG_STATUS to change to 1
			cg_spll_status = 0;
			while (!(cg_spll_status & R600_SPLL_CHG_STATUS))
				cg_spll_status = REG32(card->mmio, R600_CG_SPLL_STATUS);
			
			WRITEREG32(card->mmio, R600_ROM_CNTL, (rom_cntl & ~R600_SCK_OVERWRITE));
		}
		else
			WRITEREG32(card->mmio, R600_ROM_CNTL, (rom_cntl | R600_SCK_OVERWRITE));
		
		ret = read_vbios(TRUE);
		
		// restore regs
		if (chip_family == CHIP_FAMILY_RV730)
		{
			WRITEREG32(card->mmio, R600_CG_SPLL_FUNC_CNTL, cg_spll_func_cntl);
			
			// wait for SPLL_CHG_STATUS to change to 1
			cg_spll_status = 0;
			while (!(cg_spll_status & R600_SPLL_CHG_STATUS))
			cg_spll_status = REG32(card->mmio, R600_CG_SPLL_STATUS);
		}
		WRITEREG32(card->mmio, RADEON_VIPH_CONTROL, viph_control);
		WRITEREG32(card->mmio, RADEON_BUS_CNTL, bus_cntl);
		WRITEREG32(card->mmio, AVIVO_D1VGA_CONTROL, d1vga_control);
		WRITEREG32(card->mmio, AVIVO_D2VGA_CONTROL, d2vga_control);
		WRITEREG32(card->mmio, AVIVO_VGA_RENDER_CONTROL, vga_render_control);
		WRITEREG32(card->mmio, R600_ROM_CNTL, rom_cntl);
	}
	else
		if (chip_family >= CHIP_FAMILY_R600)
		{
			UINT32 viph_control				= REG32(card->mmio, RADEON_VIPH_CONTROL);
			UINT32 bus_cntl					= REG32(card->mmio, RADEON_BUS_CNTL);
			UINT32 d1vga_control				= REG32(card->mmio, AVIVO_D1VGA_CONTROL);
			UINT32 d2vga_control				= REG32(card->mmio, AVIVO_D2VGA_CONTROL);
			UINT32 vga_render_control			= REG32(card->mmio, AVIVO_VGA_RENDER_CONTROL);
			UINT32 rom_cntl					= REG32(card->mmio, R600_ROM_CNTL);
			UINT32 general_pwrmgt				= REG32(card->mmio, R600_GENERAL_PWRMGT);
			UINT32 low_vid_lower_gpio_cntl	= REG32(card->mmio, R600_LOW_VID_LOWER_GPIO_CNTL);
			UINT32 medium_vid_lower_gpio_cntl = REG32(card->mmio, R600_MEDIUM_VID_LOWER_GPIO_CNTL);
			UINT32 high_vid_lower_gpio_cntl	= REG32(card->mmio, R600_HIGH_VID_LOWER_GPIO_CNTL);
			UINT32 ctxsw_vid_lower_gpio_cntl	= REG32(card->mmio, R600_CTXSW_VID_LOWER_GPIO_CNTL);
			UINT32 lower_gpio_enable			= REG32(card->mmio, R600_LOWER_GPIO_ENABLE);
			
			// disable VIP
			WRITEREG32(card->mmio, RADEON_VIPH_CONTROL, (viph_control & ~RADEON_VIPH_EN));
			
			// enable the rom
			WRITEREG32(card->mmio, RADEON_BUS_CNTL, (bus_cntl & ~RADEON_BUS_BIOS_DIS_ROM));
			
			// Disable VGA mode
			WRITEREG32(card->mmio, AVIVO_D1VGA_CONTROL, (d1vga_control & ~(AVIVO_DVGA_CONTROL_MODE_ENABLE | AVIVO_DVGA_CONTROL_TIMING_SELECT)));
			WRITEREG32(card->mmio, AVIVO_D2VGA_CONTROL, (d2vga_control & ~(AVIVO_DVGA_CONTROL_MODE_ENABLE | AVIVO_DVGA_CONTROL_TIMING_SELECT)));
			WRITEREG32(card->mmio, AVIVO_VGA_RENDER_CONTROL, (vga_render_control & ~AVIVO_VGA_VSTATUS_CNTL_MASK));
			WRITEREG32(card->mmio, R600_ROM_CNTL, ((rom_cntl & ~R600_SCK_PRESCALE_CRYSTAL_CLK_MASK) | (1 << R600_SCK_PRESCALE_CRYSTAL_CLK_SHIFT) | R600_SCK_OVERWRITE));
			WRITEREG32(card->mmio, R600_GENERAL_PWRMGT, (general_pwrmgt & ~R600_OPEN_DRAIN_PADS));
			WRITEREG32(card->mmio, R600_LOW_VID_LOWER_GPIO_CNTL, (low_vid_lower_gpio_cntl & ~0x400));
			WRITEREG32(card->mmio, R600_MEDIUM_VID_LOWER_GPIO_CNTL, (medium_vid_lower_gpio_cntl & ~0x400));
			WRITEREG32(card->mmio, R600_HIGH_VID_LOWER_GPIO_CNTL, (high_vid_lower_gpio_cntl & ~0x400));
			WRITEREG32(card->mmio, R600_CTXSW_VID_LOWER_GPIO_CNTL, (ctxsw_vid_lower_gpio_cntl & ~0x400));
			WRITEREG32(card->mmio, R600_LOWER_GPIO_ENABLE, (lower_gpio_enable | 0x400));
			
			ret = read_vbios(TRUE);
			
			// restore regs
			WRITEREG32(card->mmio, RADEON_VIPH_CONTROL, viph_control);
			WRITEREG32(card->mmio, RADEON_BUS_CNTL, bus_cntl);
			WRITEREG32(card->mmio, AVIVO_D1VGA_CONTROL, d1vga_control);
			WRITEREG32(card->mmio, AVIVO_D2VGA_CONTROL, d2vga_control);
			WRITEREG32(card->mmio, AVIVO_VGA_RENDER_CONTROL, vga_render_control);
			WRITEREG32(card->mmio, R600_ROM_CNTL, rom_cntl);
			WRITEREG32(card->mmio, R600_GENERAL_PWRMGT, general_pwrmgt);
			WRITEREG32(card->mmio, R600_LOW_VID_LOWER_GPIO_CNTL, low_vid_lower_gpio_cntl);
			WRITEREG32(card->mmio, R600_MEDIUM_VID_LOWER_GPIO_CNTL, medium_vid_lower_gpio_cntl);
			WRITEREG32(card->mmio, R600_HIGH_VID_LOWER_GPIO_CNTL, high_vid_lower_gpio_cntl);
			WRITEREG32(card->mmio, R600_CTXSW_VID_LOWER_GPIO_CNTL, ctxsw_vid_lower_gpio_cntl);
			WRITEREG32(card->mmio, R600_LOWER_GPIO_ENABLE, lower_gpio_enable);
		}

	return ret;
}

BOOLEAN radeon_card_posted(VOID)
{
	UINT32 reg;
	
	// first check CRTCs
	reg = REG32(card->mmio, RADEON_CRTC_GEN_CNTL) | REG32(card->mmio, RADEON_CRTC2_GEN_CNTL);
	if (reg & RADEON_CRTC_EN)
		return TRUE;
	
	// then check MEM_SIZE, in case something turned the crtcs off
	reg = REG32(card->mmio, R600_CONFIG_MEMSIZE);
	if (reg)
		return TRUE;
	
	return FALSE;
}

#if 0
BOOLEAN devprop_add_pci_config_space(VOID)
{
	int offset;
	
	UINT8 *config_space = AllocateZeroPool(0x100);
	if (!config_space)
		return FALSE;
	
	for (offset = 0; offset < 0x100; offset += 4)
		config_space[offset / 4] = pci_config_read32(card->pci_dev, offset);
	
	devprop_add_value(card->device, "ATY,PCIConfigSpace", config_space, 0x100);
	FreePool(config_space);
	
	return TRUE;
}
#endif

static BOOLEAN init_card(pci_dt_t *pci_dev)
{
	BOOLEAN	add_vbios = gSettings.LoadVBios;
	CHAR8		*name;
	CHAR8		*name_parent;
    CHAR8 		*CfgName;
    CHAR8 		*model;
    INTN  		NameLen = 0;
	INTN		i, j;
	INTN		n_ports = 0;
    UINTN 		ExpansionRom = 0;
	
	card = AllocateZeroPool(sizeof(card_t));
	if (!card)
		return FALSE;
	
	card->pci_dev = pci_dev;
	
	for (i = 0; radeon_cards[i].device_id ; i++)
	{
		if (radeon_cards[i].device_id == pci_dev->device_id)
		{
			card->info = &radeon_cards[i];
			break;
		}
	}
  
  for (j = 0; j < NGFX; j++) {    
    if ((gGraphics[j].Vendor == Ati) &&
        (gGraphics[j].DeviceID == pci_dev->device_id)) {
      model = gGraphics[j].Model; 
      n_ports = gGraphics[j].Ports;
      add_vbios = gGraphics[j].LoadVBios;
      break;
    }
  }  
	
	if (!card->info->device_id || !card->info->cfg_name)
	{
		DBG("Unsupported ATI card! Device ID: [%04x:%04x] Subsystem ID: [%08x] \n", 
				pci_dev->vendor_id, pci_dev->device_id, pci_dev->subsys_id);
   		 DBG("search for brothers family\n");
    	for (i = 0; radeon_cards[i].device_id ; i++)
    	{
      		if ((radeon_cards[i].device_id & ~0xf) == (pci_dev->device_id & ~0xf))
      		{
        		card->info = &radeon_cards[i];
        		break;
      		}
    	}
    	if (!card->info->cfg_name) {
      		DBG("...compatible config is not found\n");
      		return FALSE;
    	}
	}
	
	card->fb		= (UINT8 *)(UINTN)(pci_config_read32(pci_dev, PCI_BASE_ADDRESS_0) & ~0x0f);
	card->mmio		= (UINT8 *)(UINTN)(pci_config_read32(pci_dev, PCI_BASE_ADDRESS_2) & ~0x0f);
	card->io		= (UINT8 *)(UINTN)(pci_config_read32(pci_dev, PCI_BASE_ADDRESS_4) & ~0x03);
  pci_dev->regs = card->mmio;
	ExpansionRom = pci_config_read32(pci_dev, PCI_EXPANSION_ROM_BASE);
	DBG("Framebuffer @0x%08X  MMIO @0x%08X	I/O Port @0x%08X ROM Addr @0x%08X\n",
		card->fb, card->mmio, card->io, ExpansionRom);
	
	card->posted = radeon_card_posted();
	DBG("ATI card %a, ", card->posted ? "POSTed" : "non-POSTed");
	DBG("\n");
	get_vram_size();
	
	if (gSettings.LoadVBios){
    load_vbios_file(pci_dev->vendor_id, pci_dev->device_id);
		if (!card->rom)
		{
			DBG("reading VBIOS from %a", card->posted ? "legacy space" : "PCI ROM");
			if (card->posted) // && ExpansionRom != 0)
				read_vbios(FALSE);
			else
				read_disabled_vbios();
			DBG("\n");
		}
  }
	
	if (card->info->chip_family >= CHIP_FAMILY_CEDAR)
	{
    DBG("ATI Radeon EVERGREEN family\n");
		card->flags |= EVERGREEN;
	}
  
  if (gMobile) {
    DBG("ATI Mobile Radeon\n");
    card->flags |= FLAGMOBILE;
  }
  
	NameLen = StrLen(gSettings.FBName);
  if (NameLen > 3) {  //fool proof: cfg_name is 4 character or more.
    CfgName = AllocateZeroPool(NameLen);
    UnicodeStrToAsciiStr((CHAR16*)&gSettings.FBName[0], CfgName);
    DBG("Users config name %a\n", CfgName);
    card->cfg_name = CfgName;
  } else {
    // use cfg_name on radeon_cards, to retrive the default name from card_configs,
		card->cfg_name = card_configs[card->info->cfg_name].name;
		
		// which means one of the fb's or kNull
		DBG("Framebuffer set to device's default: %a\n", card->cfg_name);    
  }	

	if (n_ports > 0)
	{
		card->ports = n_ports; // use it.
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
  
  if (card->ports == 0) {
    card->ports = 2; //real minimum
    DBG("Nr of ports set to min: %d\n", card->ports);
  }
//		
	name = AllocateZeroPool(24);
	AsciiSPrint(name, 24, "ATY,%a", card->cfg_name);
	aty_name.type = kStr;
	aty_name.size = AsciiStrLen(name);
	aty_name.data = (UINT8 *)name;
	
  name_parent = AllocateZeroPool(24);
	AsciiSPrint(name_parent, 24, "ATY,%aParent", card->cfg_name);
	aty_nameparent.type = kStr;
	aty_nameparent.size = AsciiStrLen(name_parent);
	aty_nameparent.data = (UINT8 *)name_parent;
//how can we free pool when we leave the procedure? Make all pointers global?	
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
	//card->device = devprop_add_device(string, devicepath);
	card->device = devprop_add_device_pci(string, ati_dev);
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

    devprop_add_value(card->device, "hda-gfx", (UINT8*)"onboard-1", 9);
	
	
	DBG("ATI %a %a %dMB (%a) [%04x:%04x] (subsys [%04x:%04x]):: %a\n",
			chip_family_name[card->info->chip_family], card->info->model_name,
			(UINT32)(card->vram_size / (1024 * 1024)), card->cfg_name,
			ati_dev->vendor_id, ati_dev->device_id,
			ati_dev->subsys_id.subsys.vendor_id, ati_dev->subsys_id.subsys.device_id,
			devicepath);
	
//  FreePool(card->info); //TODO we can't free constant so this info should be copy of constants
	FreePool(card);
	
	return TRUE;
}
