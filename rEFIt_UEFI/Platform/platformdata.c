/**
 platformdata.c
 **/

#include "Platform.h"
#include "nvidia.h"

/* Machine Default Data */

CHAR8   *DefaultMemEntry        = "N/A";

CHAR8   *DefaultSerial          = "CT288GT9VT6";

CHAR8   *BiosVendor             = "Apple Inc.";

CHAR8   *AppleManufacturer      = "Apple Computer, Inc."; //Old name, before 2007

UINT32  gFwFeatures;

UINT32  gFwFeaturesMask;

UINT64  gPlatformFeature;


// All SMBIOS data were updated by Sherlocks, PMheart.
// FredWst supported SmcExtract.

CHAR8   *AppleFirmwareVersion[] =
{
  "MB11.88Z.0061.B03.0610121324",   // MacBook1,1,
  "MB21.88Z.00A5.B07.0706270922",   // MacBook2,1,
  "MB31.88Z.008E.B02.0803051832",   // MacBook3,1,
  "MB41.88Z.00C1.B00.0802091535",   // MacBook4,1,
  "MB51.88Z.007D.B03.0904271443",   // MacBook5,1,
  "MB52.88Z.0088.B05.0904162222",   // MacBook5,2,
  "MB61.88Z.00CB.B00.1708080203",   // MacBook6,1,
  "MB71.88Z.003D.B00.1708080317",   // MacBook7,1,
  "MB81.88Z.0168.B00.1708080033",   // MacBook8,1,
  "MB91.88Z.0159.B00.1708080011",   // MacBook9,1,
  "MB101.88Z.0154.B00.1708080122",  // MacBook10,1,
  "MBP11.88Z.0055.B08.0610121325",  // MacBookPro1,1,
  "MBP12.88Z.0061.B03.0610121334",  // MacBookPro1,2,
  "MBP21.88Z.00A5.B08.0708131242",  // MacBookPro2,1,
  "MBP22.88Z.00A5.B07.0708131242",  // MacBookPro2,2,
  "MBP31.88Z.0070.B07.0803051658",  // MacBookPro3,1,
  "MBP41.88Z.00C1.B03.0802271651",  // MacBookPro4,1,
  "MBP51.88Z.007E.B06.1202061253",  // MacBookPro5,1,
  "MBP52.88Z.008E.B05.0905042202",  // MacBookPro5,2,
  "MBP53.88Z.00AC.B03.0906151647",  // MacBookPro5,3,
  "MBP53.88Z.00AC.B03.0906151647",  // MacBookPro5,4,
  "MBP55.88Z.00AC.B03.0906151708",  // MacBookPro5,5,
  "MBP61.88Z.005A.B00.1708072217",  // MacBookPro6,1,
  "MBP61.88Z.005A.B00.1708072217",  // MacBookPro6,2,
  "MBP71.88Z.003D.B00.1708080058",  // MacBookPro7,1,
  "MBP81.88Z.004D.B00.1708080655",  // MacBookPro8,1,
  "MBP81.88Z.004D.B00.1708080655",  // MacBookPro8,2,
  "MBP81.88Z.004D.B00.1708080655",  // MacBookPro8,3,
  "MBP91.88Z.00D7.B00.1708080744",  // MacBookPro9,1,
  "MBP91.88Z.00D7.B00.1708080744",  // MacBookPro9,2,
  "MBP101.88Z.00F2.B00.1708080809", // MacBookPro10,1,
  "MBP102.88Z.010B.B00.1708080805", // MacBookPro10,2,
  "MBP111.88Z.0142.B00.1708080655", // MacBookPro11,1,
  "MBP112.88Z.0142.B00.1708080655", // MacBookPro11,2,
  "MBP112.88Z.0142.B00.1708080655", // MacBookPro11,3,
  "MBP114.88Z.0177.B00.1708080033", // MacBookPro11,4,
  "MBP114.88Z.0177.B00.1708080033", // MacBookPro11,5,
  "MBP121.88Z.0171.B00.1708080033", // MacBookPro12,1,
  "MBP131.88Z.0212.B00.1708080127", // MacBookPro13,1,
  "MBP132.88Z.0233.B00.1708080034", // MacBookPro13,2,
  "MBP133.88Z.0233.B00.1708080034", // MacBookPro13,3,
  "MBP141.88Z.0167.B00.1708080034", // MacBookPro14,1,
  "MBP142.88Z.0167.B00.1708080034", // MacBookPro14,2,
  "MBP143.88Z.0167.B00.1708080129", // MacBookPro14,3,
  "MBA11.88Z.00BB.B03.0803171226",  // MacBookAir1,1,
  "MBA21.88Z.0075.B05.1003051506",  // MacBookAir2,1,
  "MBA31.88Z.0067.B00.1708080355",  // MacBookAir3,1,
  "MBA31.88Z.0067.B00.1708080355",  // MacBookAir3,2,
  "MBA41.88Z.007B.B00.1708072159",  // MacBookAir4,1,
  "MBA41.88Z.007B.B00.1708072159",  // MacBookAir4,2,
  "MBA51.88Z.00F4.B00.1708080803",  // MacBookAir5,1,
  "MBA51.88Z.00F4.B00.1708080803",  // MacBookAir5,2,
  "MBA61.88Z.0103.B00.1708080653",  // MacBookAir6,1,
  "MBA61.88Z.0103.B00.1708080653",  // MacBookAir6,2,
  "MBA71.88Z.0171.B00.1708072210",  // MacBookAir7,1,
  "MBA71.88Z.0171.B00.1708072210",  // MacBookAir7,2,
  "MM11.88Z.0055.B08.0610121326",   // Macmini1,1,
  "MM21.88Z.009A.B00.0706281359",   // Macmini2,1,
  "MM31.88Z.0081.B06.0904271717",   // Macmini3,1,
  "MM41.88Z.0045.B00.1708072325",   // Macmini4,1,
  "MM51.88Z.007B.B00.1708080744",   // Macmini5,1,
  "MM51.88Z.007B.B00.1708080744",   // Macmini5,2,
  "MM51.88Z.007B.B00.1708080744",   // Macmini5,3,
  "MM61.88Z.010B.B00.1708080649",   // Macmini6,1,
  "MM61.88Z.010B.B00.1708080649",   // Macmini6,2,
  "MM71.88Z.0224.B00.1708080033",   // Macmini7,1,
  "IM41.88Z.0055.B08.0609061538",   // iMac4,1,
  "IM42.88Z.0071.B03.0610121320",   // iMac4,2,
  "IM51.88Z.0090.B09.0706270921",   // iMac5,1,
  "IM52.88Z.0090.B09.0706270913",   // iMac5,2,
  "IM61.88Z.0093.B07.0804281538",   // iMac6,1,
  "IM71.88Z.007A.B03.0803051705",   // iMac7,1,
  "IM81.88Z.00C1.B00.0802091538",   // iMac8,1,
  "IM91.88Z.008D.B08.0904271717",   // iMac9,1,
  "IM101.88Z.00CF.B00.1708080133",  // iMac10,1,
  "IM111.88Z.0037.B00.1708080241",  // iMac11,1,
  "IM112.88Z.005B.B00.1708080439",  // iMac11,2,
  "IM112.88Z.005B.B00.1708080439",  // iMac11,3,
  "IM121.88Z.004D.B00.1708080012",  // iMac12,1,
  "IM121.88Z.004D.B00.1708080012",  // iMac12,2,
  "IM131.88Z.010F.B00.1708080805",  // iMac13,1,
  "IM131.88Z.010F.B00.1708080805",  // iMac13,2,
  "IM131.88Z.010F.B00.1708080805",  // iMac13,3,
  "IM141.88Z.0122.B00.1708080806",  // iMac14,1,
  "IM142.88Z.0122.B00.1708080739",  // iMac14,2,
  "IM143.88Z.0122.B00.1708080732",  // iMac14,3,
  "IM144.88Z.0183.B00.1708080656",  // iMac14,4,
  "IM151.88Z.0211.B00.1708080656",  // iMac15,1,
  "IM161.88Z.0212.B00.1708080033",  // iMac16,1,
  "IM162.88Z.0212.B00.1708080033",  // iMac16,2,
  "IM171.88Z.0110.B00.1708080012",  // iMac17,1,
  "IM181.88Z.0151.B00.1708080034",  // iMac18,1,
  "IM183.88Z.0151.B00.1708080034",  // iMac18,2,
  "IM183.88Z.0151.B00.1708080034",  // iMac18,3,
  "MP11.88Z.005C.B08.0707021221",   // MacPro1,1,
  "MP21.88Z.007F.B06.0707021348",   // MacPro2,1,
  "MP31.88Z.006C.B05.0802291410",   // MacPro3,1,
  "MP41.88Z.0081.B07.0910130729",   // MacPro4,1,
  "MP51.88Z.007F.B03.1010071432",   // MacPro5,1,
  "MP61.88Z.0120.B00.1708080652",   // MacPro6,1,
  "XS11.88Z.0080.B01.0706271533",   // Xserve1,1,
  "XS21.88Z.006C.B06.0804011317",   // Xserve2,1,
  "XS31.88Z.0081.B06.0908061300",   // Xserve3,1,
};

CHAR8* AppleBoardID[] =    //Lion DR1 compatible
{
  "Mac-F4208CC8",          // MacBook1,1,      Intel Core Duo T2500 @ 2.00 GHz
  "Mac-F4208CA9",          // MacBook2,1,      Intel Core 2 Duo T7200 @ 2.00 GHz
  "Mac-F22788C8",          // MacBook3,1,      Intel Core 2 Duo T7500 @ 2.20 GHz
  "Mac-F22788A9",          // MacBook4,1,      Intel Core 2 Duo T8300 @ 2.39 GHz
  "Mac-F42D89C8",          // MacBook5,1,      Intel Core 2 Duo P7350 @ 2.00 GHz
  "Mac-F22788AA",          // MacBook5,2,      Intel Core 2 Duo P7450 @ 2.13 GHz
  "Mac-F22C8AC8",          // MacBook6,1,      Intel Core 2 Duo P7550 @ 2.26 GHz
  "Mac-F22C89C8",          // MacBook7,1,      Intel Core 2 Duo P8600 @ 2.40 GHz
  "Mac-BE0E8AC46FE800CC",  // MacBook8,1,      Intel Core M-5Y51 @ 1.20 GHz
  "Mac-9AE82516C7C6B903",  // MacBook9,1,      Intel Core m5-6Y54 @ 1.20 GHz
  "Mac-EE2EBD4B90B839A8",  // MacBook10,1,     Intel Core i5-7Y54 @ 1.30 GHz
  "Mac-F425BEC8",          // MacBookPro1,1,   Intel Core Duo T2500 @ 2.00 GHz
  "Mac-F42DBEC8",          // MacBookPro1,2,   Intel Core Duo T2600 @ 2.17 GHz 
  "Mac-F42189C8",          // MacBookPro2,1,   Intel Core 2 Duo T7600 @ 2.33 GHz 
  "Mac-F42187C8",          // MacBookPro2,2,   Intel Core 2 Duo T7400 @ 2.16 GHz 
  "Mac-F4238BC8",          // MacBookPro3,1,   Intel Core 2 Duo T7700 @ 2.40 GHz 
  "Mac-F42C89C8",          // MacBookPro4,1,   Intel Core 2 Duo T8300 @ 2.40 GHz 
  "Mac-F42D86C8",          // MacBookPro5,1,   Intel Core 2 Duo P8600 @ 2.40 GHz 
  "Mac-F2268EC8",          // MacBookPro5,2,   Intel Core 2 Duo T9600 @ 2.80 GHz 
  "Mac-F22587C8",          // MacBookPro5,3,   Intel Core 2 Duo P8800 @ 2.66 GHz 
  "Mac-F22587A1",          // MacBookPro5,4,   Intel Core 2 Duo P8700 @ 2.53 GHz 
  "Mac-F2268AC8",          // MacBookPro5,5,   Intel Core 2 Duo P7550 @ 2.26 GHz 
  "Mac-F22589C8",          // MacBookPro6,1,   Intel Core i5-540M @ 2.53 GHz 
  "Mac-F22586C8",          // MacBookPro6,2,   Intel Core i7-620M @ 2.66 GHz 
  "Mac-F222BEC8",          // MacBookPro7,1,   Intel Core 2 Duo P8600 @ 2.40 GHz 
  "Mac-94245B3640C91C81",  // MacBookPro8,1,   Intel Core i5-2415M @ 2.30 GHz 
  "Mac-94245A3940C91C80",  // MacBookPro8,2,   Intel Core i7-2675QM @ 2.20 GHz 
  "Mac-942459F5819B171B",  // MacBookPro8,3,   Intel Core i7-2860QM @ 2.49 GHz
  "Mac-4B7AC7E43945597E",  // MacBookPro9,1,   Intel Core i7-3720QM @ 2.60 GHz 
  "Mac-6F01561E16C75D06",  // MacBookPro9,2,   Intel Core i5-3210M @ 2.50 GHz 
  "Mac-C3EC7CD22292981F",  // MacBookPro10,1,  Intel Core i7-3740QM @ 2.70 GHz 
  "Mac-AFD8A9D944EA4843",  // MacBookPro10,2,  Intel Core i5-3230M @ 2.60 GHz 
  "Mac-189A3D4F975D5FFC",  // MacBookPro11,1,  Intel Core i7-4558U @ 2.80 GHz 
  "Mac-3CBD00234E554E41",  // MacBookPro11,2,  Intel Core i7-4750HQ @ 2.00 GHz 
  "Mac-2BD1B31983FE1663",  // MacBookPro11,3,  Intel Core i7-4870HQ @ 2.50 GHz 
  "Mac-06F11FD93F0323C5",  // MacBookPro11,4,  Intel Core i7-4770HQ @ 2.20 GHz 
  "Mac-06F11F11946D27C5",  // MacBookPro11,5,  Intel Core i7-4870HQ @ 2.50 GHz 
  "Mac-E43C1C25D4880AD6",  // MacBookPro12,1,  Intel Core i5-5257U @ 2.70 GHz 
  "Mac-473D31EABEB93F9B",  // MacBookPro13,1,  Intel Core i5-6360U @ 2.00 GHz 
  "Mac-66E35819EE2D0D05",  // MacBookPro13,2,  Intel Core i5-6287U @ 3.10 GHz 
  "Mac-A5C67F76ED83108C",  // MacBookPro13,3,  Intel Core i7-6920HQ @ 2.90 GHz
  "Mac-B4831CEBD52A0C4C",  // MacBookPro14,1,  Intel Core i5-7360U @ 2.30 GHz
  "Mac-CAD6701F7CEA0921",  // MacBookPro14,2,  Intel Core i5-7267U @ 3.09 GHz
  "Mac-551B86E5744E2388",  // MacBookPro14,3,  Intel Core i7-7700HQ @ 2.80 GHz
  "Mac-F42C8CC8",          // MacBookAir1,1,   Intel Core 2 Duo P7500 @ 1.60 GHz 
  "Mac-F42D88C8",          // MacBookAir2,1,   Intel Core 2 Duo L9600 @ 2.13 GHz 
  "Mac-942452F5819B1C1B",  // MacBookAir3,1,   Intel Core 2 Duo U9400 @ 1.40 GHz
  "Mac-942C5DF58193131B",  // MacBookAir3,2,   Intel Core 2 Duo L9600 @ 2.13 GHz 
  "Mac-C08A6BB70A942AC2",  // MacBookAir4,1,   Intel Core i7-2677M @ 1.80 GHz 
  "Mac-742912EFDBEE19B3",  // MacBookAir4,2,   Intel Core i5-2557M @ 1.70 GHz 
  "Mac-66F35F19FE2A0D05",  // MacBookAir5,1,   Intel Core i7-3667U @ 2.00 GHz 
  "Mac-2E6FAB96566FE58C",  // MacBookAir5,2,   Intel Core i5-3427U @ 1.80 GHz 
  "Mac-35C1E88140C3E6CF",  // MacBookAir6,1,   Intel Core i7-4650U @ 1.70 GHz 
  "Mac-7DF21CB3ED6977E5",  // MacBookAir6,2,   Intel Core i5-4250U @ 1.30 GHz 
  "Mac-9F18E312C5C2BF0B",  // MacBookAir7,1,   Intel Core i5-5250U @ 1.60 GHz 
  "Mac-937CB26E2E02BB01",  // MacBookAir7,2,   Intel Core i7-5650U @ 2.20 GHz 
  "Mac-F4208EC8",          // Macmini1,1,      Intel Core 2 Duo T7200 @ 2.00 GHz
  "Mac-F4208EAA",          // Macmini2,1,      Intel Core 2 Duo T5600 @ 1.83 GHz
  "Mac-F22C86C8",          // Macmini3,1,      Intel Core 2 Duo P7550 @ 2.26 GHz
  "Mac-F2208EC8",          // Macmini4,1,      Intel Core 2 Duo P8600 @ 2.40 GHz
  "Mac-8ED6AF5B48C039E1",  // Macmini5,1,      Intel Core i5-2415M @ 2.30 GHz
  "Mac-4BC72D62AD45599E",  // Macmini5,2,      Intel Core i7-2620M @ 2.70 GHz
  "Mac-7BA5B2794B2CDB12",  // Macmini5,3,      Intel Core i7-2635QM @ 2.00 GHz
  "Mac-031AEE4D24BFF0B1",  // Macmini6,1,      Intel Core i5-3210M @ 2.50 GHz
  "Mac-F65AE981FFA204ED",  // Macmini6,2,      Intel Core i7-3615QM @ 2.30 GHz
  "Mac-35C5E08120C7EEAF",  // Macmini7,1,      Intel Core i5-4278U @ 2.60 GHz
  "Mac-F42787C8",          // iMac4,1,         Intel Core 2 Duo T7200 @ 2.00 GHz
  "Mac-F4218EC8",          // iMac4,2,         Intel Core 2 Duo T5600 @ 1.83 GHz
  "Mac-F4228EC8",          // iMac5,1,         Intel Core 2 Duo T7400 @ 2.16 GHz
  "Mac-F4218EC8",          // iMac5,2,         Intel Core 2 Duo T5600 @ 1.83 GHz
  "Mac-F4218FC8",          // iMac6,1,         Intel Core 2 Duo T7400 @ 2.16 GHz
  "Mac-F42386C8",          // iMac7,1,         Intel Core 2 Extreme X7900 @ 2.80 GHz
  "Mac-F227BEC8",          // iMac8,1,         Intel Core 2 Duo E8435 @ 3.06 GHz
  "Mac-F2218FA9",          // iMac9,1,         Intel Core 2 Duo E8135 @ 2.66 GHz
  "Mac-F2268CC8",          // iMac10,1,        Intel Core 2 Duo E7600 @ 3.06 GHz
  "Mac-F2268DAE",          // iMac11,1,        Intel Core i7-860 @ 2.80 GHz
  "Mac-F2238AC8",          // iMac11,2,        Intel Core i3-540 @ 3.06 GHz
  "Mac-F2238BAE",          // iMac11,3,        Intel Core i5-760 @ 2.80 GHz
  "Mac-942B5BF58194151B",  // iMac12,1,        Intel Core i7-2600S @ 2.80 GHz
  "Mac-942B59F58194171B",  // iMac12,2,        Intel Core i5-2500S @ 2.70 GHz
  "Mac-00BE6ED71E35EB86",  // iMac13,1,        Intel Core i7-3770S @ 3.10 GHz
  "Mac-FC02E91DDD3FA6A4",  // iMac13,2,        Intel Core i5-3470 @ 3.20 GHz
  "Mac-7DF2A3B5E5D671ED",  // iMac13,3,        Intel Core i3-3225 @ 3.30 GHz
  "Mac-031B6874CF7F642A",  // iMac14,1,        Intel Core i5-4570R @ 2.70 GHz
  "Mac-27ADBB7B4CEE8E61",  // iMac14,2,        Intel Core i5-4570 @ 3.20 GHz
  "Mac-77EB7D7DAF985301",  // iMac14,3,        Intel Core i5-4570S @ 2.90 GHz
  "Mac-81E3E92DD6088272",  // iMac14,4,        Intel Core i5-4260U @ 1.40 GHz
  "Mac-42FD25EABCABB274",  // iMac15,1,        Intel Core i5-4690 @ 3.50 GHz
  "Mac-A369DDC4E67F1C45",  // iMac16,1,        Intel Core i5-5250U @ 1.60 GHz
  "Mac-FFE5EF870D7BA81A",  // iMac16,2,        Intel Core i5-5575R @ 2.80 GHz
  "Mac-B809C3757DA9BB8D",  // iMac17,1,        Intel Core i7-6700K @ 4.00 GHz
  "Mac-4B682C642B45593E",  // iMac18,1,        Intel Core i5-7360U @ 2.30 GHz
  "Mac-77F17D7DA9285301",  // iMac18,2,        Intel Core i5-7500 @ 3.40 GHz
  "Mac-BE088AF8C5EB4FA2",  // iMac18,3,        Intel Core i7-7700K @ 4.20 GHz
  "Mac-F4208DC8",          // MacPro1,1,       Intel Xeon X5355 @ 2.66 GHz x2
  "Mac-F4208DA9",          // MacPro2,1,       Intel Xeon X5365 @ 2.99 GHz x2
  "Mac-F42C88C8",          // MacPro3,1,       Intel Xeon E5462 @ 2.80 GHz x2
  "Mac-F221BEC8",          // MacPro4,1,       Intel Xeon X5670 @ 2.93 GHz x2
  "Mac-F221BEC8",          // MacPro5,1,       Intel Xeon X5675 @ 3.06 GHz x2
  "Mac-F60DEB81FF30ACF6",  // MacPro6,1,       Intel Xeon E5-1650 v2 @ 3.50 GHz
  "Mac-F4208AC8",          // Xserve1,1,       Intel Xeon E5345 @ 2.33 GHz x2
  "Mac-F42289C8",          // Xserve2,1,       Intel Xeon E5472 @ 3.00 GHz x2
  "Mac-F223BEC8",          // Xserve3,1,       Intel Xeon E5520 @ 2.26 GHz
};

CHAR8* AppleProductName[] =
{
  "MacBook1,1",
  "MacBook2,1",
  "MacBook3,1",
  "MacBook4,1",
  "MacBook5,1",
  "MacBook5,2",
  "MacBook6,1",
  "MacBook7,1",
  "MacBook8,1",
  "MacBook9,1",
  "MacBook10,1",
  "MacBookPro1,1",
  "MacBookPro1,2",
  "MacBookPro2,1",
  "MacBookPro2,2",
  "MacBookPro3,1",
  "MacBookPro4,1",
  "MacBookPro5,1",
  "MacBookPro5,2",
  "MacBookPro5,3",
  "MacBookPro5,4",
  "MacBookPro5,5",
  "MacBookPro6,1",
  "MacBookPro6,2",
  "MacBookPro7,1",
  "MacBookPro8,1",
  "MacBookPro8,2",
  "MacBookPro8,3",
  "MacBookPro9,1",
  "MacBookPro9,2",
  "MacBookPro10,1",
  "MacBookPro10,2",
  "MacBookPro11,1",
  "MacBookPro11,2",
  "MacBookPro11,3",
  "MacBookPro11,4",
  "MacBookPro11,5",
  "MacBookPro12,1",
  "MacBookPro13,1",
  "MacBookPro13,2",
  "MacBookPro13,3",
  "MacBookPro14,1",
  "MacBookPro14,2",
  "MacBookPro14,3",
  "MacBookAir1,1",
  "MacBookAir2,1",
  "MacBookAir3,1",
  "MacBookAir3,2",
  "MacBookAir4,1",
  "MacBookAir4,2",
  "MacBookAir5,1",
  "MacBookAir5,2",
  "MacBookAir6,1",
  "MacBookAir6,2",
  "MacBookAir7,1",
  "MacBookAir7,2",
  "Macmini1,1",
  "Macmini2,1",
  "Macmini3,1",
  "Macmini4,1",
  "Macmini5,1",
  "Macmini5,2",
  "Macmini5,3",
  "Macmini6,1",
  "Macmini6,2",
  "Macmini7,1",
  "iMac4,1",
  "iMac4,2",
  "iMac5,1",
  "iMac5,2",
  "iMac6,1",
  "iMac7,1",
  "iMac8,1",
  "iMac9,1",
  "iMac10,1",
  "iMac11,1",
  "iMac11,2",
  "iMac11,3",
  "iMac12,1",
  "iMac12,2",
  "iMac13,1",
  "iMac13,2",
  "iMac13,3",
  "iMac14,1",
  "iMac14,2",
  "iMac14,3",
  "iMac14,4",
  "iMac15,1",
  "iMac16,1",
  "iMac16,2",
  "iMac17,1",
  "iMac18,1",
  "iMac18,2",
  "iMac18,3",
  "MacPro1,1",
  "MacPro2,1",
  "MacPro3,1",
  "MacPro4,1",
  "MacPro5,1",
  "MacPro6,1",
  "Xserve1,1",
  "Xserve2,1",
  "Xserve3,1",
};

CHAR8* AppleFamilies[] =
{
  "MacBook",       // MacBook1,1,
  "MacBook",       // MacBook2,1,
  "MacBook",       // MacBook3,1,
  "MacBook",       // MacBook4,1,
  "MacBook",       // MacBook5,1,
  "MacBook",       // MacBook5,2,
  "MacBook",       // MacBook6,1,
  "MacBook",       // MacBook7,1,
  "MacBook",       // MacBook8,1,
  "MacBook",       // MacBook9,1,
  "MacBook",       // MacBook10,1,
  "MacBook Pro",   // MacBookPro1,1,
  "MacBook Pro",   // MacBookPro2,1,
  "MacBook Pro",   // MacBookPro2,1,
  "MacBook Pro",   // MacBookPro2,2,
  "MacBook Pro",   // MacBookPro3,1,
  "MacBook Pro",   // MacBookPro4,1,
  "MacBook Pro",   // MacBookPro5,1,
  "MacBook Pro",   // MacBookPro5,2,
  "MacBook Pro",   // MacBookPro5,3,
  "MacBook Pro",   // MacBookPro5,4,
  "MacBook Pro",   // MacBookPro5,5,
  "MacBook Pro",   // MacBookPro6,1,
  "MacBook Pro",   // MacBookPro6,2,
  "MacBook Pro",   // MacBookPro7,1,
  "MacBook Pro",   // MacBookPro8,1,
  "MacBook Pro",   // MacBookPro8,2,
  "MacBook Pro",   // MacBookPro8,3,
  "MacBook Pro",   // MacBookPro9,1,
  "MacBook Pro",   // MacBookPro9,2,
  "MacBook Pro",   // MacBookPro10,1,
  "MacBook Pro",   // MacBookPro10,2,
  "MacBook Pro",   // MacBookPro11,1,
  "MacBook Pro",   // MacBookPro11,2,
  "MacBook Pro",   // MacBookPro11,3,
  "MacBook Pro",   // MacBookPro11,4,
  "MacBook Pro",   // MacBookPro11,5,
  "MacBook Pro",   // MacBookPro12,1,
  "MacBook Pro",   // MacBookPro13,1,
  "MacBook Pro",   // MacBookPro13,2,
  "MacBook Pro",   // MacBookPro13,3,
  "MacBook Pro",   // MacBookPro14,1,
  "MacBook Pro",   // MacBookPro14,2,
  "MacBook Pro",   // MacBookPro14,3,
  "MacBook Air",   // MacBookAir1,1,
  "MacBook Air",   // MacBookAir2,1,
  "MacBook Air",   // MacBookAir3,1,
  "MacBook Air",   // MacBookAir3,2,
  "MacBook Air",   // MacBookAir4,1,
  "MacBook Air",   // MacBookAir4,2,
  "MacBook Air",   // MacBookAir5,1,
  "MacBook Air",   // MacBookAir5,2,
  "MacBook Air",   // MacBookAir6,1,
  "MacBook Air",   // MacBookAir6,2,
  "MacBook Air",   // MacBookAir7,1,
  "MacBook Air",   // MacBookAir7,2,
  "Mac mini",      // Macmini1,1,
  "Mac mini",      // Macmini2,1,
  "Mac mini",      // Macmini3,1,
  "Mac mini",      // Macmini4,1,
  "Mac mini",      // Macmini5,1,
  "Mac mini",      // Macmini5,2,
  "Mac mini",      // Macmini5,3,
  "Mac mini",      // Macmini6,1,
  "Mac mini",      // Macmini6,2,
  "Mac mini",      // Macmini7,1,
  "iMac",          // iMac4,1,
  "iMac",          // iMac4,2,
  "iMac",          // iMac5,1,
  "iMac",          // iMac5,2,
  "iMac",          // iMac6,1,
  "iMac",          // iMac7,1,
  "iMac",          // iMac8,1,
  "iMac",          // iMac9,1,
  "iMac",          // iMac10,1,
  "iMac",          // iMac11,1,
  "iMac",          // iMac11,2,
  "iMac",          // iMac11,3,
  "iMac",          // iMac12,1,
  "iMac",          // iMac12,2,
  "iMac",          // iMac13,1,
  "iMac",          // iMac13,2,
  "iMac",          // iMac13,3,
  "iMac",          // iMac14,1,
  "iMac",          // iMac14,2,
  "iMac",          // iMac14,3,
  "iMac",          // iMac14,4,
  "iMac",          // iMac15,1,
  "iMac",          // iMac16,1,
  "iMac",          // iMac16,2,
  "iMac17,1",      // iMac17,1,
  "iMac",          // iMac18,1,
  "iMac",          // iMac18,2,
  "iMac",          // iMac18,3,
  "MacPro",        // MacPro1,1,
  "MacPro",        // MacPro2,1,
  "MacPro",        // MacPro3,1,
  "MacPro",        // MacPro4,1,
  "MacPro",        // MacPro5,1,
  "MacPro",        // MacPro6,1,
  "Xserve",        // Xserve1,1,
  "Xserve",        // Xserve2,1,
  "Xserve",        // Xserve3,1,
};


CHAR8* AppleSystemVersion[] =
{
  "1.1",  // MacBook1,1,
  "1.2",  // MacBook2,1,
  "1.3",  // MacBook3,1,
  "1.3",  // MacBook4,1,
  "1.3",  // MacBook5,1,
  "1.3",  // MacBook5,2,
  "1.0",  // MacBook6,1,
  "1.0",  // MacBook7,1,
  "1.0",  // MacBook8,1,
  "1.0",  // MacBook9,1,
  "1.0",  // MacBook10,1,
  "1.0",  // MacBookPro1,1,
  "1.0",  // MacBookPro1,2,
  "1.0",  // MacBookPro2,1,
  "1.0",  // MacBookPro2,2,
  "1.0",  // MacBookPro3,1,
  "1.0",  // MacBookPro4,1,
  "1.0",  // MacBookPro5,1,
  "1.0",  // MacBookPro5,2,
  "1.0",  // MacBookPro5,3,
  "1.0",  // MacBookPro5,4,
  "1.0",  // MacBookPro5,5,
  "1.0",  // MacBookPro6,1,
  "1.0",  // MacBookPro6,2,
  "1.0",  // MacBookPro7,1,
  "1.0",  // MacBookPro8,1,
  "1.0",  // MacBookPro8,2,
  "1.0",  // MacBookPro8,3,
  "1.0",  // MacBookPro9,1,
  "1.0",  // MacBookPro9,2,
  "1.0",  // MacBookPro10,1,
  "1.0",  // MacBookPro10,2,
  "1.0",  // MacBookPro11,1,
  "1.0",  // MacBookPro11,2,
  "1.0",  // MacBookPro11,3,
  "1.0",  // MacBookPro11,4,
  "1.0",  // MacBookPro11,5,
  "1.0",  // MacBookPro12,1,
  "1.0",  // MacBookPro13,1,
  "1.0",  // MacBookPro13,2,
  "1.0",  // MacBookPro13,3,
  "1.0",  // MacBookPro14,1,
  "1.0",  // MacBookPro14,2,
  "1.0",  // MacBookPro14,3,
  "1.0",  // MacBookAir1,1,
  "1.0",  // MacBookAir2,1,
  "1.0",  // MacBookAir3,1,
  "1.0",  // MacBookAir3,2,
  "1.0",  // MacBookAir4,1,
  "1.0",  // MacBookAir4,2,
  "1.0",  // MacBookAir5,1,
  "1.0",  // MacBookAir5,2, 
  "1.0",  // MacBookAir6,1,
  "1.0",  // MacBookAir6,2,
  "1.0",  // MacBookAir7,1,
  "1.0",  // MacBookAir7,2,
  "1.0",  // Macmini1,1,
  "1.1",  // Macmini2,1,
  "1.0",  // Macmini3,1,
  "1.0",  // Macmini4,1,
  "1.0",  // Macmini5,1,
  "1.0",  // Macmini5,2,
  "1.0",  // Macmini5,3,
  "1.0",  // Macmini6,1,
  "1.0",  // Macmini6,2,
  "1.0",  // Macmini7,1,
  "1.0",  // iMac4,1,
  "1.0",  // iMac4,2,
  "1.0",  // iMac5,1,
  "1.0",  // iMac5,2,
  "1.0",  // iMac6,1,
  "1.0",  // iMac7,1,
  "1.3",  // iMac8,1,
  "1.0",  // iMac9,1,
  "1.0",  // iMac10,1,
  "1.0",  // iMac11,1,
  "1.2",  // iMac11,2,
  "1.0",  // iMac11,3,
  "1.9",  // iMac12,1,
  "1.9",  // iMac12,2,
  "1.0",  // iMac13,1,
  "1.0",  // iMac13,2,
  "1.0",  // iMac13,3,
  "1.0",  // iMac14,1,
  "1.0",  // iMac14,2,
  "1.0",  // iMac14,3,
  "1.0",  // iMac14,4,
  "1.0",  // iMac15,1,
  "1.0",  // iMac16,1,
  "1.0",  // iMac16,2,
  "1.0",  // iMac17,1,
  "1.0",  // iMac18,1,
  "1.0",  // iMac18,2,
  "1.0",  // iMac18,3,
  "1.0",  // MacPro1,1,
  "1.0",  // MacPro2,1,
  "1.3",  // MacPro3,1,
  "1.4",  // MacPro4,1,
  "1.2",  // MacPro5,1,
  "1.0",  // MacPro6,1,
  "1.0",  // Xserve1,1,
  "1.0",  // Xserve2,1,
  "1.0",  // Xserve3,1,
};

CHAR8* AppleSerialNumber[] = //random generated
{
  "W80A041AU9B",  // MacBook1,1,
  "W88A041AWGP",  // MacBook2,1,
  "W8803HACY51",  // MacBook3,1,
  "W88A041A0P0",  // MacBook4,1,
  "W8944T1S1AQ",  // MacBook5,1,
  "W88AAAAA9GU",  // MacBook5,2,
  "451131JCGAY",  // MacBook6,1,
  "451211MEF5X",  // MacBook7,1,
  "C02RCE58GCN3", // MacBook8,1,
  "C02RM408HDNK", // MacBook9,1,
  "C02TQHACHH27", // MacBook10,1,
  "W884857JVJ1",  // MacBookPro1,1,
  "W8629HACTHY",  // MacBookPro1,2,
  "W88130WUW0H",  // MacBookPro2,1,
  "W8827B4CW0L",  // MacBookPro2,2,
  "W8841OHZX91",  // MacBookPro3,1,
  "W88484F2YP4",  // MacBookPro4,1,
  "W88439FE1G0",  // MacBookPro5,1,
  "W8908HAC2QP",  // MacBookPro5,2,
  "W8035TG97XK",  // MacBookPro5,3,
  "W8948HAC7XJ",  // MacBookPro5,4,
  "W8035TG966D",  // MacBookPro5,5,
  "C02G5834DC79", // MacBookPro6,1,
  "CK132A91AGW",  // MacBookPro6,2,
  "CK145C7NATM",  // MacBookPro7,1,
  "W89F9196DH2G", // MacBookPro8,1,
  "C02HL0FGDF8X", // MacBookPro8,2,
  "W88F9CDEDF93", // MacBookPro8,3,
  "C02LW984F1G4", // MacBookPro9,1,
  "C02HA041DTY3", // MacBookPro9,2,
  "C02K2HACDKQ1", // MacBookPro10,1,
  "C02K2HACG4N7", // MacBookPro10,2,
  "C02LSHACFH00", // MacBookPro11,1,
  "C02LSHACG86R", // MacBookPro11,2,
  "C02LSHACFR1M", // MacBookPro11,3,
  "C02SNHACG8WN", // MacBookPro11,4,
  "C02LSHACG85Y", // MacBookPro11,5,
  "C02Q51OSH1DP", // MacBookPro12,1,
  "C02SLHACGVC1", // MacBookPro13,1,
  "C02SLHACGYFH", // MacBookPro13,2,
  "C02SLHACGTFN", // MacBookPro13,3,
  "C02TNHACHV29", // MacBookPro14,1,
  "C02TQHACHV2N", // MacBookPro14,2,
  "C02TQHACHTD5", // MacBookPro14,3,
  "W864947A18X",  // MacBookAir1,1,
  "W86494769A7",  // MacBookAir2,1,
  "C02FLHACD0QX", // MacBookAir3,1,
  "C02DRHACDDR3", // MacBookAir3,2,
  "C02KGHACDRV9", // MacBookAir4,1,
  "C02GLHACDJWT", // MacBookAir4,2,
  "C02J6HACDRV6", // MacBookAir5,1,
  "C02HA041DRVC", // MacBookAir5,2,
  "C02KTHACF5NT", // MacBookAir6,1,
  "C02HACKUF5V7", // MacBookAir6,2,
  "C02PCLGFH569", // MacBookAir7,1,
  "C02Q1HACG940", // MacBookAir7,2,
  "W8702N1JU35",  // Macmini1,1,
  "W8705W9LYL2",  // Macmini2,1,
  "W8905BBE19X",  // Macmini3,1,
  "C02FHBBEDD6H", // Macmini4,1,
  "C07GA041DJD0", // Macmini5,1,
  "C07HVHACDJD1", // Macmini5,2,
  "C07GWHACDKDJ", // Macmini5,3,
  "C07JNHACDY3H", // Macmini6,1,
  "C07JD041DWYN", // Macmini6,2,
  "C02NN7NHG1J0", // Macmini7,1,
  "W8608HACU2P",  // iMac4,1,
  "W8627HACV2H",  // iMac4,2,
  "CK637HACX1A",  // iMac5,1,
  "W8716HACWH5",  // iMac5,2,
  "W8652HACVGN",  // iMac6,1,
  "W8803HACY51",  // iMac7,1,
  "W8755HAC2E2",  // iMac8,1,
  "W89A00A36MJ",  // iMac9,1,
  "W80AA98A5PE",  // iMac10,1,
  "G8942B1V5PJ",  // iMac11,1,
  "W8034342DB7",  // iMac11,2,
  "QP0312PBDNR",  // iMac11,3,
  "W80CF65ADHJF", // iMac12,1,
  "W88GG136DHJQ", // iMac12,2,
  "C02JA041DNCT", // iMac13,1,
  "C02JB041DNCW", // iMac13,2,
  "C02KVHACFFYW", // iMac13,3,
  "D25LHACKF8J2", // iMac14,1,
  "D25LHACKF8JC", // iMac14,2,
  "D25LHACKF8J3", // iMac14,3,
  "D25LHACKFY0T", // iMac14,4,
  "C02Q6HACFY10", // iMac15,1,
  "C02QQHACGF1J", // iMac16,1,
  "C02PNHACGG7G", // iMac16,2,
  "C02QFHACGG7L", // iMac17,1,
  "C02TDHACH7JY", // iMac18,1,
  "C02TDHACJ1G5", // iMac18,2,
  "C02TDHACJ1GJ", // iMac18,3,
  "W88A77AXUPZ",  // MacPro1,1,
  "W8930518UPZ",  // MacPro2,1,
  "W88A77AA5J4",  // MacPro3,1,
  "CT93051DK9Y",  // MacPro4,1,
  "C07J77F7F4MC", // MacPro5,1,  - C07J50F7F4MC  CK04000AHFC  "CG154TB9WU3"
  "F5KLA770F9VM", // MacPro6,1,
  "CK703E1EV2Q",  // Xserve1,1,
  "CK830DLQX8S",  // Xserve2,1,
  "CK933YJ16HS",  // Xserve3,1,
};

//no! ChassisVersion == BoardID
CHAR8* AppleChassisAsset[] =
{
  "MacBook-White",      // MacBook1,1,
  "MacBook-White",      // MacBook2,1,
  "MacBook-White",      // MacBook3,1,
  "MacBook-Black",      // MacBook4,1,
  "MacBook-Black",      // MacBook5,1,
  "MacBook-Black",      // MacBook5,2,
  "MacBook-White",      // MacBook6,1,
  "MacBook-White",      // MacBook7,1,
  "MacBook-Aluminum",   // MacBook8,1,
  "MacBook-Aluminum",   // MacBook9,1,
  "MacBook-Aluminum",   // MacBook10,1,
  "MacBook-Aluminum",   // MacBookPro1,1,
  "MacBook-Aluminum",   // MacBookPro1,2,
  "MacBook-Aluminum",   // MacBookPro2,1,
  "MacBook-Aluminum",   // MacBookPro2,2,
  "MacBook-Aluminum",   // MacBookPro3,1,
  "MacBook-Aluminum",   // MacBookPro4,1,
  "MacBook-Aluminum",   // MacBookPro5,1,
  "MacBook-Aluminum",   // MacBookPro5,2,
  "MacBook-Aluminum",   // MacBookPro5,3,
  "MacBook-Aluminum",   // MacBookPro5,4,
  "MacBook-Aluminum",   // MacBookPro5,5,
  "MacBook-Aluminum",   // MacBookPro6,1,
  "MacBook-Aluminum",   // MacBookPro6,2,
  "MacBook-Aluminum",   // MacBookPro7,1,
  "MacBook-Aluminum",   // MacBookPro8,1,
  "MacBook-Aluminum",   // MacBookPro8,2,
  "MacBook-Aluminum",   // MacBookPro8,3,
  "MacBook-Aluminum",   // MacBookPro9,1,
  "MacBook-Aluminum",   // MacBookPro9,2,
  "MacBook-Aluminum",   // MacBookPro10,1,
  "MacBook-Aluminum",   // MacBookPro10,2,
  "MacBook-Aluminum",   // MacBookPro11,1,
  "MacBook-Aluminum",   // MacBookPro11,2,
  "MacBook-Aluminum",   // MacBookPro11,3,
  "MacBook-Aluminum",   // MacBookPro11,4,
  "MacBook-Aluminum",   // MacBookPro11,5,
  "MacBook-Aluminum",   // MacBookPro12,1,
  "MacBook-Aluminum",   // MacBookPro13,1,
  "MacBook-Aluminum",   // MacBookPro13,2,
  "MacBook-Aluminum",   // MacBookPro13,3,
  "MacBook-Aluminum",   // MacBookPro14,1,
  "MacBook-Aluminum",   // MacBookPro14,2,
  "MacBook-Aluminum",   // MacBookPro14,3,
  "Air-Enclosure",      // MacBookAir1,1,
  "Air-Enclosure",      // MacBookAir2,1,
  "Air-Enclosure",      // MacBookAir3,1,
  "Air-Enclosure",      // MacBookAir3,2,
  "Air-Enclosure",      // MacBookAir4,1,
  "Air-Enclosure",      // MacBookAir4,2,
  "Air-Enclosure",      // MacBookAir5,1,
  "Air-Enclosure",      // MacBookAir5,2, 
  "Air-Enclosure",      // MacBookAir6,1,
  "Air-Enclosure",      // MacBookAir6,2,
  "Air-Enclosure",      // MacBookAir7,1,
  "Air-Enclosure",      // MacBookAir7,2,
  "Mini-Aluminum",      // Macmini1,1,
  "Mini-Aluminum",      // Macmini2,1,
  "Mini-Aluminum",      // Macmini3,1,
  "Mini-Aluminum",      // Macmini4,1,
  "Mini-Aluminum",      // Macmini5,1,
  "Mini-Aluminum",      // Macmini5,2,
  "Mini-Aluminum",      // Macmini5,3,
  "Mini-Aluminum",      // Macmini6,1,
  "Mini-Aluminum",      // Macmini6,2,
  "Mini-Aluminum",      // Macmini7,1,
  "iMac",               // iMac4,1,
  "iMac",               // iMac4,2,
  "iMac",               // iMac5,1,
  "iMac",               // iMac5,2,
  "iMac",               // iMac6,1,
  "iMac-Aluminum",      // iMac7,1,
  "iMac-Aluminum",      // iMac8,1,
  "iMac-Aluminum",      // iMac9,1,
  "iMac-Aluminum",      // iMac10,1,
  "iMac-Aluminum",      // iMac11,1,
  "iMac-Aluminum",      // iMac11,2,
  "iMac-Aluminum",      // iMac11,3,
  "iMac-Aluminum",      // iMac12,1,
  "iMac-Aluminum",      // iMac12,2,
  "iMac-Aluminum",      // iMac13,1,
  "iMac-Aluminum",      // iMac13,2,
  "iMac-Aluminum",      // iMac13,3,
  "iMac-Aluminum",      // iMac14,1,
  "iMac-Aluminum",      // iMac14,2,
  "iMac-Aluminum",      // iMac14,3,
  "iMac-Aluminum",      // iMac14,4,
  "iMac-Aluminum",      // iMac15,1,
  "iMac-Aluminum",      // iMac16,1,
  "iMac-Aluminum",      // iMac16,2,
  "iMac-Aluminum",      // iMac17,1,
  "iMac-Aluminum",      // iMac18,1,
  "iMac-Aluminum",      // iMac18,2,
  "iMac-Aluminum",      // iMac18,3,
  "Pro-Enclosure",      // MacPro1,1,
  "Pro-Enclosure",      // MacPro2,1,
  "Pro-Enclosure",      // MacPro3,1,
  "Pro-Enclosure",      // MacPro4,1,
  "Pro-Enclosure",      // MacPro5,1,
  "Pro-Enclosure",      // MacPro6,1,
  "Xserve",             // Xserve1,1,
  "Xserve",             // Xserve2,1,
  "Xserve",             // Xserve3,1,
};

//REV
UINT8 SmcRevision[][6] =
{
  { 0x01, 0x04, 0x0F, 0, 0, 0x12 },   // MacBook1,1,
  { 0x01, 0x13, 0x0F, 0, 0, 0x03 },   // MacBook2,1,
  { 0x01, 0x24, 0x0F, 0, 0, 0x03 },   // MacBook3,1,
  { 0x01, 0x31, 0x0F, 0, 0, 0x01 },   // MacBook4,1,
  { 0x01, 0x32, 0x0F, 0, 0, 0x08 },   // MacBook5,1,
  { 0x01, 0x38, 0x0F, 0, 0, 0x05 },   // MacBook5,2,
  { 0x01, 0x51, 0x0F, 0, 0, 0x53 },   // MacBook6,1,
  { 0x01, 0x60, 0x0F, 0, 0, 0x06 },   // MacBook7,1,
  { 0x02, 0x25, 0x0F, 0, 0, 0x87 },   // MacBook8,1,
  { 0x02, 0x35, 0x0F, 0, 1, 0x05 },   // MacBook9,1,
  { 0x02, 0x24, 0x0F, 0, 0, 0x10 },   // MacBook10,1,
  { 0x01, 0x02, 0x0F, 0, 0, 0x10 },   // MacBookPro1,1,
  { 0x01, 0x05, 0x0F, 0, 0, 0x10 },   // MacBookPro1,2,
  { 0x01, 0x14, 0x0F, 0, 0, 0x05 },   // MacBookPro2,1,
  { 0x01, 0x13, 0x0F, 0, 0, 0x03 },   // MacBookPro2,2,
  { 0x01, 0x16, 0x0F, 0, 0, 0x11 },   // MacBookPro3,1,
  { 0x01, 0x27, 0x0F, 0, 0, 0x03 },   // MacBookPro4,1,
  { 0x01, 0x33, 0x0F, 0, 0, 0x08 },   // MacBookPro5,1,
  { 0x01, 0x42, 0x0F, 0, 0, 0x04 },   // MacBookPro5,2,
  { 0x01, 0x48, 0x0F, 0, 0, 0x02 },   // MacBookPro5,3,
  { 0x01, 0x49, 0x0F, 0, 0, 0x02 },   // MacBookPro5,4,
  { 0x01, 0x47, 0x0F, 0, 0, 0x02 },   // MacBookPro5,5,
  { 0x01, 0x57, 0x0F, 0, 0, 0x18 },   // MacBookPro6,1,
  { 0x01, 0x58, 0x0F, 0, 0, 0x17 },   // MacBookPro6,2,
  { 0x01, 0x62, 0x0F, 0, 0, 0x07 },   // MacBookPro7,1,
  { 0x01, 0x68, 0x0F, 0, 0, 0x99 },   // MacBookPro8,1,
  { 0x01, 0x69, 0x0F, 0, 0, 0x04 },   // MacBookPro8,2,
  { 0x01, 0x70, 0x0F, 0, 0, 0x06 },   // MacBookPro8,3,
  { 0x02, 0x01, 0x0F, 0, 1, 0x75 },   // MacBookPro9,1,
  { 0x02, 0x02, 0x0F, 0, 0, 0x44 },   // MacBookPro9,2,
  { 0x02, 0x03, 0x0F, 0, 0, 0x36 },   // MacBookPro10,1,
  { 0x02, 0x06, 0x0F, 0, 0, 0x59 },   // MacBookPro10,2,
  { 0x02, 0x16, 0x0F, 0, 0, 0x68 },   // MacBookPro11,1,
  { 0x02, 0x18, 0x0F, 0, 0, 0x15 },   // MacBookPro11,2,
  { 0x02, 0x19, 0x0F, 0, 0, 0x12 },   // MacBookPro11,3,
  { 0x02, 0x29, 0x0F, 0, 0, 0x23 },   // MacBookPro11,4,
  { 0x02, 0x30, 0x0F, 0, 0, 0x02 },   // MacBookPro11,5,
  { 0x02, 0x28, 0x0F, 0, 0, 0x07 },   // MacBookPro12,1,
  { 0x02, 0x36, 0x0F, 0, 0, 0x97 },   // MacBookPro13,1,
  { 0x02, 0x37, 0x0F, 0, 0, 0x20 },   // MacBookPro13,2,
  { 0x02, 0x38, 0x0F, 0, 0, 0x07 },   // MacBookPro13,3,
  { 0x02, 0x43, 0x0F, 0, 0, 0x06 },   // MacBookPro14,1,
  { 0x02, 0x44, 0x0F, 0, 0, 0x01 },   // MacBookPro14,2,
  { 0x02, 0x45, 0x0F, 0, 0, 0x00 },   // MacBookPro14,3,
  { 0x01, 0x23, 0x0F, 0, 0, 0x20 },   // MacBookAir1,1,
  { 0x01, 0x34, 0x0F, 0, 0, 0x08 },   // MacBookAir2,1,
  { 0x01, 0x67, 0x0F, 0, 0, 0x10 },   // MacBookAir3,1,
  { 0x01, 0x66, 0x0F, 0, 0, 0x61 },   // MacBookAir3,2,
  { 0x01, 0x74, 0x0F, 0, 0, 0x04 },   // MacBookAir4,1,
  { 0x01, 0x73, 0x0F, 0, 0, 0x66 },   // MacBookAir4,2,
  { 0x02, 0x04, 0x0F, 0, 0, 0x19 },   // MacBookAir5,1,
  { 0x02, 0x05, 0x0F, 0, 0, 0x09 },   // MacBookAir5,2,
  { 0x02, 0x12, 0x0F, 0, 1, 0x43 },   // MacBookAir6,1,
  { 0x02, 0x13, 0x0F, 0, 0, 0x15 },   // MacBookAir6,2,
  { 0x02, 0x26, 0x0F, 0, 0, 0x02 },   // MacBookAir7,1,
  { 0x02, 0x27, 0x0F, 0, 0, 0x02 },   // MacBookAir7,2,
  { 0x01, 0x03, 0x0F, 0, 0, 0x04 },   // Macmini1,1,
  { 0x01, 0x19, 0x0F, 0, 0, 0x02 },   // Macmini2,1,
  { 0x01, 0x35, 0x0F, 0, 0, 0x01 },   // Macmini3,1,
  { 0x01, 0x65, 0x0F, 0, 0, 0x02 },   // Macmini4,1,
  { 0x01, 0x30, 0x0F, 0, 0, 0x03 },   // Macmini5,1,
  { 0x01, 0x75, 0x0F, 0, 0, 0x00 },   // Macmini5,2,
  { 0x01, 0x77, 0x0F, 0, 0, 0x00 },   // Macmini5,3,
  { 0x02, 0x07, 0x0F, 0, 0, 0x00 },   // Macmini6,1,
  { 0x02, 0x08, 0x0F, 0, 0, 0x01 },   // Macmini6,2,
  { 0x02, 0x24, 0x0F, 0, 0, 0x32 },   // Macmini7,1,
  { 0x01, 0x01, 0x0F, 0, 0, 0x05 },   // iMac4,1,
  { 0x01, 0x06, 0x0F, 0, 0, 0x00 },   // iMac4,2,
  { 0x01, 0x08, 0x0F, 0, 0, 0x02 },   // iMac5,1,
  { 0x01, 0x06, 0x0F, 0, 0, 0x00 },   // iMac5,2,
  { 0x01, 0x08, 0x0F, 0, 0, 0x02 },   // iMac6,1,
  { 0x01, 0x20, 0x0F, 0, 0, 0x04 },   // iMac7,1,
  { 0x01, 0x29, 0x0F, 0, 0, 0x01 },   // iMac8,1,
  { 0x01, 0x36, 0x0F, 0, 0, 0x03 },   // iMac9,1,
  { 0x01, 0x53, 0x0F, 0, 0, 0x13 },   // iMac10,1,
  { 0x01, 0x54, 0x0F, 0, 0, 0x36 },   // iMac11,1,
  { 0x01, 0x64, 0x0F, 0, 0, 0x05 },   // iMac11,2,
  { 0x01, 0x59, 0x0F, 0, 0, 0x02 },   // iMac11,3,
  { 0x01, 0x71, 0x0F, 0, 0, 0x22 },   // iMac12,1,
  { 0x01, 0x72, 0x0F, 0, 0, 0x02 },   // iMac12,2,
  { 0x02, 0x09, 0x0F, 0, 0, 0x05 },   // iMac13,1,
  { 0x02, 0x11, 0x0F, 0, 0, 0x16 },   // iMac13,2,
  { 0x02, 0x13, 0x0F, 0, 0, 0x15 },   // iMac13,3,
  { 0x02, 0x14, 0x0F, 0, 0, 0x24 },   // iMac14,1,
  { 0x02, 0x15, 0x0F, 0, 0, 0x07 },   // iMac14,2,
  { 0x02, 0x17, 0x0F, 0, 0, 0x07 },   // iMac14,3,
  { 0x02, 0x21, 0x0F, 0, 0, 0x92 },   // iMac14,4,
  { 0x02, 0x22, 0x0F, 0, 0, 0x16 },   // iMac15,1,
  { 0x02, 0x31, 0x0F, 0, 0, 0x36 },   // iMac16,1,
  { 0x02, 0x32, 0x0F, 0, 0, 0x20 },   // iMac16,2,
  { 0x02, 0x33, 0x0F, 0, 0, 0x10 },   // iMac17,1,  //i5 or { 0x02, 0x34, 0x0F, 0, 0, 0x02 }, for i7
  { 0x02, 0x39, 0x0F, 0, 0, 0x06 },   // iMac18,1,
  { 0x02, 0x40, 0x0F, 0, 0, 0x00 },   // iMac18,2,
  { 0x02, 0x41, 0x0F, 0, 0, 0x01 },   // iMac18,3,
  { 0x01, 0x07, 0x0F, 0, 0, 0x10 },   // MacPro1,1,
  { 0x01, 0x15, 0x0F, 0, 0, 0x03 },   // MacPro2,1,
  { 0x01, 0x30, 0x0F, 0, 0, 0x03 },   // MacPro3,1,
  { 0x01, 0x39, 0x0F, 0, 0, 0x05 },   // MacPro4,1,
  { 0x01, 0x39, 0x0F, 0, 0, 0x11 },   // MacPro5,1,
  { 0x02, 0x20, 0x0F, 0, 0, 0x18 },   // MacPro6,1,
  { 0x01, 0x11, 0x0F, 0, 0, 0x05 },   // Xserve1,1,
  { 0x01, 0x26, 0x0F, 0, 0, 0x03 },   // Xserve2,1,
  { 0x01, 0x43, 0x0F, 0, 0, 0x04 },   // Xserve3,1,
};

//TODO - find more information and correct all SMC arrays
//RBr
CHAR8* SmcBranch[] =
{
  "branch",    // MacBook1,1,
  "branch",    // MacBook2,1,
  "branch",    // MacBook3,1,
  "branch",    // MacBook4,1,
  "branch",    // MacBook5,1,
  "branch",    // MacBook5,2,
  "NA",        // MacBook6,1,      // need to find RBr key
  "k87",       // MacBook7,1,
  "j92",       // MacBook8,1,
  "j93",       // MacBook9,1,
  "j122",      // MacBook10,1,
  "m1",        // MacBookPro1,1,
  "NA",        // MacBookPro1,2,   // need to find RBr key
  "NA",        // MacBookPro2,1,   // need to find RBr key
  "NA",        // MacBookPro2,2,   // need to find RBr key
  "NA",        // MacBookPro3,1,   // need to find RBr key
  "m87",       // MacBookPro4,1,
  "m98",       // MacBookPro5,1,
  "NA",        // MacBookPro5,2,   // need to find RBr key
  "NA",        // MacBookPro5,3,   // need to find RBr key
  "NA",        // MacBookPro5,4,   // need to find RBr key
  "NA",        // MacBookPro5,5,   // need to find RBr key
  "k17",       // MacBookPro6,1,
  "k74",       // MacBookPro6,2,
  "branch",    // MacBookPro7,1,
  "k90i",      // MacBookPro8,1,
  "trunk",     // MacBookPro8,2,
  "k92",       // MacBookPro8,3,
  "j31",       // MacBookPro9,1,
  "branch",    // MacBookPro9,2,
  "d2",        // MacBookPro10,1,
  "branch",    // MacBookPro10,2,
  "j44",       // MacBookPro11,1,
  "j45",       // MacBookPro11,2,
  "j45g",      // MacBookPro11,3,
  "NA",        // MacBookPro11,4,  // need to find RBr key
  "NA",        // MacBookPro11,5,  // need to find RBr key
  "j52",       // MacBookPro12,1,
  "2016mb",    // MacBookPro13,1,
  "2016mb",    // MacBookPro13,2,
  "2016mb",    // MacBookPro13,3,
  "2017mbp",   // MacBookPro14,1,
  "2017mbp",   // MacBookPro14,2,
  "2017mbp",   // MacBookPro14,3,
  "NA",        // MacBookAir1,1,   // need to find RBr key
  "NA",        // MacBookAir2,1,   // need to find RBr key
  "k16k99",    // MacBookAir3,1,
  "NA",        // MacBookAir3,2,   // need to find RBr key
  "k21k78",    // MacBookAir4,1,
  "k21k78",    // MacBookAir4,2,
  "j11j13",    // MacBookAir5,1,
  "j11j13",    // MacBookAir5,2,
  "j41j43",    // MacBookAir6,1,
  "j41j43",    // MacBookAir6,2,
  "j110",      // MacBookAir7,1,
  "j113",      // MacBookAir7,2,
  "m40",       // Macmini1,1,
  "NA",        // Macmini2,1,      // need to find RBr key
  "NA",        // Macmini3,1,      // need to find RBr key
  "NA",        // Macmini4,1,      // need to find RBr key
  "NA",        // Macmini5,1,      // need to find RBr key
  "NA",        // Macmini5,2,      // need to find RBr key
  "NA",        // Macmini5,3,      // need to find RBr key
  "NA",        // Macmini6,1,      // need to find RBr key
  "j50s",      // Macmini6,2,
  "j64",       // Macmini7,1,
  "m38m39",    // iMac4,1,
  "NA",        // iMac4,2,         // need to find RBr key
  "NA",        // iMac5,1,         // need to find RBr key
  "NA",        // iMac5,2,         // need to find RBr key
  "NA",        // iMac6,1,         // need to find RBr key
  "NA",        // iMac7,1,         // need to find RBr key
  "k3",        // iMac8,1,
  "NA",        // iMac9,1,         // need to find RBr key
  "k22k23",    // iMac10,1,
  "k23f",      // iMac11,1,
  "k74",       // iMac11,2,
  "k74",       // iMac11,3,
  "k60",       // iMac12,1,
  "k62",       // iMac12,2,
  "d8",        // iMac13,1,
  "d8",        // iMac13,2,
  "d8",        // iMac13,3,        // need to find RBr key
  "j16j17",    // iMac14,1,
  "j16j17",    // iMac14,2,
  "j16g",      // iMac14,3,
  "j70",       // iMac14,4,
  "j78j78am",  // iMac15,1,
  "j117",      // iMac16,1,
  "j94",       // iMac16,2,
  "j95j95am",  // iMac17,1,
  "j133_4_5",  // iMac18,1,        // need to find RBr key
  "j133_4_5",  // iMac18,2,        // need to find RBr key
  "j133_4_5",  // iMac18,3,
  "m43",       // MacPro1,1,
  "m43a",      // MacPro2,1,
  "m86",       // MacPro3,1,
  "NA",        // MacPro4,1,       // need to find RBr key
  "k5",        // MacPro5,1,
  "j90",       // MacPro6,1,
  "NA",        // Xserve1,1,       // need to find RBr key
  "NA",        // Xserve2,1,       // need to find RBr key
  "NA",        // Xserve3,1,       // need to find RBr key
};

//RPlt
CHAR8* SmcPlatform[] =
{
  "m70",  // MacBook1,1,
  "m75",  // MacBook2,1,
  "k36",  // MacBook3,1,
  "m82",  // MacBook4,1,
  "m97",  // MacBook5,1,
  "k36b", // MacBook5,2,
  "NA",   // MacBook6,1,      // need to find RPlt key
  "k87",  // MacBook7,1,
  "j92",  // MacBook8,1,
  "j93",  // MacBook9,1,
  "j122", // MacBook10,1,
  "m1",   // MacBookPro1,1,
  "NA",   // MacBookPro1,2,   // need to find RPlt key
  "NA",   // MacBookPro2,1,   // need to find RPlt key
  "NA",   // MacBookPro2,2,   // need to find RPlt key
  "NA",   // MacBookPro3,1,   // need to find RPlt key
  "m87",  // MacBookPro4,1,
  "m98",  // MacBookPro5,1,
  "NA",   // MacBookPro5,2,   // need to find RPlt key
  "NA",   // MacBookPro5,3,   // need to find RPlt key
  "NA",   // MacBookPro5,4,   // need to find RPlt key
  "NA",   // MacBookPro5,5,   // need to find RPlt key
  "k17",  // MacBookPro6,1,
  "k74",  // MacBookPro6,2,
  "k6",   // MacBookPro7,1,
  "k90i", // MacBookPro8,1,
  "k91f", // MacBookPro8,2,
  "k92",  // MacBookPro8,3,
  "j31",  // MacBookPro9,1,
  "j30",  // MacBookPro9,2,
  "d2",   // MacBookPro10,1,
  "d1",   // MacBookPro10,2,
  "j44",  // MacBookPro11,1,
  "j45",  // MacBookPro11,2,
  "j45g", // MacBookPro11,3,
  "NA",   // MacBookPro11,4,  // need to find RPlt key
  "NA",   // MacBookPro11,5,  // need to find RPlt key
  "j52",  // MacBookPro12,1,
  "j130", // MacBookPro13,1,
  "j79",  // MacBookPro13,2,
  "j80g", // MacBookPro13,3,
  "j130a",// MacBookPro14,1,
  "j79a", // MacBookPro14,2,
  "j80ga",// MacBookPro14,3,
  "NA",   // MacBookAir1,1,   // need to find RPlt key
  "NA",   // MacBookAir2,1,   // need to find RPlt key
  "k99",  // MacBookAir3,1,
  "NA",   // MacBookAir3,2,   // need to find RPlt key
  "k78",  // MacBookAir4,1,
  "k21",  // MacBookAir4,2,
  "j11",  // MacBookAir5,1,
  "j13",  // MacBookAir5,2,
  "j41",  // MacBookAir6,1,
  "j43",  // MacBookAir6,2,
  "j110", // MacBookAir7,1,
  "j113", // MacBookAir7,2,
  "m40",  // Macmini1,1,
  "NA",   // Macmini2,1,      // need to find RPlt key
  "NA",   // Macmini3,1,      // need to find RPlt key
  "NA",   // Macmini4,1,      // need to find RPlt key
  "NA",   // Macmini5,1,      // need to find RPlt key
  "NA",   // Macmini5,2,      // need to find RPlt key
  "NA",   // Macmini5,3,      // need to find RPlt key
  "NA",   // Macmini6,1,      // need to find RPlt key
  "j50s", // Macmini6,2,
  "j64",  // Macmini7,1,
  "m38",  // iMac4,1,
  "NA",   // iMac4,2,         // need to find RPlt key
  "NA",   // iMac5,1,         // need to find RPlt key
  "NA",   // iMac5,2,         // need to find RPlt key
  "NA",   // iMac6,1,         // need to find RPlt key
  "NA",   // iMac7,1,         // need to find RPlt key
  "k3",   // iMac8,1,
  "NA",   // iMac9,1,         // need to find RPlt key
  "k23",  // iMac10,1,
  "k23f", // iMac11,1,
  "k74",  // iMac11,2,
  "k74",  // iMac11,3,
  "k60",  // iMac12,1,
  "k62",  // iMac12,2,
  "d8",   // iMac13,1,
  "d8",   // iMac13,2,
  "d8",   // iMac13,3,        // need to find RPlt key
  "j16",  // iMac14,1,
  "j17",  // iMac14,2,
  "j16g", // iMac14,3,
  "j70",  // iMac14,4,
  "j78",  // iMac15,1,
  "j117", // iMac16,1,
  "j94",  // iMac16,2,
  "j95",  // iMac17,1,
  "NA",   // iMac18,1,        // need to find RPlt key
  "NA",   // iMac18,2,        // need to find RPlt key
  "j135", // iMac18,3,
  "m43",  // MacPro1,1,
  "m43a", // MacPro2,1,
  "m86",  // MacPro3,1,
  "NA",   // MacPro4,1,       // need to find RPlt key
  "k5",   // MacPro5,1,
  "j90",  // MacPro6,1,
  "NA",   // Xserve1,1,       // need to find RPlt key
  "NA",   // Xserve2,1,       // need to find RPlt key
  "NA",   // Xserve3,1,       // need to find RPlt key
};

//EPCI
UINT32 SmcConfig[] =
{
  0x71001,  //"MacBook1,1",
  0x72001,  //"MacBook2,1",
  0x72001,  //"MacBook3,1",      // need to find EPCI key
  0x74001,  //"MacBook4,1",
  0x7a002,  //"MacBook5,1",      // need to find EPCI key
  0x7a002,  //"MacBook5,2",
  0x7a002,  //"MacBook6,1",      // need to find EPCI key
  0x7a002,  //"MacBook7,1",      // need to find EPCI key
  0xf0e007, //"MacBook8,1",
  0xf0e007, //"MacBook9,1",      // need to find EPCI key
  0xf08009, //"MacBook10,1",
  0x7b002,  //"MacBookPro1,1",   // need to find EPCI key
  0x7b002,  //"MacBookPro1,2",   // need to find EPCI key
  0x7b002,  //"MacBookPro2,1",   // need to find EPCI key
  0x7b002,  //"MacBookPro2,2",   // need to find EPCI key
  0x7b002,  //"MacBookPro3,1",   // need to find EPCI key
  0x7b002,  //"MacBookPro4,1",   // need to find EPCI key
  0x7b002,  //"MacBookPro5,1",
  0x7b002,  //"MacBookPro5,2",   // need to find EPCI key
  0x7b002,  //"MacBookPro5,3",   // need to find EPCI key
  0x7b002,  //"MacBookPro5,4",   // need to find EPCI key
  0x7b002,  //"MacBookPro5,5",   // need to find EPCI key
  0x7a004,  //"MacBookPro6,1",   // need to find EPCI key
  0x7a004,  //"MacBookPro6,2",
  0x7a004,  //"MacBookPro7,1",   // need to find EPCI key
  0x7b005,  //"MacBookPro8,1",
  0x7b005,  //"MacBookPro8,2",   // need to find EPCI key
  0x7c005,  //"MacBookPro8,3",
  0x76006,  //"MacBookPro9,1",   // need to find EPCI key
  0x76006,  //"MacBookPro9,2",
  0x74006,  //"MacBookPro10,1",
  0x73007,  //"MacBookPro10,2",
  0xf0b007, //"MacBookPro11,1",
  0xf0b007, //"MacBookPro11,2",  // need to find EPCI key
  0xf0b007, //"MacBookPro11,3",  // need to find EPCI key
  0xf0b007, //"MacBookPro11,4",  // need to find EPCI key
  0xf0b007, //"MacBookPro11,5",  // need to find EPCI key
  0xf01008, //"MacBookPro12,1",
  0xf02009, //"MacBookPro13,1",  // need to find EPCI key
  0xf02009, //"MacBookPro13,2",
  0xf02009, //"MacBookPro13,3",  // need to find EPCI key
  0xf0b009, //"MacBookPro14,1",
  0xf09009, //"MacBookPro14,2",
  0xf0a009, //"MacBookPro14,3",
  0x76005,  //"MacBookAir1,1",   // need to find EPCI key
  0x76005,  //"MacBookAir2,1",   // need to find EPCI key
  0x76005,  //"MacBookAir3,1",
  0x76005,  //"MacBookAir3,2",   // need to find EPCI key
  0x76005,  //"MacBookAir4,1",   // need to find EPCI key
  0x76005,  //"MacBookAir4,2",   // need to find EPCI key
  0x7b006,  //"MacBookAir5,1",   // need to find EPCI key
  0x7b006,  //"MacBookAir5,2",
  0x7b007,  //"MacBookAir6,1",   // need to find EPCI key
  0x7b007,  //"MacBookAir6,2",
  0x7b007,  //"MacBookAir7,1",
  0x7b007,  //"MacBookAir7,2",   // need to find EPCI key
  0x78002,  //"Macmini1,1",      // need to find EPCI key
  0x78002,  //"Macmini2,1",
  0x78002,  //"Macmini3,1",      // need to find EPCI key
  0x78002,  //"Macmini4,1",      // need to find EPCI key
  0x7d005,  //"Macmini5,1",
  0x7d005,  //"Macmini5,2",      // need to find EPCI key
  0x7d005,  //"Macmini5,3",      // need to find EPCI key
  0x7d006,  //"Macmini6,1",      // need to find EPCI key
  0x7d006,  //"Macmini6,2",
  0xf04008, //"Macmini7,1",
  0x73002,  //"iMac4,1",         // need to find EPCI key
  0x73002,  //"iMac4,2",         // need to find EPCI key
  0x73002,  //"iMac5,1",         // need to find EPCI key
  0x73002,  //"iMac5,2",         // need to find EPCI key
  0x73002,  //"iMac6,1",         // need to find EPCI key
  0x73002,  //"iMac7,1",         // need to find EPCI key
  0x73002,  //"iMac8,1",
  0x73002,  //"iMac9,1",         // need to find EPCI key
  0x7b002,  //"iMac10,1",
  0x7b004,  //"iMac11,1",
  0x7c004,  //"iMac11,2",
  0x7d004,  //"iMac11,3",
  0x73005,  //"iMac12,1",
  0x75005,  //"iMac12,2",
  0x78006,  //"iMac13,1",
  0x79006,  //"iMac13,2",
  0x79006,  //"iMac13,3",        // need to find EPCI key
  0x79007,  //"iMac14,1",
  0x7a007,  //"iMac14,2",
  0x7a007,  //"iMac14,3",        // need to find EPCI key
  0x7a007,  //"iMac14,4",        // need to find EPCI key
  0xf00008, //"iMac15,1",
  0xf00008, //"iMac16,1",        // need to find EPCI key
  0xf00008, //"iMac16,2",        // need to find EPCI key
  0xf0c008, //"iMac17,1",
  0xf07009, //"iMac18,1",        // need to find EPCI key
  0xf07009, //"iMac18,2",        // need to find EPCI key
  0xf07009, //"iMac18,3",
  0x79001,  //"MacPro1,1",       // need to find EPCI key
  0x79001,  //"MacPro2,1",       // need to find EPCI key
  0x79001,  //"MacPro3,1",
  0x7c002,  //"MacPro4,1",
  0x7c002,  //"MacPro5,1",
  0xf0f006, //"MacPro6,1",
  0x79001,  //"Xserve1,1",       // need to find EPCI key
  0x79001,  //"Xserve2,1",       // need to find EPCI key
  0x79001,  //"Xserve3,1",       // need to find EPCI key
};


CHAR8 *AppleBoardSN       = "C02140302D5DMT31M";
CHAR8 *AppleBoardLocation = "Part Component";

VOID
SetDMISettingsForModel (MACHINE_TYPES Model, BOOLEAN Redefine)
{
  CHAR8  *FirmwareVersion;
  CHAR8  *Res1 = AllocateZeroPool (9);
  CHAR8  *Res2 = AllocateZeroPool (11);

  AsciiStrCpyS (gSettings.VendorName, 64,      BiosVendor);
  AsciiStrCpyS (gSettings.RomVersion, 64,      AppleFirmwareVersion[Model]);

  // AppleReleaseDate
  switch (Model) {
    case MacBook11:
    case MacBook21:
    case MacBook31:
    case MacBook41:
    case MacBook51:
    case MacBook52:
    case MacBook61:
    case MacBook71:
    case MacBookPro11:
    case MacBookPro12:
    case MacBookPro21:
    case MacBookPro22:
    case MacBookPro31:
    case MacBookPro41:
    case MacBookPro51:
    case MacBookPro52:
    case MacBookPro53:
    case MacBookPro54:
    case MacBookPro55:
    case MacBookPro61:
    case MacBookPro62:
    case MacBookPro71:
    case MacBookPro81:
    case MacBookPro82:
    case MacBookPro83:
    case MacBookAir11:
    case MacBookAir21:
    case MacBookAir31:
    case MacBookAir32:
    case MacMini11:
    case MacMini21:
    case MacMini31:
    case MacMini41:
    case iMac41:
    case iMac42:
    case iMac51:
    case iMac52:
    case iMac61:
    case iMac71:
    case iMac81:
    case iMac91:
    case iMac101:
    case iMac111:
    case iMac112:
    case iMac113:
    case iMac121:
    case iMac122:
    case MacPro11:
    case MacPro21:
    case MacPro31:
    case MacPro41:
    case MacPro51:
    case Xserve11:
    case Xserve21:
    case Xserve31:
      FirmwareVersion = AppleFirmwareVersion[Model];
      FirmwareVersion += AsciiStrLen (FirmwareVersion);

      while (*FirmwareVersion != '.') {
        FirmwareVersion--;
      }
      AsciiSPrint (Res1, 9, "%c%c/%c%c/%c%c\n", FirmwareVersion[3], FirmwareVersion[4], FirmwareVersion[5], FirmwareVersion[6], FirmwareVersion[1], FirmwareVersion[2]);
      AsciiStrCpyS (gSettings.ReleaseDate, 64,     Res1);
      break;

    default:
      FirmwareVersion = AppleFirmwareVersion[Model];
      FirmwareVersion += AsciiStrLen (FirmwareVersion);

      while (*FirmwareVersion != '.') {
        FirmwareVersion--;
      }
      AsciiSPrint (Res2, 11, "%c%c/%c%c/20%c%c\n", FirmwareVersion[3], FirmwareVersion[4], FirmwareVersion[5], FirmwareVersion[6], FirmwareVersion[1], FirmwareVersion[2]);
      AsciiStrCpyS (gSettings.ReleaseDate, 64,     Res2);
      break;
  }

  AsciiStrCpyS (gSettings.ManufactureName, 64, BiosVendor);
  if (Redefine) {
    AsciiStrCpyS (gSettings.ProductName, 64,   AppleProductName[Model]);
  }
  AsciiStrCpyS (gSettings.VersionNr, 64,       AppleSystemVersion[Model]);
  AsciiStrCpyS (gSettings.SerialNr, 64,        AppleSerialNumber[Model]);
  AsciiStrCpyS (gSettings.FamilyName, 64,      AppleFamilies[Model]);
  AsciiStrCpyS (gSettings.BoardManufactureName, 64, BiosVendor);
  AsciiStrCpyS (gSettings.BoardSerialNumber, 64,    AppleBoardSN);
  AsciiStrCpyS (gSettings.BoardNumber, 64,          AppleBoardID[Model]);
  AsciiStrCpyS (gSettings.BoardVersion, 64,         AppleProductName[Model]);
  AsciiStrCpyS (gSettings.LocationInChassis, 64,    AppleBoardLocation);
  AsciiStrCpyS (gSettings.ChassisManufacturer, 64,  BiosVendor);
  AsciiStrCpyS (gSettings.ChassisAssetTag, 64,      AppleChassisAsset[Model]);

  // Firmware info for High Sierra DP6/DP7
  // by Sherlocks
  // FirmwareFeatures
  switch (Model) {
    // Verified list from Firmware
    case MacBookPro91:
    case MacBookPro92:
      gFwFeatures             = 0xC00DE137;
      break;
    case MacBookAir41:
    case MacBookAir42:
    case MacMini51:
    case MacMini52:
    case MacMini53:
      gFwFeatures             = 0xD00DE137;
      break;
    case MacBookPro101:
    case MacBookPro102:
    case MacBookAir51:
    case MacBookAir52:
    case MacMini61:
    case MacMini62:
    case iMac131:
    case iMac132:
    case iMac133:
      gFwFeatures             = 0xE00DE137;
      break;
    case MacBookAir61:
    case MacBookAir62:
    case iMac141:
    case iMac142:
    case iMac143:
      gFwFeatures             = 0xE00FE137;
      break;
    case MacPro61:
      gFwFeatures             = 0xE80FE136;
      break;
    case MacBookPro111:
    case MacBookPro112:
    case MacBookPro113:
    case MacBookPro114:
    case MacBookPro115:
      gFwFeatures             = 0xE80FE137;
      break;
    case iMac144:
      gFwFeatures             = 0xF00FE137;
      break;
    case iMac151:
      gFwFeatures             = 0xF80FE137;
      break;
    case MacBookPro131:
    case MacBookPro132:
    case MacBookPro141:
    case MacBookPro142:
    case iMac171:
    case iMac181:
    case iMac182:
    case iMac183:
      gFwFeatures             = 0xFC0FE136;
      break;
    case MacBook91:
    case MacBook101:
    case MacBookPro133:
    case MacBookPro143:
      gFwFeatures             = 0xFC0FE13E;
      break;

    // Verified list from Users
    case MacBookPro61:
    case MacBookPro62:
    case MacBookPro71:
    case MacBookPro81:
    case MacBookPro82:
    case MacBookPro83:
      gFwFeatures             = 0xC00DE137;
      break;
    case MacBookAir31:
    case MacBookAir32:
    case MacMini41:
      gFwFeatures             = 0xD00DE137;
      break;
    case iMac101:
    case iMac111:
    case iMac112:
    case iMac113:
    case iMac121:
    case iMac122:
    case MacMini71:
      gFwFeatures             = 0xE00DE137;
      break;
    case MacBookAir71:
    case MacBookAir72:
      gFwFeatures             = 0xE00FE137;
      break;
    case MacPro51:
      gFwFeatures             = 0xE80FE136;
      break;
    case iMac161:
    case iMac162:
    case MacBookPro121:
      gFwFeatures             = 0xFC0FE136;
      break;
    case MacBook61:
    case MacBook71:
    case MacBook81:
      gFwFeatures             = 0xFC0FE13E;
      break;

    default:
      gFwFeatures             = 0xE907F537; //unknown - use oem SMBIOS value to be default
      break;
  }

  // FirmwareFeaturesMask
  switch (Model) {
    // Verified list from Firmware
    case MacBook91:
    case MacBook101:
    case MacBookPro91:
    case MacBookPro92:
    case MacBookPro101:
    case MacBookPro102:
    case MacBookPro111:
    case MacBookPro112:
    case MacBookPro113:
    case MacBookPro114:
    case MacBookPro115:
    case MacBookPro131:
    case MacBookPro132:
    case MacBookPro133:
    case MacBookPro141:
    case MacBookPro142:
    case MacBookPro143:
    case MacBookAir41:
    case MacBookAir42:
    case MacBookAir51:
    case MacBookAir52:
    case MacBookAir61:
    case MacBookAir62:
    case MacMini51:
    case MacMini52:
    case MacMini53:
    case MacMini61:
    case MacMini62:
    case iMac131:
    case iMac132:
    case iMac133:
    case iMac141:
    case iMac142:
    case iMac143:
    case iMac144:
    case iMac151:
    case iMac171:
    case iMac181:
    case iMac182:
    case iMac183:
    case MacPro61:
      gFwFeaturesMask         = 0xFF1FFF3F;
      break;

    // Verified list from Users
    case MacBook61:
    case MacBook71:
    case MacBook81:
    case MacBookPro61:
    case MacBookPro62:
    case MacBookPro71:
    case MacBookPro81:
    case MacBookPro82:
    case MacBookPro83:
    case MacBookPro121:
    case MacBookAir31:
    case MacBookAir32:
    case MacBookAir71:
    case MacBookAir72:
    case iMac101:
    case iMac111:
    case iMac112:
    case iMac113:
    case iMac121:
    case iMac122:
    case iMac161:
    case iMac162:
    case MacMini41:
    case MacMini71:
    case MacPro51:
      gFwFeaturesMask         = 0xFF1FFF3F;
      break;

    default:
      gFwFeaturesMask         = 0xFFFFFFFF; //unknown - use oem SMBIOS value to be default
      break;
  }
  
  // PlatformFeature
  // the memory tab in About This Mac
  // by TheRacerMaster
  switch (Model) {
    case iMac171:
    case iMac181:
    case iMac182:
    case iMac183:
      gPlatformFeature        = 0x00;   //not soldered RAM - memory tab appearing
      break;
    case MacMini61:
    case MacMini62:
    case iMac131:
    case iMac132:
    case iMac133:
    case iMac141:
    case iMac142:
    case iMac143:
    case iMac144:
    case iMac151:
      gPlatformFeature        = 0x01;   //not soldered RAM - memory tab appearing
      break;
    case MacBookPro111:
    case MacBookPro112:
    case MacBookPro113:
    case MacBookPro114:
    case MacBookPro115:
    case MacBookPro121:
      gPlatformFeature        = 0x02;   //soldered RAM - memory tab disappearing
      break;
    case MacMini71:
      gPlatformFeature        = 0x03;   //soldered RAM - memory tab disappearing
      break;
    case MacPro61:
      gPlatformFeature        = 0x04;   //not soldered RAM - memory tab appearing
      break;
    case MacBook81:
    case MacBook91:
    case MacBook101:
    case MacBookPro131:
    case MacBookPro132:
    case MacBookPro133:
    case MacBookPro141:
    case MacBookPro142:
    case MacBookPro143:
      gPlatformFeature        = 0x1A;   //soldered RAM - memory tab disappearing. 0x18 - restoring the memory tab
      break;

    default:
      gPlatformFeature        = 0xFFFF; //disabled - memory tab appearing
      break;
  }

  if (Model >= MacPro31) {
    gSettings.BoardType = BaseBoardTypeProcessorMemoryModule; //0xB;
  } else {
    gSettings.BoardType = BaseBoardTypeMotherBoard; //0xA;
  }

  switch (Model) {
    case MacBook11:
    case MacBook21:
    case MacBook31:
    case MacBook41:
    case MacBook51:
    case MacBook52:
    case MacBook61:
    case MacBook71:
    case MacBookAir11:
    case MacBookAir21:
    case MacBookAir31:
    case MacBookAir32:
    case MacBookAir41:
    case MacBookAir42:
    case MacBookAir51:
    case MacBookAir52:
    case MacBookAir61:
    case MacBookAir62:
    case MacBookPro81:
    case MacBookPro82:
    case MacBookPro83:
    case MacBookPro91:
    case MacBookPro92:
    case MacBookPro101:
    case MacBookPro102:
    case MacBookPro111:
    case MacBookPro112:
    case MacBookPro113:
    case MacBookPro114:
    case MacBookPro115:
    case MacMini71:
      gSettings.ChassisType = MiscChassisTypeNotebook; //0x0A;
      gSettings.Mobile      = TRUE;
      break;

    case MacBook81:
    case MacBook91:
    case MacBook101:
    case MacBookAir71:
    case MacBookAir72:
    case MacBookPro121:
    case MacBookPro131:
    case MacBookPro132:
    case MacBookPro133:
    case MacBookPro141:
    case MacBookPro142:
    case MacBookPro143:
    case iMac161:
    case iMac162:
    case iMac171:
    case iMac181:
    case iMac182:
    case iMac183:
      gSettings.ChassisType = MiscChassisTypeLapTop; //0x09;
      switch (Model) {
        case iMac162:
        case iMac171:
        case iMac182:
        case iMac183:
          gSettings.Mobile      = FALSE;
          break;
        default:
          gSettings.Mobile      = TRUE;
          break;
      }
      break;

    case MacBookPro11:
    case MacBookPro12:
    case MacBookPro21:
    case MacBookPro22:
    case MacBookPro31:
    case MacBookPro41:
    case MacBookPro51:
    case MacBookPro52:
    case MacBookPro53:
    case MacBookPro54:
    case MacBookPro55:
    case MacBookPro61:
    case MacBookPro62:
    case MacBookPro71:
      gSettings.ChassisType = MiscChassisTypePortable; //0x08;
      gSettings.Mobile      = TRUE;
      break;

    case iMac41:
    case iMac42:
    case iMac51:
    case iMac52:
    case iMac61:
    case iMac71:
    case iMac81:
    case iMac91:
    case iMac101:
    case iMac111:
    case iMac112:
    case iMac113:
    case iMac121:
    case iMac122:
    case iMac131:
    case iMac132:
    case iMac133:
    case iMac141:
    case iMac142:
    case iMac143:
    case iMac144:
    case iMac151:
      gSettings.ChassisType = MiscChassisTypeAllInOne; //0x0D;
      if(Model == iMac144) {
          gSettings.Mobile      = TRUE;
          break;
      }
      gSettings.Mobile      = FALSE;
      break;

    case MacMini11:
    case MacMini21:
      gSettings.ChassisType = MiscChassisTypeLowProfileDesktop; //0x04;
      gSettings.Mobile      = FALSE;
      break;

    case MacMini31:
    case MacMini41:
    case MacMini51:
    case MacMini52:
    case MacMini53:
    case MacMini61:
    case MacMini62:
      gSettings.ChassisType = MiscChassisTypeLunchBox; //0x10;
      gSettings.Mobile      = FALSE;
      break;

    case MacPro41:
    case MacPro51:
      gSettings.ChassisType = MiscChassisTypeTower; //0x07;
      gSettings.Mobile      = FALSE;
      break;

    case MacPro11:
    case MacPro21:
    case MacPro31:
    case MacPro61:
      gSettings.ChassisType = MiscChassisTypeUnknown;  //0x02; this is a joke but think different!
      gSettings.Mobile      = FALSE;
      break;
          
    case Xserve11:
    case Xserve21:
    case Xserve31:
      gSettings.ChassisType = MiscChassisTypeRackMountChassis; //0x17;
      gSettings.Mobile      = FALSE;
      break;

    default: //unknown - use oem SMBIOS value to be default
      gSettings.Mobile      = gMobile;
      gSettings.ChassisType = 0; //let SMBIOS value to be
      /*      if (gMobile) {
       gSettings.ChassisType = 10; //notebook
       } else {
       gSettings.ChassisType = MiscChassisTypeDeskTop; //0x03;
       } */
      break;
  }

  //RBr helper
  if (SmcBranch[Model][0] != 'N') {
    AsciiStrCpyS (gSettings.RBr, 8, SmcBranch[Model]);
  } else {
    switch (gCPUStructure.Model) {
      case CPU_MODEL_PENTIUM_M:
      case CPU_MODEL_CELERON:
        AsciiStrCpyS (gSettings.RBr, 8, "m70");
        break;
                
      case CPU_MODEL_YONAH:
        AsciiStrCpyS (gSettings.RBr, 8, "k22");
        break;
                
      case CPU_MODEL_MEROM: //TODO check for mobile
        AsciiStrCpyS (gSettings.RBr, 8, "m75");
        break;
                
      case CPU_MODEL_PENRYN:
        if (gSettings.Mobile) {
          AsciiStrCpyS (gSettings.RBr, 8, "m82");
        } else {
          AsciiStrCpyS (gSettings.RBr, 8, "k36");
        }
        break;
                
      case CPU_MODEL_SANDY_BRIDGE:
        if (gSettings.Mobile) {
          AsciiStrCpyS (gSettings.RBr, 8, "k90i");
        } else {
          AsciiStrCpyS (gSettings.RBr, 8, "k60");
        }
        break;
                
      case CPU_MODEL_IVY_BRIDGE:
        AsciiStrCpyS (gSettings.RBr, 8, "j30");
        break;
                
      case CPU_MODEL_IVY_BRIDGE_E5:
        AsciiStrCpyS (gSettings.RBr, 8, "j90");
        break;
                
      case CPU_MODEL_HASWELL_ULT:
        AsciiStrCpyS (gSettings.RBr, 8, "j44");
        break;
                
      case CPU_MODEL_HASWELL_U5: //Mobile - Broadwell
        AsciiStrCpyS (gSettings.RBr, 8, "j52");
        break;
                
      case CPU_MODEL_SKYLAKE_D:
        AsciiStrCpyS (gSettings.RBr, 8, "j95j95am");
        break;
                
      case CPU_MODEL_SKYLAKE_U:
        AsciiStrCpyS (gSettings.RBr, 8, "2016mb");
        break;
                
      case CPU_MODEL_KABYLAKE1: //Mobile
        AsciiStrCpyS (gSettings.RBr, 8, "2017mbp");
        break;
                
      case CPU_MODEL_KABYLAKE2: //Desktop
        AsciiStrCpyS (gSettings.RBr, 8, "j133_4_5");
        break;
                
      default:
        AsciiStrCpyS (gSettings.RBr, 8, "t9");
        break;
    }
  }

  //RPlt helper
  if (SmcPlatform[Model][0] != 'N') {
    AsciiStrCpyS (gSettings.RPlt, 8, SmcPlatform[Model]);
  } else {
    switch (gCPUStructure.Model) {
      case CPU_MODEL_PENTIUM_M:
      case CPU_MODEL_CELERON:
        AsciiStrCpyS (gSettings.RPlt, 8, "m70");
        break;

      case CPU_MODEL_YONAH:
        AsciiStrCpyS (gSettings.RPlt, 8, "k22");
        break;

      case CPU_MODEL_MEROM: //TODO check for mobile
        AsciiStrCpyS (gSettings.RPlt, 8, "m75");
        break;

      case CPU_MODEL_PENRYN:
        if (gSettings.Mobile) {
          AsciiStrCpyS (gSettings.RPlt, 8, "m82");
        } else {
          AsciiStrCpyS (gSettings.RPlt, 8, "k36");
        }
        break;

      case CPU_MODEL_SANDY_BRIDGE:
        if (gSettings.Mobile) {
          AsciiStrCpyS (gSettings.RPlt, 8, "k90i");
        } else {
          AsciiStrCpyS (gSettings.RPlt, 8, "k60");
        }
        break;

      case CPU_MODEL_IVY_BRIDGE:
        AsciiStrCpyS (gSettings.RPlt, 8, "j30");
        break;

      case CPU_MODEL_IVY_BRIDGE_E5:
        AsciiStrCpyS (gSettings.RPlt, 8, "j90");
        break;

      case CPU_MODEL_HASWELL_ULT:
        AsciiStrCpyS (gSettings.RPlt, 8, "j44");
        break;

      case CPU_MODEL_HASWELL_U5: //Mobile - Broadwell
        AsciiStrCpyS (gSettings.RPlt, 8, "j52");
        break;

      case CPU_MODEL_SKYLAKE_D:
        AsciiStrCpyS (gSettings.RPlt, 8, "j95");
        break;

      case CPU_MODEL_SKYLAKE_U:
        AsciiStrCpyS (gSettings.RPlt, 8, "j79");
        break;

      case CPU_MODEL_KABYLAKE1: //Mobile
        AsciiStrCpyS (gSettings.RPlt, 8, "j130a");
        break;

      case CPU_MODEL_KABYLAKE2: //Desktop
        AsciiStrCpyS (gSettings.RPlt, 8, "j135");
        break;

      default:
        AsciiStrCpyS (gSettings.RPlt, 8, "t9");
        break;
    }
  }
  CopyMem (gSettings.REV,  SmcRevision[Model], 6);
  CopyMem (gSettings.EPCI, &SmcConfig[Model],  4);
}

MACHINE_TYPES
GetModelFromString (
                    CHAR8 *ProductName
                    )
{
  UINTN i;

  for (i = 0; i < MaxMachineType; ++i) {
    if (AsciiStrCmp (AppleProductName[i], ProductName) == 0) {
      return i;
    }
  }
  // return ending enum as "not found"
  return MaxMachineType;
}

VOID
GetDefaultSettings ()
{
  MACHINE_TYPES  Model;
  //UINT64         msr = 0;

  DbgHeader("GetDefaultSettings");

  //gLanguage         = english;
  Model             = GetDefaultModel ();
  gSettings.CpuType	= GetAdvancedCpuType ();

  SetDMISettingsForModel (Model, TRUE);

  //default values will be overritten by config.plist
  //use explicitly settings TRUE or FALSE (Yes or No)

  gSettings.InjectIntel          = (gGraphics[0].Vendor == Intel) || (gGraphics[1].Vendor == Intel);

  gSettings.InjectATI            = (((gGraphics[0].Vendor == Ati) && ((gGraphics[0].DeviceID & 0xF000) != 0x6000)) ||
                                    ((gGraphics[1].Vendor == Ati) && ((gGraphics[1].DeviceID & 0xF000) != 0x6000)));

  gSettings.InjectNVidia         = (((gGraphics[0].Vendor == Nvidia) && (gGraphics[0].Family < 0xE0)) ||
                                    ((gGraphics[1].Vendor == Nvidia) && (gGraphics[1].Family < 0xE0)));

  gSettings.GraphicsInjector     = gSettings.InjectATI || gSettings.InjectNVidia;
  //gSettings.CustomEDID           = NULL; //no sense to assign 0 as the structure is zeroed
  gSettings.DualLink             = 0xA; // A(auto): DualLink auto-detection
  gSettings.HDAInjection         = TRUE;
  //gSettings.HDALayoutId          = 0;
  gSettings.USBInjection         = TRUE; // enabled by default to have the same behavior as before
  StrCpyS (gSettings.DsdtName, 28, L"DSDT.aml");
  gSettings.BacklightLevel       = 0xFFFF; //0x0503; -- the value from MBA52
  gSettings.BacklightLevelConfig = FALSE;
  gSettings.TrustSMBIOS          = TRUE;

  gSettings.SmUUIDConfig         = FALSE;
  gSettings.DefaultBackgroundColor = 0x80000000; //the value to delete the variable
  gSettings.RtROM                = NULL;
  gSettings.RtROMLen             = 0;
  gSettings.CsrActiveConfig      = 0xFFFF;
  gSettings.BooterConfig         = 0;
  gSettings.DisableCloverHotkeys = FALSE;
  
  ResumeFromCoreStorage          = FALSE;

  if (gCPUStructure.Model >= CPU_MODEL_IVY_BRIDGE) {
    gSettings.GeneratePStates    = TRUE;
    gSettings.GenerateCStates    = TRUE;
    //  gSettings.EnableISS          = FALSE;
    //  gSettings.EnableC2           = TRUE;
    gSettings.EnableC6           = TRUE;
    gSettings.PluginType         = 1;

    if (gCPUStructure.Model == CPU_MODEL_IVY_BRIDGE) {
      gSettings.MinMultiplier    = 7;
    }
    //  gSettings.DoubleFirstState   = FALSE;
    gSettings.DropSSDT           = TRUE;
    gSettings.C3Latency          = 0x00FA;
  }

  //gSettings.EnableISS            = FALSE; //((gCPUStructure.CPUID[CPUID_1][ECX] & (1<<7)) != 0);
  gSettings.Turbo                = gCPUStructure.Turbo;
  //MsgLog ("Turbo default value: %a\n", gCPUStructure.Turbo ? "Yes" : "No");
  //msr                            = AsmReadMsr64(MSR_IA32_MISC_ENABLE);
  //force enable EIST
  //msr                            |= (1<<16);
  //AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, msr);
  //gSettings.Turbo                = ((msr & (1ULL<<38)) == 0);
  //gSettings.EnableISS            = ((msr & (1ULL<<16)) != 0);

  //Fill ACPI table list
  //  GetAcpiTablesList ();
}
