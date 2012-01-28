/**
 platformdata.c
 **/

#include "Platform.h"

/* Machine Default Data */

CHAR8*	DefaultMemEntry = "N/A";

CHAR8*	DefaultSerial = "CT288GT9VT6";

CHAR8* AppleBiosVendor = "Apple Inc.";

CHAR8* AppleManufacturer = "Apple Computer, Inc."; //Old name, before 2007

CHAR8* AppleFirmwareVersion[14] = 
{
	"MB11.88Z.0061.B03.0809221748",
	"MB21.88Z.00A5.B07.0706270922",
	"MB41.88Z.0073.B00.0809221748",
	"MB52.88Z.007D.003.0809221748",
	"MBP51.88Z.007E.B05.0906151647",
	"MBA31.88Z.0061.B01.0712201139",
	"MM21.88Z.009A.B00.0706281359",
	"IM81.88Z.00C1.B00.0803051705",
	"IM101.88Z.00CC.B00.0909031926",
	"IM112.88Z.0034.B00.0802091538",
	"IM121.88Z.0047.B00.1102091756",
	"MP31.88Z.006C.B05.0802291410",
	"MP41.88Z.0081.B04.0903051113",
	"MP51.88Z.007F.B00.1008031144"
};

CHAR8* AppleBoardID[14] = //Lion DR1 compatible
{
	"Mac-F4208CC8",
	"Mac-F4208CA9",
	"Mac-F22788A9",
	"Mac-F22788AA",
	"Mac-F42D86C8",
	"Mac-942452F5819B1C1B",
	"Mac-F4208EAA",
	"Mac-F227BEC8",
	"Mac-F2268CC8",
	"Mac-F2238AC8",
	"Mac-F2238BAE",
	"Mac-F4208DC8",
	"Mac-F221BEC8",
	"Mac-F22788C8"
};

CHAR8* AppleReleaseDate[14] = 
{
	"05/16/06",
	"05/15/07",
	"02/26/08",
	"01/21/09",
	"10/14/08",
	"12/08/10",
	"08/07/07",
	"08/07/09",
	"09/03/09",
	"10/20/09",
	"10/20/09",
	"01/08/08",
	"03/03/09",
	"08/09/10"
};

CHAR8* AppleProductName[14] = 
{
	"MacBook1,1",
	"MacBook2,1",
	"MacBook4,1",
	"MacBook5,2",
	"MacBookPro5,1",
	"MacBookAir3,1",
	"Macmini2,1",
	"iMac8,1",
	"iMac10,1",
	"iMac11,2",
	"iMac12,1",
	"MacPro3,1",
	"MacPro4,1",
	"MacPro5,1"
};

CHAR8* AppleFamilies[14] = 
{
	"MacBook",
	"MacBook",
	"MacBook",
	"MacBook",
	"MacBookPro",
	"MacBookAir",
	"Macmini",
	"iMac",
	"iMac",
	"iMac",
	"iMac",
	"MacPro",
	"MacPro",
	"MacPro"
};


CHAR8* AppleSystemVersion[14] = 
{
	"1.1",
	"1.2",
	"1.3",
	"1.3",
	"1.0",
	"1.0",
	"1.1",
	"1.3",
	"1.0",
	"1.2",
	"1.1",
	"1.3",
	"1.4",
	"1.2"
};

CHAR8* AppleSerialNumber[14] = //random generated
{
	"W80A041AU9B", //MB11
	"W88A041AWGP", //MB21 - merom 05/07
	"W88A041A0P0", //MB41
	"W88AAAAA9GU", //MB52
	"W88439FE1G0", //MBP51
	"W8649476DQX", //MBA31
	"W88A56BYYL2", //MM21 - merom GMA950 07/07
	"W89A00AAX88", //IM71 - merom 01/09
	"W80AA98A5PE", //IM101 - conroe? E7600 01/09
	"W8034342DB7", //IM112
	"W8403D24BG6", //IM121
	"W88A77AA5J4", //MP31 - xeon quad 02/09
	"CT93051DK9Y", //MP41
	"CG154TB9WU3"  //MP51
};
//no! ChassisVersion == BoardID
CHAR8* AppleChassisAsset[14] = 
{
	"MacBook-White",
	"MacBook-White",
	"MacBook-Black",
	"MacBook-Black",
	"MacBook-Aluminum",
	"Air-Enclosure",
	"Mini-Aluminum",
	"iMac-Aluminum",
	"iMac-Aluminum",
	"iMac-Aluminum",
	"iMac-Aluminum",
	"Pro-Enclosure",
	"Pro-Enclosure",
	"Pro-Enclosure"
};

CHAR8* AppleBoardSN = "C02032101R5DC771H";
CHAR8* AppleBoardLocation = "Part Component";

VOID GetDefaultSettings(VOID)
{
  MACHINE_TYPES   Model;
  
  Model             = GetDefaultModel();
  gSettings.CpuType	= GetAdvancedCpuType();
  gSettings.BusSpeed = DivU64x32(gCPUStructure.FSBFrequency, kilo);
  gSettings.CpuFreqMHz = DivU64x32(gCPUStructure.CPUFrequency, Mega);
  
	AsciiStrCpy(gSettings.VendorName,             AppleBiosVendor);
  AsciiStrCpy(gSettings.RomVersion,             AppleFirmwareVersion[Model]);
	AsciiStrCpy(gSettings.ReleaseDate,            AppleReleaseDate[Model]);
	AsciiStrCpy(gSettings.ManufactureName,        AppleBiosVendor);
	AsciiStrCpy(gSettings.ProductName,            AppleProductName[Model]);
	AsciiStrCpy(gSettings.VersionNr,              AppleSystemVersion[Model]);
	AsciiStrCpy(gSettings.SerialNr,               AppleSerialNumber[Model]);
	AsciiStrCpy(gSettings.FamilyName,             AppleFamilies[Model]);	
	AsciiStrCpy(gSettings.BoardManufactureName,   AppleBiosVendor);
  AsciiStrCpy(gSettings.BoardSerialNumber,      AppleBoardSN);
	AsciiStrCpy(gSettings.BoardNumber,            AppleBoardID[Model]);
	AsciiStrCpy(gSettings.LocationInChassis,      AppleBoardLocation);
	AsciiStrCpy(gSettings.ChassisManufacturer,    AppleBiosVendor);
	AsciiStrCpy(gSettings.ChassisAssetTag,        AppleChassisAsset[Model]);
  
}