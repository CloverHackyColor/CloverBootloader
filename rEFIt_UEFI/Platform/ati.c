/*
 * ATI Graphics Card Enabler, part of the Chameleon Boot Loader Project
 *
 * Copyright 2010 by Islam M. Ahmed Zaid. All rights reserved.
 *
 */

#include "Platform.h"

#include "ati.h"

#ifndef DEBUG_ALL
#define DEBUG_ATI 1
#else
#define DEBUG_ATI DEBUG_ALL
#endif

#if DEBUG_ATI == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_ATI, __VA_ARGS__)
#endif

static value_t aty_name;
static value_t aty_nameparent;
card_t *card;
//static value_t aty_model;

card_config_t card_configs[] = {
	{NULL,		0},
	/* OLDController */
	{"Wormy",	2},
	{"Alopias",	2},
	{"Caretta",	1},
	{"Kakapo",	3},
	{"Kipunji",	4},
	{"Peregrine",	2},
	{"Raven",	3},
	{"Sphyrna",	1},
	/* AMD2400Controller */
	{"Iago",	2},
	/* AMD2600Controller */
	{"Hypoprion",	2},
	{"Lamna",	2},
	/* AMD3800Controller */
	{"Megalodon",	3},
	{"Triakis",	2},
	/* AMD4600Controller */
	{"Flicker",	3},
	{"Gliff",	3},
	{"Shrike",	3},
	/* AMD4800Controller */
	{"Cardinal",	2},
	{"Motmot",	2},
	{"Quail",	3},
	/* AMD5000Controller */
	{"Douc",	2},
	{"Langur",	3},
	{"Uakari",	4},
	{"Zonalis",	6},
	{"Alouatta",	4},
	{"Hoolock",	1},
	{"Vervet",	4},
	{"Baboon",	3},
	{"Eulemur",	3},
	{"Galago",	2},
	{"Colobus",	2},
	{"Mangabey",	2},
	{"Nomascus",	5},
	{"Orangutan",	2},
	/* AMD6000Controller */
	{"Pithecia",	3},
	{"Bulrushes",	6},
	{"Cattail",	4},
	{"Hydrilla",	5},
	{"Duckweed",	4},
	{"Fanwort",	4},
	{"Elodea",	5},
	{"Kudzu",	2},
	{"Gibba",	5},
	{"Lotus",	3},
	{"Ipomoea",	3},
	{"Muskgrass",	4},
	{"Juncus",	4},
	{"Osmunda",     4},
	{"Pondweed",	3},
	{"Spikerush",   4},
	{"Typha",       5},
	/* AMD7000Controller */
    {"Namako",	4},
	{"Aji",		4},
	{"Buri",	4},
	{"Chutoro",	5},
	{"Dashimaki",	4},
	{"Ebi",		5},
	{"Gari",	5},
	{"Futomaki",	5},
	{"Hamachi",	4},
	{"OPM",         6},
    {"Ikura",	1},
    {"IkuraS",	6},
    {"Junsai",	6},
    {"Kani",	1},
    {"KaniS",	6},
    {"DashimakiS",	4},
    {"Maguro",	1},
    {"MaguroS",	6},
    /* AMD8000Controller */
    {"Baladi",      6},
    /* AMD9000Controller */
    {"Exmoor",	4},
  {"Basset", 4},
    {"Greyhound",	6},
  {"Labrador",      6},

};

radeon_card_info_t radeon_cards[] = {
	
	// Earlier cards are not supported
	//
	// Layout is device_id, fake_id, chip_family_name, display name, frame buffer
	// Cards are grouped by device id  to make it easier to add new cards
	//

  /*old series*/
	// R423
/*	{ 0x5D48, 0x00000000, CHIP_FAMILY_R423,  "ATI Radeon HD Mobile ",	kNull		},
	{ 0x5D49, 0x00000000, CHIP_FAMILY_R423,  "ATI Radeon HD Mobile ",	kNull		},
	{ 0x5D4A, 0x00000000, CHIP_FAMILY_R423,  "ATI Radeon HD Mobile ",	kNull		},
	{ 0x5D4C, 0x00000000, CHIP_FAMILY_R423,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x5D4D, 0x00000000, CHIP_FAMILY_R423,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x5D4E, 0x00000000, CHIP_FAMILY_R423,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x5D4F, 0x00000000, CHIP_FAMILY_R423,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x5D50, 0x00000000, CHIP_FAMILY_R423,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x5D52, 0x00000000, CHIP_FAMILY_R423,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x5D57, 0x00000000, CHIP_FAMILY_R423,  "ATI Radeon HD Desktop ",	kNull		},

	// RV410
	{ 0x5E48, 0x00000000, CHIP_FAMILY_RV410, "ATI Radeon HD Desktop ",	kNull		},
	{ 0x5E4A, 0x00000000, CHIP_FAMILY_RV410, "ATI Radeon HD Desktop ",	kNull		},
	{ 0x5E4B, 0x00000000, CHIP_FAMILY_RV410, "ATI Radeon HD Desktop ",	kNull		},
	{ 0x5E4C, 0x00000000, CHIP_FAMILY_RV410, "ATI Radeon HD Desktop ",	kNull		},
	{ 0x5E4D, 0x00000000, CHIP_FAMILY_RV410, "ATI Radeon HD Desktop ",	kNull		},
	{ 0x5E4F, 0x00000000, CHIP_FAMILY_RV410, "ATI Radeon HD Desktop ",	kNull		},
     */
    
    // OLAND
    
    // Oland: R7-240, 250
    { 0x6600,	0x00000000, CHIP_FAMILY_OLAND,		"AMD Radeon HD 8600/8700M",	kNull       }, // Mobile
    { 0x6601,	0x00000000, CHIP_FAMILY_OLAND,		"AMD Radeon HD 8500/8700M",	kNull       }, // Mobile
    //	{ 0x6602,	0x00000000, CHIP_FAMILY_OLAND,		"AMD Radeon",           kNull       }, // Mobile
    //	{ 0x6603,	0x00000000, CHIP_FAMILY_OLAND,		"AMD Radeon",           kNull       }, // Mobile
    { 0x6604,	0x00000000, CHIP_FAMILY_OLAND,		"AMD Radeon R7 M265",       kNull       }, // Mobile
    { 0x6605,	0x00000000, CHIP_FAMILY_OLAND,		"AMD Radeon R7 M260",       kNull       }, // Mobile
    { 0x6606,	0x00000000, CHIP_FAMILY_OLAND,		"AMD Radeon HD 8790M",      kNull       }, // Mobile
    { 0x6607,	0x00000000, CHIP_FAMILY_OLAND,		"AMD Radeon R5 M240",       kNull       }, // Mobile
    { 0x6608,	0x00000000, CHIP_FAMILY_OLAND,		"AMD FirePro W2100",        kNull       },
    { 0x6610,	0x00000000, CHIP_FAMILY_OLAND,		"AMD Radeon R7 250",        kFutomaki		},
    { 0x6611,	0x00000000, CHIP_FAMILY_OLAND,		"AMD Radeon R7 340 Series",	kNull       },
    { 0x6613,	0x00000000, CHIP_FAMILY_OLAND,		"AMD Radeon R7 240",        kFutomaki	},
    //	{ 0x6620,	0x00000000, CHIP_FAMILY_OLAND,		"AMD Radeon",           kNull       }, // Mobile
    //	{ 0x6621,	0x00000000, CHIP_FAMILY_OLAND,		"AMD Radeon",           kNull       }, // Mobile
    //	{ 0x6623,	0x00000000, CHIP_FAMILY_OLAND,		"AMD Radeon",           kNull       }, // Mobile
    //	{ 0x6631,	0x00000000, CHIP_FAMILY_OLAND,		"AMD Radeon",           kNull       },
    
    // BONAIRE
    { 0x6640,	0x00000000, CHIP_FAMILY_BONAIRE,	"AMD Radeon HD 8950",       kNull       }, // Mobile
    { 0x6641,	0x00000000, CHIP_FAMILY_BONAIRE,	"AMD Radeon HD 8930M",        kNull       }, // Mobile
    { 0x6646,	0x00000000, CHIP_FAMILY_BONAIRE,	"AMD Radeon R9 M280X",      kNull       }, // Mobile
    { 0x6647,	0x00000000, CHIP_FAMILY_BONAIRE,	"AMD Radeon R9 M270X",      kNull       }, // Mobile
    { 0x6649,	0x00000000, CHIP_FAMILY_BONAIRE,	"AMD FirePro W5100",        kNull       },
    //	{ 0x6650,	0x00000000, CHIP_FAMILY_BONAIRE,	"AMD Radeon",           kNull       },
    //	{ 0x6651,	0x00000000, CHIP_FAMILY_BONAIRE,	"AMD Radeon",           kNull       },
    { 0x6658,	0x00000000, CHIP_FAMILY_BONAIRE,	"AMD Radeon R7 260X",        kNull       },
    { 0x665C,	0x00000000, CHIP_FAMILY_BONAIRE,	"AMD Radeon HD 7790",       kFutomaki	},
    { 0x665D,	0x00000000, CHIP_FAMILY_BONAIRE,	"AMD Radeon R9 260",        kFutomaki	},
    { 0x665F,	0x00000000, CHIP_FAMILY_BONAIRE,	"AMD Radeon R9 360",        kFutomaki	},
    // HAINAN
    { 0x6660,	0x00000000, CHIP_FAMILY_HAINAN,		"AMD Radeon HD 8670M",      kNull       }, // Mobile
    { 0x6663,	0x00000000, CHIP_FAMILY_HAINAN,		"AMD Radeon HD 8570M",      kNull       }, // Mobile
    { 0x6664,	0x00000000, CHIP_FAMILY_HAINAN,		"AMD Radeon R5 M240",       kNull       }, // Mobile
    { 0x6665,	0x00000000, CHIP_FAMILY_HAINAN,		"AMD Radeon R5 M230",       kNull       }, // Mobile
    { 0x6667,	0x00000000, CHIP_FAMILY_HAINAN,		"AMD Radeon R5 M230",       kNull       }, // Mobile
    { 0x666F,	0x00000000, CHIP_FAMILY_HAINAN,		"AMD Radeon HD 8550M",      kNull       }, // Mobile
    
    /* Northen Islands */
    //0x67681002 0x67701002 0x67791002 0x67601002 0x67611002 0x67501002 0x67581002 0x67591002
    //0x67401002 0x67411002 0x67451002 0x67381002 0x67391002 0x67201002 0x67221002 0x67181002
    //Gibba, Lotus, Muskgrass
    //id from AMD6000 10.9
    //0x67681002 0x67701002 0x67791002 0x67601002 0x67611002 0x67501002 0x67581002 0x67591002
    //0x67401002 0x67411002 0x67451002 0x67381002 0x67391002 0x67201002 0x67221002 0x67181002
    //0x67191002 0x68401002 0x68411002 0x67041002
    // CAYMAN
    { 0x6701,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD Radeon HD 6xxx Series",	kLotus		},
    { 0x6702,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD Radeon HD 6xxx Series",	kLotus		},
    { 0x6703,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD Radeon HD 6xxx Series",	kLotus		},
    { 0x6704,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD FirePro V7900",          kLotus		},
    { 0x6705,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD Radeon HD 6xxx Series",	kLotus		},
    { 0x6706,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD Radeon HD 6xxx Series",	kLotus		},
    { 0x6707,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD Radeon HD 6xxx Series",	kLotus		},
    { 0x6708,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD FirePro V5900",          kLotus		},
    { 0x6709,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD Radeon HD 6xxx Series",	kLotus		},
    { 0x6718,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD Radeon HD 6970 Series",	kLotus		},
    { 0x6719,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD Radeon HD 6950 Series",	kLotus		},
    { 0x671C,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD Radeon HD 6990 Series",	kLotus		},
    { 0x671D,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD Radeon HD 6950 Series",	kLotus		},
    { 0x671F,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD Radeon HD 6930 Series",	kLotus		},
    
    // BARTS
    { 0x6720,	0x00000000, CHIP_FAMILY_BARTS,	"AMD Radeon HD 6970M Series",	kFanwort	},
    { 0x6722,	0x00000000, CHIP_FAMILY_BARTS,	"AMD Radeon HD 6900M Series",	kFanwort	},
    { 0x6729,	0x00000000, CHIP_FAMILY_BARTS,	"AMD Radeon HD 6900M Series",	kFanwort	},
    { 0x6738,	0x00000000, CHIP_FAMILY_BARTS,	"AMD Radeon HD 6870 Series",	kDuckweed	},
    { 0x6739,	0x00000000, CHIP_FAMILY_BARTS,	"AMD Radeon HD 6850 X2",      kDuckweed	},
    { 0x673E,	0x00000000, CHIP_FAMILY_BARTS,	"AMD Radeon HD 6790 Series",	kDuckweed	},
    
    // TURKS
    { 0x6740,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 6770M Series",	kCattail	},
    { 0x6741,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 6750M",        kCattail	},
    { 0x6742,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 7500/7600",    kCattail	},
    { 0x6745,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 6600M Series",	kCattail	},
    { 0x6749,	0x00000000, CHIP_FAMILY_TURKS,	"ATI Radeon FirePro V4900",   kPithecia	},
    { 0x674A,	0x00000000, CHIP_FAMILY_TURKS,	"AMD FirePro V3900",          kPithecia	},
    { 0x6750,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 6670 Series",	kPithecia	},
    { 0x6758,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 6670 Series",	kPithecia	},
    { 0x6759,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 6570 Series",	kPithecia	},
    { 0x675B,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 7600 Series",	kPithecia	},
    { 0x675D,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 7570M Series",	kCattail	},
    { 0x675F,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 6510 Series",	kPithecia	},
    
    // CAICOS
    { 0x6760,	0x00000000, CHIP_FAMILY_CAICOS,	"AMD Radeon HD 6470M Series",	kHydrilla	},
    { 0x6761,	0x00000000, CHIP_FAMILY_CAICOS,	"AMD Radeon HD 6430M Series",	kHydrilla	},
    { 0x6763,	0x00000000, CHIP_FAMILY_CAICOS,	"AMD Radeon E6460 Series",    kHydrilla	},
    { 0x6768,	0x00000000, CHIP_FAMILY_CAICOS,	"AMD Radeon HD 6400M Series",	kHydrilla	},
    { 0x6770,	0x00000000, CHIP_FAMILY_CAICOS,	"AMD Radeon HD 6400 Series",	kBulrushes	},
    { 0x6771,	0x00000000, CHIP_FAMILY_CAICOS,	"AMD Radeon HD 8490",         kBulrushes	},
    { 0x6772,	0x00000000, CHIP_FAMILY_CAICOS,	"AMD Radeon HD 7400A Series",	kBulrushes	},
    { 0x6778,	0x00000000, CHIP_FAMILY_CAICOS,	"AMD Radeon HD 7470",         kBulrushes	},
    { 0x6779,	0x00000000, CHIP_FAMILY_CAICOS,	"AMD Radeon HD 6450/7450/8450/R5 230",	kBulrushes	},
    { 0x677B,	0x00000000, CHIP_FAMILY_CAICOS,	"AMD Radeon HD 7400 Series",	kBulrushes	},
    
    // TAHITI
    //Framebuffers: Aji - 4 Desktop, Buri - 4 Mobile, Chutoro - 5 Mobile,  Dashimaki - 4, IkuraS - HMDI
    // Ebi - 5 Mobile, Gari - 5 M, Futomaki - 4 D, Hamachi - 4 D, OPM - 6 Server, Ikura - 6
    { 0x6780,	0x00000000, CHIP_FAMILY_TAHITI,	"AMD FirePro W9000",	kIkuraS	},
    { 0x6784,	0x00000000, CHIP_FAMILY_TAHITI,	"AMD Radeon HD 7900 Series",	kFutomaki	},
    { 0x6788,	0x00000000, CHIP_FAMILY_TAHITI,	"AMD Radeon HD 7900 Series",	kFutomaki	},
    { 0x678A,	0x00000000, CHIP_FAMILY_TAHITI,	"AMD FirePro W8000",	kFutomaki	}, // AMD FirePro S9000/S9050/S10000
    { 0x6790,	0x00000000, CHIP_FAMILY_TAHITI,	"AMD Radeon HD 7970",	kFutomaki	}, // Gigabyte is dumb and used this for some of their cards
    { 0x6791,	0x00000000, CHIP_FAMILY_TAHITI,	"AMD Radeon HD 7900 Series",	kFutomaki	},
    { 0x6792,	0x00000000, CHIP_FAMILY_TAHITI,	"AMD Radeon HD 7900 Series",	kFutomaki	},
    { 0x6798,	0x00000000, CHIP_FAMILY_TAHITI,	"AMD Radeon HD 7970X/8970/R9 280X",	kFutomaki	},
    { 0x6799,	0x00000000, CHIP_FAMILY_TAHITI,	"AMD Radeon HD 7990 Series",	kAji		},
    { 0x679A,	0x00000000, CHIP_FAMILY_TAHITI,	"AMD Radeon HD 7950/8950/R9 280",	kFutomaki	},
    { 0x679B,	0x00000000, CHIP_FAMILY_TAHITI,	"AMD Radeon HD 7990 Series",	kChutoro	},
    { 0x679E,	0x00000000, CHIP_FAMILY_TAHITI,	"AMD Radeon HD 7870 XT",      kFutomaki	},
    { 0x679F,	0x00000000, CHIP_FAMILY_TAHITI,	"AMD Radeon HD 7950 Series",	kFutomaki	},
    
    // HAWAII
    //	{ 0x67A0,	0x00000000, CHIP_FAMILY_HAWAII,	"AMD Radeon",             kFutomaki	},
    //	{ 0x67A1,	0x00000000, CHIP_FAMILY_HAWAII,	"AMD Radeon",             kFutomaki	},
    //	{ 0x67A2,	0x00000000, CHIP_FAMILY_HAWAII,	"AMD Radeon",             kFutomaki	},
    //	{ 0x67A8,	0x00000000, CHIP_FAMILY_HAWAII,	"AMD Radeon",             kFutomaki	},
    //	{ 0x67A9,	0x00000000, CHIP_FAMILY_HAWAII,	"AMD Radeon",             kFutomaki	},
    //	{ 0x67AA,	0x00000000, CHIP_FAMILY_HAWAII,	"AMD Radeon",             kFutomaki	},
    { 0x67B0,	0x00000000, CHIP_FAMILY_HAWAII,	"AMD Radeon R9 290X",         kBaladi	},
    { 0x67B1,	0x00000000, CHIP_FAMILY_HAWAII,	"AMD Radeon R9 290/390",          kBaladi	},
    //	{ 0x67B8,	0x00000000, CHIP_FAMILY_HAWAII,	"AMD Radeon",             kFutomaki	},
    { 0x67B9,	0x00000000, CHIP_FAMILY_HAWAII,	"AMD Radeon R9 200",          kFutomaki	},
    //	{ 0x67BA,	0x00000000, CHIP_FAMILY_HAWAII,	"AMD Radeon",             kFutomaki	},
    //	{ 0x67BE,	0x00000000, CHIP_FAMILY_HAWAII,	"AMD Radeon",             kFutomaki	},
    
    // PITCAIRN
    { 0x6800,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon HD 7970M",        kBuri	}, // Mobile
    { 0x6801,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon HD 8970M Series",	kFutomaki	}, // Mobile
    //	{ 0x6802,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon HD ???M Series",	kFutomaki	}, // Mobile
    { 0x6806,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon HD 7600 Series",	kFutomaki	},
    { 0x6808,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon HD 7600 Series",	kFutomaki	},
    { 0x6809,	0x00000000, CHIP_FAMILY_PITCAIRN,	"ATI FirePro V",	kNull		},
    //Curacao
  { 0x6810,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon R9 270X",         kNamako		}, //AMD FirePro D300, AMD Radeon R9 M290X
    { 0x6811,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon R9 270",          kFutomaki		},
    //	{ 0x6816,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon",             kFutomaki		},
    //	{ 0x6817,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon",             kFutomaki		},
    { 0x6818,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon HD 7870 Series",	kFutomaki	},
    { 0x6819,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon HD 7850 Series",	kFutomaki	}, //R7 265
    
    // VERDE
    { 0x6820,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7700 Series",    kBuri	}, // Mobile
    { 0x6821,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7700 Series",    kBuri	}, // Mobile
    { 0x6822,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon E8860",             kBuri	}, // Mobile
    { 0x6823,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 8800M Series",   kBuri	}, // Mobile
    //	{ 0x6824,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7700M Series",	kBuri	}, // Mobile
    { 0x6825,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7800M Series",   kPondweed	}, // Mobile
    { 0x6826,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7700 Series",    kBuri	}, // Mobile
    { 0x6827,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7800M Series",   kPondweed	}, // Mobile
    { 0x6828,	0x00000000, CHIP_FAMILY_VERDE,	"ATI FirePro V",	kBuri	},
    //	{ 0x6829,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD ??? Series",	kBuri	},
    //	{ 0x682A,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD",	kBuri	}, // Mobile
    { 0x682B,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 8800M Series",   kBuri	}, // Mobile
    { 0x682D,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7700 Series",    kBuri	}, // Mobile
    { 0x682F,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7730 Series",    kBuri	}, // Mobile
    { 0x6830,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7800M Series",   kBuri	}, // Mobile
    { 0x6831,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7700 Series",    kBuri	}, // Mobile
    { 0x6835,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD R7 Series",      kBuri	},
    { 0x6837,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7730 Series",    kFutomaki },
    { 0x6838,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7700 Series",    kFutomaki },
    { 0x6839,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7700 Series",    kFutomaki },
    { 0x683B,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7700 Series",    kFutomaki },
    { 0x683D,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7770 Series",    kFutomaki }, //R7 250X
    { 0x683F,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7750 Series",    kFutomaki },
    
    //actually they are controlled by 6000Controller
    // TURKS
    { 0x6840,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 7670M Series",   kPondweed	}, // Mobile
    { 0x6841,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 7650M Series",   kPondweed	}, // Mobile
    { 0x6842,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 7600M Series",   kPondweed	}, // Mobile
    { 0x6843,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 7670M Series",   kPondweed	}, // Mobile
    { 0x6849,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 7600M Series",   kPondweed	},
    
    // PITCAIRN
    //	{ 0x684C,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon HD",	kNull	},
    
    // TURKS
    { 0x6850,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 7600M Series",   kPondweed   },
    { 0x6858,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 7400 Series",    kPondweed   },
    { 0x6859,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 7600M Series",   kPondweed   },
    
    //X3000 -
    //0x68881002 0x68891002 0x68981002 0x68991002 0x689C1002 0x689D1002 0x68801002 0x68901002 0x68A81002
    //0x68A91002 0x68B81002 0x68B91002 0x68BE1002 0x68A01002 0x68A11002 0x68B01002 0x68B11002 0x68C81002
    //0x68C91002 0x68D81002 0x68D91002 0x68DE1002 0x68C01002 0x68C11002 0x68D01002 0x68D11002 0x68E81002
    //0x68E91002 0x68F81002 0x68F91002 0x68FE1002 0x68E01002 0x68E11002 0x68F01002 0x68F11002 0x67011002
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
    //10.9 AMD5000
    //  0x68981002 0x68991002 0x68E01002 0x68E11002 0x68D81002 0x68C01002 0x68C11002 0x68D91002 0x68B81002
    //  0x68B01002 0x68B11002 0x68A01002 0x68A11002
    
    // CYPRESS
    //	{ 0x6880,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD",          kNull	}, // Mobile
    { 0x6888,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI FirePro V8800",          kNull	},
    { 0x6889,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI FirePro V7800",          kNull	},
    { 0x688A,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI FirePro V9800",          kNull	},
    { 0x688C,	0x00000000, CHIP_FAMILY_CYPRESS,	"AMD FireStream 9370",        kZonalis	},
    { 0x688D,	0x00000000, CHIP_FAMILY_CYPRESS,	"AMD FireStream 9350",        kZonalis	},
    { 0x6898,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5870 Series",	kUakari	},
    { 0x6899,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5850 Series",	kUakari	},
    { 0x689B,	0x00000000, CHIP_FAMILY_CYPRESS,	"AMD Radeon HD 6800 Series",	kNull	},
    
    // HEMLOCK
    { 0x689C,	0x00000000, CHIP_FAMILY_HEMLOCK,	"ATI Radeon HD 5970 Series",	kUakari	},
    { 0x689D,	0x00000000, CHIP_FAMILY_HEMLOCK,	"ATI Radeon HD 5900 Series",	kUakari	},
    
    // CYPRESS
    { 0x689E,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5800 Series",	kUakari	},
    
    // JUNIPER
    { 0x68A0,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770 Series",	kHoolock	}, // Mobile
    { 0x68A1,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5850 Series",	kHoolock	}, // Mobile
    { 0x68A8,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 6850M",        kHoolock	},
    { 0x68A9,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI FirePro V5800 (FireGL)",	kHoolock	},
    //was Vervet but Hoolock is better.
    //doesn't matter if you made connectors patch
    { 0x68B0,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770 Series",	kHoolock	}, // Mobile
    { 0x68B1,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770 Series",	kHoolock	},
    { 0x68B8,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770 Series",	kHoolock	},
    { 0x68B9,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5700 Series",	kHoolock	},
    { 0x68BA,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 6770 Series",	kHoolock	},
    { 0x68BC,	0x00000000, CHIP_FAMILY_JUNIPER,	"AMD FireStream 9370",        kHoolock	},
    { 0x68BD,	0x00000000, CHIP_FAMILY_JUNIPER,	"AMD FireStream 9350",        kHoolock	},
    { 0x68BE,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5750 Series",	kHoolock	},
    { 0x68BF,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 6750 Series",	kHoolock	},
    
    // REDWOOD
    { 0x68C0,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5730 Series",	kGalago	}, // Mobile
    { 0x68C1,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5650 Series",	kGalago	}, // Mobile
    { 0x68C7,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5570",	kGalago	}, // Mobile
    { 0x68C8,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI FirePro v4800",          kBaboon	},
    { 0x68C9,	0x00000000, CHIP_FAMILY_REDWOOD,	"FirePro 3D V3800",	kBaboon	},
    { 0x68D8,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5670 Series",	kBaboon	},
    { 0x68D9,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5570 Series",	kBaboon	},
    { 0x68DA,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5500 Series",	kBaboon	},
    { 0x68DE,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5000 Series",	kNull	},
    
    // CEDAR
    { 0x68E0,	0x00000000, CHIP_FAMILY_CEDAR,    "ATI Radeon HD 5470 Series",    kGalago	},
    { 0x68E1,	0x00000000, CHIP_FAMILY_CEDAR,    "AMD Radeon HD 6230/6350/8350", kGalago	},
    { 0x68E4,	0x00000000, CHIP_FAMILY_CEDAR,    "ATI Radeon HD 6370M Series",   kGalago	},
    { 0x68E5,	0x00000000, CHIP_FAMILY_CEDAR,    "ATI Radeon HD 6300M Series",   kGalago	},
    //	{ 0x68E8,	0x00000000, CHIP_FAMILY_CEDAR,	"ATI Radeon HD ??? Series",	kNull		},
    //	{ 0x68E9,	0x00000000, CHIP_FAMILY_CEDAR,	"ATI Radeon HD ??? Series",	kNull		},
    { 0x68F1,	0x00000000, CHIP_FAMILY_CEDAR,    "AMD FirePro 2460",             kEulemur	},
    { 0x68F2,	0x00000000, CHIP_FAMILY_CEDAR,    "AMD FirePro 2270",             kEulemur	},
    //	{ 0x68F8,	0x00000000, CHIP_FAMILY_CEDAR,	"ATI Radeon HD ??? Series",	kNull		},
    { 0x68F9,	0x00000000, CHIP_FAMILY_CEDAR,    "ATI Radeon HD 5450 Series",    kEulemur	},
    { 0x68FA,	0x00000000, CHIP_FAMILY_CEDAR,    "ATI Radeon HD 7300 Series",    kEulemur	},
    //	{ 0x68FE,	0x00000000, CHIP_FAMILY_CEDAR,	"ATI Radeon HD ??? Series",	kNull		},
    
    //
    { 0x6900,	0x00000000, CHIP_FAMILY_TOPAS,    "ATI Radeon R7 M260/M265",  kExmoor	},
    { 0x6901,	0x00000000, CHIP_FAMILY_TOPAS,    "ATI Radeon R5 M255",       kExmoor	},
    { 0x6920,	0x00000000, CHIP_FAMILY_AMETHYST, "ATI Radeon R9 M395",      kLabrador	},
    { 0x6921,	0x00000000, CHIP_FAMILY_AMETHYST, "ATI Radeon R9 M295X",      kExmoor	},
    { 0x692b,	0x00000000, CHIP_FAMILY_TONGA,    "ATI Firepro W7100",      kBaladi	},
    { 0x6938,	0x00000000, CHIP_FAMILY_AMETHYST, "ATI Radeon R9 M295X",      kExmoor	},
    { 0x6939,	0x00000000, CHIP_FAMILY_TONGA,    "ATI Radeon R9 285",        kBaladi	},
    
/*
    6900	Topaz XT [Radeon R7 M260/M265]
    6901	Topaz PRO [Radeon R5 M255]
    6920
    6921	Amethyst XT [Radeon R9 M295X]
    6929	Tonga PRO GL [FirePro Series]
    692b	Tonga PRO GL [FirePro W7100]
    692f	Tonga XT GL [FirePro W8100]
    6938	Amethyst XT [Radeon R9 M295X Mac Edition]
    6939	Tonga PRO [Radeon R9 285/380]
*/    
    /*
  //X1000 0x71871002 0x72101002 0x71DE1002 0x71461002 0x71421002 0x71091002 0x71C51002
  //      0x71C01002 0x72401002 0x72491002 0x72911002  
	// R520
	{ 0x7100, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x7101, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Mobile ",	kNull		},
	{ 0x7102, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Mobile ",	kNull		},
	{ 0x7103, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Mobile ",	kNull		},
	{ 0x7104, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x7105, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x7106, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Mobile ",	kNull		},
	{ 0x7108, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x7109, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x710A, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x710B, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x710C, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x710E, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x710F, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Desktop ",	kNull		},
*/
	// RV515
	{ 0x7140, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7141, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7142, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7143, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7144, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kCaretta	},
	{ 0x7145, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kCaretta	},
	//7146, 7187 - Caretta
	{ 0x7146, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7147, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7149, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kCaretta	},
	{ 0x714A, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kCaretta	},
	{ 0x714B, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kCaretta	},
	{ 0x714C, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kCaretta	},
	{ 0x714D, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x714E, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x714F, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7151, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7152, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7153, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x715E, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x715F, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7180, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7181, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7183, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7186, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kCaretta	},
	{ 0x7187, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD1900 ",	kCaretta	},
	{ 0x7188, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD2300 Mobile ",	kCaretta	},
	{ 0x718A, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kCaretta	},
	{ 0x718B, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kCaretta	},
	{ 0x718C, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kCaretta	},
	{ 0x718D, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kCaretta	},
	{ 0x718F, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7193, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7196, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kCaretta	},
	{ 0x719B, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x719F, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},

	// RV530
	{ 0x71C0, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",	kWormy		},
	{ 0x71C1, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",	kWormy		},
	{ 0x71C2, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",	kWormy		},
	{ 0x71C3, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",	kWormy		},
	{ 0x71C4, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Mobile ",	kWormy		},
	//71c5 -Wormy
	{ 0x71C5, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD1600 Mobile ",	kWormy		},
	{ 0x71C6, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",	kWormy		},
	{ 0x71C7, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",	kWormy		},
	{ 0x71CD, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",	kWormy		},
	{ 0x71CE, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",	kWormy		},
	{ 0x71D2, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",	kWormy		},
	{ 0x71D4, 0x00000000, CHIP_FAMILY_RV530, "ATI Mobility FireGL V5250",	kWormy		},
	{ 0x71D5, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Mobile ",	kWormy		},
	{ 0x71D6, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Mobile ",	kWormy		},
	{ 0x71DA, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",	kWormy		},
	{ 0x71DE, 0x00000000, CHIP_FAMILY_RV530, "ASUS M66 ATI Radeon Mobile ",	kWormy		},

	// RV515
	{ 0x7200, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kWormy		},
	{ 0x7210, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kWormy		},
	{ 0x7211, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kWormy		},
/*
	// R580
	{ 0x7240, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x7243, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x7244, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x7245, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x7246, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x7247, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x7248, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},
	//7249 -Alopias
	{ 0x7249, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x724A, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x724B, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x724C, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x724D, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x724E, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x724F, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},

	// RV570
	{ 0x7280, 0x00000000, CHIP_FAMILY_RV570, "ATI Radeon X1950 Pro ",	kAlopias	},

	// RV560
	{ 0x7281, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x7283, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",	kAlopias	},

	// R580
	{ 0x7284, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Mobile ",	kAlopias	},

	// RV560
	{ 0x7287, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",	kAlopias	},

	// RV570
	{ 0x7288, 0x00000000, CHIP_FAMILY_RV570, "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x7289, 0x00000000, CHIP_FAMILY_RV570, "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x728B, 0x00000000, CHIP_FAMILY_RV570, "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x728C, 0x00000000, CHIP_FAMILY_RV570, "ATI Radeon HD Desktop ",	kAlopias	},

	// RV560
	{ 0x7290, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x7291, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x7293, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x7297, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",	kAlopias	},

	// IGP

	// RS690
	{ 0x791E,	0x00000000, CHIP_FAMILY_RS690,	"ATI Radeon IGP ",	kNull			},
	{ 0x791F,	0x00000000, CHIP_FAMILY_RS690,	"ATI Radeon IGP ",	kNull			},
	{ 0x793F,	0x00000000, CHIP_FAMILY_RS690,	"ATI Radeon IGP ",	kNull			},
	{ 0x7941,	0x00000000, CHIP_FAMILY_RS690,	"ATI Radeon IGP ",	kNull			},
	{ 0x7942,	0x00000000, CHIP_FAMILY_RS690,	"ATI Radeon IGP ",	kNull			},


	// RS740
	{ 0x796C,	0x00000000, CHIP_FAMILY_RS740,	"ATI Radeon IGP ",	kNull			},
	{ 0x796D,	0x00000000, CHIP_FAMILY_RS740,	"ATI Radeon IGP ",	kNull			},
	{ 0x796E,	0x00000000, CHIP_FAMILY_RS740,	"ATI Radeon IGP ",	kNull			},
	{ 0x796F,	0x00000000, CHIP_FAMILY_RS740,	"ATI Radeon IGP ",	kNull			},

//native for Maverics ATIRadeonX2000.kext
  // 0x94001002 0x94011002 0x94021002 0x94031002 0x95811002 0x95831002 0x95881002 0x94c81002 0x94c91002
  // 0x95001002 0x95011002 0x95051002 0x95071002 0x95041002 0x95061002 0x95981002 0x94881002 0x95991002
  // 0x95911002 0x95931002 0x94401002 0x94421002 0x944A1002 0x945A1002 0x94901002 0x949E1002 0x94801002
  // 0x95401002 0x95411002 0x954E1002 0x954F1002 0x95521002 0x95531002 0x94a01002

	// standard/default models
*/
    //
	{ 0x9400,	0x00000000, CHIP_FAMILY_R600,	"ATI Radeon HD 2900 XT",      kNull		},
	{ 0x9401,	0x00000000, CHIP_FAMILY_R600,	"ATI Radeon HD 2900 GT",      kNull		},
	{ 0x9402,	0x00000000, CHIP_FAMILY_R600,	"ATI Radeon HD 2900 GT",      kNull		},
	{ 0x9403,	0x00000000, CHIP_FAMILY_R600,	"ATI Radeon HD 2900 GT",      kNull		},
	{ 0x9405,	0x00000000, CHIP_FAMILY_R600,	"ATI Radeon HD 2900 GT",      kNull		},
	{ 0x940A,	0x00000000, CHIP_FAMILY_R600,	"ATI FireGL V8650",           kNull		},
	{ 0x940B,	0x00000000, CHIP_FAMILY_R600,	"ATI FireGL V8600",           kNull		},
	{ 0x940F,	0x00000000, CHIP_FAMILY_R600,	"ATI FireGL V7600",           kNull		},

	// RV740
	{ 0x94A0,	0x00000000, CHIP_FAMILY_RV740,	"ATI Radeon HD 4830M",      kFlicker	},
	{ 0x94A1,	0x00000000, CHIP_FAMILY_RV740,	"ATI Radeon HD 4860M",      kFlicker	},
	{ 0x94A3,	0x00000000, CHIP_FAMILY_RV740,	"ATI FirePro M7740",        kFlicker	},
	{ 0x94B1,	0x00000000, CHIP_FAMILY_RV740,	"ATI Radeon HD",            kFlicker	},
	{ 0x94B3,	0x00000000, CHIP_FAMILY_RV740,	"ATI Radeon HD 4770",       kFlicker	},
	{ 0x94B4,	0x00000000, CHIP_FAMILY_RV740,	"ATI Radeon HD 4700 Series",	kFlicker	},
	{ 0x94B5,	0x00000000, CHIP_FAMILY_RV740,	"ATI Radeon HD 4770",       kFlicker	},
	{ 0x94B9,	0x00000000, CHIP_FAMILY_RV740,	"ATI Radeon HD",            kFlicker	},

	//9440, 944A - Cardinal
	// RV770
	{ 0x9440,	0x00000000, CHIP_FAMILY_RV770,	"ATI Radeon HD 4870 ",            kMotmot		},
	{ 0x9441,	0x00000000, CHIP_FAMILY_RV770,	"ATI Radeon HD 4870 X2",          kMotmot		},
	{ 0x9442,	0x00000000, CHIP_FAMILY_RV770,	"ATI Radeon HD 4850 Series",      kMotmot		},
	{ 0x9443,	0x00000000, CHIP_FAMILY_RV770,	"ATI Radeon HD 4850 X2",          kMotmot		},
	{ 0x9444,	0x00000000, CHIP_FAMILY_RV770,	"ATI FirePro V8750 (FireGL)",     kMotmot		},
	{ 0x9446,	0x00000000, CHIP_FAMILY_RV770,	"ATI FirePro V7770 (FireGL)",     kMotmot		},
	{ 0x9447,	0x00000000, CHIP_FAMILY_RV770,	"ATI FirePro V8700 Duo (FireGL)",	kMotmot		},
	{ 0x944A,	0x00000000, CHIP_FAMILY_RV770,	"ATI Mobility Radeon HD4850",     kMotmot		},//iMac - Quail
	{ 0x944B,	0x00000000, CHIP_FAMILY_RV770,	"ATI Mobility Radeon HD4850 X2",	kMotmot		},//iMac - Quail
	{ 0x944C,	0x00000000, CHIP_FAMILY_RV770,	"ATI Radeon HD 4830 Series",      kMotmot		},
	{ 0x944E,	0x00000000, CHIP_FAMILY_RV770,	"ATI Radeon HD 4810 Series",      kMotmot		},
	{ 0x9450,	0x00000000, CHIP_FAMILY_RV770,	"AMD FireStream 9270",            kMotmot		},
	{ 0x9452,	0x00000000, CHIP_FAMILY_RV770,	"AMD FireStream 9250",            kMotmot		},
	{ 0x9456,	0x00000000, CHIP_FAMILY_RV770,	"ATI FirePro V8700 (FireGL)",     kMotmot		},
	{ 0x945A,	0x00000000, CHIP_FAMILY_RV770,	"ATI Mobility Radeon HD 4870",    kMotmot		},
	{ 0x9460,	0x00000000, CHIP_FAMILY_RV770,	"ATI Radeon HD 4800 Series",      kMotmot		},
	{ 0x9462,	0x00000000, CHIP_FAMILY_RV770,	"ATI Radeon HD 4800 Series",      kMotmot		},
//	{ 0x946A,	0x00000000, CHIP_FAMILY_RV770,	"ATI Mobility Radeon",          kMotmot		},
//	{ 0x946B,	0x00000000, CHIP_FAMILY_RV770,	"ATI Mobility Radeon",          kMotmot		},
//	{ 0x947A,	0x00000000, CHIP_FAMILY_RV770,	"ATI Mobility Radeon",          kMotmot		},
//	{ 0x947B,	0x00000000, CHIP_FAMILY_RV770,	"ATI Mobility Radeon",          kMotmot		},

	//9488, 9490 - Gliff
	// RV730
	{ 0x9480,	0x00000000, CHIP_FAMILY_RV730,	"ATI Mobility Radeon HD 550v",	kGliff		},
	{ 0x9487,	0x00000000, CHIP_FAMILY_RV730,	"ATI Radeon HD Series",         kGliff		},
	{ 0x9488,	0x00000000, CHIP_FAMILY_RV730,	"ATI Radeon HD 4650 Series",    kGliff		},
	{ 0x9489,	0x00000000, CHIP_FAMILY_RV730,	"ATI Radeon HD Series",         kGliff		},
	{ 0x948A,	0x00000000, CHIP_FAMILY_RV730,	"ATI Radeon HD Series",         kGliff		},
	{ 0x948F,	0x00000000, CHIP_FAMILY_RV730,	"ATI Radeon HD Series",         kGliff		},
	{ 0x9490,	0x00000000, CHIP_FAMILY_RV730,	"ATI Radeon HD 4670 Series",    kGliff		},
	{ 0x9491,	0x00000000, CHIP_FAMILY_RV730,	"ATI Radeon HD 4600 Series",    kGliff		},
	{ 0x9495,	0x00000000, CHIP_FAMILY_RV730,	"ATI Radeon HD 4650 Series",    kGliff		},
	{ 0x9498,	0x00000000, CHIP_FAMILY_RV730,	"ATI Radeon HD 4710 Series",    kGliff		},
	{ 0x949C,	0x00000000, CHIP_FAMILY_RV730,	"ATI FirePro V7750 (FireGL)",   kGliff		},
	{ 0x949E,	0x00000000, CHIP_FAMILY_RV730,	"ATI FirePro V5700 (FireGL)",   kGliff		},
	{ 0x949F,	0x00000000, CHIP_FAMILY_RV730,	"ATI FirePro V3750 (FireGL)",   kGliff		},

	//94C8 -Iago
	// RV610
/*	{ 0x94C0,	0x00000000, CHIP_FAMILY_RV610,	"ATI Radeon HD Series",	kIago		},
	{ 0x94C1,	0x00000000, CHIP_FAMILY_RV610,	"ATI Radeon HD 2400 Series",	kIago		},
	{ 0x94C3,	0x00000000, CHIP_FAMILY_RV610,	"ATI Radeon HD 2350 Series",	kIago		},
	{ 0x94C4,	0x00000000, CHIP_FAMILY_RV610,	"ATI Radeon HD 2400 Series",	kIago		},
	{ 0x94C5,	0x00000000, CHIP_FAMILY_RV610,	"ATI Radeon HD 2400 Series",	kIago		},
	{ 0x94C6,	0x00000000, CHIP_FAMILY_RV610,	"ATI Radeon HD 2400 Series",	kIago		},
	{ 0x94C7,	0x00000000, CHIP_FAMILY_RV610,	"ATI Radeon HD 2350",	kIago		},
	{ 0x94C8,	0x00000000, CHIP_FAMILY_RV610,	"ATI Radeon HD 2400 Series",	kIago		},
	{ 0x94C9,	0x00000000, CHIP_FAMILY_RV610,	"ATI Radeon HD 2400 Series",	kIago		},
	{ 0x94CB,	0x00000000, CHIP_FAMILY_RV610,	"ATI Radeon HD 2400 Series",	kIago		},
	{ 0x94CC,	0x00000000, CHIP_FAMILY_RV610,	"ATI Radeon HD 2400 Series",	kIago		},
	{ 0x94CD,	0x00000000, CHIP_FAMILY_RV610,	"ATI Radeon HD 2400 PRO Series",	kIago	},

	//9501 - Megalodon, Triakis HD3800
	{ 0x9500,	0x00000000, CHIP_FAMILY_RV670,	"ATI Radeon HD 3800 Series",	kMegalodon	},
	{ 0x9501,	0x00000000, CHIP_FAMILY_RV670,	"ATI Radeon HD 3690 Series",	kMegalodon	},
	{ 0x9504,	0x00000000, CHIP_FAMILY_RV670,	"ATI Radeon HD 3850M Series",	kMegalodon	},
	{ 0x9505,	0x00000000, CHIP_FAMILY_RV670,	"ATI Radeon HD 3800 Series",	kMegalodon	},
	{ 0x9506,	0x00000000, CHIP_FAMILY_RV670,	"ATI Radeon HD 3850 X2 M Series",	kMegalodon	},
	{ 0x9507,	0x00000000, CHIP_FAMILY_RV670,	"ATI Radeon HD 3830",	kMegalodon	},
	{ 0x9508,	0x00000000, CHIP_FAMILY_RV670,	"ATI Radeon HD 3870M Series",	kMegalodon	},
	{ 0x9509,	0x00000000, CHIP_FAMILY_RV670,	"ATI Radeon HD 3870 X2 MSeries",	kMegalodon	},
	{ 0x950F,	0x00000000, CHIP_FAMILY_RV670,	"ATI Radeon HD 3870 X2",	kMegalodon	},
	{ 0x9511,	0x00000000, CHIP_FAMILY_RV670,	"ATI Radeon HD 3850 X2",	kMegalodon	},
	{ 0x9513,	0x00000000, CHIP_FAMILY_RV670,	"ATI Radeon HD 3850 X2",	kMegalodon	},
	{ 0x9515,	0x00000000, CHIP_FAMILY_RV670,	"ATI Radeon HD 3850 Series",	kMegalodon	},
	{ 0x9517,	0x00000000, CHIP_FAMILY_RV670,	"ATI Radeon HD Series",	kMegalodon	},
	{ 0x9519,	0x00000000, CHIP_FAMILY_RV670,	"AMD FireStream 9170",	kMegalodon	},
*/
	// RV710
	{ 0x9540,	0x00000000, CHIP_FAMILY_RV710,	"ATI Radeon HD 4550",           kFlicker	},
	{ 0x9541,	0x00000000, CHIP_FAMILY_RV710,	"ATI Radeon HD",                kFlicker	},
	{ 0x9542,	0x00000000, CHIP_FAMILY_RV710,	"ATI Radeon HD",                kFlicker	},
	{ 0x954E,	0x00000000, CHIP_FAMILY_RV710,	"ATI Radeon HD",                kFlicker	},
	{ 0x954F,	0x00000000, CHIP_FAMILY_RV710,	"ATI Radeon HD 4350",           kFlicker	},
	{ 0x9552,	0x00000000, CHIP_FAMILY_RV710,	"ATI Mobility Radeon HD 4330", 	kShrike     },
	{ 0x9553,	0x00000000, CHIP_FAMILY_RV710,	"ATI Mobility Radeon HD 4570", 	kShrike     },
	{ 0x9555,	0x00000000, CHIP_FAMILY_RV710,	"ATI Mobility Radeon HD 4550", 	kShrike     },
	{ 0x9557,	0x00000000, CHIP_FAMILY_RV710,	"ATI FirePro RG220",            kFlicker	},
	{ 0x955F,	0x00000000, CHIP_FAMILY_RV710,	"ATI Radeon HD 4330M series",   kFlicker	},
/*
	//9583, 9588 - Lamna, Hypoprion HD2600
	// RV630
	{ 0x9580,	0x00000000, CHIP_FAMILY_RV630,	"ATI Radeon HD Series",	kHypoprion	},
	{ 0x9581,	0x00000000, CHIP_FAMILY_RV630,	"ATI Radeon HD 2600 XT",	kHypoprion  },
	{ 0x9583,	0x00000000, CHIP_FAMILY_RV630,	"ATI Radeon HD 2600 XT",	kHypoprion  },
	{ 0x9586,	0x00000000, CHIP_FAMILY_RV630,	"ATI Radeon HD 2600 XT Series",	kHypoprion	},
	{ 0x9587,	0x00000000, CHIP_FAMILY_RV630,	"ATI Radeon HD 2600 Pro Series",	kHypoprion	},
	{ 0x9588,	0x00000000, CHIP_FAMILY_RV630,	"ATI Radeon HD 2600 XT",        kHypoprion  },
	{ 0x9589,	0x00000000, CHIP_FAMILY_RV630,	"ATI Radeon HD 2600 PRO",       kHypoprion  },
	{ 0x958A,	0x00000000, CHIP_FAMILY_RV630,	"ATI Radeon HD 2600 X2 Series",	kLamna      },
	{ 0x958B,	0x00000000, CHIP_FAMILY_RV630,	"ATI Radeon HD 2600 X2 Series",	kLamna      },
	{ 0x958C,	0x00000000, CHIP_FAMILY_RV630,	"ATI Radeon HD 2600 X2 Series",	kLamna      },
	{ 0x958D,	0x00000000, CHIP_FAMILY_RV630,	"ATI Radeon HD 2600 X2 Series",	kLamna      },
	{ 0x958E,	0x00000000, CHIP_FAMILY_RV630,	"ATI Radeon HD 2600 X2 Series",	kLamna      },
	{ 0x958F,	0x00000000, CHIP_FAMILY_RV630,	"ATI Radeon HD Series",	kHypoprion	},

	// RV635
//	{ 0x9590,	0x00000000, CHIP_FAMILY_RV635,	"ATI Radeon HD",	kMegalodon  },
	{ 0x9591,	0x00000000, CHIP_FAMILY_RV635,	"ATI Radeon HD 3600 Series",	kMegalodon  }, // Mobile
//	{ 0x9593,	0x00000000, CHIP_FAMILY_RV635,	"ATI Radeon HD",	kMegalodon  }, // Mobile
//	{ 0x9595,	0x00000000, CHIP_FAMILY_RV635,	"ATI Radeon HD",	kMegalodon  }, // Mobile
//	{ 0x9596,	0x00000000, CHIP_FAMILY_RV635,	"ATI Radeon HD",	kMegalodon  },
//	{ 0x9597,	0x00000000, CHIP_FAMILY_RV635,	"ATI Radeon HD",	kMegalodon  },
	{ 0x9598,	0x00000000, CHIP_FAMILY_RV635,	"ATI Radeon HD 3600 Series",	kMegalodon  },
//	{ 0x9599,	0x00000000, CHIP_FAMILY_RV635,	"ATI Radeon HD",	kMegalodon  },
//	{ 0x959B,	0x00000000, CHIP_FAMILY_RV635,	"ATI Radeon HD",	kMegalodon  }, // Mobile

	// RV620
	{ 0x95C0,	0x00000000, CHIP_FAMILY_RV620,	"ATI Radeon HD 3550 Series",	kIago       },
//	{ 0x95C2,	0x00000000, CHIP_FAMILY_RV620,	"ATI Radeon HD",	kIago       }, // Mobile
	{ 0x95C4,	0x00000000, CHIP_FAMILY_RV620,	"ATI Radeon HD 3470 Series",	kIago       }, // Mobile
	{ 0x95C5,	0x00000000, CHIP_FAMILY_RV620,	"ATI Radeon HD 3450 Series",	kIago       },
	{ 0x95C6,	0x00000000, CHIP_FAMILY_RV620,	"ATI Radeon HD 3450 AGP",	kIago       },
//	{ 0x95C7,	0x00000000, CHIP_FAMILY_RV620,	"ATI Radeon HD",	kIago       },
//	{ 0x95C9,	0x00000000, CHIP_FAMILY_RV620,	"ATI Radeon HD",	kIago       },
//	{ 0x95CC,	0x00000000, CHIP_FAMILY_RV620,	"ATI Radeon HD",	kIago       },
//	{ 0x95CD,	0x00000000, CHIP_FAMILY_RV620,	"ATI Radeon HD",	kIago       },
//	{ 0x95CE,	0x00000000, CHIP_FAMILY_RV620,	"ATI Radeon HD",	kIago       },
     { 0x95CF,	0x00000000, CHIP_FAMILY_RV620,	"ATI FirePro 2260",	kIago       },

	// IGP

	// RS780
	{ 0x9610,	0x00000000, CHIP_FAMILY_RS780,	"ATI Radeon HD 3200 Graphics",	kNull       },
	{ 0x9611,	0x00000000, CHIP_FAMILY_RS780,	"ATI Radeon HD 3100 Graphics",	kNull       },
//	{ 0x9612,	0x00000000, CHIP_FAMILY_RS780,	"ATI Radeon HD",	kNull       },
//	{ 0x9613,	0x00000000, CHIP_FAMILY_RS780,	"ATI Radeon HD",	kNull       },
	{ 0x9614,	0x00000000, CHIP_FAMILY_RS780,	"ATI Radeon HD 3300 Graphics",	kNull       },
//	{ 0x9615,	0x00000000, CHIP_FAMILY_RS780,	"ATI Radeon HD",	kNull       },
	{ 0x9616,	0x00000000, CHIP_FAMILY_RS780,	"AMD 760G",                     kNull       },

	// SUMO
	//mobile = G desktop = D
	{ 0x9640,	0x00000000, CHIP_FAMILY_SUMO,	"AMD Radeon HD 6550D",	kNull       },
	{ 0x9641,	0x00000000, CHIP_FAMILY_SUMO,	"AMD Radeon HD 6620G",	kNull       }, // Mobile

    // SUMO2
	{ 0x9642,	0x00000000, CHIP_FAMILY_SUMO2,	"AMD Radeon HD 6370D",	kNull       },
	{ 0x9643,	0x00000000, CHIP_FAMILY_SUMO2,	"AMD Radeon HD 6380G",	kNull       }, // Mobile
	{ 0x9644,	0x00000000, CHIP_FAMILY_SUMO2,	"AMD Radeon HD 6410D",	kNull       },
	{ 0x9645,	0x00000000, CHIP_FAMILY_SUMO2,	"AMD Radeon HD 6410D",	kNull       }, // Mobile

    // SUMO
	{ 0x9647,	0x00000000, CHIP_FAMILY_SUMO,	"AMD Radeon HD 6520G",	kNull       }, // Mobile
	{ 0x9648,	0x00000000, CHIP_FAMILY_SUMO,	"AMD Radeon HD 6480G",	kNull       }, // Mobile

    // SUMO2
	{ 0x9649,	0x00000000, CHIP_FAMILY_SUMO2,	"AMD Radeon(TM) HD 6480G",	kNull       }, // Mobile

    // SUMO
	{ 0x964A,	0x00000000, CHIP_FAMILY_SUMO,	"AMD Radeon HD 6530D",	kNull       },
//	{ 0x964B,	0x00000000, CHIP_FAMILY_SUMO,	"AMD Radeon HD",	kNull       },
//	{ 0x964C,	0x00000000, CHIP_FAMILY_SUMO,	"AMD Radeon HD",	kNull       },
//	{ 0x964E,	0x00000000, CHIP_FAMILY_SUMO,	"AMD Radeon HD",	kNull       }, // Mobile
//	{ 0x964F,	0x00000000, CHIP_FAMILY_SUMO,	"AMD Radeon HD",	kNull       }, // Mobile

	// RS880
	{ 0x9710,	0x00000000, CHIP_FAMILY_RS880,	"ATI Radeon HD 4200 Series",	kNull	},
//	{ 0x9711,	0x00000000, CHIP_FAMILY_RS880,	"ATI Radeon HD",	kNull	},
	{ 0x9712,	0x00000000, CHIP_FAMILY_RS880,	"ATI Radeon HD 4200 Series",	kNull	}, // Mobile
//	{ 0x9713,	0x00000000, CHIP_FAMILY_RS880,	"ATI Radeon HD",	kNull	}, // Mobile
	{ 0x9714,	0x00000000, CHIP_FAMILY_RS880,	"ATI Radeon HD 4290",	kNull	},
	{ 0x9715,	0x00000000, CHIP_FAMILY_RS880,	"ATI Radeon HD 4250",	kNull	},
	{ 0x9723,	0x00000000, CHIP_FAMILY_RS880,	"ATI Radeon HD 5450 Series",	kNull	},

	// PALM // 0x9804 - AMD HD6250 Wrestler
	{ 0x9802,	0x00000000, CHIP_FAMILY_PALM,	"AMD Radeon HD 6310 Graphics",  kNull       },
	{ 0x9803,	0x00000000, CHIP_FAMILY_PALM,	"AMD Radeon HD 6250 Graphics",  kNull       },
	{ 0x9804,	0x00000000, CHIP_FAMILY_PALM,	"AMD Radeon HD 6250 Graphics",  kNull       },
	{ 0x9805,	0x00000000, CHIP_FAMILY_PALM,	"AMD Radeon HD 6250 Graphics",  kNull       },
	{ 0x9806,	0x00000000, CHIP_FAMILY_PALM,	"AMD Radeon HD 6320 Graphics",  kNull       },
	{ 0x9807,	0x00000000, CHIP_FAMILY_PALM,	"AMD Radeon HD 6290 Graphics",  kNull       },
	{ 0x9808,	0x00000000, CHIP_FAMILY_PALM,	"AMD Radeon HD 7340 Graphics",  kNull       },
	{ 0x9809,	0x00000000, CHIP_FAMILY_PALM,	"AMD Radeon HD 7310 Graphics",  kNull       },
//	{ 0x980A,	0x00000000, CHIP_FAMILY_PALM,	"AMD Radeon HD",  kNull       },

	// KABINI
//	{ 0x9830,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       }, // Mobile
//	{ 0x9831,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       },
//	{ 0x9832,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       }, // Mobile
//	{ 0x9833,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       },
//	{ 0x9834,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       }, // Mobile
//	{ 0x9835,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       },
//	{ 0x9836,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       }, // Mobile
//	{ 0x9837,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       },
//	{ 0x9838,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       }, // Mobile
//	{ 0x9839,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       }, // Mobile
//	{ 0x983A,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       },
//	{ 0x983B,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       }, // Mobile
//	{ 0x983C,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       },
     { 0x983D,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD 8250",  kNull       },
//	{ 0x983E,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       },
//	{ 0x983F,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       },

     // MULLINS
     { 0x9850,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
     { 0x9851,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
     { 0x9852,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
     { 0x9853,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
     { 0x9854,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
     { 0x9855,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
     { 0x9856,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
     { 0x9857,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
     { 0x9858,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
     { 0x9859,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
     { 0x985A,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
     { 0x985B,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
     { 0x985C,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
     { 0x985D,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
     { 0x985E,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
     { 0x985F,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
     
	// ARUBA //TrinityGL //mobile = G desktop = D
	{ 0x9900,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7660G",      kNull       }, // Mobile
	{ 0x9901,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7660D",      kNull       },
	{ 0x9903,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7640G",      kNull       }, // Mobile
	{ 0x9904,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7560D",      kNull       },
//	{ 0x9905,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD",      kNull       },
	{ 0x9906,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD FirePro A300 Series",  kNull       },
	{ 0x9907,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7620G",      kNull       }, // Mobile
	{ 0x9908,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7600G",      kNull       }, // Mobile
//	{ 0x9909,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD",      kNull       }, // Mobile
     { 0x990A,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7500G",      kNull       }, // Mobile
     { 0x990B,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 8650G",      kNull       }, // Mobile
     { 0x990C,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 8670D",      kNull       },
     { 0x990D,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 8550G",      kNull       }, // Mobile
     { 0x990E,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 8570D",      kNull       },
     { 0x990F,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 8610G",      kNull       }, // Mobile
	{ 0x9910,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7660G",      kNull       }, // Mobile
	{ 0x9913,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7640G",      kNull       }, // Mobile
     { 0x9917,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7620G",      kNull       },
     { 0x9918,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7600G",      kNull       },
     { 0x9919,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7500G",      kNull       },
	{ 0x9990,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7520G",      kNull       }, // Mobile
	{ 0x9991,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7540D",      kNull       },
	{ 0x9992,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7420G",      kNull       }, // Mobile
     { 0x9993,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7480D",      kNull       },
	{ 0x9994,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7400G",      kNull       }, // Mobile
     { 0x9995,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 8450G",      kNull       }, // Mobile
     { 0x9996,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 8470D",      kNull       },
     { 0x9997,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 8350G",      kNull       }, // Mobile
     { 0x9998,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 8370D",      kNull       },
     { 0x9999,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 8510G",      kNull       }, // Mobile
     { 0x999A,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 8410G",      kNull       }, // Mobile
     { 0x999B,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 8310G",      kNull       }, // Mobile
     { 0x999C,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 8650D",      kNull       },
     { 0x999D,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 8550D",      kNull       },
     { 0x99A0,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7520G",      kNull       }, // Mobile
     { 0x99A2,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7420G",      kNull       }, // Mobile
     { 0x99A4,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7400G",      kNull       },
*/

	{ 0x0000,	0x00000000, CHIP_FAMILY_UNKNOW,	"AMD Unknown",			kNull		}
};

//native ID for 10.8.3
/*
 ATI7000
 0x26001002 0x22001002 0x67901002 0x67981002 0x679A1002 0x679E1002 0x67801002 0x68201002 0x68211002 
 0x68251002 0x68271002 0x682D1002 0x682F1002 0x68391002 0x683B1002 0x683D1002 0x683F1002 0x68001002 
 0x68061002 0x68081002 0x68181002

 Barts
 0x67381002 0x67391002 0x67201002 0x67221002

 Bonaire
 0x66401002 0x66501002

 Caicos
 0x67681002 0x67701002 0x67791002 0x67601002 0x67611002

 Cayman
 0x67181002 0x67191002 0x67041002

 Cedar
 0x68E01002

 Cypress
 0x68981002 0x68991002

 Juniper
 0x68B81002 0x68B01002 0x68B11002 0x68A01002 0x68A11002

 Lombok = R476
 0x68401002 0x68411002

 Pitcairn = R575B
 0x68001002 0x68061002 0x68081002 0x68181002

 Redwood
 0x68D81002 0x68C01002 0x68C11002 0x68D91002

 Tahiti - R9-280X
 0x67901002 0x67981002 0x679A1002 0x679E1002 0x67801002

 Turks = NI
 0x67501002 0x67581002 0x67591002 0x67401002 0x67411002 0x67451002

 Verde = R575A, R576
 0x68201002 0x68211002 0x68251002 0x68271002 0x682D1002 0x682F1002 0x68391002 0x683B1002 0x683D1002 0x683F1002
Yosemite news
 AMD8000Controller
 
 0x67B01002 - R9 290X
 0x665C1002 - HD 7790
 0x665D1002 - Radeon R7 200 (Bonaire)
 0x66511002
 0x66501002
 0x66461002
 0x66411002
 0x66401002
 0x46001002
 
 AMD9000Controller
 
 0x69201002
 0x69211002
 0x69381002
 0x69391002
 
*/

const CHAR8 *chip_family_name[] = {
	"UNKNOW",
	"R420",
    "R423",
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
	"RV772",
	"RV790",
	/* Evergreen */
	"Cedar",
	"Cypress",
	"Hemlock",
	"Juniper",
	"Redwood",
    //   "Broadway",
	/* Northern Islands */
	"Barts",
	"Caicos",
	"Cayman",
	"Turks",
	/* Southern Islands */
    "Palm",
    "Sumo",
    "Sumo2",
    "Aruba",
	"Tahiti",
	"Pitcairn",
    "Verde",
    "Oland",
    "Hainan",
    "Bonaire",
    "Kaveri",
    "Kabini",
    "Hawaii",
    /* ... */
    "Mullins",
    "Topas",
    "Amethyst",
    "Tonga",
	""
};

AtiDevProp ati_devprop_list[] = {
	{FLAGTRUE,	FALSE,	"@0,AAPL,boot-display",		get_bootdisplay_val,	NULVAL				},
  //	{FLAGTRUE,	FALSE,	"@0,ATY,EFIDisplay",		NULL,					STRVAL("TMDSA")			},
	
  //{FLAGTRUE,	TRUE,	"@0,AAPL,vram-memory",		get_vrammemory_val,		NULVAL				},
  {FLAGTRUE,	TRUE,	"AAPL00,override-no-connect",		get_edid_val,       NULVAL        },
  {FLAGNOTFAKE,	TRUE,	"@0,compatible",              get_name_val,       NULVAL				},
  {FLAGTRUE,	TRUE,	"@0,connector-type",          get_conntype_val,		NULVAL        },
  {FLAGTRUE,	TRUE,	"@0,device_type",             NULL,					STRVAL("display")   },
//	{FLAGTRUE,	FALSE,	"@0,display-connect-flags", NULL,				DWRVAL(0)   },
  
  //some set of properties for mobile radeons
  {FLAGMOBILE,	FALSE,	"@0,display-link-component-bits",  NULL,		DWRVAL(6)	},
  {FLAGMOBILE,	FALSE,	"@0,display-pixel-component-bits", NULL,		DWRVAL(6)	},
  {FLAGMOBILE,	FALSE,	"@0,display-dither-support",       NULL,		DWRVAL(0)	},
  {FLAGMOBILE,	FALSE,	"@0,backlight-control",       NULL,		DWRVAL(1)	},
  {FLAGTRUE,  	FALSE,	"AAPL00,Dither", NULL,		DWRVAL(0)	},
  
  
//  {FLAGTRUE,	TRUE,	"@0,display-type",          NULL,					STRVAL("NONE")			},
	{FLAGTRUE,	TRUE,	"@0,name",                    get_name_val,			NULVAL          },
  {FLAGTRUE,	TRUE,	"@0,VRAM,memsize",			get_vrammemsize_val,	NULVAL          },
//  {FLAGTRUE,	TRUE,	"@0,ATY,memsize",		  	get_vrammemsize_val,	NULVAL          },
	
  {FLAGTRUE,	FALSE,	"AAPL,aux-power-connected", NULL,					DWRVAL(1)		},
  {FLAGTRUE,	FALSE,	"AAPL00,DualLink",          get_dual_link_val,			NULVAL 	},
  {FLAGMOBILE,	FALSE,	"AAPL,HasPanel",          NULL,					DWRVAL(1)   },
  {FLAGMOBILE,	FALSE,	"AAPL,HasLid",            NULL,					DWRVAL(1)   },
  {FLAGMOBILE,	FALSE,	"AAPL,backlight-control", NULL,					DWRVAL(1)   },
	{FLAGTRUE,	FALSE,	"AAPL,overwrite_binimage",	get_binimage_owr,		NULVAL				},
	{FLAGTRUE,	FALSE,	"ATY,bin_image",            get_binimage_val,		NULVAL				},
	{FLAGTRUE,	FALSE,	"ATY,Copyright",	NULL,	STRVAL("Copyright AMD Inc. All Rights Reserved. 2005-2011") },
  {FLAGTRUE,	FALSE,	"ATY,EFIVersion",	NULL,	STRVAL("01.00.3180")                  },
	{FLAGTRUE,	FALSE,	"ATY,Card#",			get_romrevision_val,	NULVAL                },
//  {FLAGTRUE,	FALSE,	"ATY,Rom#",	NULL,	STRVAL("www.amd.com")                  },
	{FLAGNOTFAKE,	FALSE,	"ATY,VendorID",		NULL,					WRDVAL(0x1002)        },
	{FLAGNOTFAKE,	FALSE,	"ATY,DeviceID",		get_deviceid_val,		NULVAL                  },
	
  //	{FLAGTRUE,	FALSE,	"ATY,MCLK",					get_mclk_val,			NULVAL							},
  //	{FLAGTRUE,	FALSE,	"ATY,SCLK",					get_sclk_val,			NULVAL							},
  {FLAGTRUE,	FALSE,	"ATY,RefCLK",				get_refclk_val,			DWRVAL(0x0a8c)		},
	
  {FLAGTRUE,	FALSE,	"ATY,PlatformInfo",			get_platforminfo_val,	NULVAL					},
	{FLAGOLD, 	FALSE,	"compatible",						get_name_pci_val,     NULVAL							},
	{FLAGTRUE,	FALSE,	"name",						get_nameparent_val,     NULVAL							},
	{FLAGTRUE,	FALSE,	"device_type",		get_nameparent_val,     NULVAL							},
	{FLAGTRUE,	FALSE,	"model",					get_model_val,          STRVAL("ATI Radeon")},
//  {FLAGTRUE,	FALSE,	"VRAM,totalsize",	get_vramtotalsize_val,	NULVAL              },
	
	{FLAGTRUE,	FALSE,	NULL,	NULL,	NULVAL}
};

BOOLEAN get_bootdisplay_val(value_t *val, INTN index)
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

BOOLEAN get_dual_link_val(value_t *val, INTN index)
{
  static UINT32 v = 0;
	
	if (v)
		return FALSE;

  v = gSettings.DualLink;
	val->type = kCst;
	val->size = 4;
	val->data = (UINT8 *)&v;
  
  return TRUE;
}


BOOLEAN get_vrammemory_val(value_t *val, INTN index)
{
	return FALSE;
}

BOOLEAN get_edid_val(value_t *val, INTN index)
{
  static UINT32 v = 0;
  if (!gSettings.InjectEDID) {
    return FALSE;
  }
	
	if (v)
		return FALSE;
	  
  if (!gSettings.CustomEDID) {
    return FALSE;
  }
  v = 1;
  val->type = kPtr;
  val->size = 128;
  val->data = AllocateCopyPool(val->size, gSettings.CustomEDID);
  return TRUE;
}

static CONST CHAR8* dtyp[] = {"LCD", "CRT", "DVI", "NONE"};
static UINT32 dti = 0;

BOOLEAN get_display_type(value_t *val, INTN index)
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


BOOLEAN get_name_val(value_t *val, INTN index)
{
	val->type = aty_name.type;
	val->size = aty_name.size;
	val->data = aty_name.data;
	
	return TRUE;
}

BOOLEAN get_nameparent_val(value_t *val, INTN index)
{
	val->type = aty_nameparent.type;
	val->size = aty_nameparent.size;
	val->data = aty_nameparent.data;
	
	return TRUE;
}

static CHAR8 pciName[15];
BOOLEAN get_name_pci_val(value_t *val, INTN index)
{  
	if (!card->info->model_name || !gSettings.FakeATI)
		return FALSE;
  AsciiSPrint(pciName, 15, "pci1002,%x", gSettings.FakeATI >> 16);
  LowCase(pciName);
	val->type = kStr;
	val->size = 13;
	val->data = (UINT8 *)&pciName[0];

	return TRUE;
}


BOOLEAN get_model_val(value_t *val, INTN index)
{
	if (!card->info->model_name)
		return FALSE;
	
	val->type = kStr;
	val->size = (UINT32)AsciiStrLen(card->info->model_name);
	val->data = (UINT8 *)card->info->model_name;
	
	return TRUE;
}

//static CONST UINT32 ctm[] = {0x02, 0x10, 0x800, 0x400}; //mobile
//static CONST UINT32 ctd[] = {0x04, 0x10, 0x800, 0x400}; //desktop
//static UINT32 cti = 0;

KERNEL_AND_KEXT_PATCHES *CurrentPatches;

//TODO - get connectors from ATIConnectorsPatch
BOOLEAN get_conntype_val(value_t *val, INTN index)
{
  UINT8 *ct;
//Connector types:
//0x10:  VGA
//0x04:  DL DVI-I
//0x800: HDMI
//0x400: DisplayPort
//0x02:  LVDS  
  
  if ((CurrentPatches == NULL) || (CurrentPatches->KPATIConnectorsDataLen == 0)) {
    return FALSE;
  }
  ct = CurrentPatches->KPATIConnectorsPatch;
  
/*  if (gMobile) {
    ct = (UINT32*)&ctm[0];
  } else
    ct = (UINT32*)&ctd[0]; */
  
  val->type = kCst;
	val->size = 4;
	val->data = (UINT8*)&ct[index * 16];
  
//  cti++;
//  if(cti > 3) cti = 0;
  
	return TRUE;
}

BOOLEAN get_vrammemsize_val(value_t *val, INTN index)
{
	static INTN idx = -1;
	static UINT64 memsize;
	
	idx++;
	memsize = LShiftU64((UINT64)card->vram_size, 32);
	if (idx == 0)
		memsize = memsize | (UINT64)card->vram_size;
	
	val->type = kCst;
	val->size = 8;
	val->data = (UINT8 *)&memsize;
	
	return TRUE;
}

BOOLEAN get_binimage_val(value_t *val, INTN index)
{
	if (!card->rom)
		return FALSE;
	
	val->type = kPtr;
	val->size = card->rom_size;
	val->data = card->rom;
	
	return TRUE;
}

BOOLEAN get_binimage_owr(value_t *val, INTN index)
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



BOOLEAN get_romrevision_val(value_t *val, INTN index)
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
	val->size = (UINT32)AsciiStrLen((CHAR8 *)rev);
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

BOOLEAN get_deviceid_val(value_t *val, INTN index)
{
	val->type = kCst;
	val->size = 2;
	val->data = (UINT8 *)&card->pci_dev->device_id;
	
	return TRUE;
}

BOOLEAN get_mclk_val(value_t *val, INTN index)
{
	return FALSE;
}

BOOLEAN get_sclk_val(value_t *val, INTN index)
{
	return FALSE;
}

BOOLEAN get_refclk_val(value_t *val, INTN index)
{
  if (!gSettings.RefCLK) {
    return FALSE;
  }
//	
  val->type = kCst;
	val->size = 4;
	val->data = (UINT8 *)&gSettings.RefCLK;
	
	return TRUE;
  
}

BOOLEAN get_platforminfo_val(value_t *val, INTN index)
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

BOOLEAN get_vramtotalsize_val(value_t *val, INTN index)
{
  
	val->type = kCst;
	val->size = 4;
	val->data = (UINT8 *)&card->vram_size;
	
	return TRUE;
}

VOID free_val(value_t *val )
{
	if (val->type == kPtr)
		FreePool(val->data);
	
	ZeroMem(val, sizeof(value_t));
}

//  {FLAGTRUE,	TRUE,	"@0,compatible",    get_name_val,       NULVAL				},
// 	{FLAGTRUE,	FALSE,	"ATY,VendorID",		NULL,					WRDVAL(0x1002)        },
/*typedef struct {
	UINT32				flags;
	BOOLEAN				all_ports;
	CHAR8					*name;
	BOOLEAN				(*get_value)(value_t *val, INTN index);
	value_t				default_val;
} AtiDevProp;
*/
VOID devprop_add_list(AtiDevProp devprop_list[])
{
	INTN i, pnum;
	value_t *val = AllocateZeroPool(sizeof(value_t));
	
	for (i = 0; devprop_list[i].name != NULL; i++) {
		if ((devprop_list[i].flags == FLAGTRUE) || (devprop_list[i].flags & card->flags)) {
			if (devprop_list[i].get_value && devprop_list[i].get_value(val, 0)) {
				devprop_add_value(card->device, devprop_list[i].name, val->data, val->size);
				free_val(val);
				
				if (devprop_list[i].all_ports) {
					for (pnum = 1; pnum < card->ports; pnum++) {
						if (devprop_list[i].get_value(val, pnum)) {
							devprop_list[i].name[1] = (CHAR8)(0x30 + pnum); // convert to ascii
							devprop_add_value(card->device, devprop_list[i].name, val->data, val->size);
							free_val(val);
						}
					}
					devprop_list[i].name[1] = 0x30; // write back our "@0," for a next possible card
				}
			} else {
				if (devprop_list[i].default_val.type != kNul) {
					devprop_add_value(card->device, devprop_list[i].name,
						devprop_list[i].default_val.type == kCst ?
						(UINT8 *)&(devprop_list[i].default_val.data) : devprop_list[i].default_val.data,
						devprop_list[i].default_val.size);
				}
				
				if (devprop_list[i].all_ports) {
					for (pnum = 1; pnum < card->ports; pnum++) {
						if (devprop_list[i].default_val.type != kNul) {
							devprop_list[i].name[1] = (CHAR8)(0x30 + pnum); // convert to ascii
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
	UINTN bufferLen = 0;
	CHAR16 FileName[64];
  UINT8*  buffer = 0;

	//if we are here then TRUE
//	if (!gSettings.LoadVBios)
//		return FALSE;
	
	UnicodeSPrint(FileName, 128, L"\\ROM\\%04x_%04x.rom", vendor_id, device_id);
  if (FileExists(OEMDir, FileName)){
    DBG("Found generic VBIOS ROM file (%04x_%04x.rom)\n", vendor_id, device_id);
    Status = egLoadFile(OEMDir, FileName, &buffer, &bufferLen);
  }
  if (EFI_ERROR(Status)) {
    UnicodeSPrint(FileName, 128, L"\\EFI\\CLOVER\\ROM\\%04x_%04x.rom", vendor_id, device_id);
    if (FileExists(SelfRootDir, FileName)){
      DBG("Found generic VBIOS ROM file (%04x_%04x.rom)\n", vendor_id, device_id);
      Status = egLoadFile(SelfRootDir, FileName, &buffer, &bufferLen);
    }
  }

	if (EFI_ERROR(Status) || (bufferLen == 0)){
 	    DBG("ATI ROM not found \n");
	    card->rom_size = 0;
		card->rom = 0;
		return FALSE;
  }
  DBG("Loaded ROM len=%d\n", bufferLen);
	card->rom_size = (UINT32)bufferLen;
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
	bufferLen = ((option_rom_header_t *)card->rom)->rom_size;
	card->rom_size = (UINT32)(bufferLen << 9);
	DBG("Calculated ROM len=%d\n", card->rom_size);
//	close(fd);
  FreePool(buffer);
	
	return TRUE;
}

void get_vram_size(void)
{
  //check card->vram_size in bytes!
	ati_chip_family_t chip_family = card->info->chip_family;
	
	card->vram_size = 128 << 20; //default 128Mb, this is minimum for OS
  if (gSettings.VRAM != 0) {
    card->vram_size = gSettings.VRAM;
    DBG("Set VRAM from config=%dMb\n", (INTN)RShiftU64(card->vram_size, 20));
//    WRITEREG32(card->mmio, RADEON_CONFIG_MEMSIZE, card->vram_size);
  } else {
    if (chip_family >= CHIP_FAMILY_CEDAR) {
      // size in MB on evergreen
      // XXX watch for overflow!!!
      card->vram_size = ((UINT64)REG32(card->mmio, R600_CONFIG_MEMSIZE)) << 20;
      DBG("Set VRAM for Cedar=%dMb\n", (INTN)RShiftU64(card->vram_size, 20));
    } else if (chip_family >= CHIP_FAMILY_R600) {
			card->vram_size = REG32(card->mmio, R600_CONFIG_MEMSIZE);
    } else {
      card->vram_size = REG32(card->mmio, RADEON_CONFIG_MEMSIZE);
      if (card->vram_size == 0) {
        card->vram_size = REG32(card->mmio, RADEON_CONFIG_APER_SIZE);
        //Slice - previously I successfully made Radeon9000 working
        //by writing this register
        WRITEREG32(card->mmio, RADEON_CONFIG_MEMSIZE, (UINT32)card->vram_size);
      }
    }
  }	
  gSettings.VRAM = card->vram_size; 
  DBG("ATI: get_vram_size returned 0x%x\n", card->vram_size);
}

BOOLEAN read_vbios(BOOLEAN from_pci)
{
	option_rom_header_t *rom_addr;
	
	if (from_pci)
	{
		rom_addr = (option_rom_header_t *)(UINTN)(pci_config_read32(card->pci_dev, PCI_EXPANSION_ROM_BASE) & ~0x7ff);
		DBG(" @0x%x\n", rom_addr);
	}
	else
		rom_addr = (option_rom_header_t *)(UINTN)0xc0000;
	
	if (!validate_rom(rom_addr, card->pci_dev)){
    DBG("There is no ROM @0x%x\n", rom_addr);
 //   gBS->Stall(3000000);
		return FALSE;
  }
	
	card->rom_size = (UINT32)(rom_addr->rom_size) << 9;
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
	ati_chip_family_t chip_family = card->info->chip_family;
	
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
		else {
			WRITEREG32(card->mmio, R600_ROM_CNTL, (rom_cntl | R600_SCK_OVERWRITE));
    }
		
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
  //    CHAR8 		*model;
  INTN  		NameLen = 0;
	UINTN		i, j;
	INTN		n_ports = 0;
  UINTN 		ExpansionRom = 0;
	
	card = AllocateZeroPool(sizeof(card_t));
	if (!card)
		return FALSE;
	
	card->pci_dev = pci_dev;
	
	for (i = 0; radeon_cards[i].device_id ; i++) {
    card->info = &radeon_cards[i];
		if (radeon_cards[i].device_id == pci_dev->device_id) break;
	}
  
  for (j = 0; j < NGFX; j++) {
    if ((gGraphics[j].Vendor == Ati) &&
        (gGraphics[j].DeviceID == pci_dev->device_id)) {
      //      model = gGraphics[j].Model;
      n_ports = gGraphics[j].Ports;
      add_vbios = gGraphics[j].LoadVBios;
      break;
    }
  }
	
	if (!card->info || !card->info->device_id || !card->info->cfg_name) {
		DBG("Unsupported ATI card! Device ID: [%04x:%04x] Subsystem ID: [%08x] \n",
				pci_dev->vendor_id, pci_dev->device_id, pci_dev->subsys_id);
    DBG("search for brothers family\n");
    for (i = 0; radeon_cards[i].device_id ; i++) {
      if ((radeon_cards[i].device_id & ~0xf) == (pci_dev->device_id & ~0xf)) {
        card->info = &radeon_cards[i];
        break;
      }
    }
    if (!card->info->cfg_name) {
      DBG("...compatible config is not found\n");
      return FALSE;
    }
	}
	
	card->fb    = (UINT8 *)(UINTN)(pci_config_read32(pci_dev, PCI_BASE_ADDRESS_0) & ~0x0f);
	card->mmio	= (UINT8 *)(UINTN)(pci_config_read32(pci_dev, PCI_BASE_ADDRESS_2) & ~0x0f);
	card->io    = (UINT8 *)(UINTN)(pci_config_read32(pci_dev, PCI_BASE_ADDRESS_4) & ~0x03);
  pci_dev->regs = card->mmio;
	ExpansionRom = pci_config_read32(pci_dev, PCI_EXPANSION_ROM_BASE); //0x30 as Chimera
	DBG("Framebuffer @0x%08X  MMIO @0x%08X	I/O Port @0x%08X ROM Addr @0x%08X\n",
      card->fb, card->mmio, card->io, ExpansionRom);
	
	card->posted = radeon_card_posted();
	DBG("ATI card %a, ", card->posted ? "POSTed" : "non-POSTed");
	DBG("\n");
	get_vram_size();
	
	if (add_vbios) {
    load_vbios_file(pci_dev->vendor_id, pci_dev->device_id);
		if (!card->rom) {
			DBG("reading VBIOS from %a", card->posted ? "legacy space" : "PCI ROM");
			if (card->posted) // && ExpansionRom != 0)
				read_vbios(FALSE);
			else
				read_disabled_vbios();
			DBG("\n");
		} else {
      DBG("VideoBIOS read from file\n");
    }
    
  }
	
	if (card->info->chip_family >= CHIP_FAMILY_CEDAR) {
    DBG("ATI Radeon EVERGREEN family\n");
		card->flags |= EVERGREEN;
	}
  
  if (card->info->chip_family <= CHIP_FAMILY_RV670) {
    DBG("ATI Radeon OLD family\n");
		card->flags |= FLAGOLD;
	}
  
  if (gMobile) {
    DBG("ATI Mobile Radeon\n");
    card->flags |= FLAGMOBILE;
  }
  
  card->flags |= FLAGNOTFAKE;
  
	NameLen = StrLen(gSettings.FBName);
  if (NameLen > 2) {  //fool proof: cfg_name is 3 character or more.
    CfgName = AllocateZeroPool(NameLen);
    UnicodeStrToAsciiStr((CHAR16*)&gSettings.FBName[0], CfgName);
    DBG("Users config name %a\n", CfgName);
    card->cfg_name = CfgName;
  } else {
    // use cfg_name on radeon_cards, to retrive the default name from card_configs,
		card->cfg_name = card_configs[card->info->cfg_name].name;
		n_ports = card_configs[card->info->cfg_name].ports;
		// which means one of the fb's or kNull
		DBG("Framebuffer set to device's default: %a\n", card->cfg_name);
    DBG(" N ports defaults to %d\n", n_ports);
  }
  
  if (gSettings.VideoPorts != 0) {
    n_ports = gSettings.VideoPorts;
    DBG(" use N ports setting from config.plist: %d\n", n_ports);
  }
  
	if (n_ports > 0) {
		card->ports = (UINT8)n_ports; // use it.
		DBG("(AtiPorts) Nr of ports set to: %d\n", card->ports);
  } else {
    // if (card->cfg_name > 0) // do we want 0 ports if fb is kNull or mistyped ?
    
		// else, match cfg_name with card_configs list and retrive default nr of ports.
		for (i = 0; i < kCfgEnd; i++) {
			if (AsciiStrCmp(card->cfg_name, card_configs[i].name) == 0) {
				card->ports = card_configs[i].ports; // default
      }
    }
		
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
  aty_name.size = (UINT32)AsciiStrLen(name);
  aty_name.data = (UINT8 *)name;
	
  name_parent = AllocateZeroPool(24);
  AsciiSPrint(name_parent, 24, "ATY,%aParent", card->cfg_name);
  aty_nameparent.type = kStr;
  aty_nameparent.size = (UINT32)AsciiStrLen(name_parent);
  aty_nameparent.data = (UINT8 *)name_parent;
  //how can we free pool when we leave the procedure? Make all pointers global?
  return TRUE;
}

BOOLEAN setup_ati_devprop(LOADER_ENTRY *Entry, pci_dt_t *ati_dev)
{
  CHAR8 compatible[64];
  CHAR8 *devicepath;
  UINT32 FakeID = 0;
  INT32 i;

	if (!init_card(ati_dev))
		return FALSE;

  CurrentPatches = Entry->KernelAndKextPatches;
	
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
  
  if (gSettings.FakeATI) {
    card->flags &= ~FLAGNOTFAKE;
    card->flags |= FLAGOLD;
  }
  
	if (!gSettings.NoDefaultProperties) {
    devprop_add_list(ati_devprop_list);
    if (gSettings.UseIntelHDMI) {
      devprop_add_value(card->device, "hda-gfx", (UINT8*)"onboard-2", 10);
    } else {
      devprop_add_value(card->device, "hda-gfx", (UINT8*)"onboard-1", 10);
    }

  } else {
    DBG("ATI: No default properties injected\n");
  }
  
  if (gSettings.FakeATI) {
    FakeID = gSettings.FakeATI >> 16;
    devprop_add_value(card->device, "device-id", (UINT8*)&FakeID, 4);
    devprop_add_value(card->device, "ATY,DeviceID", (UINT8*)&FakeID, 2);
    AsciiSPrint(compatible, 64, "pci1002,%04x", FakeID);
    LowCase(compatible);
    devprop_add_value(card->device, "@0,compatible", (UINT8*)&compatible[0], 12);
    FakeID = gSettings.FakeATI & 0xFFFF;
    devprop_add_value(card->device, "vendor-id", (UINT8*)&FakeID, 4);
    devprop_add_value(card->device, "ATY,VendorID", (UINT8*)&FakeID, 2);
  }

  if (gSettings.NrAddProperties != 0xFFFE) {
    for (i = 0; i < gSettings.NrAddProperties; i++) {
      if (gSettings.AddProperties[i].Device != DEV_ATI) {
        continue;
      }
      devprop_add_value(card->device,
                        gSettings.AddProperties[i].Key,
                        (UINT8*)gSettings.AddProperties[i].Value,
                        gSettings.AddProperties[i].ValueLen);
    }
  }

	
	DBG("ATI %a %a %dMB (%a) [%04x:%04x] (subsys [%04x:%04x]):: %a\n",
			chip_family_name[card->info->chip_family], card->info->model_name,
			(INTN)RShiftU64(card->vram_size, 20), card->cfg_name,
			ati_dev->vendor_id, ati_dev->device_id,
			ati_dev->subsys_id.subsys.vendor_id, ati_dev->subsys_id.subsys.device_id,
			devicepath);
	
//  FreePool(card->info); //TODO we can't free constant so this info should be copy of constants
	FreePool(card);
	
	return TRUE;
}
