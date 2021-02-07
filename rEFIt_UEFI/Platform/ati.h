/*
 *  ati.h
 *
 *  Created by Slice on 19.02.12.
 *
 *  the code ported from Chameleon project as well as from RadeonFB by Joblo and RadeonHD by dong
 *  big thank to Islam M. Ahmed Zaid for the updating the collection
 */

#include "../include/Pci.h"
#include "device_inject.h"
class LOADER_ENTRY;

#define OFFSET_TO_GET_ATOMBIOS_STRINGS_START 0x6e
#define DATVAL(x)   {kPtr, sizeof(x), (UINT8 *)x}
#define STRVAL(x)   {kStr, sizeof(x)-1, (UINT8 *)x}
#define BYTVAL(x)   {kCst, 1, (UINT8 *)(UINTN)x}
#define WRDVAL(x)   {kCst, 2, (UINT8 *)(UINTN)x}
#define DWRVAL(x)   {kCst, 4, (UINT8 *)(UINTN)x}
// QWRVAL would work only in 64 bit
//#define QWRVAL(x)   {kCst, 8, (UINT8 *)(UINTN)x}
#define NULVAL    {kNul, 0, (UINT8 *)NULL}


/*Typedefs ENUMS*/
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
  CHIP_FAMILY_R423,
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
  CHIP_FAMILY_RV772,
  CHIP_FAMILY_RV790,
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
  /* Southern Islands */
  CHIP_FAMILY_PALM,
  CHIP_FAMILY_SUMO,
  CHIP_FAMILY_SUMO2,
  CHIP_FAMILY_ARUBA,
  CHIP_FAMILY_TAHITI,
  CHIP_FAMILY_PITCAIRN,
  CHIP_FAMILY_VERDE,
  CHIP_FAMILY_OLAND,
  /* Sea Islands */
  CHIP_FAMILY_HAINAN,
  CHIP_FAMILY_BONAIRE,
  CHIP_FAMILY_KAVERI,
  CHIP_FAMILY_KABINI,
  CHIP_FAMILY_HAWAII,
  CHIP_FAMILY_MULLINS,
  /* Volcanic Islands */
  CHIP_FAMILY_TOPAZ,
  CHIP_FAMILY_AMETHYST,
  CHIP_FAMILY_TONGA,
  CHIP_FAMILY_FIJI,
  CHIP_FAMILY_CARRIZO,
  CHIP_FAMILY_TOBAGO,
  CHIP_FAMILY_ELLESMERE, /* Polaris 10 */
  CHIP_FAMILY_BAFFIN,   /* Polaris 11 */
  CHIP_FAMILY_GREENLAND, /* Polaris 12 */
  CHIP_FAMILY_VEGA10,   /* Vega 10 */
  CHIP_FAMILY_VEGA20,   /* Vega 20 */
  CHIP_FAMILY_NAVI10,
  CHIP_FAMILY_LAST
} ati_chip_family_t;

typedef struct {
  const CHAR8  *name;
  UINT8   ports;
} card_config_t;

typedef enum {
  kNull,
  /* OLDController */
  kWormy,
  kAlopias,
  kCaretta,
  kKakapo,
  kKipunji,
  kPeregrine,
  kRaven,
  kSphyrna,
  /* AMD2400Controller */
  kIago,
  /* AMD2600Controller */
  kHypoprion,
  kLamna,
  /* AMD3800Controller */
  kMegalodon,
  kTriakis,
  /* AMD4600Controller */
  kFlicker,
  kGliff,
  kShrike,
  /* AMD4800Controller */
  kCardinal,
  kMotmot,
  kQuail,
  /* AMD5000Controller */
  kDouc,
  kLangur,
  kUakari,
  kZonalis,
  kAlouatta,
  kHoolock,
  kVervet,
  kBaboon,
  kEulemur,
  kGalago,
  kColobus,
  kMangabey,
  kNomascus,
  kOrangutan,
  /* AMD6000Controller */
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
  kMuskgrass,
  kJuncus,
  kOsmunda,
  kPondweed,
  kSpikerush,
  kTypha,
  /* AMD7000Controller */
  kRamen,
  kTako,
  kNamako,
  kAji,
  kBuri,
  kChutoro,
  kDashimaki,
  kEbi,
  kGari,
  kFutomaki,
  kHamachi,
  kOPM,
  kIkura,
  kIkuraS,
  kJunsai,
  kKani,
  kKaniS,
  kDashimakiS,
  kMaguro,
  kMaguroS,
  /* AMD8000Controller */
  kExmoor,
  kBaladi,
  /* AMD9000Controller */
  kMalteseS,
  kLagotto,
  kGreyhoundS,
  kMaltese,
  kBasset,
  kGreyhound,
  kLabrador,
  /* AMD9300Controller */
  kFleuveSWIP,
  /* AMD9500Controller */
  kAcre,
  kDayman,
  kGuariba,
  kHuallaga,
  kOrinoco,
  /* AMD9510Controller*/
  kBerbice,
  /* AMD9515Controller */
  kMazaruni,
  kLongavi,
  /* AMD9520Controller */
  kElqui,
  kCaroni,
  kFlorin,
  kRadeon, // this is absent FB if not set
  kCfgEnd
} config_name_t;

typedef struct {
  UINT16            device_id;
  // UINT32         subsys_id;
  ati_chip_family_t chip_family;
  const CHAR8       *model_name;
  config_name_t     cfg_name;
} radeon_card_info_t;

typedef struct {
  DevPropDevice       *device;
  radeon_card_info_t  *info;
  pci_dt_t            *pci_dev;
  UINT8               *fb;
  UINT8               *mmio;
  UINT8               *io;
  UINT8               *rom;
  UINT32              rom_size;
  UINT64              vram_size;
  const CHAR8         *cfg_name;
  UINT8               ports;
  UINT32              flags;
  BOOLEAN             posted;
} card_t;

// Chip flags
/* enum radeon_chip_flags {
 RADEON_FAMILY_MASK = 0x0000ffffUL,
 RADEON_FLAGS_MASK = 0xffff0000UL,
 RADEON_IS_MOBILITY = 0x00010000UL,
 RADEON_IS_IGP = 0x00020000UL,
 RADEON_SINGLE_CRTC = 0x00040000UL,
 RADEON_IS_AGP = 0x00080000UL,
 RADEON_HAS_HIERZ = 0x00100000UL,
 RADEON_IS_PCIE = 0x00200000UL,
 RADEON_NEW_MEMMAP = 0x00400000UL,
 RADEON_IS_PCI = 0x00800000UL,
 RADEON_IS_IGPGART = 0x01000000UL,
 };*/
#define MKFLAG(n)   (1 << n)
#define FLAGTRUE    MKFLAG(0)
#define EVERGREEN   MKFLAG(1)
#define FLAGMOBILE  MKFLAG(2)
#define FLAGOLD     MKFLAG(3)
#define FLAGNOTFAKE MKFLAG(4)
#define FLAGDYNAMIC MKFLAG(5)

typedef struct {
  type_t  type;
  UINT32  size;
  UINT8   *data;
} value_t;

typedef struct {
  UINT32    flags;
  BOOLEAN   all_ports;
  CONST CHAR8     *name;
  BOOLEAN   (*get_value)(value_t *val, INTN index, BOOLEAN Sier);
  value_t   default_val;
} AtiDevProp;

BOOLEAN get_bootdisplay_val(value_t *val, INTN index, BOOLEAN Sier);
BOOLEAN get_vrammemory_val(value_t *val, INTN index, BOOLEAN Sier);
BOOLEAN get_edid_val(value_t *val, INTN index, BOOLEAN Sier);
BOOLEAN get_display_type(value_t *val, INTN index, BOOLEAN Sier);
BOOLEAN get_name_val(value_t *val, INTN index, BOOLEAN Sier);
BOOLEAN get_nameparent_val(value_t *val, INTN index, BOOLEAN Sier);
BOOLEAN get_model_val(value_t *val, INTN index, BOOLEAN Sier);
BOOLEAN get_conntype_val(value_t *val, INTN index, BOOLEAN Sier);
BOOLEAN get_vrammemsize_val(value_t *val, INTN index, BOOLEAN Sier);
BOOLEAN get_binimage_val(value_t *val, INTN index, BOOLEAN Sier);
BOOLEAN get_binimage_owr(value_t *val, INTN index, BOOLEAN Sier);
BOOLEAN get_romrevision_val(value_t *val, INTN index, BOOLEAN Sier);
BOOLEAN get_deviceid_val(value_t *val, INTN index, BOOLEAN Sier);
BOOLEAN get_mclk_val(value_t *val, INTN index, BOOLEAN Sier);
BOOLEAN get_sclk_val(value_t *val, INTN index, BOOLEAN Sier);
BOOLEAN get_refclk_val(value_t *val, INTN index, BOOLEAN Sier);
BOOLEAN get_platforminfo_val(value_t *val, INTN index, BOOLEAN Sier);
BOOLEAN get_vramtotalsize_val(value_t *val, INTN index, BOOLEAN Sier);
BOOLEAN get_dual_link_val(value_t *val, INTN index, BOOLEAN Sier);
BOOLEAN get_name_pci_val(value_t *val, INTN index, BOOLEAN Sier);

extern card_config_t card_configs[];
extern radeon_card_info_t radeon_cards[];
extern AtiDevProp ati_devprop_list[];
extern const CHAR8 *chip_family_name[];


BOOLEAN
setup_ati_devprop (
  LOADER_ENTRY *Entry,
  pci_dt_t     *ati_dev
  );

