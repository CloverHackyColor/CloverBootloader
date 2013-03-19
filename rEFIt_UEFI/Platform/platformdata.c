/**
 platformdata.c
 **/

#include "Platform.h"
#include "nvidia.h"

/* Machine Default Data */

CHAR8*	DefaultMemEntry = "N/A";

CHAR8*	DefaultSerial = "CT288GT9VT6";

CHAR8* BiosVendor = "Apple Inc.";

CHAR8* AppleManufacturer = "Apple Computer, Inc."; //Old name, before 2007

UINT32 gFwFeatures = 0xE001f537; //default values for iMac13,1

CHAR8* AppleFirmwareVersion[] = 
{
	"MB11.88Z.0061.B03.0809221748",
	"MB21.88Z.00A5.B07.0706270922",
	"MB41.88Z.0073.B00.0809221748",
	"MB52.88Z.0088.B05.0809221748",
	"MBP51.88Z.007E.B06.0906151647",
  "MBP81.88Z.0047.B27.1102071707",
  "MBP83.88Z.0047.B24.1110261426",
  "MBP91.88Z.00D3.B08.1205101904",
	"MBA31.88Z.0061.B07.0712201139",
  "MBA51.88Z.00EF.B01.1205221442",
	"MM21.88Z.009A.B00.0706281359",
  "MM51.88Z.0077.B10.1102291410",
  "MM61.88Z.0106.B00.1208091121",
	"IM81.88Z.00C1.B00.0803051705",
	"IM101.88Z.00CC.B00.0909031926",
  "IM111.88Z.0034.B02.1003171314",
	"IM112.88Z.0057.B01.0802091538",
  "IM113.88Z.0057.B01.1005031455",
	"IM121.88Z.0047.B1F.1201241648",
  "IM122.88Z.0047.B1F.1223021110",
  "IM131.88Z.010A.B05.1209042338",
	"MP31.88Z.006C.B05.0802291410",
	"MP41.88Z.0081.B04.0903051113",
	"MP51.88Z.007F.B00.1008031144"
};

CHAR8* AppleBoardID[] = //Lion DR1 compatible
{
	"Mac-F4208CC8",  //MB11 - yonah
	"Mac-F4208CA9",  //MB21 - merom 05/07
	"Mac-F22788A9",  //MB41 - penryn
	"Mac-F22788AA",  //MB52
	"Mac-F42D86C8",  //MBP51
  "Mac-94245B3640C91C81",  //MBP81 - i5 SB IntelHD3000
  "Mac-942459F5819B171B",  //MBP83 - i7 SB  ATI
  "Mac-6F01561E16C75D06",  //MBP92 - i5-3210M IvyBridge HD4000
	"Mac-942452F5819B1C1B",  //MBA31
  "Mac-2E6FAB96566FE58C",  //MBA52 - i5-3427U IVY BRIDGE IntelHD4000 did=166
	"Mac-F4208EAA",          //MM21 - merom GMA950 07/07
  "Mac-8ED6AF5B48C039E1",  //MM51 - Sandy + Intel 30000
  "Mac-F65AE981FFA204ED",  //MM62 - Ivy
	"Mac-F227BEC8",  //IM81 - merom 01/09
	"Mac-F2268CC8",  //IM101 - wolfdale? E7600 01/
  "Mac-F2268DAE",  //IM111 - Nehalem
	"Mac-F2238AC8",  //IM112 - Clarkdale
  "Mac-F2238BAE",  //IM113 - lynnfield
  "Mac-942B5BF58194151B",  //IM121 - i5-2500 - sandy
  "Mac-942B59F58194171B",  //IM122 - i7-2600
  "Mac-00BE6ED71E35EB86",  //IM131 - -i5-3470S -IVY
	"Mac-F2268DC8",  //MP31 - xeon quad 02/09 conroe
	"Mac-F4238CC8",  //MP41 - xeon wolfdale
	"Mac-F221BEC8"   //MP51 - Xeon Nehalem 4 cores
};

CHAR8* AppleReleaseDate[] = 
{
	"09/22/08",  //mb11
	"06/27/07",
	"09/22/08",
	"01/21/09",
	"06/15/09",  //mbp51
  "02/07/11",
  "10/26/11",
  "05/10/2012", //MBP92
	"12/20/07",
  "05/22/2012", //mba52
	"08/07/07",  //mm21
  "02/29/11",  //MM51
  "08/09/2012", //MM62
	"03/05/08",
	"09/03/09",  //im101
	"03/17/10",
  "03/17/10",  //11,2
	"05/03/10",
  "01/24/12",  //121 120124
  "02/23/12",  //122
  "09/04/2012",  //131
	"02/29/08",
	"03/05/09",
	"08/03/10"
};

CHAR8* AppleProductName[] = 
{
	"MacBook1,1",
	"MacBook2,1",
	"MacBook4,1",
	"MacBook5,2",
	"MacBookPro5,1",
  "MacBookPro8,1",
  "MacBookPro8,3",
  "MacBookPro9,2",
	"MacBookAir3,1",
  "MacBookAir5,2",
	"Macmini2,1",
  "Macmini5,1",
  "Macmini6,2",
	"iMac8,1",
	"iMac10,1",
  "iMac11,1",
	"iMac11,2",
  "iMac11,3",
	"iMac12,1",
  "iMac12,2",
  "iMac13,1",
	"MacPro3,1",
	"MacPro4,1",
	"MacPro5,1"
};

CHAR8* AppleFamilies[] = 
{
	"MacBook",
	"MacBook",
	"MacBook",
	"MacBook",
	"MacBookPro",
  "MacBookPro",
  "MacBookPro",
  "MacBook Pro",
	"MacBookAir",
  "MacBook Air",
	"Macmini",
  "Mac mini",
  "Macmini",
	"iMac",
	"iMac",
	"iMac",
	"iMac",
	"iMac",
	"iMac",
  "iMac",
  "iMac",
	"MacPro",
	"MacPro",
	"MacPro"
};


CHAR8* AppleSystemVersion[] = 
{
	"1.1",
	"1.2",
	"1.3",
	"1.3",
	"1.0",
	"1.0",
  "1.0",
  "1.0",
	"1.0",
  "1.0",
  "1.1",
  "1.0", //MM51
  "1.0",
	"1.3",
	"1.0",
  "1.0",
	"1.2",
  "1.0",
	"1.9",
  "1.9",
  "1.0",
	"1.3",
	"1.4",
	"1.2"
};

CHAR8* AppleSerialNumber[] = //random generated
{
	"W80A041AU9B",  //MB11
	"W88A041AWGP",  //MB21 - merom 05/07
	"W88A041A0P0",  //MB41
	"W88AAAAA9GU",  //MB52
	"W88439FE1G0",  //MBP51
  "W89F9196DH2G", //MBP81 - i5 SB IntelHD3000
  "W88F9CDEDF93", //MBP83 -i7 SB  ATI
  "C02HA041DTY3", //MBP92 - i5 IvyBridge HD4000
	"W8649476DQX",  //MBA31
  "C02HA041DRVC", //MBA52 - IvyBridge
	"W88A56BYYL2",  //MM21 - merom GMA950 07/07
  "C07GA041DJD0", //MM51 - sandy
  "C07JD041DWYN", //MM62 - IVY
	"W89A00AAX88",  //IM81 - merom 01/09
	"W80AA98A5PE",  //IM101 - wolfdale? E7600 01/09
  "G8942B1V5PJ",  //IM111 - Nehalem
	"W8034342DB7",  //IM112 - Clarkdale
  "QP0312PBDNR",  //IM113 - lynnfield
	"W80CF65ADHJF", //IM121 - i5-2500 - sandy
  "W88GG136DHJQ", //IM122 -i7-2600
  "C02JA041DNCT", //IM131 -i5-3470S -IVY
	"W88A77AA5J4",  //MP31 - xeon quad 02/09
	"CT93051DK9Y",  //MP41
	"CG154TB9WU3"   //MP51 C07J50F7F4MC  CK04000AHFC
};
//no! ChassisVersion == BoardID
CHAR8* AppleChassisAsset[] = 
{
	"MacBook-White",
	"MacBook-White",
	"MacBook-Black",
	"MacBook-Black",
	"MacBook-Aluminum",
	"MacBook-Aluminum",
	"MacBook-Aluminum",
  "MacBook-Aluminum",
	"Air-Enclosure",
  "Air-Enclosure",
	"Mini-Aluminum",
  "Mini-Aluminum",
  "Mini-Aluminum",
	"iMac-Aluminum",
	"iMac-Aluminum",
	"iMac-Aluminum",
	"iMac-Aluminum",
	"iMac-Aluminum",
	"iMac-Aluminum",
  "iMac-Aluminum",
  "iMac-Aluminum",
	"Pro-Enclosure",
	"Pro-Enclosure",
	"Pro-Enclosure"
};
//TODO - find more information and correct all SMC arrays
CHAR8* SmcPlatform[] =
{
	"m70",  //"MacBook1,1",
	"m75",  //"MacBook2,1",
	"m82",  //"MacBook4,1",
	"m97",  //"MacBook5,2",
	"NA",   //"MacBookPro5,1",
  "k90i", //"MacBookPro8,1",
  "k92i", //"MacBookPro8,3",
  "j30",  //"MacBookPro9,2",
	"NA",  //"MacBookAir3,1",
  "NA",  //"MacBookAir5,2",
	"NA",  //"Macmini2,1",  //31 -> m88
  "NA",  //"Macmini5,1",
  "j50s", //"Macmini6,2",
	"NA",  //"iMac8,1",
	"NA",  //"iMac10,1",
  "NA",  //"iMac11,1",
	"k74",  //"iMac11,2",
  "NA",  //"iMac11,3",
	"k60", //"iMac12,1",
  "k62", //"iMac12,2",
  "d8",  //"iMac13,1",  <- iMac13,2
	"m86",  //"MacPro3,1",
	"NA",  //"MacPro4,1",
	"k5",  //"MacPro5,1"
};


UINT8 SmcRevision[][6] = {
  {0x01, 0x04, 0x0F, 0, 0, 0x12},  //"MacBook1,1",
	{0x01, 0x13, 0x0F, 0, 0, 0x03},  //"MacBook2,1",
	{0x01, 0x31, 0x0F, 0, 0, 0x01},  //"MacBook4,1",
	{0x01, 0x38, 0x0F, 0, 0, 0x05},  //"MacBook5,2",
	{0x01, 0x47, 0x0F, 0, 0, 0x02},  //"MacBookPro5,1",
  {0x01, 0x68, 0x0F, 0, 0, 0x99},  //"MacBookPro8,1",
  {0x01, 0x70, 0x0F, 0, 0, 0x05},  //"MacBookPro8,3",
  {0x02, 0x02, 0x0F, 0, 0, 0x41},  //"MacBookPro9,2",
	{0x01, 0x67, 0x0F, 0, 0, 0x09},  //"MacBookAir3,1",
  {0x02, 0x05, 0x0F, 0, 0, 0x07},  //"MacBookAir5,2",
	{0x01, 0x19, 0x0F, 0, 0, 0x02},  //"Macmini2,1", 
  {0x01, 0x30, 0x0F, 0, 0, 0x03},  //"Macmini5,1",
  {0x02, 0x08, 0x0F, 0, 0, 0x00},  //"Macmini6,2",
	{0x01, 0x29, 0x0F, 0, 0, 0x01},  //"iMac8,1",
	{0x01, 0x53, 0x0F, 0, 0, 0x13},  //"iMac10,1",
  {0x01, 0x54, 0x0F, 0, 0, 0x36},  //"iMac11,1",
	{0x01, 0x64, 0x0F, 0, 0, 0x05},  //"iMac11,2",
  {0x01, 0x30, 0x0F, 0, 0, 0x03},  //"iMac11,3",
	{0x01, 0x71, 0x0F, 0, 0, 0x22},  //"iMac12,1",
  {0x01, 0x72, 0x0F, 0, 0, 0x02},  //"iMac12,2",
  {0x02, 0x11, 0x0F, 0, 0, 0x14},  //"iMac13,1", <- iMac13,2
	{0x01, 0x25, 0x0F, 0, 0, 0x04},  //"MacPro3,1",
	{0x01, 0x39, 0x0F, 0, 0, 0x05},  //"MacPro4,1",
	{0x01, 0x39, 0x0F, 0, 0, 0x11},  //"MacPro5,1"

};


UINT32 SmcConfig[] =
{
	0x71001,  //"MacBook1,1",
	0x72001,  //"MacBook2,1",
	0x74001,  //"MacBook4,1",
	0x7a002,  //"MacBook5,2",
	0x7b002,  //"MacBookPro5,1",
  0x7b005,  //"MacBookPro8,1",
  0x7c005,  //"MacBookPro8,3",
  0x76006,  //"MacBookPro9,2",
	0x7a004,  //"MacBookAir3,1",
  0x7b006,  //"MacBookAir5,2",
	0x78002,  //"Macmini2,1",  //31 -> m88
  0x7d005,  //"Macmini5,1",
  0x7d006,  //"Macmini6,2",
	0x7b001,  //"iMac8,1",
	0x7b002,  //"iMac10,1",
  0x7b004,  //"iMac11,1",
	0x7c004,  //"iMac11,2",
  0x7d004,  //"iMac11,3",
	0x73005,  //"iMac12,1",
  0x75005,  //"iMac12,2",
  0x78006,  //"iMac13,1",  //79006 == iMac13,2
	0x79001,  //"MacPro3,1",
	0x7c002,  //"MacPro4,1",
	0x7c002,  //"MacPro5,1"
};


CHAR8* AppleBoardSN = "C02140302D5DMT31M";
CHAR8* AppleBoardLocation = "Part Component";

VOID SetDMISettingsForModel(MACHINE_TYPES Model)
{
  AsciiStrCpy(gSettings.VendorName,             BiosVendor);
  AsciiStrCpy(gSettings.RomVersion,             AppleFirmwareVersion[Model]);
  AsciiStrCpy(gSettings.ReleaseDate,            AppleReleaseDate[Model]);
  AsciiStrCpy(gSettings.ManufactureName,        BiosVendor);
  AsciiStrCpy(gSettings.ProductName,            AppleProductName[Model]);
  AsciiStrCpy(gSettings.VersionNr,              AppleSystemVersion[Model]);
  AsciiStrCpy(gSettings.SerialNr,               AppleSerialNumber[Model]);
  AsciiStrCpy(gSettings.FamilyName,             AppleFamilies[Model]);
  AsciiStrCpy(gSettings.BoardManufactureName,   BiosVendor);
  AsciiStrCpy(gSettings.BoardSerialNumber,      AppleBoardSN);
  AsciiStrCpy(gSettings.BoardNumber,            AppleBoardID[Model]);
  AsciiStrCpy(gSettings.BoardVersion,           AppleSystemVersion[Model]);
  AsciiStrCpy(gSettings.LocationInChassis,      AppleBoardLocation);
  AsciiStrCpy(gSettings.ChassisManufacturer,    BiosVendor);
  AsciiStrCpy(gSettings.ChassisAssetTag,        AppleChassisAsset[Model]);
  
  if (Model >= MacPro31) {
    gSettings.BoardType = BaseBoardTypeProcessorMemoryModule; //11;
  } else {
    gSettings.BoardType = BaseBoardTypeMotherBoard; //10; 
  }
  switch (Model) {
    case MacBook11:
    case MacBook21:
    case MacBook41:
    case MacBook52:
    case MacBookAir31:
    case MacBookAir52:
      gSettings.ChassisType = MiscChassisTypeNotebook; //10; 
      gSettings.Mobile = TRUE;
      break;
    case MacBookPro51:
    case MacBookPro81:
    case MacBookPro83:
    case MacBookPro92:
      gSettings.ChassisType = MiscChassisTypePortable; //08;
      gSettings.Mobile = TRUE;
      break;
    case iMac81:
    case iMac101:
    case iMac111:
    case iMac112:
    case iMac113:
    case iMac121:
    case iMac122:
    case iMac131:  
      gSettings.ChassisType = MiscChassisTypeAllInOne; //13; 
      gSettings.Mobile = FALSE;
      break;
    case MacMini21:
    case MacMini51:
    case MacMini62:  
      gSettings.ChassisType = MiscChassisTypeLunchBox; //16; 
      gSettings.Mobile = FALSE;
      break;
    case MacPro31:
    case MacPro41:
    case MacPro51:
      gSettings.ChassisType = MiscChassisTypeMiniTower; //06; 
      gSettings.Mobile = FALSE;
      break;
      
    default: //unknown - use oem SMBIOS value to be default
      gSettings.Mobile = gMobile;
      gSettings.ChassisType = 0; //let SMBIOS value to be
/*      if (gMobile) {
        gSettings.ChassisType = 10; //notebook
      } else {
        gSettings.ChassisType = MiscChassisTypeDeskTop; //03; 
      } */
      break;
  }
 //smc helper
  if (SmcPlatform[Model][0] != 'N') {
    AsciiStrCpy(gSettings.RPlt, SmcPlatform[Model]);
  } else {
    switch (gCPUStructure.Model) {
      case CPU_MODEL_PENTIUM_M:
        AsciiStrCpy(gSettings.RPlt, "M70");
        break;
      case CPU_MODEL_YONAH:
        AsciiStrCpy(gSettings.RPlt, "k22");
        break;
      case CPU_MODEL_MEROM: //TODO check for mobile
        AsciiStrCpy(gSettings.RPlt, "M75");
        break;
      case CPU_MODEL_PENRYN:
        if (gSettings.Mobile) {
          AsciiStrCpy(gSettings.RPlt, "M82");
        } else {
          AsciiStrCpy(gSettings.RPlt, "k36");
        }
        break;
      case CPU_MODEL_SANDY_BRIDGE:
        if (gSettings.Mobile) {
          AsciiStrCpy(gSettings.RPlt, "k90i");
        } else {
          AsciiStrCpy(gSettings.RPlt, "k60");
        }
        break;
      case CPU_MODEL_IVY_BRIDGE:
        AsciiStrCpy(gSettings.RPlt, "j30");
        break;

      default:
        AsciiStrCpy(gSettings.RPlt, "T9");
        break;
    }
  }

  CopyMem(gSettings.REV,  SmcRevision[Model], 6);
  AsciiStrCpy(gSettings.RBr,  gSettings.RPlt); //SmcBranch[Model]); // as no other ideas
  CopyMem(gSettings.EPCI, &SmcConfig[Model], 4);
}

//Other info
/*
 MacBookPro7,1 - penryn P8800 RPlt=k6 REV=1.62f5
 MacBookPro6,2 - i5 M520 arrandale
*/

MACHINE_TYPES GetModelFromString(CHAR8 *ProductName)
{
  UINTN   Index;
  
  for (Index = 0; Index < MaxMachineType; Index++) {
    if (AsciiStrCmp(AppleProductName[Index], ProductName) == 0) {
      return Index;
    }
  }
  // return ending enum as "not found"
  return MaxMachineType;
}

VOID GetDefaultSettings(VOID)
{
  MACHINE_TYPES   Model;
  
  gLanguage         = english;
  Model             = GetDefaultModel();
  gSettings.CpuType	= GetAdvancedCpuType();
  
  SetDMISettingsForModel(Model);
 
  gSettings.KextPatchesAllowed = TRUE;
//  gSettings.NrKexts = 0;
//  gSettings.ResetAddr  = 0;  //0x64; //I wish it will be default
//  gSettings.ResetVal = 0;  //0xFE;
//  gSettings.FixDsdt  = 0x00; //No fixes as we apply patches even for patched DSDT
  
  gSettings.GraphicsInjector = !(((gGraphics[0].Vendor == Ati) &&
                                  ((gGraphics[0].DeviceID & 0xF000) == 0x6000)) ||
                                 ((gGraphics[0].Vendor == Nvidia) &&
                                  (gGraphics[0].DeviceID > 0x1080)));
//  gSettings.CustomEDID = NULL; //no sense to assign 0 as the structure is zeroed
  gSettings.DualLink = 1;
  gSettings.HDAInjection = TRUE;
//  gSettings.HDALayoutId = 0;
  gSettings.USBInjection = TRUE; // enabled by default to have the same behavior as before
  StrCpy(gSettings.DsdtName, L"DSDT.aml");
  gSettings.BacklightLevel = 0xFFFF; //0x0503; -- the value from MBA52
//  gSettings.PointerSpeed = 2;
//  gSettings.DoubleClickTime = 500;
//  gSettings.PointerMirror = FALSE;
/*  
  t0 = AsmReadTsc();
  gBS->Stall(100000); //100ms
  t1 = AsmReadTsc();
  gCPUStructure.TSCCalibr = MultU64x32((t1 - t0), 10); //ticks for 1second
*/
  gSettings.EnableISS = FALSE; //((gCPUStructure.CPUID[CPUID_1][ECX] & (1<<7)) != 0);
  gSettings.Turbo = gCPUStructure.Turbo;
//  msr = AsmReadMsr64(MSR_IA32_MISC_ENABLE);
//  gSettings.Turbo = ((msr & (1ULL<<38)) == 0);
//  gSettings.EnableISS = ((msr & (1ULL<<16)) != 0);
}
