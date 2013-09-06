/*
 * Copyright (c) 2011-2012 Frank Peng. All rights reserved.
 *
 */

#include "StateGenerator.h"
#include "Display.h"
#include <IndustryStandard/PciCommand.h>

#ifdef DBG
#undef DBG
#endif

#ifndef DEBUG_FIX
#ifndef DEBUG_ALL
#define DEBUG_FIX 1
#else
#define DEBUG_FIX DEBUG_ALL
#endif
#endif

#if DEBUG_FIX==0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_FIX, __VA_ARGS__)	
#endif



CHAR8*  device_name[11];  // 0=>Display  1=>network  2=>firewire 3=>LPCB 4=>HDAAudio 5=>RTC 6=>TMR 7=>SBUS 8=>PIC 9=>Airport 10=>XHCI
CHAR8*  UsbName[10];
CHAR8*  Netmodel;

BOOLEAN HDAFIX = TRUE;
BOOLEAN GFXHDAFIX = TRUE;
BOOLEAN DisplayName1;
BOOLEAN DisplayName2;
BOOLEAN NetworkName;
BOOLEAN ArptName;
BOOLEAN XhciName;
BOOLEAN ArptBCM;
BOOLEAN ArptAtheros;
BOOLEAN LPCBFIX;
BOOLEAN IDEFIX;
BOOLEAN SATAFIX;
BOOLEAN ASUSFIX;
BOOLEAN USBIntel;
BOOLEAN USBNForce;
BOOLEAN USBIDFIX = TRUE;
BOOLEAN Display1PCIE;
BOOLEAN Display2PCIE;
BOOLEAN FirewireName;

// for read computer data
UINT32 DisplayADR1[2];
UINT32 DisplayADR2[2];
UINT32 NetworkADR1;
UINT32 NetworkADR2;
UINT32 ArptADR1;
UINT32 ArptADR2;
UINT32 FirewireADR1;
UINT32 FirewireADR2;
UINT32 SBUSADR1;
UINT32 SBUSADR2;
UINT32 IDEADR1;
UINT32 IDEADR2;
UINT32 SATAADR1;
UINT32 SATAADR2;
UINT32 SATAAHCIADR1;
UINT32 SATAAHCIADR2;
UINT32 IDEVENDOR;
UINT32 SATAVENDOR;
UINT32 SATAAHCIVENDOR;
UINT32 DisplayVendor[2];
UINT16 DisplayID[2];
UINT32 DisplaySubID[2];

//UINT32 PWRBADR;

UINT32 HDAADR1;
UINT32 USBADR[12];
UINT32 USBADR2[12];
UINT32 USBADR3[12]; /*<-NFORCE_USB*/
UINT32 USBID[12];
UINT32 USB20[12];
UINT32 USB30[12];
UINT32 USB40[12];  /*<-NFORCE_USB*/

UINT32 HDAcodecId=0;
UINT32 HDAlayoutId=0;
UINT32 GfxcodecId[2];
UINT32 GfxlayoutId[2];

pci_dt_t   Displaydevice[2];


UINTN usb;

struct lpc_device_t 
{
	UINT32		id;
};

//static
CHAR8 dataBuiltin[] = {0x00};
//static
CHAR8 dataBuiltin1[] = {0x01};

static struct lpc_device_t lpc_chipset[] =
{
    {0x00000000},
    //
    {0x80862811},
    {0x80862815},
    {0x808627b9},
    {0x808627bd},
    {0x80862670},
    {0x80868119},
    {0x80862916},
    {0x80863a18},
    {0x80863b00},
    {0x80863b01},
    {0x80863b02},
    {0x80863b09},
    {0x10de0aac},
    {0x10de0aae},
    {0x10de0aaf},
    {0x10de0d80},
    {0x10de0d81},
    {0x10de0d82},
    {0x10de0d83},
    //SB
    {0x80861c42},
    {0x80861c44},
    {0x80861c4e},
    {0x80861c4c},
    {0x80861c50},
    {0x80861c4a},
    {0x80861c46},
    {0x80861c5c},
    {0x80861c52},
    {0x80861c54},
    {0x80861c56}, 
    {0x80861c43},
    {0x80861c4f},
    {0x80861c47},
    {0x80861c4b},
    {0x80861c49},
    {0x80861c41},
    {0x80861c4d},
};

struct net_chipsets_t {
	UINT32 id;
	char *name;
};

static struct net_chipsets_t NetChipsets[] = {
	{ 0x00000000, "Unknown" },
	// 8169
	{ 0x10EC8169, "Realtek 8169/8110 Gigabit Ethernet" },
	{ 0x10EC8168, "Realtek 8168/8101E Gigabit Ethernet" },
	{ 0x10EC8167, "Realtek 8169/8110 Gigabit Ethernet" },
  { 0x10EC8136, "Realtek 8168/8101E Gigabit Ethernet" },
  // 8139
  { 0x10EC8139, "Realtek RTL8139/810x Family Fast Ethernet" },
  { 0x11861300, "Realtek RTL8139/810x Family Fast Ethernet" },
  { 0x11131211, "Realtek RTL8139/810x Family Fast Ethernet" },
  // Broadcom 57XX 
  { 0x14e41600, "Broadcom 5751 Ethernet" },
  { 0x14e41659, "Broadcom 57XX Ethernet" },
  { 0x14e4165A, "BCM5722 NetXtreme Server Gigabit Ethernet" },
  { 0x14e4166A, "Broadcom 57XX Ethernet" },
  { 0x14e41672, "BCM5754M NetXtreme Gigabit Ethernet" },
  { 0x14e41673, "BCM5755M NetXtreme Gigabit Ethernet" },
  { 0x14e4167A, "BCM5754 NetXtreme Gigabit Ethernet" },
  { 0x14e4167B, "BCM5755 NetXtreme Gigabit Ethernet" },
  { 0x14e41684, "Broadcom 57XX Ethernet" },
  { 0x14e41691, "BCM57788 NetLink (TM) Gigabit Ethernet" },
  { 0x14e41693, "BCM5787M NetLink (TM) Gigabit Ethernet" },
  { 0x14e4169B, "BCM5787 NetLink (TM) Gigabit Ethernet" },
  { 0x14e416B4, "Broadcom 57XX Ethernet" },
  { 0x14e416B5, "BCM57785 Gigabit Ethernet PCIe" },
  { 0x14e41712, "BCM5906 NetLink (TM) Fast Ethernet" },
  { 0x14e41713, "BCM5906M NetLink (TM) Fast Ethernet" },
  // Intel 8255x Ethernet
  { 0x80861051, "Intel 8255x Ethernet" },
  { 0x80861050, "Intel 8255x Ethernet" },
  { 0x80861029, "Intel 8255x Ethernet" },
  { 0x80861030, "Intel 8255x Ethernet" },    
  { 0x80861209, "Intel 8255x Ethernet" },
  { 0x80861227, "Intel 8255x Ethernet" },
  { 0x80861228, "Intel 8255x Ethernet" },
  { 0x80861229, "Intel 8255x Ethernet" },  
  { 0x80861503, "Intel 82579V Gigabit Network Controller" },
  { 0x80862449, "Intel 8255x Ethernet" },
  { 0x80862459, "Intel 8255x Ethernet" },
  { 0x8086245D, "Intel 8255x Ethernet" },
  { 0x80861091, "Intel 8255x Ethernet" }, 
  { 0x80861060, "Intel 8255x Ethernet" },
  // Atheros AR8151 Ethernet  
  { 0x19691083, "Qualcomm Atheros AR8151 v2.0 Gigabit Ethernet" },
};

struct ide_chipsets_t {
    UINT32 id;
};

static struct ide_chipsets_t ide_chipset[] =
{
    // IDE
    {0x00000000},
    //
    {0x8086269e},
    {0x808627df},
    {0x80862850},

    //SATA
    {0x80862680},
    {0x808627c0},
    {0x808627c4},
    {0x80862828},
};
//2820? 2825?
struct ahci_chipsets_t {
    UINT32 id;
};

static struct ahci_chipsets_t ahci_chipset[] =
{
    // SATA AHCI
    {0x00000000},
    //
    {0x80863a22},
    {0x80862681},
  {0x80862682},
    {0x808627c5},
  {0x80862825},
    {0x80862829},
    {0x80863b29},
    {0x80863b22},
    {0x80863b2f},
    {0x80861c02},
    {0x80861c03},
    {0x10de0ab9},
    {0x10de0b88},
};

UINT8 dtgp[] = // Method (DTGP, 5, NotSerialized) ......
{
   0x14, 0x3F, 0x44, 0x54, 0x47, 0x50, 0x05, 0xA0, 
   0x30, 0x93, 0x68, 0x11, 0x13, 0x0A, 0x10, 0xC6,
   0xB7, 0xB5, 0xA0, 0x18, 0x13, 0x1C, 0x44, 0xB0,
   0xC9, 0xFE, 0x69, 0x5E, 0xAF, 0x94, 0x9B, 0xA0,
   0x18, 0x93, 0x69, 0x01, 0xA0, 0x0C, 0x93, 0x6A,
   0x00, 0x70, 0x11, 0x03, 0x01, 0x03, 0x6C, 0xA4,
   0x01, 0xA0, 0x06, 0x93, 0x6A, 0x01, 0xA4, 0x01,
   0x70, 0x11, 0x03, 0x01, 0x00, 0x6C, 0xA4, 0x00
};

UINT8 sbus1[] = 
{              //  Device (SBUS) _ADR,1F0003 size=E9+2=EB
  0x5B, 0x82, 0x49, 0x0E, 0x53, 0x42, 0x55, 0x53,  // 00000080    "[.I.SBUS"
  0x08, 0x5F, 0x41, 0x44, 0x52, 0x0C,              // 00000088    "._ADR."
  0x03, 0x00, 0x1F, 0x00, 0x5B, 0x82, 0x4B, 0x05,  // 00000090    "....[.K."
  0x42, 0x55, 0x53, 0x30, 0x08, 0x5F, 0x43, 0x49,  // 00000098    "BUS0._CI"
  0x44, 0x0D, 0x73, 0x6D, 0x62, 0x75, 0x73, 0x00,  // 000000A0    "D.smbus."
  0x08, 0x5F, 0x41, 0x44, 0x52, 0x00, 0x5B, 0x82,  // 000000A8    "._ADR.[."
  0x41, 0x04, 0x44, 0x56, 0x4C, 0x30, 0x08, 0x5F,  // 000000B0    "A.DVL0._"
  0x41, 0x44, 0x52, 0x0A, 0x57, 0x08, 0x5F, 0x43,  // 000000B8    "ADR.W._C"
  0x49, 0x44, 0x0D, 0x64, 0x69, 0x61, 0x67, 0x73,  // 000000C0    "ID.diags"
  0x76, 0x61, 0x75, 0x6C, 0x74, 0x00, 0x14, 0x22,  // 000000C8    "vault..""
  0x5F, 0x44, 0x53, 0x4D, 0x04, 0x70, 0x12, 0x0D,  // 000000D0    "_DSM.p.."
  0x02, 0x0D, 0x61, 0x64, 0x64, 0x72, 0x65, 0x73,  // 000000D8    "..addres"
  0x73, 0x00, 0x0A, 0x57, 0x60, 0x44, 0x54, 0x47,  // 000000E0    "s..W`DTG"
  0x50, 0x68, 0x69, 0x6A, 0x6B, 0x71, 0x60, 0xA4,  // 000000E8    "Phijkq`."
  0x60, 0x14, 0x4B, 0x07, 0x5F, 0x44, 0x53, 0x4D,  // 000000F0    "`.K._DSM"
  0x04, 0x70, 0x12, 0x45, 0x06, 0x08, 0x0D, 0x62,  // 000000F8    ".p.E...b"
  0x75, 0x69, 0x6C, 0x74, 0x2D, 0x69, 0x6E, 0x00,  // 00000100    "uilt-in."
  0x11, 0x03, 0x01, 0x01, 0x0D, 0x64, 0x65, 0x76,  // 00000108    ".....dev"
  0x69, 0x63, 0x65, 0x2D, 0x69, 0x64, 0x00, 0x11,  // 00000110    "ice-id.."
  0x07, 0x0A, 0x04, 0x30, 0x3A, 0x00, 0x00, 0x0D,  // 00000118    "...0:..."
  0x6D, 0x6F, 0x64, 0x65, 0x6C, 0x00, 0x11, 0x1E,  // 00000120    "model..."
  0x0A, 0x1B, 0x49, 0x6E, 0x74, 0x65, 0x6C, 0x20,  // 00000128    "..Intel "
  0x38, 0x32, 0x38, 0x30, 0x31, 0x4A, 0x49, 0x20,  // 00000130    "82801JI "
  0x49, 0x43, 0x48, 0x31, 0x30, 0x20, 0x46, 0x61,  // 00000138    "ICH10 Fa"
  0x6D, 0x69, 0x6C, 0x79, 0x00, 0x0D, 0x6E, 0x61,  // 00000140    "mily..na"
  0x6D, 0x65, 0x00, 0x11, 0x14, 0x0A, 0x11, 0x53,  // 00000148    "me.....S"
  0x4D, 0x42, 0x75, 0x73, 0x20, 0x63, 0x6F, 0x6E,  // 00000150    "MBus con"
  0x74, 0x72, 0x6F, 0x6C, 0x6C, 0x65, 0x72, 0x00,  // 00000158    "troller."
  0x60, 0x44, 0x54, 0x47, 0x50, 0x68, 0x69, 0x6A,  // 00000160    "`DTGPhij"
  0x6B, 0x71, 0x60, 0xA4, 0x60  
};

UINT8 bus0[] =
{  //size=5B+2=5D
   0x5B, 0x82, 0x4B, 0x05, 0x42, 0x55,
   0x53, 0x30, 0x08, 0x5F, 0x43, 0x49, 0x44, 0x0D,
   0x73, 0x6D, 0x62, 0x75, 0x73, 0x00, 0x08, 0x5F,
   0x41, 0x44, 0x52, 0x00, 0x5B, 0x82, 0x41, 0x04,
   0x44, 0x56, 0x4C, 0x30, 0x08, 0x5F, 0x41, 0x44,
   0x52, 0x0A, 0x57, 0x08, 0x5F, 0x43, 0x49, 0x44,
   0x0D, 0x64, 0x69, 0x61, 0x67, 0x73, 0x76, 0x61,
   0x75, 0x6C, 0x74, 0x00, 0x14, 0x22, 0x5F, 0x44,
   0x53, 0x4D, 0x04, 0x70, 0x12, 0x0D, 0x02, 0x0D,
   0x61, 0x64, 0x64, 0x72, 0x65, 0x73, 0x73, 0x00,
   0x0A, 0x57, 0x60, 0x44, 0x54, 0x47, 0x50, 0x68,
   0x69, 0x6A, 0x6B, 0x71, 0x60, 0xA4, 0x60
};

UINT8 patafix[] =
{
/*  OperationRegion (IDET, PCI_Config, 0x40, 0x04)
  Field (IDET, WordAcc, NoLock, Preserve)
  {
    M1,     8, 
    M2,     8, 
    M3,     8, 
    M4,     8
  }
  
  Method (_INI, 0, NotSerialized)
  {
    Store (0x07, M1)
    Store (0xE3, M2)
    Store (Zero, M3)
    Store (0xC0, M4)
    Return (Zero)
  }
*/  
  0x5B, 
  0x80, 0x49, 0x44, 0x45, 0x54, 0x02, 0x0A, 0x40, //000001C0    ".IDET..@"
  0x0A, 0x04, 0x5B, 0x81, 0x1A, 0x49, 0x44, 0x45, //000001C8    "..[..IDE"
  0x54, 0x02, 0x4D, 0x31, 0x5F, 0x5F, 0x08, 0x4D, //000001D0    "T.M1__.M"
  0x32, 0x5F, 0x5F, 0x08, 0x4D, 0x33, 0x5F, 0x5F, //000001D8    "2__.M3__"
  0x08, 0x4D, 0x34, 0x5F, 0x5F, 0x08, 0x14, 0x23, //000001E0    ".M4__..#"
  0x5F, 0x49, 0x4E, 0x49, 0x00, 0x70, 0x0A, 0x07, //000001E8    "_INI.p.."
  0x4D, 0x31, 0x5F, 0x5F, 0x70, 0x0A, 0xE3, 0x4D, //000001F0    "M1__p..M"
  0x32, 0x5F, 0x5F, 0x70, 0x00, 0x4D, 0x33, 0x5F, //000001F8    "2__p.M3_"
  0x5F, 0x70, 0x0A, 0xC0, 0x4D, 0x34, 0x5F, 0x5F, //00000200    "_p..M4__"
  0xA4, 0x00                                 	    //00000208    ".."
  
};

UINT8 hpet0[] =
{
  0x5B, 0x82, 0x47, 0x04, 0x48, 0x50, 0x45, 0x54,                 //Device (HPET)
  0x08, 0x5F, 0x48, 0x49, 0x44, 0x0C, 0x41, 0xD0, 0x01, 0x03,     //Name (_HID, EisaId ("PNP0103"))
  0x08, 0x5F, 0x43, 0x49, 0x44, 0x0C, 0x41, 0xD0, 0x0C, 0x01,     //Name (_CID, EisaId ("PNP0C01"))
  0x08, 0x41, 0x54, 0x54, 0x30, 0x11, 0x14, 0x0A, 0x11,           //Name (ATT0, ResourceTemplate ()
  0x86, 0x09, 0x00, 0x01,                                         //  Memory32Fixed (ReadWrite,
  0x00, 0x00, 0xD0, 0xFE, 0x00, 0x04, 0x00, 0x00,                 //    0xFED00000, 0x00000400, )
  0x22, 0x01, 0x09, 0x79, 0x00,                                   //  IRQNoFlags () {0,8,11}
//  0x14, 0x09, 0x5F, 0x53, 0x54, 0x41, 0x00,                       //Method (_STA, 0, NotSerialized)
//  0xA4, 0x0A, 0x0F,                                               //  Return (0x0F)
  0x08, 0x5F, 0x53, 0x54, 0x41, 0x0A, 0x0F,                       //  Name (_STA, 0x0F)
  0x14, 0x0B, 0x5F, 0x43, 0x52, 0x53, 0x00,                       //Method (_CRS, 0, NotSerialized)
  0xA4, 0x41, 0x54, 0x54, 0x30                                    //  Return (ATT0)
};
/*
UINT8 hpet1[] =  // Name (_CID, EisaId ("PNP0C01"))
{
    0x08, 0x5F, 0x43, 0x49, 0x44, 0x0C, 0x41, 0xD0, 0x0C, 0x01
};
*/
CHAR8 wakret[] =
{
    0xA4, 0x12, 0x04, 0x02, 0x00, 0x00
};

CHAR8 pwrb[] = //? \_SB_PWRB, 0x02
{
  0x86, 0x5C, 0x2E, 0x5F, 0x53, 0x42, 0x5F, 0x50, 0x57, 0x52, 0x42, 0x0A, 0x02
};


CHAR8 acpi3[] = {  //Name(_HID, "ACPI003")
  0x08, 0x5F, 0x48, 0x49, 0x44, 0x0D,
  0x41, 0x43, 0x50, 0x49, 0x30, 0x30, 0x30, 0x33, 0x00
};

  //Name (_PRW, Package (0x02){0x1C, 0x03}
CHAR8 prw1c[] = 
{
  0x08, 0x5F, 0x50, 0x52, 0x57, 0x12, 0x06, 0x02, 0x0A, 0x1C, 0x0A, 0x03
};


CHAR8 dtgp_1[] =    // DTGP (Arg0, Arg1, Arg2, Arg3, RefOf (Local0))  
{                   // Return (Local0)
   0x44, 0x54, 0x47, 0x50, 0x68, 0x69, 0x6A, 0x6B,
   0x71, 0x60, 0xA4, 0x60
};

CHAR8 pwrbcid[] =
{
    0x08, 0x5F, 0x43, 0x49, 0x44, 0x0C, 0x41, 0xD0, 0x0C, 0x0E, 0x14,
    0x0E, 0x5F, 0x50, 0x52, 0x57, 0x00, 0xA4, 0x12, 0x06, 0x02, 0x0A,
    0x0B, 0x0A, 0x04
};

CHAR8 pwrbprw[] =
{
  0x14, 0x0E, 0x5F, 0x50, 0x52, 0x57, 0x00, 0xA4, 0x12, 0x06, 0x02,
  0x0A, 0x0B, 0x0A, 0x04
};

CHAR8 shutdown[] =
{
    0xA0, 0x05, 0x93, 0x68, 0x0A, 0x05, 0xA1, 0x01
};

CHAR8 pnlf[] =
{
  0x5B, 0x82, 0x2D, 0x50, 0x4E, 0x4C, 0x46,                         //Device (PNLF)
  0x08, 0x5F, 0x48, 0x49, 0x44, 0x0C, 0x06, 0x10, 0x00, 0x02,       //  Name (_HID, EisaId ("APP0002"))
  0x08, 0x5F, 0x43, 0x49, 0x44,                                     //  Name (_CID, 
  0x0D, 0x62, 0x61, 0x63, 0x6B, 0x6C, 0x69, 0x67, 0x68, 0x74, 0x00, //              "backlight")
  0x08, 0x5F, 0x55, 0x49, 0x44, 0x0A, 0x0A,                         //  Name (_UID, 0x0A)
  0x08, 0x5F, 0x53, 0x54, 0x41, 0x0A, 0x0B                          //  Name (_STA, 0x0B)
};

CHAR8 app2[] = //Name (_HID, EisaId("APP0002"))
{
  0x08, 0x5F, 0x48, 0x49, 0x44, 0x0C, 0x06, 0x10, 0x00, 0x02
};

CHAR8 darwin[] =
{  //addresses shifted by 0x24
  0x08, 0x56, 0x45, 0x52, // 00000020    " .. .VER"
  0x30, 0x0D, 0x43, 0x6C, 0x6F, 0x76, 0x65, 0x72, // 00000028    "0.Clover"
  0x20, 0x61, 0x75, 0x74, 0x6F, 0x70, 0x61, 0x74, // 00000030    " autopat"
  0x63, 0x68, 0x65, 0x64, 0x00, 0x08, 0x57, 0x58, // 00000038    "ched..WX"
  0x50, 0x31, 0x0D, 0x57, 0x69, 0x6E, 0x64, 0x6F, // 00000040    "P1.Windo"
  0x77, 0x73, 0x20, 0x32, 0x30, 0x30, 0x31, 0x00, // 00000048    "ws 2001."
  0x14, 0x12, 0x47, 0x45, 0x54, 0x39, 0x02, 0x8C, // 00000050    "..GET9.."
  0x68, 0x69, 0x54, 0x43, 0x48, 0x39, 0xA4, 0x54, // 00000058    "hiTCH9.T"
  0x43, 0x48, 0x39, 0x14, 0x40, 0x05, 0x53, 0x54, // 00000060    "CH9.@.ST"
  0x52, 0x39, 0x02, 0x08, 0x53, 0x54, 0x52, 0x38, // 00000068    "R9..STR8"
  0x11, 0x03, 0x0A, 0x50, 0x08, 0x53, 0x54, 0x52, // 00000070    "...P.STR"
  0x39, 0x11, 0x03, 0x0A, 0x50, 0x70, 0x68, 0x53, // 00000078    "9...PphS"
  0x54, 0x52, 0x38, 0x70, 0x69, 0x53, 0x54, 0x52, // 00000080    "TR8piSTR"
  0x39, 0x70, 0x00, 0x60, 0x70, 0x01, 0x61, 0xA2, // 00000088    "9p.`p.a."
  0x22, 0x61, 0x70, 0x47, 0x45, 0x54, 0x39, 0x53, // 00000090    ""apGET9S"
  0x54, 0x52, 0x38, 0x60, 0x61, 0x70, 0x47, 0x45, // 00000098    "TR8`apGE"
  0x54, 0x39, 0x53, 0x54, 0x52, 0x39, 0x60, 0x62, // 000000A0    "T9STR9`b"
  0xA0, 0x07, 0x92, 0x93, 0x61, 0x62, 0xA4, 0x00, // 000000A8    "....ab.."
  0x75, 0x60, 0xA4, 0x01, 0x14, 0x15, 0x4F, 0x4F, // 000000B0    "u`....OO"
  0x53, 0x49, 0x01, 0xA0, 0x0C, 0x53, 0x54, 0x52, // 000000B8    "SI...STR"
  0x39, 0x57, 0x58, 0x50, 0x31, 0x68, 0xA4, 0x01, // 000000C0    "9WXP1h.."
  0xA4, 0x00                               		  // 000000C8    ".."
  
};

CHAR8 ClassFix[] =	{ 0x00, 0x00, 0x03, 0x00 };   


// for HDA from device_inject.c and mark device_inject function
extern UINT32 HDA_IC_sendVerb(EFI_PCI_IO_PROTOCOL *PciIo, UINT32 codecAdr, UINT32 nodeId, UINT32 verb);
extern UINT32 HDA_getCodecVendorAndDeviceIds(EFI_PCI_IO_PROTOCOL *PciIo);
extern UINT32 getLayoutIdFromVendorAndDeviceId(UINT32 vendorDeviceId);

BOOLEAN get_lpc_model(UINT32 id) {
	int	i;

	for (i=1; i< (sizeof(lpc_chipset) / sizeof(lpc_chipset[0])); i++) {
		if (lpc_chipset[i].id == id) {
			return FALSE;
		}
	}
	return TRUE;
}

BOOLEAN get_ide_model(UINT32 id) {
	int	i;

	for (i=1; i< (sizeof(ide_chipset) / sizeof(ide_chipset[0])); i++) {
		if (ide_chipset[i].id == id) {
			return FALSE;
		}
	}
	return TRUE;
}

BOOLEAN get_ahci_model(UINT32 id) {
	int	i;

	for (i=1; i< (sizeof(ahci_chipset) / sizeof(ahci_chipset[0])); i++) {
		if (ahci_chipset[i].id == id) {
			return FALSE;
		}
	}
	return TRUE;
}

CHAR8* get_net_model(UINT32 id) {
	int	i;

	for (i=1; i< (sizeof(NetChipsets) / sizeof(NetChipsets[0])); i++) {
		if (NetChipsets[i].id == id) {
			return NetChipsets[i].name;
		}
	}
	return NetChipsets[0].name;
}

VOID GetPciADR(IN EFI_DEVICE_PATH_PROTOCOL *DevicePath, OUT UINT32 *Addr1, OUT UINT32 *Addr2, OUT UINT32 *Addr3)
{
  PCI_DEVICE_PATH *PciNode;
  UINTN           PciNodeCount;
  EFI_DEVICE_PATH_PROTOCOL *TmpDevicePath = DuplicateDevicePath(DevicePath);

  // default to 0
  if (Addr1 != NULL) *Addr1 = 0;
  if (Addr2 != NULL) *Addr2 = 0xFFFE; //some code we will consider as "non-exists" b/c 0 is meaningful value
                                      // as well as 0xFFFF
  if (Addr3 != NULL) *Addr3 = 0xFFFE;
  if (!TmpDevicePath) {
    return;
  }
  
  // sanity check - expecting ACPI path for PciRoot
  if (TmpDevicePath->Type != ACPI_DEVICE_PATH && TmpDevicePath->SubType != ACPI_DP) {
    return;
  }
  
  PciNodeCount = 0;
  while (TmpDevicePath && !IsDevicePathEndType(TmpDevicePath)) {
    if (TmpDevicePath->Type == HARDWARE_DEVICE_PATH && TmpDevicePath->SubType == HW_PCI_DP) {
      PciNodeCount++;
      PciNode = (PCI_DEVICE_PATH *)TmpDevicePath;
      if (PciNodeCount == 1 && Addr1 != NULL) {
        *Addr1 = (PciNode->Device << 16) | PciNode->Function;
      } else if (PciNodeCount == 2 && Addr2 != NULL) {
        *Addr2 = (PciNode->Device << 16) | PciNode->Function;
      } else if (PciNodeCount == 3 && Addr3 != NULL) {
        *Addr3 = (PciNode->Device << 16) | PciNode->Function;
      } else {
        break;
      }
    }
    TmpDevicePath = NextDevicePathNode(TmpDevicePath);
  }
  return;
}

/* Discussion
 Native USB mean for those chipsets IOUSBFamily set some "errata"
 for example native 0x1cXX has no such errata */
BOOLEAN NativeUSB(UINT16 DID)
{
  UINT16 d = DID & 0xFF00;
  return ((d == 0x2600) || (d == 0x2700) || (d == 0x2800) || (d == 0x3a00) || (d == /*NFORCE_USB->*/0x0a00));
}

VOID CheckHardware()
{
  EFI_STATUS          Status;
	EFI_HANDLE          *HandleBuffer = NULL;
  EFI_HANDLE          Handle;
	EFI_PCI_IO_PROTOCOL *PciIo;
	PCI_TYPE00          Pci;
	UINTN               HandleCount = 0;
	UINTN               HandleIndex;
  
  //	UINT16		  did, vid;
	UINTN               Segment;
	UINTN               Bus;
	UINTN               Device;
	UINTN               Function;
	UINTN               display=0;
	UINTN               gfxid=0;
  
	pci_dt_t            PCIdevice;
	EFI_DEVICE_PATH_PROTOCOL *DevicePath = NULL;
	
   usb=0;
  // Scan PCI handles 
  Status = gBS->LocateHandleBuffer (
                                    ByProtocol,
                                    &gEfiPciIoProtocolGuid,
                                    NULL,
                                    &HandleCount,
                                    &HandleBuffer
                                    );
  if (!EFI_ERROR (Status)) {
//    DBG("PciIo handles count=%d\n", HandleCount);
    for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
      Handle = HandleBuffer[HandleIndex];
      Status = gBS->HandleProtocol (
                                    Handle,
                                    &gEfiPciIoProtocolGuid,
                                    (VOID **)&PciIo
                                    );
      if (!EFI_ERROR (Status)) {
        UINT32 deviceid;
        /* Read PCI BUS */
        Status = PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
        Status = PciIo->Pci.Read (
                                  PciIo,
                                  EfiPciIoWidthUint32,
                                  0,
                                  sizeof (Pci) / sizeof (UINT32),
                                  &Pci
                                  );
        
        deviceid = Pci.Hdr.DeviceId | (Pci.Hdr.VendorId << 16);
        
        // add for auto patch dsdt get DSDT Device _ADR
        PCIdevice.DeviceHandle = Handle;
        DevicePath = DevicePathFromHandle (Handle);
        if (DevicePath) {
  //        DBG("Device patch = %s \n", DevicePathToStr(DevicePath));
          
          //Display ADR
          if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_DISPLAY) &&
              (Pci.Hdr.ClassCode[1] == PCI_CLASS_DISPLAY_VGA)) {
            UINT32 dadr1, dadr2;
            PCI_IO_DEVICE *PciIoDevice;

            GetPciADR(DevicePath, &DisplayADR1[display], &DisplayADR2[display], NULL);
            DBG("VideoCard devID=0x%x\n", ((Pci.Hdr.DeviceId << 16) | Pci.Hdr.VendorId));
            dadr1 = DisplayADR1[display];
            dadr2 = DisplayADR2[display];
            DBG("DisplayADR1[%d] = 0x%x, DisplayADR2[%d] = 0x%x\n", display, dadr1, display, dadr2);
                 dadr2 = dadr1; //to avoid warning "unused variable" :(
            DisplayVendor[display] = Pci.Hdr.VendorId;
            DisplayID[display] = Pci.Hdr.DeviceId;
            DisplaySubID[display] = (Pci.Device.SubsystemID << 16) | (Pci.Device.SubsystemVendorID << 0);
            // for get display data
            Displaydevice[display].DeviceHandle = HandleBuffer[HandleIndex];
            Displaydevice[display].dev.addr = (UINT32)PCIADDR(Bus, Device, Function);
            Displaydevice[display].vendor_id = Pci.Hdr.VendorId;
            Displaydevice[display].device_id = Pci.Hdr.DeviceId;
            Displaydevice[display].revision = Pci.Hdr.RevisionID;
            Displaydevice[display].subclass = Pci.Hdr.ClassCode[0];
            Displaydevice[display].class_id = *((UINT16*)(Pci.Hdr.ClassCode+1));
            Displaydevice[display].subsys_id.subsys.vendor_id = Pci.Device.SubsystemVendorID;
            Displaydevice[display].subsys_id.subsys.device_id = Pci.Device.SubsystemID;
            //
            // Detect if PCI Express Device
            //
            //
            // Go through the Capability list
            //
            PciIoDevice = PCI_IO_DEVICE_FROM_PCI_IO_THIS (PciIo);
            if (PciIoDevice->IsPciExp) {
              if (display==0)
                Display1PCIE = TRUE;
              else
                Display2PCIE = TRUE;
            }  
            DBG("Display %d is %a PCIE\n", display, (PciIoDevice->IsPciExp)?"":"not");
            display++;
          }
          
          //Network ADR
          if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_NETWORK) &&
              (Pci.Hdr.ClassCode[1] == PCI_CLASS_NETWORK_ETHERNET)) {
            GetPciADR(DevicePath, &NetworkADR1, &NetworkADR2, NULL);
 //           DBG("NetworkADR1 = 0x%x, NetworkADR2 = 0x%x\n", NetworkADR1, NetworkADR2);
            Netmodel = get_net_model(deviceid);            
          }
          
          //Network WiFi ADR
          if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_NETWORK) &&
              (Pci.Hdr.ClassCode[1] == PCI_CLASS_NETWORK_OTHER)) {
            GetPciADR(DevicePath, &ArptADR1, &ArptADR2, NULL);
   //         DBG("ArptADR1 = 0x%x, ArptADR2 = 0x%x\n", ArptADR1, ArptADR2);
   //         Netmodel = get_arpt_model(deviceid);  
            ArptBCM = (Pci.Hdr.VendorId == 0x14e4);
            ArptAtheros = (Pci.Hdr.VendorId == 0x168c);
          }
          
          //Firewire ADR
          if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_SERIAL) &&
              (Pci.Hdr.ClassCode[1] == PCI_CLASS_SERIAL_FIREWIRE)) {
            GetPciADR(DevicePath, &FirewireADR1, &FirewireADR2, NULL);
 //           DBG("FirewireADR1 = 0x%x, FirewireADR2 = 0x%x\n", FirewireADR1, FirewireADR2);
          }
          
          //SBUS ADR
          if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_SERIAL) &&
              (Pci.Hdr.ClassCode[1] == PCI_CLASS_SERIAL_SMB)) {
            GetPciADR(DevicePath, &SBUSADR1, &SBUSADR2, NULL);
  //          DBG("SBUSADR1 = 0x%x, SBUSADR2 = 0x%x\n", SBUSADR1, SBUSADR2);
          }
          
          //USB
          if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_SERIAL) &&
              (Pci.Hdr.ClassCode[1] == PCI_CLASS_SERIAL_USB)) {
            UINT16 DID = Pci.Hdr.DeviceId;
            USBIntel = (Pci.Hdr.VendorId == 0x8086);
            USBNForce = (Pci.Hdr.VendorId == 0x10de);
            GetPciADR(DevicePath, &USBADR[usb], &USBADR2[usb], &USBADR3[usb]);
            DBG("USBADR[%d] = 0x%x and PCIe = 0x%x\n", usb, USBADR[usb], USBADR2[usb]);
            if (USBIDFIX)
            {
              if (USBADR[usb] == 0x001D0000 && !NativeUSB(DID)) DID = 0x3a34;
              if (USBADR[usb] == 0x001D0001 && !NativeUSB(DID)) DID = 0x3a35;
              if (USBADR[usb] == 0x001D0002 && !NativeUSB(DID)) DID = 0x3a36;
              if (USBADR[usb] == 0x001D0003 && !NativeUSB(DID)) DID = 0x3a37;
              if (USBADR[usb] == 0x001A0000 && !NativeUSB(DID)) DID = 0x3a37;
              if (USBADR[usb] == 0x001A0001 && !NativeUSB(DID)) DID = 0x3a38;
              if (USBADR[usb] == 0x001A0002 && !NativeUSB(DID)) DID = 0x3a39;
              if (USBADR[usb] == 0x001D0007 && !NativeUSB(DID)) DID = 0x3a3a;
              if (USBADR[usb] == 0x001A0007 && !NativeUSB(DID)) DID = 0x3a3c;
              //NFORCE_USB_START
              if (USBADR3[usb] == 0x00040000 && !NativeUSB(DID)) DID = 0x0aa5;
              if (USBADR3[usb] == 0x00040001 && !NativeUSB(DID)) DID = 0x0aa6;
              if (USBADR3[usb] == 0x00060000 && !NativeUSB(DID)) DID = 0x0aa7;
              if (USBADR3[usb] == 0x00060001 && !NativeUSB(DID)) DID = 0x0aa9;
              //NFORCE_USB_END
            }       
            USBID[usb] = DID;
            USB20[usb] = (Pci.Hdr.ClassCode[0] == 0x20)?1:0;
            USB30[usb] = (Pci.Hdr.ClassCode[0] == 0x30)?1:0;
            USB40[usb] = (Pci.Hdr.ClassCode[0] == 0x20)?1:0;
            usb++;
          }
          
          // HDA Audio
          if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_MEDIA) &&
              (Pci.Hdr.ClassCode[1] == PCI_CLASS_MEDIA_HDA)) {
             UINT32 codecId = 0, layoutId = 0;
            GetPciADR(DevicePath, &HDAADR1, NULL, NULL);
  //          DBG("HDAADR = 0x%x\n", HDAADR1);
            codecId = HDA_getCodecVendorAndDeviceIds(PciIo);
            if (codecId > 0) {
              layoutId = getLayoutIdFromVendorAndDeviceId(codecId);
              if (layoutId == 0) {
                layoutId = 12;
              }
            }
            if (gSettings.HDALayoutId > 0) {
              // layoutId is specified - use it
              layoutId = (UINT32)gSettings.HDALayoutId;
              DBG(" setting specified layout-id=%d (0x%x)\n", layoutId, layoutId);
            }
            if (layoutId > 0) {
              HDAFIX = TRUE;
              HDAcodecId = codecId;
              HDAlayoutId = layoutId;
            } else {
              GFXHDAFIX = TRUE;
              GfxcodecId[gfxid] = codecId;
              GfxlayoutId[gfxid] = layoutId;
              gfxid++;
            }                                   
          }
          
          // LPC
          if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_BRIDGE) &&
              (Pci.Hdr.ClassCode[1] == PCI_CLASS_BRIDGE_ISA)) {
            LPCBFIX = get_lpc_model(deviceid);
          }
          
          // IDE device
          if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_MASS_STORAGE) &&
              (Pci.Hdr.ClassCode[1] == PCI_CLASS_MASS_STORAGE_IDE)) {
            GetPciADR(DevicePath, &IDEADR1, &IDEADR2, NULL);
  //          DBG("IDEADR1 = 0x%x, IDEADR2 = 0x%x\n", IDEADR1, IDEADR2);
            IDEFIX = get_ide_model(deviceid);
            IDEVENDOR = Pci.Hdr.VendorId;
          }
          
          // SATA 
          if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_MASS_STORAGE) &&
              (Pci.Hdr.ClassCode[1] == PCI_CLASS_MASS_STORAGE_SATADPA) &&
              (Pci.Hdr.ClassCode[0] == 0x00)) {
            GetPciADR(DevicePath, &SATAADR1, &SATAADR2, NULL);
 //           DBG("SATAADR1 = 0x%x, SATAADR2 = 0x%x\n", SATAADR1, SATAADR2);
            SATAFIX = get_ide_model(deviceid);
            SATAVENDOR = Pci.Hdr.VendorId;
          }
          
          // SATA AHCI
          if ((Pci.Hdr.ClassCode[2] == PCI_CLASS_MASS_STORAGE) &&
              (Pci.Hdr.ClassCode[1] == PCI_CLASS_MASS_STORAGE_SATADPA) &&
              (Pci.Hdr.ClassCode[0] == 0x01)) {
            GetPciADR(DevicePath, &SATAAHCIADR1, &SATAAHCIADR2, NULL);
  //          DBG("SATAAHCIADR1 = 0x%x, SATAAHCIADR2 = 0x%x\n", SATAAHCIADR1, SATAAHCIADR2);
            //AHCIFIX = get_ahci_model(deviceid);
            SATAAHCIVENDOR = Pci.Hdr.VendorId;
          }
        }
        // detected finish						
      }
    }
  }
}

VOID findCPU(UINT8* dsdt, UINT32 length)
{
	UINT32 i;
	
	acpi_cpu_count = 0;
	
	for (i=0; i<length-20; i++) {
		if (dsdt[i] == 0x5B && dsdt[i+1] == 0x83) { // ProcessorOP
			UINT32 offset = i + 3 + (dsdt[i+2] >> 6);			
			BOOLEAN add_name = TRUE;
			UINT8 j;
			
			for (j=0; j<4; j++) {
				char c = dsdt[offset+j];
				if( c == '\\') {
				    offset = i + 8 + (dsdt[i+7] >> 6);
				    c = dsdt[offset+j];
				}
		
				if (!(IS_UPPER(c) || IS_DIGIT(c) || c == '_')) {
					add_name = FALSE;
					DBG("Invalid character found in ProcessorOP 0x%x!\n", c);
					break;
				}
			}
			
			if (add_name) {
				acpi_cpu_name[acpi_cpu_count] = AllocateZeroPool(5);
				CopyMem(acpi_cpu_name[acpi_cpu_count], dsdt+offset, 4);
				i = offset + 5;
                
                //if (acpi_cpu_count == 0)
                //    acpi_cpu_p_blk = dsdt[i] | (dsdt[i+1] << 8);
				
				if (acpi_cpu_count == 0) {
				    DBG("Found ACPI CPU: %a ", acpi_cpu_name[acpi_cpu_count]);
				} else { 
				    DBG("And %a ", acpi_cpu_name[acpi_cpu_count]);
				}    
				if (++acpi_cpu_count == 32)
				    break;
			}
		}
	}
	DBG("\n");
  
  if (!acpi_cpu_count) {
    for (i=0; i<15; i++) {
      acpi_cpu_name[i] = AllocateZeroPool(5);
      AsciiSPrint(acpi_cpu_name[i], 5, "CPU%1x", i);
    }
  }
	return;
}


//                start => move data start address
//                offset => data move how many byte 
//                len => initial length of the buffer
// return final length of the buffer
// we suppose that buffer allocation is more then len+offset
UINT32 move_data(UINT32 start, UINT8* buffer, UINT32 len, INT32 offset)
{
  UINT32 i;
  
  if (offset<0) {
    for (i=start; i<len+offset; i++) {
      buffer[i] = buffer[i-offset];
    }
  }
  else  { // data move to back        
    for (i=len-1; i>=start; i--) {
      buffer[i+offset] = buffer[i];
    }
  }
  return len + offset;
}

UINT32 get_size(UINT8* Buffer, UINT32 adr)
{
	UINT32 temp;
	
	temp = Buffer[adr] & 0xF0; //keep bits 0x30 to check if this is valid size field
	
	if(temp <= 0x30)	{		    // 0
		temp = Buffer[adr];		
	}
	else if(temp == 0x40)	{   // 4
		temp =  (Buffer[adr]   - 0x40)  << 0|
		         Buffer[adr+1]          << 4;
	}
	else if(temp == 0x80)	{ 	// 8
		temp = (Buffer[adr]   - 0x80)  <<  0|
		        Buffer[adr+1]          <<  4|
		        Buffer[adr+2]          << 12;	
	}	
	else if(temp == 0xC0)	{ 	// C
		temp = (Buffer[adr]   - 0xC0) <<  0|
		        Buffer[adr+1]         <<  4|
		        Buffer[adr+2]         << 12|
		        Buffer[adr+3]         << 20;	
	} 
  else {
    DBG("wrong pointer to size field at %x\n", adr);
    return 0;  //this means wrong pointer to size field
  }
	return temp;
}

//return 1 if new size is two bytes else 0
UINT32 write_offset(UINT32 adr, UINT8* buffer, UINT32 len, INT32 offset)
{
  UINT32 i, shift = 0;
  UINT32 size = offset + 1;
  
  if (size >= 0x3F) {
    for (i=len; i>adr; i--) {
      buffer[i+1] = buffer[i];
    }
    shift = 1;
    size += 1;
  }
  aml_write_size(size, (CHAR8 *)buffer, adr);
  return shift;
}

/*
  adr - a place to write new size. Size of some object.
  buffer - the binary aml codes array
  len - its length
  sizeoffset - how much the object increased in size
  return address shift from original  +/- n from outers
 When we increase the object size there is a chance that new size field +1
 so out devices should also be corrected +1 and this may lead to new shift 
*/
//Slice - I excluded check (oldsize <= 0x0fffff && size > 0x0fffff)
//because I think size of DSDT will never be 1Mb
INT32 write_size(UINT32 adr, UINT8* buffer, UINT32 len, INT32 sizeoffset)
{
  UINT32 size, oldsize;
  INT32 offset = 0;
  oldsize = get_size(buffer, adr);
  if (!oldsize) {
    return 0; //wrong address, will not write here
  }
  size = oldsize + sizeoffset;
  // data move to back
  if (oldsize <= 0x3f && size > 0x0fff) {
    offset = 2;
  } else if ((oldsize <= 0x3f && size > 0x3f) || (oldsize<=0x0fff && size > 0x0fff)) {
    offset = 1;
  }  // data move to front
  else if ((size <= 0x3f && oldsize > 0x3f) || (size<=0x0fff && oldsize > 0x0fff)) {
    offset = -1;
  }  else if (oldsize > 0x0fff && size <= 0x3f) {
    offset = -2;
  }
  len = move_data(adr, buffer, len, offset);
  size += offset;
  aml_write_size(size, (CHAR8 *)buffer, adr); //reuse existing codes  
	return offset;
}

//the procedure can find BIN array UNSIGNED CHAR8 sizeof N inside part of large array "dsdt" size of len
INT32 FindBin (UINT8 *dsdt, UINT32 len, UINT8* bin, UINTN N)
{
  UINT32 i, j;
  BOOLEAN eq;
  
  for (i=0; i<len-N; i++) {
    eq = TRUE;
    for (j=0; j<N; j++) {
      if (dsdt[i+j] != bin[j]) {
        eq = FALSE;
        break;
      }
    }
    if (eq) {
      return (INT32)i;
    }
  }
  return -1;
}
                
//if (!FindMethod(dsdt, len, "DTGP")) 
// return address of size field. Assume size not more then 0x0FFF = 4095 bytes
UINT32 FindMethod (UINT8 *dsdt, UINT32 len, /* CONST*/ CHAR8* Name)
{
  UINT32 i;
  for (i=10; i<len-7; i++) {
    if (((dsdt[i] == 0x14) || (dsdt[i+1] == 0x14) || (dsdt[i-1] == 0x14)) &&
        (dsdt[i+3] == Name[0]) && (dsdt[i+4] == Name[1]) &&
        (dsdt[i+5] == Name[2]) && (dsdt[i+6] == Name[3])
        ){
      if (dsdt[i-1] == 0x14) return i;
      return (dsdt[i+1] == 0x14)?(i+2):(i+1); //pointer to size field 
    }
  }
  return 0;
}

//return final length of dsdt
UINT32 CorrectOuters (UINT8 *dsdt, UINT32 len, UINT32 adr,  INT32 shift)
{
  INT32    i, j, k;
  UINT32   size = 0;
  INT32  offset = 0;
  UINT32   SBSIZE = 0, SBADR = 0;

  if (shift == 0) {
    return len;
  }
  
  i = adr; //usually adr = @5B - 1 = sizefield - 3
  while (i>0x20) {  //find devices that previous to adr
    //check device
    k = i + 2;
    if ((dsdt[i] == 0x5B) && (dsdt[i+1] == 0x82) && (dsdt[i-1] != 0x0A)) { //device candidate      
      size = get_size(dsdt, k);
      if (size) {
        if ((k+size) > adr+4) {  //Yes - it is outer
              DBG("found outer device begin=%x end=%x\n", k, k+size);
          offset = write_size(k, dsdt, len, shift);  //size corrected to sizeoffset at address j
          shift += offset;
          len += offset;
        }  //else not an outer device          
      } //else wrong size field - not a device
    } //else not a device
// check scope
    SBSIZE = 0;
    if (dsdt[i] == '_' && dsdt[i+1] == 'S' && dsdt[i+2] == 'B' && dsdt[i+3] == '_') {
      for (j=0; j<10; j++) {
        if ((dsdt[i-j] == 0x10) && (dsdt[i-j-1] != 0x0A)) {
          SBADR = i-j+1;
          SBSIZE = get_size(dsdt, SBADR);
       //     DBG("found Scope(\\_SB) address = 0x%08x size = 0x%08x\n", SBADR, SBSIZE);
          if ((SBSIZE != 0) && (SBSIZE < len)) {  //if zero or too large then search more
            break;
          }          
        }
      }
      if (SBSIZE) { //if found
        k = SBADR - 6;
        if ((SBADR + SBSIZE) > adr+4) {  //Yes - it is outer
              DBG("found outer scope begin=%x end=%x\n", SBADR, SBADR+SBSIZE);
          offset = write_size(SBADR, dsdt, len, shift); 
          shift += offset;
          len += offset;
        }  //else not an outer scope          
      }
    } //else not a scope
    i = k - 3;    //if found then search again from found 
  }  
  return len;
}

//ReplaceName(dsdt, len, "AZAL", "HDEF");
VOID ReplaceName(UINT8 *dsdt, UINT32 len, /* CONST*/ CHAR8 *OldName, /* CONST*/ CHAR8 *NewName)
{
  UINTN i;
  for (i=10; i<len; i++) {
    if ((dsdt[i+0] == NewName[0]) && (dsdt[i+1] == NewName[1]) &&
        (dsdt[i+2] == NewName[2]) && (dsdt[i+3] == NewName[3])) {
      DBG("Name %a already present, renaming impossibble\n", NewName);
      return;
    }
  }
  
  for (i=10; i<len; i++) {
    if ((dsdt[i+0] == OldName[0]) && (dsdt[i+1] == OldName[1]) &&
        (dsdt[i+2] == OldName[2]) && (dsdt[i+3] == OldName[3])) {
      DBG("Name %a present at 0x%x, renaming to %a\n", OldName, i, NewName);
      dsdt[i+0] = NewName[0];
      dsdt[i+1] = NewName[1];
      dsdt[i+2] = NewName[2];
      dsdt[i+3] = NewName[3];      
    }       
  }
}

// if (CmpAdr(dsdt, j, NetworkADR1))
// Name (_ADR, 0x90000)                
BOOLEAN CmpAdr (UINT8 *dsdt, UINT32 j, UINT32 PciAdr)
{ 
  // Name (_ADR, 0x001f0001)
  return (BOOLEAN)
         ((dsdt[j + 4] == 0x08) &&
          (dsdt[j + 5] == 0x5F) &&
          (dsdt[j + 6] == 0x41) &&
          (dsdt[j + 7] == 0x44) &&
          (dsdt[j + 8] == 0x52) &&
          (//--------------------
           ((dsdt[j +  9] == 0x0C) &&
            (dsdt[j + 10] == ((PciAdr & 0x000000ff) >> 0)) &&
            (dsdt[j + 11] == ((PciAdr & 0x0000ff00) >> 8)) &&
            (dsdt[j + 12] == ((PciAdr & 0x00ff0000) >> 16)) &&
            (dsdt[j + 13] == ((PciAdr & 0xff000000) >> 24))
           ) ||
           //--------------------
           ((dsdt[j +  9] == 0x0B) &&
            (dsdt[j + 10] == ((PciAdr & 0x000000ff) >> 0)) &&
            (dsdt[j + 11] == ((PciAdr & 0x0000ff00) >> 8)) &&
            (PciAdr < 0x10000)
           ) ||
           //-----------------------
           ((dsdt[j +  9] == 0x0A) &&
            (dsdt[j + 10] == (PciAdr & 0x000000ff)) &&
            (PciAdr < 0x100)
           ) ||
           //-----------------
           ((dsdt[j +  9] == 0x00) && (PciAdr == 0)) ||    
           //------------------
           ((dsdt[j +  9] == 0x01) && (PciAdr == 1)) 
          )
        );
}

BOOLEAN CmpPNP (UINT8 *dsdt, UINT32 j, UINT16 PNP)
{
  // Name (_HID, EisaId ("PNP0C0F")) for PNP=0x0C0F BigEndian
  if (PNP == 0) {
    return (BOOLEAN)
           ((dsdt[j + 0] == 0x08) &&
            (dsdt[j + 1] == 0x5F) &&
            (dsdt[j + 2] == 0x48) &&
            (dsdt[j + 3] == 0x49) &&
            (dsdt[j + 4] == 0x44) &&
            (dsdt[j + 5] == 0x0B) &&
            (dsdt[j + 6] == 0x41) &&
            (dsdt[j + 7] == 0xD0));
  }
  return (BOOLEAN)
         ((dsdt[j + 0] == 0x08) &&
          (dsdt[j + 1] == 0x5F) &&
          (dsdt[j + 2] == 0x48) &&
          (dsdt[j + 3] == 0x49) &&
          (dsdt[j + 4] == 0x44) &&
          (dsdt[j + 5] == 0x0C) &&
          (dsdt[j + 6] == 0x41) &&
          (dsdt[j + 7] == 0xD0) &&
          (dsdt[j + 8] == ((PNP & 0xff00) >> 8)) &&
          (dsdt[j + 9] == ((PNP & 0x00ff) >> 0)));
}

//the procedure search nearest "Device" code before given address
//should restrict the search by 6 bytes... OK, 10, .. until dsdt begin
//hmmm? will check device name
UINT32 devFind(UINT8 *dsdt, UINT32 address)
{
  UINT32 k = address;
  INT32 size = 0;
  while (k > 30) {
    k--;
    if (dsdt[k] == 0x82 && dsdt[k-1] == 0x5B) {
      size = get_size(dsdt, k+1);
      if (!size) {
        continue;
      }
      if ((k + size + 1) > address) {
        return (k+1); //pointer to size
      }  //else continue
    }
  }
/*  for (k=address; k>address-10; k--) {
    if (dsdt[k] == 0x82 && dsdt[k-1] == 0x5B) {
      return (k+1); //pointer to size
    }
  } */
  DBG("Device definition before adr=%x not found\n", address);
  return 0; //impossible value for fool proof  
}

//len = DeleteDevice("AZAL", dsdt, len);
UINT32 DeleteDevice(CONST CHAR8 *Name, UINT8 *dsdt, UINT32 len)
{
  UINT32 i, j;
  INT32 size = 0, sizeoffset;
  DBG(" deleting device %a\n", Name);
  for (i=20; i<len; i++) {
    if ((dsdt[i+0] == Name[0]) && (dsdt[i+1] == Name[1]) &&
        (dsdt[i+2] == Name[2]) && (dsdt[i+3] == Name[3]) &&
        ((dsdt[i-3] == 0x82) || (dsdt[i-2] == 0x82)) && 
        ((dsdt[i-4] == 0x5B) || (dsdt[i-3] == 0x5B))) {
      if ((dsdt[i-3] == 0x82) && (dsdt[i-4] == 0x5B)) {
        j = i - 2;
      } else {
        j = i - 1;
      }
      size = get_size(dsdt, j);
      if (!size) {
        continue;
      }
      sizeoffset = - 2 - size;
      len = move_data(j-2, dsdt, len, sizeoffset);
      //to correct outers we have to calculate offset
      len = CorrectOuters(dsdt, len, j-3, sizeoffset);
      break;
    }
  }  
  return len;
}

UINT32 GetPciDevice(UINT8 *dsdt, UINT32 len)
{
  UINT32 i;
  UINT32 PCIADR = 0, PCISIZE = 0;
  for (i=20; i<len; i++) {
    // Find Device PCI0   // PNP0A03
    if (CmpPNP(dsdt, i, 0x0A03)) {
      PCIADR = devFind(dsdt, i);
      if (!PCIADR) {
        continue;
      }

      PCISIZE = get_size(dsdt, PCIADR);
      if (PCISIZE) {
        break;
      }
    } // End find  
  }
  if (!PCISIZE) {
    for (i=20; i<len; i++) {
      // Find Device PCIE   // PNP0A08
      if (CmpPNP(dsdt, i, 0x0A08)) {
        PCIADR = devFind(dsdt, i);
        if (!PCIADR) {
          continue;
        }

        PCISIZE = get_size(dsdt, PCIADR);
        if (PCISIZE) {
          break;
        }
      } // End find   
    }
  }
  if (PCISIZE)
    return PCIADR;
  return 0;
}

// Find PCIRootUID and all need Fix Device 
UINTN  findPciRoot (UINT8 *dsdt, UINT32 len)
{
	UINTN    j;
	UINTN    root = 0;
  UINT32 PCIADR, PCISIZE;

  //initialising
  NetworkName   = FALSE;
  DisplayName1  = FALSE;
  DisplayName2  = FALSE;
  FirewireName  = FALSE;
  ArptName      = FALSE;
  XhciName      = FALSE;
  
  PCIADR = GetPciDevice(dsdt, len);
  if (PCIADR) {
    PCISIZE = get_size(dsdt, PCIADR);
  }
  //sample
  /*
   5B 82 8A F1 05 50 43 49 30         Device (PCI0)	{
   08 5F 48 49 44 0C 41 D0 0A 08        Name (_HID, EisaId ("PNP0A08"))
   08 5F 43 49 44 0C 41 D0 0A 03        Name (_CID, EisaId ("PNP0A03"))
   08 5F 41 44 52 00                    Name (_ADR, Zero)
   14 09 5E 42 4E 30 30 00 A4 00        Method (^BN00, 0, NotSerialized) {Return (Zero)}
   14 0B 5F 42 42 4E 00 A4 42 4E 30 30  Method (_BBN, 0, NotSerialized) {Return (BN00 ())}
   08 5F 55 49 44 00                    Name (_UID, Zero)
   14 16 5F 50 52 54 00                 Method (_PRT, 0, NotSerialized)
   */

  // find PCIRootUID
  for (j=PCIADR; j<PCIADR+64; j++) {
    if (dsdt[j] == '_' && dsdt[j+1] == 'U' && dsdt[j+2] == 'I' && dsdt[j+3] == 'D') {
      // Slice - I want to set root to zero instead of keeping original value
      if (dsdt[j+4] == 0x0A)
        dsdt[j+5] = 0;  //AML_BYTE_PREFIX followed by a number
      else
        dsdt[j+4] = 0;  //any other will be considered as ONE or WRONG, replace to ZERO
      DBG("found PCIROOTUID = %d\n", root);
      break;
    }	
  }  
	return root;
}


// read device name, replace to ADP1
//check for
/* Name (_PRW, Package (0x02)
 {
 0x1C, 
 0x03
 }) */
//if absent - add it 

/*
 5B 82 4B 04 41 44 50 31            //device (ADP1)
 08 5F 48 49 44 0D                  //	name (_HID.
 41 43 50 49 30 30 30 33 00         // ACPI0003
 08 5F 50 52 57 12 06 02 0A 1C 0A 03	//.._PRW..
 
 */

UINT32 FixADP1 (UINT8* dsdt, UINT32 len)
{
  UINT32 i, j;
  UINT32 adr, size; 
  INT32 sizeoffset, shift;
  CHAR8 Name[4];
  DBG("Start ADP1 fix\n");
  shift = FindBin(dsdt, len, (UINT8*)acpi3, sizeof(acpi3));
  if (shift < 0) {
    // not found - create new one or do nothing
    DBG("no device(AC) exists\n");
    return len;
  }
  adr = devFind(dsdt, (UINT32)shift);
  if (!adr) {
    return len;
  }
  size = get_size(dsdt, adr);
  //check name and replace
  if (size < 0x40) {
    j = adr + 1;
  } else {
    j = adr + 2;
  }
  for (i=0; i<4; i++) {
    Name[i] = dsdt[j+i];
  } 
  ReplaceName(dsdt, len, Name, "ADP1");  
  //find PRW
  if(FindBin(dsdt+adr, size, (UINT8*)prw1c, 8) >= 0){
    DBG("_prw is present\n");
    return len;
  }  
  j = adr + size;
  sizeoffset = sizeof(prw1c);
  len = move_data(j, dsdt, len, sizeoffset);
  CopyMem(dsdt + j, prw1c, sizeoffset);
  shift = write_size(adr, dsdt, len, sizeoffset);
  sizeoffset += shift;
  len += shift;
  len = CorrectOuters(dsdt, len, adr - 3, sizeoffset);
  return len;
}

UINT32 FixAny (UINT8* dsdt, UINT32 len, UINT8* ToFind, UINT32 LenTF, UINT8* ToReplace, UINT32 LenTR)
{
  INT32 sizeoffset, adr;
  UINT32 i;
  BOOLEAN found = FALSE;
  if (!ToFind) {
    return len;
  }
  DBG("Patch DSDT %01x%01x%01x%01x\n", ToFind[0], ToFind[1], ToFind[2], ToFind[3]);
  sizeoffset = LenTR - LenTF;
  for (i = 20; i < len; i++) {
    adr = FindBin(dsdt + i, len, ToFind, LenTF);
    if (adr < 0) {
      if (!found) {
        DBG("  bin not found\n");
      }
      return len;
    }
    DBG("  Patch at %x\n", adr);
    found = TRUE;
    len = move_data(adr + i, dsdt, len, sizeoffset);
    if ((LenTR > 0) && (ToReplace != NULL)) {
      CopyMem(dsdt + adr + i, ToReplace, LenTR);
    }
    len = CorrectOuters(dsdt, len, adr + i - 3, sizeoffset);
  }

  return len;
}


UINT32 FIXDarwin (UINT8* dsdt, UINT32 len)
{
  CONST UINT32  adr  = 0x24;
  DBG("Start Darwin Fix\n");
  ReplaceName(dsdt, len, "_OSI", "OOSI");
  len = move_data(adr, dsdt, len, sizeof(darwin));
  CopyMem(dsdt+adr, darwin, sizeof(darwin));
  return len;  
}

VOID FixS3D (UINT8* dsdt, UINT32 len)
{
  UINT32 i;
  DBG("Start _S3D Fix\n");
  for (i=20; i<len-5; i++) {
    if ((dsdt[i + 0] == 0x08) &&
        (dsdt[i + 1] == '_') &&
        (dsdt[i + 2] == 'S') &&
        (dsdt[i + 3] == '3') &&
        (dsdt[i + 4] == 'D') &&
        (dsdt[i + 5] == 0x0A) &&
        (dsdt[i + 6] == 0x02)) {
      dsdt[i + 6] = 3;
    }
  }
}  

UINT32 AddPNLF (UINT8 *dsdt, UINT32 len)
{
  UINT32 i; //, j, size;
  UINT32  adr  = 0;
  DBG("Start PNLF Fix\n");

  if (FindBin(dsdt, len, (UINT8*)app2, 10) >= 0) {
    return len; //the device already exists
  }
  //search  PWRB PNP0C0C
  for (i=0x20; i<len-6; i++) {
    if (CmpPNP(dsdt, i, 0x0C0C)) {
      DBG("found PWRB at %x\n", i);
      adr = devFind(dsdt, i);
      break;
    }
  }
  if (!adr) {
    //search battery
    DBG("not found PWRB, look BAT0\n");
    for (i=0x20; i<len-6; i++) {
      if (CmpPNP(dsdt, i, 0x0C0A)) {
        adr = devFind(dsdt, i);
        DBG("found BAT0 at %x\n", i);
        break;
      }
    }
  }
  if (!adr) {
    return len;
  }
  i = adr - 2;
  len = move_data(i, dsdt, len, sizeof(pnlf));
  CopyMem(dsdt+i, pnlf, sizeof(pnlf));
  len = CorrectOuters(dsdt, len, adr-3, sizeof(pnlf));
	return len;  
}

UINT32 FixRTC (UINT8 *dsdt, UINT32 len)
{
	UINT32 i, j, k, l;
	UINT32 IOADR   = 0;
	UINT32 RESADR  = 0;
  UINT32 adr     = 0;
  UINT32 rtcsize = 0;
  INT32  shift, sizeoffset  = 0;
  
  DBG("Start RTC Fix\n");
  
  for (j=20; j<len; j++) {
    // Find Device RTC // Name (_HID, EisaId ("PNP0B00")) for RTC
    if (CmpPNP(dsdt, j, 0x0B00))
    {
      adr = devFind(dsdt, j);
      if (!adr) {
        continue;
      }
      rtcsize = get_size(dsdt, adr);
      if (rtcsize) {
        break;
      }
    } // End RTC    
  }
    
  if (!rtcsize) {
    DBG("BUG! rtcsize not found\n");
    return len;
  }

	for (i=adr+4; i<adr+rtcsize; i++) {
	  // IO (Decode16, ((0x0070, 0x0070)) =>> find this
    if (dsdt[i] == 0x70 && dsdt[i+1] == 0x00 && dsdt[i+2] == 0x70 && dsdt[i+3] == 0x00) 
    {   
      // First Fix RTC CMOS Reset Problem
      if (dsdt[i+4] != 0x00 || dsdt[i+5] != 0x02)  //dsdt[j+4] => Alignment  dsdt[j+5] => Length
      {
        dsdt[i+4] = 0x00;  //Alignment
        dsdt[i+5] = 0x02;  //Length
        DBG("found RTC Length not match, Maybe will case CMOS reset will patch it.\n");
      }
      for (l=adr+4; l<i; l++)
      {
        if (dsdt[l] == 0x11 && dsdt[l+2] == 0x0A)
        {
          RESADR = l+1;  //Format 11, size, 0A, size-3,... 79, 00
          IOADR = l+3;  //IO (Decode16 ==> 47, 01
        }  
      }        
      break;
    }
    if ((dsdt[i+1] == 0x5B) && (dsdt[i+2] == 0x82)) {
      break; //end of RTC device and begin of new Device()
    }    
  }
  
  for (l=adr+4; l<adr+rtcsize; l++)
  {
    if ((dsdt[l] == 0x22) && (l>IOADR) && (l<IOADR+dsdt[IOADR]))  // Had IRQNoFlag
    {
      for (k=l; k<l+20; k++)   
      {
        if ((dsdt[k] == 0x79) || ((dsdt[k] == 0x47) && (dsdt[k+1] == 0x01)) ||
            ((dsdt[k] == 0x86) && (dsdt[k+1] == 0x09)))
        {
          sizeoffset = l - k;  //usually = -3
          DBG("found RTC had IRQNoFlag will move %d bytes\n", sizeoffset);
          // First move offset byte remove IRQNoFlag
          len = move_data(l, dsdt, len, sizeoffset);
          DBG("...len=%x\n", len);
          // Fix IO (Decode16, size and _CRS size 
          dsdt[RESADR] += (UINT8)sizeoffset;
          dsdt[IOADR] += (UINT8)sizeoffset;
          break;
        }
      }
    }
    
    // if offset > 0 Fix Device RTC size
    if (sizeoffset != 0) 
    {        
      // RTC size
      shift = write_size(adr, dsdt, len, sizeoffset); 
      sizeoffset += shift; //sizeoffset changed
      len += shift;  
      DBG("new size written to %x shift=%x len=%x\n", adr, shift, len);
      len = CorrectOuters(dsdt, len, adr-3, sizeoffset);
      DBG("len after correct outers %x\n", len);
      sizeoffset = 0;
    } // sizeoffset if
  } // l loop
	return len;
}	


UINT32 FixTMR (UINT8 *dsdt, UINT32 len)
{
	UINT32 i, j, k;
	UINT32 IOADR   = 0;
	UINT32 RESADR  = 0;
  UINT32 adr     = 0;
  UINT32 TMRADR, tmrsize = 0;
  INT32  offset  = 0, sizeoffset = 0;
//	DBG("Start TMR Fix\n");
  
  for (j=20; j<len; j++) {
    // Find Device TMR   PNP0100
    if (CmpPNP(dsdt, j, 0x0100)) {
      TMRADR = devFind(dsdt, j);
      adr = TMRADR;
      if (!adr) {
        continue;
      }
      tmrsize = get_size(dsdt, adr);
 //     DBG("TMR size=%x at %x\n", tmrsize, adr);
      if (tmrsize) {
        break;
      }
    } // End TMR      
  }
  if (!adr) {
    DBG("TMR device not found!\n");
    return len;
  }


  // Fix TMR
	// Find Name(_CRS, ResourceTemplate ()) find ResourceTemplate 0x11
  j = 0;
	for (i=adr; i<adr+tmrsize; i++) {  //until next Device()
    if (dsdt[i] == 0x11 && dsdt[i+2] == 0x0A)	{
      RESADR = i+1;  //Format 11, size, 0A, size-3,... 79, 00
      IOADR = i+3;  //IO (Decode16 ==> 47, 01
      j = get_size(dsdt, IOADR);
		}  
    
    if (dsdt[i] == 0x22) {  // Had IRQNoFlag
      for (k=i; k<i+j; k++) {
        if ((dsdt[k] == 0x79) || ((dsdt[k] == 0x47) && (dsdt[k+1] == 0x01)) ||
            ((dsdt[k] == 0x86) && (dsdt[k+1] == 0x09))) {
          sizeoffset = i - k;
          //DBG("found TMR had IRQNoFlag will move %d bytes\n", sizeoffset);
          // First move offset byte remove IRQNoFlag
          len = move_data(i, dsdt, len, sizeoffset);
          // Fix IO (Decode16, size and _CRS size 
          dsdt[RESADR] += (UINT8)sizeoffset;
          dsdt[IOADR] += (UINT8)sizeoffset;
          break;
        }
      }
    }    
    
    // if offset > 0 Fix Device TMR size
		if (sizeoffset != 0) 
    {        
      // TMR size
      offset = write_size(adr, dsdt, len, sizeoffset);
      sizeoffset += offset;
      len += offset;
      len = CorrectOuters(dsdt, len, adr-3, sizeoffset);
      sizeoffset = 0;
    } // offset if
    
    if ((dsdt[i+1] == 0x5B) && (dsdt[i+2] == 0x82)) {
      break; //end of TMR device and begin of new Device()
    }
	} // i loop
	
	return len;
}	

UINT32 FixPIC (UINT8 *dsdt, UINT32 len)
{
	UINT32 i, j, k;
	UINT32 IOADR  = 0;
	UINT32 RESADR = 0;
  UINT32 adr = 0;
  INT32  offset = 0, sizeoffset = 0;
  UINT32 PICADR, picsize = 0;
  
 // DBG("Start PIC Fix\n");
  for (j=20; j<len; j++) {
    // Find Device PIC or IPIC  PNP0000
    if (CmpPNP(dsdt, j, 0x0000)) {
      PICADR = devFind(dsdt, j);
      adr = PICADR;
      if (!adr) {
        continue;
      }
      picsize = get_size(dsdt, adr);
      DBG("PIC size=%x at %x\n", picsize, adr);
      if (picsize) {
        break;
      }
    } // End PIC    
  }  
  if (!picsize) {
    DBG("IPIC not found\n");
    return len;
  }
  
	for (i=adr; i<adr+picsize; i++) 
	{  
    if (dsdt[i] == 0x11 && dsdt[i+2] == 0x0A) {
      RESADR = i+1;  //Format 11, size, 0A, size-3,... 79, 00
      IOADR = i+3;  //IO (Decode16 ==> 47, 01
      continue;
		} else {
      // or 11 41 09 0A 8D 47 01 20 00 -> size=0x91 size-4=0x89
      if ((dsdt[i] == 0x11) &&
          (dsdt[i+3] == 0x0A) &&
          ((dsdt[i+1] & 0xF0) == 0x40)) {
        RESADR = i+1;  //Format 11, size1, size2, 0A, size-4,... 79, 00
        IOADR = i+4;  //IO (Decode16 ==> 47, 01
        DBG("found CRS at %x size %x\n", RESADR, dsdt[IOADR]);
        continue;
      }
    }

    if (dsdt[i] == 0x22) { // Had IRQNoFlag
      for (k = i; k < RESADR + dsdt[IOADR] + 4; k++) {
        if ((dsdt[k] == 0x79) ||
            ((dsdt[k] == 0x47) && (dsdt[k+1] == 0x01)) ||
            ((dsdt[k] == 0x86) && (dsdt[k+1] == 0x09))) {
          sizeoffset = i - k;
          DBG("found PIC had IRQNoFlag will move %d bytes\n", sizeoffset);
          // First move offset byte remove IRQNoFlag
          len = move_data(i, dsdt, len, sizeoffset);
          // Fix IO (Decode16, size and _CRS size 
      //   dsdt[RESADR] += (UINT8)sizeoffset;
          dsdt[IOADR] += (UINT8)sizeoffset;
          offset = write_size(RESADR, dsdt, len, sizeoffset);
          sizeoffset += offset;
          len += offset;
          break;
        }
      }
    }    
    
    // if offset > 0 Fix Device PIC size
		if (sizeoffset != 0 ) {        
      offset = write_size(adr, dsdt, len, sizeoffset);
      sizeoffset += offset;
      DBG("Fix Device PIC size %d\n", sizeoffset);
      len += offset;
      len = CorrectOuters(dsdt, len, adr-3, sizeoffset);
      sizeoffset = 0;
    } // sizeoffset if
    if ((dsdt[i+1] == 0x5B) && (dsdt[i+2] == 0x82)) {
      break; //end of PIC device and begin of new Device()
    }    
	} // i loop
	
	return len;
}	

UINT32 FixHPET (UINT8* dsdt, UINT32 len)
{
  UINT32  i, j;
  UINT32  adr    = 0;
  UINT32  hpetsize = 0;
  INT32   sizeoffset = sizeof(hpet0);
  INT32   shift = 0;
  UINT32  LPCBADR = 0, LPCBSIZE = 0;
  
//	DBG("Start HPET Fix\n");  
  //have to find LPC
  for (j=0x20; j<len-10; j++) {
    if (CmpAdr(dsdt, j, 0x001F0000)) {
      LPCBADR = devFind(dsdt, j);
      if (!LPCBADR) {
        continue;
      }
      LPCBSIZE = get_size(dsdt, LPCBADR);
    }  // End LPCB find
  }
  if (!LPCBSIZE) {
    DBG("No LPCB device! Patch HPET will not be applied\n");
    return len;
  }
  for (j=20; j<len; j++) {
    // Find Device HPET   // PNP0103
    if (CmpPNP(dsdt, j, 0x0103)) {
      adr = devFind(dsdt, j);
      if (!adr) {
        continue;
      }
      hpetsize = get_size(dsdt, adr);
      if (hpetsize) {
        break;
      }
    } // End HPET   
  }
 
  if (hpetsize) {
    i = adr - 2;  //pointer to device HPET
    j = hpetsize + 2;
    sizeoffset -= j;
    len = move_data(i, dsdt, len, sizeoffset);
    // add new HPET code 
    CopyMem(dsdt+i, hpet0, sizeof(hpet0));  
    len = CorrectOuters(dsdt, len, i - 3, sizeoffset); //assume LPC may be outer or not
  } else {
    i = LPCBADR + LPCBSIZE; //pointer to the end of LPC
    //in this case LPC becomes Outer device
    shift = write_size(LPCBADR, dsdt, len, sizeoffset); //correct LPC
    sizeoffset += shift;
    len += shift;
    i += shift;
    len = move_data(i, dsdt, len, sizeof(hpet0));
    // add new HPET code 
    CopyMem(dsdt + i, hpet0, sizeof(hpet0));  
    len = CorrectOuters(dsdt, len, LPCBADR - 3, sizeoffset); //outer for LPC
  }
	return len;
}	

CHAR8 dataLPC[] = {0x18, 0x3A, 0x00, 0x00};

UINT32 FIXLPCB (UINT8 *dsdt, UINT32 len)
{ 
  UINT32 i, j, k;
  INT32 sizeoffset, shift = 0, Size;
  UINT32  LPCBADR = 0, LPCBSIZE = 0, LPCBADR1 = 0;
  AML_CHUNK* root;
  AML_CHUNK* met;
  AML_CHUNK* pack;
  CHAR8 *lpcb;
  DBG("Start LPCB Fix\n");
	//DBG("len = 0x%08x\n", len);
  //have to find LPC
  for (j=0x20; j<len-10; j++) {
    if (CmpAdr(dsdt, j, 0x001F0000))
    {
      LPCBADR = devFind(dsdt, j);
      if (!LPCBADR) {
        continue;
      }
      LPCBSIZE = get_size(dsdt, LPCBADR);
      device_name[3] = AllocateZeroPool(5);
      CopyMem(device_name[3], dsdt + j, 4);
      DBG("found LPCB device NAME(_ADR,0x001F0000) at %x And Name is %a\n", j,
          device_name[3]);

      if (LPCBSIZE) break;
    }  // End LPCB find
  }
  if (!LPCBSIZE) return len;
  LPCBADR1 = LPCBADR + LPCBSIZE;
  ReplaceName(dsdt, len, device_name[3], "LPCB"); 
  
  if (LPCBADR) { // bridge or device
    i = LPCBADR;
    Size = get_size(dsdt, i);
    k = FindMethod(dsdt + i, Size, "_DSM");
    if (k != 0) {
      DBG("_DSM already exists, patch LPC will not be applied\n");
      return len;
    }
  }  
  
	root = aml_create_node(NULL);
	// add Method(_DSM,4,NotSerialized) for LPC
  met = aml_add_method(root, "_DSM", 4);
  met = aml_add_store(met);
  pack = aml_add_package(met);
  aml_add_string(pack, "device-id");
  aml_add_byte_buffer(pack, dataLPC, 4);
  aml_add_local0(met);
  aml_add_buffer(met, dtgp_1, sizeof(dtgp_1));
  // finish Method(_DSM,4,NotSerialized)
  
  aml_calculate_size(root);
  lpcb = AllocateZeroPool(root->Size);  
  sizeoffset = root->Size;  
  aml_write_node(root, lpcb, 0);      
  aml_destroy_node(root);
  // add LPCB code 
  len = move_data(LPCBADR1, dsdt, len, sizeoffset);
  CopyMem(dsdt + LPCBADR1, lpcb, sizeoffset);
	shift = write_size(LPCBADR, dsdt, len, sizeoffset);
  sizeoffset += shift;
  len += shift;
  len = CorrectOuters(dsdt, len, LPCBADR-3, sizeoffset);
  FreePool(lpcb);
  return len;  
}

//CONST
CHAR8 Yes[] = {0x01,0x00,0x00,0x00};
CHAR8 data2[] = {0xe0,0x00,0x56,0x28};
CHAR8 VenATI[] = {0x02, 0x10};

UINT32 FIXDisplay1 (UINT8 *dsdt, UINT32 len)
{
  UINT32 i, j, k;
  INT32 sizeoffset = 0;
  UINT32 PCIADR = 0, PCISIZE = 0, Size;
//  CHAR8 *portname;
  CHAR8 *CFGname = NULL;
  CHAR8 *name = NULL;
  CHAR8 *display;
  UINT32 devadr=0, devsize=0, devadr1=0, devsize1=0;
  AML_CHUNK* root;
  AML_CHUNK* gfx0;
  AML_CHUNK* met;
  BOOLEAN DISPLAYFIX = FALSE;
  AML_CHUNK* pack;
//  UINT32 VideoRam;
//  UINT8 ports;
//  CHAR8 *cfgname;
//  CHAR8 *cardver;
  UINT32 FakeID = 0;
  UINT32 FakeVen = 0;
  DisplayName1 = FALSE;
  

  PCIADR = GetPciDevice(dsdt, len);
  if (PCIADR) {
    PCISIZE = get_size(dsdt, PCIADR);
  }
  if (!PCISIZE) return len; //what is the bad DSDT ?!
    
  DBG("Start Display1 Fix\n");
	//DBG("len = 0x%08x\n", len);
  // Display device_id 
  root = aml_create_node(NULL);
  gfx0 = aml_create_node(NULL);
  met = aml_create_node(NULL);
  
//search DisplayADR1[0]
  for (j=0x20; j<len-10; j++) {
    if (DisplayADR1[0] != 0x00000000 && 
        CmpAdr(dsdt, j, DisplayADR1[0])) {
      DisplayName1 = TRUE;
      devadr = devFind(dsdt, j);
      if (!devadr) {
        continue;
      }
      devsize = get_size(dsdt, devadr);
      if (devsize) {
        break;
      }
    } // End Display1
  }
  
  for (j=devadr; j<devadr+devsize; j++) {
    if (CmpAdr(dsdt, j, 0)) {
      devadr1 = devFind(dsdt, j);
      if (!devadr1) {
        continue;
      }
      devsize1 = get_size(dsdt, devadr1);
      DISPLAYFIX = TRUE;
      break;      
    }
  }
  
  if (devadr1) { // bridge or device
    i = devadr1;
    Size = get_size(dsdt, i);
    k = FindMethod(dsdt + i, Size, "_DSM");
    if (k != 0) {
      DBG("_DSM already exists, patch GFX will not be applied\n");
      return len;
    }
  }

  if (DisplayADR1[0]) {
    if (!DisplayName1) {
      gfx0 = aml_add_device(root, "GFX0");
      aml_add_name(gfx0, "_ADR");
      if (DisplayADR2[0] > 0x3F)
        aml_add_dword(gfx0, DisplayADR2[0]);
      else
        aml_add_byte(gfx0, (UINT8)DisplayADR2[0]);
    }
    // Intel GMA and HD
    if (DisplayVendor[0] == 0x8086) {
/*      AML_CHUNK* pack;
      CHAR8 *modelname = get_gma_model(DisplayID[0]);
      if (AsciiStrnCmp(modelname, "Unknown", 7) == 0)
      {
        DBG("Found Unsupported Intel Display Card Vendor id 0x%04x, device id 0x%04x, don't patch DSDT.\n",
            DisplayVendor[0], DisplayID[0]);
        return len;
      }   
      
      // add Method(_DSM,4,NotSerialized) for GFX0
      //if (!DISPLAYFIX)
      //{
      //    met = aml_add_method(gfx0, "_DSM", 4);
      //}
      //else
      //{      
      
      met = aml_add_method(gfx0, "_DSM", 4);
      //}
      met = aml_add_store(met);
      pack = aml_add_package(met);
      aml_add_string(pack, "AAPL,aux-power-connected");
      aml_add_byte_buffer(pack, Yes, sizeof(Yes));
      aml_add_string(pack, "model");
      aml_add_string_buffer(pack, modelname);
      aml_add_string(pack, "device_type");
      aml_add_string_buffer(pack, "display");
      //
      if ((AsciiStrnCmp(modelname, "Mobile GMA950", 13) == 0) ||
          (AsciiStrnCmp(modelname, "Mobile GMA3150", 14) == 0)) {
        aml_add_string(pack, "AAPL,HasPanel");
        aml_add_byte_buffer(pack, Yes, sizeof(Yes)); 
        aml_add_string(pack, "built-in");
        aml_add_byte_buffer(pack, dataBuiltin1, 1);
        //    aml_add_string(pack, "class-code");
        //    aml_add_byte_buffer(pack, ClassFix, sizeof(ClassFix)); 
      } 
      else if (AsciiStrnCmp(modelname, "Desktop GMA950", 14) == 0 ||
               AsciiStrnCmp(modelname, "Desktop GMA3150", 15) == 0) {
        aml_add_string(pack, "built-in");
        aml_add_byte_buffer(pack, dataBuiltin1, 1);
        //     aml_add_string(pack, "class-code");
        //     aml_add_byte_buffer(pack, ClassFix, sizeof(ClassFix));
      } 
      else if (AsciiStrnCmp(modelname, "GMAX3100", 8) == 0) {
        aml_add_string(pack, "AAPL,HasPanel");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[0], 4); 
        aml_add_string(pack, "AAPL,SelfRefreshSupported");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[1], 4); 
        aml_add_string(pack, "AAPL,backlight-control");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[2], 4);
        aml_add_string(pack, "AAPL00,blackscreen-preferences");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[3], 4);
        aml_add_string(pack, "AAPL01,BacklightIntensity");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[4], 4);
        aml_add_string(pack, "AAPL01,blackscreen-preferences");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[5], 4);
        aml_add_string(pack, "AAPL01,DataJustify");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[6], 4);
        aml_add_string(pack, "AAPL01,Depth");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[7], 4);
        aml_add_string(pack, "AAPL01,Dither");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[8], 4);
        aml_add_string(pack, "AAPL01,DualLink");
        aml_add_byte_buffer(pack, (CHAR8*)&gSettings.DualLink , 4);
        aml_add_string(pack, "AAPL01,Height");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[10], 4);
        aml_add_string(pack, "AAPL01,Interlace");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[11], 4);
        aml_add_string(pack, "AAPL01,Inverter");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[12], 4);
 //       aml_add_string(pack, "AAPL01,InverterCurrent");
 //       aml_add_byte_buffer(pack, GMAX3100_vals_bad[13], 4);
 //       aml_add_string(pack, "AAPL01,InverterCurrency");
 //       aml_add_byte_buffer(pack, GMAX3100_vals_bad[14], 4);
        aml_add_string(pack, "AAPL01,LinkFormat");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[15], 4);
        aml_add_string(pack, "AAPL01,LinkType");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[16], 4);
        aml_add_string(pack, "AAPL01,Pipe");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[17], 4);
        aml_add_string(pack, "AAPL01,PixelFormat");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[18], 4);
        aml_add_string(pack, "AAPL01,Refresh");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[19], 4);
        aml_add_string(pack, "AAPL01,Stretch");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[20], 4);
        //     aml_add_string(pack, "class-code");
        //     aml_add_byte_buffer(pack, ClassFix, sizeof(ClassFix));
        aml_add_string(pack, "AAPL01,boot-display");
        aml_add_byte_buffer(pack, Yes, sizeof(Yes));
      }
      else if (AsciiStrnCmp(modelname, "HD2000", 29) == 0) {
            aml_add_string(pack, "class-code");
            aml_add_byte_buffer(pack, ClassFix, sizeof(ClassFix));
        aml_add_string(pack, "hda-gfx");
        aml_add_string_buffer(pack, "onboard-1");
        aml_add_string(pack, "AAPL00,PixelFormat");
        aml_add_byte_buffer(pack, HD2000_vals[0], 4); 
        aml_add_string(pack, "AAPL00,T1");
        aml_add_byte_buffer(pack, HD2000_vals[1], 4);
        aml_add_string(pack, "AAPL00,T2");
        aml_add_byte_buffer(pack, HD2000_vals[2], 4);
        aml_add_string(pack, "AAPL00,T3");
        aml_add_byte_buffer(pack, HD2000_vals[3], 4);
        aml_add_string(pack, "AAPL00,T4");
        aml_add_byte_buffer(pack, HD2000_vals[4], 4);
        aml_add_string(pack, "AAPL00,T5");
        aml_add_byte_buffer(pack, HD2000_vals[5], 4);
        aml_add_string(pack, "AAPL00,T6");
        aml_add_byte_buffer(pack, HD2000_vals[6], 4);
        aml_add_string(pack, "AAPL00,T7");
        aml_add_byte_buffer(pack, HD2000_vals[7], 4);
        aml_add_string(pack, "AAPL00,LinkType");
        aml_add_byte_buffer(pack, HD2000_vals[8], 4);
        aml_add_string(pack, "AAPL00,LinkFormat");
        aml_add_byte_buffer(pack, HD2000_vals[9], 4);
        aml_add_string(pack, "AAPL00,DualLink");
        aml_add_byte_buffer(pack, (CHAR8*)&gSettings.DualLink , 4);
        aml_add_string(pack, "AAPL00,Dither");
        aml_add_byte_buffer(pack, HD2000_vals[11], 4);
        aml_add_string(pack, "AAPL00,DataJustify");
        aml_add_byte_buffer(pack, HD2000_vals[12], 4);
        aml_add_string(pack, "graphic-options");
        aml_add_byte_buffer(pack, HD2000_vals[13], 4);
        aml_add_string(pack, "AAPL,tbl-info");
        aml_add_byte_buffer(pack, HD2000_tbl_info, 18);
        aml_add_string(pack, "AAPL,os-info");
        aml_add_byte_buffer(pack, HD2000_os_info, 20);
        aml_add_string(pack, "AAPL00,boot-display");
        aml_add_byte_buffer(pack, Yes, sizeof(Yes));
        aml_add_string(pack, "built-in");
        aml_add_byte_buffer(pack, dataBuiltin1, 1);
        
      }
      else if (AsciiStrnCmp(modelname, "HD3000", 29) == 0) {
        aml_add_string(pack, "class-code");
        aml_add_byte_buffer(pack, ClassFix, sizeof(ClassFix));
        aml_add_string(pack, "hda-gfx");
        aml_add_string_buffer(pack, "onboard-1");
        aml_add_string(pack, "AAPL00,PixelFormat");
        aml_add_byte_buffer(pack, HD3000_vals[0], 4); 
        aml_add_string(pack, "AAPL00,T1");
        aml_add_byte_buffer(pack, HD3000_vals[1], 4);
        aml_add_string(pack, "AAPL00,T2");
        aml_add_byte_buffer(pack, HD3000_vals[2], 4);
        aml_add_string(pack, "AAPL00,T3");
        aml_add_byte_buffer(pack, HD3000_vals[3], 4);
        aml_add_string(pack, "AAPL00,T4");
        aml_add_byte_buffer(pack, HD3000_vals[4], 4);
        aml_add_string(pack, "AAPL00,T5");
        aml_add_byte_buffer(pack, HD3000_vals[5], 4);
        aml_add_string(pack, "AAPL00,T6");
        aml_add_byte_buffer(pack, HD3000_vals[6], 4);
        aml_add_string(pack, "AAPL00,T7");
        aml_add_byte_buffer(pack, HD3000_vals[7], 4);
        aml_add_string(pack, "AAPL00,LinkType");
        aml_add_byte_buffer(pack, HD3000_vals[8], 4);
        aml_add_string(pack, "AAPL00,LinkFormat");
        aml_add_byte_buffer(pack, HD3000_vals[9], 4);
        aml_add_string(pack, "AAPL00,DualLink");
        aml_add_byte_buffer(pack, (CHAR8*)&gSettings.DualLink, 4);
        aml_add_string(pack, "AAPL00,Dither");
        aml_add_byte_buffer(pack, HD3000_vals[11], 4);
        aml_add_string(pack, "AAPL00,DataJustify");
        aml_add_byte_buffer(pack, HD3000_vals[12], 4);
        aml_add_string(pack, "graphic-options");
        aml_add_byte_buffer(pack, HD3000_vals[13], 4);
        aml_add_string(pack, "AAPL,tbl-info");
        aml_add_byte_buffer(pack, HD3000_tbl_info, 18);
        aml_add_string(pack, "AAPL,os-info");
        aml_add_byte_buffer(pack, HD3000_os_info, 20);
        aml_add_string(pack, "AAPL,snb-platform-id");
        aml_add_byte_buffer(pack, HD3000_vals[16], 4);
        aml_add_string(pack, "AAPL00,boot-display");
        aml_add_byte_buffer(pack, Yes, sizeof(Yes));
      }
      else if (AsciiStrStr(modelname, "HD Graphics 2000")) {
        aml_add_string(pack, "built-in");
        aml_add_byte_buffer(pack, dataBuiltin1, 1);
        //      aml_add_string(pack, "class-code");
        //      aml_add_byte_buffer(pack, ClassFix, sizeof(ClassFix));
        aml_add_string(pack, "device-id");
        aml_add_byte_buffer(pack, (CHAR8*)&DisplayID[0], 4);
        aml_add_string(pack, "hda-gfx");
        aml_add_string_buffer(pack, "onboard-1");
        aml_add_string(pack, "AAPL,tbl-info");
        aml_add_byte_buffer(pack, HD2000_tbl_info, 18);
        aml_add_string(pack, "AAPL,os-info");
        aml_add_byte_buffer(pack, HD2000_os_info, 20);
//        aml_add_string(pack, "AAPL,snb-platform-id");
//        aml_add_byte_buffer(pack, HD3000_vals[16], 4);
        aml_add_string(pack, "AAPL00,boot-display");
        aml_add_byte_buffer(pack, Yes, sizeof(Yes));
      }
      else if (AsciiStrStr(modelname, "HD Graphics 3000")) {
        aml_add_string(pack, "built-in");
        aml_add_byte_buffer(pack, dataBuiltin1, 1);
        //      aml_add_string(pack, "class-code");
        //      aml_add_byte_buffer(pack, ClassFix, sizeof(ClassFix));
        aml_add_string(pack, "device-id");
        aml_add_byte_buffer(pack, (CHAR8*)&DisplayID[0], 4);
        aml_add_string(pack, "hda-gfx");
        aml_add_string_buffer(pack, "onboard-1");
        aml_add_string(pack, "AAPL,tbl-info");
        aml_add_byte_buffer(pack, HD3000_tbl_info, 18);
        aml_add_string(pack, "AAPL,os-info");
        aml_add_byte_buffer(pack, HD3000_os_info, 20);
        aml_add_string(pack, "AAPL,snb-platform-id");
        aml_add_byte_buffer(pack, HD3000_vals[16], 4);
        aml_add_string(pack, "AAPL00,boot-display");
        aml_add_byte_buffer(pack, Yes, sizeof(Yes));
      }
 */
      met = aml_add_method(root, "_DSM", 4);
      met = aml_add_store(met);
      pack = aml_add_package(met);
      
      if (gSettings.FakeIntel) {
        FakeID = gSettings.FakeIntel >> 16;
        aml_add_string(pack, "device-id");
        aml_add_byte_buffer(pack, (CHAR8*)&FakeID, 4);
        FakeVen = gSettings.FakeIntel & 0xFFFF;
        aml_add_string(pack, "vendor-id");
        aml_add_byte_buffer(pack, (CHAR8*)&FakeVen, 4);
      }
      
      
      aml_add_local0(met);
      aml_add_buffer(met, dtgp_1, sizeof(dtgp_1));
      // finish Method(_DSM,4,NotSerialized)
    }
    
    // NVIDIA
  if (DisplayVendor[0] == 0x10DE) {
  //    AML_CHUNK* pack;
  //    UINT64 VideoRam;
  //    CHAR8 *modelname = nv_name((UINT16)DisplayVendor[0], DisplayID[0]);
      // add Method(_DSM,4,NotSerialized) for GFX0
   
      if (!DISPLAYFIX) {
        met = aml_add_method(gfx0, "_DSM", 4);
      } else {
        met = aml_add_method(root, "_DSM", 4);
      }
      met = aml_add_store(met);
      //Slice - next I mark what is in Natit, and what no
      pack = aml_add_package(met);
 /*     aml_add_string(pack, "AAPL,aux-power-connected");  //-
      aml_add_byte_buffer(pack, Yes, sizeof(Yes));
      aml_add_string(pack, "AAPL00,DualLink");          //-
      aml_add_byte_buffer(pack, Yes, sizeof(Yes));
      aml_add_string(pack, "@0,AAPL,boot-display");     //-
      aml_add_byte_buffer(pack, Yes, sizeof(Yes));  
      aml_add_string(pack, "@0,name");
      aml_add_string_buffer(pack, "NVDA,Display-A");  //+
      aml_add_string(pack, "@0,compatible");
      aml_add_string_buffer(pack, "NVDA,NVMac");    //+
      aml_add_string(pack, "@0,device_type");
      aml_add_string_buffer(pack, "display");     //+
      aml_add_string(pack, "@1,name");
      aml_add_string_buffer(pack, "NVDA,Display-B");      //+
      aml_add_string(pack, "@1,compatible");
      aml_add_string_buffer(pack, "NVDA,NVMac");    //+
      aml_add_string(pack, "@1,device_type");
      aml_add_string_buffer(pack, "display");       //+
      aml_add_string(pack, "device_type");
      aml_add_string_buffer(pack, "NVDA,Parent");   //+
      aml_add_string(pack, "NVCAP");
      aml_add_byte_buffer(pack, (CHAR8*)&gSettings.NVCAP[0], 20);  //+
      aml_add_string(pack, "NVPM");
      aml_add_byte_buffer(pack, (CHAR8*)&default_NVPM[0], 28);  //+
      aml_add_string(pack, "model");
      aml_add_string_buffer(pack, modelname);       //+
      aml_add_string(pack, "rom-revision");
      aml_add_string_buffer(pack, "Clover auto patch DSDT ver1.1");  //+
      aml_add_string(pack, "hda-gfx");
      aml_add_string_buffer(pack, "onboard-1");         //-
      VideoRam = nv_mem_detect(&Displaydevice[0]); 
      aml_add_string(pack, "VRAM,totalsize");         //+
      aml_add_dword(pack, (UINT32)VideoRam); 
//      aml_add_string(pack, "device-id");              //-
//      aml_add_byte_buffer(pack, (CHAR8*)&DisplayID[0], 4);
//      aml_add_string(pack, "name");
//      aml_add_string_buffer(pack, "display");       //+

      if (!Display1PCIE)
      {
//        aml_add_string(pack, "IOPCIExpressLinkCapabilities"); //-
//        aml_add_dword(pack, 0x130d1) ;//0x1e80); 
//        aml_add_string(pack, "IOPCIExpressLinkStatus");     //-
//        aml_add_dword(pack, 0x10880); //0x880); 
      }
*/
    if (gSettings.FakeNVidia) {
      FakeID = gSettings.FakeNVidia >> 16;
      aml_add_string(pack, "device-id");
      aml_add_byte_buffer(pack, (CHAR8*)&FakeID, 4);
      FakeVen = gSettings.FakeNVidia & 0xFFFF;
      aml_add_string(pack, "vendor-id");
      aml_add_byte_buffer(pack, (CHAR8*)&FakeVen, 4);
    }
    
      aml_add_local0(met);
      aml_add_buffer(met, dtgp_1, sizeof(dtgp_1));
      // finish Method(_DSM,4,NotSerialized)
    }
  
    // ATI
    if (DisplayVendor[0] == 0x1002) {
//      CHAR8 *modelname = ati_name(DisplayID[0], DisplaySubID[0]);
      // add Method(_DSM,4,NotSerialized) for GFX0
      if (!DISPLAYFIX) {
        met = aml_add_method(gfx0, "_DSM", 4);
      } else {
        met = aml_add_method(root, "_DSM", 4);
      }
      met = aml_add_store(met);
      pack = aml_add_package(met);
  /*
      aml_add_string(pack, "AAPL,aux-power-connected");
      aml_add_byte_buffer(pack, Yes, sizeof(Yes));
      aml_add_string(pack, "AAPL00,DualLink");
      aml_add_byte_buffer(pack, (CHAR8*)&gSettings.DualLink, 4);
      aml_add_string(pack, "@0,AAPL,boot-display");
      aml_add_byte_buffer(pack, Yes, sizeof(Yes));
      ports = ati_port(DisplayID[0], DisplaySubID[0]);
      cfgname = ati_cfg_name(DisplayID[0], DisplaySubID[0]);
      CFGname = AllocateZeroPool(sizeof(cfgname)+5);
      AsciiSPrint(CFGname, sizeof(cfgname)+5, "ATY,%a", cfgname);
      for (i=0; i<ports; i++) {
        portname = AllocateZeroPool(8);
        AsciiSPrint(portname, 8, "@%d,name", i);
        aml_add_string(pack, portname);
        aml_add_string_buffer(pack, CFGname);  
      } 
      
      cardver = ATI_romrevision(&Displaydevice[0]);
      aml_add_string(pack, "ATY,Card#");
      aml_add_string_buffer(pack, cardver);       
      aml_add_string(pack, "ATY,Copyright");
      aml_add_string_buffer(pack, "Copyright AMD Inc. All Rights Reserved. 2005-2011");    
      aml_add_string(pack, "ATY,EFIVersion");
      aml_add_string_buffer(pack, "01.00.3180");  
      aml_add_string(pack, "model");
      aml_add_string_buffer(pack, modelname);
      aml_add_string(pack, "name");
      name = AllocateZeroPool(sizeof(cfgname)+11);
      AsciiSPrint(name, sizeof(cfgname)+11, "ATY,%aParent", cfgname);
      aml_add_string_buffer(pack, name);  
      aml_add_string(pack, "ATY,VendorID");
      aml_add_byte_buffer(pack, (CHAR8*)&DisplayVendor[0], 4); 
      aml_add_string(pack, "ATY,DeviceID");
      aml_add_byte_buffer(pack, (CHAR8*)&DisplayID[0], 4); 
 //     aml_add_string(pack, "device-id");
 //     CHAR8 data[] = {0xE1,0x68,0x00,0x00};
 //     aml_add_byte_buffer(pack, data, sizeof(data));
      aml_add_string(pack, "org-device-id");
      aml_add_byte_buffer(pack, (CHAR8*)&DisplayID[0], 4); 
      aml_add_string(pack, "hda-gfx");
      aml_add_string_buffer(pack, "onboard-1");
      VideoRam = ATI_vram_size(&Displaydevice[0]); 
      aml_add_string(pack, "VRAM,totalsize");
      aml_add_dword(pack, VideoRam); 
      if (!Display1PCIE) {
        aml_add_string(pack, "IOPCIExpressLinkCapabilities");
        aml_add_dword(pack, 0x130d1) ;//0x1e80); 
        aml_add_string(pack, "IOPCIExpressLinkStatus");
        aml_add_dword(pack, 0x10880); //0x880);  
      }
*/   
      if (gSettings.FakeATI) {
        FakeID = gSettings.FakeATI >> 16;
        aml_add_string(pack, "device-id");
        aml_add_byte_buffer(pack, (CHAR8*)&FakeID, 4);
        aml_add_string(pack, "ATY,DeviceID");
        aml_add_byte_buffer(pack, (CHAR8*)&FakeID, 2);
        FakeVen = gSettings.FakeATI & 0xFFFF;
        aml_add_string(pack, "vendor-id");
        aml_add_byte_buffer(pack, (CHAR8*)&FakeVen, 4);
        aml_add_string(pack, "ATY,VendorID");
        aml_add_byte_buffer(pack, (CHAR8*)&FakeVen, 2);
      } else {
        aml_add_string(pack, "ATY,VendorID");
        aml_add_byte_buffer(pack, VenATI, 2);
      }
      aml_add_local0(met);
      aml_add_buffer(met, dtgp_1, sizeof(dtgp_1));
      // finish Method(_DSM,4,NotSerialized)
    }
    
    // HDAU
    
    if (GFXHDAFIX && (FindBin(dsdt, len, (UINT8*)"HDAU", 4) < 0)) {
 //     CHAR8 data2[] = {0xe0,0x00,0x56,0x28};
 //     AML_CHUNK* met;
      AML_CHUNK* pack;
      AML_CHUNK* device = aml_add_device(root, "HDAU");
      aml_add_name(device, "_ADR");
      aml_add_byte(device, 0x01);
      // add Method(_DSM,4,NotSerialized) for GFX0
      met = aml_add_method(device, "_DSM", 4);
      met = aml_add_store(met);
      pack = aml_add_package(met);
      //aml_add_string(pack, "device-id");
      //CHAR8 data[] = {0x38,0xAA,0x00,0x00};
      //aml_add_byte_buffer(pack, data, sizeof(data));
      //aml_add_string(pack, "codec-id");
      //aml_add_byte_buffer(pack, (CHAR8*)&GfxcodecId, 4);
      aml_add_string(pack, "layout-id");
      aml_add_byte_buffer(pack, (CHAR8*)&GfxlayoutId, 4);
      aml_add_string(pack, "hda-gfx");
      aml_add_string_buffer(pack, "onboard-2");
      //aml_add_string(pack, "name");
      //aml_add_string(pack, "pci1002,aa38");
      //aml_add_string(pack, "IOName");
      //aml_add_string(pack, "pci1002,aa38");
      //aml_add_string(pack, "layout-id");
      //CHAR8 data1[] = {0x12,0x00,0x00,0x00};
      //aml_add_byte_buffer(pack, data1, sizeof(data1));
      aml_add_string(pack, "PinConfigurations");
      aml_add_byte_buffer(pack, data2, sizeof(data2));
      aml_add_local0(met);
      aml_add_buffer(met, dtgp_1, sizeof(dtgp_1));
      // finish Method(_DSM,4,NotSerialized)
    }
  }
  
  aml_calculate_size(root);  
  display = AllocateZeroPool(root->Size);
  sizeoffset = root->Size;
  aml_write_node(root, display, 0);  
  aml_destroy_node(root);
  if (DisplayName1) {
    // move data to back for add Display
    if (!DISPLAYFIX) { 
      i = devadr+devsize;
      len = move_data(i, dsdt, len, sizeoffset);
      CopyMem(dsdt+i, display, sizeoffset);
      len = CorrectOuters(dsdt, len, devadr-3, sizeoffset);
    } else {
      i = devadr1+devsize1;
      len = move_data(i, dsdt, len, sizeoffset);
      CopyMem(dsdt+i, display, sizeoffset);
      j = write_size(devadr1, dsdt, len, sizeoffset);
      sizeoffset += j;
      len += j;
      len = CorrectOuters(dsdt, len, devadr1-3, sizeoffset);
    }
  } else {
    len = move_data(PCIADR+PCISIZE, dsdt, len, sizeoffset);
    CopyMem(dsdt+PCIADR+PCISIZE, display, sizeoffset);
    // Fix PCIX size
    k = write_size(PCIADR, dsdt, len, sizeoffset);
    sizeoffset += k;
    len += k;
    len = CorrectOuters(dsdt, len, PCIADR-3, sizeoffset);    
  }
  
  if (CFGname) {
    FreePool(CFGname);
  }
  if (name) {
    FreePool(name);
  } 
  
  FreePool(display);
  return len;  
}


UINT32 FIXDisplay2 (UINT8 *dsdt, UINT32 len)
{
  UINT32 i, j, k;
  INT32 sizeoffset;
  UINT32 PCIADR, PCISIZE = 0, Size;
  AML_CHUNK* root;
  AML_CHUNK* gfx0;
  AML_CHUNK* met;
  AML_CHUNK* pack;
//  CHAR8 *portname;
  CHAR8 *CFGname  = NULL;
  CHAR8 *name     = NULL;
  CHAR8 *display  = NULL;
  UINT32 devadr=0, devsize=0, devadr1=0, devsize1=0;
  BOOLEAN DISPLAYFIX = FALSE;
  UINT32 FakeID = 0;
  UINT32 FakeVen = 0;
  
  DBG("Start Display2 Fix\n");
  PCIADR = GetPciDevice(dsdt, len);
  if (PCIADR) {
    PCISIZE = get_size(dsdt, PCIADR);
  }
  if (!PCISIZE)
    return len; //what is the bad DSDT ?!
  
  root = aml_create_node(NULL);
  gfx0 = aml_create_node(NULL);
  met = aml_create_node(NULL);
  
  for (j=0x20; j<len-10; j++) {
    if (DisplayADR1[1] != 0x00000000 && 
        CmpAdr(dsdt, j, DisplayADR1[1])) {
      //        DBG("Found DisplayADR1[1]=%x at %x\n", DisplayADR1[1], j);
      DisplayName2 = TRUE;
      devadr = devFind(dsdt, j);
      if (!devadr) {
        continue;
      }
      devsize = get_size(dsdt, devadr);
      if (devsize) {
        break;
      } // End Display
    }
  }  
  if (!devadr) return len;
  for (j=devadr; j<devadr+devsize; j++) {
    if (CmpAdr(dsdt, j, 0)) {
      devadr1 = devFind(dsdt, j);
      if (!devadr1) {
        continue;
      }
      devsize1 = get_size(dsdt, devadr1);
      DISPLAYFIX = TRUE;
      break;      
    }
  }
  
  if (devadr1) { // bridge or device
    i = devadr1;
    Size = get_size(dsdt, i);
    k = FindMethod(dsdt + i, Size, "_DSM");
    if (k != 0) {
      DBG("_DSM already exists, patch GFX will not be applied\n");
      return len;
    }
  }  
    
  if (DisplayADR1[1]) {
    if (!DisplayName2) {
      AML_CHUNK* pegp = aml_add_device(root, "PEGP");
      aml_add_name(pegp, "_ADR");
      aml_add_dword(pegp, DisplayADR1[1]);
      gfx0 = aml_add_device(pegp, "GFX0");
      aml_add_name(gfx0, "_ADR");
      if (DisplayADR2[1] > 0x3F)
        aml_add_dword(gfx0, DisplayADR2[1]);
      else
        aml_add_byte(gfx0, (UINT8)DisplayADR2[1]);
    } else {    
      if(!DISPLAYFIX && DisplayVendor[1] != 0x8086) {
        gfx0 = aml_add_device(root, "GFX0");
        aml_add_name(gfx0, "_ADR");
        if (DisplayADR2[1] > 0x3F)
          aml_add_dword(gfx0, DisplayADR2[1]);
        else
          aml_add_byte(gfx0, (UINT8)DisplayADR2[1]);  
      }
    }
    
    // Intel GMA and HD
    if (DisplayVendor[1] == 0x8086) {
/*      
      CHAR8 *modelname = get_gma_model(DisplayID[1]);
      if (AsciiStrnCmp(modelname, "Unknown", 7) == 0) {
        DBG("Found Unsupported Intel Display Card Vendor id 0x%04x, device id 0x%04x, don't patch DSDT.\n",
            DisplayVendor[1], DisplayID[1]);
        return len;
      }   
      
      //CHAR8 ClassFix[] =	{ 0x00, 0x00, 0x03, 0x00 };   
      // add Method(_DSM,4,NotSerialized) for GFX0
      //if (!DISPLAYFIX)
      //{
      //    met = aml_add_method(gfx0, "_DSM", 4);
      //}
      //else
      //{
      met = aml_add_method(root, "_DSM", 4);
      //}
      met = aml_add_store(met);
      pack = aml_add_package(met);
      aml_add_string(pack, "AAPL,aux-power-connected");
      aml_add_byte_buffer(pack, Yes, sizeof(Yes));
      aml_add_string(pack, "model");
      aml_add_string_buffer(pack, modelname);
      aml_add_string(pack, "device_type");
      aml_add_string_buffer(pack, "display");
      //
      if (AsciiStrnCmp(modelname, "Mobile GMA950", 13) == 0 ||
          AsciiStrnCmp(modelname, "Mobile GMA3150", 14) == 0) {
        aml_add_string(pack, "AAPL,HasPanel");
        aml_add_byte_buffer(pack, Yes, sizeof(Yes)); 
        aml_add_string(pack, "built-in");
        aml_add_byte_buffer(pack, dataBuiltin1, 1);
        //   aml_add_string(pack, "class-code");
        //   aml_add_byte_buffer(pack, ClassFix, sizeof(ClassFix)); 
      } else if (AsciiStrnCmp(modelname, "Desktop GMA950", 14) == 0 ||
               AsciiStrnCmp(modelname, "Desktop GMA3150", 15) == 0) {
        aml_add_string(pack, "built-in");
        aml_add_byte_buffer(pack, dataBuiltin1, 1);
        //    aml_add_string(pack, "class-code");
        //    aml_add_byte_buffer(pack, ClassFix, sizeof(ClassFix));
      } else if (AsciiStrnCmp(modelname, "GMAX3100", 8) == 0) {
        aml_add_string(pack, "AAPL,HasPanel");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[0], 4); 
        aml_add_string(pack, "AAPL,SelfRefreshSupported");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[1], 4); 
        aml_add_string(pack, "AAPL,backlight-control");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[2], 4);
        aml_add_string(pack, "AAPL00,blackscreen-preferences");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[3], 4);
        aml_add_string(pack, "AAPL01,BacklightIntensity");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[4], 4);
        aml_add_string(pack, "AAPL01,blackscreen-preferences");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[5], 4);
        aml_add_string(pack, "AAPL01,DataJustify");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[6], 4);
        aml_add_string(pack, "AAPL01,Depth");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[7], 4);
        aml_add_string(pack, "AAPL01,Dither");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[8], 4);
        aml_add_string(pack, "AAPL01,DualLink");
        aml_add_byte_buffer(pack, (CHAR8*)&gSettings.DualLink , 4);
        aml_add_string(pack, "AAPL01,Height");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[10], 4);
        aml_add_string(pack, "AAPL01,Interlace");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[11], 4);
        aml_add_string(pack, "AAPL01,Inverter");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[12], 4);
        aml_add_string(pack, "AAPL01,InverterCurrent");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[13], 4);
        aml_add_string(pack, "AAPL01,InverterCurrency");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[14], 4);
        aml_add_string(pack, "AAPL01,LinkFormat");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[15], 4);
        aml_add_string(pack, "AAPL01,LinkType");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[16], 4);
        aml_add_string(pack, "AAPL01,Pipe");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[17], 4);
        aml_add_string(pack, "AAPL01,PixelFormat");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[18], 4);
        aml_add_string(pack, "AAPL01,Refresh");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[19], 4);
        aml_add_string(pack, "AAPL01,Stretch");
        aml_add_byte_buffer(pack, GMAX3100_vals_bad[20], 4);
        //     aml_add_string(pack, "class-code");
        //     aml_add_byte_buffer(pack, ClassFix, sizeof(ClassFix));
        aml_add_string(pack, "AAPL01,boot-display");
        aml_add_byte_buffer(pack, Yes, sizeof(Yes));
      }  else if (AsciiStrStr(modelname, "HD Graphics 2000")) {
        aml_add_string(pack, "built-in");
        aml_add_byte_buffer(pack, dataBuiltin1, 1);
        //      aml_add_string(pack, "class-code");
        //      aml_add_byte_buffer(pack, ClassFix, sizeof(ClassFix));
        aml_add_string(pack, "device-id");
        aml_add_byte_buffer(pack, (CHAR8*)&DisplayID[1], 4);
        aml_add_string(pack, "hda-gfx");
        aml_add_string_buffer(pack, "onboard-1");
        aml_add_string(pack, "AAPL,tbl-info");
        aml_add_byte_buffer(pack, HD2000_tbl_info, 18);
        aml_add_string(pack, "AAPL,os-info");
        aml_add_byte_buffer(pack, HD2000_os_info, 20);
        aml_add_string(pack, "AAPL00,boot-display");
        aml_add_byte_buffer(pack, Yes, sizeof(Yes));
      } else if (AsciiStrStr(modelname, "HD Graphics 3000")) {
        aml_add_string(pack, "built-in");
        aml_add_byte_buffer(pack, dataBuiltin1, 1);
        aml_add_string(pack, "class-code");
        aml_add_byte_buffer(pack, ClassFix, sizeof(ClassFix));
        aml_add_string(pack, "device-id");
        aml_add_byte_buffer(pack, (CHAR8*)&DisplayID[1], 4);
        aml_add_string(pack, "hda-gfx");
        aml_add_string_buffer(pack, "onboard-1");
        aml_add_string(pack, "AAPL,tbl-info");
        aml_add_byte_buffer(pack, HD3000_tbl_info, 18);
        aml_add_string(pack, "AAPL,os-info");
        aml_add_byte_buffer(pack, HD3000_os_info, 20);
        aml_add_string(pack, "AAPL,snb-platform-id");
        aml_add_byte_buffer(pack, HD3000_vals[16], 4);
        aml_add_string(pack, "AAPL00,boot-display");
        aml_add_byte_buffer(pack, Yes, sizeof(Yes));
      }
 */
      met = aml_add_method(root, "_DSM", 4);
      met = aml_add_store(met);
      pack = aml_add_package(met);
      
      if (gSettings.FakeIntel) {
        FakeID = gSettings.FakeIntel >> 16;
        aml_add_string(pack, "device-id");
        aml_add_byte_buffer(pack, (CHAR8*)&FakeID, 4);
        FakeVen = gSettings.FakeIntel & 0xFFFF;
        aml_add_string(pack, "vendor-id");
        aml_add_byte_buffer(pack, (CHAR8*)&FakeVen, 4);
      }
      
      aml_add_local0(met);
      aml_add_buffer(met, dtgp_1, sizeof(dtgp_1));
      // finish Method(_DSM,4,NotSerialized)
    }
    
    // NVIDIA
   if (DisplayVendor[1] == 0x10DE) {
 //     AML_CHUNK* pack;
 //     UINT64 VideoRam;
 //     CHAR8 *modelname = nv_name((UINT16)DisplayVendor[1], DisplayID[1]);
      // add Method(_DSM,4,NotSerialized) for GFX0
      if (!DISPLAYFIX) {
        met = aml_add_method(gfx0, "_DSM", 4);
      } else {
        met = aml_add_method(root, "_DSM", 4);
      }
      met = aml_add_store(met);
      pack = aml_add_package(met);
/*     
      aml_add_string(pack, "AAPL,aux-power-connected");
      aml_add_byte_buffer(pack, Yes, sizeof(Yes));
      aml_add_string(pack, "AAPL00,DualLink");
      aml_add_byte_buffer(pack, (CHAR8*)&gSettings.DualLink, 4);
      aml_add_string(pack, "@0,AAPL,boot-display");
      aml_add_byte_buffer(pack, Yes, sizeof(Yes));  
      aml_add_string(pack, "@0,name");
      aml_add_string_buffer(pack, "NVDA,Display-A");  
      aml_add_string(pack, "@0,compatible");
      aml_add_string_buffer(pack, "NVDA,NVMac");  
      aml_add_string(pack, "@0,device_type");
      aml_add_string_buffer(pack, "display");  
      aml_add_string(pack, "@1,name");
      aml_add_string_buffer(pack, "NVDA,Display-B");  
      aml_add_string(pack, "@1,compatible");
      aml_add_string_buffer(pack, "NVDA,NVMac"); 
      aml_add_string(pack, "@1,device_type");
      aml_add_string_buffer(pack, "display"); 
      aml_add_string(pack, "device_type");
      aml_add_string_buffer(pack, "NVDA,Parent"); 
      aml_add_string(pack, "NVCAP");
      aml_add_byte_buffer(pack, (CHAR8*)&gSettings.NVCAP[0], 20); 
      aml_add_string(pack, "NVPM");
      aml_add_byte_buffer(pack, (CHAR8*)&default_NVPM[0], 28); 
      aml_add_string(pack, "model");
      aml_add_string_buffer(pack, modelname); 
      aml_add_string(pack, "rom-revision");
      aml_add_string_buffer(pack, "pcj auto patch DSDT ver1.0");
      aml_add_string(pack, "hda-gfx");
      aml_add_string_buffer(pack, "onboard-1"); 
      VideoRam = nv_mem_detect(&Displaydevice[1]); 
      aml_add_string(pack, "VRAM,totalsize");
      aml_add_dword(pack, (UINT32)VideoRam); 
      aml_add_string(pack, "device-id");
      aml_add_byte_buffer(pack, (CHAR8*)&DisplayID[1], 4);
      if (!Display2PCIE) {
        aml_add_string(pack, "IOPCIExpressLinkCapabilities");
        aml_add_dword(pack, 0x130d1) ;//0x1e80); 
        aml_add_string(pack, "IOPCIExpressLinkStatus");
        aml_add_dword(pack, 0x10880); //0x880);  
      }
 */
     if (gSettings.FakeNVidia) {
       FakeID = gSettings.FakeNVidia >> 16;
       aml_add_string(pack, "device-id");
       aml_add_byte_buffer(pack, (CHAR8*)&FakeID, 4);
       FakeVen = gSettings.FakeNVidia & 0xFFFF;
       aml_add_string(pack, "vendor-id");
       aml_add_byte_buffer(pack, (CHAR8*)&FakeVen, 4);
     }
     
      aml_add_local0(met);
      aml_add_buffer(met, dtgp_1, sizeof(dtgp_1));
      // finish Method(_DSM,4,NotSerialized)
    }
    
    // ATI
    if (DisplayVendor[1] == 0x1002) {
      // add Method(_DSM,4,NotSerialized) for GFX0
      if (!DISPLAYFIX) {
        met = aml_add_method(gfx0, "_DSM", 4);
      } else {
        met = aml_add_method(root, "_DSM", 4);
      }
      met = aml_add_store(met);
      pack = aml_add_package(met);
/*      aml_add_string(pack, "AAPL,aux-power-connected");
      aml_add_byte_buffer(pack, Yes, sizeof(Yes));
      aml_add_string(pack, "AAPL00,DualLink");
      aml_add_byte_buffer(pack, (CHAR8*)&gSettings.DualLink, 4);
      aml_add_string(pack, "@0,AAPL,boot-display");
      aml_add_byte_buffer(pack, Yes, sizeof(Yes));  
      ports = ati_port(DisplayID[1], DisplaySubID[1]);
      cfgname = ati_cfg_name(DisplayID[1], DisplaySubID[1]);
      CFGname = AllocateZeroPool(sizeof(cfgname)+5);
      AsciiSPrint(CFGname, sizeof(cfgname)+5, "ATY,%a", cfgname);
      for (i=0; i<ports; i++) {
        portname = AllocateZeroPool(8);
        AsciiSPrint(portname, 8, "@%d,name", i);
        aml_add_string(pack, portname);
        aml_add_string_buffer(pack, CFGname);  
      }          
      cardver = ATI_romrevision(&Displaydevice[1]);
      aml_add_string(pack, "ATY,Card#");
      aml_add_string_buffer(pack, cardver);       
      aml_add_string(pack, "ATY,Copyright");
      aml_add_string_buffer(pack, "Copyright AMD Inc. All Rights Reserved. 2005-2011");    
      aml_add_string(pack, "ATY,EFIVersion");
      aml_add_string_buffer(pack, "01.00.3180");  
      aml_add_string(pack, "model");
      aml_add_string_buffer(pack, modelname);
      aml_add_string(pack, "name");
      name = AllocateZeroPool(sizeof(cfgname)+11);
      AsciiSPrint(name, sizeof(cfgname)+11, "ATY,%aParent", cfgname);
      aml_add_string_buffer(pack, name);  
      aml_add_string(pack, "ATY,VendorID");
      aml_add_byte_buffer(pack, (CHAR8*)&DisplayVendor[1], 4); 
      aml_add_string(pack, "ATY,DeviceID");
      aml_add_byte_buffer(pack, (CHAR8*)&DisplayID[1], 4); 
 */
      if (gSettings.FakeATI) {
        FakeID = gSettings.FakeATI >> 16;
        aml_add_string(pack, "device-id");
        aml_add_byte_buffer(pack, (CHAR8*)&FakeID, 4);
        aml_add_string(pack, "ATY,DeviceID");
        aml_add_byte_buffer(pack, (CHAR8*)&FakeID, 2);
        FakeVen = gSettings.FakeATI & 0xFFFF;
        aml_add_string(pack, "vendor-id");
        aml_add_byte_buffer(pack, (CHAR8*)&FakeVen, 4);
        aml_add_string(pack, "ATY,VendorID");
        aml_add_byte_buffer(pack, (CHAR8*)&FakeVen, 2);
      } else {
        aml_add_string(pack, "ATY,VendorID");
        aml_add_byte_buffer(pack, VenATI, 2);
      }
      
/*      
      aml_add_string(pack, "device-id");
      //     CHAR8 data[] = {0xE1,0x68,0x00,0x00};
      aml_add_byte_buffer(pack, data, sizeof(data));
      aml_add_string(pack, "org-device-id");
      aml_add_byte_buffer(pack, (CHAR8*)&DisplayID[1], 4); 
      aml_add_string(pack, "hda-gfx");
      aml_add_string_buffer(pack, "onboard-1");
      VideoRam = ATI_vram_size(&Displaydevice[1]); 
      aml_add_string(pack, "VRAM,totalsize");
      aml_add_dword(pack, (UINT32)VideoRam); 
      if (!Display1PCIE) {
        aml_add_string(pack, "IOPCIExpressLinkCapabilities");
        aml_add_dword(pack, 0x130d1) ;//0x1e80); 
        aml_add_string(pack, "IOPCIExpressLinkStatus");
        aml_add_dword(pack, 0x10880); //0x880); 
      }
      */
      aml_add_local0(met);
      aml_add_buffer(met, dtgp_1, sizeof(dtgp_1));
      // finish Method(_DSM,4,NotSerialized)
    }
    
    // HDAU
    if (GFXHDAFIX) {
//      AML_CHUNK* met;
      AML_CHUNK* pack;
      AML_CHUNK* device = aml_add_device(root, "HDAU");
      aml_add_name(device, "_ADR");
      aml_add_byte(device, 0x01);
      // add Method(_DSM,4,NotSerialized) for GFX0
      met = aml_add_method(device, "_DSM", 4);
      met = aml_add_store(met);
      pack = aml_add_package(met);
      //aml_add_string(pack, "device-id");
      //CHAR8 data[] = {0x38,0xAA,0x00,0x00};
      //aml_add_byte_buffer(pack, data, sizeof(data));
      //aml_add_string(pack, "codec-id");
      //aml_add_byte_buffer(pack, (CHAR8*)&GfxcodecId, 4);
      aml_add_string(pack, "layout-id");
      aml_add_byte_buffer(pack, (CHAR8*)&GfxlayoutId, 4);
      aml_add_string(pack, "hda-gfx");
      aml_add_string_buffer(pack, "onboard-2");
      //aml_add_string(pack, "name");
      //aml_add_string(pack, "pci1002,aa38");
      //aml_add_string(pack, "IOName");
      //aml_add_string(pack, "pci1002,aa38");
      //aml_add_string(pack, "layout-id");
      //CHAR8 data1[] = {0x12,0x00,0x00,0x00};
      //aml_add_byte_buffer(pack, data1, sizeof(data1));
      aml_add_string(pack, "PinConfigurations");
      aml_add_byte_buffer(pack, data2, sizeof(data2));
      aml_add_local0(met);
      aml_add_buffer(met, dtgp_1, sizeof(dtgp_1));
      // finish Method(_DSM,4,NotSerialized)
    }
  }
  
  aml_calculate_size(root);  
  display = AllocateZeroPool(root->Size);
  sizeoffset = root->Size;
  aml_write_node(root, display, 0);  
  aml_destroy_node(root);  
  if (DisplayName2) {
    // move data to back for add Display
    if (!DISPLAYFIX) { 
      i = devadr+devsize;
      len = move_data(i, dsdt, len, sizeoffset);
      CopyMem(dsdt+i, display, sizeoffset);
      len = CorrectOuters(dsdt, len, devadr-3, sizeoffset);
    } else {
      i = devadr1+devsize1;
      len = move_data(i, dsdt, len, sizeoffset);
      CopyMem(dsdt+i, display, sizeoffset);
      j = write_size(devadr1, dsdt, len, sizeoffset);
      sizeoffset += j;
      len += j;
      len = CorrectOuters(dsdt, len, devadr1-3, sizeoffset);
    }
  } else {
    len = move_data(PCIADR+PCISIZE, dsdt, len, sizeoffset);
    CopyMem(dsdt+PCIADR+PCISIZE, display, sizeoffset);
    // Fix PCIX size
    k = write_size(PCIADR, dsdt, len, sizeoffset);
    sizeoffset += k;
    len += k;
    len = CorrectOuters(dsdt, len, PCIADR-3, sizeoffset);    
  }

  if (CFGname) {
    FreePool(CFGname);
  }
  if (name) {
    FreePool(name);
  } 
  FreePool(display);
  
  return len;  
}


UINT32 FIXNetwork (UINT8 *dsdt, UINT32 len)
{
  UINT32 i, k;
  UINT32 NetworkADR = 0, BridgeSize, Size;
  UINT32 PCIADR, PCISIZE = 0;
  INT32 sizeoffset;
  AML_CHUNK* met;
  AML_CHUNK* root;
  AML_CHUNK* pack;
  CHAR8 *network;
  UINT32 FakeID = 0;
  UINT32 FakeVen = 0;
  CHAR8 NameCard[32];
  
  if (!NetworkADR1) return len;
  DBG("Start NetWork Fix\n");
  
  if (gSettings.FakeLAN) {
    FakeID = gSettings.FakeLAN >> 16;
    FakeVen = gSettings.FakeLAN & 0xFFFF;
    AsciiSPrint(NameCard, 32, "pci%x,%x\0", FakeVen, FakeID);
    Netmodel = get_net_model((FakeVen << 16) + FakeID);
  }
 
  PCIADR = GetPciDevice(dsdt, len);
  if (PCIADR) {
    PCISIZE = get_size(dsdt, PCIADR);
  }
  if (!PCISIZE) return len; //what is the bad DSDT ?!
  NetworkName = FALSE;
  // Network Address
  for (i=0x20; i<len-10; i++) {
    if (CmpAdr(dsdt, i, NetworkADR1)) {
      //this is not a LAN, this is bridge of the lan
      // we have to find next Device
      /*
       Device (NIC)
       {
       Name (_ADR, Zero) - this is NetworkADR2
       */
      //         DBG("found NetworkADR1=%x at %x\n", NetworkADR1, i);
      NetworkADR = devFind(dsdt, i);
      if (!NetworkADR) {
        continue;
      }

      //         DBG("now NetworkADR=%x\n", NetworkADR);
      BridgeSize = get_size(dsdt, NetworkADR);
      if (!BridgeSize) {
        continue;
      }
      //      DBG("its size=%x\n", BridgeSize);
      if (NetworkADR2 != 0xFFFE){
        for (k=NetworkADR+9; k<NetworkADR+BridgeSize; k++) {
          if (CmpAdr(dsdt, k, NetworkADR2))
          {
            NetworkADR = devFind(dsdt, k);
            if (!NetworkADR) {
              continue;
            }

            device_name[1] = AllocateZeroPool(5);
            CopyMem(device_name[1], dsdt+k, 4);
            DBG("found NetWork device NAME(_ADR,0x%08x) at %x And Name is %a\n", 
                NetworkADR2, NetworkADR, device_name[1]);
            ReplaceName(dsdt + NetworkADR, BridgeSize, device_name[1], "GIGE");
            NetworkName = TRUE;   
            break;
          }
        }
        if (!NetworkName) {
          DBG("have no Network device while NetworkADR2=%x\n", NetworkADR2);
          //in this case NetworkADR point to bridge
        }
      }
      break;
    } // End if Network
  }
  if (NetworkADR) { // bridge or device
    i = NetworkADR;
    Size = get_size(dsdt, i);
    k = FindMethod(dsdt + i, Size, "_DSM");
    if (k != 0) {
      DBG("_DSM already exists, patch LAN will not be applied\n");
      return len;
    }
  }  
    
  root = aml_create_node(NULL);
  
  DBG("NetworkADR1=%x NetworkADR2=%x\n", NetworkADR1, NetworkADR2);
  if (!NetworkName) //there is no network device at dsdt, creating new one
  {
    AML_CHUNK* dev = aml_add_device(root, "GIGE");
    aml_add_name(dev, "_ADR");
    if (NetworkADR2) {
      if (NetworkADR2> 0x3F)
        aml_add_dword(dev, NetworkADR2);
      else
        aml_add_byte(dev, (UINT8)NetworkADR2);
    } else {
      aml_add_byte(dev, 0x00);
    }
    met = aml_add_method(dev, "_DSM", 4);
  } else {
    met = aml_add_method(root, "_DSM", 4);
  }  
  // add Method(_DSM,4,NotSerialized) for network
  met = aml_add_store(met);
  pack = aml_add_package(met);
  aml_add_string(pack, "built-in");
  aml_add_byte_buffer(pack, dataBuiltin, sizeof(dataBuiltin));
  aml_add_string(pack, "model");
  aml_add_string_buffer(pack, Netmodel);
  aml_add_string(pack, "device_type");
  aml_add_string_buffer(pack, "Ethernet");
  if (gSettings.FakeLAN) {
//    aml_add_string(pack, "model");
//    aml_add_string_buffer(pack, "Apple LAN card");
    aml_add_string(pack, "device-id");
    aml_add_byte_buffer(pack, (CHAR8 *)&FakeID, 4);
    aml_add_string(pack, "vendor-id");
    aml_add_byte_buffer(pack, (CHAR8 *)&FakeVen, 4);
    aml_add_string(pack, "name");
    aml_add_string_buffer(pack, &NameCard[0]);
  }

  aml_add_local0(met);
  aml_add_buffer(met, dtgp_1, sizeof(dtgp_1));
  // finish Method(_DSM,4,NotSerialized)
  aml_calculate_size(root);  
  network = AllocateZeroPool(root->Size);
  sizeoffset = root->Size;
  DBG("network DSM created, size=%x\n", sizeoffset);
  aml_write_node(root, network, 0);  
  aml_destroy_node(root);
  if (NetworkADR) { // bridge or lan
    i = NetworkADR;
  } else { //this is impossible
    i = PCIADR;
  }  
  Size = get_size(dsdt, i);
//  DBG("Will attach to %x size %x\n", i, Size);
  // move data to back for add patch 
  k = i + Size;
  len = move_data(k, dsdt, len, sizeoffset);
//  DBG("data moved, len=%x\n", len);
  CopyMem(dsdt+k, network, sizeoffset);
  // Fix Device network size
  k = write_size(i, dsdt, len, sizeoffset);
  sizeoffset += k;
  len += k;
//  DBG("size written, shift=%x\n", k);
  len = CorrectOuters(dsdt, len, i-3, sizeoffset);
//  DBG("final len=%x\n", len);
  FreePool(network);
  return len;
}

CHAR8 dataBCM[]  = {0x12, 0x43, 0x00, 0x00};
CHAR8 data1ATH[] = {0x2a, 0x00, 0x00, 0x00};
CHAR8 data2ATH[] = {0x8F, 0x00, 0x00, 0x00};
CHAR8 data3ATH[] = {0x6B, 0x10, 0x00, 0x00};

UINT32 FIXAirport (UINT8 *dsdt, UINT32 len)
{
  UINT32  i, k;
  UINT32 ArptADR = 0, BridgeSize, Size;
  UINT32 PCIADR, PCISIZE = 0;
  INT32 sizeoffset;
  AML_CHUNK* met;
  AML_CHUNK* root;
  AML_CHUNK* pack;
  CHAR8 *network;
  UINT32 FakeID = 0;
  UINT32 FakeVen = 0;
  CHAR8 NameCard[32];

  if (!ArptADR1) return len; // no device - no patch
  
  if (gSettings.FakeWIFI) {
    FakeID = gSettings.FakeWIFI >> 16;
    FakeVen = gSettings.FakeWIFI & 0xFFFF;
    AsciiSPrint(NameCard, 32, "pci%x,%x\0", FakeVen, FakeID);
  }
  
  PCIADR = GetPciDevice(dsdt, len);
  if (PCIADR) {
    PCISIZE = get_size(dsdt, PCIADR);
  }
  if (!PCISIZE) return len; //what is the bad DSDT ?!
  
  DBG("Start Airport Fix\n");
  ArptName = FALSE;
  for (i=0x20; i<len-10; i++) {
    // AirPort Address
    if (CmpAdr(dsdt, i, ArptADR1)) {
      ArptADR = devFind(dsdt, i);
      if (!ArptADR) {
        continue;
      }
      BridgeSize = get_size(dsdt, ArptADR);
      if(!BridgeSize) continue;
      if (ArptADR2 != 0xFFFE){
        for (k=ArptADR+9; k<ArptADR+BridgeSize; k++) {
          if (CmpAdr(dsdt, k, ArptADR2)) {
            ArptADR = devFind(dsdt, k);
            if (!ArptADR) {
              continue;
            }
            device_name[9] = AllocateZeroPool(5);
            CopyMem(device_name[9], dsdt+k, 4);
            DBG("found Airport device NAME(_ADR,0x%08x)/(0x%x) at %x And Name is %a\n", 
                ArptADR1, ArptADR2, ArptADR, device_name[9]);
            ReplaceName(dsdt + ArptADR, BridgeSize, device_name[9], "ARPT");
            ArptName = TRUE;
            break;
          }
        }
      }
      break;
    } // End ArptADR2
  }
  if (ArptADR) { // bridge or device
    i = ArptADR;
    Size = get_size(dsdt, i);
    k = FindMethod(dsdt + i, Size, "_DSM");
    if (k != 0) {
      DBG("_DSM already exists, patch WiFi will not be applied\n");
      return len;
    }
  }  
    
  root = aml_create_node(NULL);
  if (!ArptName) {//there is no Airport device at dsdt, creating new one
    AML_CHUNK* dev = aml_add_device(root, "ARPT");
    aml_add_name(dev, "_ADR");
    if (ArptADR2) {
      if (ArptADR2> 0x3F)
        aml_add_dword(dev, ArptADR2);
      else
        aml_add_byte(dev, (UINT8)ArptADR2);
    } else {
      aml_add_byte(dev, 0x00);
    }
    met = aml_add_method(dev, "_DSM", 4);
  } else {
    met = aml_add_method(root, "_DSM", 4);
  }  
  // add Method(_DSM,4,NotSerialized) for network
  met = aml_add_store(met);
  pack = aml_add_package(met);
  aml_add_string(pack, "built-in");  
  aml_add_byte_buffer(pack, dataBuiltin, sizeof(dataBuiltin));
  
  if (gSettings.FakeWIFI) {
    aml_add_string(pack, "model");
    aml_add_string_buffer(pack, "Apple WiFi card");
    aml_add_string(pack, "device-id");
    aml_add_byte_buffer(pack, (CHAR8 *)&FakeID, 4);
    aml_add_string(pack, "vendor-id");
    aml_add_byte_buffer(pack, (CHAR8 *)&FakeVen, 4);
    aml_add_string(pack, "name");
    aml_add_string_buffer(pack, (CHAR8 *)&NameCard[0]);
    aml_add_string(pack, "subsystem-id");
    aml_add_byte_buffer(pack, data2ATH, 4);
    aml_add_string(pack, "subsystem-vendor-id");
    aml_add_byte_buffer(pack, data3ATH, 4);    
  } else
  if (ArptBCM) {
    aml_add_string(pack, "model");
    aml_add_string_buffer(pack, "Dell Wireless 1395");
    aml_add_string(pack, "name");
    aml_add_string_buffer(pack, "pci14e4,4312");
    aml_add_string(pack, "device-id");
    aml_add_byte_buffer(pack, dataBCM, 4);
  } else if (ArptAtheros) {
    aml_add_string(pack, "model");
    aml_add_string_buffer(pack, "Atheros AR9285 WiFi card");
    aml_add_string(pack, "name");
    aml_add_string_buffer(pack, "pci168c,2a");
    aml_add_string(pack, "device-id");
    aml_add_byte_buffer(pack, data1ATH, 4);        
    aml_add_string(pack, "subsystem-id");
    aml_add_byte_buffer(pack, data2ATH, 4);
    aml_add_string(pack, "subsystem-vendor-id");
    aml_add_byte_buffer(pack, data3ATH, 4);        
  }
  aml_add_string(pack, "device_type");
  aml_add_string_buffer(pack, "Airport");
  aml_add_string(pack, "AAPL,slot-name");
  aml_add_string_buffer(pack, "AirPort");
  
  aml_add_local0(met);
  aml_add_buffer(met, dtgp_1, sizeof(dtgp_1));
  // finish Method(_DSM,4,NotSerialized)
  
  aml_calculate_size(root);  
  network = AllocateZeroPool(root->Size);  
  sizeoffset = root->Size;  
  aml_write_node(root, network, 0);  
  aml_destroy_node(root);
  DBG("AirportADR=%x add patch size=%x\n", ArptADR, sizeoffset);
  
  if (ArptADR) { // bridge or WiFi
    i = ArptADR;
  } else { //this is impossible
    i = PCIADR;
  }  
  Size = get_size(dsdt, i);
  DBG("adr %x size of arpt=%x\n", i, Size);
  // move data to back for add patch 
  k = i + Size;
  len = move_data(k, dsdt, len, sizeoffset);
  DBG("move from %x size %x len=%x\n", k, sizeoffset);
  CopyMem(dsdt+k, network, sizeoffset);
  // Fix Device size
  k = write_size(i, dsdt, len, sizeoffset);
  sizeoffset += k;
  len += k;
  DBG("adr %x shift %x so=%x len=%x\n", i, k, sizeoffset, len);
  len = CorrectOuters(dsdt, len, i-3, sizeoffset);
  FreePool(network);
  return len;
}

UINT32 FIXSBUS (UINT8 *dsdt, UINT32 len)
{
  UINT32  i, k;
  UINT32  SBUSADR=0, Size=0;
  UINT32 PCIADR, PCISIZE = 0;
  INT32 sizeoffset;
  PCIADR = GetPciDevice(dsdt, len);
  if (PCIADR) {
    PCISIZE = get_size(dsdt, PCIADR);
  }
  if (!PCISIZE) {
    DBG("wrong PCI0 address, patch SBUS will not be applied\n");
    return len;
  }
  DBG("Start SBUS Fix PCI=%x len=%x\n", PCIADR, len);

  // Find Device SBUS
  if (SBUSADR1) {
    for (i=0x20; i<len-10; i++) {
      if (CmpAdr(dsdt, i, SBUSADR1))
      {
        SBUSADR = devFind(dsdt, i);
        if (SBUSADR) {
          DBG("device (SBUS) found at %x\n", SBUSADR);
          break;
        }
      } // end SBUS
    }
  }
  
  if (SBUSADR) { // bridge or device
    i = SBUSADR;
    Size = get_size(dsdt, i);
    k = FindMethod(dsdt + i, Size, "_DSM");
    if (k != 0) {
      DBG("_DSM already exists, patch SBUS will not be applied\n");
      return len;
    }
    k = FindMethod(dsdt + i, Size, "BUS0");
    if (k != 0) {
      DBG("BUS0 already exists, patch SBUS will not be applied\n");
      return len;
    }    
  }  
  
  if (SBUSADR)
    sizeoffset = sizeof(bus0);
  else
    sizeoffset = sizeof(sbus1); 
  
  DBG("SBUS address %x code size = 0x%08x\n", SBUSADR, sizeoffset);
    
  if (SBUSADR) {
    // move data to back for add sbus 
    i = SBUSADR + Size;
    len = move_data(i, dsdt, len, sizeoffset);
    CopyMem(dsdt+i, bus0, sizeoffset);
    // Fix Device sbus size
    k = write_size(SBUSADR, dsdt, len, sizeoffset);
    sizeoffset += k;
    len += k;
    len = CorrectOuters(dsdt, len, SBUSADR-3, sizeoffset);
  //  SBUSADR = adr1;
    DBG("SBUS code size fix = 0x%08x\n", sizeoffset);
  } else {
    i = PCIADR + PCISIZE;
    DBG("SBUS absent, adding to the end of PCI0 at %x\n", i);
    len = move_data(i, dsdt, len, sizeoffset);
    CopyMem(dsdt+i, sbus1, sizeoffset);
    
    // Fix PCIX size
    k = write_size(PCIADR, dsdt, len, sizeoffset);
    sizeoffset += k;
    len += k;
    DBG("shift=%x so=%x len=%x\n", k, sizeoffset, len);
    len = CorrectOuters(dsdt, len, PCIADR-3, sizeoffset);    
  }
  return len;
}

CHAR8 dataMCHC[] = {0x44,0x00,0x00,0x00};

UINT32 AddMCHC (UINT8 *dsdt, UINT32 len)
{    
  UINT32  k;
  UINT32 PCIADR, PCISIZE = 0;
  INT32 sizeoffset;
  AML_CHUNK* root;
  AML_CHUNK* device;
  AML_CHUNK* met;
  AML_CHUNK* pack;
  CHAR8 *mchc;

  PCIADR = GetPciDevice(dsdt, len);
  if (PCIADR) {
    PCISIZE = get_size(dsdt, PCIADR);
  }
  if (!PCISIZE) {
    DBG("wrong PCI0 address, patch MCHC will not be applied\n");
    return len;
  }
  
  DBG("Start Add MCHC\n");
  root = aml_create_node(NULL);
  	
  device = aml_add_device(root, "MCHC");
  aml_add_name(device, "_ADR");
  aml_add_byte(device, 0x00); 
  
	// add Method(_DSM,4,NotSerialized) for MCHC
  met = aml_add_method(device, "_DSM", 4);
  met = aml_add_store(met);
  pack = aml_add_package(met);
  aml_add_string(pack, "device-id");
  aml_add_byte_buffer(pack, dataMCHC, sizeof(dataMCHC));
  aml_add_string(pack, "name");
  aml_add_string(pack, "pci8086,44");
//  aml_add_string(pack, "IOName");
//  aml_add_string(pack, "pci8086,44");
  aml_add_local0(met);
  aml_add_buffer(met, dtgp_1, sizeof(dtgp_1));
  // finish Method(_DSM,4,NotSerialized) 
  
  aml_calculate_size(root);  
  mchc = AllocateZeroPool(root->Size);
  sizeoffset = root->Size;
  aml_write_node(root, mchc, 0);  
  aml_destroy_node(root);
  // always add on PCIX back
  len = move_data(PCIADR+PCISIZE, dsdt, len, sizeoffset);
  CopyMem(dsdt+PCIADR+PCISIZE, mchc, sizeoffset);
  // Fix PCIX size
  k = write_size(PCIADR, dsdt, len, sizeoffset);
  sizeoffset += k;
  len += k;
  len = CorrectOuters(dsdt, len, PCIADR-3, sizeoffset);    
  FreePool(mchc);
  return len;  
}

UINT32 AddIMEI (UINT8 *dsdt, UINT32 len)
{
  UINT32  k;
  UINT32 PCIADR, PCISIZE = 0;
  INT32 sizeoffset;
  AML_CHUNK* root;
  AML_CHUNK* device;
  CHAR8 *imei;

  PCIADR = GetPciDevice(dsdt, len);
  if (PCIADR) {
    PCISIZE = get_size(dsdt, PCIADR);
  }
  if (!PCISIZE) {
    DBG("wrong PCI0 address, patch IMEI will not be applied\n");
    return len;
  }

  DBG("Start Add IMEI\n");
  root = aml_create_node(NULL);
  device = aml_add_device(root, "IMEI");
  aml_add_name(device, "_ADR");
  aml_add_dword(device, 0x00160000);

  aml_calculate_size(root);
  imei = AllocateZeroPool(root->Size);
  sizeoffset = root->Size;
  aml_write_node(root, imei, 0);
  aml_destroy_node(root);
  // always add on PCIX back
  len = move_data(PCIADR+PCISIZE, dsdt, len, sizeoffset);
  CopyMem(dsdt+PCIADR+PCISIZE, imei, sizeoffset);
  // Fix PCIX size
  k = write_size(PCIADR, dsdt, len, sizeoffset);
  sizeoffset += k;
  len += k;
  len = CorrectOuters(dsdt, len, PCIADR-3, sizeoffset);
  FreePool(imei);
  return len;
}

CHAR8 dataFW[] = {0x00,0x00,0x00,0x00};

UINT32 FIXFirewire (UINT8 *dsdt, UINT32 len)
{
  UINT32  i, k;
  UINT32 FirewireADR = 0, BridgeSize,  Size;
  INT32 sizeoffset;
  UINT32 PCIADR, PCISIZE = 0;
  AML_CHUNK* met;
  AML_CHUNK* root;
  AML_CHUNK* stro;
  AML_CHUNK* pack;
  CHAR8 *firewire;
  
  PCIADR = GetPciDevice(dsdt, len);
  if (PCIADR) {
    PCISIZE = get_size(dsdt, PCIADR);
  }
  if (!PCISIZE) {
    DBG("wrong PCI0 address, patch FRWR will not be applied\n");
    return len;
  }

  // Firewire Address
  for (i=0x20; i<len-10; i++) {
    if (FirewireADR1 != 0x00000000 && 
        CmpAdr(dsdt, i, FirewireADR1)) {
      FirewireADR = devFind(dsdt, i);
      if (!FirewireADR) {
        continue;
      }

      BridgeSize = get_size(dsdt, FirewireADR);
      if (FirewireADR2 != 0xFFFE ){
        for (k=FirewireADR+9; k<FirewireADR+BridgeSize; k++) {
          if (CmpAdr(dsdt, k, FirewireADR2)) {
            FirewireADR = devFind(dsdt, k);
            if (!FirewireADR) {
              continue;
            }

            device_name[2] = AllocateZeroPool(5);
            CopyMem(device_name[2], dsdt+k, 4);
            DBG("found Firewire device NAME(_ADR,0x%08x) at %x And Name is %a\n", 
                FirewireADR2, k, device_name[2]);
            ReplaceName(dsdt + FirewireADR, BridgeSize, device_name[2], "FRWR");
            FirewireName = TRUE;   
            break;
          }
        }
      }
      break;
    } // End Firewire
  }
  //safe for twice fix: if _DSM already present then cancel fix
  if (FirewireADR) { // bridge or device
    i = FirewireADR;
  } else { 
    i = PCIADR;
  }  
  Size = get_size(dsdt, i);
  k = FindMethod(dsdt + i, Size, "_DSM");
  if (k != 0) {
    DBG("_DSM already exists, patch FRWR will not be applied\n");
    return len;
  }
  
  root = aml_create_node(NULL);
  
  DBG("Start Firewire Fix\n");
  
  if (!FirewireName) {
    AML_CHUNK* device = aml_add_device(root, "FRWR");
    aml_add_name(device, "_ADR");
    if (FirewireADR2) {
      if (FirewireADR2 <= 0x3F) {
        aml_add_byte(device, (UINT8)FirewireADR2);
      } else {
        aml_add_dword(device, FirewireADR2);
      }
    } else aml_add_byte(device, 0);
    aml_add_name(device, "_GPE");
    aml_add_byte(device, 0x1A);
    met = aml_add_method(device, "_DSM", 4);
  } else
    met = aml_add_method(root, "_DSM", 4);
  stro = aml_add_store(met);
  pack = aml_add_package(stro);
  aml_add_string(pack, "fwhub");
  aml_add_byte_buffer(pack, dataFW, sizeof(dataFW));
  aml_add_local0(stro);
  aml_add_buffer(met, dtgp_1, sizeof(dtgp_1));
  // finish Method(_DSM,4,NotSerialized)  
  
  aml_calculate_size(root);
  firewire = AllocateZeroPool(root->Size);
  sizeoffset = root->Size;
  aml_write_node(root, firewire, 0);
  aml_destroy_node(root);
  
  // move data to back for add patch 
  k = i + Size;
  len = move_data(k, dsdt, len, sizeoffset);
  CopyMem(dsdt+k, firewire, sizeoffset);
  // Fix Device size
  k = write_size(i, dsdt, len, sizeoffset);
  sizeoffset += k;
  len += k;
  len = CorrectOuters(dsdt, len, i-3, sizeoffset);
  FreePool(firewire);
  return len;
}

UINT32 AddHDEF (UINT8 *dsdt, UINT32 len)
{  
  UINT32  i, k;
  UINT32 PCIADR, PCISIZE = 0;
  INT32 sizeoffset;
  UINT32 HDAADR = 0, BridgeSize = 0, Size;
  AML_CHUNK* root;
  AML_CHUNK* met;
  AML_CHUNK* device;
  AML_CHUNK* pack;
  CHAR8 *hdef;

  PCIADR = GetPciDevice(dsdt, len);
  if (PCIADR) {
    PCISIZE = get_size(dsdt, PCIADR);
  }
  if (!PCISIZE) return len; //what is the bad DSDT ?!
  DBG("Start HDA Fix\n");
//  len = DeleteDevice("AZAL", dsdt, len);
 
  // HDA Address
  for (i=0x20; i<len-10; i++) {
    if (HDAADR1 != 0x00000000 && HDAFIX &&
        CmpAdr(dsdt, i, HDAADR1)) {
      HDAADR = devFind(dsdt, i);
      if (!HDAADR) {
        continue;
      }

      BridgeSize = get_size(dsdt, HDAADR);
      device_name[4] = AllocateZeroPool(5);
      CopyMem(device_name[4], dsdt+i, 4);
      DBG("found HDA device NAME(_ADR,0x%08x) And Name is %a\n", 
          HDAADR1, device_name[4]);
      ReplaceName(dsdt, len, device_name[4], "HDEF");
      HDAFIX = FALSE;
      break;
    } // End HDA
  }
  
  if (HDAADR) { // bridge or device
    i = HDAADR;
    Size = get_size(dsdt, i);
    k = FindMethod(dsdt + i, Size, "_DSM");
    if (k != 0) {
      DBG("_DSM already exists, patch HDA will not be applied\n");
      return len;
    }
  }
  
  root = aml_create_node(NULL);
  if (HDAFIX) {
    DBG("Start Add Device HDEF\n");
    device = aml_add_device(root, "HDEF");
    aml_add_name(device, "_ADR");
    aml_add_dword(device, 0x001B0000);  
    
    // add Method(_DSM,4,NotSerialized) 
    met = aml_add_method(device, "_DSM", 4);
  } else
    met = aml_add_method(root, "_DSM", 4);
  
  met = aml_add_store(met);
  pack = aml_add_package(met);
  //aml_add_string(pack, "codec-id");
  //aml_add_byte_buffer(pack, (CHAR8*)&HDAcodecId, 4);
  aml_add_string(pack, "layout-id");
  aml_add_byte_buffer(pack, (CHAR8*)&HDAlayoutId, 4);
  aml_add_string(pack, "PinConfigurations");
  //CHAR8 data[] = {};
  aml_add_byte_buffer(pack, 0, 0);//data, sizeof(data));
  aml_add_local0(met);
  aml_add_buffer(met, dtgp_1, sizeof(dtgp_1));
  // finish Method(_DSM,4,NotSerialized)
  
  aml_calculate_size(root);  
  hdef = AllocateZeroPool(root->Size);
  sizeoffset = root->Size;
  aml_write_node(root, hdef, 0);
  aml_destroy_node(root);
  
  if (!HDAFIX) { // bridge or device
    i = HDAADR;
  } else { 
    i = PCIADR;
  }  
  Size = get_size(dsdt, i);
  // move data to back for add patch 
  k = i + Size;
  len = move_data(k, dsdt, len, sizeoffset);
  CopyMem(dsdt+k, hdef, sizeoffset);
  // Fix Device size
  k = write_size(i, dsdt, len, sizeoffset);
  sizeoffset += k;
  len += k;
  len = CorrectOuters(dsdt, len, i-3, sizeoffset);
  FreePool(hdef);
  return len;
}

UINT32 FIXUSB (UINT8 *dsdt, UINT32 len)
{
  UINT32 i, j, k;
  UINT32 Size, size1, size2, size3, size4;
  UINT32 adr=0, adr1=0;
  INT32 sizeoffset;
  AML_CHUNK* root;
  AML_CHUNK* root1;
  AML_CHUNK* met;
  AML_CHUNK* pack;
  AML_CHUNK* met1;
  AML_CHUNK* pack1;
  CHAR8 *USBDATA1;
  CHAR8 *USBDATA2;
  CHAR8 *USBDATA3;
  CHAR8 *USBDATA4;
  
  DBG("Start USB Fix\n");
	//DBG("len = 0x%08x\n", len);
  
	root = aml_create_node(NULL);
	root1 = aml_create_node(NULL);
  
  // add Method(_DSM,4,NotSerialized) for USB
  met = aml_add_method(root, "_DSM", 4);
  met = aml_add_store(met);
  pack = aml_add_package(met);
  aml_add_string(pack, "device-id");
  aml_add_byte_buffer(pack, (/* CONST*/ CHAR8*)&USBID[0], 4);
  aml_add_string(pack, "built-in");
  aml_add_byte_buffer(pack, dataBuiltin, sizeof(dataBuiltin));
  aml_add_string(pack, "device_type");
    if (USBIntel) {
  aml_add_string_buffer(pack, "UHCI");
    } else if (USBNForce) {
        aml_add_string_buffer(pack, "OHCI");
    }
  if (gSettings.InjectClockID) {
    aml_add_string(pack, "AAPL,clock-id");
    aml_add_byte_buffer(pack, dataBuiltin, 1);
  }
  
  aml_add_local0(met);
  aml_add_buffer(met, dtgp_1, sizeof(dtgp_1));
  // finish Method(_DSM,4,NotSerialized)      
  aml_calculate_size(root);
  
  USBDATA1 = AllocateZeroPool(root->Size);
  size1 = root->Size;
  //DBG("USB code size = 0x%08x\n", sizeoffset);
  aml_write_node(root, USBDATA1, 0);
  aml_destroy_node(root);
  
  // add Method(_DSM,4,NotSerialized) for USB2
  met1 = aml_add_method(root1, "_DSM", 4);
  met1 = aml_add_store(met1);
  pack1 = aml_add_package(met1);
  aml_add_string(pack1, "device-id");
  aml_add_byte_buffer(pack1, (/* CONST*/ CHAR8*)&USBID[0], 4);
  aml_add_string(pack1, "built-in");
  aml_add_byte_buffer(pack1, dataBuiltin, sizeof(dataBuiltin));
  aml_add_string(pack1, "device_type");
  aml_add_string_buffer(pack1, "EHCI");
  if (gSettings.InjectClockID) {
    aml_add_string(pack1, "AAPL,clock-id");
    aml_add_byte_buffer(pack1, dataBuiltin, sizeof(dataBuiltin));
  }
  if (USBIntel) {
  aml_add_string(pack1, "AAPL,current-available");
  aml_add_word(pack1, 0x05DC);
  aml_add_string(pack1, "AAPL,current-extra");
  aml_add_word(pack1, 0x03E8);
  aml_add_string(pack1, "AAPL,current-in-sleep");
  aml_add_word(pack1, 0x0BB8);
//  aml_add_string(pack1, "AAPL,device-internal");
//  aml_add_byte(pack1, 0x02);
  } else if (USBNForce) {
    aml_add_string(pack1, "AAPL,current-available");
    aml_add_word(pack1, 0x04B0);
    aml_add_string(pack1, "AAPL,current-extra");
    aml_add_word(pack1, 0x02BC);
    aml_add_string(pack1, "AAPL,current-in-sleep");
    aml_add_word(pack1, 0x03E8);
  }

  aml_add_byte_buffer(pack1, dataBuiltin, sizeof(dataBuiltin));
  aml_add_local0(met1);
  aml_add_buffer(met1, dtgp_1, sizeof(dtgp_1));
  // finish Method(_DSM,4,NotSerialized)
  
  aml_calculate_size(root1);
  USBDATA2 = AllocateZeroPool(root1->Size);
  size2 = root1->Size;
  //DBG("USB code size = 0x%08x\n", sizeoffset);
  aml_write_node(root1, USBDATA2, 0);
  aml_destroy_node(root1);
 
  //NFORCE_USB_START
  aml_calculate_size(root1);
  USBDATA4 = AllocateZeroPool(root1->Size);
  size4 = root1->Size;
  //DBG("USB code size = 0x%08x\n", sizeoffset);
  aml_write_node(root1, USBDATA4, 0);
  aml_destroy_node(root1);
  //NFORCE_USB_END
    
  // add Method(_DSM,4,NotSerialized) for USB3
  root1 = aml_create_node(NULL);
  met1 = aml_add_method(root1, "_DSM", 4);
  met1 = aml_add_store(met1);
  pack1 = aml_add_package(met1);
  aml_add_string(pack1, "device-id");
  aml_add_byte_buffer(pack1, (/* CONST*/ CHAR8*)&USBID[0], 4);
  aml_add_string(pack1, "built-in");
  aml_add_byte_buffer(pack1, dataBuiltin, sizeof(dataBuiltin));
  aml_add_string(pack1, "device_type");
  aml_add_string_buffer(pack1, "XHCI");
  if (gSettings.InjectClockID) {
    aml_add_string(pack1, "AAPL,clock-id");
    aml_add_byte_buffer(pack1, dataBuiltin, sizeof(dataBuiltin));
  }
  aml_add_string(pack1, "AAPL,current-available");
  aml_add_word(pack1, 0x0834);
  aml_add_string(pack1, "AAPL,current-extra");
  aml_add_word(pack1, 0x0A8C);
  aml_add_string(pack1, "AAPL,current-in-sleep");
  aml_add_word(pack1, 0x0A8C);
  aml_add_string(pack1, "AAPL,max-port-current-in-sleep");
  aml_add_word(pack1, 0x0834);
  aml_add_string(pack1, "AAPL,device-internal");
  aml_add_byte(pack1, 0x00);
  
  aml_add_byte_buffer(pack1, dataBuiltin, sizeof(dataBuiltin));
  aml_add_local0(met1);
  aml_add_buffer(met1, dtgp_1, sizeof(dtgp_1));
  // finish Method(_DSM,4,NotSerialized)
  
  aml_calculate_size(root1);
  USBDATA3 = AllocateZeroPool(root1->Size);
  size3 = root1->Size;
  //DBG("USB code size = 0x%08x\n", sizeoffset);
  aml_write_node(root1, USBDATA3, 0);
  aml_destroy_node(root1);
  
  if (usb > 0) {
    
    for (i = 0; i < usb; i++) {
      INTN XhciCount = 1;
      INTN EhciCount = 0;
      // find USB adr
      for (j = 0; j < len - 4; j++) {
        if (CmpAdr(dsdt, j, USBADR[i])) {
          XhciName = FALSE;
          UsbName[i] = AllocateZeroPool(5);
          CopyMem(UsbName[i], dsdt+j, 4);
          
          adr1 = devFind(dsdt, j);
          if (!adr1) {
            continue;
          }

          Size = get_size(dsdt, adr1); //bridgesize
          DBG("USB bridge[%x] at %x, size = %x\n", USBADR[i], adr1, Size);
          if (USBADR2[i] != 0xFFFE ){
            for (k = adr1 + 9; k < adr1 + Size; k++) {
              if (CmpAdr(dsdt, k, USBADR2[i])) {
                adr = devFind(dsdt, k);
                if (!adr) {
                  continue;
                }

                device_name[10] = AllocateZeroPool(5);
                CopyMem(device_name[10], dsdt+k, 4);
                DBG("found USB device NAME(_ADR,0x%08x) at %x and Name is %a\n",
                    USBADR2[i], k, device_name[10]);
                if (USB30[i]) {
                  AsciiSPrint(UsbName[i], 5, "XHC%d", XhciCount++);
                } else if (USB20[i]) {
                  AsciiSPrint(UsbName[i], 5, "EHC%d", EhciCount++);
                } else {
                  AsciiSPrint(UsbName[i], 5, "USB%d", i);
                }
                ReplaceName(dsdt + adr1, Size, device_name[10], UsbName[i]);
                XhciName = TRUE;
                Size = get_size(dsdt, adr);
                break;
              }
            }
          }
          if (!XhciName) {
            adr = adr1;
          }

          k = FindMethod(dsdt + adr, Size, "_DSM");
          if (k != 0) {
            DBG("_DSM already exists, patch USB will not be applied\n");
            continue;
          }
          

          //UINT32 k = (adr > 0x3F)?1:0; 
          /*
          14 45 06 5F 44 53 4D 04 70 12 4F 04 08 0D 64 65
          76 69 63 65 2D 69 64 00 11 07 0A 04 31 1E 00 00
          0D 62 75 69 6C 74 2D 69 6E 00 11 04 0A 01 00 0D
          64 65 76 69 63 65 5F 74 79 70 65 00 11 08 0A 04
          55 48 43 49 00 0D 41 41 50 4C 2C 63 6C 6F 63 6B
          2D 69 64 00 11 04 0A 01 00 60 44 54 47 50 68 69
          6A 6B 71 60 A4 60 
           */
          if (USB30[i]) {
            if ((USBDATA3[25] == 0x0A) && (USBDATA3[26] == 0x04)) {
              k = 27;
            } else if ((USBDATA3[26] == 0x0A) && (USBDATA3[27] == 0x04)) {
              k = 28;
            } else {
              continue;
            }
            if (gSettings.FakeXHCI) {
              USBID[i] = gSettings.FakeXHCI >> 16;
            }            
            CopyMem(USBDATA3+k, (VOID*)&USBID[i], 4);
            sizeoffset = size3;
          } else if (USB20[i]) {
            if ((USBDATA2[25] == 0x0A) && (USBDATA2[26] == 0x04)) {
              k = 27;
            } else if ((USBDATA2[26] == 0x0A) && (USBDATA2[27] == 0x04)) {
              k = 28;
            } else {
              continue;
            }            
            CopyMem(USBDATA2+k, (VOID*)&USBID[i], 4);
            sizeoffset = size2;
          } else {
            if ((USBDATA1[25] == 0x0A) && (USBDATA1[26] == 0x04)) {
              k = 27;
            } else if ((USBDATA1[26] == 0x0A) && (USBDATA1[27] == 0x04)) {
              k = 28;
            } else {
              continue;
            }
            
            CopyMem(USBDATA1+k, (VOID*)&USBID[i], 4);
            sizeoffset = size1;
          }
          
          len = move_data(adr + Size, dsdt, len, sizeoffset);
          if (USB30[i]) {
            CopyMem(dsdt + adr + Size, USBDATA3, sizeoffset);
          } else if (USB20[i]) {
            CopyMem(dsdt + adr + Size, USBDATA2, sizeoffset);
          } else {
            CopyMem(dsdt + adr + Size, USBDATA1, sizeoffset);
          }
          // Fix Device USB size
          k = write_size(adr, dsdt, len, sizeoffset);
          sizeoffset += k;
          len += k;
          len = CorrectOuters(dsdt, len, adr-3, sizeoffset);
          break;
        }  
          //NFORCE_USB_START
        else if (CmpAdr(dsdt, j, USBADR3[i]))
        {
            UsbName[i] = AllocateZeroPool(5);
            CopyMem(UsbName[i], dsdt+j, 4);
            
            adr1 = devFind(dsdt, j);
            if (!adr1) {
                continue;
            }
            
            adr = get_size(dsdt, adr1);
            //UINT32 k = (adr > 0x3F)?1:0;
            /*
             14 45 06 5F 44 53 4D 04 70 12 4F 04 08 0D 64 65
             76 69 63 65 2D 69 64 00 11 07 0A 04 31 1E 00 00
             0D 62 75 69 6C 74 2D 69 6E 00 11 04 0A 01 00 0D
             64 65 76 69 63 65 5F 74 79 70 65 00 11 08 0A 04
             55 48 43 49 00 0D 41 41 50 4C 2C 63 6C 6F 63 6B
             2D 69 64 00 11 04 0A 01 00 60 44 54 47 50 68 69
             6A 6B 71 60 A4 60
             */
            
            if (USB40[i]) {
                if ((USBDATA4[25] == 0x0A) && (USBDATA4[26] == 0x04)) {
                    k = 27;
                } else if ((USBDATA4[26] == 0x0A) && (USBDATA4[27] == 0x04)) {
                    k = 28;
                } else {
                    continue;
                }
                
                CopyMem(USBDATA4+k, (VOID*)&USBID[i], 4);
                sizeoffset = size4;
                
                
            } else {
                if ((USBDATA1[25] == 0x0A) && (USBDATA1[26] == 0x04)) {
                    k = 27;
                } else if ((USBDATA1[26] == 0x0A) && (USBDATA1[27] == 0x04)) {
                    k = 28;
                } else {
                    continue;
                }
                
                CopyMem(USBDATA1+k, (VOID*)&USBID[i], 4);
                sizeoffset = size1;
            }
            
            len = move_data(adr1+adr, dsdt, len, sizeoffset);
            
            if (USB40[i]) {
                CopyMem(dsdt+adr1+adr, USBDATA4, sizeoffset);
                
            } else {
                CopyMem(dsdt+adr1+adr, USBDATA1, sizeoffset);
            }
            // Fix Device USB size
            k = write_size(adr1, dsdt, len, sizeoffset);
            sizeoffset += k;
            len += k;
            len = CorrectOuters(dsdt, len, adr1-3, sizeoffset);
            break;
        }
          //NFORCE_USB_END
      }
    }
  }
  FreePool(USBDATA1);
  FreePool(USBDATA2);
  FreePool(USBDATA3);
  FreePool(USBDATA4);
  return len;  
}


CHAR8 DevIDE[] = {0x9E,0x26,0x00,0x00};
CHAR8 VenIDE[] = {0x86,0x80,0x00,0x00};

UINT32 FIXIDE (UINT8 *dsdt, UINT32 len)
{
  UINT32  i, k;
  UINT32 j;
  UINT32 IDEADR = 0, BridgeSize = 0, Size;
  INT32 sizeoffset;
  AML_CHUNK* root;
  AML_CHUNK* device;
  CHAR8 *ide;
  BOOLEAN PATAFIX=TRUE;

  if (!IDEADR1) return len;
  
  for (i=0x20; i<len-10; i++) {    
    if (CmpAdr(dsdt, i, IDEADR1)) {
              DBG("Found IDEADR1=%x at %x\n", IDEADR1, i);
      IDEADR = devFind(dsdt, i);
      if (!IDEADR) {
        continue;
      }

      BridgeSize = get_size(dsdt, IDEADR);
      if (BridgeSize) break;
    } // End IDE
  }
  if (!BridgeSize) return len;
  DBG("Start IDE Fix\n");
  // find Name(_ADR, Zero) if yes, don't need to inject PATA name
  for (j=IDEADR+9; j<IDEADR+BridgeSize; j++)
  {
    if (CmpAdr(dsdt, j, 0))
    {
      PATAFIX = FALSE;
      break;
    }
  } 
  
  if (IDEADR) { // bridge or device
    i = IDEADR;
    Size = get_size(dsdt, i);
    k = FindMethod(dsdt + i, Size, "_DSM");
    if (k != 0) {
      DBG("_DSM already exists, patch IDE will not be applied\n");
      return len;
    }
  }  
  
  
	root = aml_create_node(NULL);
	device = root;
	if (PATAFIX)
	{
    AML_CHUNK* met;
    AML_CHUNK* pack;
    AML_CHUNK* device1;
    AML_CHUNK* device2;
    device = aml_add_device(root, "ICHX");
    aml_add_name(device, "_ADR");
    if (IDEADR2 < 0x3F) {
      aml_add_byte(device, (UINT8)IDEADR2);
    } else {
      aml_add_dword(device, IDEADR2);
    }
    met = aml_add_method(device, "_DSM", 4);
    met = aml_add_store(met);
    pack = aml_add_package(met);
    aml_add_string(pack, "device-id");
    aml_add_byte_buffer(pack, DevIDE, sizeof(DevIDE));
    aml_add_string(pack, "vendor-id");
    aml_add_byte_buffer(pack, VenIDE, sizeof(VenIDE));
    aml_add_string(pack, "name");
    aml_add_string(pack, "pci8086,269e");
    aml_add_string(pack, "IOName");
    aml_add_string(pack, "pci8086,269e");
    aml_add_local0(met);
    aml_add_buffer(met, dtgp_1, sizeof(dtgp_1));
    device1 = aml_add_device(device, "PRIM");
    aml_add_name(device1, "_ADR");
    aml_add_byte(device1, 0x00);
    device2 = aml_add_device(device1, "MAST");
    aml_add_name(device2, "_ADR");
    aml_add_byte(device2, 0x00);
    device2 = aml_add_device(device1, "SLAV");
    aml_add_name(device2, "_ADR");
    aml_add_byte(device2, 0x01);
    // Marvell only one connected cable
    //AML_CHUNK* device3 = aml_add_device(device, "SLAB");
    //aml_add_name(device3, "_ADR");
    //aml_add_byte(device3, 0x00);
    //AML_CHUNK* device4 = aml_add_device(device3, "MAST");
    //aml_add_name(device4, "_ADR");
    //aml_add_byte(device4, 0x00);
    //device4 = aml_add_device(device3, "SLAV");
    //aml_add_name(device4, "_ADR");
    //aml_add_byte(device4, 0x01);
    
  }
  else
  {
    AML_CHUNK* pack;
    AML_CHUNK* met = aml_add_method(root, "_DSM", 4);
    met = aml_add_store(met);
    pack = aml_add_package(met);
    aml_add_string(pack, "device-id");
    aml_add_byte_buffer(pack, DevIDE, sizeof(DevIDE));
    aml_add_string(pack, "vendor-id");
    aml_add_byte_buffer(pack, VenIDE, sizeof(VenIDE));
    aml_add_string(pack, "name");
    aml_add_string(pack, "pci8086,269e");
    aml_add_string(pack, "IOName");
    aml_add_string(pack, "pci8086,269e");
    aml_add_local0(met);
    aml_add_buffer(met, dtgp_1, sizeof(dtgp_1));
  }
  // finish Method(_DSM,4,NotSerialized) 
  
  aml_calculate_size(root);  
  ide = AllocateZeroPool(root->Size);
  sizeoffset = root->Size;
  aml_write_node(root, ide, 0);  
  aml_destroy_node(root);
    // move data to back for add DSM 
  j = IDEADR + BridgeSize;
    len = move_data(j, dsdt, len, sizeoffset);
    CopyMem(dsdt+j, ide, sizeoffset);  
    // Fix Device ide size
    k = write_size(IDEADR, dsdt, len, sizeoffset);   
  sizeoffset += k;
  len += k;
  len = CorrectOuters(dsdt, len, IDEADR-3, sizeoffset);
    
    //add patafix
  sizeoffset = sizeof(patafix);
  DBG("add patafix size=%x\n", sizeoffset);
  i = get_size(dsdt, IDEADR);
  j = IDEADR + i;
  len = move_data(j, dsdt, len, sizeoffset);
  CopyMem(dsdt+j, patafix, sizeoffset); 
  k = write_size(IDEADR, dsdt, len, sizeoffset);
  sizeoffset += k;
  len += k;
  len = CorrectOuters(dsdt, len, IDEADR-3, sizeoffset);
  FreePool(ide);
  return len;
}

CHAR8 DevSATA[] = {0x81, 0x26, 0x00, 0x00};

UINT32 FIXSATAAHCI (UINT8 *dsdt, UINT32 len)
{
  UINT32  i, k;
  UINT32 SATAAHCIADR = 0, BridgeSize = 0, Size;
  INT32 sizeoffset;
  AML_CHUNK* root;
  AML_CHUNK* met;
  AML_CHUNK* pack;
  CHAR8 *sata;
  UINT32 FakeID;
  UINT32 FakeVen;
  
  if (gSettings.FakeSATA) {
    FakeID = gSettings.FakeSATA >> 16;
    FakeVen = gSettings.FakeSATA & 0xFFFF;
  }


  if (!SATAAHCIADR1) return len;
  
  for (i=0x20; i<len-10; i++) {    
    if (CmpAdr(dsdt, i, SATAAHCIADR1)) {
           DBG("Found SATAAHCIADR1=%x at %x\n", SATAAHCIADR1, i);
      SATAAHCIADR = devFind(dsdt, i);
      if (!SATAAHCIADR) {
        continue;
      }

      BridgeSize = get_size(dsdt, SATAAHCIADR);
      if (BridgeSize) break;
    } 
  }
  if (!BridgeSize) return len;
  
  if (SATAAHCIADR) { // bridge or device
    i = SATAAHCIADR;
    Size = get_size(dsdt, i);
    k = FindMethod(dsdt + i, Size, "_DSM");
    if (k != 0) {
      DBG("_DSM already exists, patch SATA will not be applied\n");
      return len;
    }
  }  
  
  DBG("Start SATA AHCI Fix\n");
  
	root = aml_create_node(NULL);
	
	// add Method(_DSM,4,NotSerialized) 
  met = aml_add_method(root, "_DSM", 4);
  met = aml_add_store(met);
  pack = aml_add_package(met);
  if (gSettings.FakeSATA) {
    aml_add_string(pack, "device-id");
    aml_add_byte_buffer(pack, (CHAR8*)&FakeID, 4);
    aml_add_string(pack, "vendor-id");
    aml_add_byte_buffer(pack, (CHAR8*)&FakeVen, 4);
  }
  aml_add_local0(met);
  aml_add_buffer(met, dtgp_1, sizeof(dtgp_1));
  // finish Method(_DSM,4,NotSerialized)
  
  aml_calculate_size(root);  
  sata = AllocateZeroPool(root->Size); 
  sizeoffset = root->Size;
  aml_write_node(root, sata, 0);  
  aml_destroy_node(root);
    // move data to back for add DSM
  i = SATAAHCIADR + BridgeSize;
  len = move_data(i, dsdt, len, sizeoffset);
  CopyMem(dsdt+i, sata, sizeoffset);
    // Fix Device SATA size
  k = write_size(SATAAHCIADR, dsdt, len, sizeoffset);   
  sizeoffset += k;
  len += k;
  len = CorrectOuters(dsdt, len, SATAAHCIADR-3, sizeoffset);
  FreePool(sata);
  return len;
}

CHAR8 DevSATA0[] = {0x80, 0x26, 0x00, 0x00};

UINT32 FIXSATA (UINT8 *dsdt, UINT32 len)
{
  UINT32  i, k;
  UINT32 SATAADR = 0, BridgeSize = 0, Size;
  INT32 sizeoffset;
  AML_CHUNK* root;
  AML_CHUNK* met;
  AML_CHUNK* pack;
  CHAR8 *sata;
  UINT32 FakeID;
  UINT32 FakeVen;
  
  if (gSettings.FakeSATA) {
    FakeID = gSettings.FakeSATA >> 16;
    FakeVen = gSettings.FakeSATA & 0xFFFF;
  }
  
  if (!SATAADR1) return len;
  
  for (i=0x20; i<len-10; i++) {
    if (CmpAdr(dsdt, i, SATAADR1)) {
      //        DBG("Found SATAAHCIADR1=%x at %x\n", SATAAHCIADR1, j);
      SATAADR = devFind(dsdt, i);
      if (!SATAADR) {
        continue;
      }
      
      BridgeSize = get_size(dsdt, SATAADR);
      if (BridgeSize) break;
    } // End IDE
  }
  if (!BridgeSize) return len;
  if (SATAADR) { // bridge or device
    i = SATAADR;
    Size = get_size(dsdt, i);
    k = FindMethod(dsdt + i, Size, "_DSM");
    if (k != 0) {
      DBG("_DSM already exists, patch SATA will not be applied\n");
      return len;
    }
  }  
  
  DBG("Start SATA Fix\n");
  
	root = aml_create_node(NULL);
	// add Method(_DSM,4,NotSerialized)
  met = aml_add_method(root, "_DSM", 4);
  met = aml_add_store(met);
  pack = aml_add_package(met);
  if (gSettings.FakeSATA) {
    aml_add_string(pack, "device-id");
    aml_add_byte_buffer(pack, (CHAR8*)&FakeID, 4);
    aml_add_string(pack, "vendor-id");
    aml_add_byte_buffer(pack, (CHAR8*)&FakeVen, 4);
  }
  aml_add_local0(met);
  aml_add_buffer(met, dtgp_1, sizeof(dtgp_1));
  // finish Method(_DSM,4,NotSerialized)
  
  aml_calculate_size(root);
  sata = AllocateZeroPool(root->Size);
  sizeoffset = root->Size;
  aml_write_node(root, sata, 0);
  aml_destroy_node(root);
  // move data to back for add DSM
  i = SATAADR + BridgeSize;
  len = move_data(i, dsdt, len, sizeoffset);
  CopyMem(dsdt+i, sata, sizeoffset);
  // Fix Device SATA size
  k = write_size(SATAADR, dsdt, len, sizeoffset);
  sizeoffset += k;
  len += k;
  
  len = CorrectOuters(dsdt, len, SATAADR-3, sizeoffset);
  FreePool(sata);
  return len;
}


UINT32 FIXCPU1 (UINT8 *dsdt, UINT32 len)
{    
  UINT32 i, j;
  UINT32 count=0;
  UINT32 pradr=0;
  UINT32 prsize=0, size=0;
  UINT32 prsize1=0;
  INT32 offset, sizeoffset;
  
  DBG("Start CPUS=1 Fix\n");
	DBG("len = 0x%08x\n", len);
  
  // find _PR_ and get PR size
  for (i=0x20; i<len-4; i++)
  {
    if (dsdt[i] == '_' && dsdt[i+1] == 'P' && dsdt[i+2] == 'R' && dsdt[i+3] == '_')
    {
      DBG("Found _PR_\n");
      for (j=0; j<10; j++)
      {
        if (dsdt[i-j] == 0x10)
        {
          prsize = get_size(dsdt, i-j+1);
          if(!prsize) continue;
          prsize1 = prsize;
          pradr = i-j+1;
          // size > 0x3F there should be had P_states code so don't fix
          if (prsize > 0x3F) return len;
          DBG("_PR_ adr = 0x%08x size = 0x%08x\n", pradr, prsize);
          break;
        }
      }
      break;
    }
  }
  
  sizeoffset = 9;
  
  // find alias
  for (i=pradr; i<prsize1; i++)
  {
    if (dsdt[i] == 0x5B && dsdt[i+1] == 0x83)
    {
      size = get_size(dsdt, i+2);
      DBG("OP size = 0x%08x\n", size);
      // if OP name not CPUX.... need add alias in OP back 
      offset = i + 3 + (dsdt[i+2] >> 6); 
      if (dsdt[offset] == '\\') offset = i + 8 + (dsdt[i+7] >> 6); ; 
      if (dsdt[i+2+size] != 0x06 && dsdt[offset] != 'C' && dsdt[offset+1] != 'P' && dsdt[offset+2] != 'U')
      {
        DBG("Found alias CPU.\n");
        len = move_data(i+2+size, dsdt, len, sizeoffset);
        dsdt[i+2+size] = 0x06;
        dsdt[i+3+size] = dsdt[i+2];
        dsdt[i+4+size] = dsdt[i+3];
        dsdt[i+5+size] = dsdt[i+4];
        dsdt[i+6+size] = dsdt[i+5];
        dsdt[i+7+size] = 'C';
        dsdt[i+8+size] = 'P';
        dsdt[i+9+size] = 'U';
        dsdt[i+10+size] = dsdt[i+5];
        j = write_size(pradr, dsdt, len, sizeoffset);
        sizeoffset += j;
        len += j;
        count++;
        continue;
      }
    }
  }
  DBG("return len=%x\n", len);
  return len;             
}

UINT32 FIXWAK (UINT8 *dsdt, UINT32 len)
{    
  UINT32 i, j, k;
  UINT32 wakadr=0;
  UINT32 waksize=0;
  UINT32 sizeoffset = 0;
  
  DBG("Start _WAK Return Fix\n");
  DBG("len = 0x%08x\n", len);
	
	for (i=20; i<len-5; i++) 
	{ 	
		if(dsdt[i] == '_' && dsdt[i+1] == 'W' && dsdt[i+2] == 'A' && dsdt[i+3] == 'K')
		{
      for (j=0; j<10; j++) 
      {
        if(dsdt[i-j] == 0x14) 
        {
          waksize = get_size(dsdt, i-j+1);
          wakadr = i-j+1;
          //DBG( "_WAK adr = 0x%08x, size = 0x%08x\n", wakadr, waksize);
          for (k=0; k<waksize; k++) 
          {
            if (dsdt[i+k] == 0xA4) 
            {
              DBG( "_WAK Method find return data, don't need to patch.\n");
              return len;
            }
          }
          DBG( "_WAK Method need return data, will patch it.\n");
          sizeoffset = sizeof(wakret);
          len = move_data(wakadr+waksize, dsdt, len, sizeoffset);
          CopyMem(dsdt+wakadr+waksize, wakret, sizeoffset);
          k = write_size(wakadr, dsdt, len, sizeoffset);
          sizeoffset += k;
          len += k;
          len = CorrectOuters(dsdt, len, wakadr-2, sizeoffset);
          break;
        }
      }
      break;    
		}
	}	    
  DBG("return len=%x\n", len);
  return len;             
}

UINT32 FIXGPE (UINT8 *dsdt, UINT32 len)
{    
  UINT32 i, j, k, l, m, n;
  UINT32 gpeadr=0;
  UINT32 gpesize=0;
  UINT32 pwrbadr=0;
  UINT32 pwrbsize=0;
  UINT32 usbcount=0;
  UINT32 PWRBADR = 0;
//  UINT32 adr=0;
  INT32  offset=0;
  INT32 sizeoffset;
//  BOOLEAN pwrbfix = FALSE;
  BOOLEAN usbpwrb = FALSE;
//  BOOLEAN foundpwrb = FALSE;

  sizeoffset = sizeof(pwrb);
  if (!PWRBADR) {
    return len;
  }
    
  DBG("Start _GPE device remove error Fix\n");
	//DBG("len = 0x%08x\n", len);
  
	for (i=0; i<len-10; i++) 
	{ 	//какой же здесь бред!
		if(dsdt[i] == '_' && dsdt[i+1] == 'L' && (dsdt[i-2] == 0x14 || dsdt[i-3] == 0x14 || dsdt[i-4] == 0x14 || dsdt[i-5] == 0x14))
		{
      for (j=0; j<10; j++) 
      {
        if(dsdt[i-j] == 0x14) 
        {
          pwrbsize = get_size(dsdt, i-j+1);
          pwrbadr = i-j+1;
          //DBG( "_LXX adr = 0x%08x, size = 0x%08x\n", pwrbadr, pwrbsize);
          for (k=pwrbadr; k<pwrbadr+pwrbsize; k++) 
          {
            for (l=0; l<usb; l++)
            {   // find USB _LXX
              if (dsdt[k] == UsbName[l][0] && dsdt[k+1] == UsbName[l][1] && 
                  dsdt[k+2] == UsbName[l][2] && dsdt[k+3] == UsbName[l][3]) 
              {
                //DBG( "found USB _GPE Method.\n");
                usbpwrb = TRUE;
                
                if (!usbcount)
                {
                  //DBG( "will to find Scope(\\_GPE).\n");
                  for (m=0; m<300; m++)
                  {
                    if (dsdt[i-m] == 'E' && dsdt[i-m-1] == 'P' && dsdt[i-m-2] == 'G' && dsdt[i-m-3] == '_')
                    {
                      for (n=0; n<15; n++)
                      {
                        if (dsdt[i-m-n] == 0x10)
                        {
                          gpeadr = i-m-n+1;
                          gpesize = get_size(dsdt, i-m-n+1);
                          //DBG( "_GPE adr = 0x%08x, size = 0x%08x\n", gpeadr, gpesize);
                          break;
                        }
                      }
                      break; 
                    }
                  }
                }
                break;
              }
            }
          }
        }
      }
		}
	}	
	
	if (usbcount)
	{
    sizeoffset = offset;
    k = write_size(gpeadr, dsdt, len, sizeoffset);
    sizeoffset += k;
    len += k;
    len = CorrectOuters(dsdt, len, gpeadr-3, sizeoffset);
  }
  
  return len;             
}

UINT32 FIXPWRB (UINT8* dsdt, UINT32 len)
{
  UINT32 i, j=0;
  UINT32 adr=0, hid=0, size = 0;
  CHAR8 Name[4];
  INT32 sizeoffset;
  //search  PWRB PNP0C0C
  for (i=0x20; i<len-6; i++) {
    if (CmpPNP(dsdt, i, 0x0C0C)) {
      adr = devFind(dsdt, i);
      if (!adr) {
        continue;
      }
      size = get_size(dsdt, adr);
      if(size){
        hid = i + 10; //the place after HID PNP0C0C
        break;
      }
    }
  }
  
  if (size) {
    //check name and replace
    if (size < 0x40) {
      j = adr + 1;
    } else {
      j = adr + 2;
    }
    for (i=0; i<4; i++) {
      Name[i] = dsdt[j+i];
    } 
    ReplaceName(dsdt, len, Name, "PWRB");        
    sizeoffset = sizeof(pwrbprw);
    len = move_data(hid, dsdt, len, sizeoffset);
    CopyMem(dsdt+hid, pwrbprw, sizeoffset);
    i = write_size(adr, dsdt, len, sizeoffset);
    sizeoffset += i;
    len += i;
    len = CorrectOuters(dsdt, len, adr-3, sizeoffset);  
  }

  return len;  
}

UINT32 FIXSHUTDOWN_ASUS (UINT8 *dsdt, UINT32 len)
{
	UINT32 i, j, sizeoffset;
	UINT32 adr, adr1 = 0, adr2, size, shift = 0;
	
	DBG("Start SHUTDOWN Fix len=%x\n", len);
  adr = FindMethod(dsdt, len, "_PTS");
  if (!adr) {
    DBG("no _PTS???\n");
    return len;
  }
  //one more patch possible here
  /*
   70 53 4D 49 4D 5C 2E 5F 53 42 5F 50 49 4E 58 5C
   5C 2E 5F 53 42 5F 49 53 4D 49 0A 90 
   Store (SMIM, \_SB.PINX)
   \_SB.ISMI (0x90)
   */
/*  size = get_size(dsdt, adr);
  adr1 = FindBin(dsdt, len, Ismi, 28);
  if (adr1 < adr + size) {
    len = move_data(adr1, dsdt, len, -28);
  } */
  
  /*
      adr \  _  P  T  S       insert               offset
   14 16 5C 5F 50 54 53 01  < A0 05 93 68 0A 05 A1 08	>
   body
   53 4D 49 5F 0A 8A 68 

   */
  sizeoffset = sizeof(shutdown); // == 8
  size = get_size(dsdt, adr);
  adr1 = adr;
  for (j=0; j<20; j++) {
    if ((dsdt[adr+j] == 'T') && (dsdt[adr+j+1] == 'S')) {
      adr1 = adr+j+3; //address of body
      break;
    }
  }
  adr2 = adr1 + sizeoffset - 1;  //jump adr
//  DBG("adr=%x adr1=%x size=%x sizeoffset=%x\n", adr, adr1, size, sizeoffset);
  len = move_data(adr1, dsdt, len, sizeoffset);  //new len
  CopyMem(dsdt+adr1, shutdown, sizeoffset);  //insert shutdown
  i = adr + size - adr1; //body size
  shift = write_offset(adr2, dsdt, len, i); //may return 0 or 1
  len += shift;
  sizeoffset += shift;
//  DBG("body=%x size=%x len=%x\n", i, size, len);
  shift = write_size(adr, dsdt, len, sizeoffset);
  sizeoffset += shift;
  len += shift;
//  DBG("shift=%x sizeoffset=%x len=%x\n", shift, sizeoffset, len);
  return len;  
}

//Slice - this procedure was not corrected and mostly wrong
UINT32 FIXOTHER (UINT8 *dsdt, UINT32 len)
{
	UINT32 i, j, k, m, offset, l;
	UINT32 size;
	
	// Fix USB _PRW value for 0x0X, 0x04 ==> 0x0X, 0x01
	for(j=0; j<usb; j++) {
    for (i=0; i<len-5; i++) { 
      if (CmpAdr(dsdt, i, USBADR[j])) {
          // get USB name
          UsbName[j] = AllocateZeroPool(5);
          CopyMem(UsbName[j], dsdt+i, 4);
          DBG("found USB device NAME(_ADR,0x%08x) And Name is %a\n", 
              USBADR[j], UsbName[j]);
          
          for (k=i+1; k<i+200; k++) {
            if (dsdt[k] == 0x14 && dsdt[k+2] == '_' && dsdt[k+3] == 'P' && dsdt[k+4] == 'R' && dsdt[k+5] == 'W') {
              offset = k;
              m = dsdt[k+1];
              if (dsdt[offset+m] != 0x03) {
                if (dsdt[offset+m] == 0x01)
                  dsdt[offset+m] = 0x03;
		            
                if (dsdt[offset+m] == 0x04)
                  dsdt[offset+m] = 0x01;
                
                //DBG("found USB Method(_PRW) and will patch fix\n");
              }
              break;
            }
          for (k=i+1; k<i+200; k++) {
            if (dsdt[k] == 0x14 && dsdt[k+2] == '_' && dsdt[k+3] == 'S' && dsdt[k+4] == '3' && dsdt[k+5] == 'D') {
              size = dsdt[k+1];
              for (l=0; l<size; l++) {
                if (dsdt[k+1+l] == 0xA4 && dsdt[k+2+l] == 0x0A && dsdt[k+3+l] != 0x03) {
                  dsdt[k+3+l] = 0x03;
                }
              }
              break;
            }
          } 
          //for (k=i+1; k<i+200; k++) {
          //    if (dsdt[k] == 0x14 && dsdt[k+2] == '_' && dsdt[k+3] == 'P' && dsdt[k+4] == 'S' && dsdt[k+5] == 'W') 
          //    {
          //        size = dsdt[k+1];
          //        for (l=0; l<size; l++)
          //        {
          //            if (dsdt[k+1+l] == 0x70 && dsdt[k+2+l] == 0x00) 
          //            {
          //                dsdt[k+2+l] = 0x03;
          //            }
          //        }
          //        break;
          //    }
          //}
          break;
        }
      }
    }
	}    
	/*
   // chage USB name to UHC1..... for Mac name
   for (j=0; j<usb; j++)
   {
   for (i=0; i<len-5; i++)
   {
   if (dsdt[i] == UsbName[j][0] && dsdt[i+1] == UsbName[j][1] && dsdt[i+2] == UsbName[j][2] && dsdt[i+3] == UsbName[j][3])
   {
   if (USBADR[j] == 0x001D0000)
   {
   dsdt[i] = 'U';
   dsdt[i+1] = 'H';
   dsdt[i+2] = 'C';
   dsdt[i+3] = '1';
   }
   if (USBADR[j] == 0x001D0001)
   {
   dsdt[i] = 'U';
   dsdt[i+1] = 'H';
   dsdt[i+2] = 'C';
   dsdt[i+3] = '2';
   }
   if (USBADR[j] == 0x001D0002)
   {
   dsdt[i] = 'U';
   dsdt[i+1] = 'H';
   dsdt[i+2] = 'C';
   dsdt[i+3] = '3';
   }	
   if (USBADR[j] == 0x001D0003)
   {
   dsdt[i] = 'U';
   dsdt[i+1] = 'H';
   dsdt[i+2] = 'C';
   dsdt[i+3] = '6';
   }            
   if (USBADR[j] == 0x001A0000)
   {
   dsdt[i] = 'U';
   dsdt[i+1] = 'H';
   dsdt[i+2] = 'C';
   dsdt[i+3] = '4';
   }	            
   if (USBADR[j] == 0x001A0001)
   {
   dsdt[i] = 'U';
   dsdt[i+1] = 'H';
   dsdt[i+2] = 'C';
   dsdt[i+3] = '5';
   }	            
   if (USBADR[j] == 0x001D0007)
   {
   dsdt[i] = 'E';
   dsdt[i+1] = 'H';
   dsdt[i+2] = 'C';
   dsdt[i+3] = '1';
   }	            
   if (USBADR[j] == 0x001A0007)
   {
   dsdt[i] = 'E';
   dsdt[i+1] = 'H';
   dsdt[i+2] = 'C';
   dsdt[i+3] = '2';
   }	            
   }
   }
   }            
   */            
  for (j=0; j<usb; j++){
    FreePool(UsbName[j]);
  }
    
	// fix _T_0 _T_1 _T_2 _T_3
	for (i=0; i<len-10; i++) { 
		if (dsdt[i] == '_' && dsdt[i+1] == 'T' && dsdt[i+2] == '_' &&
        (dsdt[i+3] == '0' || dsdt[i+3] == '1' || dsdt[i+3] == '2' || dsdt[i+3] == '3')) {
      dsdt[i] = dsdt[i+1];
      dsdt[i+1] = dsdt[i+2];
      dsdt[i+2] = dsdt[i+3];
      dsdt[i+3] = '_';
		}
	}
	
	// fix MUTE Possible operator timeout is ignored
	for (i=0; i<len-10; i++) { 
		if (dsdt[i] == 0x5B && dsdt[i+1] == 0x23 && dsdt[i+2] == 0x4D &&
		    dsdt[i+3] == 0x55 && dsdt[i+4] == 0x54 && dsdt[i+5] == 0x45) {
      dsdt[i+6] = 0xFF;
      dsdt[i+7] = 0xFF;
		}
	}
  
  return len;
  
}

VOID FixBiosDsdt (UINT8* temp)
{    
  UINT32 DsdtLen;
  if (!temp)
    return;
  
  //Reenter requires ZERO values

  HDAFIX = TRUE;
  GFXHDAFIX = TRUE;
  USBIDFIX = TRUE;  
  
  DsdtLen = ((EFI_ACPI_DESCRIPTION_HEADER*)temp)->Length;
  if ((DsdtLen < 20) || (DsdtLen > 400000)) { //fool proof (some ASUS dsdt > 300kb?)
    DBG("DSDT length out of range\n");
    return;
  }
  
  DBG("\nAuto patch DSDT Starting.................\n\n");
  
  // First check hardware address: GetPciADR(DevicePath, &NetworkADR1, &NetworkADR2);
  CheckHardware();

  //arbitrary fixes
  if (gSettings.PatchDsdtNum > 0) {
    UINTN i;
    for (i = 0; i < gSettings.PatchDsdtNum; i++) {
      DsdtLen = FixAny(temp, DsdtLen,
                       gSettings.PatchDsdtFind[i], gSettings.LenToFind[i],
                       gSettings.PatchDsdtReplace[i], gSettings.LenToReplace[i]);
    }
  }
  
  // find ACPI CPU name and hardware address
  findCPU(temp, DsdtLen);
  
  // add Method (DTGP, 5, NotSerialized)
  if ((gSettings.FixDsdt & FIX_DTGP)) {
    if (!FindMethod(temp, DsdtLen, "DTGP")) {
      CopyMem((CHAR8 *)temp+DsdtLen, dtgp, sizeof(dtgp));
      DsdtLen += sizeof(dtgp);
      ((EFI_ACPI_DESCRIPTION_HEADER*)temp)->Length = DsdtLen;      
    }
  }
  
  // get PCIRootUID and all DSDT Fix address : NetworkADR = devFind(dsdt, k);
  // ReplaceName(dsdt, len, device_name[1], "GIGE");??
  // move Rename into dedicated places
  gSettings.PCIRootUID = (UINT16)findPciRoot(temp, DsdtLen);
  
  // Fix RTC
  if ((gSettings.FixDsdt & FIX_HPET) != 0) {
    DBG("patch RTC in DSDT \n");
    DsdtLen = FixRTC(temp, DsdtLen);
  }
  
  // Fix TMR
  if ((gSettings.FixDsdt & FIX_HPET) != 0) {
    DBG("patch TMR in DSDT \n");
    DsdtLen = FixTMR(temp, DsdtLen);
  }
  
  // Fix PIC or IPIC
  if ((gSettings.FixDsdt & FIX_IPIC) != 0) {
    DBG("patch IPIC in DSDT \n");
    DsdtLen = FixPIC(temp, DsdtLen);
  }
  
  // Fix HPET
  if ((gSettings.FixDsdt & FIX_HPET) != 0) {
    DBG("patch HPET in DSDT \n");
    DsdtLen = FixHPET(temp, DsdtLen);
  }
  
  // Fix LPC if don't had HPET don't need to inject LPC??
  if (LPCBFIX && (gCPUStructure.Family == 0x06)  && (gSettings.FixDsdt & FIX_LPC)) {
    DBG("patch LPC in DSDT \n");
    DsdtLen = FIXLPCB(temp, DsdtLen);
  }
  
  // Fix Display
  if (gSettings.FixDsdt & FIX_DISPLAY) {
    if (DisplayADR1[0]) {
//      DBG("patch Display0 in DSDT \n");
      DsdtLen = FIXDisplay1(temp, DsdtLen);
    }
    
    if (DisplayADR1[1]) {
//      DBG("patch Display1 in DSDT \n");
      DsdtLen = FIXDisplay2(temp, DsdtLen);
    }    
  }
  
  // Fix Network
  if (NetworkADR1 && (gSettings.FixDsdt & FIX_LAN)) {
//    DBG("patch LAN in DSDT \n");
    DsdtLen = FIXNetwork(temp, DsdtLen);
  }

  // Fix Airport
  if (ArptADR1 && (gSettings.FixDsdt & FIX_WIFI)) {
//    DBG("patch Airport in DSDT \n");
    DsdtLen = FIXAirport(temp, DsdtLen);
  }
  
  // Fix SBUS
  if (SBUSADR1  && (gSettings.FixDsdt & FIX_SBUS)) {
//    DBG("patch SBUS in DSDT \n");
    DsdtLen = FIXSBUS(temp, DsdtLen);
  }
  
  // Fix IDE inject
  if (IDEFIX && (IDEVENDOR == 0x8086 || IDEVENDOR == 0x11ab)  && (gSettings.FixDsdt & FIX_IDE)) {
//    DBG("patch IDE in DSDT \n");
    DsdtLen = FIXIDE(temp, DsdtLen);
  }
  
  // Fix SATA AHCI orange icon
  if (SATAAHCIADR1 && (SATAAHCIVENDOR == 0x8086)  && (gSettings.FixDsdt & FIX_SATA)) {
    DBG("patch AHCI in DSDT \n");
    DsdtLen = FIXSATAAHCI(temp, DsdtLen);
  }
  
  // Fix SATA inject
  if (SATAFIX && (SATAVENDOR == 0x8086)  && (gSettings.FixDsdt & FIX_SATA)) {
    DBG("patch SATA in DSDT \n");
    DsdtLen = FIXSATA(temp, DsdtLen);
  }
  
  // Fix Firewire
  if (FirewireADR1  && (gSettings.FixDsdt & FIX_FIREWIRE)) {
    DBG("patch FRWR in DSDT \n");
    DsdtLen = FIXFirewire(temp, DsdtLen);
  }
  
  // HDA HDEF
  if (HDAFIX  && (gSettings.FixDsdt & FIX_HDA)) {
    DBG("patch HDEF in DSDT \n");
    DsdtLen = AddHDEF(temp, DsdtLen);
  }
  
  //Always add MCHC for PM
  if ((gCPUStructure.Family == 0x06)  && (gSettings.FixDsdt & FIX_MCHC)) {
    DBG("patch MCHC in DSDT \n");
    DsdtLen = AddMCHC(temp, DsdtLen);
    DsdtLen = AddIMEI(temp, DsdtLen);
  }
  
  // Always Fix USB
  if ((gSettings.FixDsdt & FIX_USB)) {
    DBG("patch USB in DSDT \n");
    DsdtLen = FIXUSB(temp, DsdtLen);
  }
  
  if ((gSettings.FixDsdt & FIX_WARNING)) {
    DBG("patch warnings \n");
    // Always Fix alias cpu FIX cpus=1
  //  DsdtLen = FIXCPU1(temp, DsdtLen);
  //  DsdtLen = FIXPWRB(temp, DsdtLen);
    
    // Always Fix _WAK Return value
    DsdtLen = FIXWAK(temp, DsdtLen);
    
    // USB Device remove error Fix
   // DsdtLen = FIXGPE(temp, DsdtLen);
    
    //I want these fixes even if no Display fix. We have GraphicsInjector
    DsdtLen = DeleteDevice("CRT_", temp, DsdtLen);  
    DsdtLen = DeleteDevice("DVI_", temp, DsdtLen);
    //good company
    DsdtLen = DeleteDevice("SPKR", temp, DsdtLen);
    DsdtLen = DeleteDevice("ECP_", temp, DsdtLen);
    DsdtLen = DeleteDevice("LPT_", temp, DsdtLen);
    DsdtLen = DeleteDevice("FDC0", temp, DsdtLen);
    DsdtLen = DeleteDevice("ECP1", temp, DsdtLen);
    DsdtLen = DeleteDevice("LPT1", temp, DsdtLen);
    
    ReplaceName(temp, DsdtLen, "ACST", "OCST");
    ReplaceName(temp, DsdtLen, "ACSS", "OCSS");
    ReplaceName(temp, DsdtLen, "APSS", "OPSS");
    ReplaceName(temp, DsdtLen, "APSN", "OPSN");
    ReplaceName(temp, DsdtLen, "APLF", "OPLF");
    
    if (gMobile) {
      DsdtLen = AddPNLF(temp, DsdtLen);
    }
    FixS3D(temp, DsdtLen);
     // pwrb add _CID sleep button fix
    DsdtLen = FixADP1(temp, DsdtLen); 
    // other compiler warning fix _T_X,  MUTE .... USB _PRW value form 0x04 => 0x01
//     DsdtLen = FIXOTHER(temp, DsdtLen);
    if (!FindMethod(temp, DsdtLen, "GET9") && 
        !FindMethod(temp, DsdtLen, "STR9") &&
        !FindMethod(temp, DsdtLen, "OOSI")) {
      DsdtLen = FIXDarwin(temp, DsdtLen);
    }
  } 
  // Fix SHUTDOWN For ASUS
  if ((gSettings.FixDsdt & FIX_SHUTDOWN)) {
    DsdtLen = FIXSHUTDOWN_ASUS(temp, DsdtLen); //safe to do twice
  }
  
  // Finish DSDT patch and resize DSDT Length
  temp[4] = (DsdtLen & 0x000000FF);
  temp[5] = (UINT8)((DsdtLen & 0x0000FF00) >>  8);
  temp[6] = (UINT8)((DsdtLen & 0x00FF0000) >> 16);
  temp[7] = (UINT8)((DsdtLen & 0xFF000000) >> 24);
  
  CopyMem((UINT8*)((EFI_ACPI_DESCRIPTION_HEADER*)temp)->OemId, (UINT8*)BiosVendor, 6);
  //DBG("orgBiosDsdtLen = 0x%08x\n", orgBiosDsdtLen);
  ((EFI_ACPI_DESCRIPTION_HEADER*)temp)->Checksum = 0;
  ((EFI_ACPI_DESCRIPTION_HEADER*)temp)->Checksum = (UINT8)(256-Checksum8(temp, DsdtLen));
    
  DBG("\nAuto patch BiosDSDT Finish.................\n\n");
  //PauseForKey(L"waiting for key press...\n");
}
