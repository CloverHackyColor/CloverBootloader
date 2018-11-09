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
CHAR8   *AppleBoardSN           = "C02140302D5DMT31M";
CHAR8   *AppleBoardLocation     = "Part Component";

UINT32  gFwFeatures;
UINT32  gFwFeaturesMask;
UINT64  gPlatformFeature;

// All SMBIOS data were updated by Sherlocks, PMheart.
// FredWst supported SmcExtract.

// Refactored to single data structure by RehabMan

typedef struct
{
  const CHAR8* productName;
  const CHAR8* firmwareVersion;
  const CHAR8* efiversion;
  const CHAR8* boardID;
  const CHAR8* productFamily;
  const CHAR8* systemVersion;
  const CHAR8* serialNumber;
  const CHAR8* chassisAsset;
  UINT8 smcRevision[6];
  const CHAR8* smcBranch;
  const CHAR8* smcPlatform;
  UINT32 smcConfig;
} PLATFORMDATA;

PLATFORMDATA ApplePlatformData[] =
{
  //MacBook1,1
  { "MacBook1,1", "MB11.88Z.0061.B03.0610121324", NULL, "Mac-F4208CC8", // Intel Core Duo T2500 @ 2.00 GHz
    "MacBook", "1.1", "W80A041AU9B", "MacBook-White",
    { 0x01, 0x04, 0x0f, 0, 0, 0x12 },  "branch", "m70", 0x71001 },
  //MacBook2,1
  { "MacBook2,1", "MB21.88Z.00A5.B07.0706270922", NULL, "Mac-F4208CA9", // Intel Core 2 Duo T7200 @ 2.00 GHz
    "MacBook", "1.2", "W88A041AWGP", "MacBook-White",
    { 0x01, 0x13, 0x0f, 0, 0, 0x03 },  "branch", "m75", 0x72001 },
  //MacBook3,1
  { "MacBook3,1", "MB31.88Z.008E.B02.0803051832", NULL, "Mac-F22788C8", // Intel Core 2 Duo T7500 @ 2.20 GHz
    "MacBook", "1.3", "W8803HACY51", "MacBook-White",
    { 0x01, 0x24, 0x0f, 0, 0, 0x03 },  "branch", "k36", 0x72001 }, // need ECPI
  //MacBook4,1
  { "MacBook4,1", "MB41.88Z.00C1.B00.0802091535", NULL, "Mac-F22788A9", // Intel Core 2 Duo T8300 @ 2.39 GHz
    "MacBook", "1.3", "W88A041A0P0", "MacBook-Black",
    { 0x01, 0x31, 0x0f, 0, 0, 0x01 },  "branch", "m82", 0x74001 },
  //MacBook5,1
  { "MacBook5,1", "MB51.88Z.007D.B03.0904271443", NULL, "Mac-F42D89C8", // Intel Core 2 Duo P7350 @ 2.00 GHz
    "MacBook", "1.3", "W8944T1S1AQ", "MacBook-Black",
    { 0x01, 0x32, 0x0f, 0, 0, 0x08 },  "branch", "m97", 0x7a002 }, // need ECPI
  //MacBook5,2
  { "MacBook5,2", "MB52.88Z.0088.B05.0904162222", NULL, "Mac-F22788AA", // Intel Core 2 Duo P7450 @ 2.13 GHz
    "MacBook", "1.3", "W88AAAAA9GU", "MacBook-Black",
    { 0x01, 0x38, 0x0f, 0, 0, 0x05 },  "branch", "k36b", 0x7a002 },
  //MacBook6,1
  { "MacBook6,1", "MB61.88Z.00CC.B00.1802021826", NULL, "Mac-F22C8AC8", // Intel Core 2 Duo P7550 @ 2.26 GHz
    "MacBook", "1.0", "451131JCGAY", "MacBook-White",
    { 0x01, 0x51, 0x0f, 0, 0, 0x53 },  "NA", "NA", 0x7a002 }, // need rBR RPlt ECPI
  //MacBook7,1
  { "MacBook7,1", "MB71.88Z.003F.B00.1802022149", NULL, "Mac-F22C89C8", // Intel Core 2 Duo P8600 @ 2.40 GHz
    "MacBook", "1.0", "451211MEF5X", "MacBook-White",
    { 0x01, 0x60, 0x0f, 0, 0, 0x06 },  "k87", "k87", 0x7a002 }, // need ECPI
  //MacBook8,1
  { "MacBook8,1", "MB81.88Z.0175.B00.1809171422", "177.0.0.0.0", "Mac-BE0E8AC46FE800CC", // Intel Core M-5Y51 @ 1.20 GHz
    "MacBook", "1.0", "C02RCE58GCN3", "MacBook-Aluminum",
    { 0x02, 0x25, 0x0f, 0, 0, 0x87 },  "j92", "j92", 0xf0e007 },
  //MacBook9,1
  { "MacBook9,1", "MB91.88Z.F000.B00.1809171414", "175.0.0.0.0", "Mac-9AE82516C7C6B903", // Intel Core m5-6Y54 @ 1.20 GHz
    "MacBook", "1.0", "C02RM408HDNK", "MacBook-Aluminum",
    { 0x02, 0x35, 0x0f, 0, 1, 0x05 },  "j93", "j93", 0xf0e007 }, // need ECPI
  //MacBook10,1
  { "MacBook10,1", "MB101.88Z.F000.B00.1809191505", "168.0.0.0.0", "Mac-EE2EBD4B90B839A8", // Intel Core i5-7Y54 @ 1.30 GHz
    "MacBook", "1.0", "C02TQHACHH27", "MacBook-Aluminum",
    { 0x02, 0x24, 0x0f, 0, 0, 0x10 },  "j122", "j122", 0xf08009 }, // need ECPI
  //MacBookPro1,1
  { "MacBookPro1,1", "MBP11.88Z.0055.B08.0610121325", NULL, "Mac-F425BEC8", // Intel Core Duo T2500 @ 2.00 GHz
    "MacBook Pro", "1.0", "W884857JVJ1", "MacBook-Aluminum",
    { 0x01, 0x02, 0x0f, 0, 0, 0x10 },  "m1", "m1", 0x7b002 }, // need ECPI
  //MacBookPro1,2
  { "MacBookPro1,2", "MBP12.88Z.0061.B03.0610121334", NULL, "Mac-F42DBEC8", // Intel Core Duo T2600 @ 2.17 GHz
    "MacBook Pro", "1.0", "W8629HACTHY", "MacBook-Aluminum",
    { 0x01, 0x05, 0x0f, 0, 0, 0x10 },  "NA", "NA", 0x7b002 }, // need rBR RPlt ECPI
  //MacBookPro2,1
  { "MacBookPro2,1", "MBP21.88Z.00A5.B08.0708131242", NULL, "Mac-F42189C8", // Intel Core 2 Duo T7600 @ 2.33 GHz
    "MacBook Pro", "1.0", "W88130WUW0H", "MacBook-Aluminum",
    { 0x01, 0x14, 0x0f, 0, 0, 0x05 },  "NA", "NA", 0x7b002 }, // need rBR RPlt ECPI
  //MacBookPro2,2
  { "MacBookPro2,2", "MBP22.88Z.00A5.B07.0708131242", NULL, "Mac-F42187C8", // Intel Core 2 Duo T7400 @ 2.16 GHz
    "MacBook Pro", "1.0", "W8827B4CW0L", "MacBook-Aluminum",
    { 0x01, 0x13, 0x0f, 0, 0, 0x03 },  "NA", "NA", 0x7b002 }, // need rBR RPlt ECPI
  //MacBookPro3,1
  { "MacBookPro3,1", "MBP31.88Z.0070.B07.0803051658", NULL, "Mac-F4238BC8", // Intel Core 2 Duo T7700 @ 2.40 GHz
    "MacBook Pro", "1.0", "W8841OHZX91", "MacBook-Aluminum",
    { 0x01, 0x16, 0x0f, 0, 0, 0x11 },  "NA", "NA", 0x7b002 }, // need rBR RPlt ECPI
  //MacBookPro4,1
  { "MacBookPro4,1", "MBP41.88Z.00C1.B03.0802271651", NULL, "Mac-F42C89C8", // Intel Core 2 Duo T8300 @ 2.40 GHz
    "MacBook Pro", "1.0", "W88484F2YP4", "MacBook-Aluminum",
    { 0x01, 0x27, 0x0f, 0, 0, 0x03 },  "m87", "m87", 0x7b002 }, // need ECPI
  //MacBookPro5,1
  { "MacBookPro5,1", "MBP51.88Z.007E.B06.1202061253", NULL, "Mac-F42D86C8", // Intel Core 2 Duo P8600 @ 2.40 GHz
    "MacBook Pro", "1.0", "W88439FE1G0", "MacBook-Aluminum",
    { 0x01, 0x33, 0x0f, 0, 0, 0x08 },  "m98", "m98", 0x7b002 },
  //MacBookPro5,2
  { "MacBookPro5,2", "MBP52.88Z.008E.B05.0905042202", NULL, "Mac-F2268EC8", // Intel Core 2 Duo T9600 @ 2.80 GHz
    "MacBook Pro", "1.0", "W8908HAC2QP", "MacBook-Aluminum",
    { 0x01, 0x42, 0x0f, 0, 0, 0x04 },  "NA", "NA", 0x7b002 }, // need rBR RPlt ECPI
  //MacBookPro5,3
  { "MacBookPro5,3", "MBP53.88Z.00AC.B03.0906151647", NULL, "Mac-F22587C8", // Intel Core 2 Duo P8800 @ 2.66 GHz
    "MacBook Pro", "1.0", "W8035TG97XK", "MacBook-Aluminum",
    { 0x01, 0x48, 0x0f, 0, 0, 0x02 },  "NA", "NA", 0x7b002 }, // need rBR RPlt ECPI
  //MacBookPro5,4
  { "MacBookPro5,4", "MBP53.88Z.00AC.B03.0906151647", NULL, "Mac-F22587A1", // Intel Core 2 Duo P8700 @ 2.53 GHz
    "MacBook Pro", "1.0", "W8948HAC7XJ", "MacBook-Aluminum",
    { 0x01, 0x49, 0x0f, 0, 0, 0x02 },  "NA", "NA", 0x7b002 }, // need rBR RPlt ECPI
  //MacBookPro5,5
  { "MacBookPro5,5", "MBP55.88Z.00AC.B03.0906151708", NULL, "Mac-F2268AC8", // Intel Core 2 Duo P7550 @ 2.26 GHz
    "MacBook Pro", "1.0", "W8035TG966D", "MacBook-Aluminum",
    { 0x01, 0x47, 0x0f, 0, 0, 0x02 },  "NA", "NA", 0x7b002 }, // need rBR RPlt ECPI
  //MacBookPro6,1
  { "MacBookPro6,1", "MBP61.88Z.005D.B00.1804100943", NULL, "Mac-F22589C8", // Intel Core i5-540M @ 2.53 GHz
    "MacBook Pro", "1.0", "C02G5834DC79", "MacBook-Aluminum",
    { 0x01, 0x57, 0x0f, 0, 0, 0x18 },  "k17", "k17", 0x7a004 }, // need ECPI
  //MacBookPro6,2
  { "MacBookPro6,2", "MBP61.88Z.005D.B00.1804100943", NULL, "Mac-F22586C8", // Intel Core i7-620M @ 2.66 GHz
    "MacBook Pro", "1.0", "CK132A91AGW", "MacBook-Aluminum",
    { 0x01, 0x58, 0x0f, 0, 0, 0x17 },  "k74", "k74", 0x7a004 },
  //MacBookPro7,1
  { "MacBookPro7,1", "MBP71.88Z.003F.B00.1802021458", NULL, "Mac-F222BEC8", // Intel Core 2 Duo P8600 @ 2.40 GHz
    "MacBook Pro", "1.0", "CK145C7NATM", "MacBook-Aluminum",
    { 0x01, 0x62, 0x0f, 0, 0, 0x07 },  "branch", "k6", 0x7a004 }, // need ECPI
  //MacBookPro8,1
  { "MacBookPro8,1", "MBP81.88Z.0050.B00.1804101331", NULL, "Mac-94245B3640C91C81", // Intel Core i5-2415M @ 2.30 GHz
    "MacBook Pro", "1.0", "W89F9196DH2G", "MacBook-Aluminum",
    { 0x01, 0x68, 0x0f, 0, 0, 0x99 },  "k90i", "k90i", 0x7b005 },
  //MacBookPro8,2
  { "MacBookPro8,2", "MBP81.88Z.0050.B00.1804101331", NULL, "Mac-94245A3940C91C80", // Intel Core i7-2675QM @ 2.20 GHz
    "MacBook Pro", "1.0", "C02HL0FGDF8X", "MacBook-Aluminum",
    { 0x01, 0x69, 0x0f, 0, 0, 0x04 },  "trunk", "k91f", 0x7b005 }, // need ECPI
  //MacBookPro8,3
  { "MacBookPro8,3", "MBP81.88Z.0050.B00.1804101331", NULL, "Mac-942459F5819B171B", // Intel Core i7-2860QM @ 2.49 GHz
    "MacBook Pro", "1.0", "W88F9CDEDF93", "MacBook-Aluminum",
    { 0x01, 0x70, 0x0f, 0, 0, 0x06 },  "k92", "k92", 0x7c005 },
  //MacBookPro9,1
  { "MacBookPro9,1", "MBP91.88Z.F000.B00.1809210851", "222.0.0.0.0", "Mac-4B7AC7E43945597E", // Intel Core i7-3720QM @ 2.60 GHz
    "MacBook Pro", "1.0", "C02LW984F1G4", "MacBook-Aluminum",
    { 0x02, 0x01, 0x0f, 0, 1, 0x75 },  "j31", "j31", 0x76006 }, // need ECPI
  //MacBookPro9,2
  { "MacBookPro9,2", "MBP91.88Z.F000.B00.1809210851", "222.0.0.0.0", "Mac-6F01561E16C75D06", // Intel Core i5-3210M @ 2.50 GHz
    "MacBook Pro", "1.0", "C02HA041DTY3", "MacBook-Aluminum",
    { 0x02, 0x02, 0x0f, 0, 0, 0x44 },  "branch", "j30", 0x76006 },
  //MacBookPro10,1
  { "MacBookPro10,1", "MBP101.88Z.F000.B00.1809210852", "251.0.0.0.0", "Mac-C3EC7CD22292981F", // Intel Core i7-3740QM @ 2.70 GHz
    "MacBook Pro", "1.0", "C02K2HACDKQ1", "MacBook-Aluminum",
    { 0x02, 0x03, 0x0f, 0, 0, 0x36 },  "d2", "d2", 0x74006 },
  //MacBookPro10,2
  { "MacBookPro10,2", "MBP102.88Z.F000.B00.1809171348", "274.0.0.0.0", "Mac-AFD8A9D944EA4843", // Intel Core i5-3230M @ 2.60 GHz
    "MacBook Pro", "1.0", "C02K2HACG4N7", "MacBook-Aluminum",
    { 0x02, 0x06, 0x0f, 0, 0, 0x59 },  "branch", "d1", 0x73007 },
  //MacBookPro11,1
  { "MacBookPro11,1", "MBP111.88Z.0147.B00.1809171520", "149.0.0.0.0", "Mac-189A3D4F975D5FFC", // Intel Core i7-4558U @ 2.80 GHz
    "MacBook Pro", "1.0", "C02LSHACFH00", "MacBook-Aluminum",
    { 0x02, 0x16, 0x0f, 0, 0, 0x68 },  "j44", "j44", 0xf0b007 },
  //MacBookPro11,2
  { "MacBookPro11,2", "MBP112.88Z.0147.B00.1809171519", "149.0.0.0.0", "Mac-3CBD00234E554E41", // Intel Core i7-4750HQ @ 2.00 GHz
    "MacBook Pro", "1.0", "C02LSHACG86R", "MacBook-Aluminum",
    { 0x02, 0x18, 0x0f, 0, 0, 0x15 },  "j45", "j45", 0xf0b007 }, // need ECPI
  //MacBookPro11,3
  { "MacBookPro11,3", "MBP112.88Z.0147.B00.1809171519", "149.0.0.0.0", "Mac-2BD1B31983FE1663", // Intel Core i7-4870HQ @ 2.50 GHz
    "MacBook Pro", "1.0", "C02LSHACFR1M", "MacBook-Aluminum",
    { 0x02, 0x19, 0x0f, 0, 0, 0x12 },  "j45g", "j45g", 0xf0d007 },
  //MacBookPro11,4
  { "MacBookPro11,4", "MBP114.88Z.0185.B00.1809171422", "187.0.0.0.0", "Mac-06F11FD93F0323C5", // Intel Core i7-4770HQ @ 2.20 GHz
    "MacBook Pro", "1.0", "C02SNHACG8WN", "MacBook-Aluminum",
    { 0x02, 0x29, 0x0f, 0, 0, 0x23 },  "NA", "NA", 0xf0b007 }, // need rBR RPlt ECPI
  //MacBookPro11,5
  { "MacBookPro11,5", "MBP114.88Z.0185.B00.1809171422", "187.0.0.0.0", "Mac-06F11F11946D27C5", // Intel Core i7-4870HQ @ 2.50 GHz
    "MacBook Pro", "1.0", "C02LSHACG85Y", "MacBook-Aluminum",
    { 0x02, 0x30, 0x0f, 0, 0, 0x02 },  "NA", "NA", 0xf0b007 }, // need rBR RPlt ECPI
  //MacBookPro12,1
  { "MacBookPro12,1", "MBP121.88Z.0178.B00.1809171422", "180.0.0.0.0", "Mac-E43C1C25D4880AD6", // Intel Core i5-5257U @ 2.70 GHz
    "MacBook Pro", "1.0", "C02Q51OSH1DP", "MacBook-Aluminum",
    { 0x02, 0x28, 0x0f, 0, 0, 0x07 },  "j52", "j52", 0xf01008 },
  //MacBookPro13,1
  { "MacBookPro13,1", "MBP131.88Z.F000.B00.1809171523", "227.0.0.0.0", "Mac-473D31EABEB93F9B", // Intel Core i5-6360U @ 2.00 GHz
    "MacBook Pro", "1.0", "C02SLHACGVC1", "MacBook-Aluminum",
    { 0x02, 0x36, 0x0f, 0, 0, 0x97 },  "2016mb", "j130", 0xf02009 }, // need ECPI
  //MacBookPro13,2
  { "MacBookPro13,2", "MBP132.88Z.F000.B00.1809171523", "250.0.0.0.0", "Mac-66E35819EE2D0D05", // Intel Core i5-6287U @ 3.10 GHz
    "MacBook Pro", "1.0", "C02SLHACGYFH", "MacBook-Aluminum",
    { 0x02, 0x37, 0x0f, 0, 0, 0x20 },  "2016mb", "j79", 0xf02009 },
  //MacBookPro13,3
  { "MacBookPro13,3", "MBP133.88Z.F000.B00.1809171523", "250.0.0.0.0", "Mac-A5C67F76ED83108C", // Intel Core i7-6920HQ @ 2.90 GHz
    "MacBook Pro", "1.0", "C02SLHACGTFN", "MacBook-Aluminum",
    { 0x02, 0x38, 0x0f, 0, 0, 0x07 },  "2016mb", "j80g", 0xf04009 },
  //MacBookPro14,1
  { "MacBookPro14,1", "MBP141.88Z.F000.B00.1809171524", "184.0.0.0.0", "Mac-B4831CEBD52A0C4C", // Intel Core i5-7360U @ 2.30 GHz
    "MacBook Pro", "1.0", "C02TNHACHV29", "MacBook-Aluminum",
    { 0x02, 0x43, 0x0f, 0, 0, 0x06 },  "2017mbp", "j130a", 0xf0b009 },
  //MacBookPro14,2
  { "MacBookPro14,2", "MBP142.88Z.F000.B00.1809171524", "184.0.0.0.0", "Mac-CAD6701F7CEA0921", // Intel Core i5-7267U @ 3.09 GHz
    "MacBook Pro", "1.0", "C02TQHACHV2N", "MacBook-Aluminum",
    { 0x02, 0x44, 0x0f, 0, 0, 0x01 },  "2017mbp", "j79a", 0xf09009 },
  //MacBookPro14,3
  { "MacBookPro14,3", "MBP143.88Z.F000.B00.1809280842", "185.0.0.0.0", "Mac-551B86E5744E2388", // Intel Core i7-7700HQ @ 2.80 GHz
    "MacBook Pro", "1.0", "C02TQHACHTD5", "MacBook-Aluminum",
    { 0x02, 0x45, 0x0f, 0, 0, 0x00 },  "2017mbp", "j80ga", 0xf0a009 },
  //MacBookPro15,1
  { "MacBookPro15,1", "MBP151.88Z.F000.B00.1809280842", "220.200.252.0.0", "Mac-937A206F2EE63C01", // Intel Core i9-8950HK @ 2.90 GHz
    "MacBook Pro", "1.0", "C02X1HACKGYG", "MacBook-Aluminum",
    { 0x02, 0x45, 0x0f, 0, 0, 0x00 },  "2018mbp", "j130b", 0xf0a009 }, // need BIOS REV rBR RPlt ECPI
  //MacBookPro15,2
  { "MacBookPro15,2", "MBP152.88Z.F000.B00.1809171524", "220.200.252.0.0", "Mac-827FB448E656EC26", // Intel Core i5-8259U @ 2.30 GHz
    "MacBook Pro", "1.0", "C02X1HACJHCD", "MacBook-Aluminum",
    { 0x02, 0x44, 0x0f, 0, 0, 0x01 },  "2018mbp", "j79b", 0xf09009 }, // need BIOS REV rBR RPlt ECPI
  //MacBookAir1,1
  { "MacBookAir1,1", "MBA11.88Z.00BB.B03.0803171226", NULL, "Mac-F42C8CC8", // Intel Core 2 Duo P7500 @ 1.60 GHz
    "MacBook Air", "1.0", "W864947A18X", "Air-Enclosure",
    { 0x01, 0x23, 0x0f, 0, 0, 0x20 },  "NA", "NA", 0x76005 }, // need rBR RPlt ECPI
  //MacBookAir2,1
  { "MacBookAir2,1", "MBA21.88Z.0075.B05.1003051506", NULL, "Mac-F42D88C8", // Intel Core 2 Duo L9600 @ 2.13 GHz
    "MacBook Air", "1.0", "W86494769A7", "Air-Enclosure",
    { 0x01, 0x34, 0x0f, 0, 0, 0x08 },  "NA", "NA", 0x76005 }, // need rBR RPlt ECPI
  //MacBookAir3,1
  { "MacBookAir3,1", "MBA31.88Z.0069.B00.1802022340", NULL, "Mac-942452F5819B1C1B", // Intel Core 2 Duo U9400 @ 1.40 GHz
    "MacBook Air", "1.0", "C02FLHACD0QX", "Air-Enclosure",
    { 0x01, 0x67, 0x0f, 0, 0, 0x10 },  "k16k99", "k99", 0x76005 },
  //MacBookAir3,2
  { "MacBookAir3,2", "MBA31.88Z.0069.B00.1802022340", NULL, "Mac-942C5DF58193131B", // Intel Core 2 Duo L9600 @ 2.13 GHz
    "MacBook Air", "1.0", "C02DRHACDDR3", "Air-Enclosure",
    { 0x01, 0x66, 0x0f, 0, 0, 0x61 },  "NA", "NA", 0x76005 }, // need rBR RPlt ECPI
  //MacBookAir4,1
  { "MacBookAir4,1", "MBA41.88Z.0080.B00.1804111222", NULL, "Mac-C08A6BB70A942AC2", // Intel Core i7-2677M @ 1.80 GHz
    "MacBook Air", "1.0", "C02KGHACDRV9", "Air-Enclosure",
    { 0x01, 0x74, 0x0f, 0, 0, 0x04 },  "k21k78", "k78", 0x76005 }, // need ECPI
  //MacBookAir4,2
  { "MacBookAir4,2", "MBA41.88Z.0080.B00.1804111222", NULL, "Mac-742912EFDBEE19B3", // Intel Core i5-2557M @ 1.70 GHz
    "MacBook Air", "1.0", "C02GLHACDJWT", "Air-Enclosure",
    { 0x01, 0x73, 0x0f, 0, 0, 0x66 },  "k21k78", "k21", 0x76005 }, // need ECPI
  //MacBookAir5,1
  { "MacBookAir5,1", "MBA51.88Z.F000.B00.1809210852", "253.0.0.0.0", "Mac-66F35F19FE2A0D05", // Intel Core i7-3667U @ 2.00 GHz
    "MacBook Air", "1.0", "C02J6HACDRV6", "Air-Enclosure",
    { 0x02, 0x04, 0x0f, 0, 0, 0x19 },  "j11j13", "j11", 0x7b006 }, // need ECPI
  //MacBookAir5,2
  { "MacBookAir5,2", "MBA51.88Z.F000.B00.1809210852", "253.0.0.0.0", "Mac-2E6FAB96566FE58C", // Intel Core i5-3427U @ 1.80 GHz
    "MacBook Air", "1.0", "C02HA041DRVC", "Air-Enclosure",
    { 0x02, 0x05, 0x0f, 0, 0, 0x09 },  "j11j13", "j13", 0x7b006 },
  //MacBookAir6,1
  { "MacBookAir6,1", "MBA61.88Z.0108.B00.1809171520", "110.0.0.0.0", "Mac-35C1E88140C3E6CF", // Intel Core i7-4650U @ 1.70 GHz
    "MacBook Air", "1.0", "C02KTHACF5NT", "Air-Enclosure",
    { 0x02, 0x12, 0x0f, 0, 1, 0x43 },  "j41j43", "j41", 0x7b007 }, // need ECPI
  //MacBookAir6,2
  { "MacBookAir6,2", "MBA61.88Z.0108.B00.1809171520", "110.0.0.0.0", "Mac-7DF21CB3ED6977E5", // Intel Core i5-4250U @ 1.30 GHz
    "MacBook Air", "1.0", "C02HACKUF5V7", "Air-Enclosure",
    { 0x02, 0x13, 0x0f, 0, 0, 0x15 },  "j41j43", "j43", 0x7b007 },
  //MacBookAir7,1
  { "MacBookAir7,1", "MBA71.88Z.0180.B00.1809171321", "182.0.0.0.0", "Mac-9F18E312C5C2BF0B", // Intel Core i5-5250U @ 1.60 GHz
    "MacBook Air", "1.0", "C02PCLGFH569", "Air-Enclosure",
    { 0x02, 0x26, 0x0f, 0, 0, 0x02 },  "j110", "j110", 0x7b007 },
  //MacBookAir7,2
  { "MacBookAir7,2", "MBA71.88Z.0180.B00.1809171321", "182.0.0.0.0", "Mac-937CB26E2E02BB01", // Intel Core i7-5650U @ 2.20 GHz
    "MacBook Air", "1.0", "C02Q1HACG940", "Air-Enclosure",
    { 0x02, 0x27, 0x0f, 0, 0, 0x02 },  "j113", "j113", 0x7b007 }, // need ECPI
  //MacBookAir8,1
  { "MacBookAir8,1", "MBA81.88Z.F000.B00.1809171321", "220.220.100.0.0", "Mac-827FAC58A8FDFA22", // Intel Core i5-8210Y @ 1.60 GHz
    "MacBook Air", "1.0", "FVFXJHACJK77", "Air-Enclosure",
    { 0x02, 0x24, 0x0f, 0, 0, 0x10 },  "j122", "j122", 0xf08009 }, // need BIOS REV rBR RPlt ECPI
  //Macmini1,1
  { "Macmini1,1", "MM11.88Z.0055.B08.0610121326", NULL, "Mac-F4208EC8", // Intel Core 2 Duo T7200 @ 2.00 GHz
    "Mac mini", "1.0", "W8702N1JU35", "Mini-Aluminum",
    { 0x01, 0x03, 0x0f, 0, 0, 0x04 },  "m40", "m40", 0x78002 }, // need ECPI
  //Macmini2,1
  { "Macmini2,1", "MM21.88Z.009A.B00.0706281359", NULL, "Mac-F4208EAA", // Intel Core 2 Duo T5600 @ 1.83 GHz
    "Mac mini", "1.1", "W8705W9LYL2", "Mini-Aluminum",
    { 0x01, 0x19, 0x0f, 0, 0, 0x02 },  "NA", "NA", 0x78002 }, // need rBR RPlt
  //Macmini3,1
  { "Macmini3,1", "MM31.88Z.0081.B06.0904271717", NULL, "Mac-F22C86C8", // Intel Core 2 Duo P7550 @ 2.26 GHz
    "Mac mini", "1.0", "W8905BBE19X", "Mini-Aluminum",
    { 0x01, 0x35, 0x0f, 0, 0, 0x01 },  "NA", "NA", 0x78002 }, // need rBR RPlt ECPI
  //Macmini4,1
  { "Macmini4,1", "MM41.88Z.0047.B00.1802021546", NULL, "Mac-F2208EC8", // Intel Core 2 Duo P8600 @ 2.40 GHz
    "Mac mini", "1.0", "C02FHBBEDD6H", "Mini-Aluminum",
    { 0x01, 0x65, 0x0f, 0, 0, 0x02 },  "NA", "NA", 0x78002 }, // need rBR RPlt ECPI
  //Macmini5,1
  { "Macmini5,1", "MM51.88Z.0080.B00.1804091930", NULL, "Mac-8ED6AF5B48C039E1", // Intel Core i5-2415M @ 2.30 GHz
    "Mac mini", "1.0", "C07GA041DJD0", "Mini-Aluminum",
    { 0x01, 0x30, 0x0f, 0, 0, 0x03 },  "NA", "NA", 0x7d005 }, // need rBR RPlt
  //Macmini5,2
  { "Macmini5,2", "MM51.88Z.0080.B00.1804091930", NULL, "Mac-4BC72D62AD45599E", // Intel Core i7-2620M @ 2.70 GHz
    "Mac mini", "1.0", "C07HVHACDJD1", "Mini-Aluminum",
    { 0x01, 0x75, 0x0f, 0, 0, 0x00 },  "NA", "NA", 0x7d005 }, // need rBR RPlt ECPI
  //Macmini5,3
  { "Macmini5,3", "MM51.88Z.0080.B00.1804091930", NULL, "Mac-7BA5B2794B2CDB12", // Intel Core i7-2635QM @ 2.00 GHz
    "Mac mini", "1.0", "C07GWHACDKDJ", "Mini-Aluminum",
    { 0x01, 0x77, 0x0f, 0, 0, 0x00 },  "NA", "NA", 0x7d005 }, // need rBR RPlt ECPI
  //Macmini6,1
  { "Macmini6,1", "MM61.88Z.F000.B00.1809171514", "274.0.0.0.0", "Mac-031AEE4D24BFF0B1", // Intel Core i5-3210M @ 2.50 GHz
    "Mac mini", "1.0", "C07JNHACDY3H", "Mini-Aluminum",
    { 0x02, 0x07, 0x0f, 0, 0, 0x00 },  "NA", "NA", 0x7d006 }, // need rBR RPlt ECPI
  //Macmini6,2
  { "Macmini6,2", "MM61.88Z.F000.B00.1809171514", "274.0.0.0.0", "Mac-F65AE981FFA204ED", // Intel Core i7-3615QM @ 2.30 GHz
    "Mac mini", "1.0", "C07JD041DWYN", "Mini-Aluminum",
    { 0x02, 0x08, 0x0f, 0, 0, 0x01 },  "j50s", "j50s", 0x7d006 },
  //Macmini7,1
  { "Macmini7,1", "MM71.88Z.0234.B00.1809171422", "236.0.0.0.0", "Mac-35C5E08120C7EEAF", // Intel Core i5-4278U @ 2.60 GHz
    "Mac mini", "1.0", "C02NN7NHG1J0", "Mini-Aluminum",
    { 0x02, 0x24, 0x0f, 0, 0, 0x32 },  "j64", "j64", 0xf04008 },
  //Macmini8,1
  { "Macmini8,1", "MM81.88Z.F000.B00.1809171422", "220.207.27.0.0", "Mac-7BA5B2DFE22DDD8C", // Intel Core i7-8700B @ 3.20 GHz
    "Mac mini", "1.0", "C07XL9WEJYVX", "Mini-Aluminum",
    { 0x02, 0x40, 0x0f, 0, 0, 0x00 },  "j133_4_5", "j135", 0xf07009 }, // need BIOS REV rBR RPlt ECPI
  //iMac4,1
  { "iMac4,1", "IM41.88Z.0055.B08.0609061538", NULL, "Mac-F42787C8", // Intel Core 2 Duo T7200 @ 2.00 GHz
    "iMac", "1.0", "W8608HACU2P", "iMac",
    { 0x01, 0x01, 0x0f, 0, 0, 0x05 },  "m38m39", "m38", 0x73002 }, // need ECPI
  //iMac4,2
  { "iMac4,2", "IM42.88Z.0071.B03.0610121320", NULL, "Mac-F4218EC8", // Intel Core 2 Duo T5600 @ 1.83 GHz
    "iMac", "1.0", "W8627HACV2H", "iMac",
    { 0x01, 0x06, 0x0f, 0, 0, 0x00 },  "NA", "NA", 0x73002 }, // need rBR RPlt ECPI
  //iMac5,1
  { "iMac5,1", "IM51.88Z.0090.B09.0706270921", NULL, "Mac-F4228EC8", // Intel Core 2 Duo T7400 @ 2.16 GHz
    "iMac", "1.0", "CK637HACX1A", "iMac",
    { 0x01, 0x08, 0x0f, 0, 0, 0x02 },  "NA", "NA", 0x73002 }, // need rBR RPlt ECPI
  //iMac5,2
  { "iMac5,2", "IM52.88Z.0090.B09.0706270913", NULL, "Mac-F4218EC8", // Intel Core 2 Duo T5600 @ 1.83 GHz
    "iMac", "1.0", "W8716HACWH5", "iMac",
    { 0x01, 0x06, 0x0f, 0, 0, 0x00 },  "NA", "NA", 0x73002 }, // need rBR RPlt ECPI
  //iMac6,1
  { "iMac6,1", "IM61.88Z.0093.B07.0804281538", NULL, "Mac-F4218FC8", // Intel Core 2 Duo T7400 @ 2.16 GHz
    "iMac", "1.0", "W8652HACVGN", "iMac",
    { 0x01, 0x08, 0x0f, 0, 0, 0x02 },  "NA", "NA", 0x73002 }, // need rBR RPlt ECPI
  //iMac7,1
  { "iMac7,1", "IM71.88Z.007A.B03.0803051705", NULL, "Mac-F42386C8", // Intel Core 2 Extreme X7900 @ 2.80 GHz
    "iMac", "1.0", "W8803HACY51", "iMac-Aluminum",
    { 0x01, 0x20, 0x0f, 0, 0, 0x04 },  "NA", "NA", 0x73002 }, // need rBR RPlt ECPI
  //iMac8,1
  { "iMac8,1", "IM81.88Z.00C1.B00.0802091538", NULL, "Mac-F227BEC8", // Intel Core 2 Duo E8435 @ 3.06 GHz
    "iMac", "1.3", "W8755HAC2E2", "iMac-Aluminum",
    { 0x01, 0x29, 0x0f, 0, 0, 0x01 },  "k3", "k3", 0x73002 },
  //iMac9,1
  { "iMac9,1", "IM91.88Z.008D.B08.0904271717", NULL, "Mac-F2218FA9", // Intel Core 2 Duo E8135 @ 2.66 GHz
    "iMac", "1.0", "W89A00A36MJ", "iMac-Aluminum",
    { 0x01, 0x36, 0x0f, 0, 0, 0x03 },  "NA", "NA", 0x73002 }, // need rBR RPlt ECPI
  //iMac10,1
  { "iMac10,1", "IM101.88Z.00D0.B00.1802021905", NULL, "Mac-F2268CC8", // Intel Core 2 Duo E7600 @ 3.06 GHz
    "iMac", "1.0", "W80AA98A5PE", "iMac-Aluminum",
    { 0x01, 0x53, 0x0f, 0, 0, 0x13 },  "k22k23", "k23", 0x7b002 },
  //iMac11,1
  { "iMac11,1", "IM111.88Z.0039.B00.1804101044", NULL, "Mac-F2268DAE", // Intel Core i7-860 @ 2.80 GHz
    "iMac", "1.0", "G8942B1V5PJ", "iMac-Aluminum",
    { 0x01, 0x54, 0x0f, 0, 0, 0x36 },  "k23f", "k23f", 0x7b004 },
  //iMac11,2
  { "iMac11,2", "IM112.88Z.005D.B00.1804101436", NULL, "Mac-F2238AC8", // Intel Core i3-540 @ 3.06 GHz
    "iMac", "1.2", "W8034342DB7", "iMac-Aluminum",
    { 0x01, 0x64, 0x0f, 0, 0, 0x05 },  "k74", "k74", 0x7c004 },
  //iMac11,3
  { "iMac11,3", "IM112.88Z.005D.B00.1804101436", NULL, "Mac-F2238BAE", // Intel Core i5-760 @ 2.80 GHz
    "iMac", "1.0", "QP0312PBDNR", "iMac-Aluminum",
    { 0x01, 0x59, 0x0f, 0, 0, 0x02 },  "k74", "k74", 0x7d004 },
  //iMac12,1
  { "iMac12,1", "IM121.88Z.004F.B00.1804101150", NULL, "Mac-942B5BF58194151B", // Intel Core i7-2600S @ 2.80 GHz
    "iMac", "1.9", "W80CF65ADHJF", "iMac-Aluminum",
    { 0x01, 0x71, 0x0f, 0, 0, 0x22 },  "k60", "k60", 0x73005 },
  //iMac12,2
  { "iMac12,2", "IM121.88Z.004F.B00.1804101150", NULL, "Mac-942B59F58194171B", // Intel Core i5-2500S @ 2.70 GHz
    "iMac", "1.9", "W88GG136DHJQ", "iMac-Aluminum",
    { 0x01, 0x72, 0x0f, 0, 0, 0x02 },  "k62", "k62", 0x75005 },
  //iMac13,1
  { "iMac13,1", "IM131.88Z.F000.B00.1809171346", "281.0.0.0.0", "Mac-00BE6ED71E35EB86", // Intel Core i7-3770S @ 3.10 GHz
    "iMac", "1.0", "C02JA041DNCT", "iMac-Aluminum",
    { 0x02, 0x09, 0x0f, 0, 0, 0x05 },  "d8", "d8", 0x78006 },
  //iMac13,2
  { "iMac13,2", "IM131.88Z.F000.B00.1809171346", "281.0.0.0.0", "Mac-FC02E91DDD3FA6A4", // Intel Core i5-3470 @ 3.20 GHz
    "iMac", "1.0", "C02JB041DNCW", "iMac-Aluminum",
    { 0x02, 0x11, 0x0f, 0, 0, 0x16 },  "d8", "d8", 0x79006 },
  //iMac13,3
  { "iMac13,3", "IM131.88Z.F000.B00.1809171346", "281.0.0.0.0", "Mac-7DF2A3B5E5D671ED", // Intel Core i3-3225 @ 3.30 GHz
    "iMac", "1.0", "C02KVHACFFYW", "iMac-Aluminum",
    { 0x02, 0x13, 0x0f, 0, 0, 0x15 },  "d8", "d8", 0x79006 }, // need ECPI
  //iMac14,1
  { "iMac14,1", "IM141.88Z.0131.B00.1809171347", "133.0.0.0.0", "Mac-031B6874CF7F642A", // Intel Core i5-4570R @ 2.70 GHz
    "iMac", "1.0", "D25LHACKF8J2", "iMac-Aluminum",
    { 0x02, 0x14, 0x0f, 0, 0, 0x24 },  "j16j17", "j16", 0x79007 },
  //iMac14,2
  { "iMac14,2", "IM142.88Z.0131.B00.1809171347", "133.0.0.0.0", "Mac-27ADBB7B4CEE8E61", // Intel Core i5-4570 @ 3.20 GHz
    "iMac", "1.0", "D25LHACKF8JC", "iMac-Aluminum",
    { 0x02, 0x15, 0x0f, 0, 0, 0x07 },  "j16j17", "j17", 0x7a007 },
  //iMac14,3
  { "iMac14,3", "IM143.88Z.0131.B00.1809171346", "133.0.0.0.0", "Mac-77EB7D7DAF985301", // Intel Core i5-4570S @ 2.90 GHz
    "iMac", "1.0", "D25LHACKF8J3", "iMac-Aluminum",
    { 0x02, 0x17, 0x0f, 0, 0, 0x07 },  "j16g", "j16g", 0x7a007 }, // need ECPI
  //iMac14,4
  { "iMac14,4", "IM144.88Z.0190.B00.1809171521", "192.0.0.0.0", "Mac-81E3E92DD6088272", // Intel Core i5-4260U @ 1.40 GHz
    "iMac", "1.0", "D25LHACKFY0T", "iMac-Aluminum",
    { 0x02, 0x21, 0x0f, 0, 0, 0x92 },  "j70", "j70", 0x7a007 }, // need ECPI
  //iMac15,1
  { "iMac15,1", "IM151.88Z.0219.B00.1809190740", "222.0.0.0.0", "Mac-42FD25EABCABB274", // Intel Core i5-4690 @ 3.50 GHz
    "iMac", "1.0", "C02Q6HACFY10", "iMac-Aluminum",
    { 0x02, 0x22, 0x0f, 0, 0, 0x16 },  "j78j78am", "j78", 0xf00008 },
  //iMac16,1
  { "iMac16,1", "IM161.88Z.0221.B00.1809171321", "223.0.0.0.0", "Mac-A369DDC4E67F1C45", // Intel Core i5-5250U @ 1.60 GHz
    "iMac", "1.0", "C02QQHACGF1J", "iMac-Aluminum",
    { 0x02, 0x31, 0x0f, 0, 0, 0x36 },  "j117", "j117", 0xf00008 }, // need ECPI
  //iMac16,2
  { "iMac16,2", "IM162.88Z.0221.B00.1809171530", "223.0.0.0.0", "Mac-FFE5EF870D7BA81A", // Intel Core i5-5575R @ 2.80 GHz
    "iMac", "1.0", "C02PNHACGG7G", "iMac-Aluminum",
    { 0x02, 0x32, 0x0f, 0, 0, 0x20 },  "j94", "j94", 0xf00008 }, // need ECPI
  //iMac17,1
  { "iMac17,1", "IM171.88Z.F000.B00.1809251200", "161.0.0.0.0", "Mac-B809C3757DA9BB8D", // Intel Core i7-6700K @ 4.00 GHz
    "iMac17,1", "1.0", "C02QFHACGG7L", "iMac-Aluminum",
    { 0x02, 0x33, 0x0f, 0, 0, 0x10 },  "j95j95am", "j95", 0xf0c008 }, //Note: i5 but for i7 { 0x02, 0x34, 0x0F, 0, 0, 0x02 }
  //iMac18,1
  { "iMac18,1", "IM181.88Z.F000.B00.1809171524", "165.0.0.0.0", "Mac-4B682C642B45593E", // Intel Core i5-7360U @ 2.30 GHz
    "iMac", "1.0", "C02TDHACH7JY", "iMac-Aluminum",
    { 0x02, 0x39, 0x0f, 0, 0, 0x06 },  "j133_4_5", "j135", 0xf07009 }, // need RPlt ECPI
  //iMac18,2
  { "iMac18,2", "IM183.88Z.F000.B00.1809280842", "166.0.0.0.0", "Mac-77F17D7DA9285301", // Intel Core i5-7500 @ 3.40 GHz
    "iMac", "1.0", "C02TDHACJ1G5", "iMac-Aluminum",
    { 0x02, 0x40, 0x0f, 0, 0, 0x00 },  "j133_4_5", "j135", 0xf07009 }, // need RPlt ECPI
  //iMac18,3
  { "iMac18,3", "IM183.88Z.F000.B00.1809280842", "166.0.0.0.0", "Mac-BE088AF8C5EB4FA2", // Intel Core i7-7700K @ 4.20 GHz
    "iMac", "1.0", "C02TDHACJ1GJ", "iMac-Aluminum",
    { 0x02, 0x41, 0x0f, 0, 0, 0x01 },  "j133_4_5", "j135", 0xf07009 },
  //iMacPro1,1
  { "iMacPro1,1", "IMP11.88Z.0064.B30.1712081714", "220.200.252.0.0", "Mac-7BA5B2D9E42DDD94", // Intel Xeon W-2140B CPU @ 3.20 GHz
    "iMac Pro", "1.0", "C02VVHACHX87", "iMacPro-Aluminum",
    { 0x02, 0x41, 0x0f, 0, 0, 0x01 },  "j137", "j137", 0xf07009 }, // need REV rBR ECPI
  //MacPro1,1
  { "MacPro1,1", "MP11.88Z.005C.B08.0707021221", NULL, "Mac-F4208DC8", // Intel Xeon X5355 @ 2.66 GHz x2
    "MacPro", "1.0", "W88A77AXUPZ", "Pro-Enclosure",
    { 0x01, 0x07, 0x0f, 0, 0, 0x10 },  "m43", "m43", 0x79001 }, // need ECPI
  //MacPro2,1
  { "MacPro2,1", "MP21.88Z.007F.B06.0707021348", NULL, "Mac-F4208DA9", // Intel Xeon X5365 @ 2.99 GHz x2
    "MacPro", "1.0", "W8930518UPZ", "Pro-Enclosure",
    { 0x01, 0x15, 0x0f, 0, 0, 0x03 },  "m43a", "m43a", 0x79001 }, // need ECPI
  //MacPro3,1
  { "MacPro3,1", "MP31.88Z.006C.B05.0802291410", NULL, "Mac-F42C88C8", // Intel Xeon E5462 @ 2.80 GHz x2
    "MacPro", "1.3", "W88A77AA5J4", "Pro-Enclosure",
    { 0x01, 0x30, 0x0f, 0, 0, 0x03 },  "m86", "m86", 0x79001 },
  //MacPro4,1
  { "MacPro4,1", "MP41.88Z.0081.B07.0910130729", NULL, "Mac-F221BEC8", // Intel Xeon X5670 @ 2.93 GHz x2
    "MacPro", "1.4", "CT93051DK9Y", "Pro-Enclosure",
    { 0x01, 0x39, 0x0f, 0, 0, 0x05 },  "NA", "NA", 0x7c002 }, // need rBR RPlt
  //MacPro5,1
  { "MacPro5,1", "MP51.88Z.F000.B00.1809191555", "140.0.0.0.0", "Mac-F221BEC8", // Intel Xeon X5675 @ 3.06 GHz x2
    "MacPro", "1.2", "C07J77F7F4MC", "Pro-Enclosure", //Note: C07J50F7F4MC CK04000AHFC CG154TB9WU3
    { 0x01, 0x39, 0x0f, 0, 0, 0x11 },  "k5", "k5", 0x7c002 },
  //MacPro6,1
  { "MacPro6,1", "MP61.88Z.0125.B00.1809171517", "127.0.0.0.0", "Mac-F60DEB81FF30ACF6", // Intel Xeon E5-1650 v2 @ 3.50 GHz
    "MacPro", "1.0", "F5KLA770F9VM", "Pro-Enclosure",
    { 0x02, 0x20, 0x0f, 0, 0, 0x18 },  "j90", "j90", 0xf0f006 },
  //Xserve1,1
  { "Xserve1,1", "XS11.88Z.0080.B01.0706271533", NULL, "Mac-F4208AC8", // Intel Xeon E5345 @ 2.33 GHz x2
    "Xserve", "1.0", "CK703E1EV2Q", "Xserve",
    { 0x01, 0x11, 0x0f, 0, 0, 0x05 },  "NA", "NA", 0x79001 }, // need rBR RPlt ECPI
  //Xserve2,1
  { "Xserve2,1", "XS21.88Z.006C.B06.0804011317", NULL, "Mac-F42289C8", // Intel Xeon E5472 @ 3.00 GHz x2
    "Xserve", "1.0", "CK830DLQX8S", "Xserve",
    { 0x01, 0x26, 0x0f, 0, 0, 0x03 },  "NA", "NA", 0x79001 }, // need rBR RPlt ECPI
  //Xserve3,1
  { "Xserve3,1", "XS31.88Z.0081.B06.0908061300", NULL, "Mac-F223BEC8", // Intel Xeon E5520 @ 2.26 GHz
    "Xserve", "1.0", "CK933YJ16HS", "Xserve",
    { 0x01, 0x43, 0x0f, 0, 0, 0x04 },  "NA", "NA", 0x79001 }, // need rBR RPlt ECPI
};

VOID SetDMISettingsForModel(MACHINE_TYPES Model, BOOLEAN Redefine)
{
  const CHAR8  *i;
  CHAR8 *Res1 = AllocateZeroPool(9), *Res2 = AllocateZeroPool(11);

  AsciiStrCpyS (gSettings.VendorName, 64,      BiosVendor);
  AsciiStrCpyS (gSettings.RomVersion, 64,      ApplePlatformData[Model].firmwareVersion);

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
      i = ApplePlatformData[Model].firmwareVersion;
      i += AsciiStrLen (i);

      while (*i != '.') {
        i--;
      }
      AsciiSPrint (Res1, 9, "%c%c/%c%c/%c%c\n", i[3], i[4], i[5], i[6], i[1], i[2]);
      AsciiStrCpyS (gSettings.ReleaseDate, 64, Res1);
      break;

    default:
      i = ApplePlatformData[Model].firmwareVersion;
      i += AsciiStrLen (i);

      while (*i != '.') {
        i--;
      }
      AsciiSPrint (Res2, 11, "%c%c/%c%c/20%c%c\n", i[3], i[4], i[5], i[6], i[1], i[2]);
      AsciiStrCpyS (gSettings.ReleaseDate, 64, Res2);
      break;
  }

  AsciiStrCpyS (gSettings.EfiVersion, 64,           ApplePlatformData[Model].efiversion);
  AsciiStrCpyS (gSettings.ManufactureName, 64,      BiosVendor);
  if (Redefine) {
    AsciiStrCpyS (gSettings.ProductName, 64,        ApplePlatformData[Model].productName);
  }
  AsciiStrCpyS (gSettings.VersionNr, 64,            ApplePlatformData[Model].systemVersion);
  AsciiStrCpyS (gSettings.SerialNr, 64,             ApplePlatformData[Model].serialNumber);
  AsciiStrCpyS (gSettings.FamilyName, 64,           ApplePlatformData[Model].productFamily);
  AsciiStrCpyS (gSettings.BoardManufactureName, 64, BiosVendor);
  AsciiStrCpyS (gSettings.BoardSerialNumber, 64,    AppleBoardSN);
  AsciiStrCpyS (gSettings.BoardNumber, 64,          ApplePlatformData[Model].boardID);
  AsciiStrCpyS (gSettings.BoardVersion, 64,         ApplePlatformData[Model].productName);
  AsciiStrCpyS (gSettings.LocationInChassis, 64,    AppleBoardLocation);
  AsciiStrCpyS (gSettings.ChassisManufacturer, 64,  BiosVendor);
  AsciiStrCpyS (gSettings.ChassisAssetTag, 64,      ApplePlatformData[Model].chassisAsset);

  // Firmware info for 10.13+
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
      gFwFeatures             = 0xE80FE137;
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
      gFwFeatures             = 0xFC0FE137;
      break;
    case MacBook91:
    case MacBook101:
    case MacBookPro133:
    case MacBookPro143:
      gFwFeatures             = 0xFC0FE13F;
      break;
    case iMacPro11:
      gFwFeatures             = 0xFD8FF53F;
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
      gFwFeatures             = 0xE80FE137;
      break;
    case MacBookPro121:
    case MacBookPro151:
    case MacBookPro152:
    case MacBookAir81:
    case MacMini81:
    case iMac161:
    case iMac162:
      gFwFeatures             = 0xFC0FE137;
      break;
    case MacBook61:
    case MacBook71:
    case MacBook81:
      gFwFeatures             = 0xFC0FE13F;
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
    case iMacPro11:
      gFwFeaturesMask         = 0xFF9FFF3F;
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
    case MacBookPro151:
    case MacBookPro152:
    case MacBookAir31:
    case MacBookAir32:
    case MacBookAir71:
    case MacBookAir72:
    case MacBookAir81:
    case MacMini41:
    case MacMini71:
    case MacMini81:
    case iMac101:
    case iMac111:
    case iMac112:
    case iMac113:
    case iMac121:
    case iMac122:
    case iMac161:
    case iMac162:
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
    case MacMini81:
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
    case MacBookPro151:
    case MacBookPro152:
    case MacBookAir81:
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

  // MiscChassisType
  // Mobile: the battery tab in Energy Saver
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
      switch (Model) {
        case MacMini71:
          gSettings.Mobile      = FALSE;
          break;
        default:
          gSettings.Mobile      = TRUE;
          break;
      }
      break;

    case MacBook81:
    case MacBook91:
    case MacBook101:
    case MacBookPro121:
    case MacBookPro131:
    case MacBookPro132:
    case MacBookPro133:
    case MacBookPro141:
    case MacBookPro142:
    case MacBookPro143:
    case MacBookPro151:
    case MacBookPro152:
    case MacBookAir71:
    case MacBookAir72:
    case MacBookAir81:
    case MacMini81:
    case iMac161:
    case iMac162:
    case iMac171:
    case iMac181:
    case iMac182:
    case iMac183:
    case iMacPro11:
      gSettings.ChassisType = MiscChassisTypeLapTop; //0x09;
      switch (Model) {
        case MacMini81:
        case iMac161:
        case iMac162:
        case iMac171:
        case iMac181:
        case iMac182:
        case iMac183:
        case iMacPro11:
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
      gSettings.ChassisType = MiscChassisTypeUnknown; //0x02; this is a joke but think different!
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
      /*if (gMobile) {
        gSettings.ChassisType = 10; //notebook
      } else {
        gSettings.ChassisType = MiscChassisTypeDeskTop; //0x03;
      }*/
      break;
  }

  //RBr helper
  if (ApplePlatformData[Model].smcBranch[0] != 'N') {
    AsciiStrCpyS (gSettings.RBr, 8, ApplePlatformData[Model].smcBranch);
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
  if (ApplePlatformData[Model].smcPlatform[0] != 'N') {
    AsciiStrCpyS (gSettings.RPlt, 8, ApplePlatformData[Model].smcPlatform);
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
  CopyMem (gSettings.REV,  ApplePlatformData[Model].smcRevision, 6);
  CopyMem (gSettings.EPCI, &ApplePlatformData[Model].smcConfig,  4);
}

MACHINE_TYPES GetModelFromString(CHAR8 *ProductName)
{
  UINTN i;

  for (i = 0; i < MaxMachineType; ++i) {
    if (AsciiStrCmp (ApplePlatformData[i].productName, ProductName) == 0) {
      return i;
    }
  }
  // return ending enum as "not found"
  return MaxMachineType;
}

VOID GetDefaultSettings()
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
  CopyMem (gSettings.NVCAP, default_NVCAP, 20); 
  CopyMem (gSettings.Dcfg, default_dcfg_0, 4);
  CopyMem (&gSettings.Dcfg[4], default_dcfg_1, 4);
  //gSettings.CustomEDID           = NULL; //no sense to assign 0 as the structure is zeroed
  gSettings.DualLink             = 0xA; // A(auto): DualLink auto-detection
  gSettings.HDAInjection         = FALSE;
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
  gSettings.UIScale              = 1;
  
  ResumeFromCoreStorage          = FALSE;

  if (gCPUStructure.Model >= CPU_MODEL_IVY_BRIDGE) {
    gSettings.GeneratePStates    = TRUE;
    gSettings.GenerateCStates    = TRUE;
    // backward compatibility, APFS, APLF, PluginType follow PStates
    gSettings.GenerateAPSN = gSettings.GeneratePStates;
    gSettings.GenerateAPLF = gSettings.GeneratePStates;
    gSettings.GeneratePluginType = gSettings.GeneratePStates;
    //  gSettings.EnableISS          = FALSE;
    //  gSettings.EnableC2           = TRUE;
    gSettings.EnableC6           = TRUE;
    gSettings.PluginType         = 1;

    if (gCPUStructure.Model == CPU_MODEL_IVY_BRIDGE) {
      gSettings.MinMultiplier    = 7;
    }
    //  gSettings.DoubleFirstState   = FALSE;
    //gSettings.DropSSDT           = TRUE;    //why drop all???
    gSettings.C3Latency          = 0x00FA;
  }
  
//CPU
  //gSettings.EnableISS            = FALSE; //((gCPUStructure.CPUID[CPUID_1][ECX] & (1<<7)) != 0);
  gSettings.Turbo                = gCPUStructure.Turbo;
  gSettings.SavingMode           = 0xFF;  //means not set
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
