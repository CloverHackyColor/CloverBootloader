/**
 platformdata.c
 **/

#include "Platform.h"

/* Machine Default Data */

CHAR8*	DefaultMemEntry = "N/A";

CHAR8*	DefaultSerial = "CT288GT9VT6";

CHAR8* BiosVendor = "Apple Inc.";

CHAR8* AppleManufacturer = "Apple Computer, Inc."; //Old name, before 2007

UINT32 gFwFeatures = 0xC001f537; //default valuse

CHAR8* AppleFirmwareVersion[] = 
{
	"MB11.88Z.0061.B03.0809221748",
	"MB21.88Z.00A5.B07.0706270922",
	"MB41.88Z.0073.B00.0809221748",
	"MB52.88Z.007D.003.0809221748",
	"MBP51.88Z.007E.B05.0906151647",
  "MBP81.88Z.0047.B04.1102071707",
  "MBP83.88Z.0047.B24.1110261426",
  "MBP91.88Z.00D3.B06.1205101904",
	"MBA31.88Z.0061.B01.0712201139",
  "MBA51.88Z.00EF.B00.1205221442",
	"MM21.88Z.009A.B00.0706281359",
  "MM51.88Z.006C.B05.1102291410",
	"IM81.88Z.00C1.B00.0803051705",
	"IM101.88Z.00CC.B00.0909031926",
  "IM111.88Z.0034.B02.1003171314",
	"IM112.88Z.0034.B00.0802091538",
  "IM113.88Z.0057.B00.1005031455",
	"IM121.88Z.0047.B1F.1201241648",
  "IM122.88Z.0047.B1F.1223021110",
	"MP31.88Z.006C.B05.0802291410",
	"MP41.88Z.0081.B04.0903051113",
	"MP51.88Z.007F.B00.1008031144"
};

CHAR8* AppleBoardID[] = //Lion DR1 compatible
{
	"Mac-F4208CC8", //MB11
	"Mac-F4208CA9",  //MB21 - merom 05/07
	"Mac-F22788A9",  //MB41
	"Mac-F22788AA",  //MB52
	"Mac-F42D86C8",  //MBP51
  "Mac-94245B3640C91C81",  //MBP81 - i5 SB IntelHD3000
  "Mac-942459F5819B171B",  //MBP83 - i7 SB  ATI
  "Mac-6F01561E16C75D06",  //MBP92 - i5-3210M IvyBridge HD4000
	"Mac-942452F5819B1C1B",  //MBA31
  "Mac-2E6FAB96566FE58C",  //MBA52 - i5-3427U IVY BRIDGE IntelHD4000 did=166
	"Mac-F4208EAA",  //MM21 - merom GMA950 07/07
  "Mac-8ED6AF5B48C039E1",  //MM51 - Sandy + Intel 30000
	"Mac-F227BEC8",  //IM81 - merom 01/09
	"Mac-F2268CC8",  //IM101 - wolfdale? E7600 01/
  "Mac-F2268DAE",  //IM111 - Nehalem
	"Mac-F2238AC8",  //IM112 - Clarkdale
  "Mac-F2238BAE",  //IM113 - lynnfield
  "Mac-942B5BF58194151B",  //IM121 - i5-2500 - sandy
  "Mac-942B59F58194171B",  //IM122 - i7-2600
	"Mac-F2268DC8",  //MP31 - xeon quad 02/09
	"Mac-F221BEC8",  //MP41
	"Mac-F22788C8"   //MP51
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
	"03/05/08",
	"09/03/09",  //im101
	"03/17/10",
  "03/17/10",  //11,2
	"05/03/10",
  "01/24/12",  //121 120124
  "02/23/12",  //122
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
	"iMac8,1",
	"iMac10,1",
  "iMac11,1",
	"iMac11,2",
  "iMac11,3",
	"iMac12,1",
  "iMac12,2",
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
  "1.0",
	"1.3",
	"1.0",
  "1.0",
	"1.2",
  "1.0",
	"1.9",
  "1.9",
	"1.3",
	"1.4",
	"1.2"
};

CHAR8* AppleSerialNumber[] = //random generated
{
	"W80A041AU9B", //MB11
	"W88A041AWGP", //MB21 - merom 05/07
	"W88A041A0P0", //MB41
	"W88AAAAA9GU", //MB52
	"W88439FE1G0", //MBP51
  "W89F9196DH2G", //MBP81 - i5 SB IntelHD3000
  "W88F9CDEDF93", //MBP83 -i7 SB  ATI
  "C02HMAX6DTY3", //MBP92 - i5 IvyBridge HD4000
	"W8649476DQX",  //MBA31
  "C02HRJXWDRVC", //MBA52 - IvyBridge
	"W88A56BYYL2",  //MM21 - merom GMA950 07/07
  "C07G3VUFDJD0", //MM51 - sandy
	"W89A00AAX88", //IM81 - merom 01/09
	"W80AA98A5PE", //IM101 - wolfdale? E7600 01/09
  "G8942B1V5PJ", //IM111 - Nehalem
	"W8034342DB7", //IM112 - Clarkdale
  "QP0312PBDNR", //IM113 - lynnfield
	"W80CF65ADHJF", //IM121 - i5-2500 - sandy
  "W88GG136DHJQ", //IM122 -i7-2600
	"W88A77AA5J4", //MP31 - xeon quad 02/09
	"CT93051DK9Y", //MP41
	"CG154TB9WU3"  //MP51
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

CHAR8* AppleBoardSN = "C02140302D5DMT31M";
CHAR8* AppleBoardLocation = "Part Component";

VOID GetDefaultSettings(VOID)
{
  MACHINE_TYPES   Model;
  
  gLanguage         = english;
  Model             = GetDefaultModel();
  gSettings.CpuType	= GetAdvancedCpuType();
//  gSettings.BusSpeed = DivU64x32(gCPUStructure.FSBFrequency, kilo); //Hz -> kHz
//  gSettings.CpuFreqMHz = DivU64x32(gCPUStructure.CPUFrequency, Mega); //Hz ->MHz
  
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
 
  gSettings.KextPatchesAllowed = TRUE;
  gSettings.NrKexts = 0;
  gSettings.ResetAddr  = 0x64; //I wish it will be default
  gSettings.ResetVal = 0xFE;
  gSettings.FixDsdt  = 0x00; //No fixes as we apply patches even for patched DSDT
  gSettings.GraphicsInjector = TRUE;
  gSettings.HDAInjection = TRUE;
  gSettings.HDALayoutId = 0;
  gSettings.USBInjection = TRUE; // enabled by default to have the same behavior as before
  gSettings.Mobile = gMobile;  //default
  gSettings.ChassisType = 0;
  StrCpy(gSettings.DsdtName, L"DSDT.aml");
  gSettings.BacklightLevel = 0xFFFF; //0x0503; -- the value from MBA52

}