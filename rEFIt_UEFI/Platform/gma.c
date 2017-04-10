/*
	Original patch by Nawcom
	http://forum.voodooprojects.org/index.php/topic,1029.0.html
*/


/*
	Information
	http://www.insanelymac.com/forum/topic/286092-guide-1st-generation-intel-hd-graphics-qeci/
	http://www.insanelymac.com/forum/topic/290783-intel-hd-graphics-4600-haswell-working-displayport/
	https://github.com/anholt/mesa/blob/master/include/pci_ids/i965_pci_ids.h
	https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-skl-vol04-configurations.pdf
	https://fossies.org/linux/mesa/include/pci_ids/i965_pci_ids.h
	https://public.xzenue.com/file/data/55wgusibooqbpc6a5pgy/PHID-FILE-7acmi5x2tsudnpizwzs7/2600.diff
	https://github.com/RehabMan/OS-X-Clover-Laptop-Config/blob/master/hotpatch/SSDT-IGPU.dsl
*/


// All Intel graphics data were updated and the code was rewritten by Sherlocks

#include "Platform.h"
#include "gma.h"

#ifndef DEBUG_GMA
#ifndef DEBUG_ALL
#define DEBUG_GMA 1
#else
#define DEBUG_GMA DEBUG_ALL
#endif
#endif

#if DEBUG_GMA == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_GMA, __VA_ARGS__)
#endif


extern CHAR8*    gDeviceProperties;
extern CHAR8     ClassFix[];       // { 0x00, 0x00, 0x03, 0x00 }; from FixBiosDsdt.c


UINT8 common_vals[3][4] = {
    { 0x00, 0x00, 0x00, 0x00 },	  //0 reg_FALSE
    { 0x01, 0x00, 0x00, 0x00 },	  //1 reg_TRUE
    { 0x6B, 0x10, 0x00, 0x00 },	  //2 "subsystem-vendor-id"
};


UINT8 GMAX3100_vals[23][4] = {
    { 0x01, 0x00, 0x00, 0x00 },	  //0 "AAPL,HasPanel"
    { 0x01, 0x00, 0x00, 0x00 },	  //1 "AAPL,SelfRefreshSupported"
    { 0x01, 0x00, 0x00, 0x00 },	  //2 "AAPL,aux-power-connected"
    { 0x01, 0x00, 0x00, 0x08 },	  //3 "AAPL,backlight-control"
    { 0x00, 0x00, 0x00, 0x00 },	  //4 "AAPL00,blackscreen-preferences"
    { 0x56, 0x00, 0x00, 0x08 },	  //5 "AAPL01,BacklightIntensity"
    { 0x00, 0x00, 0x00, 0x00 },	  //6 "AAPL01,blackscreen-preferences"
    { 0x01, 0x00, 0x00, 0x00 },	  //7 "AAPL01,DataJustify"
    { 0x20, 0x00, 0x00, 0x00 },	  //8 "AAPL01,Depth"
    { 0x01, 0x00, 0x00, 0x00 },	  //9 "AAPL01,Dither"
    { 0x20, 0x03, 0x00, 0x00 },	  //10 "AAPL01,Height"
    { 0x00, 0x00, 0x00, 0x00 },	  //11 "AAPL01,Interlace"
    { 0x00, 0x00, 0x00, 0x00 },	  //12 "AAPL01,Inverter"
    { 0x08, 0x52, 0x00, 0x00 },	  //13 "AAPL01,InverterCurrent"
    { 0x00, 0x00, 0x00, 0x00 },	  //14 "AAPL01,LinkFormat"
    { 0x00, 0x00, 0x00, 0x00 },	  //15 "AAPL01,LinkType"
    { 0x01, 0x00, 0x00, 0x00 },	  //16 "AAPL01,Pipe"
    { 0x01, 0x00, 0x00, 0x00 },	  //17 "AAPL01,PixelFormat"
    { 0x01, 0x00, 0x00, 0x00 },	  //18 "AAPL01,Refresh"
    { 0x3B, 0x00, 0x00, 0x00 },	  //19 "AAPL01,Stretch"
    { 0xc8, 0x95, 0x00, 0x00 },	  //20 "AAPL01,InverterFrequency"
    { 0x6B, 0x10, 0x00, 0x00 },	  //21 "subsystem-vendor-id"
    { 0xA2, 0x00, 0x00, 0x00 },	  //22 "subsystem-id"
};


UINT8 arrandale_vals[1][4] = {
    { 0x00, 0x00, 0x00, 0x12 },	  //0 "VRAM,totalsize"
};


UINT8 sandy_bridge_snb_vals[5][4] = {
    { 0x00, 0x00, 0x01, 0x00 },	  //0 MacBookPro81
    { 0x00, 0x00, 0x02, 0x00 },	  //1 MacBookPro83
    { 0x10, 0x00, 0x03, 0x00 },	  //2 MacMini51
    { 0x20, 0x00, 0x03, 0x00 },	  //3 MacMini52
    { 0x00, 0x00, 0x04, 0x00 },	  //4 MacBookAir41
};

UINT8 sandy_bridge_hd_vals[14][4] = {
    { 0x00, 0x00, 0x00, 0x00 },	  //0 "AAPL00,PixelFormat"
    { 0x00, 0x00, 0x00, 0x00 },	  //1 "AAPL00,T1"
    { 0x14, 0x00, 0x00, 0x00 },	  //2 "AAPL00,T2"
    { 0xfa, 0x00, 0x00, 0x00 },	  //3 "AAPL00,T3"
    { 0x2c, 0x01, 0x00, 0x00 },	  //4 "AAPL00,T4"
    { 0x00, 0x00, 0x00, 0x00 },	  //5 "AAPL00,T5"
    { 0x14, 0x00, 0x00, 0x00 },	  //6 "AAPL00,T6"
    { 0xf4, 0x01, 0x00, 0x00 },	  //7 "AAPL00,T7"
    { 0x00, 0x00, 0x00, 0x00 },	  //8 "AAPL00,LinkType"
    { 0x00, 0x00, 0x00, 0x00 },	  //9 "AAPL00,LinkFormat"
    { 0x00, 0x00, 0x00, 0x00 },	  //10 "AAPL00,DualLink"
    { 0x00, 0x00, 0x00, 0x00 },	  //11 "AAPL00,Dither"
    { 0x00, 0x00, 0x00, 0x00 },	  //12 "AAPL00,DataJustify"
    { 0x00, 0x00, 0x00, 0x00 },	  //13 "graphic-options"
};


UINT8 ivy_bridge_ig_vals[12][4] = {
    { 0x00, 0x00, 0x66, 0x01 },	  //0 "AAPL,ig-platform-id" //FB: 96MB, Pipes: 3, Ports: 4, FBMem: 3
    { 0x01, 0x00, 0x66, 0x01 },	  //1 "AAPL,ig-platform-id" //FB: 96MB, Pipes: 3, Ports: 4, FBMem: 3
    { 0x02, 0x00, 0x66, 0x01 },	  //2 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 1, FBMem: 1
    { 0x03, 0x00, 0x66, 0x01 },	  //3 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 2, Ports: 2, FBMem: 2
    { 0x04, 0x00, 0x66, 0x01 },	  //4 "AAPL,ig-platform-id" //FB: 32MB, Pipes: 3, Ports: 1, FBMem: 1
    { 0x05, 0x00, 0x62, 0x01 },	  //5 "AAPL,ig-platform-id" //FB: 32MB, Pipes: 2, Ports: 3, FBMem: 2
    { 0x06, 0x00, 0x62, 0x01 },	  //6 "AAPL,ig-platform-id" //FB: 0MB, Pipes: 0, Ports: 0, FBMem: 0
    { 0x07, 0x00, 0x62, 0x01 },	  //7 "AAPL,ig-platform-id" //FB: 0MB, Pipes: 0, Ports: 0, FBMem: 0
    { 0x08, 0x00, 0x66, 0x01 },	  //8 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 3, FBMem: 3
    { 0x09, 0x00, 0x66, 0x01 },	  //9 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 3, FBMem: 3
    { 0x0a, 0x00, 0x66, 0x01 },	  //10 "AAPL,ig-platform-id" //FB: 32MB, Pipes: 2, Ports: 3, FBMem: 2
    { 0x0b, 0x00, 0x66, 0x01 },	  //11 "AAPL,ig-platform-id" //FB: 32MB, Pipes: 2, Ports: 3, FBMem: 2
};

UINT8 ivy_bridge_hd_vals[15][4] = {
    { 0x00, 0x00, 0x00, 0x00 },	  //0 "AAPL00,PixelFormat"
    { 0x00, 0x00, 0x00, 0x00 },	  //1 "AAPL00,T1"
    { 0x01, 0x00, 0x00, 0x00 },	  //2 "AAPL00,T2"
    { 0xc8, 0x00, 0x00, 0x00 },	  //3 "AAPL00,T3"
    { 0xc8, 0x00, 0x00, 0x00 },	  //4 "AAPL00,T4"
    { 0x01, 0x00, 0x00, 0x00 },	  //5 "AAPL00,T5"
    { 0x00, 0x00, 0x00, 0x00 },	  //6 "AAPL00,T6"
    { 0x90, 0x01, 0x00, 0x00 },	  //7 "AAPL00,T7"
    { 0x01, 0x00, 0x00, 0x00 },	  //8 "AAPL00,LinkType"
    { 0x00, 0x00, 0x00, 0x00 },	  //9 "AAPL00,LinkFormat"
    { 0x01, 0x00, 0x00, 0x00 },	  //10 "AAPL00,DualLink"
    { 0x00, 0x00, 0x00, 0x00 },	  //11 "AAPL00,Dither"
    { 0xc3, 0x8c, 0x64, 0x00 },	  //12 "AAPL,gray-value"
    { 0x01, 0x00, 0x00, 0x00 },	  //13 "AAPL,gray-page"
    { 0x0c, 0x00, 0x00, 0x00 },	  //14 "graphics-options"
};


UINT8 haswell_ig_vals[17][4] = {
    { 0x00, 0x00, 0x06, 0x04 },	  //0 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 3, FBMem: 3 - mobile GT1
    { 0x00, 0x00, 0x06, 0x0c },	  //1 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 3, FBMem: 3 - SDV mobile GT1
    { 0x00, 0x00, 0x16, 0x04 },	  //2 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 3, FBMem: 3 - mobile GT2
    { 0x00, 0x00, 0x16, 0x0a },	  //3 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 3, FBMem: 3 - ULT mobile GT2
    { 0x00, 0x00, 0x16, 0x0c },	  //4 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 3, FBMem: 3 - SDV mobile GT2
    { 0x00, 0x00, 0x26, 0x04 },	  //5 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 3, FBMem: 3 - mobile GT3
    { 0x00, 0x00, 0x26, 0x0a },	  //6 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 3, FBMem: 3 - ULT mobile GT3
    { 0x00, 0x00, 0x26, 0x0c },	  //7 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 3, FBMem: 3 - SDV mobile GT3
    { 0x00, 0x00, 0x26, 0x0d },	  //8 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 3, FBMem: 3 - CRW mobile GT3
    { 0x02, 0x00, 0x16, 0x04 },	  //9 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 1, Ports: 1, FBMem: 1 - mobile GT2
    { 0x03, 0x00, 0x22, 0x0d },	  //10 "AAPL,ig-platform-id" //FB: 0MB, Pipes: 0, Ports: 0, FBMem: 0 - CRW Desktop GT3
    { 0x04, 0x00, 0x12, 0x04 },	  //11 "AAPL,ig-platform-id" //FB: 32MB, Pipes: 3, Ports: 3, FBMem: 3 - ULT mobile GT3
    { 0x05, 0x00, 0x26, 0x0a },	  //12 "AAPL,ig-platform-id" //FB: 32MB, Pipes: 3, Ports: 3, FBMem: 3 - ULT mobile GT3
    { 0x06, 0x00, 0x26, 0x0a },	  //13 "AAPL,ig-platform-id" //FB: 32MB, Pipes: 3, Ports: 3, FBMem: 3 - ULT mobile GT3
    { 0x07, 0x00, 0x26, 0x0d },	  //14 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 4, FBMem: 3 - CRW mobile GT3
    { 0x08, 0x00, 0x26, 0x0a },	  //15 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 3, FBMem: 3 - ULT mobile GT3
    { 0x08, 0x00, 0x2e, 0x0a },	  //16 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 3, FBMem: 3 - ULT reserved GT3
};

UINT8 haswell_hd_vals[2][4] = {
    { 0x00, 0x00, 0x00, 0x00 },	  //0 "AAPL,gray-value"
    { 0x0c, 0x00, 0x00, 0x00 },	  //1 "graphics-options"
};


UINT8 broadwell_ig_vals[19][4] = {
    { 0x00, 0x00, 0x06, 0x16 },   //0 Broadwell GT1 (Intel HD Graphics)
    { 0x00, 0x00, 0x06, 0x16 },   //1 Broadwell GT1 (Intel HD Graphics)
    { 0x00, 0x00, 0x16, 0x16 },   //2 Broadwell GT2 (Intel HD Graphics 5500)
    { 0x00, 0x00, 0x1e, 0x16 },   //3 Broadwell GT2 (MacBook) (Intel HD Graphics 5300)
    { 0x00, 0x00, 0x26, 0x16 },   //4 Broadwell GT3 (MacBook Air) (Intel HD Graphics 6000)
    { 0x00, 0x00, 0x2b, 0x16 },   //5 Broadwell GT3 (MacBook Pro) (Intel Iris Graphics 6100)
    { 0x00, 0x00, 0x22, 0x16 },   //6 Broadwell GT3 (Intel Iris Pro Graphics 6200)
    { 0x01, 0x00, 0x0e, 0x16 },   //7 Broadwell GT1 (Intel HD Graphics)
    { 0x01, 0x00, 0x1e, 0x16 },   //8 Broadwell GT2 (MacBook) (Intel HD Graphics 5300)
    { 0x02, 0x00, 0x06, 0x16 },   //9 Broadwell GT1 (Intel HD Graphics)
    { 0x02, 0x00, 0x16, 0x16 },   //10 Broadwell GT2 (Intel HD Graphics 5500)
    { 0x02, 0x00, 0x26, 0x16 },   //11 Broadwell GT3 (MacBook Air) (Intel HD Graphics 6000)
    { 0x02, 0x00, 0x06, 0x16 },   //12 Broadwell GT3 (Intel Iris Pro Graphics 6200)
    { 0x02, 0x00, 0x2b, 0x16 },   //13 Broadwell GT3 (MacBook Pro) (Intel Iris Graphics 6100)
    { 0x03, 0x00, 0x12, 0x16 },   //14 Broadwell GT2 (Intel HD Graphics 5600)
    { 0x04, 0x00, 0x2b, 0x16 },   //15 Broadwell GT3 (MacBook Pro) (Intel Iris Graphics 6100)
    { 0x04, 0x00, 0x26, 0x16 },   //16 Broadwell GT3 (MacBook Air) (Intel HD Graphics 6000)
    { 0x05, 0x00, 0x26, 0x16 },   //17 Broadwell GT3 (MacBook Air) (Intel HD Graphics 6000)
    { 0x06, 0x00, 0x26, 0x16 },   //18 Broadwell GT3 (MacBook Air) (Intel HD Graphics 6000)
};

UINT8 broadwell_hd_vals[3][4] = {
    { 0x00, 0x00, 0x00, 0x00 },	  //0 "AAPL,gray-value"
    { 0x01, 0x00, 0x00, 0x00 },	  //1 "AAPL,gray-page"
    { 0x0c, 0x00, 0x00, 0x00 },	  //2 "graphics-options"
};


UINT8 skylake_ig_vals[12][4] = {
    { 0x00, 0x00, 0x1e, 0x19 },	  //0 Intel® HD Graphics 515
    { 0x00, 0x00, 0x16, 0x19 },	  //1 Intel® HD Graphics 520
    { 0x00, 0x00, 0x26, 0x19 },	  //2 Intel® Iris™ Graphics 550
    { 0x00, 0x00, 0x1b, 0x19 },	  //3 Intel® HD Graphics 530
    { 0x00, 0x00, 0x3b, 0x19 },	  //4 Intel® Iris™ Pro Graphics 580
    { 0x00, 0x00, 0x12, 0x19 },	  //5 Intel® HD Graphics 530
    { 0x02, 0x00, 0x16, 0x19 },	  //6 Intel® HD Graphics 520
    { 0x02, 0x00, 0x26, 0x19 },	  //7 Intel® Iris™ Graphics 540
    { 0x03, 0x00, 0x1e, 0x19 },	  //8 Intel® HD Graphics 515
    { 0x04, 0x00, 0x26, 0x19 },	  //9 Intel® Iris™ Graphics 540
    { 0x05, 0x00, 0x3b, 0x19 },	  //10 Intel® Iris™ Pro Graphics 580
    { 0x06, 0x00, 0x3b, 0x19 },	  //11 Intel® Iris™ Pro Graphics 580
};

UINT8 skylake_hd_vals[8][4] = {
    { 0x01, 0x00, 0x00, 0x00 },	  //0 "AAPL,Gfx324"
    { 0x01, 0x00, 0x00, 0x00 },	  //1 "AAPL,GfxYTile"
    { 0xfa, 0x00, 0x00, 0x00 },	  //2 "AAPL00,PanelCycleDelay"
    { 0x3c, 0x00, 0x00, 0x08 },	  //3 "AAPL00,PanelPowerDown"
    { 0x11, 0x00, 0x00, 0x00 },	  //4 "AAPL00,PanelPowerOff"
    { 0x19, 0x01, 0x00, 0x08 },	  //5 "AAPL00,PanelPowerOn"
    { 0x30, 0x00, 0x00, 0x00 },	  //6 "AAPL00,PanelPowerUp"
    { 0x0c, 0x00, 0x00, 0x00 },	  //7 "graphic-options"
};


UINT8 OsInfo[20]  = {
    0x30, 0x49, 0x01, 0x11, 0x01, 0x10, 0x08, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF
};

UINT8 HD2000_tbl_info[18] = {
    0x30, 0x44, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x02, 0x02, 0x02, 0x00, 0x01, 0x02, 0x02
};

UINT8 HD2000_os_info[20] = {
    0x30, 0x49, 0x01, 0x11, 0x11, 0x11, 0x08, 0x00, 0x00, 0x01,
    0xf0, 0x1f, 0x01, 0x00, 0x00, 0x00, 0x10, 0x07, 0x00, 0x00
};

// The following values came from a Sandy Bridge MacBook Air
UINT8 HD3000_tbl_info[18] = {
    0x30, 0x44, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00,
    0x00, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01
};

// The following values came from a Sandy Bridge MacBook Air
UINT8 HD3000_os_info[20] = {
    0x30, 0x49, 0x01, 0x12, 0x12, 0x12, 0x08, 0x00, 0x00, 0x01,
    0xf0, 0x1f, 0x01, 0x00, 0x00, 0x00, 0x10, 0x07, 0x00, 0x00
};


static struct gma_gpu_t KnownGPUS[] = {
  { 0x0000, "Intel Unknown"                  }, //
  { 0x2582, "GMA 915"                        }, //
  { 0x2592, "GMA 915"                        }, //
  { 0x27A2, "GMA 950"                        }, //
  { 0x27AE, "GMA 950"                        }, //
  //{ 0x27A6, "Mobile GMA950"                  }, // not a GPU
  { 0xA011, "Mobile GMA3150"                 }, //
  { 0xA012, "Mobile GMA3150"                 }, //
  { 0x2772, "Desktop GMA950"                 }, //
  { 0x29C2, "Desktop GMA3100"                }, //
  //{ 0x2776, "Desktop GMA950"                 }, // not a GPU
  //{ 0xA001, "Desktop GMA3150"                }, //
  { 0xA001, "Mobile GMA3150"                 }, //
  { 0xA002, "Desktop GMA3150"                }, //
  { 0x2A02, "GMAX3100"                       }, //
  { 0x2A03, "GMAX3100"                       }, // not a GPU
  { 0x2A12, "GMAX3100"                       }, //
  //{ 0x2A13, "GMAX3100"                       }, //
  { 0x2A42, "GMAX3100"                       }, //
  //{ 0x2A43, "GMAX3100"                       }, //


//----------------Ironlake-------------------
  { 0x0042, "Intel HD Graphics"              }, // Ironlake Desktop
  { 0x0046, "Intel HD Graphics"              }, // Ironlake Mobile


//----------------Sandybridge----------------
//GT1
  { 0x0102, "Intel HD Graphics 2000"         }, //
  { 0x0106, "Intel HD Graphics 3000"         }, //
  { 0x010A, "Intel HD Graphics P3000"        }, // Xeon E3 1200 v1, needs FakeID
//GT2
  { 0x0112, "Intel HD Graphics 3000"         }, //
  { 0x0116, "Intel HD Graphics 3000"         }, //
  { 0x0122, "Intel HD Graphics 3000"         }, //
  { 0x0126, "Intel HD Graphics 3000"         }, //


//----------------Ivybridge------------------
//GT1
  { 0x0152, "Intel HD Graphics 2500"         }, // iMac
  { 0x0156, "Intel HD Graphics 2500"         }, // MacBook
  { 0x015A, "Intel HD Graphics 4000"         }, //
//GT2
  { 0x0162, "Intel HD Graphics 4000"         }, // Desktop
  { 0x0166, "Intel HD Graphics 4000"         }, // MacBookPro10,1 have this string as model name whatever chameleon team may say
  { 0x016A, "Intel HD Graphics P4000"        }, // Xeon E3-1245
//GT3
  { 0x015E, "Intel HD Graphics"              }, //
//GT4
  { 0x0172, "Intel HD Graphics 2500"         }, //
//GT5
  { 0x0176, "Intel HD Graphics 2500"         }, //


//----------------Haswell--------------------
//GT1
  { 0x0402, "Intel Haswell Desktop"          }, //
  { 0x0406, "Intel Haswell Mobile"           }, //
  { 0x040A, "Intel Haswell Server"           }, //
  { 0x040B, "Intel Haswell"                  }, //
  { 0x040E, "Intel Haswell"                  }, //
//GT2
  { 0x0412, "Intel HD Graphics 4600"         }, //
  { 0x0416, "Intel HD Graphics 4600"         }, //
  { 0x041A, "Intel HD Graphics P4600"        }, //
  { 0x041B, "Intel Haswell"                  }, //
  { 0x041E, "Intel Intel HD Graphics 4400"   }, //
//GT3
  { 0x0422, "Intel Haswell Desktop"          }, //
  { 0x0426, "Intel Haswell Mobile"           }, //
  { 0x042A, "Intel Haswell Server"           }, //
  { 0x042B, "Intel Haswell"                  }, //
  { 0x042E, "Intel Haswell"                  }, //

//GT1
  { 0x0A02, "Intel Haswell Desktop"          }, //
  { 0x0A06, "Intel Haswell Mobile"           }, //
  { 0x0A0A, "Intel Haswell Server"           }, //
  { 0x0A0B, "Intel Haswell"                  }, //
  { 0x0A0E, "Intel Intel HD Graphics 4400"   }, //
//GT2
  { 0x0A12, "Intel Haswell Desktop"          }, //
  { 0x0A16, "Intel HD Graphics 4400"         }, //
  { 0x0A1A, "Intel Haswell Server"           }, //
  { 0x0A1B, "Intel Haswell"                  }, //
  { 0x0A1E, "Intel Intel HD Graphics 4400"   }, //
//GT3
  { 0x0A22, "Intel Haswell Desktop"          }, //
  { 0x0A26, "Intel HD Graphics 5000"         }, //
  { 0x0A2A, "Intel Haswell Server"           }, //
  { 0x0A2B, "Intel Haswell"                  }, //
  { 0x0A2E, "Intel Iris 5100"                }, // Haswell Intel Iris 5100 (i7-4558U, i7-4578U)

//GT1
  { 0x0C02, "Intel Haswell Desktop"          }, //
  { 0x0C06, "Intel Haswell Mobile"           }, //
  { 0x0C0A, "Intel Haswell Server"           }, //
  { 0x0C0B, "Intel Haswell"                  }, //
  { 0x0C0E, "Intel Haswell"                  }, //
//GT2
  { 0x0C12, "Intel Haswell Desktop"          }, //
  { 0x0C16, "Intel Haswell Mobile"           }, //
  { 0x0C1A, "Intel Haswell Server"           }, //
  { 0x0C1B, "Intel Haswell"                  }, //
  { 0x0C1E, "Intel Haswell"                  }, //
//GT3
  { 0x0C22, "Intel Haswell Desktop"          }, //
  { 0x0C26, "Intel Haswell Mobile"           }, //
  { 0x0C2A, "Intel Haswell Server"           }, //
  { 0x0C2B, "Intel Haswell"                  }, //
  { 0x0C2E, "Intel Haswell"                  }, //

//GT1
  { 0x0D02, "Intel Haswell Desktop"          }, //
  { 0x0D06, "Intel HD Graphics 5200"         }, //
  { 0x0D0A, "Intel Haswell Server"           }, //
  { 0x0D0B, "Intel Haswell"                  }, //
  { 0x0D0E, "Intel Haswell"                  }, //
//GT2
  { 0x0D12, "Intel HD Graphics 5200"         }, //
  { 0x0D16, "Intel HD Graphics 5200"         }, //
  { 0x0D1A, "Intel Haswell Server"           }, //
  { 0x0D1B, "Intel Haswell"                  }, //
  { 0x0D1E, "Intel Haswell"                  }, //
//GT3
  { 0x0D22, "Intel Iris Pro 5200"            }, //
  { 0x0D26, "Intel Iris Pro 5200"            }, // Haswell i7 4860HQ
  { 0x0D2A, "Intel Haswell"                  }, //
  { 0x0D2B, "Intel Haswell"                  }, //
  { 0x0D2E, "Intel Haswell"                  }, //


//----------------Broadwell------------------
//GT1
  { 0x1602, "Intel HD Graphics"              }, //
  { 0x1606, "Intel HD Graphics"              }, //
  { 0x160A, "Intel HD Graphics"              }, // Broadwell-U Integrated
  { 0x160B, "Intel HD Graphics"              }, // Broadwell-U Integrated
  { 0x160D, "Intel HD Graphics"              }, // Broadwell-U Integrated
  { 0x160E, "Intel HD Graphics"              }, // Broadwell-U Integrated
//GT2
  { 0x1612, "Intel HD Graphics 5600"         }, // Broadwell i7-5700HQ
  { 0x1616, "Intel HD Graphics 5500"         }, //
  { 0x161A, "Intel HD Graphics"              }, // Broadwell-U Integrated
  { 0x161B, "Intel HD Graphics"              }, // Broadwell-U Integrated
  { 0x161D, "Intel HD Graphics"              }, // Broadwell-U Integrated
  { 0x161E, "Intel HD Graphics 5300"         }, //
//GT3
  { 0x1622, "Intel Iris Pro Graphics 6200"   }, //
  { 0x1626, "Intel HD Graphics 6000"         }, //
  { 0x162A, "Intel Iris Pro Graphics P6300"  }, // Intel(R) Iris(TM) Pro Graphics
  { 0x162B, "Intel Iris Graphics 6100"       }, // Intel NUC 5i7RYH, i7-5557U
  { 0x162D, "Intel Iris Pro Graphics P6300"  }, //
  { 0x162E, "Intel HD Graphics"              }, // Broadwell-U Integrated Graphics
  { 0x1632, "Intel HD Graphics"              }, // Broadwell-U Integrated Graphics
  { 0x1636, "Intel HD Graphics"              }, // Broadwell-U Integrated Graphics
  { 0x163A, "Intel HD Graphics"              }, // Broadwell-U Integrated Graphics
  { 0x163B, "Intel HD Graphics"              }, // Broadwell-U Integrated Graphics
  { 0x163D, "Intel HD Graphics"              }, // Broadwell-U Integrated Graphics
  { 0x163E, "Intel HD Graphics"              }, // Broadwell-U Integrated Graphics


//----------------Skylake--------------------
//GT1
  { 0x1902, "Intel HD Graphics 510"          }, //
  { 0x1906, "Intel HD Graphics 510"          }, //
  { 0x190A, "Intel Skylake GT1"              }, //
  { 0x190B, "Intel HD Graphics 510"          }, //
  { 0x190E, "Intel Skylake GT1"              }, //
//GT2
  { 0x1912, "Intel HD Graphics 530"          }, //
  { 0x1913, "Intel Skylake GT2f"             }, //
  { 0x1915, "Intel Skylake GT2f"             }, //
  { 0x1916, "Intel HD Graphics 520"          }, //
  { 0x1917, "Intel Skylake GT2f"             }, //
  { 0x191A, "Intel Skylake GT2"              }, //
  { 0x191B, "Intel HD Graphics 530"          }, //
  { 0x191D, "Intel HD Graphics P530"         }, //
  { 0x191E, "Intel HD Graphics 515"          }, //
  { 0x1921, "Intel Skylake GT2"              }, //
//GT3
  { 0x1923, "Intel HD Graphics 535"          }, //
//GT3e
  { 0x1926, "Intel Iris Graphics 540"        }, //
  { 0x1927, "Intel Iris Graphics 550"        }, //
  { 0x192A, "Intel Skylake GT4"              }, //
  { 0x192B, "Intel Iris Graphics 555"        }, //
  { 0x192D, "Intel Iris Graphics P555"       }, //
//GT4e
  { 0x1932, "Intel Iris Pro Graphics 580"    }, //
  { 0x193A, "Intel Iris Pro Graphics P580"   }, //
  { 0x193B, "Intel Iris Pro Graphics 580"    }, //
  { 0x193D, "Intel Iris Pro Graphics P580"   }, //


//----------------Kabylake-------------------
//GT1
  { 0x5902, "Intel HD Graphics 610"          }, //
  { 0x5906, "Intel HD Graphics 610"          }, //
  { 0x590A, "Intel Kabylake GT1"             }, //
  { 0x5908, "Intel Kabylake GT1"             }, //
  { 0x590B, "Intel HD Graphics 610"          }, //
  { 0x590E, "Intel Kabylake GT1"             }, //
//GT1.5
  { 0x5913, "Intel Kabylake GT1.5"           }, //
  { 0x5915, "Intel Kabylake GT1.5"           }, //
  { 0x5917, "Intel Kabylake GT1.5"           }, //
//GT2
  { 0x5912, "Intel HD Graphics 630"          }, //
  { 0x5916, "Intel HD Graphics 620"          }, //
  { 0x591A, "Intel Kabylake GT2"             }, //
  { 0x591B, "Intel HD Graphics 630"          }, //
  { 0x591D, "Intel HD Graphics P630"         }, //
  { 0x591E, "Intel HD Graphics 615"          }, //
//GTF2
  { 0x5921, "Intel Kabylake GT2F"            }, //
//GT3
  { 0x5923, "Intel HD Graphics 635"          }, //
  { 0x5926, "Intel Iris Pro Graphics 640"    }, //
  { 0x5927, "Intel Iris Pro Graphics 650"    }, //
//GT4
  { 0x593B, "Intel Kabylake GT4"             }, //
};


CHAR8 *get_gma_model(UINT16 id)
{
	INT32 i;
	
	for (i = 0; i < (sizeof(KnownGPUS) / sizeof(KnownGPUS[0])); i++)
	{
		if (KnownGPUS[i].device == id)
			return KnownGPUS[i].name;
	}
	return KnownGPUS[0].name;
}


/*
// getVBEVideoRam - need to find workaround
UINT32 getVBEVideoRam()
{
    VESA_BIOS_EXTENSIONS_INFORMATION_BLOCK vbeInfo;
    int err, small;
    
    bzero(&vbeInfo, sizeof(vbeInfo));
    StrCpy((char *)&vbeInfo, "VBE2");
    err = getVBEInfo( &vbeInfo );
    if (err != 0)
        return 0;
    
    if ( StrnCmp((char *)vbeInfo.VESASignature, "VESA", 4))
        return 0;
    
    small = (vbeInfo.TotalMemory < 16);
    
    return vbeInfo.TotalMemory * 64 * 1024;
}
*/


BOOLEAN setup_gma_devprop(pci_dt_t *gma_dev)
{
  UINTN           j;
  INT32           i;
  CHAR8           *devicepath;
  CHAR8           *model;
  DevPropDevice   *device;
	UINT8           BuiltIn = 0x00;
	UINT16			device_id = gma_dev->device_id;
	UINT32          DualLink = 0; // local variable must be inited
  //UINT32          ram = (((getVBEVideoRam() + 512) / 1024) + 512) / 1024;
  BOOLEAN         Injected = FALSE;
  BOOLEAN         SetSnb = FALSE;
  
  MACHINE_TYPES   MacModel = GetModelFromString(gSettings.ProductName);
  
	devicepath = get_pci_dev_path(gma_dev);
  
	model = get_gma_model(gma_dev->device_id);
  for (j = 0; j < NGFX; j++) {
    if ((gGraphics[j].Vendor == Intel) &&
        (gGraphics[j].DeviceID == gma_dev->device_id)) {
      model = gGraphics[j].Model;
      break;
    }
  }
  //DBG("Finally model=%a\n", model);
	
  DBG("%a [%04x:%04x] :: %a\n",
      model, gma_dev->vendor_id, gma_dev->device_id, devicepath);
	
  if (!string) {
    string = devprop_create_string();
  }
	
  //device = devprop_add_device(string, devicepath); //AllocatePool inside
  device = devprop_add_device_pci(string, gma_dev);
	
  if (!device) {
    DBG("Failed initializing dev-prop string dev-entry.\n");
    //pause();
    return FALSE;
  }
  
  if (gSettings.NrAddProperties != 0xFFFE) {
    for (i = 0; i < gSettings.NrAddProperties; i++) {
      if (gSettings.AddProperties[i].Device != DEV_INTEL) {
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
    DBG("custom IntelGFX properties injected, continue\n");
  }
  
  if (gSettings.FakeIntel) {
    UINT32 FakeID = gSettings.FakeIntel >> 16;
    devprop_add_value(device, "device-id", (UINT8*)&FakeID, 4);
    FakeID = gSettings.FakeIntel & 0xFFFF;
    devprop_add_value(device, "vendor-id", (UINT8*)&FakeID, 4);
    DBG("  FakeIntel = 0x%08lx\n", gSettings.FakeIntel);
  }
  
  if (gSettings.UseIntelHDMI) {
    devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
  }
  
  if (gSettings.InjectEDID && gSettings.CustomEDID) {
    devprop_add_value(device, "AAPL00,override-no-connect", gSettings.CustomEDID, 128);
  }
  
  if (gSettings.IgPlatform != 0) {
    if (gma_dev->device_id < 0x130) {
      devprop_add_value(device, "AAPL,snb-platform-id",	(UINT8*)&gSettings.IgPlatform, 4);
    } else {
      devprop_add_value(device, "AAPL,ig-platform-id", (UINT8*)&gSettings.IgPlatform, 4);
    }
    SetSnb = TRUE;
    DBG("  ig-platform-id = %08lx\n", gSettings.IgPlatform);
  }
  
  if (gSettings.DualLink != 0) {
    devprop_add_value(device, "AAPL00,DualLink", (UINT8*)&gSettings.DualLink, 1);
  }
  
  if (gSettings.NoDefaultProperties) {
    DBG("Intel: no default properties\n");
    return TRUE;
  }
  
  devprop_add_value(device, "model", (UINT8*)model, (UINT32)AsciiStrLen(model));
  devprop_add_value(device, "device_type", (UINT8*)"display", 7);
  devprop_add_value(device, "subsystem-vendor-id", common_vals[2], 4);
  devprop_add_value(device, "class-code",	(UINT8*)ClassFix, 4);
  
  
  // if not set ig-platform-id
  if (!SetSnb) {
    DBG("  Not set ig-platform-id. Automatically detect it or use ACPI injection\n");
    /*
     1. ACPI injection is first
     2. if there is no ig-platform-id in ACPI, Clover will automatically detect ig-platform-id.
     */
    switch (gma_dev->device_id) {
      case 0x2582: // GMA 915"                         //
      case 0x2592: // GMA 915"                         //
      case 0x27A2: // GMA 950"                         //
      case 0x27AE: // GMA 950"                         //
        //case 0x27A6: // Mobile GMA950"                   // not a GPU
        devprop_add_value(device, "AAPL,HasPanel", common_vals[1], 4);
        devprop_add_value(device, "built-in", &BuiltIn, 1);
        break;
      case 0xA011: // Mobile GMA3150"                  //
      case 0xA012: // Mobile GMA3150"                  //
      case 0x2772: // Desktop GMA950"                  //
      case 0x29C2: // Desktop GMA3100"                 //
        //case 0x2776: // Desktop GMA950"                  // not a GPU
        //case 0xA001: // Desktop GMA3150"                 //
      case 0xA001: // Mobile GMA3150"                  //
      case 0xA002: // Desktop GMA3150"                 //
        devprop_add_value(device, "built-in", &BuiltIn, 1);
        devprop_add_value(device, "AAPL00,DualLink", (UINT8*)&DualLink, 1);
        devprop_add_value(device, "AAPL,os-info", (UINT8*)&OsInfo, sizeof(OsInfo));
        break;
      case 0x2A02: // GMAX3100"                        //
      case 0x2A03: // GMAX3100"                        // not a GPU
      case 0x2A12: // GMAX3100"                        //
        //case 0x2A13: // GMAX3100"                        //
      case 0x2A42: // GMAX3100"                        //
        //case 0x2A43: // GMAX3100"                        //
        devprop_add_value(device, "AAPL,HasPanel", GMAX3100_vals[0], 4);
        devprop_add_value(device, "AAPL,SelfRefreshSupported", GMAX3100_vals[1], 4);
        devprop_add_value(device, "AAPL,aux-power-connected", GMAX3100_vals[2], 4);
        devprop_add_value(device, "AAPL,backlight-control", GMAX3100_vals[3], 4);
        devprop_add_value(device, "AAPL00,blackscreen-preferences", GMAX3100_vals[4], 4);
        devprop_add_value(device, "AAPL01,BacklightIntensity", GMAX3100_vals[5], 4);
        devprop_add_value(device, "AAPL01,blackscreen-preferences", GMAX3100_vals[6], 4);
        devprop_add_value(device, "AAPL01,DataJustify", GMAX3100_vals[7], 4);
        //devprop_add_value(device, "AAPL01,Depth", GMAX3100_vals[8], 4);
        devprop_add_value(device, "AAPL01,Dither", GMAX3100_vals[9], 4);
        devprop_add_value(device, "AAPL01,DualLink", (UINT8 *)&DualLink, 1);
        //devprop_add_value(device, "AAPL01,Height", GMAX3100_vals[10], 4);
        devprop_add_value(device, "AAPL01,Interlace", GMAX3100_vals[11], 4);
        devprop_add_value(device, "AAPL01,Inverter", GMAX3100_vals[12], 4);
        devprop_add_value(device, "AAPL01,InverterCurrent", GMAX3100_vals[13], 4);
        //devprop_add_value(device, "AAPL01,InverterCurrency", GMAX3100_vals[15], 4);
        devprop_add_value(device, "AAPL01,LinkFormat", GMAX3100_vals[14], 4);
        devprop_add_value(device, "AAPL01,LinkType", GMAX3100_vals[15], 4);
        devprop_add_value(device, "AAPL01,Pipe", GMAX3100_vals[16], 4);
        //devprop_add_value(device, "AAPL01,PixelFormat", GMAX3100_vals[17], 4);
        devprop_add_value(device, "AAPL01,Refresh", GMAX3100_vals[18], 4);
        devprop_add_value(device, "AAPL01,Stretch", GMAX3100_vals[19], 4);
        devprop_add_value(device, "AAPL01,InverterFrequency", GMAX3100_vals[20], 4);
        //devprop_add_value(device, "subsystem-vendor-id", GMAX3100_vals[21], 4);
        devprop_add_value(device, "subsystem-id", GMAX3100_vals[22], 4);
        devprop_add_value(device, "built-in", &BuiltIn, 1);
        break;
        
        
        //----------------Ironlake-------------------
      case 0x0042: // "Intel HD Graphics"               // Ironlake Desktop
      case 0x0046: // "Intel HD Graphics"               // Ironlake Mobile
        devprop_add_value(device, "AAPL,os-info", (UINT8*)&OsInfo, sizeof(OsInfo));
        devprop_add_value(device, "VRAM,totalsize", arrandale_vals[0], 4);
        //devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
        break;
        
        
        //----------------Sandybridge----------------
      case 0x0102: // "Intel HD Graphics 2000"          //
        devprop_add_value(device, "built-in", &BuiltIn, 1);
        devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
        devprop_add_value(device, "AAPL00,PixelFormat", sandy_bridge_hd_vals[0], 4);
        devprop_add_value(device, "AAPL00,T1", sandy_bridge_hd_vals[1], 4);
        devprop_add_value(device, "AAPL00,T2", sandy_bridge_hd_vals[2], 4);
        devprop_add_value(device, "AAPL00,T3", sandy_bridge_hd_vals[3], 4);
        devprop_add_value(device, "AAPL00,T4", sandy_bridge_hd_vals[4], 4);
        devprop_add_value(device, "AAPL00,T5", sandy_bridge_hd_vals[5], 4);
        devprop_add_value(device, "AAPL00,T6", sandy_bridge_hd_vals[6], 4);
        devprop_add_value(device, "AAPL00,T7", sandy_bridge_hd_vals[7], 4);
        devprop_add_value(device, "AAPL00,LinkType", sandy_bridge_hd_vals[8], 4);
        devprop_add_value(device, "AAPL00,LinkFormat", sandy_bridge_hd_vals[9], 4);
        devprop_add_value(device, "AAPL00,DualLink", sandy_bridge_hd_vals[10], 4);
        devprop_add_value(device, "AAPL00,Dither", sandy_bridge_hd_vals[11], 4);
        devprop_add_value(device, "AAPL00,DataJustify", sandy_bridge_hd_vals[12], 4);
        devprop_add_value(device, "graphic-options", sandy_bridge_hd_vals[13], 4);
        devprop_add_value(device, "AAPL,tbl-info", HD2000_tbl_info, 18);
        devprop_add_value(device, "AAPL,os-info", HD2000_os_info, 20);
        break;
      case 0x0106: // "Intel HD Graphics 3000"          //
      case 0x010A: // "Intel HD Graphics P3000"         // Xeon E3 1200 v1, needs FakeID
      case 0x0112: // "Intel HD Graphics 3000"          //
      case 0x0122: // "Intel HD Graphics 3000"          //
        if ((MacModel == MacBookPro81) || (MacModel == MacBookPro82)){
          devprop_add_value(device, "AAPL,snb-platform-id", sandy_bridge_snb_vals[0], 4);
        }
        else if (MacModel == MacBookPro83){
          devprop_add_value(device, "AAPL,snb-platform-id", sandy_bridge_snb_vals[1], 4);
        }
        else if (MacModel == MacMini51){
          devprop_add_value(device, "AAPL,snb-platform-id", sandy_bridge_snb_vals[2], 4);
        }
        else if (MacModel == MacMini52){
          devprop_add_value(device, "AAPL,snb-platform-id", sandy_bridge_snb_vals[3], 4);
        }
        else if (MacModel == MacBookAir41){
          devprop_add_value(device, "AAPL,snb-platform-id", sandy_bridge_snb_vals[4], 4);
        }
        device_id = 0x00000126;
        devprop_add_value(device, "device-id", (UINT8*)&device_id, sizeof(device_id));
        devprop_add_value(device, "built-in", &BuiltIn, 1);
        devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
        devprop_add_value(device, "AAPL,tbl-info", HD3000_tbl_info, 18);
        devprop_add_value(device, "AAPL,os-info", HD3000_os_info, 20);
        break;
      case 0x0116: // "Intel HD Graphics 3000"          //
      case 0x0126: // "Intel HD Graphics 3000"          //
          if ((MacModel == MacBookPro81) || (MacModel == MacBookPro82)){
            devprop_add_value(device, "AAPL,snb-platform-id", sandy_bridge_snb_vals[0], 4);
          }
          else if (MacModel == MacBookPro83){
            devprop_add_value(device, "AAPL,snb-platform-id", sandy_bridge_snb_vals[1], 4);
          }
          else if (MacModel == MacMini51){
            devprop_add_value(device, "AAPL,snb-platform-id", sandy_bridge_snb_vals[2], 4);
          }
          else if (MacModel == MacMini52){
            devprop_add_value(device, "AAPL,snb-platform-id", sandy_bridge_snb_vals[3], 4);
          }
          else if (MacModel == MacBookAir41){
            devprop_add_value(device, "AAPL,snb-platform-id", sandy_bridge_snb_vals[4], 4);
          }
          devprop_add_value(device, "built-in", &BuiltIn, 1);
          devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
          devprop_add_value(device, "AAPL00,PixelFormat", sandy_bridge_hd_vals[0], 4);
          devprop_add_value(device, "AAPL00,T1", sandy_bridge_hd_vals[1], 4);
          devprop_add_value(device, "AAPL00,T2", sandy_bridge_hd_vals[2], 4);
          devprop_add_value(device, "AAPL00,T3", sandy_bridge_hd_vals[3], 4);
          devprop_add_value(device, "AAPL00,T4", sandy_bridge_hd_vals[4], 4);
          devprop_add_value(device, "AAPL00,T5", sandy_bridge_hd_vals[5], 4);
          devprop_add_value(device, "AAPL00,T6", sandy_bridge_hd_vals[6], 4);
          devprop_add_value(device, "AAPL00,T7", sandy_bridge_hd_vals[7], 4);
          devprop_add_value(device, "AAPL00,LinkType", sandy_bridge_hd_vals[8], 4);
          devprop_add_value(device, "AAPL00,LinkFormat", sandy_bridge_hd_vals[9], 4);
          devprop_add_value(device, "AAPL00,DualLink", sandy_bridge_hd_vals[10], 4);
          devprop_add_value(device, "AAPL00,Dither", sandy_bridge_hd_vals[11], 4);
          devprop_add_value(device, "AAPL00,DataJustify", sandy_bridge_hd_vals[12], 4);
          devprop_add_value(device, "graphic-options", sandy_bridge_hd_vals[13], 4);
          devprop_add_value(device, "AAPL,tbl-info", HD3000_tbl_info, 18);
          devprop_add_value(device, "AAPL,os-info", HD3000_os_info, 20);
          break;
        
        //----------------Ivybridge------------------
      case 0x0152: // "Intel HD Graphics 2500"          // iMac
      case 0x0156: // "Intel HD Graphics 2500"          // MacBook
      case 0x015A: // "Intel HD Graphics 4000"          //
      case 0x0162: // "Intel HD Graphics 4000"          // Desktop
      case 0x0166: // "Intel HD Graphics 4000"          // MacBookPro10,1 have this string as model name whatever chameleon team may say
      case 0x016A: // "Intel HD Graphics P4000"         // Xeon E3-1245
      case 0x015E: // "Intel HD Graphics"               //
      case 0x0172: // "Intel HD Graphics 2500"          //
      case 0x0176: // "Intel HD Graphics 2500"          //
          // until take a vram size, use ig-platform-id "01660004"
          device_id = 0x00000166;
          devprop_add_value(device, "device-id", (UINT8*)&device_id, sizeof(device_id));
          devprop_add_value(device, "AAPL,ig-platform-id", ivy_bridge_ig_vals[4], 4);
          devprop_add_value(device, "built-in", &BuiltIn, 1);
          devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
          devprop_add_value(device, "AAPL00,PixelFormat", ivy_bridge_hd_vals[0], 4);
          devprop_add_value(device, "AAPL00,T1", ivy_bridge_hd_vals[1], 4);
          devprop_add_value(device, "AAPL00,T2", ivy_bridge_hd_vals[2], 4);
          devprop_add_value(device, "AAPL00,T3", ivy_bridge_hd_vals[3], 4);
          devprop_add_value(device, "AAPL00,T4", ivy_bridge_hd_vals[4], 4);
          devprop_add_value(device, "AAPL00,T5", ivy_bridge_hd_vals[5], 4);
          devprop_add_value(device, "AAPL00,T6", ivy_bridge_hd_vals[6], 4);
          devprop_add_value(device, "AAPL00,T7", ivy_bridge_hd_vals[7], 4);
          devprop_add_value(device, "AAPL00,LinkType", ivy_bridge_hd_vals[8], 4);
          devprop_add_value(device, "AAPL00,LinkFormat", ivy_bridge_hd_vals[9], 4);
          devprop_add_value(device, "AAPL00,DualLink", ivy_bridge_hd_vals[10], 4);
          devprop_add_value(device, "AAPL00,Dither", ivy_bridge_hd_vals[11], 4);
          devprop_add_value(device, "AAPL,gray-value", ivy_bridge_hd_vals[12], 4);
          devprop_add_value(device, "AAPL,gray-page", ivy_bridge_hd_vals[13], 4);
          devprop_add_value(device, "graphic-options", ivy_bridge_hd_vals[14], 4);
          break;
          // got idea from Chameleon. need to take a vram size
          /*
           if ((gma_dev->device_id >= 0x0152) && (gma_dev->device_id <= 0x0176)){
           switch (ram)
           {
           DBG("%dMB RAM in the bios\n", ram);
           case 96:
           devprop_add_value(device, "AAPL,ig-platform-id", ivy_bridge_ig_vals[0], 4);
           case 64:
           devprop_add_value(device, "AAPL,ig-platform-id", ivy_bridge_ig_vals[3], 4);
           case 32:
           devprop_add_value(device, "AAPL,ig-platform-id", ivy_bridge_ig_vals[4], 4);
           }
           devprop_add_value(device, "built-in", &BuiltIn, 1);
           devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
           devprop_add_value(device, "AAPL00,PixelFormat", ivy_bridge_hd_vals[0], 4);
           devprop_add_value(device, "AAPL00,T1", ivy_bridge_hd_vals[1], 4);
           devprop_add_value(device, "AAPL00,T2", ivy_bridge_hd_vals[2], 4);
           devprop_add_value(device, "AAPL00,T3", ivy_bridge_hd_vals[3], 4);
           devprop_add_value(device, "AAPL00,T4", ivy_bridge_hd_vals[4], 4);
           devprop_add_value(device, "AAPL00,T5", ivy_bridge_hd_vals[5], 4);
           devprop_add_value(device, "AAPL00,T6", ivy_bridge_hd_vals[6], 4);
           devprop_add_value(device, "AAPL00,T7", ivy_bridge_hd_vals[7], 4);
           devprop_add_value(device, "AAPL00,LinkType", ivy_bridge_hd_vals[8], 4);
           devprop_add_value(device, "AAPL00,LinkFormat", ivy_bridge_hd_vals[9], 4);
           devprop_add_value(device, "AAPL00,DualLink", ivy_bridge_hd_vals[10], 4);
           devprop_add_value(device, "AAPL00,Dither", ivy_bridge_hd_vals[11], 4);
           devprop_add_value(device, "AAPL,gray-value", ivy_bridge_hd_vals[12], 4);
           devprop_add_value(device, "AAPL,gray-page", ivy_bridge_hd_vals[13], 4);
           devprop_add_value(device, "graphic-options", ivy_bridge_hd_vals[14], 4);
           break;
           }
           */
        
        //----------------Haswell--------------------
      case 0x0402: // "Intel Haswell Desktop"           //
      case 0x0406: // "Intel Haswell Mobile"            //
        devprop_add_value(device, "AAPL,ig-platform-id", haswell_ig_vals[0], 4);
        goto A42E;
      case 0x040A: // "Intel Haswell Server"            //
      case 0x040B: // "Intel Haswell"                   //
      case 0x040E: // "Intel Haswell"                   //
      case 0x0412: // "Intel HD Graphics 4600"          //
        devprop_add_value(device, "AAPL,ig-platform-id", haswell_ig_vals[11], 4);
        goto A42E;
      case 0x0416: // "Intel HD Graphics 4600"          //
        devprop_add_value(device, "AAPL,ig-platform-id", haswell_ig_vals[13], 4);
        goto A42E;
      case 0x041A: // "Intel HD Graphics P4600"         //
      case 0x041B: // "Intel Haswell"                   //
      case 0x041E: // "Intel HD Graphics 4400"          //
      case 0x0422: // "Intel Haswell Desktop"           //
      case 0x0426: // "Intel Haswell Mobile"            //
        devprop_add_value(device, "AAPL,ig-platform-id", haswell_ig_vals[5], 4);
        goto A42E;
      case 0x042A: // "Intel Haswell Server"            //
      case 0x042B: // "Intel Haswell"                   //
      case 0x042E: // "Intel Haswell"                   //
        devprop_add_value(device, "AAPL,ig-platform-id", haswell_ig_vals[6], 4);
      A42E:
        devprop_add_value(device, "built-in", &BuiltIn, 1);
        devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
        devprop_add_value(device, "AAPL,gray-value", haswell_hd_vals[0], 4);
        devprop_add_value(device, "graphic-options", haswell_hd_vals[1], 4);
        break;
      case 0x0A02: // "Intel Haswell Desktop"           //
      case 0x0A06: // "Intel Haswell Mobile"            //
      case 0x0A0A: // "Intel Haswell Server"            //
      case 0x0A0B: // "Intel Haswell"                   //
      case 0x0A0E: // "Intel HD Graphics 4400"          //
      case 0x0A12: // "Intel Haswell Desktop"           //
      case 0x0A16: // "Intel HD Graphics 4400"          //
      case 0x0A1A: // "Intel Haswell Server"            //
      case 0x0A1B: // "Intel Haswell"                   //
      case 0x0A1E: // "Intel HD Graphics 4400"          //
      case 0x0A22: // "Intel Haswell Desktop"           //
      case 0x0A26: // "Intel HD Graphics 5000"          //
      case 0x0A2A: // "Intel Haswell Server"            //
      case 0x0A2B: // "Intel Haswell"                   //
      case 0x0A2E: // "Intel Iris 5100"                 // Intel Haswell Intel Iris 5100 (i7-4558U, i7-4578U)
        devprop_add_value(device, "device-id", (UINT8*)&device_id, sizeof(device_id));
        devprop_add_value(device, "AAPL,ig-platform-id", haswell_ig_vals[13], 4);
        devprop_add_value(device, "built-in", &BuiltIn, 1);
        devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
        devprop_add_value(device, "AAPL,gray-value", haswell_hd_vals[0], 4);
        devprop_add_value(device, "graphic-options", haswell_hd_vals[1], 4);
        break;
      case 0x0C02: // "Intel Haswell Desktop"           //
      case 0x0C06: // "Intel Haswell Mobile"            //
        devprop_add_value(device, "AAPL,ig-platform-id", haswell_ig_vals[1], 4);
        goto C2E;
      case 0x0C0A: // "Intel Haswell Server"            //
      case 0x0C0B: // "Intel Haswell"                   //
      case 0x0C0E: // "Intel Haswell"                   //
      case 0x0C12: // "Intel Haswell Desktop"           //
      case 0x0C16: // "Intel Haswell Mobile"            //
        devprop_add_value(device, "AAPL,ig-platform-id", haswell_ig_vals[4], 4);
        goto C2E;
      case 0x0C1A: // "Intel Haswell Server"            //
      case 0x0C1B: // "Intel Haswell"                   //
      case 0x0C1E: // "Intel Haswell"                   //
      case 0x0C22: // "Intel Haswell Desktop"           //
      case 0x0C26: // "Intel Haswell Mobile"            //
        devprop_add_value(device, "AAPL,ig-platform-id", haswell_ig_vals[7], 4);
        goto C2E;
      case 0x0C2A: // "Intel Haswell Server"            //
      case 0x0C2B: // "Intel Haswell"                   //
      case 0x0C2E: // "Intel Haswell"                   //
        devprop_add_value(device, "AAPL,ig-platform-id", haswell_ig_vals[6], 4);
      C2E:
        devprop_add_value(device, "built-in", &BuiltIn, 1);
        devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
        devprop_add_value(device, "AAPL,gray-value", haswell_hd_vals[0], 4);
        devprop_add_value(device, "graphic-options", haswell_hd_vals[1], 4);
        break;
      case 0x0D02: // "Intel Haswell Desktop"           //
      case 0x0D06: // "Intel HD Graphics 5200"          //
      case 0x0D0A: // "Intel Haswell Server"            //
      case 0x0D0B: // "Intel Haswell"                   //
      case 0x0D0E: // "Intel Haswell"                   //
      case 0x0D12: // "Intel HD Graphics 5200"          //
      case 0x0D16: // "Intel HD Graphics 5200"          //
      case 0x0D1A: // "Intel Haswell Server"            //
      case 0x0D1B: // "Intel Haswell"                   //
      case 0x0D1E: // "Intel Haswell"                   //
      case 0x0D22: // "Intel Iris Pro 5200"             //
      case 0x0D26: // "Intel Iris Pro 5200"             // Intel Haswell i7 4860HQ
      case 0x0D2A: // "Intel Haswell"                   //
      case 0x0D2B: // "Intel Haswell"                   //
      case 0x0D2E: // "Intel Haswell"                   //
          //device_id = 0x00000a26;
          devprop_add_value(device, "device-id", (UINT8*)&device_id, sizeof(device_id));
          devprop_add_value(device, "AAPL,ig-platform-id", haswell_ig_vals[13], 4);
          devprop_add_value(device, "built-in", &BuiltIn, 1);
          devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
          devprop_add_value(device, "AAPL,gray-value", haswell_hd_vals[0], 4);
          devprop_add_value(device, "graphic-options", haswell_hd_vals[1], 4);
          break;
        
        //----------------Broadwell------------------
      case 0x1602: // "Intel HD Graphics"               //
      case 0x1606: // "Intel HD Graphics"               //
      case 0x160A: // "Intel HD Graphics"               // Broadwell-U Integrated
      case 0x160B: // "Intel HD Graphics"               // Broadwell-U Integrated
      case 0x160D: // "Intel HD Graphics"               // Broadwell-U Integrated
      case 0x160E: // "Intel HD Graphics"               // Broadwell-U Integrated
      case 0x1612: // "Intel HD Graphics 5600"          // Broadwell i7-5700HQ
        devprop_add_value(device, "AAPL,ig-platform-id", broadwell_ig_vals[14], 4);
        goto A163E;
      case 0x1616: // "Intel HD Graphics 5500"          //
        devprop_add_value(device, "AAPL,ig-platform-id", broadwell_ig_vals[10], 4);
        goto A163E;
      case 0x161A: // "Intel HD Graphics"               // Broadwell-U Integrated
      case 0x161B: // "Intel HD Graphics"               // Broadwell-U Integrated
      case 0x161D: // "Intel HD Graphics"               // Broadwell-U Integrated
      case 0x161E: // "Intel HD Graphics 5300"          //
        devprop_add_value(device, "AAPL,ig-platform-id", broadwell_ig_vals[3], 4);
        goto A163E;
      case 0x1622: // "Intel Iris Pro Graphics 6200"    //
        devprop_add_value(device, "AAPL,ig-platform-id", broadwell_ig_vals[12], 4);
        goto A163E;
      case 0x1626: // "Intel HD Graphics 6000"          //
        devprop_add_value(device, "AAPL,ig-platform-id", broadwell_ig_vals[16], 4);
        goto A163E;
      case 0x162A: // "Intel Iris Pro Graphics P6300"   // Intel(R) Iris(TM) Pro Graphics
      case 0x162B: // "Intel Iris Graphics 6100"        // Intel NUC 5i7RYH, i7-5557U
        devprop_add_value(device, "AAPL,ig-platform-id", broadwell_ig_vals[5], 4);
        goto A163E;
      case 0x162D: // "Intel Iris Pro Graphics P6300"   //
      case 0x162E: // "Intel HD Graphics"               // Broadwell-U Integrated Graphics
      case 0x1632: // "Intel HD Graphics"               // Broadwell-U Integrated Graphics
      case 0x1636: // "Intel HD Graphics"               // Broadwell-U Integrated Graphics
      case 0x163A: // "Intel HD Graphics"               // Broadwell-U Integrated Graphics
      case 0x163B: // "Intel HD Graphics"               // Broadwell-U Integrated Graphics
      case 0x163D: // "Intel HD Graphics"               // Broadwell-U Integrated Graphics
      case 0x163E: // "Intel HD Graphics"               // Broadwell-U Integrated Graphics
        if ((gma_dev->device_id == 0x162A) || (gma_dev->device_id == 0x162D)){
          device_id = 0x00001622;
          devprop_add_value(device, "device-id", (UINT8*)&device_id, sizeof(device_id));
          devprop_add_value(device, "AAPL,ig-platform-id", broadwell_ig_vals[12], 4);
        } else {
          devprop_add_value(device, "AAPL,ig-platform-id", broadwell_ig_vals[15], 4);
        }
      A163E:
        devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
        devprop_add_value(device, "AAPL,gray-value", broadwell_hd_vals[0], 4);
        devprop_add_value(device, "AAPL,gray-page", broadwell_hd_vals[1], 4);
        devprop_add_value(device, "graphic-options", broadwell_hd_vals[2], 4);
        break;
        
        //----------------Skylake--------------------
      case 0x1902: // "Intel HD Graphics 510"           //
      case 0x1906: // "Intel HD Graphics 510"           //
      case 0x190A: // "Intel Skylake GT1"               //
      case 0x190B: // "Intel HD Graphics 510"           //
      case 0x190E: // "Intel Skylake GT1"               //
      case 0x1912: // "Intel HD Graphics 530"           //
        devprop_add_value(device, "AAPL,ig-platform-id", skylake_ig_vals[5], 4);
        goto A193D;
      case 0x1913: // "Intel Skylake GT2f"              //
      case 0x1915: // "Intel Skylake GT2f"              //
      case 0x1916: // "Intel HD Graphics 520"           //
        devprop_add_value(device, "AAPL,ig-platform-id", skylake_ig_vals[1], 4);
        goto A193D;
      case 0x1917: // "Intel Skylake GT2f"              //
      case 0x191A: // "Intel Skylake GT2"               //
      case 0x191B: // "Intel HD Graphics 530"           //
        devprop_add_value(device, "AAPL,ig-platform-id", skylake_ig_vals[3], 4);
        goto A193D;
      case 0x191D: // "Intel HD Graphics P530"          //
      case 0x191E: // "Intel HD Graphics 515"           //
        devprop_add_value(device, "AAPL,ig-platform-id", skylake_ig_vals[0], 4);
        goto A193D;
      case 0x1921: // "Intel Skylake GT2"               //
      case 0x1923: // "Intel HD Graphics 535"           //
      case 0x1926: // "Intel Iris Graphics 540"         //
        devprop_add_value(device, "AAPL,ig-platform-id", skylake_ig_vals[7], 4);
        goto A193D;
      case 0x1927: // "Intel Iris Graphics 550"         //
        devprop_add_value(device, "AAPL,ig-platform-id", skylake_ig_vals[2], 4);
        goto A193D;
      case 0x192A: // "Intel Skylake GT4"               //
      case 0x192B: // "Intel Iris Graphics 555"         //
      case 0x192D: // "Intel Iris Graphics P555"        //
      case 0x1932: // "Intel Iris Pro Graphics 580"     //
      case 0x193A: // "Intel Iris Pro Graphics P580"    //
      case 0x193B: // "Intel Iris Pro Graphics 580"     //        
      case 0x193D: // "Intel Iris Pro Graphics P580"    //
        if ((gma_dev->device_id == 0x191D) || (gma_dev->device_id == 0x1923)){
          device_id = 0x00001912;
          devprop_add_value(device, "device-id", (UINT8*)&device_id, sizeof(device_id));
          devprop_add_value(device, "AAPL,ig-platform-id", skylake_ig_vals[5], 4);
        }
        else if ((gma_dev->device_id == 0x192B) || (gma_dev->device_id == 0x192D)){
          device_id = 0x00001927;
          devprop_add_value(device, "device-id", (UINT8*)&device_id, sizeof(device_id));
          devprop_add_value(device, "AAPL,ig-platform-id", skylake_ig_vals[2], 4);
        }
        else if ((gma_dev->device_id == 0x1932) || (gma_dev->device_id == 0x193A) || (gma_dev->device_id == 0x193D)){
          device_id = 0x0000193B;
          devprop_add_value(device, "device-id", (UINT8*)&device_id, sizeof(device_id));
          devprop_add_value(device, "AAPL,ig-platform-id", skylake_ig_vals[10], 4);
        } else {
          devprop_add_value(device, "AAPL,ig-platform-id", skylake_ig_vals[10], 4);
        }
      A193D:
          if ((MacModel == iMac171) || (MacModel == MacPro61)){
            devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
            devprop_add_value(device, "AAPL,GfxYTile", skylake_hd_vals[1], 4); //<01000000>
            break;
          }
          devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
          devprop_add_value(device, "AAPL,Gfx324", skylake_hd_vals[0], 4);  //<01000000>
          devprop_add_value(device, "AAPL,GfxYTile", skylake_hd_vals[1], 4); //<01000000>
          devprop_add_value(device, "AAPL00,PanelCycleDelay", skylake_hd_vals[2], 4); //<fa000000>
          devprop_add_value(device, "AAPL00,PanelPowerDown", skylake_hd_vals[3], 4);  //<3c000000>
          devprop_add_value(device, "AAPL00,PanelPowerOff", skylake_hd_vals[4], 4);  //<11000000>
          devprop_add_value(device, "AAPL00,PanelPowerOn", skylake_hd_vals[5], 4);  //<19010000>
          devprop_add_value(device, "AAPL00,PanelPowerUp", skylake_hd_vals[6], 4);  //<30000000>
          devprop_add_value(device, "graphic-options", skylake_hd_vals[7], 4);  //<0c000000>
          break;
        
        
        //----------------Kabylake-------------------
      case 0x5902: // "Intel HD Graphics 610"           //
      case 0x5906: // "Intel HD Graphics 610"           //
      case 0x590A: // "Intel Kabylake GT1"              //
      case 0x5908: // "Intel Kabylake GT1"              //
      case 0x590B: // "Intel HD Graphics 610"           //
      case 0x590E: // "Intel Kabylake GT1"              //
      case 0x5913: // "Intel Kabylake GT1.5"            //
      case 0x5915: // "Intel Kabylake GT1.5"            //
      case 0x5917: // "Intel Kabylake GT1.5"            //
      case 0x5912: // "Intel HD Graphics 630"           //
        devprop_add_value(device, "AAPL,ig-platform-id", skylake_ig_vals[5], 4);
        device_id = 0x00001912;
        devprop_add_value(device, "device-id", (UINT8*)&device_id, sizeof(device_id));
        goto A593B;
      case 0x5916: // "Intel HD Graphics 620"           //
        devprop_add_value(device, "AAPL,ig-platform-id", skylake_ig_vals[1], 4);
        device_id = 0x00001916;
        devprop_add_value(device, "device-id", (UINT8*)&device_id, sizeof(device_id));
        goto A593B;
      case 0x591A: // "Intel Kabylake GT2"              //
      case 0x591B: // "Intel HD Graphics 630"           //
        devprop_add_value(device, "AAPL,ig-platform-id", skylake_ig_vals[3], 4);
        device_id = 0x0000191b;
        devprop_add_value(device, "device-id", (UINT8*)&device_id, sizeof(device_id));
        goto A593B;
      case 0x591D: // "Intel HD Graphics P630"          //
      case 0x591E: // "Intel HD Graphics 615"           //
        devprop_add_value(device, "AAPL,ig-platform-id", skylake_ig_vals[0], 4);
        device_id = 0x0000191e;
        devprop_add_value(device, "device-id", (UINT8*)&device_id, sizeof(device_id));
        goto A593B;
      case 0x5921: // "Intel Kabylake GT2F"             //
      case 0x5923: // "Intel HD Graphics 635"           //
      case 0x5926: // "Intel Iris Pro Graphics 640"     //
        devprop_add_value(device, "AAPL,ig-platform-id", skylake_ig_vals[7], 4);
        device_id = 0x00001926;
        devprop_add_value(device, "device-id", (UINT8*)&device_id, sizeof(device_id));
        goto A593B;
      case 0x5927: // "Intel Iris Pro Graphics 650"     //        
        device_id = 0x00001927;
        devprop_add_value(device, "device-id", (UINT8*)&device_id, sizeof(device_id));
      case 0x593B: // "Intel Kabylake GT4"              //
        if ((gma_dev->device_id == 0x591D) || (gma_dev->device_id == 0x5923)){
          device_id = 0x00001912;
          devprop_add_value(device, "device-id", (UINT8*)&device_id, sizeof(device_id));
          devprop_add_value(device, "AAPL,ig-platform-id", skylake_ig_vals[5], 4);
        } else {
          devprop_add_value(device, "AAPL,ig-platform-id", skylake_ig_vals[2], 4);
        }
      A593B:
        if ((gma_dev->device_id >= 0x5902) && (gma_dev->device_id <= 0x593B)){
          if ((MacModel == iMac171) || (MacModel == MacPro61)){
            devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
            devprop_add_value(device, "AAPL,GfxYTile", skylake_hd_vals[1], 4); //<01000000>
            break;
          }
          devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
          devprop_add_value(device, "AAPL,Gfx324", skylake_hd_vals[0], 4);  //<01000000>
          devprop_add_value(device, "AAPL,GfxYTile", skylake_hd_vals[1], 4); //<01000000>
          devprop_add_value(device, "AAPL00,PanelCycleDelay", skylake_hd_vals[2], 4); //<fa000000>
          devprop_add_value(device, "AAPL00,PanelPowerDown", skylake_hd_vals[3], 4);  //<3c000000>
          devprop_add_value(device, "AAPL00,PanelPowerOff", skylake_hd_vals[4], 4);  //<11000000>
          devprop_add_value(device, "AAPL00,PanelPowerOn", skylake_hd_vals[5], 4);  //<19010000>
          devprop_add_value(device, "AAPL00,PanelPowerUp", skylake_hd_vals[6], 4);  //<30000000>
          devprop_add_value(device, "graphic-options", skylake_hd_vals[7], 4);  //<0c000000>
          break;
        }
        break;
      default:
        DBG("  Not detect the proper ig-platform-id");
        break;
    }
  }
  // if set ig-platform-id
  else {
    switch (gma_dev->device_id) {
      case 0x2582: // GMA 915"                         //
      case 0x2592: // GMA 915"                         //
      case 0x27A2: // GMA 950"                         //
      case 0x27AE: // GMA 950"                         //
        //case 0x27A6: // Mobile GMA950"                   // not a GPU
        devprop_add_value(device, "AAPL,HasPanel", common_vals[1], 4);
        devprop_add_value(device, "built-in", &BuiltIn, 1);
        break;
      case 0xA011: // Mobile GMA3150"                  //
      case 0xA012: // Mobile GMA3150"                  //
      case 0x2772: // Desktop GMA950"                  //
      case 0x29C2: // Desktop GMA3100"                 //
        //case 0x2776: // Desktop GMA950"                  // not a GPU
        //case 0xA001: // Desktop GMA3150"                 //
      case 0xA001: // Mobile GMA3150"                  //
      case 0xA002: // Desktop GMA3150"                 //
        devprop_add_value(device, "built-in", &BuiltIn, 1);
        devprop_add_value(device, "AAPL00,DualLink", (UINT8*)&DualLink, 1);
        devprop_add_value(device, "AAPL,os-info", (UINT8*)&OsInfo, sizeof(OsInfo));
        break;
      case 0x2A02: // GMAX3100"                        //
      case 0x2A03: // GMAX3100"                        // not a GPU
      case 0x2A12: // GMAX3100"                        //
        //case 0x2A13: // GMAX3100"                        //
      case 0x2A42: // GMAX3100"                        //
        //case 0x2A43: // GMAX3100"                        //
        devprop_add_value(device, "AAPL,HasPanel", GMAX3100_vals[0], 4);
        devprop_add_value(device, "AAPL,SelfRefreshSupported", GMAX3100_vals[1], 4);
        devprop_add_value(device, "AAPL,aux-power-connected", GMAX3100_vals[2], 4);
        devprop_add_value(device, "AAPL,backlight-control", GMAX3100_vals[3], 4);
        devprop_add_value(device, "AAPL00,blackscreen-preferences", GMAX3100_vals[4], 4);
        devprop_add_value(device, "AAPL01,BacklightIntensity", GMAX3100_vals[5], 4);
        devprop_add_value(device, "AAPL01,blackscreen-preferences", GMAX3100_vals[6], 4);
        devprop_add_value(device, "AAPL01,DataJustify", GMAX3100_vals[7], 4);
        //devprop_add_value(device, "AAPL01,Depth", GMAX3100_vals[8], 4);
        devprop_add_value(device, "AAPL01,Dither", GMAX3100_vals[9], 4);
        devprop_add_value(device, "AAPL01,DualLink", (UINT8 *)&DualLink, 1);
        //devprop_add_value(device, "AAPL01,Height", GMAX3100_vals[10], 4);
        devprop_add_value(device, "AAPL01,Interlace", GMAX3100_vals[11], 4);
        devprop_add_value(device, "AAPL01,Inverter", GMAX3100_vals[12], 4);
        devprop_add_value(device, "AAPL01,InverterCurrent", GMAX3100_vals[13], 4);
        //devprop_add_value(device, "AAPL01,InverterCurrency", GMAX3100_vals[15], 4);
        devprop_add_value(device, "AAPL01,LinkFormat", GMAX3100_vals[14], 4);
        devprop_add_value(device, "AAPL01,LinkType", GMAX3100_vals[15], 4);
        devprop_add_value(device, "AAPL01,Pipe", GMAX3100_vals[16], 4);
        //devprop_add_value(device, "AAPL01,PixelFormat", GMAX3100_vals[17], 4);
        devprop_add_value(device, "AAPL01,Refresh", GMAX3100_vals[18], 4);
        devprop_add_value(device, "AAPL01,Stretch", GMAX3100_vals[19], 4);
        devprop_add_value(device, "AAPL01,InverterFrequency", GMAX3100_vals[20], 4);
        //devprop_add_value(device, "subsystem-vendor-id", GMAX3100_vals[21], 4);
        devprop_add_value(device, "subsystem-id", GMAX3100_vals[22], 4);
        devprop_add_value(device, "built-in", &BuiltIn, 1);
        break;
        
        //----------------Ironlake-------------------
      case 0x0042: // "Intel HD Graphics"               // Ironlake Desktop
      case 0x0046: // "Intel HD Graphics"               // Ironlake Mobile
        devprop_add_value(device, "AAPL,os-info", (UINT8*)&OsInfo, sizeof(OsInfo));
        devprop_add_value(device, "VRAM,totalsize", arrandale_vals[0], 4);
        //devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
        break;
        
        
        //----------------Sandybridge----------------
      case 0x0102: // "Intel HD Graphics 2000"          //
        devprop_add_value(device, "built-in", &BuiltIn, 1);
        devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
        devprop_add_value(device, "AAPL00,PixelFormat", sandy_bridge_hd_vals[0], 4);
        devprop_add_value(device, "AAPL00,T1", sandy_bridge_hd_vals[1], 4);
        devprop_add_value(device, "AAPL00,T2", sandy_bridge_hd_vals[2], 4);
        devprop_add_value(device, "AAPL00,T3", sandy_bridge_hd_vals[3], 4);
        devprop_add_value(device, "AAPL00,T4", sandy_bridge_hd_vals[4], 4);
        devprop_add_value(device, "AAPL00,T5", sandy_bridge_hd_vals[5], 4);
        devprop_add_value(device, "AAPL00,T6", sandy_bridge_hd_vals[6], 4);
        devprop_add_value(device, "AAPL00,T7", sandy_bridge_hd_vals[7], 4);
        devprop_add_value(device, "AAPL00,LinkType", sandy_bridge_hd_vals[8], 4);
        devprop_add_value(device, "AAPL00,LinkFormat", sandy_bridge_hd_vals[9], 4);
        devprop_add_value(device, "AAPL00,DualLink", sandy_bridge_hd_vals[10], 4);
        devprop_add_value(device, "AAPL00,Dither", sandy_bridge_hd_vals[11], 4);
        devprop_add_value(device, "AAPL00,DataJustify", sandy_bridge_hd_vals[12], 4);
        devprop_add_value(device, "graphic-options", sandy_bridge_hd_vals[13], 4);
        devprop_add_value(device, "AAPL,tbl-info", HD2000_tbl_info, 18);
        devprop_add_value(device, "AAPL,os-info", HD2000_os_info, 20);
        break;
      case 0x0106: // "Intel HD Graphics 3000"          //
      case 0x010A: // "Intel HD Graphics P3000"         // Xeon E3 1200 v1, needs FakeID
      case 0x0112: // "Intel HD Graphics 3000"          //
      case 0x0122: // "Intel HD Graphics 3000"          //
        devprop_add_value(device, "built-in", &BuiltIn, 1);
        devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
        devprop_add_value(device, "AAPL,tbl-info", HD3000_tbl_info, 18);
        devprop_add_value(device, "AAPL,os-info", HD3000_os_info, 20);
        break;
      case 0x0116: // "Intel HD Graphics 3000"          //
      case 0x0126: // "Intel HD Graphics 3000"          //
          devprop_add_value(device, "built-in", &BuiltIn, 1);
          devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
          devprop_add_value(device, "AAPL00,PixelFormat", sandy_bridge_hd_vals[0], 4);
          devprop_add_value(device, "AAPL00,T1", sandy_bridge_hd_vals[1], 4);
          devprop_add_value(device, "AAPL00,T2", sandy_bridge_hd_vals[2], 4);
          devprop_add_value(device, "AAPL00,T3", sandy_bridge_hd_vals[3], 4);
          devprop_add_value(device, "AAPL00,T4", sandy_bridge_hd_vals[4], 4);
          devprop_add_value(device, "AAPL00,T5", sandy_bridge_hd_vals[5], 4);
          devprop_add_value(device, "AAPL00,T6", sandy_bridge_hd_vals[6], 4);
          devprop_add_value(device, "AAPL00,T7", sandy_bridge_hd_vals[7], 4);
          devprop_add_value(device, "AAPL00,LinkType", sandy_bridge_hd_vals[8], 4);
          devprop_add_value(device, "AAPL00,LinkFormat", sandy_bridge_hd_vals[9], 4);
          devprop_add_value(device, "AAPL00,DualLink", sandy_bridge_hd_vals[10], 4);
          devprop_add_value(device, "AAPL00,Dither", sandy_bridge_hd_vals[11], 4);
          devprop_add_value(device, "AAPL00,DataJustify", sandy_bridge_hd_vals[12], 4);
          devprop_add_value(device, "graphic-options", sandy_bridge_hd_vals[13], 4);
          devprop_add_value(device, "AAPL,tbl-info", HD3000_tbl_info, 18);
          devprop_add_value(device, "AAPL,os-info", HD3000_os_info, 20);
          break;
        
        //----------------Ivybridge------------------
      case 0x0152: // "Intel HD Graphics 2500"          // iMac
      case 0x0156: // "Intel HD Graphics 2500"          // MacBook
      case 0x015A: // "Intel HD Graphics 4000"          //
      case 0x0162: // "Intel HD Graphics 4000"          // Desktop
      case 0x0166: // "Intel HD Graphics 4000"          // MacBookPro10,1 have this string as model name whatever chameleon team may say
      case 0x016A: // "Intel HD Graphics P4000"         // Xeon E3-1245
      case 0x015E: // "Intel HD Graphics"               //
      case 0x0172: // "Intel HD Graphics 2500"          //
      case 0x0176: // "Intel HD Graphics 2500"          //
          devprop_add_value(device, "built-in", &BuiltIn, 1);
          devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
          devprop_add_value(device, "AAPL00,PixelFormat", ivy_bridge_hd_vals[0], 4);
          devprop_add_value(device, "AAPL00,T1", ivy_bridge_hd_vals[1], 4);
          devprop_add_value(device, "AAPL00,T2", ivy_bridge_hd_vals[2], 4);
          devprop_add_value(device, "AAPL00,T3", ivy_bridge_hd_vals[3], 4);
          devprop_add_value(device, "AAPL00,T4", ivy_bridge_hd_vals[4], 4);
          devprop_add_value(device, "AAPL00,T5", ivy_bridge_hd_vals[5], 4);
          devprop_add_value(device, "AAPL00,T6", ivy_bridge_hd_vals[6], 4);
          devprop_add_value(device, "AAPL00,T7", ivy_bridge_hd_vals[7], 4);
          devprop_add_value(device, "AAPL00,LinkType", ivy_bridge_hd_vals[8], 4);
          devprop_add_value(device, "AAPL00,LinkFormat", ivy_bridge_hd_vals[9], 4);
          devprop_add_value(device, "AAPL00,DualLink", ivy_bridge_hd_vals[10], 4);
          devprop_add_value(device, "AAPL00,Dither", ivy_bridge_hd_vals[11], 4);
          devprop_add_value(device, "AAPL,gray-value", ivy_bridge_hd_vals[12], 4);
          devprop_add_value(device, "AAPL,gray-page", ivy_bridge_hd_vals[13], 4);
          devprop_add_value(device, "graphic-options", ivy_bridge_hd_vals[14], 4);
          break;
        
        //----------------Haswell--------------------
      case 0x0402: // "Intel Haswell Desktop"           //
      case 0x0406: // "Intel Haswell Mobile"            //
      case 0x040A: // "Intel Haswell Server"            //
      case 0x040B: // "Intel Haswell"                   //
      case 0x040E: // "Intel Haswell"                   //
      case 0x0412: // "Intel HD Graphics 4600"          //
      case 0x0416: // "Intel HD Graphics 4600"          //
      case 0x041A: // "Intel HD Graphics P4600"         //
      case 0x041B: // "Intel Haswell"                   //
      case 0x041E: // "Intel HD Graphics 4400"          //
      case 0x0422: // "Intel Haswell Desktop"           //
      case 0x0426: // "Intel Haswell Mobile"            //
      case 0x042A: // "Intel Haswell Server"            //
      case 0x042B: // "Intel Haswell"                   //
      case 0x042E: // "Intel Haswell"                   //
        devprop_add_value(device, "built-in", &BuiltIn, 1);
        devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
        devprop_add_value(device, "AAPL,gray-value", haswell_hd_vals[0], 4);
        devprop_add_value(device, "graphic-options", haswell_hd_vals[1], 4);
        break;
        
      case 0x0A02: // "Intel Haswell Desktop"           //
      case 0x0A06: // "Intel Haswell Mobile"            //
      case 0x0A0A: // "Intel Haswell Server"            //
      case 0x0A0B: // "Intel Haswell"                   //
      case 0x0A0E: // "Intel HD Graphics 4400"          //
      case 0x0A12: // "Intel Haswell Desktop"           //
      case 0x0A16: // "Intel HD Graphics 4400"          //
      case 0x0A1A: // "Intel Haswell Server"            //
      case 0x0A1B: // "Intel Haswell"                   //
      case 0x0A1E: // "Intel HD Graphics 4400"          //
      case 0x0A22: // "Intel Haswell Desktop"           //
      case 0x0A26: // "Intel HD Graphics 5000"          //
      case 0x0A2A: // "Intel Haswell Server"            //
      case 0x0A2B: // "Intel Haswell"                   //
      case 0x0A2E: // "Intel Iris 5100"                 // Intel Haswell Intel Iris 5100 (i7-4558U, i7-4578U)
        //device_id = 0x00000a26;
        devprop_add_value(device, "built-in", &BuiltIn, 1);
        devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
        devprop_add_value(device, "AAPL,gray-value", haswell_hd_vals[0], 4);
        devprop_add_value(device, "graphic-options", haswell_hd_vals[1], 4);
        break;
      case 0x0C02: // "Intel Haswell Desktop"           //
      case 0x0C06: // "Intel Haswell Mobile"            //
      case 0x0C0A: // "Intel Haswell Server"            //
      case 0x0C0B: // "Intel Haswell"                   //
      case 0x0C0E: // "Intel Haswell"                   //
      case 0x0C12: // "Intel Haswell Desktop"           //
      case 0x0C16: // "Intel Haswell Mobile"            //
      case 0x0C1A: // "Intel Haswell Server"            //
      case 0x0C1B: // "Intel Haswell"                   //
      case 0x0C1E: // "Intel Haswell"                   //
      case 0x0C22: // "Intel Haswell Desktop"           //
      case 0x0C26: // "Intel Haswell Mobile"            //
      case 0x0C2A: // "Intel Haswell Server"            //
      case 0x0C2B: // "Intel Haswell"                   //
      case 0x0C2E: // "Intel Haswell"                   //
        devprop_add_value(device, "built-in", &BuiltIn, 1);
        devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
        devprop_add_value(device, "AAPL,gray-value", haswell_hd_vals[0], 4);
        devprop_add_value(device, "graphic-options", haswell_hd_vals[1], 4);
        break;
      case 0x0D02: // "Intel Haswell Desktop"           //
      case 0x0D06: // "Intel HD Graphics 5200"          //
      case 0x0D0A: // "Intel Haswell Server"            //
      case 0x0D0B: // "Intel Haswell"                   //
      case 0x0D0E: // "Intel Haswell"                   //
      case 0x0D12: // "Intel HD Graphics 5200"          //
      case 0x0D16: // "Intel HD Graphics 5200"          //
      case 0x0D1A: // "Intel Haswell Server"            //
      case 0x0D1B: // "Intel Haswell"                   //
      case 0x0D1E: // "Intel Haswell"                   //
      case 0x0D22: // "Intel Iris Pro 5200"             //
      case 0x0D26: // "Intel Iris Pro 5200"             // Intel Haswell i7 4860HQ
      case 0x0D2A: // "Intel Haswell"                   //
      case 0x0D2B: // "Intel Haswell"                   //
      case 0x0D2E: // "Intel Haswell"                   //
        devprop_add_value(device, "built-in", &BuiltIn, 1);
        devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
        devprop_add_value(device, "AAPL,gray-value", haswell_hd_vals[0], 4);
        devprop_add_value(device, "graphic-options", haswell_hd_vals[1], 4);
        break;
        
        //----------------Broadwell------------------
      case 0x1602: // "Intel HD Graphics"               //
      case 0x1606: // "Intel HD Graphics"               //
      case 0x160A: // "Intel HD Graphics"               // Broadwell-U Integrated
      case 0x160B: // "Intel HD Graphics"               // Broadwell-U Integrated
      case 0x160D: // "Intel HD Graphics"               // Broadwell-U Integrated
      case 0x160E: // "Intel HD Graphics"               // Broadwell-U Integrated
      case 0x1612: // "Intel HD Graphics 5600"          // Broadwell i7-5700HQ
      case 0x1616: // "Intel HD Graphics 5500"          //
      case 0x161A: // "Intel HD Graphics"               // Broadwell-U Integrated
      case 0x161B: // "Intel HD Graphics"               // Broadwell-U Integrated
      case 0x161D: // "Intel HD Graphics"               // Broadwell-U Integrated
      case 0x161E: // "Intel HD Graphics 5300"          //
      case 0x1622: // "Intel Iris Pro Graphics 6200"    //
      case 0x1626: // "Intel HD Graphics 6000"          //
      case 0x162A: // "Intel Iris Pro Graphics P6300"   // Intel(R) Iris(TM) Pro Graphics
      case 0x162B: // "Intel Iris Graphics 6100"        // Intel NUC 5i7RYH, i7-5557U
      case 0x162D: // "Intel Iris Pro Graphics P6300"   //
      case 0x162E: // "Intel HD Graphics"               // Broadwell-U Integrated Graphics
      case 0x1632: // "Intel HD Graphics"               // Broadwell-U Integrated Graphics
      case 0x1636: // "Intel HD Graphics"               // Broadwell-U Integrated Graphics
      case 0x163A: // "Intel HD Graphics"               // Broadwell-U Integrated Graphics
      case 0x163B: // "Intel HD Graphics"               // Broadwell-U Integrated Graphics
      case 0x163D: // "Intel HD Graphics"               // Broadwell-U Integrated Graphics
      case 0x163E: // "Intel HD Graphics"               // Broadwell-U Integrated Graphics
          devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
          devprop_add_value(device, "AAPL,gray-value", broadwell_hd_vals[0], 4);
          devprop_add_value(device, "AAPL,gray-page", broadwell_hd_vals[1], 4);
          devprop_add_value(device, "graphic-options", broadwell_hd_vals[2], 4);
          break;
        
        //----------------Skylake--------------------
      case 0x1902: // "Intel HD Graphics 510"           //
      case 0x1906: // "Intel HD Graphics 510"           //
      case 0x190A: // "Intel Skylake GT1"               //
      case 0x190B: // "Intel HD Graphics 510"           //
      case 0x190E: // "Intel Skylake GT1"               //
      case 0x1912: // "Intel HD Graphics 530"           //
      case 0x1913: // "Intel Skylake GT2f"              //
      case 0x1915: // "Intel Skylake GT2f"              //
      case 0x1916: // "Intel HD Graphics 520"           //
      case 0x1917: // "Intel Skylake GT2f"              //
      case 0x191A: // "Intel Skylake GT2"               //
      case 0x191B: // "Intel HD Graphics 530"           //
      case 0x191D: // "Intel HD Graphics P530"          //
      case 0x191E: // "Intel HD Graphics 515"           //
      case 0x1921: // "Intel Skylake GT2"               //
      case 0x1923: // "Intel HD Graphics 535"           //
      case 0x1926: // "Intel Iris Graphics 540"         //
      case 0x1927: // "Intel Iris Graphics 550"         //
      case 0x192A: // "Intel Skylake GT4"               //
      case 0x192B: // "Intel Iris Graphics 555"         //
      case 0x192D: // "Intel Iris Graphics P555"        //
      case 0x1932: // "Intel Iris Pro Graphics 580"     //
      case 0x193A: // "Intel Iris Pro Graphics P580"    //
      case 0x193B: // "Intel Iris Pro Graphics 580"     //
      case 0x193D: // "Intel Iris Pro Graphics P580"    //
          if ((MacModel == iMac171) || (MacModel == MacPro61)){
            devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
            devprop_add_value(device, "AAPL,GfxYTile", skylake_hd_vals[1], 4); //<01000000>
            break;
          }
          devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
          devprop_add_value(device, "AAPL,Gfx324", skylake_hd_vals[0], 4);  //<01000000>
          devprop_add_value(device, "AAPL,GfxYTile", skylake_hd_vals[1], 4); //<01000000>
          devprop_add_value(device, "AAPL00,PanelCycleDelay", skylake_hd_vals[2], 4); //<fa000000>
          devprop_add_value(device, "AAPL00,PanelPowerDown", skylake_hd_vals[3], 4);  //<3c000000>
          devprop_add_value(device, "AAPL00,PanelPowerOff", skylake_hd_vals[4], 4);  //<11000000>
          devprop_add_value(device, "AAPL00,PanelPowerOn", skylake_hd_vals[5], 4);  //<19010000>
          devprop_add_value(device, "AAPL00,PanelPowerUp", skylake_hd_vals[6], 4);  //<30000000>
          devprop_add_value(device, "graphic-options", skylake_hd_vals[7], 4);  //<0c000000>
          break;        
        
        //----------------Kabylake-------------------
      case 0x5902: // "Intel HD Graphics 610"           //
      case 0x5906: // "Intel HD Graphics 610"           //
      case 0x590A: // "Intel Kabylake GT1"              //
      case 0x5908: // "Intel Kabylake GT1"              //
      case 0x590B: // "Intel HD Graphics 610"           //
      case 0x590E: // "Intel Kabylake GT1"              //
      case 0x5913: // "Intel Kabylake GT1.5"            //
      case 0x5915: // "Intel Kabylake GT1.5"            //
      case 0x5917: // "Intel Kabylake GT1.5"            //
      case 0x5912: // "Intel HD Graphics 630"           //
      case 0x5916: // "Intel HD Graphics 620"           //
      case 0x591A: // "Intel Kabylake GT2"              //
      case 0x591B: // "Intel HD Graphics 630"           //
      case 0x591D: // "Intel HD Graphics P630"          //
      case 0x591E: // "Intel HD Graphics 615"           //
      case 0x5921: // "Intel Kabylake GT2F"             //
      case 0x5923: // "Intel HD Graphics 635"           //
      case 0x5926: // "Intel Iris Pro Graphics 640"     //
      case 0x5927: // "Intel Iris Pro Graphics 650"     //
      case 0x593B: // "Intel Kabylake GT4"              //
          if ((MacModel == iMac171) || (MacModel == MacPro61)){
            devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
            devprop_add_value(device, "AAPL,GfxYTile", skylake_hd_vals[1], 4); //<01000000>
            break;
          }
          devprop_add_value(device, "hda-gfx", (UINT8*)"onboard-1", 10);
          devprop_add_value(device, "AAPL,Gfx324", skylake_hd_vals[0], 4);  //<01000000>
          devprop_add_value(device, "AAPL,GfxYTile", skylake_hd_vals[1], 4); //<01000000>
          devprop_add_value(device, "AAPL00,PanelCycleDelay", skylake_hd_vals[2], 4); //<fa000000>
          devprop_add_value(device, "AAPL00,PanelPowerDown", skylake_hd_vals[3], 4);  //<3c000000>
          devprop_add_value(device, "AAPL00,PanelPowerOff", skylake_hd_vals[4], 4);  //<11000000>
          devprop_add_value(device, "AAPL00,PanelPowerOn", skylake_hd_vals[5], 4);  //<19010000>
          devprop_add_value(device, "AAPL00,PanelPowerUp", skylake_hd_vals[6], 4);  //<30000000>
          devprop_add_value(device, "graphic-options", skylake_hd_vals[7], 4);  //<0c000000>
          break;
      default:
        DBG("  Intel card id=%x unsupported, please report to the clover thread\n", gma_dev->device_id);
        return FALSE;
    }
  }
#if DEBUG_GMA == 2
	gBS->Stall(5000000);
#endif
  
	
	return TRUE;
}
