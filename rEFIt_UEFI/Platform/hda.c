/*
 * HDA injector, part of the Chameleon Boot Loader Project
 *
 * Ported and adapted by Fabio (ErmaC), October 2016.
 *
 * HDA injector is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HDA injector is distributed in the hope that it will be useful, but
 * WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Platform.h"
#include "hda.h"

#ifndef DEBUG_HDA
#ifndef DEBUG_ALL
#define DEBUG_HDA 1
#else
#define DEBUG_HDA DEBUG_ALL
#endif
#endif

#if DEBUG_HDA == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_HDA, __VA_ARGS__)
#endif

// HDA layout-id device injection by dmazar

#define HDA_VMIN  0x02 // Minor, Major Version
#define HDA_GCTL  0x08 // Global Control Register
#define HDA_ICO    0x60 // Immediate Command Output Interface
#define HDA_IRI    0x64 // Immediate Response Input Interface
#define HDA_ICS    0x68 // Immediate Command Status

/* Structures */

static hda_controller_devices know_hda_controller[] = {
  //8086  Intel Corporation
  { HDA_INTEL_OAK,  "Oaktrail"    /*, 0, 0 */ },
  { HDA_INTEL_BAY,  "BayTrail"    /*, 0, 0 */ },
  { HDA_INTEL_HSW1,  "Haswell"    /*, 0, 0 */ },
  { HDA_INTEL_HSW2,  "Haswell"    /*, 0, 0 */ },
  { HDA_INTEL_HSW3,  "Haswell"    /*, 0, 0 */ },
  { HDA_INTEL_BDW,  "Broadwell"    /*, 0, 0 */ },
  { HDA_INTEL_BROXTON_T,  "Broxton-T"  /*, 0, 0 */ },
  { HDA_INTEL_CPT,  "Cougar Point"    /*, 0, 0 */ },
  { HDA_INTEL_PATSBURG,  "Patsburg"    /*, 0, 0 */ },
  { HDA_INTEL_PPT1,  "Panther Point"    /*, 0, 0 */ },
  { HDA_INTEL_BRASWELL,  "Braswell"    /*, 0, 0 */ },
  { HDA_INTEL_82801F,  "82801F"    /*, 0, 0 */ },
  { HDA_INTEL_63XXESB,  "631x/632xESB"    /*, 0, 0 */ },
  { HDA_INTEL_82801G,  "82801G"    /*, 0, 0 */ },
  { HDA_INTEL_82801H,  "82801H"    /*, 0, 0 */ },
  { HDA_INTEL_82801I,  "82801I"    /*, 0, 0 */ },
  { HDA_INTEL_ICH9,  "ICH9"      /*, 0, 0 */ },
  { HDA_INTEL_82801JI,  "82801JI"    /*, 0, 0 */ },
  { HDA_INTEL_82801JD,  "82801JD"    /*, 0, 0 */ },
  { HDA_INTEL_PCH,  "5 Series/3400 Series"  /*, 0, 0 */ },
  { HDA_INTEL_PCH2,  "5 Series/3400 Series"  /*, 0, 0 */ },
  { HDA_INTEL_BROXTON_P,  "Apollolake"  /*, 0, 0 */ }, // Broxton-P
  { HDA_INTEL_SCH,  "SCH"      /*, 0, 0 */ },
  { HDA_INTEL_LPT1,  "Lynx Point"    /*, 0, 0 */ },
  { HDA_INTEL_LPT2,  "Lynx Point"    /*, 0, 0 */ },
  { HDA_INTEL_WCPT,  "Wildcat Point"    /*, 0, 0 */ },
  { HDA_INTEL_WELLS1,  "Wellsburg"    /*, 0, 0 */ },
  { HDA_INTEL_WELLS2,  "Wellsburg"    /*, 0, 0 */ },
  { HDA_INTEL_WCPTLP,  "Wildcat Point-LP"  /*, 0, 0 */ },
  { HDA_INTEL_LPTLP1,  "Lynx Point-LP"    /*, 0, 0 */ },
  { HDA_INTEL_LPTLP2,  "Lynx Point-LP"    /*, 0, 0 */ },
  { HDA_INTEL_SRSPLP,  "Sunrise Point-LP"  /*, 0, 0 */ },
  { HDA_INTEL_KABYLAKE_LP,  "Kabylake-LP"  /*, 0, 0 */ }, // Kabylake-LP
  { HDA_INTEL_SRSP,  "Sunrise Point"    /*, 0, 0 */ },
  { HDA_INTEL_KABYLAKE,  "Kabylake"  /*, 0, 0 */ }, // Kabylake
  { HDA_INTEL_LEWISBURG1,  "Lewisburg"  /*, 0, 0 */ }, // Lewisburg
  { HDA_INTEL_LEWISBURG2,  "Lewisburg"  /*, 0, 0 */ }, // Lewisburg
  { HDA_INTEL_UNPT,  "Union Point"    /*, 0, 0 */ }, // Kabylake-H
  
  //10de  NVIDIA Corporation
  { HDA_NVIDIA_MCP51,  "MCP51" /*, 0, HDAC_QUIRK_MSI */ },
  { HDA_NVIDIA_MCP55,  "MCP55" /*, 0, HDAC_QUIRK_MSI */ },
  { HDA_NVIDIA_MCP61_1,  "MCP61" /*, 0, 0 */ },
  { HDA_NVIDIA_MCP61_2,  "MCP61" /*, 0, 0 */ },
  { HDA_NVIDIA_MCP65_1,  "MCP65" /*, 0, 0 */ },
  { HDA_NVIDIA_MCP65_2,  "MCP65" /*, 0, 0 */ },
  { HDA_NVIDIA_MCP67_1,  "MCP67" /*, 0, 0 */ },
  { HDA_NVIDIA_MCP67_2,  "MCP67" /*, 0, 0 */ },
  { HDA_NVIDIA_MCP73_1,  "MCP73" /*, 0, 0 */ },
  { HDA_NVIDIA_MCP73_2,  "MCP73" /*, 0, 0 */ },
  { HDA_NVIDIA_MCP78_1,  "MCP78" /*, 0, HDAC_QUIRK_64BIT */ },
  { HDA_NVIDIA_MCP78_2,  "MCP78" /*, 0, HDAC_QUIRK_64BIT */ },
  { HDA_NVIDIA_MCP78_3,  "MCP78" /*, 0, HDAC_QUIRK_64BIT */ },
  { HDA_NVIDIA_MCP78_4,  "MCP78" /*, 0, HDAC_QUIRK_64BIT */ },
  { HDA_NVIDIA_MCP79_1,  "MCP79" /*, 0, 0 */ },
  { HDA_NVIDIA_MCP79_2,  "MCP79" /*, 0, 0 */ },
  { HDA_NVIDIA_MCP79_3,  "MCP79" /*, 0, 0 */ },
  { HDA_NVIDIA_MCP79_4,  "MCP79" /*, 0, 0 */ },
  { HDA_NVIDIA_MCP89_1,  "MCP89" /*, 0, 0 */ },
  { HDA_NVIDIA_MCP89_2,  "MCP89" /*, 0, 0 */ },
  { HDA_NVIDIA_MCP89_3,  "MCP89" /*, 0, 0 */ },
  { HDA_NVIDIA_MCP89_4,  "MCP89" /*, 0, 0 */ },
  { HDA_NVIDIA_0BE2,  "(0x0be2)" /*, 0, HDAC_QUIRK_MSI */ },
  { HDA_NVIDIA_0BE3,  "(0x0be3)" /*, 0, HDAC_QUIRK_MSI */ },
  { HDA_NVIDIA_0BE4,  "(0x0be4)" /*, 0, HDAC_QUIRK_MSI */ },
  { HDA_NVIDIA_GT100,  "GT100" /*, 0, HDAC_QUIRK_MSI */ },
  { HDA_NVIDIA_GT104,  "GT104" /*, 0, HDAC_QUIRK_MSI */ },
  { HDA_NVIDIA_GT106,  "GT106" /*, 0, HDAC_QUIRK_MSI */ },
  { HDA_NVIDIA_GT108,  "GT108" /*, 0, HDAC_QUIRK_MSI */ },
  { HDA_NVIDIA_GT116,  "GT116" /*, 0, HDAC_QUIRK_MSI */ },
  { HDA_NVIDIA_GF119,  "GF119" /*, 0, 0 */ },
  { HDA_NVIDIA_GF110_1,  "GF110" /*, 0, HDAC_QUIRK_MSI */ },
  { HDA_NVIDIA_GF110_2,  "GF110" /*, 0, HDAC_QUIRK_MSI */ },
  { HDA_NVIDIA_GK110,  "GK110" /*, 0, ? */ },
  { HDA_NVIDIA_GK106,  "GK106" /*, 0, ? */ },
  { HDA_NVIDIA_GK107,  "GK107" /*, 0, ? */ },
  { HDA_NVIDIA_GK104,  "GK104" /*, 0, ? */ },
  { HDA_NVIDIA_GP104_2, "Pascal GP104-200" /*, 0, ? */ },
  { HDA_NVIDIA_GM204_2, "Maxwell GP204-200" /*, 0, ? */ },
  
  //1002  Advanced Micro Devices [AMD] nee ATI Technologies Inc
  { HDA_ATI_SB450,  "SB4x0" /*, 0, 0 */ },
  { HDA_ATI_SB600,  "SB600" /*, 0, 0 */ },
  { HDA_ATI_RS600,  "RS600" /*, 0, 0 */ },
  { HDA_ATI_HUDSON,  "Hudson" /*, 0, 0 */ },
  { HDA_ATI_RS690,  "RS690" /*, 0, 0 */ },
  { HDA_ATI_RS780,  "RS780" /*, 0, 0 */ },
  { HDA_ATI_RS880,  "RS880" /*, 0, 0 */ },
  { HDA_ATI_TRINITY,  "Trinity" /*, 0, ? */ },
  { HDA_ATI_R600,    "R600" /*, 0, 0 */ },
  { HDA_ATI_RV610,  "RV610" /*, 0, 0 */ },
  { HDA_ATI_RV620,  "RV620" /*, 0, 0 */ },
  { HDA_ATI_RV630,  "RV630" /*, 0, 0 */ },
  { HDA_ATI_RV635,  "RV635" /*, 0, 0 */ },
  { HDA_ATI_RV710,  "RV710" /*, 0, 0 */ },
  { HDA_ATI_RV730,  "RV730" /*, 0, 0 */ },
  { HDA_ATI_RV740,  "RV740" /*, 0, 0 */ },
  { HDA_ATI_RV770,  "RV770" /*, 0, 0 */ },
  { HDA_ATI_RV810,  "RV810" /*, 0, 0 */ },
  { HDA_ATI_RV830,  "RV830" /*, 0, 0 */ },
  { HDA_ATI_RV840,  "RV840" /*, 0, 0 */ },
  { HDA_ATI_RV870,  "RV870" /*, 0, 0 */ },
  { HDA_ATI_RV910,  "RV910" /*, 0, 0 */ },
  { HDA_ATI_RV930,  "RV930" /*, 0, 0 */ },
  { HDA_ATI_RV940,  "RV940" /*, 0, 0 */ },
  { HDA_ATI_RV970,  "RV970" /*, 0, 0 */ },
  { HDA_ATI_R1000,  "R1000" /*, 0, 0 */ }, // HDMi
  { HDA_ATI_SI,    "SI" /*, 0, 0 */ },
  { HDA_ATI_VERDE,  "Cape Verde" /*, 0, ? */ }, // HDMi
  
  //17f3  RDC Semiconductor, Inc.
  { HDA_RDC_M3010,  "M3010" /*, 0, 0 */ },
  
  //1106  VIA Technologies, Inc.
  { HDA_VIA_VT82XX,  "VT8251/8237A" /*, 0, 0 */ },
  
  //1039  Silicon Integrated Systems [SiS]
  { HDA_SIS_966,    "966" /*, 0, 0 */ },
  
  //10b9  ULi Electronics Inc.(Split off ALi Corporation in 2003)
  { HDA_ULI_M5461,  "M5461" /*, 0, 0 */ },
  
  /* Unknown */
  { HDA_INTEL_ALL,  "Unknown Intel device" /*, 0, 0 */ },
  { HDA_NVIDIA_ALL,  "Unknown NVIDIA device" /*, 0, 0 */ },
  { HDA_ATI_ALL,    "Unknown ATI device" /*, 0, 0 */ },
  { HDA_VIA_ALL,    "Unknown VIA device" /*, 0, 0 */ },
  { HDA_SIS_ALL,    "Unknown SiS device" /*, 0, 0 */ },
  { HDA_ULI_ALL,    "Unknown ULI device" /*, 0, 0 */ },
};
#define HDAC_DEVICES_LEN (sizeof(know_hda_controller) / sizeof(know_hda_controller[0]))

/* CODECs */
/*
 * ErmaC: There's definitely a lot of different versions of the same audio codec variant out there...
 * in the next struct you will find a "generic" but IMHO detailed list of
 * possible codec... anyway to specific a new one or find difference beetween revision
 * check it under linux enviroment with:
 * $cat /proc/asound/Intel/codec#0
 * --------------------------------
 *  Codec: Analog Devices AD1989B
 *  Address: 0
 *  AFG Function Id: 0x1 (unsol 0)
 *  Vendor Id: 0x11d4989b
 *  Subsystem Id: 0x10438372
 *  Revision Id: 0x100300
 * --------------------------------
 * or
 * $cat /proc/asound/NVidia/codec#0
 * --------------------------------
 *  Codec: Nvidia GPU 14 HDMI/DP
 *  Address: 0
 *  AFG Function Id: 0x1 (unsol 0)
 *  Vendor Id: 0x10de0014
 *  Subsystem Id: 0x10de0101
 *  Revision Id: 0x100100
 * --------------------------------
 */

static hdacc_codecs know_codecs[] = {
  { HDA_CODEC_CS4206, 0,    "CS4206" },
  { HDA_CODEC_CS4207, 0,    "CS4207" },
  { HDA_CODEC_CS4208, 0,    "CS4208" },
  { HDA_CODEC_CS4210, 0,    "CS4210" },
  { HDA_CODEC_CS4213, 0,          "CS4213" },
  
  { HDA_CODEC_ALC221, 0,          "ALC221" },
  { HDA_CODEC_ALC231, 0,          "ALC231" },
  { HDA_CODEC_ALC233, 0,          "ALC233" },
  { HDA_CODEC_ALC233, 0x0003,  "ALC3236" },
  { HDA_CODEC_ALC235, 0,          "ALC235" },
  { HDA_CODEC_ALC255, 0,          "ALC255" },
  { HDA_CODEC_ALC256, 0,          "ALC256" },
  { HDA_CODEC_ALC260, 0,          "ALC260" },
  //  { HDA_CODEC_ALC262, 0x0100,  "ALC262" }, // Revision Id: 0x100100
  { HDA_CODEC_ALC262, 0,          "ALC262" },
  { HDA_CODEC_ALC267, 0,          "ALC267" },
  { HDA_CODEC_ALC268, 0,          "ALC268" },
  { HDA_CODEC_ALC269, 0,          "ALC269" },
  { HDA_CODEC_ALC270, 0,          "ALC270" },
  { HDA_CODEC_ALC272, 0,          "ALC272" },
  { HDA_CODEC_ALC273, 0,          "ALC273" },
  { HDA_CODEC_ALC275, 0,          "ALC275" },
  { HDA_CODEC_ALC276, 0,          "ALC276" },
  { HDA_CODEC_ALC280, 0,          "ALC280" },
  { HDA_CODEC_ALC282, 0,          "ALC282" },
  { HDA_CODEC_ALC283, 0,          "ALC283" },
  { HDA_CODEC_ALC284, 0,          "ALC284" },
  { HDA_CODEC_ALC285, 0,          "ALC285" },
  { HDA_CODEC_ALC286, 0,          "ALC286" },
  { HDA_CODEC_ALC288, 0,          "ALC288" },
  { HDA_CODEC_ALC290, 0,          "ALC290" },
  { HDA_CODEC_ALC292, 0,          "ALC292" },
  { HDA_CODEC_ALC292, 0x0001,     "ALC3232" },
  { HDA_CODEC_ALC293, 0,          "ALC293" },
  { HDA_CODEC_ALC298, 0,          "ALC298" },
  { HDA_CODEC_ALC660, 0,          "ALC660-VD" },
  { HDA_CODEC_ALC662, 0,          "ALC662" },
  { HDA_CODEC_ALC662, 0x0101,  "ALC662 rev1" },
  { HDA_CODEC_ALC662, 0x0002,  "ALC662 rev2" },
  { HDA_CODEC_ALC662, 0x0300,  "ALC662 rev3" },
  { HDA_CODEC_ALC663, 0,          "ALC663" },
  { HDA_CODEC_ALC665, 0,          "ALC665" },
  { HDA_CODEC_ALC667, 0,          "ALC667" },
  { HDA_CODEC_ALC668, 0,          "ALC668" },
  { HDA_CODEC_ALC670, 0,          "ALC670" },
  { HDA_CODEC_ALC671, 0,          "ALC671" },
  { HDA_CODEC_ALC680, 0,          "ALC680" },
  { HDA_CODEC_ALC861, 0x0340,  "ALC660" },
  { HDA_CODEC_ALC861, 0,          "ALC861" },
  { HDA_CODEC_ALC861VD, 0,        "ALC861-VD" },
  { HDA_CODEC_ALC867, 0,          "ALC891" },
  //  { HDA_CODEC_ALC880, 0x0800,  "ALC880" }, // Revision Id: 0x100800
  { HDA_CODEC_ALC880, 0,          "ALC880" },
  { HDA_CODEC_ALC882, 0,          "ALC882" },
  { HDA_CODEC_ALC883, 0,          "ALC883" },
  { HDA_CODEC_ALC885, 0x0101,  "ALC889A" }, // Revision Id: 0x100101
  { HDA_CODEC_ALC885, 0x0103,  "ALC889A" }, // Revision Id: 0x100103
  { HDA_CODEC_ALC885, 0,          "ALC885" },
  { HDA_CODEC_ALC886, 0,          "ALC886" },
  { HDA_CODEC_ALC887, 0,          "ALC887" },
  { HDA_CODEC_ALC888, 0x0101,  "ALC1200" }, // Revision Id: 0x100101
  { HDA_CODEC_ALC888, 0,          "ALC888" },
  { HDA_CODEC_ALC889, 0,          "ALC889" },
  { HDA_CODEC_ALC892, 0,          "ALC892" },
  { HDA_CODEC_ALC898, 0,          "ALC898" },
  //  { HDA_CODEC_ALC899, 0,    "ALC899" },
  { HDA_CODEC_ALC900, 0,          "ALC1150" },
  { HDA_CODEC_ALCS1220A, 0,    "ALCS1220A" },
  { HDA_CODEC_ALC1220, 0,    "ALC1220" },
  
  { HDA_CODEC_AD1882, 0,          "AD1882" },
  { HDA_CODEC_AD1882A, 0,         "AD1882A" },
  { HDA_CODEC_AD1883, 0,          "AD1883" },
  { HDA_CODEC_AD1884, 0,          "AD1884" },
  { HDA_CODEC_AD1884A, 0,         "AD1884A" },
  { HDA_CODEC_AD1981HD, 0,        "AD1981HD" },
  { HDA_CODEC_AD1983, 0,          "AD1983" },
  { HDA_CODEC_AD1984, 0,          "AD1984" },
  { HDA_CODEC_AD1984A, 0,         "AD1984A" },
  { HDA_CODEC_AD1984B, 0,         "AD1984B" },
  { HDA_CODEC_AD1986A, 0,         "AD1986A" },
  { HDA_CODEC_AD1987, 0,          "AD1987" },
  { HDA_CODEC_AD1988, 0,          "AD1988A" },
  { HDA_CODEC_AD1988B, 0,         "AD1988B" },
  { HDA_CODEC_AD1989A, 0,         "AD1989A" },
  { HDA_CODEC_AD1989B, 0x0200,  "AD2000B" }, // Revision Id: 0x100200
  { HDA_CODEC_AD1989B, 0x0300,  "AD2000B" }, // Revision Id: 0x100300
  { HDA_CODEC_AD1989B, 0,         "AD1989B" },
  
  { HDA_CODEC_XFIEA, 0,           "X-Fi Extreme A" },
  { HDA_CODEC_XFIED, 0,           "X-Fi Extreme D" },
  { HDA_CODEC_CA0132, 0,          "CA0132" },
  { HDA_CODEC_SB0880, 0,          "SB0880 X-Fi" },
  { HDA_CODEC_CMI9880, 0,         "CMI9880" },
  { HDA_CODEC_CMI98802, 0,        "CMI9880" },
  
  { HDA_CODEC_CXD9872RDK, 0,      "CXD9872RD/K" },
  { HDA_CODEC_CXD9872AKD, 0,      "CXD9872AKD" },
  { HDA_CODEC_STAC9200D, 0,       "STAC9200D" },
  { HDA_CODEC_STAC9204X, 0,       "STAC9204X" },
  { HDA_CODEC_STAC9204D, 0,       "STAC9204D" },
  { HDA_CODEC_STAC9205X, 0,       "STAC9205X" },
  { HDA_CODEC_STAC9205D, 0,       "STAC9205D" },
  { HDA_CODEC_STAC9220, 0,        "STAC9220" },
  { HDA_CODEC_STAC9220_A1, 0,     "STAC9220_A1" },
  { HDA_CODEC_STAC9220_A2, 0,     "STAC9220_A2" },
  { HDA_CODEC_STAC9221, 0,        "STAC9221" },
  { HDA_CODEC_STAC9221_A2, 0,     "STAC9221_A2" },
  { HDA_CODEC_STAC9221D, 0,       "STAC9221D" },
  { HDA_CODEC_STAC922XD, 0,       "STAC9220D/9223D" },
  { HDA_CODEC_STAC9227X, 0,       "STAC9227X" },
  { HDA_CODEC_STAC9227D, 0,       "STAC9227D" },
  { HDA_CODEC_STAC9228X, 0,       "STAC9228X" },
  { HDA_CODEC_STAC9228D, 0,       "STAC9228D" },
  { HDA_CODEC_STAC9229X, 0,       "STAC9229X" },
  { HDA_CODEC_STAC9229D, 0,       "STAC9229D" },
  { HDA_CODEC_STAC9230X, 0,       "STAC9230X" },
  { HDA_CODEC_STAC9230D, 0,       "STAC9230D" },
  { HDA_CODEC_STAC9250, 0,        "STAC9250" },
  { HDA_CODEC_STAC9250D, 0,  "STAC9250D" },
  { HDA_CODEC_STAC9251, 0,        "STAC9251" },
  { HDA_CODEC_STAC9250D_1, 0,  "STAC9250D" },
  { HDA_CODEC_STAC9255, 0,        "STAC9255" },
  { HDA_CODEC_STAC9255D, 0,       "STAC9255D" },
  { HDA_CODEC_STAC9254, 0,        "STAC9254" },
  { HDA_CODEC_STAC9254D, 0,       "STAC9254D" },
  { HDA_CODEC_STAC9271X, 0,       "STAC9271X" },
  { HDA_CODEC_STAC9271D, 0,       "STAC9271D" },
  { HDA_CODEC_STAC9272X, 0,       "STAC9272X" },
  { HDA_CODEC_STAC9272D, 0,       "STAC9272D" },
  { HDA_CODEC_STAC9273X, 0,       "STAC9273X" },
  { HDA_CODEC_STAC9273D, 0,       "STAC9273D" },
  { HDA_CODEC_STAC9274, 0,        "STAC9274" },
  { HDA_CODEC_STAC9274D, 0,       "STAC9274D" },
  { HDA_CODEC_STAC9274X5NH, 0,    "STAC9274X5NH" },
  { HDA_CODEC_STAC9274D5NH, 0,    "STAC9274D5NH" },
  { HDA_CODEC_STAC9202, 0,  "STAC9202" },
  { HDA_CODEC_STAC9202D, 0,  "STAC9202D" },
  { HDA_CODEC_STAC9872AK, 0,      "STAC9872AK" },
  
  { HDA_CODEC_IDT92HD005, 0,      "92HD005" },
  { HDA_CODEC_IDT92HD005D, 0,     "92HD005D" },
  { HDA_CODEC_IDT92HD206X, 0,     "92HD206X" },
  { HDA_CODEC_IDT92HD206D, 0,     "92HD206D" },
  { HDA_CODEC_IDT92HD66B1X5, 0,   "92HD66B1X5" },
  { HDA_CODEC_IDT92HD66B2X5, 0,   "92HD66B2X5" },
  { HDA_CODEC_IDT92HD66B3X5, 0,   "92HD66B3X5" },
  { HDA_CODEC_IDT92HD66C1X5, 0,   "92HD66C1X5" },
  { HDA_CODEC_IDT92HD66C2X5, 0,   "92HD66C2X5" },
  { HDA_CODEC_IDT92HD66C3X5, 0,   "92HD66C3X5" },
  { HDA_CODEC_IDT92HD66B1X3, 0,   "92HD66B1X3" },
  { HDA_CODEC_IDT92HD66B2X3, 0,   "92HD66B2X3" },
  { HDA_CODEC_IDT92HD66B3X3, 0,   "92HD66B3X3" },
  { HDA_CODEC_IDT92HD66C1X3, 0,   "92HD66C1X3" },
  { HDA_CODEC_IDT92HD66C2X3, 0,   "92HD66C2X3" },
  { HDA_CODEC_IDT92HD66C3_65, 0,  "92HD66C3_65" },
  { HDA_CODEC_IDT92HD700X, 0,     "92HD700X" },
  { HDA_CODEC_IDT92HD700D, 0,     "92HD700D" },
  { HDA_CODEC_IDT92HD71B5, 0,     "92HD71B5" },
  { HDA_CODEC_IDT92HD71B5_2, 0,   "92HD71B5" },
  { HDA_CODEC_IDT92HD71B6, 0,     "92HD71B6" },
  { HDA_CODEC_IDT92HD71B6_2, 0,   "92HD71B6" },
  { HDA_CODEC_IDT92HD71B7, 0,     "92HD71B7" },
  { HDA_CODEC_IDT92HD71B7_2, 0,   "92HD71B7" },
  { HDA_CODEC_IDT92HD71B8, 0,     "92HD71B8" },
  { HDA_CODEC_IDT92HD71B8_2, 0,   "92HD71B8" },
  { HDA_CODEC_IDT92HD73C1, 0,     "92HD73C1" },
  { HDA_CODEC_IDT92HD73D1, 0,     "92HD73D1" },
  { HDA_CODEC_IDT92HD73E1, 0,     "92HD73E1" },
  { HDA_CODEC_IDT92HD95, 0,  "92HD95" },
  { HDA_CODEC_IDT92HD75B3, 0,     "92HD75B3" },
  { HDA_CODEC_IDT92HD88B3, 0,     "92HD88B3" },
  { HDA_CODEC_IDT92HD88B1, 0,     "92HD88B1" },
  { HDA_CODEC_IDT92HD88B2, 0,     "92HD88B2" },
  { HDA_CODEC_IDT92HD88B4, 0,     "92HD88B4" },
  { HDA_CODEC_IDT92HD75BX, 0,     "92HD75BX" },
  { HDA_CODEC_IDT92HD81B1C, 0,    "92HD81B1C" },
  { HDA_CODEC_IDT92HD81B1X, 0,    "92HD81B1X" },
  { HDA_CODEC_IDT92HD83C1C, 0,    "92HD83C1C" },
  { HDA_CODEC_IDT92HD83C1X, 0,    "92HD83C1X" },
  { HDA_CODEC_IDT92HD87B1_3, 0,   "92HD87B1/3" },
  { HDA_CODEC_IDT92HD87B2_4, 0,   "92HD87B2/4" },
  { HDA_CODEC_IDT92HD89C3, 0,     "92HD89C3" },
  { HDA_CODEC_IDT92HD89C2, 0,     "92HD89C2" },
  { HDA_CODEC_IDT92HD89C1, 0,     "92HD89C1" },
  { HDA_CODEC_IDT92HD89B3, 0,     "92HD89B3" },
  { HDA_CODEC_IDT92HD89B2, 0,     "92HD89B2" },
  { HDA_CODEC_IDT92HD89B1, 0,     "92HD89B1" },
  { HDA_CODEC_IDT92HD89E3, 0,     "92HD89E3" },
  { HDA_CODEC_IDT92HD89E2, 0,     "92HD89E2" },
  { HDA_CODEC_IDT92HD89E1, 0,     "92HD89E1" },
  { HDA_CODEC_IDT92HD89D3, 0,     "92HD89D3" },
  { HDA_CODEC_IDT92HD89D2, 0,     "92HD89D2" },
  { HDA_CODEC_IDT92HD89D1, 0,     "92HD89D1" },
  { HDA_CODEC_IDT92HD89F3, 0,     "92HD89F3" },
  { HDA_CODEC_IDT92HD89F2, 0,     "92HD89F2" },
  { HDA_CODEC_IDT92HD89F1, 0,     "92HD89F1" },
  { HDA_CODEC_IDT92HD90BXX, 0,    "92HD90BXX" },
  { HDA_CODEC_IDT92HD91BXX, 0,    "92HD91BXX" },
  { HDA_CODEC_IDT92HD93BXX, 0,    "92HD93BXX" },
  { HDA_CODEC_IDT92HD98BXX, 0,    "92HD98BXX" },
  { HDA_CODEC_IDT92HD99BXX, 0,    "92HD99BXX" },
  
  { HDA_CODEC_CX20549, 0,         "CX20549 (Venice)" },
  { HDA_CODEC_CX20551, 0,         "CX20551 (Waikiki)" },
  { HDA_CODEC_CX20561, 0,         "CX20561 (Hermosa)" },
  { HDA_CODEC_CX20582, 0,         "CX20582 (Pebble)" },
  { HDA_CODEC_CX20583, 0,         "CX20583 (Pebble HSF)" },
  { HDA_CODEC_CX20584, 0,         "CX20584" },
  { HDA_CODEC_CX20585, 0,         "CX20585" },
  { HDA_CODEC_CX20588, 0,         "CX20588" },
  { HDA_CODEC_CX20590, 0,         "CX20590" },
  { HDA_CODEC_CX20631, 0,         "CX20631" },
  { HDA_CODEC_CX20632, 0,         "CX20632" },
  { HDA_CODEC_CX20641, 0,         "CX20641" },
  { HDA_CODEC_CX20642, 0,         "CX20642" },
  { HDA_CODEC_CX20651, 0,         "CX20651" },
  { HDA_CODEC_CX20652, 0,         "CX20652" },
  { HDA_CODEC_CX20664, 0,         "CX20664" },
  { HDA_CODEC_CX20665, 0,         "CX20665" },
  { HDA_CODEC_CX20751, 0,    "CX20751/2" },
  { HDA_CODEC_CX20751_2, 0,  "CX20751/2" },
  { HDA_CODEC_CX20751_4, 0,  "CX20753/4" },
  { HDA_CODEC_CX20755, 0,         "CX20755" },
  { HDA_CODEC_CX20756, 0,         "CX20756" },
  { HDA_CODEC_CX20757, 0,         "CX20757" },
  { HDA_CODEC_CX20952, 0,         "CX20952" },
  
  { HDA_CODEC_VT1708_8, 0,        "VT1708_8" },
  { HDA_CODEC_VT1708_9, 0,        "VT1708_9" },
  { HDA_CODEC_VT1708_A, 0,        "VT1708_A" },
  { HDA_CODEC_VT1708_B, 0,        "VT1708_B" },
  { HDA_CODEC_VT1709_0, 0,        "VT1709_0" },
  { HDA_CODEC_VT1709_1, 0,        "VT1709_1" },
  { HDA_CODEC_VT1709_2, 0,        "VT1709_2" },
  { HDA_CODEC_VT1709_3, 0,        "VT1709_3" },
  { HDA_CODEC_VT1709_4, 0,        "VT1709_4" },
  { HDA_CODEC_VT1709_5, 0,        "VT1709_5" },
  { HDA_CODEC_VT1709_6, 0,        "VT1709_6" },
  { HDA_CODEC_VT1709_7, 0,        "VT1709_7" },
  { HDA_CODEC_VT1708B_0, 0,       "VT1708B_0" },
  { HDA_CODEC_VT1708B_1, 0,       "VT1708B_1" },
  { HDA_CODEC_VT1708B_2, 0,       "VT1708B_2" },
  { HDA_CODEC_VT1708B_3, 0,       "VT1708B_3" },
  { HDA_CODEC_VT1708B_4, 0,       "VT1708B_4" },
  { HDA_CODEC_VT1708B_5, 0,       "VT1708B_5" },
  { HDA_CODEC_VT1708B_6, 0,       "VT1708B_6" },
  { HDA_CODEC_VT1708B_7, 0,       "VT1708B_7" },
  { HDA_CODEC_VT1708S_0, 0,       "VT1708S_0" },
  { HDA_CODEC_VT1708S_1, 0,       "VT1708S_1" },
  { HDA_CODEC_VT1708S_2, 0,       "VT1708S_2" },
  { HDA_CODEC_VT1708S_3, 0,       "VT1708S_3" },
  { HDA_CODEC_VT1708S_4, 0,       "VT1708S_4" },
  { HDA_CODEC_VT1708S_5, 0,       "VT1708S_5" },
  { HDA_CODEC_VT1708S_6, 0,       "VT1708S_6" },
  { HDA_CODEC_VT1708S_7, 0,       "VT1708S_7" },
  { HDA_CODEC_VT1702_0, 0,        "VT1702_0" },
  { HDA_CODEC_VT1702_1, 0,        "VT1702_1" },
  { HDA_CODEC_VT1702_2, 0,        "VT1702_2" },
  { HDA_CODEC_VT1702_3, 0,        "VT1702_3" },
  { HDA_CODEC_VT1702_4, 0,        "VT1702_4" },
  { HDA_CODEC_VT1702_5, 0,        "VT1702_5" },
  { HDA_CODEC_VT1702_6, 0,        "VT1702_6" },
  { HDA_CODEC_VT1702_7, 0,        "VT1702_7" },
  { HDA_CODEC_VT1716S_0, 0,       "VT1716S_0" },
  { HDA_CODEC_VT1716S_1, 0,       "VT1716S_1" },
  { HDA_CODEC_VT1718S_0, 0,       "VT1718S_0" },
  { HDA_CODEC_VT1718S_1, 0,       "VT1718S_1" },
  { HDA_CODEC_VT1802_0, 0,        "VT1802_0" },
  { HDA_CODEC_VT1802_1, 0,        "VT1802_1" },
  { HDA_CODEC_VT1812, 0,          "VT1812" },
  { HDA_CODEC_VT1818S, 0,         "VT1818S" },
  { HDA_CODEC_VT1828S, 0,         "VT1828S" },
  { HDA_CODEC_VT2002P_0, 0,       "VT2002P_0" },
  { HDA_CODEC_VT2002P_1, 0,       "VT2002P_1" },
  { HDA_CODEC_VT2020, 0,          "VT2020" },
  
  { HDA_CODEC_ATIRS600_1, 0,      "RS600" },
  { HDA_CODEC_ATIRS600_2, 0,      "RS600" },
  { HDA_CODEC_ATIRS690, 0,        "RS690/780" },
  { HDA_CODEC_ATIR6XX, 0,         "R6xx" },
  
  { HDA_CODEC_NVIDIAMCP67, 0,     "MCP67" },
  { HDA_CODEC_NVIDIAMCP73, 0,     "MCP73" },
  { HDA_CODEC_NVIDIAMCP78, 0,     "MCP78" },
  { HDA_CODEC_NVIDIAMCP78_2, 0,   "MCP78" },
  { HDA_CODEC_NVIDIAMCP78_3, 0,   "MCP78" },
  { HDA_CODEC_NVIDIAMCP78_4, 0,   "MCP78" },
  { HDA_CODEC_NVIDIAMCP7A, 0,     "MCP7A" },
  { HDA_CODEC_NVIDIAGT220, 0,     "GT220" },
  { HDA_CODEC_NVIDIAGT21X, 0,     "GT21x" },
  { HDA_CODEC_NVIDIAMCP89, 0,     "MCP89" },
  { HDA_CODEC_NVIDIAGT240, 0,     "GT240" },
  { HDA_CODEC_NVIDIAGTS450, 0,    "GTS450" },
  { HDA_CODEC_NVIDIAGT440, 0,     "GT440" }, // Revision Id: 0x100100
  { HDA_CODEC_NVIDIAGTX470, 0,     "GT470" },
  { HDA_CODEC_NVIDIAGTX550, 0,    "GTX550" },
  { HDA_CODEC_NVIDIAGTX570, 0,    "GTX570" },
  { HDA_CODEC_NVIDIAGT610, 0,  "GT610" },
  
  { HDA_CODEC_INTELIP, 0,         "Ibex Peak" },
  { HDA_CODEC_INTELBL, 0,         "Bearlake" },
  { HDA_CODEC_INTELCA, 0,         "Cantiga" },
  { HDA_CODEC_INTELEL, 0,         "Eaglelake" },
  { HDA_CODEC_INTELIP2, 0,        "Ibex Peak" },
  { HDA_CODEC_INTELCPT, 0,        "Cougar Point" },
  { HDA_CODEC_INTELPPT, 0,        "Panther Point" },
  { HDA_CODEC_INTELLLP, 0,        "Haswell" },
  { HDA_CODEC_INTELBRW, 0,        "Broadwell" },
  { HDA_CODEC_INTELSKL, 0,        "Skylake" },
  { HDA_CODEC_INTELBRO, 0,        "Broxton" },
  { HDA_CODEC_INTELKAB, 0,        "Kabylake" },
  { HDA_CODEC_INTELCDT, 0,        "CedarTrail" },
  { HDA_CODEC_INTELVLV, 0,        "Valleyview2" },
  { HDA_CODEC_INTELBSW, 0,        "Braswell" },
  { HDA_CODEC_INTELCL, 0,         "Crestline" },
  
  { HDA_CODEC_SII1390, 0,         "SiI1390 HDMi" },
  { HDA_CODEC_SII1392, 0,         "SiI1392 HDMi" },
  
  // Unknown CODECs
  { HDA_CODEC_ADXXXX, 0,          "Analog Devices" },
  { HDA_CODEC_AGEREXXXX, 0,       "LSI" },
  { HDA_CODEC_ALCXXXX, 0,         "Realtek" },
  { HDA_CODEC_ATIXXXX, 0,         "ATI" },
  { HDA_CODEC_CAXXXX, 0,          "Creative" },
  { HDA_CODEC_CMIXXXX, 0,         "CMedia" },
  { HDA_CODEC_CMIXXXX2, 0,        "CMedia" },
  { HDA_CODEC_CSXXXX, 0,          "Cirrus Logic" },
  { HDA_CODEC_CXXXXX, 0,          "Conexant" },
  { HDA_CODEC_CHXXXX, 0,          "Chrontel" },
  { HDA_CODEC_LGXXXX, 0,          "LG" },
  { HDA_CODEC_WMXXXX, 0,          "Wolfson Microelectronics" },
  { HDA_CODEC_QEMUXXXX, 0,        "QEMU" },
  { HDA_CODEC_IDTXXXX, 0,         "IDT" },
  { HDA_CODEC_INTELXXXX, 0,       "Intel" },
  { HDA_CODEC_MOTOXXXX, 0,        "Motorola" },
  { HDA_CODEC_NVIDIAXXXX, 0,      "NVIDIA" },
  { HDA_CODEC_SIIXXXX, 0,         "Silicon Image" },
  { HDA_CODEC_STACXXXX, 0,        "Sigmatel" },
  { HDA_CODEC_VTXXXX, 0,          "VIA" },
};
#define HDACC_CODECS_LEN (sizeof(know_codecs) / sizeof(know_codecs[0]))

/*****************
 * Device Methods
 *****************/

/* get HDA device name */
CHAR8 *get_hda_controller_name(UINT16 controller_device_id, UINT16 controller_vendor_id)
{
  static char desc[128];
  
  CHAR8 *name_format  = "Unknown HD Audio device %a";
  UINT32 controller_model = ((controller_device_id << 16) | controller_vendor_id);
  INT32 i;
  
  /* Get format for vendor ID */
  switch (controller_vendor_id)
  {
    case ATI_VENDORID:
      name_format = "ATI %a HDA Controller (HDMi)"; break;
      
    case INTEL_VENDORID:
      name_format = "Intel %a HDA Controller"; break;
      
    case NVIDIA_VENDORID:
      name_format = "Nvidia %a HDA Controller (HDMi)"; break;
      
    case RDC_VENDORID:
      name_format = "RDC %a HDA Controller"; break;
      
    case SIS_VENDORID:
      name_format = "SiS %a HDA Controller"; break;
      
    case ULI_VENDORID:
      name_format = "ULI %a HDA Controller"; break;
      
    case VIA_VENDORID:
      name_format = "VIA %a HDA Controller"; break;
      
    default:
      break;
  }
  
  for (i = 0; i < HDAC_DEVICES_LEN; i++)
  {
    if (know_hda_controller[i].model == controller_model)
    {
      AsciiSPrint(desc, sizeof(desc), name_format, know_hda_controller[i].desc);
      return desc;
    }
  }
  
  /* Not in table */
  AsciiSPrint(desc, sizeof(desc), "Unknown HDA device, vendor %04x, model %04x",
              controller_vendor_id, controller_device_id);
  return desc;
}

/* get Codec name */
CHAR8 *get_hda_codec_name(UINT16 codec_vendor_id, UINT16 codec_device_id, UINT8 codec_revision_id, UINT8 codec_stepping_id)
{
  static char desc[128];
  
  CHAR8    *lName_format  = NULL;
  UINT32    lCodec_model = ((UINT32)(codec_vendor_id) << 16) + (codec_device_id);
  UINT32    lCodec_rev = (((UINT16)(codec_revision_id) << 8) + codec_stepping_id);
  INT32    i;
  
  // Get format for vendor ID
  switch ( codec_vendor_id ) // UINT16
  {
    case ATI_VENDORID:
      lName_format = "ATI %a"; break;
      
    case CIRRUSLOGIC_VENDORID:
      lName_format = "Cirrus Logic %a"; break;
      
    case MOTO_VENDORID:
      lName_format = "Motorola %a"; break;
      
    case SII_VENDORID:
      lName_format = "Silicon Image %a"; break;
      
    case NVIDIA_VENDORID:
      lName_format = "Nvidia %a"; break;
      
    case REALTEK_VENDORID:
      lName_format = "Realtek %a"; break;
      
    case CREATIVE_VENDORID:
      lName_format = "Creative %a"; break;
      
    case VIA_VENDORID:
      lName_format = "VIA %a"; break;
      
    case IDT_VENDORID:
      lName_format = "IDT %a"; break;
      
    case AGERE_VENDORID:
      lName_format = "LSI %a"; break;
      
    case ANALOGDEVICES_VENDORID:
      lName_format = "Analog Devices %a"; break;
      
    case CMEDIA_VENDORID:
    case CMEDIA2_VENDORID:
      lName_format = "CMedia %a"; break;
      
    case CONEXANT_VENDORID:
      lName_format = "Conexant %s"; break;
      
    case CHRONTEL_VENDORID:
      lName_format = "Chrontel %a"; break;
      
    case LG_VENDORID:
      lName_format = "LG %a"; break;
      
    case WOLFSON_VENDORID:
      lName_format = "Wolfson Microelectronics %a"; break;
      
    case QEMU_VENDORID:
      lName_format = "QEMU %a"; break;
      
    case INTEL_VENDORID:
      lName_format = "Intel %a"; break;
      
    case SIGMATEL_VENDORID:
      lName_format = "Sigmatel %a"; break;
      
    default:
      lName_format = UNKNOWN; break;
      break;
  }
  
  for (i = 0; i < HDACC_CODECS_LEN; i++)
  {
    if ( know_codecs[i].id == lCodec_model )
    {
      if ( ( know_codecs[i].rev == 0x00000000 ) || ( know_codecs[i].rev == lCodec_rev ) )
      {
        AsciiSPrint(desc, sizeof(desc), lName_format, know_codecs[i].name);
        return desc;
      }
    }
  }
  
  if ( AsciiStrStr(lName_format, "%a" ) != NULL )
  {
    // Dirty way to remove '%a' from the end of the lName_format
    lName_format[AsciiStrLen(lName_format)-3] = '\0';
  }
  
  // Not in table
  AsciiSPrint(desc, sizeof(desc), "unknown %a Codec", lName_format);
  return desc;
}

// executing HDA verb command using Immediate Command Input and Output Registers
UINT32 HDA_IC_sendVerb(EFI_PCI_IO_PROTOCOL *PciIo, UINT32 codecAdr, UINT32 nodeId, UINT32 verb)
{
  EFI_STATUS  Status;
  UINT16    ics = 0;
  UINT32    data32 = 0;
  UINT64    data64 = 0;
  
  // about that polling below ...
  // spec says that delay is in 100ns units. value 1.000.000.0
  // should then be 1 second, but users of newer Aptio boards were reporting
  // delays of 10-20 secs when this value was used. maybe this polling timeout
  // value does not mean the same on all implementations?
  // anyway, delay is lowered now to 10.000.0 (10 millis).
  
  // poll ICS[0] to become 0
  Status = PciIo->PollMem(PciIo, EfiPciIoWidthUint16, 0/*bar*/, HDA_ICS/*offset*/, 0x1/*mask*/, 0/*value*/, 100000/*delay in 100ns*/, &data64);
  ics = (UINT16)(data64 & 0xFFFF);
  //DBG("poll ICS[0] == 0: Status=%r, ICS=%x, ICS[0]=%d\n", Status, ics, (ics & 0x0001));
  if (EFI_ERROR(Status)) return 0;
  // prepare and write verb to ICO
  data32 = codecAdr << 28 | ((nodeId & 0xFF)<<20) | (verb & 0xFFFFF);
  Status = PciIo->Mem.Write(PciIo, EfiPciIoWidthUint32, 0, HDA_ICO, 1, &data32);
  //DBG("ICO write verb Codec=%x, Node=%x, verb=%x, command verb=%x: Status=%r\n", codecAdr, nodeId, verb, data32, Status);
  if (EFI_ERROR(Status)) return 0;
  // write 11b to ICS[1:0] to send command
  ics |= 0x3;
  Status = PciIo->Mem.Write(PciIo, EfiPciIoWidthUint16, 0, HDA_ICS, 1, &ics);
  //DBG("ICS[1:0] = 11b: Status=%r\n", Status);
  if (EFI_ERROR(Status)) return 0;
  // poll ICS[1:0] to become 10b
  Status = PciIo->PollMem(PciIo, EfiPciIoWidthUint16, 0/*bar*/, HDA_ICS/*offset*/, 0x3/*mask*/, 0x2/*value*/, 100000/*delay in 100ns*/, &data64);
  //DBG("poll ICS[0] == 0: Status=%r\n", Status);
  if (EFI_ERROR(Status)) return 0;
  // read IRI for VendorId/DeviceId
  Status = PciIo->Mem.Read(PciIo, EfiPciIoWidthUint32, 0, HDA_IRI, 1, &data32);
  if (EFI_ERROR(Status)) return 0;
  return data32;
}

UINT32 HDA_getCodecVendorAndDeviceIds(EFI_PCI_IO_PROTOCOL *PciIo) {
  EFI_STATUS  Status;
  //UINT8    ver[2];
  UINT32    data32 = 0;
  
  // check HDA version - should be 1.0
  /*
   Status = PciIo->Mem.Read(PciIo, EfiPciIoWidthUint8, 0, HDA_VMIN, 2, &ver[0]);
   DBG("HDA Version: Status=%r, version=%d.%d\n", Status, ver[1], ver[0]);
   if (EFI_ERROR(Status)) {
   return 0;
   }
   */
  
  // check if controller is out of reset - GCTL-08h[CRST-bit 0] == 1
  Status = PciIo->Mem.Read(PciIo, EfiPciIoWidthUint32, 0, HDA_GCTL, 1, &data32);
  //DBG("check CRST == 1: Status=%r, CRST=%d\n", Status, (data32 & 0x1));
  if (EFI_ERROR(Status)) {
    return 0;
  }
  if ((data32 & 0x1) == 0) {
    // this controller is not inited yet - we can not read codec ids
    // if needed, we can init it by:
    // - set Control reset bit in Global Control reg 08h[0] to 1
    // - poll it to become 1 again
    // - wait at least 521 micro sec for codecs to init
    // - we can check STATESTS reg 0eh where each running codec will set one bit
    //   codec addr 0 bit 0, codec addr 1 bit 1 ...
    
    return 0;
  }
  //Slice - TODO check codecAdr=2 - it is my Dell 1525.
  // all ok - read Ids
  return HDA_IC_sendVerb(PciIo, 0/*codecAdr*/, 0/*nodeId*/, 0xF0000/*verb*/);
}

UINT32 getLayoutIdFromVendorAndDeviceId(UINT32 vendorDeviceId)
{
  UINT32  layoutId = 0;
  UINT8    hexDigit = 0;
  
  // extract device id - 2 lower bytes,
  // convert it to decimal like this: 0x0887 => 887 decimal
  hexDigit = vendorDeviceId & 0xF;
  if (hexDigit > 9) return 0;
  layoutId = hexDigit;
  
  vendorDeviceId = vendorDeviceId >> 4;
  hexDigit = vendorDeviceId & 0xF;
  if (hexDigit > 9) return 0;
  layoutId += hexDigit * 10;
  
  vendorDeviceId = vendorDeviceId >> 4;
  hexDigit = vendorDeviceId & 0xF;
  if (hexDigit > 9) return 0;
  layoutId += hexDigit * 100;
  
  vendorDeviceId = vendorDeviceId >> 4;
  hexDigit = vendorDeviceId & 0xF;
  if (hexDigit > 9) return 0;
  layoutId += hexDigit * 1000;
  
  return layoutId;
}

BOOLEAN IsHDMIAudio(EFI_HANDLE PciDevHandle)
{
  EFI_STATUS          Status;
  EFI_PCI_IO_PROTOCOL *PciIo;
  UINTN               Segment;
  UINTN               Bus;
  UINTN               Device;
  UINTN               Function;
  UINTN               Index;
  
  // get device PciIo protocol
  Status = gBS->OpenProtocol(PciDevHandle, &gEfiPciIoProtocolGuid, (VOID **)&PciIo, gImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
  if (EFI_ERROR(Status)) {
    return FALSE;
  }
  
  // get device location
  Status = PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
  if (EFI_ERROR(Status)) {
    return FALSE;
  }
  
  // iterate over all GFX devices and check for sibling
  for (Index = 0; Index < NGFX; Index++) {
    if (gGraphics[Index].Segment == Segment
        && gGraphics[Index].Bus == Bus
        && gGraphics[Index].Device == Device) {
      return TRUE;
    }
  }
  
  return FALSE;
}

BOOLEAN setup_hda_devprop(EFI_PCI_IO_PROTOCOL *PciIo, pci_dt_t *hda_dev, CHAR8 *OSVersion)
{
#if DEBUG_INJECT
  CHAR8           *devicepath;
#endif
  DevPropDevice           *device = NULL;
  UINT32                  layoutId = 0;
  UINT32                  codecId = 0;
  BOOLEAN                 Injected = FALSE;
  UINTN                   i;
  
  if (!string) {
    string = devprop_create_string();
  }
#if DEBUG_INJECT
  devicepath = get_pci_dev_path(hda_dev);
#endif
  if (hda_dev && !hda_dev->used) {
    device = devprop_add_device_pci(string, hda_dev, NULL);
    hda_dev->used = TRUE;
  }
  
  if (!device) {
    return FALSE;
  }
  
#if DEBUG_INJECT
  DBG("HDA Controller [%04x:%04x] :: %a =>", hda_dev->vendor_id, hda_dev->device_id, devicepath);
#endif
  
  if (IsHDMIAudio(hda_dev->DeviceHandle)) {
    if (!gSettings.HDMIInjection) {
      return FALSE;
    }
    
    if (hda_dev && !hda_dev->used) {
      device = devprop_add_device_pci(string, hda_dev, NULL);
      hda_dev->used = TRUE;
    }
    if (!device) {
      return FALSE;
    }
        
    if (gSettings.NrAddProperties != 0xFFFE) {
      for (i = 0; i < gSettings.NrAddProperties; i++) {
        if (gSettings.AddProperties[i].Device != DEV_HDMI) {
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
      DBG("Additional HDMI properties injected, continue\n");
      //    return TRUE;
    } else if (gSettings.UseIntelHDMI) {
      DBG(" HDMI Audio, setting hda-gfx=onboard-1\n");
      devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
    }
  } else {
    if (!gSettings.HDAInjection) {
      return FALSE;
    }
    if (hda_dev && !hda_dev->used) {
      device = devprop_add_device_pci(string, hda_dev, NULL);
      hda_dev->used = TRUE;
    }
    if (!device) {
      return FALSE;
    }
    // HDA - determine layout-id
    if (gSettings.HDALayoutId > 0) {
      // layoutId is specified - use it
      layoutId = (UINT32)gSettings.HDALayoutId;
      DBG(" setting specified layout-id=%d (0x%x)\n", layoutId, layoutId);
    } else {
      // use detection: layoutId=codec dviceId or use default 12
      codecId = HDA_getCodecVendorAndDeviceIds(PciIo);
      if (codecId != 0) {
        DBG(" detected codec: %04x:%04x\n", (codecId >> 16), (codecId & 0xFFFF));
        layoutId = getLayoutIdFromVendorAndDeviceId(codecId);
      } else {
        DBG(" codec not detected\n");
      }
      // if not detected - use 12 as default
      if (layoutId == 0) {
        layoutId = 12;
      }
      //     DBG(", setting layout-id=%d (0x%x)\n", layoutId, layoutId);
    }
    if (gSettings.NrAddProperties != 0xFFFE) {
      for (i = 0; i < gSettings.NrAddProperties; i++) {
        if (gSettings.AddProperties[i].Device != DEV_HDA) {
          continue;
        }
        Injected = TRUE;
        devprop_add_value(device,
                          gSettings.AddProperties[i].Key,
                          (UINT8*)gSettings.AddProperties[i].Value,
                          gSettings.AddProperties[i].ValueLen);
      }
    }
    if (!Injected) {
      if ((OSVersion != NULL && AsciiOSVersionToUint64(OSVersion) < AsciiOSVersionToUint64("10.8")) || (gSettings.HDALayoutId > 0)) {
        devprop_add_value(device, "layout-id", (UINT8 *)&layoutId, 4);
      }
      layoutId = 0; // reuse variable
      if (gSettings.UseIntelHDMI) {
        devprop_add_value(device, "hda-gfx", (UINT8 *)"onboard-1", 10);
      }
      codecId = 1; // reuse variable again
      if (gSettings.AFGLowPowerState) {
        devprop_add_value(device, "AFGLowPowerState", (UINT8 *)&codecId, 4);
      }
      
      devprop_add_value(device, "MaximumBootBeepVolume", (UINT8 *)&layoutId, 1);
      devprop_add_value(device, "PinConfigurations", (UINT8 *)&layoutId, 1);
    }
  }
  return TRUE;
}

