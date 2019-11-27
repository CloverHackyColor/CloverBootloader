/* @file HdaDevicesLib.c

*/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>
#include <Library/HdaModels.h>

// Names

//
// Controller models.
//
HDA_CONTROLLER_LIST_ENTRY gHdaControllerList[] = {
    //1002  Advanced Micro Devices [AMD] nee ATI Technologies Inc
    { HDA_CONTROLLER(AMD, 0x437b),  "AMD SB4x0 HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0x4383),  "AMD SB600 HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0x780d),  "AMD Hudson HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0x7919),  "AMD RS690 HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0x793b),  "AMD RS600 HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0x960f),  "AMD RS780 HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0x970f),  "AMD RS880 HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0x9902),  "AMD Trinity HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0xaa00),  "AMD R600 HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0xaa08),  "AMD RV630 HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0xaa10),  "AMD RV610 HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0xaa18),  "AMD RV670/680 HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0xaa20),  "AMD RV635 HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0xaa28),  "AMD RV620 HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0xaa30),  "AMD RV770 HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0xaa38),  "AMD RV730 HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0xaa40),  "AMD RV710 HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0xaa48),  "AMD RV740 HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0xaa50),  "AMD RV870 HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0xaa58),  "AMD RV840 HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0xaa60),  "AMD RV830 HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0xaa68),  "AMD RV810 HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0xaa80),  "AMD RV970 HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0xaa88),  "AMD RV940 HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0xaa90),  "AMD RV930 HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0xaa98),  "AMD RV910 HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0xaaa0),  "AMD R1000 HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0xaaa8),  "AMD SI HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0xaab0),  "AMD Cape Verde HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0xaae0),  "AMD Buffin HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0xaaf0),  "AMD Ellesmere HD Audio Controller" },
    { HDA_CONTROLLER(AMD, 0xffff),  "AMD HD Audio Controller" },

    //8086  Intel Corporation
    { HDA_CONTROLLER(INTEL, 0x080a),  "Intel Oaktrail HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x0a0c),  "Intel Haswell HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x0c0c),  "Intel Ivy Bridge/Haswell HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x0d0c),  "Intel Crystal Well HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x0f04),  "Intel BayTrail HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x160c),  "Intel Broadwell HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x1a98),  "Intel Broxton-T HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x1c20),  "Intel 6 Series HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x1d20),  "Intel X79/C600 Series HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x1e20),  "Intel 7 Series HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x2284),  "Intel Braswell HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x2668),  "Intel ICH6 HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x269a),  "Intel 63XXESB HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x27d8),  "Intel ICH7 HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x284b),  "Intel ICH8 HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x293e),  "Intel ICH9 HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x293f),  "Intel ICH9 HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x3a3e),  "Intel ICH10 HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x3a6e),  "Intel ICH10 HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x3b56),  "Intel 5 Series/3400 Series HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x3b57),  "Intel 5 Series/3400 Series HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x5a98),  "Intel Apollolake HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x811b),  "Intel Poulsbo HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x8c20),  "Intel 8 Series HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x8c21),  "Intel 8 Series HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x8ca0),  "Intel 9 Series HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x8d20),  "Intel X99/C610 Series HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x8d21),  "Intel X99/C610 Series HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x9c20),  "Intel 8 Series HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x9c21),  "Intel 8 Series HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x9ca0),  "Intel 9 Series HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x9d70),  "Intel Sunrise Point-LP HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0x9d71),  "Intel Kabylake-LP HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0xa170),  "Intel 100 Series/C230 Series HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0xa171),  "Intel CM238 HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0xa1f0),  "Intel Lewisburg HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0xa270),  "Intel Lewisburg HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0xa2f0),  "Intel 200 Series HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0xa348),  "Intel 300 Series HD Audio Controller" },
    { HDA_CONTROLLER(INTEL, 0xffff),  "Intel HD Audio Controller" },

    //10de  NVIDIA Corporation
    { HDA_CONTROLLER(NVIDIA, 0x026c),  "Nvidia MCP51 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0371),  "Nvidia MCP55 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x03e4),  "Nvidia MCP61 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x03f0),  "Nvidia MCP61 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x044a),  "Nvidia MCP65 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x044b),  "Nvidia MCP65 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x055c),  "Nvidia MCP67 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x055d),  "Nvidia MCP67 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0774),  "Nvidia MCP78 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0775),  "Nvidia MCP78 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0776),  "Nvidia MCP78 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0777),  "Nvidia MCP78 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x07fc),  "Nvidia MCP73 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x07fd),  "Nvidia MCP73 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0ac0),  "Nvidia MCP79 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0ac1),  "Nvidia MCP79 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0ac2),  "Nvidia MCP79 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0ac3),  "Nvidia MCP79 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0be2),  "Nvidia GT216 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0be3),  "Nvidia GT218 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0be4),  "Nvidia GT215 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0be5),  "Nvidia GF100 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0be9),  "Nvidia GF106 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0bea),  "Nvidia GF108 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0beb),  "Nvidia GF104 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0bee),  "Nvidia GF116 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0d94),  "Nvidia MCP89 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0d95),  "Nvidia MCP89 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0d96),  "Nvidia MCP89 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0d97),  "Nvidia MCP89 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0e08),  "Nvidia GF119 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0e09),  "Nvidia GF110 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0e0a),  "Nvidia GK104 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0e0b),  "Nvidia GK106 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0e0c),  "Nvidia GF114 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0e0f),  "Nvidia GK208 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0e1a),  "Nvidia GK110 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0e1b),  "Nvidia GK107 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0fb0),  "Nvidia GM200 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0fb8),  "Nvidia GP108 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0fb9),  "Nvidia GP107GL HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0fba),  "Nvidia GM206 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0fbb),  "Nvidia GM204 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x0fbc),  "Nvidia GM107 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x10ef),  "Nvidia GP102 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x10f0),  "Nvidia GP104 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x10f1),  "Nvidia GP106 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x10f2),  "Nvidia GV100 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x10f7),  "Nvidia TU102 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x10f8),  "Nvidia TU104 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x10f9),  "Nvidia TU106 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0x1aeb),  "Nvidia TU116 HD Audio Controller" },
    { HDA_CONTROLLER(NVIDIA, 0xffff),  "Nvidia HD Audio Controller" },

    //17f3  RDC Semiconductor, Inc.
    { HDA_CONTROLLER(RDC, 0x3010),  "RDC M3010 HD Audio Controller" },

    //1039  Silicon Integrated Systems [SiS]
    { HDA_CONTROLLER(SIS, 0x7502),  "SiS 966 HD Audio Controller" },
    { HDA_CONTROLLER(SIS, 0xffff),  "SiS HD Audio Controller" },

    //10b9  ULi Electronics Inc.(Split off ALi Corporation in 2003)
    { HDA_CONTROLLER(ULI, 0x5461),  "ULI M5461 HD Audio Controller"},
    { HDA_CONTROLLER(ULI, 0xffff),  "ULI HD Audio Controller"},

    //1106  VIA Technologies, Inc.
    { HDA_CONTROLLER(VIA, 0x3288),  "VIA VT8251/8237A HD Audio Controller" },
    { HDA_CONTROLLER(VIA, 0xffff),  "VIA HD Audio Controller" },

    // End.
    { 0,                            NULL }
};

//
// Codec models.
//

HDA_CODEC_LIST_ENTRY gHdaCodecList[] = {
    // AMD.
    { HDA_CODEC(AMD, 0xFFFF),  0x0000, "AMD (Unknown)" },

    // Analog Devices.
    { HDA_CODEC(ANALOGDEVICES, 0x1882),  0x0000, "Analog Devices AD1882" },
    { HDA_CODEC(ANALOGDEVICES, 0x882A),  0x0000, "Analog Devices AD1882A" },
    { HDA_CODEC(ANALOGDEVICES, 0x1883),  0x0000, "Analog Devices AD1883" },
    { HDA_CODEC(ANALOGDEVICES, 0x1884),  0x0000, "Analog Devices AD1884" },
    { HDA_CODEC(ANALOGDEVICES, 0x184A),  0x0000, "Analog Devices AD1884A" },
    { HDA_CODEC(ANALOGDEVICES, 0x1981),  0x0000, "Analog Devices AD1981HD" },
    { HDA_CODEC(ANALOGDEVICES, 0x1983),  0x0000, "Analog Devices AD1983" },
    { HDA_CODEC(ANALOGDEVICES, 0x1984),  0x0000, "Analog Devices AD1984" },
    { HDA_CODEC(ANALOGDEVICES, 0x194A),  0x0000, "Analog Devices AD1984A" },
    { HDA_CODEC(ANALOGDEVICES, 0x194B),  0x0000, "Analog Devices AD1984B" },
    { HDA_CODEC(ANALOGDEVICES, 0x1986),  0x0000, "Analog Devices AD1986A" },
    { HDA_CODEC(ANALOGDEVICES, 0x1987),  0x0000, "Analog Devices AD1987" },
    { HDA_CODEC(ANALOGDEVICES, 0x1988),  0x0000, "Analog Devices AD1988A" },
    { HDA_CODEC(ANALOGDEVICES, 0x198B),  0x0000, "Analog Devices AD1988B" },
    { HDA_CODEC(ANALOGDEVICES, 0x989A),  0x0000, "Analog Devices AD1989A" },
    { HDA_CODEC(ANALOGDEVICES, 0x989B),  0x0000, "Analog Devices AD2000b" },
    { HDA_CODEC(ANALOGDEVICES, 0xFFFF),  0x0000, "Analog Devices (Unknown)" },

    // Cirrus Logic.
    { HDA_CODEC(CIRRUSLOGIC, 0x4206),  0x0000, "Cirrus Logic CS4206" },
    { HDA_CODEC(CIRRUSLOGIC, 0x4207),  0x0000, "Cirrus Logic CS4207" },
    { HDA_CODEC(CIRRUSLOGIC, 0x4208),  0x0000, "Cirrus Logic CS4208" },
    { HDA_CODEC(CIRRUSLOGIC, 0x4210),  0x0000, "Cirrus Logic CS4210" },
    { HDA_CODEC(CIRRUSLOGIC, 0x4213),  0x0000, "Cirrus Logic CS4213" },
    { HDA_CODEC(CIRRUSLOGIC, 0xFFFF),  0x0000, "Cirrus Logic (Unknown)" },

    // Conexant.
    { HDA_CODEC(CONEXANT, 0x5045),  0x0000, "Conexant CX20549 (Venice)" },
    { HDA_CODEC(CONEXANT, 0x5047),  0x0000, "Conexant CX20551 (Waikiki)" },
    { HDA_CODEC(CONEXANT, 0x5051),  0x0000, "Conexant CX20561 (Hermosa)" },
    { HDA_CODEC(CONEXANT, 0x5066),  0x0000, "Conexant CX20582 (Pebble)" },
    { HDA_CODEC(CONEXANT, 0x5067),  0x0000, "Conexant CX20583 (Pebble HSF)" },
    { HDA_CODEC(CONEXANT, 0x5068),  0x0000, "Conexant CX20584" },
    { HDA_CODEC(CONEXANT, 0x5069),  0x0000, "Conexant CX20585" },
    { HDA_CODEC(CONEXANT, 0x506C),  0x0000, "Conexant CX20588" },
    { HDA_CODEC(CONEXANT, 0x506E),  0x0000, "Conexant CX20590" },
    { HDA_CODEC(CONEXANT, 0x5097),  0x0000, "Conexant CX20631" },
    { HDA_CODEC(CONEXANT, 0x5098),  0x0000, "Conexant CX20632" },
    { HDA_CODEC(CONEXANT, 0x50A1),  0x0000, "Conexant CX20641" },
    { HDA_CODEC(CONEXANT, 0x50A2),  0x0000, "Conexant CX20642" },
    { HDA_CODEC(CONEXANT, 0x50AB),  0x0000, "Conexant CX20651" },
    { HDA_CODEC(CONEXANT, 0x50AC),  0x0000, "Conexant CX20652" },
    { HDA_CODEC(CONEXANT, 0x50B8),  0x0000, "Conexant CX20664" },
    { HDA_CODEC(CONEXANT, 0x50B9),  0x0000, "Conexant CX20665" },
    { HDA_CODEC(CONEXANT, 0xffff),  0x0000, "Conexant (Unknown)" },

    // Creative.
    { HDA_CODEC(CREATIVE, 0x000A),  0x0000, "Creative CA0110-IBG" },
    { HDA_CODEC(CREATIVE, 0x000B),  0x0000, "Creative CA0110-IBG" },
    { HDA_CODEC(CREATIVE, 0x000D),  0x0000, "Creative SB0880 X-Fi" },
    { HDA_CODEC(CREATIVE, 0x0011),  0x0000, "Creative CA0132" },
    { HDA_CODEC(CREATIVE, 0xffff),  0x0000, "Creative (Unknown)" },

    // CMedia
    { HDA_CODEC(CMEDIA, 0xffff),  0x0000, "C-Media (Unknown)" },
    { HDA_CODEC(CMEDIA2, 0xffff),  0x0000, "C-Media (Unknown)" },

    // IDT.
    { HDA_CODEC(SIGMATEL, 0x7698),  0x0000, "IDT 92HD005" },
    { HDA_CODEC(SIGMATEL, 0x7699),  0x0000, "IDT 92HD005D" },
    { HDA_CODEC(SIGMATEL, 0x7645),  0x0000, "IDT 92HD206X" },
    { HDA_CODEC(SIGMATEL, 0x7646),  0x0000, "IDT 92HD206D" },
    { HDA_CODEC(IDT, 0x76E8),  0x0000, "IDT 92HD66B1X5" },
    { HDA_CODEC(IDT, 0x76E9),  0x0000, "IDT 92HD66B2X5" },
    { HDA_CODEC(IDT, 0x76EA),  0x0000, "IDT 92HD66B3X5" },
    { HDA_CODEC(IDT, 0x76EB),  0x0000, "IDT 92HD66C1X5" },
    { HDA_CODEC(IDT, 0x76EC),  0x0000, "IDT 92HD66C2X5" },
    { HDA_CODEC(IDT, 0x76ED),  0x0000, "IDT 92HD66C3X5" },
    { HDA_CODEC(IDT, 0x76EE),  0x0000, "IDT 92HD66B1X3" },
    { HDA_CODEC(IDT, 0x76EF),  0x0000, "IDT 92HD66B2X3" },
    { HDA_CODEC(IDT, 0x76F0),  0x0000, "IDT 92HD66B3X3" },
    { HDA_CODEC(IDT, 0x76F1),  0x0000, "IDT 92HD66C1X3" },
    { HDA_CODEC(IDT, 0x76F2),  0x0000, "IDT 92HD66C2X3" },
    { HDA_CODEC(IDT, 0x76F3),  0x0000, "IDT 92HD66C3_65" },
    { HDA_CODEC(SIGMATEL, 0x7638),  0x0000, "IDT 92HD700X" },
    { HDA_CODEC(SIGMATEL, 0x7639),  0x0000, "IDT 92HD700D" },
    { HDA_CODEC(IDT, 0x76B6),  0x0000, "IDT 92HD71B5" },
    { HDA_CODEC(IDT, 0x76B7),  0x0000, "IDT 92HD71B5" },
    { HDA_CODEC(IDT, 0x76B4),  0x0000, "IDT 92HD71B6" },
    { HDA_CODEC(IDT, 0x76B5),  0x0000, "IDT 92HD71B6" },
    { HDA_CODEC(IDT, 0x76B2),  0x0000, "IDT 92HD71B7" },
    { HDA_CODEC(IDT, 0x76B3),  0x0000, "IDT 92HD71B7" },
    { HDA_CODEC(IDT, 0x76B0),  0x0000, "IDT 92HD71B8" },
    { HDA_CODEC(IDT, 0x76B1),  0x0000, "IDT 92HD71B8" },
    { HDA_CODEC(IDT, 0x7675),  0x0000, "IDT 92HD73C1" },
    { HDA_CODEC(IDT, 0x7674),  0x0000, "IDT 92HD73D1" },
    { HDA_CODEC(IDT, 0x7676),  0x0000, "IDT 92HD73E1" },
    { HDA_CODEC(IDT, 0x7608),  0x0000, "IDT 92HD75B3" },
    { HDA_CODEC(IDT, 0x7603),  0x0000, "IDT 92HD75BX" },
    { HDA_CODEC(IDT, 0x76D5),  0x0000, "IDT 92HD81B1C" },
    { HDA_CODEC(IDT, 0x7605),  0x0000, "IDT 92HD81B1X" },
    { HDA_CODEC(IDT, 0x76D4),  0x0000, "IDT 92HD83C1C" },
    { HDA_CODEC(IDT, 0x7604),  0x0000, "IDT 92HD83C1X" },
    { HDA_CODEC(IDT, 0x76D1),  0x0000, "IDT 92HD87B1/3" },
    { HDA_CODEC(IDT, 0x76D9),  0x0000, "IDT 92HD87B2/4" },
    { HDA_CODEC(IDT, 0x76C0),  0x0000, "IDT 92HD89C3" },
    { HDA_CODEC(IDT, 0x76C1),  0x0000, "IDT 92HD89C2" },
    { HDA_CODEC(IDT, 0x76C2),  0x0000, "IDT 92HD89C1" },
    { HDA_CODEC(IDT, 0x76C3),  0x0000, "IDT 92HD89B3" },
    { HDA_CODEC(IDT, 0x76C4),  0x0000, "IDT 92HD89B2" },
    { HDA_CODEC(IDT, 0x76C5),  0x0000, "IDT 92HD89B1" },
    { HDA_CODEC(IDT, 0x76C6),  0x0000, "IDT 92HD89E3" },
    { HDA_CODEC(IDT, 0x76C7),  0x0000, "IDT 92HD89E2" },
    { HDA_CODEC(IDT, 0x76C8),  0x0000, "IDT 92HD89E1" },
    { HDA_CODEC(IDT, 0x76C9),  0x0000, "IDT 92HD89D3" },
    { HDA_CODEC(IDT, 0x76CA),  0x0000, "IDT 92HD89D2" },
    { HDA_CODEC(IDT, 0x76CB),  0x0000, "IDT 92HD89D1" },
    { HDA_CODEC(IDT, 0x76CC),  0x0000, "IDT 92HD89F3" },
    { HDA_CODEC(IDT, 0x76CD),  0x0000, "IDT 92HD89F2" },
    { HDA_CODEC(IDT, 0x76CE),  0x0000, "IDT 92HD89F1" },
    { HDA_CODEC(IDT, 0x76E7),  0x0000, "IDT 92HD90BXX" },
    { HDA_CODEC(IDT, 0x76E0),  0x0000, "IDT 92HD91BXX" },
    { HDA_CODEC(IDT, 0x76Df),  0x0000, "IDT 92HD93BXX" },
    { HDA_CODEC(IDT, 0x76E3),  0x0000, "IDT 92HD98BXX" },
    { HDA_CODEC(IDT, 0x76E5),  0x0000, "IDT 92HD99BXX" },
    { HDA_CODEC(IDT, 0xFFFF),  0x0000, "IDT (Unknown)" },

    // Intel.
    { HDA_CODEC(INTEL, 0x29FB),  0x0000, "Intel Crestline HDMI" },
    { HDA_CODEC(INTEL, 0x2801),  0x0000, "Intel Bearlake HDMI" },
    { HDA_CODEC(INTEL, 0x2802),  0x0000, "Intel Cantiga HDMI" },
    { HDA_CODEC(INTEL, 0x2803),  0x0000, "Intel Eaglelake HDMI" },
    { HDA_CODEC(INTEL, 0x2804),  0x0000, "Intel Ibex Peak HDMI" },
    { HDA_CODEC(INTEL, 0x0054),  0x0000, "Intel Ibex Peak HDMI" },
    { HDA_CODEC(INTEL, 0x2805),  0x0000, "Intel Cougar Point HDMI" },
    { HDA_CODEC(INTEL, 0x2806),  0x0000, "Intel Panther Point HDMI" },
    { HDA_CODEC(INTEL, 0x2807),  0x0000, "Intel Haswell HDMI" },
    { HDA_CODEC(INTEL, 0x2808),  0x0000, "Intel Broadwell HDMI" },
    { HDA_CODEC(INTEL, 0x2809),  0x0000, "Intel Skylake HDMI" },
    { HDA_CODEC(INTEL, 0x280A),  0x0000, "Intel Broxton HDMI" },
    { HDA_CODEC(INTEL, 0x280B),  0x0000, "Intel Kaby Lake HDMI" },
    { HDA_CODEC(INTEL, 0x280C),  0x0000, "Intel Cannon Lake HDMI" },
    { HDA_CODEC(INTEL, 0x280D),  0x0000, "Intel Gemini Lake HDMI" },
    { HDA_CODEC(INTEL, 0x2800),  0x0000, "Intel Gemini Lake HDMI" },
    { HDA_CODEC(INTEL, 0xFFFF),  0x0000, "Intel (Unknown)" },

    // Motorola.
    { HDA_CODEC(MOTO, 0xFFFF),  0x0000, "Motorola (Unknown)" },

    // Silicon Image.
    { HDA_CODEC(SII, 0xFFFF),  0x0000, "Silicon Image (Unknown)" },

    // LSI - Lucent/Agere
    { HDA_CODEC(AGERE, 0xFFFF),  0x0000, "LSI (Unknown)" },

    // Chrontel
    { HDA_CODEC(CHRONTEL, 0xFFFF),  0x0000, "Chrontel (Unknown)" },

    // LG
    { HDA_CODEC(LG, 0xFFFF),  0x0000, "LG (Unknown)" },

    // Wolfson Microelectronics
    { HDA_CODEC(WOLFSON, 0xFFFF),  0x0000, "Wolfson Microelectronics (Unknown)" },

    // QEMU
    { HDA_CODEC(QEMU, 0xFFFF),  0x0000, "QEMU (Unknown)" },

    // Nvidia.
    { HDA_CODEC(NVIDIA, 0xFFFF),  0x0000, "Nvidia (Unknown)" },

    // Realtek.
    { HDA_CODEC(REALTEK, 0x0221),  0x0000, "Realtek ALC221" },
    { HDA_CODEC(REALTEK, 0x0225),  0x0000, "Realtek ALC225" },
    { HDA_CODEC(REALTEK, 0x0230),  0x0000, "Realtek ALC230" },
    { HDA_CODEC(REALTEK, 0x0233),  0x0000, "Realtek ALC233" },
    { HDA_CODEC(REALTEK, 0x0235),  0x0000, "Realtek ALC235" },
    { HDA_CODEC(REALTEK, 0x0236),  0x0000, "Realtek ALC236" },
    { HDA_CODEC(REALTEK, 0x0255),  0x0000, "Realtek ALC255" },
    { HDA_CODEC(REALTEK, 0x0256),  0x0000, "Realtek ALC256" },
    { HDA_CODEC(REALTEK, 0x0257),  0x0000, "Realtek ALC257" },
    { HDA_CODEC(REALTEK, 0x0260),  0x0000, "Realtek ALC260" },
    { HDA_CODEC(REALTEK, 0x0262),  0x0000, "Realtek ALC262" },
    { HDA_CODEC(REALTEK, 0x0267),  0x0000, "Realtek ALC267" },
    { HDA_CODEC(REALTEK, 0x0268),  0x0000, "Realtek ALC268" },
    { HDA_CODEC(REALTEK, 0x0269),  0x0000, "Realtek ALC269" },
    { HDA_CODEC(REALTEK, 0x0270),  0x0000, "Realtek ALC270" },
    { HDA_CODEC(REALTEK, 0x0272),  0x0000, "Realtek ALC272" },
    { HDA_CODEC(REALTEK, 0x0273),  0x0000, "Realtek ALC273" },
    { HDA_CODEC(REALTEK, 0x0275),  0x0000, "Realtek ALC275" },
    { HDA_CODEC(REALTEK, 0x0276),  0x0000, "Realtek ALC276" },
    { HDA_CODEC(REALTEK, 0x0280),  0x0000, "Realtek ALC280" },
    { HDA_CODEC(REALTEK, 0x0282),  0x0000, "Realtek ALC282" },
    { HDA_CODEC(REALTEK, 0x0283),  0x0000, "Realtek ALC283" },
    { HDA_CODEC(REALTEK, 0x0284),  0x0000, "Realtek ALC284" },
    { HDA_CODEC(REALTEK, 0x0285),  0x0000, "Realtek ALC285" },
    { HDA_CODEC(REALTEK, 0x0286),  0x0000, "Realtek ALC286" },
    { HDA_CODEC(REALTEK, 0x0288),  0x0000, "Realtek ALC288" },
    { HDA_CODEC(REALTEK, 0x0289),  0x0000, "Realtek ALC289" },
    { HDA_CODEC(REALTEK, 0x0290),  0x0000, "Realtek ALC290" },
    { HDA_CODEC(REALTEK, 0x0292),  0x0000, "Realtek ALC292" },
    { HDA_CODEC(REALTEK, 0x0293),  0x0000, "Realtek ALC293" },
    { HDA_CODEC(REALTEK, 0x0294),  0x0000, "Realtek ALC294" },
    { HDA_CODEC(REALTEK, 0x0295),  0x0000, "Realtek ALC295" },
    { HDA_CODEC(REALTEK, 0x0298),  0x0000, "Realtek ALC298" },
    { HDA_CODEC(REALTEK, 0x0660),  0x0000, "Realtek ALC660" },
    { HDA_CODEC(REALTEK, 0x0662),  0x0002, "Realtek ALC662v2" },
    { HDA_CODEC(REALTEK, 0x0662),  0x0000, "Realtek ALC662" },
    { HDA_CODEC(REALTEK, 0x0663),  0x0000, "Realtek ALC663" },
    { HDA_CODEC(REALTEK, 0x0665),  0x0000, "Realtek ALC665" },
    { HDA_CODEC(REALTEK, 0x0668),  0x0000, "Realtek ALC668" },
    { HDA_CODEC(REALTEK, 0x0670),  0x0000, "Realtek ALC670" },
    { HDA_CODEC(REALTEK, 0x0671),  0x0000, "Realtek ALC671" },
    { HDA_CODEC(REALTEK, 0x0680),  0x0000, "Realtek ALC680" },
    { HDA_CODEC(REALTEK, 0x0861),  0x0000, "Realtek ALC861" },
    { HDA_CODEC(REALTEK, 0x0862),  0x0000, "Realtek ALC861-VD" },
    { HDA_CODEC(REALTEK, 0x0880),  0x0000, "Realtek ALC880" },
    { HDA_CODEC(REALTEK, 0x0882),  0x0000, "Realtek ALC882" },
    { HDA_CODEC(REALTEK, 0x0883),  0x0000, "Realtek ALC883" },
    { HDA_CODEC(REALTEK, 0x0885),  0x0103, "Realtek ALC889A" },
    { HDA_CODEC(REALTEK, 0x0885),  0x0101, "Realtek ALC889A" },
    { HDA_CODEC(REALTEK, 0x0885),  0x0000, "Realtek ALC885" },
    { HDA_CODEC(REALTEK, 0x0887),  0x0302, "Realtek ALC888B" },
    { HDA_CODEC(REALTEK, 0x0887),  0x0002, "Realtek ALC887-VD2" },
    { HDA_CODEC(REALTEK, 0x0887),  0x0001, "Realtek ALC887-VD" },
    { HDA_CODEC(REALTEK, 0x0887),  0x0000, "Realtek ALC887" },
    { HDA_CODEC(REALTEK, 0x0888),  0x0003, "Realtek ALC888S-VD" },
    { HDA_CODEC(REALTEK, 0x0888),  0x0002, "Realtek ALC888S-VC" },
    { HDA_CODEC(REALTEK, 0x0888),  0x0001, "Realtek ALC888S" },
    { HDA_CODEC(REALTEK, 0x0888),  0x0000, "Realtek ALC888" },
    { HDA_CODEC(REALTEK, 0x0889),  0x0000, "Realtek ALC889" },
    { HDA_CODEC(REALTEK, 0x0892),  0x0000, "Realtek ALC892" },
    { HDA_CODEC(REALTEK, 0x0898),  0x0000, "Realtek ALC898" },
    { HDA_CODEC(REALTEK, 0x0899),  0x0000, "Realtek ALC899" },
    { HDA_CODEC(REALTEK, 0x0900),  0x0000, "Realtek ALC1150" },
    { HDA_CODEC(REALTEK, 0x1220),  0x0000, "Realtek ALC1220" },
    { HDA_CODEC(REALTEK, 0xFFFF),  0x0000, "Realtek (Unknown)" },

    // Sigmatel.
    { HDA_CODEC(SIGMATEL, 0x7661),  0x0000, "Sigmatel CXD9872RD/K" },
    { HDA_CODEC(SIGMATEL, 0x7664),  0x0000, "Sigmatel CXD9872AKD" },
    { HDA_CODEC(SIGMATEL, 0x7691),  0x0000, "Sigmatel STAC9200D" },
    { HDA_CODEC(SIGMATEL, 0x76A2),  0x0000, "Sigmatel STAC9204X" },
    { HDA_CODEC(SIGMATEL, 0x76A3),  0x0000, "Sigmatel STAC9204D" },
    { HDA_CODEC(SIGMATEL, 0x76A0),  0x0000, "Sigmatel STAC9205X" },
    { HDA_CODEC(SIGMATEL, 0x76A1),  0x0000, "Sigmatel STAC9205D" },
    { HDA_CODEC(SIGMATEL, 0x7690),  0x0000, "Sigmatel STAC9220" },
    { HDA_CODEC(SIGMATEL, 0x7882),  0x0000, "Sigmatel STAC9220_A1" },
    { HDA_CODEC(SIGMATEL, 0x7880),  0x0000, "Sigmatel STAC9220_A2" },
    { HDA_CODEC(SIGMATEL, 0x7680),  0x0000, "Sigmatel STAC9221" },
    { HDA_CODEC(SIGMATEL, 0x7682),  0x0000, "Sigmatel STAC9221_A2" },
    { HDA_CODEC(SIGMATEL, 0x7683),  0x0000, "Sigmatel STAC9221D" },
    { HDA_CODEC(SIGMATEL, 0x7681),  0x0000, "Sigmatel STAC9220D/9223D" },
    { HDA_CODEC(SIGMATEL, 0x7618),  0x0000, "Sigmatel STAC9227X" },
    { HDA_CODEC(SIGMATEL, 0x7619),  0x0000, "Sigmatel STAC9227D" },
    { HDA_CODEC(SIGMATEL, 0x7616),  0x0000, "Sigmatel STAC9228X" },
    { HDA_CODEC(SIGMATEL, 0x7617),  0x0000, "Sigmatel STAC9228D" },
    { HDA_CODEC(SIGMATEL, 0x7614),  0x0000, "Sigmatel STAC9229X" },
    { HDA_CODEC(SIGMATEL, 0x7615),  0x0000, "Sigmatel STAC9229D" },
    { HDA_CODEC(SIGMATEL, 0x7612),  0x0000, "Sigmatel STAC9230X" },
    { HDA_CODEC(SIGMATEL, 0x7613),  0x0000, "Sigmatel STAC9230D" },
    { HDA_CODEC(SIGMATEL, 0x7634),  0x0000, "Sigmatel STAC9250" },
    { HDA_CODEC(SIGMATEL, 0x7636),  0x0000, "Sigmatel STAC9251" },
    { HDA_CODEC(SIGMATEL, 0x76A4),  0x0000, "Sigmatel STAC9255" },
    { HDA_CODEC(SIGMATEL, 0x76A5),  0x0000, "Sigmatel STAC9255D" },
    { HDA_CODEC(SIGMATEL, 0x76A6),  0x0000, "Sigmatel STAC9254" },
    { HDA_CODEC(SIGMATEL, 0x76A7),  0x0000, "Sigmatel STAC9254D" },
    { HDA_CODEC(SIGMATEL, 0x7626),  0x0000, "Sigmatel STAC9271X" },
    { HDA_CODEC(SIGMATEL, 0x7627),  0x0000, "Sigmatel STAC9271D" },
    { HDA_CODEC(SIGMATEL, 0x7624),  0x0000, "Sigmatel STAC9272X" },
    { HDA_CODEC(SIGMATEL, 0x7625),  0x0000, "Sigmatel STAC9272D" },
    { HDA_CODEC(SIGMATEL, 0x7622),  0x0000, "Sigmatel STAC9273X" },
    { HDA_CODEC(SIGMATEL, 0x7623),  0x0000, "Sigmatel STAC9273D" },
    { HDA_CODEC(SIGMATEL, 0x7620),  0x0000, "Sigmatel STAC9274" },
    { HDA_CODEC(SIGMATEL, 0x7621),  0x0000, "Sigmatel STAC9274D" },
    { HDA_CODEC(SIGMATEL, 0x7628),  0x0000, "Sigmatel STAC9274X5NH" },
    { HDA_CODEC(SIGMATEL, 0x7629),  0x0000, "Sigmatel STAC9274D5NH" },
    { HDA_CODEC(SIGMATEL, 0x7662),  0x0000, "Sigmatel STAC9872AK" },
    { HDA_CODEC(SIGMATEL, 0xFFFF),  0x0000, "Sigmatel (Unknown)" },

    // VIA.
    { HDA_CODEC(VIA, 0x1708),  0x0000, "VIA VT1708_8" },
    { HDA_CODEC(VIA, 0x1709),  0x0000, "VIA VT1708_9" },
    { HDA_CODEC(VIA, 0x170A),  0x0000, "VIA VT1708_A" },
    { HDA_CODEC(VIA, 0x170B),  0x0000, "VIA VT1708_B" },
    { HDA_CODEC(VIA, 0xe710),  0x0000, "VIA VT1709_0" },
    { HDA_CODEC(VIA, 0xe711),  0x0000, "VIA VT1709_1" },
    { HDA_CODEC(VIA, 0xe712),  0x0000, "VIA VT1709_2" },
    { HDA_CODEC(VIA, 0xe713),  0x0000, "VIA VT1709_3" },
    { HDA_CODEC(VIA, 0xe714),  0x0000, "VIA VT1709_4" },
    { HDA_CODEC(VIA, 0xe715),  0x0000, "VIA VT1709_5" },
    { HDA_CODEC(VIA, 0xe716),  0x0000, "VIA VT1709_6" },
    { HDA_CODEC(VIA, 0xe717),  0x0000, "VIA VT1709_7" },
    { HDA_CODEC(VIA, 0xe720),  0x0000, "VIA VT1708B_0" },
    { HDA_CODEC(VIA, 0xe721),  0x0000, "VIA VT1708B_1" },
    { HDA_CODEC(VIA, 0xe722),  0x0000, "VIA VT1708B_2" },
    { HDA_CODEC(VIA, 0xe723),  0x0000, "VIA VT1708B_3" },
    { HDA_CODEC(VIA, 0xe724),  0x0000, "VIA VT1708B_4" },
    { HDA_CODEC(VIA, 0xe725),  0x0000, "VIA VT1708B_5" },
    { HDA_CODEC(VIA, 0xe726),  0x0000, "VIA VT1708B_6" },
    { HDA_CODEC(VIA, 0xe727),  0x0000, "VIA VT1708B_7" },
    { HDA_CODEC(VIA, 0x0397),  0x0000, "VIA VT1708S_0" },
    { HDA_CODEC(VIA, 0x1397),  0x0000, "VIA VT1708S_1" },
    { HDA_CODEC(VIA, 0x2397),  0x0000, "VIA VT1708S_2" },
    { HDA_CODEC(VIA, 0x3397),  0x0000, "VIA VT1708S_3" },
    { HDA_CODEC(VIA, 0x4397),  0x0000, "VIA VT1708S_4" },
    { HDA_CODEC(VIA, 0x5397),  0x0000, "VIA VT1708S_5" },
    { HDA_CODEC(VIA, 0x6397),  0x0000, "VIA VT1708S_6" },
    { HDA_CODEC(VIA, 0x7397),  0x0000, "VIA VT1708S_7" },
    { HDA_CODEC(VIA, 0x0398),  0x0000, "VIA VT1702_0" },
    { HDA_CODEC(VIA, 0x1398),  0x0000, "VIA VT1702_1" },
    { HDA_CODEC(VIA, 0x2398),  0x0000, "VIA VT1702_2" },
    { HDA_CODEC(VIA, 0x3398),  0x0000, "VIA VT1702_3" },
    { HDA_CODEC(VIA, 0x4398),  0x0000, "VIA VT1702_4" },
    { HDA_CODEC(VIA, 0x5398),  0x0000, "VIA VT1702_5" },
    { HDA_CODEC(VIA, 0x6398),  0x0000, "VIA VT1702_6" },
    { HDA_CODEC(VIA, 0x7398),  0x0000, "VIA VT1702_7" },
    { HDA_CODEC(VIA, 0x0433),  0x0000, "VIA VT1716S_0" },
    { HDA_CODEC(VIA, 0xA721),  0x0000, "VIA VT1716S_1" },
    { HDA_CODEC(VIA, 0x0428),  0x0000, "VIA VT1718S_0" },
    { HDA_CODEC(VIA, 0x4428),  0x0000, "VIA VT1718S_1" },
    { HDA_CODEC(VIA, 0x0446),  0x0000, "VIA VT1802_0" },
    { HDA_CODEC(VIA, 0x8446),  0x0000, "VIA VT1802_1" },
    { HDA_CODEC(VIA, 0x0448),  0x0000, "VIA VT1812" },
    { HDA_CODEC(VIA, 0x0440),  0x0000, "VIA VT1818S" },
    { HDA_CODEC(VIA, 0x4441),  0x0000, "VIA VT1828S" },
    { HDA_CODEC(VIA, 0x0438),  0x0000, "VIA VT2002P_0" },
    { HDA_CODEC(VIA, 0x4438),  0x0000, "VIA VT2002P_1" },
    { HDA_CODEC(VIA, 0x0441),  0x0000, "VIA VT2020" },
    { HDA_CODEC(VIA, 0xFFFF),  0x0000, "VIA (Unknown)" },

    // End.
    { 0,                            0x0000, NULL }
};


/*****************
 * Device Methods
 *****************/

/* get HDA device name */

VOID
EFIAPI
HdaControllerGetName(IN UINT32 ControllerID, OUT CHAR16 **Name)
{
  // DEBUG((DEBUG_INFO, "HdaControllerGetName(): start\n"));

  // Try to match controller name.
  UINTN ControllerIndex = 0;
  while (gHdaControllerList[ControllerIndex].Id != 0) {
    // Check ID and revision against array element.
    if (gHdaControllerList[ControllerIndex].Id == ControllerID) {
      *Name = AllocateZeroPool(64 * sizeof(CHAR16));
      AsciiStrToUnicodeStrS(gHdaControllerList[ControllerIndex].Name, *Name, 64);
      return;
    }
    ControllerIndex++;
  }

  // If match wasn't found, try again with a generic device ID. 
    ControllerIndex = 0;
    while (gHdaControllerList[ControllerIndex].Id != 0) {
      // Check ID and revision against array element.
      if (gHdaControllerList[ControllerIndex].Id == GET_PCI_GENERIC_ID(ControllerID)) {
        *Name = AllocateZeroPool(64 * sizeof(CHAR16));
        AsciiStrToUnicodeStrS(gHdaControllerList[ControllerIndex].Name, *Name, 64);
        return;
      }
      ControllerIndex++;
    }
  
    // If match still wasn't found, controller is unknown.
    *Name = AllocateCopyPool(StrSize(HDA_CONTROLLER_MODEL_GENERIC), HDA_CONTROLLER_MODEL_GENERIC);
  
  //  DEBUG((DEBUG_INFO, "HdaControllerGetName(): controller is %a\n", *Name));
}

VOID
EFIAPI
HdaCodecGetName(IN UINT32 CodecID, IN UINT16 RevisionId, OUT CHAR16 **Name)
{
  UINTN CodecIndex = 0;
  // Try to match codec name.
  
  while (gHdaCodecList[CodecIndex].Id != 0) {
    // Check ID and revision against array element.
    if ((gHdaCodecList[CodecIndex].Id == CodecID) && (gHdaCodecList[CodecIndex].Rev <= RevisionId)) {
      *Name = AllocateZeroPool(64 * sizeof(CHAR16));
      AsciiStrToUnicodeStrS(gHdaCodecList[CodecIndex].Name, *Name, 64);
      return;
    }
    CodecIndex++;
  }

  // If match wasn't found, try again with a generic device ID.
    CodecIndex = 0;
    while (gHdaCodecList[CodecIndex].Id != 0) {
      // Check ID and revision against array element.
      if (gHdaCodecList[CodecIndex].Id == GET_CODEC_GENERIC_ID(CodecID)) {
        *Name = AllocateZeroPool(64 * sizeof(CHAR16));
        AsciiStrToUnicodeStrS(gHdaCodecList[CodecIndex].Name, *Name, 64);
        return;
      }
      CodecIndex++;
    }

  // If match still wasn't found, codec is unknown.
  *Name = AllocateCopyPool(StrSize(HDA_CODEC_MODEL_GENERIC), HDA_CODEC_MODEL_GENERIC);

  // DEBUG((DEBUG_INFO, "Codec name: %s\n", HdaCodecDev->Name));
}
