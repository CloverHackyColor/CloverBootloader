/*
 *  ati.h
 *  
 *  Created by Slice on 19.02.12.
 *  
 *  the code ported from Chameleon project as well as from RadeonFB by Joblo and RadeonHD by dong
 *  big thank to Islam M. Ahmed Zaid for the updating the collection
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


typedef struct {
	const CHAR8		*name;
	UINT8			ports;
} card_config_t;

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
#define FLAGMOBILE    MKFLAG(2)

//static UINT8 atN = 0;

typedef struct {
	type_t				type;
	UINT32				size;
	UINT8					*data;
} value_t;

typedef struct {
	UINT32				flags;
	BOOLEAN				all_ports;
	CHAR8					*name;
	BOOLEAN				(*get_value)(value_t *val);
	value_t				default_val;
} AtiDevProp;

BOOLEAN get_bootdisplay_val(value_t *val);
BOOLEAN get_vrammemory_val(value_t *val);
BOOLEAN get_edid_val(value_t *val);
BOOLEAN get_name_val(value_t *val);
BOOLEAN get_nameparent_val(value_t *val);
BOOLEAN get_model_val(value_t *val);
BOOLEAN get_conntype_val(value_t *val);
BOOLEAN get_vrammemsize_val(value_t *val);
BOOLEAN get_binimage_val(value_t *val);
BOOLEAN get_binimage_owr(value_t *val);
BOOLEAN get_romrevision_val(value_t *val);
BOOLEAN get_deviceid_val(value_t *val);
BOOLEAN get_mclk_val(value_t *val);
BOOLEAN get_sclk_val(value_t *val);
BOOLEAN get_refclk_val(value_t *val);
BOOLEAN get_platforminfo_val(value_t *val);
BOOLEAN get_vramtotalsize_val(value_t *val);

extern card_config_t card_configs[];
extern radeon_card_info_t radeon_cards[];
extern AtiDevProp ati_devprop_list[];
extern const CHAR8 *chip_family_name[];
