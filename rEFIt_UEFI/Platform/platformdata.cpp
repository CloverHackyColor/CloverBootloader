/**
 platformdata.c
 **/

#include "Platform.h"
#include "nvidia.h"

/* Machine Default Data */

CONST CHAR8   *DefaultMemEntry        = "N/A";
CONST CHAR8   *DefaultSerial          = "CT288GT9VT6";
CONST CHAR8   *BiosVendor             = "Apple Inc.";
CONST CHAR8   *AppleManufacturer      = "Apple Computer, Inc."; //Old name, before 2007
CONST CHAR8   *AppleBoardSN           = "C02140302D5DMT31M";
CONST CHAR8   *AppleBoardLocation     = "Part Component";

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

//--------------------------
/* AppleGraphicsDevicePolicy.kext in 10.14.6 contains follow board-id to choose from graphics config
 none:
 Mac-00BE6ED71E35EB86 iMac13,1
 Mac-27ADBB7B4CEE8E61 iMac14,2  GTX775M devID=119d
 Mac-4B7AC7E43945597E MacBookPro9,1 HD4000+GT650M devID=fd5 display connected to nvidia
 Mac-77EB7D7DAF985301 iMac14,3
 Mac-C3EC7CD22292981F MacBookPro10,1 HD4000 + Kepler
 Mac-C9CF552659EA9913
 Mac-FC02E91DDD3FA6A4 iMac13,2  GTX675MX devID=11a2
 
 GFX1 only
 Mac-F60DEB81FF30ACF6 MacPro6,1
 
 GFX0 only
 Mac-031B6874CF7F642A iMac14,1  Intel Iris Pro devID=0d22
 Mac-42FD25EABCABB274 iMac15,1  Radeon R9 M290X == R9 270X devID=6810
 Mac-65CE76090165799A iMac17,1
 Mac-81E3E92DD6088272 iMac14,4
 Mac-B809C3757DA9BB8D iMac17,1
 Mac-DB15BD556843C820 iMac17,1  HD530(no FB) + Radeon HD7850 devID=6819
 Mac-FA842E06C61E91C5 iMac15,1
 
 GFX0+IGPU
 Mac-63001698E7A34814 iMac19,2
 Mac-77F17D7DA9285301 iMac18,2 Radeon Pro 555  devID=67ef
 Mac-AA95B1DDAB278B95 iMac19,1 Radeon Pro 570X devID=67df
 Mac-BE088AF8C5EB4FA2 iMac18,3 Radeon Pro 575 == RX480/580 devID=67df
 
 GFX0+IGPU+display
 Mac-7BA5B2D9E42DDD94 iMacPro1,1
 
 */
//--------------------------

PLATFORMDATA ApplePlatformData[] =
{
  //MacBook1,1 / MacBook (13-inch)
  { "MacBook1,1", "MB11.88Z.0061.B03.0610121324", NULL, "Mac-F4208CC8", // Intel Core Duo T2500 @ 2.00 GHz
    "MacBook", "1.1", "4H625HACVTH", "MacBook-White",
    { 0x01, 0x04, 0x0f, 0, 0, 0x12 },  "branch", "m70", 0x71001 },
  //MacBook2,1 / MacBook (13-inch Late 2006)
  { "MacBook2,1", "MB21.88Z.00A5.B07.0706270922", NULL, "Mac-F4208CA9", // Intel Core 2 Duo T7200 @ 2.00 GHz
    "MacBook", "1.2", "W8713HACWGL", "MacBook-White",
    { 0x01, 0x13, 0x0f, 0, 0, 0x03 },  "branch", "m75", 0x72001 },
  //MacBook3,1 / MacBook (13-inch Late 2007)
  { "MacBook3,1", "MB31.88Z.008E.B02.0803051832", NULL, "Mac-F22788C8", // Intel Core 2 Duo T7500 @ 2.20 GHz
    "MacBook", "1.3", "W8747HACZ63", "MacBook-White",
    { 0x01, 0x24, 0x0f, 0, 0, 0x03 },  "branch", "k36", 0x72001 }, // need EPCI
  //MacBook4,1 / MacBook (13-inch, Early 2008)
  { "MacBook4,1", "MB41.88Z.00C1.B00.0802091535", NULL, "Mac-F22788A9", // Intel Core 2 Duo T8300 @ 2.40 GHz
    "MacBook", "1.3", "W88A041A0P0", "MacBook-Black",
    { 0x01, 0x31, 0x0f, 0, 0, 0x01 },  "branch", "m82", 0x74001 },
  //MacBook5,1 / MacBook (13-inch, Aluminum, Late 2008)
  { "MacBook5,1", "MB51.88Z.007D.B03.0904271443", NULL, "Mac-F42D89C8", // Intel Core 2 Duo P7350 @ 2.00 GHz
    "MacBook", "1.3", "W8944T1S1AQ", "MacBook-Aluminum",
    { 0x01, 0x32, 0x0f, 0, 0, 0x08 },  "branch", "m97", 0x7a002 }, // need EPCI
  //MacBook5,2 / MacBook (13-inch, Early 2009)
  { "MacBook5,2", "MB52.88Z.0088.B05.0904162222", NULL, "Mac-F22788AA", // Intel Core 2 Duo P7450 @ 2.13 GHz
    "MacBook", "1.3", "W8913HAC4R1", "MacBook-Black",
    { 0x01, 0x38, 0x0f, 0, 0, 0x05 },  "branch", "k36b", 0x7a002 },
  //MacBook6,1 / MacBook (13-inch, Late 2009)
  { "MacBook6,1", "MB61.88Z.F000.B00.1906140014", "209.0.0.0.0", "Mac-F22C8AC8", // Intel Core 2 Duo P7550 @ 2.26 GHz
    "MacBook", "1.0", "451131JCGAY", "MacBook-White",
    { 0x01, 0x51, 0x0f, 0, 0, 0x53 },  "k84", "k84", 0x72004 },
  //MacBook7,1 / MacBook (13-inch, Mid 2010)
  { "MacBook7,1", "MB71.88Z.F000.B00.1906140026", "68.0.0.0.0", "Mac-F22C89C8", // Intel Core 2 Duo P8600 @ 2.40 GHz
    "MacBook", "1.0", "451211MEF5X", "MacBook-White",
    { 0x01, 0x60, 0x0f, 0, 0, 0x06 },  "k87", "k87", 0x72005 },
  //MacBook8,1 / MacBook (Retina, 12-inch, Early 2015)
  { "MacBook8,1", "MB81.88Z.F000.B00.2002051746", "188.0.0.0.0", "Mac-BE0E8AC46FE800CC", // Intel Core M-5Y51 @ 1.20 GHz
    "MacBook", "1.0", "C02RCE58GCN3", "MacBook-Aluminum",
    { 0x02, 0x25, 0x0f, 0, 0, 0x87 },  "j92", "j92", 0xf0e007 },
  //MacBook9,1 / MacBook (Retina, 12-inch, Early 2016)
  { "MacBook9,1", "MB91.88Z.F000.B00.2001311734", "190.0.0.0.0", "Mac-9AE82516C7C6B903", // Intel Core m5-6Y54 @ 1.20 GHz
    "MacBook", "1.0", "C02RM408HDNK", "MacBook-Aluminum",
    { 0x02, 0x35, 0x0f, 0, 1, 0x06 },  "j93", "j93", 0xf0e007 }, // need EPCI
  //MacBook10,1 / MacBook (Retina, 12-inch, 2017)
  { "MacBook10,1", "MB101.88Z.F000.B00.2001311754", "185.0.0.0.0", "Mac-EE2EBD4B90B839A8", // Intel Core i5-7Y54 @ 1.30 GHz
    "MacBook", "1.0", "C02TQHACHH27", "MacBook-Aluminum",
    { 0x02, 0x42, 0x0f, 0, 0, 0x12 },  "j122", "j122", 0xf08009 }, // need EPCI
  //MacBookPro1,1 / MacBook Pro (15-inch Glossy)
  { "MacBookPro1,1", "MBP11.88Z.0055.B08.0610121325", NULL, "Mac-F425BEC8", // Intel Core Duo T2500 @ 2.00 GHz
    "MacBook Pro", "1.0", "W8634HACVWZ", "MacBook-Aluminum",
    { 0x01, 0x02, 0x0f, 0, 0, 0x10 },  "m1", "m1", 0x7b002 }, // need EPCI
  //MacBookPro1,2 / MacBook Pro (17-inch)
  { "MacBookPro1,2", "MBP12.88Z.0061.B03.0610121334", NULL, "Mac-F42DBEC8", // Intel Core Duo T2600 @ 2.17 GHz
    "MacBook Pro", "1.0", "W8629HACTHY", "MacBook-Aluminum",
    { 0x01, 0x05, 0x0f, 0, 0, 0x10 },  "NA", "NA", 0x7b002 }, // need rBR RPlt EPCI
  //MacBookPro2,1 / MacBook Pro (17-inch Core 2 Duo)
  { "MacBookPro2,1", "MBP21.88Z.00A5.B08.0708131242", NULL, "Mac-F42189C8", // Intel Core 2 Duo T7600 @ 2.33 GHz
    "MacBook Pro", "1.0", "W8715HACW0J", "MacBook-Aluminum",
    { 0x01, 0x14, 0x0f, 0, 0, 0x05 },  "NA", "NA", 0x7b002 }, // need rBR RPlt EPCI
  //MacBookPro2,2 / MacBook Pro (15-inch Core 2 Duo)
  { "MacBookPro2,2", "MBP22.88Z.00A5.B07.0708131242", NULL, "Mac-F42187C8", // Intel Core 2 Duo T7400 @ 2.16 GHz
    "MacBook Pro", "1.0", "W8827B4CW0L", "MacBook-Aluminum",
    { 0x01, 0x13, 0x0f, 0, 0, 0x03 },  "NA", "NA", 0x7b002 }, // need rBR RPlt EPCI
  //MacBookPro3,1 / MacBook Pro (17-inch 2.4GHZ) - not exists in server
  { "MacBookPro3,1", "MBP31.88Z.0070.B07.0803051658", NULL, "Mac-F4238BC8", // Intel Core 2 Duo T7700 @ 2.40 GHz
    "MacBook Pro", "1.0", "W8841OHZX91", "MacBook-Aluminum",
    { 0x01, 0x16, 0x0f, 0, 0, 0x11 },  "NA", "NA", 0x7b002 }, // need rBR RPlt EPCI
  //MacBookPro4,1 / MacBook Pro (17-inch, Early 2008)
  { "MacBookPro4,1", "MBP41.88Z.00C1.B03.0802271651", NULL, "Mac-F42C89C8", // Intel Core 2 Duo T9500 @ 2.60 GHz
    "MacBook Pro", "1.0", "W88484F2YP4", "MacBook-Aluminum",
    { 0x01, 0x27, 0x0f, 0, 0, 0x03 },  "m87", "m87", 0x7b002 }, // need EPCI
  //MacBookPro5,1 / MacBook Pro (15-inch, Late 2008)
  { "MacBookPro5,1", "MBP51.88Z.007E.B06.1202061253", NULL, "Mac-F42D86C8", // Intel Core 2 Duo P8600 @ 2.40 GHz
    "MacBook Pro", "1.0", "W88439FE1G0", "MacBook-Aluminum",
    { 0x01, 0x33, 0x0f, 0, 0, 0x08 },  "m98", "m98", 0x7b002 },
  //MacBookPro5,2 / MacBook Pro (17-inch, Early 2009)
  { "MacBookPro5,2", "MBP52.88Z.008E.B05.0905042202", NULL, "Mac-F2268EC8", // Intel Core 2 Duo T9600 @ 2.80 GHz
    "MacBook Pro", "1.0", "W8908HAC2QP", "MacBook-Aluminum",
    { 0x01, 0x42, 0x0f, 0, 0, 0x04 },  "NA", "NA", 0x7b002 }, // need rBR RPlt EPCI
  //MacBookPro5,3 / MacBook Pro (15-inch, Mid 2009)
  { "MacBookPro5,3", "MBP53.88Z.00AC.B03.0906151647", NULL, "Mac-F22587C8", // Intel Core 2 Duo P8800 @ 2.66 GHz
    "MacBook Pro", "1.0", "W89E6HAC64C", "MacBook-Aluminum",
    { 0x01, 0x48, 0x0f, 0, 0, 0x02 },  "NA", "NA", 0x7b002 }, // need rBR RPlt EPCI
  //MacBookPro5,4 / MacBook Pro (15-inch, 2.53GHz, Mid 2009)
  { "MacBookPro5,4", "MBP53.88Z.00AC.B03.0906151647", NULL, "Mac-F22587A1", // Intel Core 2 Duo P8700 @ 2.53 GHz
    "MacBook Pro", "1.0", "W8948HAC7XJ", "MacBook-Aluminum",
    { 0x01, 0x49, 0x0f, 0, 0, 0x02 },  "NA", "NA", 0x7b002 }, // need rBR RPlt EPCI
  //MacBookPro5,5 / MacBook Pro (13-inch, Mid 2009)
  { "MacBookPro5,5", "MBP55.88Z.00AC.B03.0906151708", NULL, "Mac-F2268AC8", // Intel Core 2 Duo P7550 @ 2.26 GHz
    "MacBook Pro", "1.0", "W8951HAC66E", "MacBook-Aluminum",
    { 0x01, 0x47, 0x0f, 0, 0, 0x02 },  "branch", "k24", 0x7a003 },
  //MacBookPro6,1 / MacBook Pro (17-inch, Mid 2010)
  { "MacBookPro6,1", "MBP61.88Z.F000.B00.1906132235", "99.0.0.0.0", "Mac-F22589C8", // Intel Core i5-540M @ 2.53 GHz
    "MacBook Pro", "1.0", "C02G5834DC79", "MacBook-Aluminum",
    { 0x01, 0x57, 0x0f, 0, 0, 0x18 },  "k17", "k17", 0x7a004 }, // need EPCI
  //MacBookPro6,2 / MacBook Pro (15-inch, Mid 2010)
  { "MacBookPro6,2", "MBP61.88Z.F000.B00.1906132235", "99.0.0.0.0", "Mac-F22586C8", // Intel Core i7-620M @ 2.66 GHz
    "MacBook Pro", "1.0", "CK132A91AGW", "MacBook-Aluminum",
    { 0x01, 0x58, 0x0f, 0, 0, 0x17 },  "k74", "k74", 0x7a004 },
  //MacBookPro7,1 / MacBook Pro (13-inch, Mid 2010)
  { "MacBookPro7,1", "MBP71.88Z.F000.B00.1906132329", "68.0.0.0.0", "Mac-F222BEC8", // Intel Core 2 Duo P8600 @ 2.40 GHz
    "MacBook Pro", "1.0", "CK145C7NATM", "MacBook-Aluminum",
    { 0x01, 0x62, 0x0f, 0, 0, 0x07 },  "branch", "k6", 0x7a004 }, // need EPCI
  //MacBookPro8,1 / MacBook Pro (13-inch, Early 2011)
  { "MacBookPro8,1", "MBP81.88Z.F000.B00.1906132217", "87.0.0.0.0", "Mac-94245B3640C91C81", // Intel Core i5-2415M @ 2.30 GHz
    "MacBook Pro", "1.0", "W89F9196DH2G", "MacBook-Aluminum",
    { 0x01, 0x68, 0x0f, 0, 0, 0x99 },  "k90i", "k90i", 0x7b005 },
  //MacBookPro8,2 / MacBook Pro (15-inch, Early 2011)
  { "MacBookPro8,2", "MBP81.88Z.0050.B00.1906132217", "87.0.0.0.0", "Mac-94245A3940C91C80", // Intel Core i7-2675QM @ 2.20 GHz
    "MacBook Pro", "1.0", "C02HL0FGDF8X", "MacBook-Aluminum",
    { 0x01, 0x69, 0x0f, 0, 0, 0x04 },  "trunk", "k91f", 0x7b005 }, // need EPCI
  //MacBookPro8,3 / MacBook Pro (17-inch, Early 2011)
  { "MacBookPro8,3", "MBP81.88Z.0050.B00.1906132217", "87.0.0.0.0", "Mac-942459F5819B171B", // Intel Core i7-2860QM @ 2.49 GHz
    "MacBook Pro", "1.0", "W88F9CDEDF93", "MacBook-Aluminum",
    { 0x01, 0x70, 0x0f, 0, 0, 0x06 },  "k92", "k92", 0x7c005 },
  //MacBookPro9,1 / MacBook Pro (15-inch, Mid 2012)
  { "MacBookPro9,1", "MBP91.88Z.F000.B00.2002052053", "231.0.0.0.0", "Mac-4B7AC7E43945597E", // Intel Core i7-3720QM @ 2.60 GHz
    "MacBook Pro", "1.0", "C02LW984F1G4", "MacBook-Aluminum",
    { 0x02, 0x01, 0x0f, 0, 1, 0x75 },  "j31", "j31", 0x76006 }, // need EPCI
  //MacBookPro9,2 / MacBook Pro (13-inch, Mid 2012)
  { "MacBookPro9,2", "MBP91.88Z.F000.B00.2002052053", "231.0.0.0.0", "Mac-6F01561E16C75D06", // Intel Core i5-3210M @ 2.50 GHz
    "MacBook Pro", "1.0", "C02HA041DTY3", "MacBook-Aluminum",
    { 0x02, 0x02, 0x0f, 0, 0, 0x44 },  "branch", "j30", 0x76006 },
  //MacBookPro10,1 / MacBook Pro (Retina, 15-inch, Early 2013)
  { "MacBookPro10,1", "MBP101.88Z.F000.B00.2002051902", "260.0.0.0.0", "Mac-C3EC7CD22292981F", // Intel Core i7-3740QM @ 2.70 GHz
    "MacBook Pro", "1.0", "C02LHHACFFT4", "MacBook-Aluminum",
    { 0x02, 0x03, 0x0f, 0, 0, 0x36 },  "d2", "d2", 0x74006 },
  //MacBookPro10,2 / MacBook Pro (Retina, 13-inch, Early 2013)
  { "MacBookPro10,2", "MBP102.88Z.F000.B00.2002052052", "283.0.0.0.0", "Mac-AFD8A9D944EA4843", // Intel Core i5-3230M @ 2.60 GHz
    "MacBook Pro", "1.0", "C02K4HACFFRP", "MacBook-Aluminum",
    { 0x02, 0x06, 0x0f, 0, 0, 0x59 },  "branch", "d1", 0x73007 },
  //MacBookPro11,1 / MacBook Pro (Retina, 13-inch, Mid 2014)
  { "MacBookPro11,1", "MBP111.88Z.F000.B00.2002052022", "159.0.0.0.0", "Mac-189A3D4F975D5FFC", // Intel Core i7-4558U @ 2.80 GHz
    "MacBook Pro", "1.0", "C17N4HACG3QJ", "MacBook-Aluminum",
    { 0x02, 0x16, 0x0f, 0, 0, 0x68 },  "j44", "j44", 0xf0b007 },
  //MacBookPro11,2 / MacBook Pro (Retina, 15-inch, Mid 2014)
  { "MacBookPro11,2", "MBP112.88Z.F000.B00.2002051908", "159.0.0.0.0", "Mac-3CBD00234E554E41", // Intel Core i7-4750HQ @ 2.00 GHz
    "MacBook Pro", "1.0", "C02P9HACG9FT", "MacBook-Aluminum",
    { 0x02, 0x18, 0x0f, 0, 0, 0x15 },  "j45", "j45", 0xf0b007 }, // need EPCI
  //MacBookPro11,3 / MacBook Pro (Retina, 15-inch, Mid 2014)
  { "MacBookPro11,3", "MBP112.88Z.F000.B00.2002051908", "159.0.0.0.0", "Mac-2BD1B31983FE1663", // Intel Core i7-4870HQ @ 2.50 GHz
    "MacBook Pro", "1.0", "C02NNHACG3QP", "MacBook-Aluminum",
    { 0x02, 0x19, 0x0f, 0, 0, 0x12 },  "j45g", "j45g", 0xf0d007 },
  //MacBookPro11,4 / MacBook Pro (Retina, 15-inch, Mid 2015)
  { "MacBookPro11,4", "MBP114.88Z.F000.B00.2002051744", "197.0.0.0.0", "Mac-06F11FD93F0323C5", // Intel Core i7-4770HQ @ 2.20 GHz
    "MacBook Pro", "1.0", "C02Q7HACG8WL", "MacBook-Aluminum",
    { 0x02, 0x29, 0x0f, 0, 0, 0x23 },  "j145", "j145", 0xf07008 },
  //MacBookPro11,5 / MacBook Pro (Retina, 15-inch, Mid 2015)
  { "MacBookPro11,5", "MBP114.88Z.F000.B00.2002051744", "197.0.0.0.0", "Mac-06F11F11946D27C5", // Intel Core i7-4870HQ @ 2.50 GHz
    "MacBook Pro", "1.0", "C02Q3HACG8WM", "MacBook-Aluminum",
    { 0x02, 0x30, 0x0f, 0, 0, 0x02 },  "NA", "NA", 0xf0b007 }, // need rBR RPlt EPCI
  //MacBookPro12,1 / MacBook Pro (Retina, 13-inch, Early 2015)
  { "MacBookPro12,1", "MBP121.88Z.F000.B00.2002051745", "190.0.0.0.0", "Mac-E43C1C25D4880AD6", // Intel Core i5-5257U @ 2.70 GHz
    "MacBook Pro", "1.0", "C02Q51OSH1DP", "MacBook-Aluminum",
    { 0x02, 0x28, 0x0f, 0, 0, 0x07 },  "j52", "j52", 0xf01008 },
  //MacBookPro13,1 / MacBook Pro (13-inch, 2016, Two Thunderbolt 3 ports)
  { "MacBookPro13,1", "MBP131.88Z.F000.B00.2001311752", "243.0.0.0.0", "Mac-473D31EABEB93F9B", // Intel Core i5-6360U @ 2.00 GHz
    "MacBook Pro", "1.0", "C02SLHACGVC1", "MacBook-Aluminum",
    { 0x02, 0x36, 0x0f, 0, 0, 0x98 },  "2016mb", "j130", 0xf02009 }, // need EPCI
  //MacBookPro13,2 / MacBook Pro (13-inch, 2016, Four Thunderbolt 3 Ports)
  { "MacBookPro13,2", "MBP132.88Z.F000.B00.2001311618", "265.0.0.0.0", "Mac-66E35819EE2D0D05", // Intel Core i5-6287U @ 3.10 GHz
    "MacBook Pro", "1.0", "C02SLHACGYFH", "MacBook-Aluminum",
    { 0x02, 0x37, 0x0f, 0, 0, 0x21 },  "2016mb", "j79", 0xf02009 },
  //MacBookPro13,3 / MacBook Pro (15-inch, 2016)
  { "MacBookPro13,3", "MBP133.88Z.F000.B00.2001311618", "265.0.0.0.0", "Mac-A5C67F76ED83108C", // Intel Core i7-6920HQ @ 2.90 GHz
    "MacBook Pro", "1.0", "C02SLHACGTFN", "MacBook-Aluminum",
    { 0x02, 0x38, 0x0f, 0, 0, 0x08 },  "2016mb", "j80g", 0xf04009 },
  //MacBookPro14,1 / MacBook Pro (13-inch, 2017, Two Thunderbolt 3 ports)
  { "MacBookPro14,1", "MBP141.88Z.F000.B00.2001311754", "205.0.0.0.0", "Mac-B4831CEBD52A0C4C", // Intel Core i5-7360U @ 2.30 GHz
    "MacBook Pro", "1.0", "C02TNHACHV29", "MacBook-Aluminum",
    { 0x02, 0x43, 0x0f, 0, 0, 0x07 },  "2017mbp", "j130a", 0xf0b009 },
  //MacBookPro14,2 / MacBook Pro (13-inch, 2017, Four Thunderbolt 3 Ports)
  { "MacBookPro14,2", "MBP142.88Z.F000.B00.2001311618", "205.0.0.0.0", "Mac-CAD6701F7CEA0921", // Intel Core i5-7267U @ 3.09 GHz
    "MacBook Pro", "1.0", "C02TQHACHV2N", "MacBook-Aluminum",
    { 0x02, 0x44, 0x0f, 0, 0, 0x02 },  "2017mbp", "j79a", 0xf09009 },
  //MacBookPro14,3 / MacBook Pro (15-inch, 2017)
  { "MacBookPro14,3", "MBP143.88Z.F000.B00.2001311754", "205.0.0.0.0", "Mac-551B86E5744E2388", // Intel Core i7-7700HQ @ 2.80 GHz
    "MacBook Pro", "1.0", "C02TQHACHTD5", "MacBook-Aluminum",
    { 0x02, 0x45, 0x0f, 0, 0, 0x01 },  "2017mbp", "j80ga", 0xf0a009 },
  //MacBookPro15,1 / MacBook Pro (15-inch, 2018)
  { "MacBookPro15,1", "MBP151.88Z.F000.B00.2003170204", "1037.100.359.0.0", "Mac-937A206F2EE63C01", // Intel Core i9-8950HK @ 2.90 GHz
    "MacBook Pro", "1.0", "C02X1HACKGYG", "MacBook-Aluminum",
    { 0, 0, 0, 0, 0, 0 }, NULL, "j680", 0 },
  //MacBookPro15,2 / MacBook Pro (13-inch, 2018, Four Thunderbolt 3 Ports)
  { "MacBookPro15,2", "MBP152.88Z.F000.B00.2003170152", "1037.100.359.0.0", "Mac-827FB448E656EC26", // Intel Core i5-8259U @ 2.30 GHz
    "MacBook Pro", "1.0", "C02X1HACJHCD", "MacBook-Aluminum",
    { 0, 0, 0, 0, 0, 0 }, NULL, "j132", 0 },
  //MacBookPro15,3 / MacBook Pro (15-inch, 2019)
  { "MacBookPro15,3", "MBP153.88Z.F000.B00.2003170137", "1037.100.359.0.0", "Mac-1E7E29AD0135F9BC", // Intel Core i9-9980HK @ 2.40 GHz
    "MacBook Pro", "1.0", "C02X1HACLVCG", "MacBook-Aluminum",
    { 0, 0, 0, 0, 0, 0 }, NULL, "j780", 0 },
  //MacBookPro15,4 / MacBook Pro (13-inch, 2019, Two Thunderbolt 3 ports)
  { "MacBookPro15,4", "MBP154.88Z.F000.B00.2003170058", "1037.100.359.0.0", "Mac-53FDB3D8DB8CA971", // Intel Core i7-8557U @ 1.70 GHz
    "MacBook Pro", "1.0", "FVFYXHACL411", "MacBook-Aluminum",
    { 0, 0, 0, 0, 0, 0 }, NULL, "j213", 0 },
  //MacBookPro16,1 / MacBook Pro (16-inch, 2019)
  { "MacBookPro16,1", "MBP161.88Z.F000.B00.2003170228", "1037.100.359.0.0", "Mac-E1008331FDC96864", // Intel Core i9-9980HK @ 2.40 GHz
    "MacBook Pro", "1.0", "C02ZPHACPG8W", "MacBook-Aluminum",
    { 0, 0, 0, 0, 0, 0 }, NULL, "j152f", 0 },
  //MacBookAir1,1 / MacBook Air (Original)
  { "MacBookAir1,1", "MBA11.88Z.00BB.B03.0803171226", NULL, "Mac-F42C8CC8", // Intel Core 2 Duo P7500 @ 1.60 GHz
    "MacBook Air", "1.0", "W864947A18X", "Air-Enclosure",
    { 0x01, 0x23, 0x0f, 0, 0, 0x20 },  "NA", "NA", 0x76005 }, // need rBR RPlt EPCI
  //MacBookAir2,1 / MacBook Air (Mid 2009)
  { "MacBookAir2,1", "MBA21.88Z.0075.B05.1003051506", NULL, "Mac-F42D88C8", // Intel Core 2 Duo L9600 @ 2.13 GHz
    "MacBook Air", "1.0", "W86494769A7", "Air-Enclosure",
    { 0x01, 0x34, 0x0f, 0, 0, 0x08 },  "NA", "NA", 0x76005 }, // need rBR RPlt EPCI
  //MacBookAir3,1 / MacBook Air (11-inch, Late 2010)
  { "MacBookAir3,1", "MBA31.88Z.F000.B00.1906132254", "110.0.0.0.0", "Mac-942452F5819B1C1B", // Intel Core 2 Duo U9600 @ 1.60 GHz
    "MacBook Air", "1.0", "C02DNHACDDR0", "Air-Enclosure",
    { 0x01, 0x67, 0x0f, 0, 0, 0x10 },  "k16k99", "k99", 0x76005 },
  //MacBookAir3,2 / MacBook Air (13-inch, Late 2010)
  { "MacBookAir3,2", "MBA31.88Z.F000.B00.1906132254", "110.0.0.0.0", "Mac-942C5DF58193131B", // Intel Core 2 Duo L9600 @ 2.13 GHz
    "MacBook Air", "1.0", "C02DRHACDDR3", "Air-Enclosure",
    { 0x01, 0x66, 0x0f, 0, 0, 0x61 },  "NA", "NA", 0x76005 }, // need rBR RPlt EPCI
  //MacBookAir4,1 / MacBook Air (11-inch, Mid 2011)
  { "MacBookAir4,1", "MBA41.88Z.F000.B00.1906140715", "135.0.0.0.0", "Mac-C08A6BB70A942AC2", // Intel Core i7-2677M @ 1.80 GHz
    "MacBook Air", "1.0", "C02HVHACDJYC", "Air-Enclosure",
    { 0x01, 0x74, 0x0f, 0, 0, 0x04 },  "k21k78", "k78", 0x76005 }, // need EPCI
  //MacBookAir4,2 / MacBook Air (13-inch, Mid 2011)
  { "MacBookAir4,2", "MBA41.88Z.F000.B00.1906140715", "135.0.0.0.0", "Mac-742912EFDBEE19B3", // Intel Core i5-2557M @ 1.70 GHz
    "MacBook Air", "1.0", "C02GLHACDJWT", "Air-Enclosure",
    { 0x01, 0x73, 0x0f, 0, 0, 0x66 },  "k21k78", "k21", 0x76005 }, // need EPCI
  //MacBookAir5,1 / MacBook Air (11-inch, Mid 2012)
  { "MacBookAir5,1", "MBA51.88Z.F000.B00.2002051902", "262.0.0.0.0", "Mac-66F35F19FE2A0D05", // Intel Core i7-3667U @ 2.00 GHz
    "MacBook Air", "1.0", "C02J6HACDRV6", "Air-Enclosure",
    { 0x02, 0x04, 0x0f, 0, 0, 0x19 },  "j11j13", "j11", 0x7b006 }, // need EPCI
  //MacBookAir5,2 / MacBook Air (13-inch, Mid 2012)
  { "MacBookAir5,2", "MBA51.88Z.F000.B00.2002051902", "262.0.0.0.0", "Mac-2E6FAB96566FE58C", // Intel Core i5-3427U @ 1.80 GHz
    "MacBook Air", "1.0", "C02HA041DRVC", "Air-Enclosure",
    { 0x02, 0x05, 0x0f, 0, 0, 0x09 },  "j11j13", "j13", 0x7b006 },
  //MacBookAir6,1 / MacBook Air (11-inch, Mid 2013)
  { "MacBookAir6,1", "MBA61.88Z.F000.B00.2002052050", "120.0.0.0.0", "Mac-35C1E88140C3E6CF", // Intel Core i7-4650U @ 1.70 GHz
    "MacBook Air", "1.0", "C2QM6HACFKYN", "Air-Enclosure",
    { 0x02, 0x12, 0x0f, 0, 1, 0x43 },  "j41j43", "j41", 0x7b007 }, // need EPCI
  //MacBookAir6,2 / MacBook Air (13-inch, Mid 2013)
  { "MacBookAir6,2", "MBA61.88Z.F000.B00.2002052050", "120.0.0.0.0", "Mac-7DF21CB3ED6977E5", // Intel Core i5-4250U @ 1.30 GHz
    "MacBook Air", "1.0", "C02L9HACF5V7", "Air-Enclosure",
    { 0x02, 0x13, 0x0f, 0, 0, 0x15 },  "j41j43", "j43", 0x7b007 },
  //MacBookAir7,1 / MacBook Air (11-inch, Early 2015)
  { "MacBookAir7,1", "MBA71.88Z.F000.B00.2002051744", "193.0.0.0.0", "Mac-9F18E312C5C2BF0B", // Intel Core i5-5250U @ 1.60 GHz
    "MacBook Air", "1.0", "C02PVHACGFWL", "Air-Enclosure",
    { 0x02, 0x26, 0x0f, 0, 0, 0x02 },  "j110", "j110", 0x7b007 },
  //MacBookAir7,2 / MacBook Air (13-inch, Early 2015)
  { "MacBookAir7,2", "MBA71.88Z.F000.B00.2002051744", "193.0.0.0.0", "Mac-937CB26E2E02BB01", // Intel Core i7-5650U @ 2.20 GHz
    "MacBook Air", "1.0", "C02Q1HACG940", "Air-Enclosure",
    { 0x02, 0x27, 0x0f, 0, 0, 0x02 },  "j113", "j113", 0xf0a008 },
  //MacBookAir8,1 / MacBook Air (Retina, 13-inch, 2018)
  { "MacBookAir8,1", "MBA81.88Z.F000.B00.2003170019", "1037.100.359.0.0", "Mac-827FAC58A8FDFA22", // Intel Core i5-8210Y @ 1.60 GHz
    "MacBook Air", "1.0", "FVFXJHACJK77", "Air-Enclosure",
    { 0, 0, 0, 0, 0, 0 }, NULL, "j140k", 0 },
  //MacBookAir8,2 / MacBook Air (Retina, 13-inch, 2019)
  { "MacBookAir8,2", "MBA82.88Z.F000.B00.2003170153", "1037.100.359.0.0", "Mac-226CB3C6A851A671", // Intel Core i5-8210Y @ 1.60 GHz
    "MacBook Air", "1.0", "FVFXJHACLYWM", "Air-Enclosure",
    { 0, 0, 0, 0, 0, 0 }, NULL, "j140a", 0 },
  //MacBookAir9,1 / MacBook Air (Retina, 13-inch, 2020)
  { "MacBookAir9,1", "J230K.88Z.F000.B00.2003170036", "1037.100.359.0.0", "Mac-0CFF9C7C2B63DF8D", // Intel Core i5-1030NG7 @ 1.10 GHz
    "MacBook Air", "1.0", "FVFCCHACMNHP", "Air-Enclosure",
    { 0, 0, 0, 0, 0, 0 }, NULL, "J230K", 0 },
  //Macmini1,1 / Mac mini (Early 2006)
  { "Macmini1,1", "MM11.88Z.0055.B08.0610121326", NULL, "Mac-F4208EC8", // Intel Core 2 Duo T2300 @ 1.67 GHz
    "Mac mini", "1.0", "W8702N1JU35", "Mini-Aluminum",
    { 0x01, 0x03, 0x0f, 0, 0, 0x04 },  "m40", "m40", 0x78002 }, // need EPCI
  //Macmini2,1 / Mac mini (Mid 2007)
  { "Macmini2,1", "MM21.88Z.009A.B00.0706281359", NULL, "Mac-F4208EAA", // Intel Core 2 Duo T7200 @ 2.00 GHz
    "Mac mini", "1.1", "W8705W9LYL2", "Mini-Aluminum",
    { 0x01, 0x19, 0x0f, 0, 0, 0x02 },  "NA", "NA", 0x78002 }, // need rBR EPCI
  //Macmini3,1 / Mac mini (Late 2009)
  { "Macmini3,1", "MM31.88Z.0081.B06.0904271717", NULL, "Mac-F22C86C8", // Intel Core 2 Duo P8700 @ 2.53 GHz
    "Mac mini", "1.0", "YM003HAC9G6", "Mini-Aluminum",
    { 0x01, 0x35, 0x0f, 0, 0, 0x01 },  "NA", "NA", 0x78002 }, // need rBR RPlt EPCI
  //Macmini4,1 / Mac mini (Mid 2010)
  { "Macmini4,1", "MM41.88Z.F000.B00.1906132344", "76.0.0.0.0", "Mac-F2208EC8", // Intel Core 2 Duo P8800 @ 2.66 GHz
    "Mac mini", "1.0", "C02FHBBEDD6H", "Mini-Aluminum",
    { 0x01, 0x65, 0x0f, 0, 0, 0x02 },  "NA", "NA", 0x78002 }, // need rBR RPlt EPCI
  //Macmini5,1 / Mac mini (Mid 2011)
  { "Macmini5,1", "MM51.88Z.F000.B00.1906131918", "135.0.0.0.0", "Mac-8ED6AF5B48C039E1", // Intel Core i5-2415M @ 2.30 GHz
    "Mac mini", "1.0", "C07GA041DJD0", "Mini-Aluminum",
    { 0x01, 0x30, 0x0f, 0, 0, 0x03 },  "NA", "NA", 0x7d005 }, // need rBR EPCI
  //Macmini5,2 / Mac mini (Mid 2011)
  { "Macmini5,2", "MM51.88Z.F000.B00.1906131918", "135.0.0.0.0", "Mac-4BC72D62AD45599E", // Intel Core i7-2620M @ 2.70 GHz
    "Mac mini", "1.0", "C07HVHACDJD1", "Mini-Aluminum",
    { 0x01, 0x75, 0x0f, 0, 0, 0x00 },  "NA", "NA", 0x7d005 }, // need rBR RPlt EPCI
  //Macmini5,3 / Mac mini Server (Mid 2011)
  { "Macmini5,3", "MM51.88Z.F000.B00.1906131918", "135.0.0.0.0", "Mac-7BA5B2794B2CDB12", // Intel Core i7-2635QM @ 2.00 GHz
    "Mac mini", "1.0", "C07GWHACDKDJ", "Mini-Aluminum",
    { 0x01, 0x77, 0x0f, 0, 0, 0x00 },  "NA", "NA", 0x7d005 }, // need rBR RPlt EPCI
  //Macmini6,1 / Mac mini (Late 2012)
  { "Macmini6,1", "MM61.88Z.F000.B00.2002051859", "283.0.0.0.0", "Mac-031AEE4D24BFF0B1", // Intel Core i5-3210M @ 2.50 GHz
    "Mac mini", "1.0", "C07JNHACDY3H", "Mini-Aluminum",
    { 0x02, 0x07, 0x0f, 0, 0, 0x00 },  "NA", "NA", 0x7d006 }, // need rBR RPlt EPCI
  //Macmini6,2 / Mac mini Server (Late 2012)
  { "Macmini6,2", "MM61.88Z.F000.B00.2002051859", "283.0.0.0.0", "Mac-F65AE981FFA204ED", // Intel Core i7-3615QM @ 2.30 GHz
    "Mac mini", "1.0", "C07JD041DWYN", "Mini-Aluminum",
    { 0x02, 0x08, 0x0f, 0, 0, 0x01 },  "j50s", "j50s", 0x7d006 },
  //Macmini7,1 / Mac mini (Late 2014)
  { "Macmini7,1", "MM71.88Z.F000.B00.2002051745", "247.0.0.0.0", "Mac-35C5E08120C7EEAF", // Intel Core i5-4278U @ 2.60 GHz
    "Mac mini", "1.0", "C02NN7NHG1J0", "Mini-Aluminum",
    { 0x02, 0x24, 0x0f, 0, 0, 0x32 },  "j64", "j64", 0xf04008 },
  //Macmini8,1 / Mac mini (2018)
  { "Macmini8,1", "MM81.88Z.F000.B00.2003170108", "1037.100.359.0.0", "Mac-7BA5B2DFE22DDD8C", // Intel Core i7-8700B @ 3.20 GHz
    "Mac mini", "1.0", "C07XL9WEJYVX", "Mini-Aluminum",
    { 0, 0, 0, 0, 0, 0 }, NULL, "j174", 0 },
  //iMac4,1 / iMac (20-inch, Early 2006)
  { "iMac4,1", "IM41.88Z.0055.B08.0609061538", NULL, "Mac-F42786C8", // Intel Core 2 Duo T2500 @ 2.00 GHz
    "iMac", "1.0", "W8610HACVGM", "iMac",
    { 0x01, 0x01, 0x0f, 0, 0, 0x05 },  "m38m39", "m38", 0x73002 }, // need EPCI
  //iMac4,2 / iMac (17-inch, Late 2006 CD)
  { "iMac4,2", "IM42.88Z.0071.B03.0610121320", NULL, "Mac-F4218EC8", // Intel Core 2 Duo T2400 @ 1.83 GHz
    "iMac", "1.0", "W8727HACWH5", "iMac",
    { 0x01, 0x06, 0x0f, 0, 0, 0x00 },  "NA", "NA", 0x73002 }, // need rBR RPlt EPCI
  //iMac5,1 / iMac (20-inch, Late 2006)
  { "iMac5,1", "IM51.88Z.0090.B09.0706270921", NULL, "Mac-F4228EC8", // Intel Core 2 Duo T7400 @ 2.16 GHz
    "iMac", "1.0", "CK708HACVUW", "iMac",
    { 0x01, 0x08, 0x0f, 0, 0, 0x02 },  "NA", "NA", 0x73002 }, // need rBR RPlt EPCI
  //iMac5,2 / iMac (17-inch, Late 2006 CD)
  { "iMac5,2", "IM52.88Z.0090.B09.0706270913", NULL, "Mac-F4218EC8", // Intel Core 2 Duo T5600 @ 1.83 GHz
    "iMac", "1.0", "QP702HACWH4", "iMac",
    { 0x01, 0x06, 0x0f, 0, 0, 0x00 },  "NA", "NA", 0x73002 }, // need rBR RPlt EPCI
  //iMac6,1 / iMac (24-inch, Late 2006)
  { "iMac6,1", "IM61.88Z.0093.B07.0804281538", NULL, "Mac-F4218FC8", // Intel Core 2 Duo T7600 @ 2.33 GHz
    "iMac", "1.0", "QP708HACXA6", "iMac",
    { 0x01, 0x08, 0x0f, 0, 0, 0x02 },  "NA", "NA", 0x73002 }, // need rBR RPlt EPCI
  //iMac7,1 / iMac (20-inch, Mid 2007)
  { "iMac7,1", "IM71.88Z.007A.B03.0803051705", NULL, "Mac-F42386C8", // Intel Core 2 Extreme X7900 @ 2.80 GHz
    "iMac", "1.0", "W8739HACX85", "iMac-Aluminum",
    { 0x01, 0x20, 0x0f, 0, 0, 0x04 },  "NA", "NA", 0x73002 }, // need rBR RPlt EPCI
  //iMac8,1 / iMac (24-inch, Early 2008)
  { "iMac8,1", "IM81.88Z.00C1.B00.0802091538", NULL, "Mac-F227BEC8", // Intel Core 2 Duo E8235 @ 2.80 GHz
    "iMac", "1.3", "QP849HACZE7", "iMac-Aluminum",
    { 0x01, 0x29, 0x0f, 0, 0, 0x01 },  "k3", "k3", 0x73002 },
  //iMac9,1 / iMac (24-inch, Early 2009)
  { "iMac9,1", "IM91.88Z.008D.B08.0904271717", NULL, "Mac-F2218FA9", // Intel Core 2 Duo E8435 @ 3.06 GHz
    "iMac", "1.0", "W8919HAC0TG", "iMac-Aluminum",
    { 0x01, 0x36, 0x0f, 0, 0, 0x03 },  "NA", "NA", 0x73002 }, // need rBR RPlt EPCI
  //iMac10,1 / iMac (27-inch, Late 2009)
  { "iMac10,1", "IM101.88Z.F000.B00.1906141458", "215.0.0.0.0", "Mac-F2268CC8", // Intel Core 2 Duo E7600 @ 3.06 GHz
    "iMac", "1.0", "W80AA98A5PE", "iMac-Aluminum",
    { 0x01, 0x53, 0x0f, 0, 0, 0x13 },  "k22k23", "k23", 0x7b002 },
  //iMac11,1 / iMac (27-inch, Late 2009)
  { "iMac11,1", "IM111.88Z.F000.B00.1906132358", "63.0.0.0.0", "Mac-F2268DAE", // Intel Core i7-860 @ 2.80 GHz
    "iMac", "1.0", "G8942B1V5PJ", "iMac-Aluminum",
    { 0x01, 0x54, 0x0f, 0, 0, 0x36 },  "k23f", "k23f", 0x7b004 },
  //iMac11,2 / iMac (21.5-inch, Mid 2010)
  { "iMac11,2", "IM112.88Z.F000.B00.1906132310", "99.0.0.0.0", "Mac-F2238AC8", // Intel Core i3-540 @ 3.06 GHz
    "iMac", "1.2", "W8034342DB7", "iMac-Aluminum",
    { 0x01, 0x64, 0x0f, 0, 0, 0x05 },  "k74", "k74", 0x7c004 },
  //iMac11,3 / iMac (27-inch, Mid 2010)
  { "iMac11,3", "IM112.88Z.F000.B00.1906132310", "99.0.0.0.0", "Mac-F2238BAE", // Intel Core i5-760 @ 2.80 GHz
    "iMac", "1.0", "QP0312PBDNR", "iMac-Aluminum",
    { 0x01, 0x59, 0x0f, 0, 0, 0x02 },  "k74", "k74", 0x7d004 },
  //iMac12,1 / iMac (21.5-inch, Mid 2011)
  { "iMac12,1", "IM121.88Z.F000.B00.1906140041", "87.0.0.0.0", "Mac-942B5BF58194151B", // Intel Core i7-2600S @ 2.80 GHz
    "iMac", "1.9", "W80CF65ADHJF", "iMac-Aluminum",
    { 0x01, 0x71, 0x0f, 0, 0, 0x22 },  "k60", "k60", 0x73005 },
  //iMac12,2 / iMac (27-inch, Mid 2011)
  { "iMac12,2", "IM121.88Z.F000.B00.1906140041", "87.0.0.0.0", "Mac-942B59F58194171B", // Intel Core i5-2500S @ 2.70 GHz
    "iMac", "1.9", "W88GG136DHJQ", "iMac-Aluminum",
    { 0x01, 0x72, 0x0f, 0, 0, 0x02 },  "k62", "k62", 0x75005 },
  //iMac13,1 / iMac (21.5-inch, Late 2012)
  { "iMac13,1", "IM131.88Z.F000.B00.2002052050", "290.0.0.0.0", "Mac-00BE6ED71E35EB86", // Intel Core i7-3770S @ 3.10 GHz
    "iMac", "1.0", "C02JA041DNCT", "iMac-Aluminum",
    { 0x02, 0x09, 0x0f, 0, 0, 0x05 },  "d8", "d8", 0x78006 },
  //iMac13,2 / iMac (27-inch, Late 2012)
  { "iMac13,2", "IM131.88Z.F000.B00.2002052050", "290.0.0.0.0", "Mac-FC02E91DDD3FA6A4", // Intel Core i5-3470 @ 3.20 GHz
    "iMac", "1.0", "C02JB041DNCW", "iMac-Aluminum",
    { 0x02, 0x11, 0x0f, 0, 0, 0x16 },  "d8", "d8", 0x79006 },
  //iMac13,3 / iMac (21.5-inch, Early 2013) - not exists in server
  { "iMac13,3", "IM131.88Z.F000.B00.2002052050", "290.0.0.0.0", "Mac-7DF2A3B5E5D671ED", // Intel Core i3-3225 @ 3.30 GHz
    "iMac", "1.0", "C02KVHACFFYV", "iMac-Aluminum",
    { 0x02, 0x13, 0x0f, 0, 0, 0x15 },  "d8", "d8", 0x79006 }, // need EPCI
  //iMac14,1 / iMac (21.5-inch, Late 2013)
  { "iMac14,1", "IM141.88Z.F000.B00.2002051900", "142.0.0.0.0", "Mac-031B6874CF7F642A", // Intel Core i5-4570R @ 2.70 GHz
    "iMac", "1.0", "D25LHACKF8J2", "iMac-Aluminum",
    { 0x02, 0x14, 0x0f, 0, 0, 0x24 },  "j16j17", "j16", 0x79007 },
  //iMac14,2 / iMac (27-inch, Late 2013)
  { "iMac14,2", "IM142.88Z.F000.B00.2002051900", "142.0.0.0.0", "Mac-27ADBB7B4CEE8E61", // Intel Core i5-4570 @ 3.20 GHz
    "iMac", "1.0", "D25LHACKF8JC", "iMac-Aluminum",
    { 0x02, 0x15, 0x0f, 0, 0, 0x07 },  "j16j17", "j17", 0x7a007 },
  //iMac14,3 / iMac (21.5-inch, Late 2013)
  { "iMac14,3", "IM143.88Z.F000.B00.2002051900", "142.0.0.0.0", "Mac-77EB7D7DAF985301", // Intel Core i5-4570S @ 2.90 GHz
    "iMac", "1.0", "D25LHACKF8J3", "iMac-Aluminum",
    { 0x02, 0x17, 0x0f, 0, 0, 0x07 },  "j16g", "j16g", 0x7a007 }, // need EPCI
  //iMac14,4 / iMac (21.5-inch, Mid 2014)
  { "iMac14,4", "IM144.88Z.F000.B00.2002051909", "202.0.0.0.0", "Mac-81E3E92DD6088272", // Intel Core i5-4260U @ 1.40 GHz
    "iMac", "1.0", "D25LHACKFY0T", "iMac-Aluminum",
    { 0x02, 0x21, 0x0f, 0, 0, 0x92 },  "j70", "j70", 0x7a007 }, // need EPCI
  //iMac15,1 / iMac (Retina 5K, 27-inch, Mid 2015)
  { "iMac15,1", "IM151.88Z.F000.B00.2002052035", "233.0.0.0.0", "Mac-42FD25EABCABB274", // Intel Core i5-4690 @ 3.50 GHz
    "iMac", "1.0", "C02Q6HACFY10", "iMac-Aluminum", // i5: Mac-42FD25EABCABB274, i7: Mac-FA842E06C61E91C5
    { 0x02, 0x22, 0x0f, 0, 0, 0x16 },  "j78j78am", "j78", 0xf00008 }, // i5: 2.22f16, i7: 2.23f11
  //iMac16,1 / iMac (21.5-inch, Late 2015)
  { "iMac16,1", "IM161.88Z.F000.B00.2002051745", "233.0.0.0.0", "Mac-A369DDC4E67F1C45", // Intel Core i5-5250U @ 1.60 GHz
    "iMac", "1.0", "C02QQHACGF1J", "iMac-Aluminum",
    { 0x02, 0x31, 0x0f, 0, 0, 0x37 },  "j117", "j117", 0xf00008 }, // need EPCI
  //iMac16,2 / iMac (Retina 4K, 21.5-inch, Late 2015)
  { "iMac16,2", "IM162.88Z.F000.B00.2002051745", "233.0.0.0.0", "Mac-FFE5EF870D7BA81A", // Intel Core i5-5575R @ 2.80 GHz
    "iMac", "1.0", "C02PNHACGG78", "iMac-Aluminum",
    { 0x02, 0x32, 0x0f, 0, 0, 0x21 },  "j94", "j94", 0xf00008 }, // need EPCI
  //iMac17,1 / iMac (Retina 5K, 27-inch, Late 2015)
  { "iMac17,1", "IM171.88Z.F000.B00.2001311733", "176.0.0.0.0", "Mac-B809C3757DA9BB8D", // Intel Core i7-6700K @ 4.00 GHz
    "iMac17,1", "1.0", "C02QFHACGG7L", "iMac-Aluminum", // i5: Mac-65CE76090165799A/Mac-DB15BD556843C820, i7: Mac-B809C3757DA9BB8D
    { 0x02, 0x33, 0x0f, 0, 0, 0x12 },  "j95j95am", "j95", 0xf0c008 }, //Note: why? i5: 2.33f12 but for i7: 2.34f3
  //iMac18,1 / iMac (21.5-inch, 2017)
  { "iMac18,1", "IM181.88Z.F000.B00.2001311754", "181.0.0.0.0", "Mac-4B682C642B45593E", // Intel Core i5-7360U @ 2.30 GHz
    "iMac", "1.0", "C02TDHACH7JY", "iMac-Aluminum",
    { 0x02, 0x39, 0x0f, 0, 0, 0x40 },  "j133_4_5", "j135", 0xf07009 }, // need RPlt EPCI
  //iMac18,2 / iMac (Retina 4K, 21.5-inch, 2017)
  { "iMac18,2", "IM183.88Z.F000.B00.2001311618", "181.0.0.0.0", "Mac-77F17D7DA9285301", // Intel Core i5-7500 @ 3.40 GHz/i5-7400 @ 3.00GHz
    "iMac", "1.0", "C02TDHACJ1G5", "iMac-Aluminum",
    { 0x02, 0x40, 0x0f, 0, 0, 0x01 },  "j133_4_5", "j134", 0xf06009 }, 
  //iMac18,3 / iMac (Retina 5K, 27-inch, 2017)
  { "iMac18,3", "IM183.88Z.F000.B00.2001311618", "181.0.0.0.0", "Mac-BE088AF8C5EB4FA2", // Intel Core i7-7700K @ 4.20 GHz
    "iMac", "1.0", "C02TDHACJ1GJ", "iMac-Aluminum",
    { 0x02, 0x41, 0x0f, 0, 0, 0x02 },  "j133_4_5", "j135", 0xf07009 },
  //iMac19,1 / iMac (Retina 5K, 27-inch, 2019)
  { "iMac19,1", "IM191.88Z.F000.B00.2002172336", "1037.100.345.0.0", "Mac-AA95B1DDAB278B95", // Intel Core i9-9900K @ 3.60 GHz
    "iMac", "1.0", "C02Y9HACJV3P", "iMac-Aluminum",
    { 0x02, 0x46, 0x0f, 0, 0, 0x12 },  "j138_9", "j138", 0xf0d009 },
  //iMac19,2 / iMac (Retina 4K, 21.5-inch, 2019)
  { "iMac19,2", "IM191.88Z.F000.B00.2002172336", "1037.100.345.0.0", "Mac-63001698E7A34814", // Intel Core i7-8700B @ 3.20 GHz
    "iMac", "1.0", "C02Y9HACJWDW", "iMac-Aluminum",
    { 0x02, 0x47, 0x0f, 0, 0, 0x03 },  "j138_9", "j138", 0xf0d009 },
  //iMacPro1,1 /iMac Pro (2017)
  { "iMacPro1,1", "IMP11.88Z.F000.B00.2003170108", "1037.100.359.0.0", "Mac-7BA5B2D9E42DDD94", // Intel Xeon W-2140B CPU @ 3.20 GHz
    "iMac Pro", "1.0", "C02VVHACHX87", "iMacPro-Aluminum",
    { 0, 0, 0, 0, 0, 0 }, NULL, "j137", 0 },
  //MacPro1,1 / Mac Pro
  { "MacPro1,1", "MP11.88Z.005C.B08.0707021221", NULL, "Mac-F4208DC8", // Intel Xeon X5355 @ 2.66 GHz x2
    "MacPro", "1.0", "W88A7HACUQ2", "Pro-Enclosure",
    { 0x01, 0x07, 0x0f, 0, 0, 0x10 },  "m43", "m43", 0x79001 }, // need EPCI
  //MacPro2,1 / Mac Pro
  { "MacPro2,1", "MP21.88Z.007F.B06.0707021348", NULL, "Mac-F4208DA9", // Intel Xeon X5365 @ 2.99 GHz x2
    "MacPro", "1.0", "W8930518UPZ", "Pro-Enclosure",
    { 0x01, 0x15, 0x0f, 0, 0, 0x03 },  "m43a", "m43a", 0x79001 }, // need EPCI
  //MacPro3,1 / Mac Pro (Early 2008)
  { "MacPro3,1", "MP31.88Z.006C.B05.0802291410", NULL, "Mac-F42C88C8", // Intel Xeon E5462 @ 2.80 GHz x2
    "MacPro", "1.3", "W88A77AA5J4", "Pro-Enclosure",
    { 0x01, 0x30, 0x0f, 0, 0, 0x03 },  "m86", "m86", 0x79001 },
  //MacPro4,1 / Mac Pro (Early 2009)
  { "MacPro4,1", "MP41.88Z.0081.B08.1001221313", NULL, "Mac-F221BEC8", // Intel Xeon X5670 @ 2.93 GHz x2
    "MacPro", "1.4", "CT930HAC4PD", "Pro-Enclosure",
    { 0x01, 0x39, 0x0f, 0, 0, 0x05 },  "NA", "NA", 0x7c002 }, // need rBR RPlt
  //MacPro5,1 / Mac Pro (Mid 2012)
  { "MacPro5,1", "MP51.88Z.F000.B00.1904121248", "144.0.0.0.0", "Mac-F221BEC8", // Intel Xeon X5675 @ 3.06 GHz x2
    "MacPro", "1.2", "C07J77F7F4MC", "Pro-Enclosure", // Note: C07J50F7F4MC CK04000AHFC CG154TB9WU3
    { 0x01, 0x39, 0x0f, 0, 0, 0x11 },  "k5", "k5", 0x7c002 },
  //MacPro6,1 / Mac Pro (Late 2013)
  { "MacPro6,1", "MP61.88Z.F000.B00.2002052032", "135.0.0.0.0", "Mac-F60DEB81FF30ACF6", // Intel Xeon E5-1650 v2 @ 3.50 GHz
    "MacPro", "1.0", "F5KLA770F9VM", "Pro-Enclosure",
    { 0x02, 0x20, 0x0f, 0, 0, 0x18 },  "j90", "j90", 0xf0f006 },
  //MacPro7,1 / Mac Pro (2019)
  { "MacPro7,1", "MP71.88Z.F000.B00.2003170019", "1037.100.359.0.0", "Mac-27AD2F918AE68F61", // Intel Xeon W-3245M CPU @ 3.20 GHz
    "MacPro", "1.0", "F5KZNHACP7QM", "Pro-Enclosure",
    { 0, 0, 0, 0, 0, 0 }, NULL, "j16O", 0 },
  //Xserve1,1 / Xserve (Late 2006)
  { "Xserve1,1", "XS11.88Z.0080.B01.0706271533", NULL, "Mac-F4208AC8", // Intel Xeon E5345 @ 2.33 GHz x2
    "Xserve", "1.0", "CK703E1EV2Q", "Xserve",
    { 0x01, 0x11, 0x0f, 0, 0, 0x05 },  "NA", "NA", 0x79001 }, // need rBR RPlt EPCI
  //Xserve2,1 / Xserve (Early 2008)
  { "Xserve2,1", "XS21.88Z.006C.B06.0804011317", NULL, "Mac-F42289C8", // Intel Xeon E5472 @ 3.00 GHz x2
    "Xserve", "1.0", "CK830DLQX8S", "Xserve",
    { 0x01, 0x26, 0x0f, 0, 0, 0x03 },  "NA", "NA", 0x79001 }, // need rBR RPlt EPCI
  //Xserve3,1 / Xserve (Early 2009)
  { "Xserve3,1", "XS31.88Z.0081.B06.0908061300", NULL, "Mac-F223BEC8", // Intel Xeon E5520 @ 2.26 GHz
    "Xserve", "1.0", "CK933YJ16HS", "Xserve",
    { 0x01, 0x43, 0x0f, 0, 0, 0x04 },  "NA", "NA", 0x79001 }, // need rBR RPlt EPCI
};

VOID SetDMISettingsForModel(MACHINE_TYPES Model, BOOLEAN Redefine)
{
  const CHAR8  *i;
  CHAR8 *Res1 = (__typeof__(Res1))AllocateZeroPool(9);
  CHAR8 *Res2 = (__typeof__(Res2))AllocateZeroPool(11);

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
      snprintf (Res1, 9, "%c%c/%c%c/%c%c\n", i[3], i[4], i[5], i[6], i[1], i[2]);
      AsciiStrCpyS (gSettings.ReleaseDate, 64, Res1);
      break;

    default:
      i = ApplePlatformData[Model].firmwareVersion;
      i += AsciiStrLen (i);

      while (*i != '.') {
        i--;
      }
      snprintf (Res2, 11, "%c%c/%c%c/20%c%c\n", i[3], i[4], i[5], i[6], i[1], i[2]);
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
    case MacMini81:
      gFwFeatures             = 0xFD8FF466;
      break;
    case MacBookAir61:
    case MacBookAir62:
    case iMac141:
    case iMac142:
    case iMac143:
      gFwFeatures             = 0xE00FE137;
      break;
    case MacBookPro111:
    case MacBookPro112:
    case MacBookPro113:
    case MacBookPro114:
    case MacBookPro115:
      gFwFeatures             = 0xE80FE137;
      break;
    case iMac144:
      gFwFeatures             = 0xF00FE177;
      break;
    case iMac151:
      gFwFeatures             = 0xF80FE177;
      break;
    case MacBookPro131:
    case MacBookPro132:
    case MacBookPro141:
    case MacBookPro142:
    case iMac171:
    case iMac181:
    case iMac182:
    case iMac183:
      gFwFeatures             = 0xFC0FE176;
      break;
    case MacBook91:
    case MacBook101:
    case MacBookPro133:
    case MacBookPro143:
      gFwFeatures             = 0xFC0FE17E;
      break;
    case iMacPro11:
      gFwFeatures             = 0xFD8FF53F;
      break;
    case MacBookAir91:
      gFwFeatures             = 0xFD8FF42E;
      break;
    case iMac191:
    case iMac192:
      gFwFeatures             = 0xFD8FF576;
      break;

    // Verified list from Users
    case MacBookAir31:
    case MacBookAir32:
    case MacMini41:
      gFwFeatures             = 0xD00DE137;
      break;
    case MacBookAir71:
    case MacBookAir72:
      gFwFeatures             = 0xE00FE137;
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
    case MacPro51:
      gFwFeatures             = 0xE80FE137;
      break;
    case MacPro61:
      gFwFeatures             = 0xE80FE176;
      break;
    case MacPro71:
      gFwFeatures             = 0xFD8FF53F;
      break;
    case MacBookPro61:
    case MacBookPro62:
    case MacBookPro71:
    case MacBookPro81:
    case MacBookPro82:
    case MacBookPro83:
      gFwFeatures             = 0xC00DE137;
      break;
    case MacBookPro121:
    case MacBookPro151:
    case MacBookPro152:
    case MacBookPro153:
    case MacBookPro154:
    case MacBookPro161:
    case MacBookAir81:
    case MacBookAir82:
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
    case MacBookPro91:
    case MacBookPro92:
    case MacBookPro101:
    case MacBookPro102:
    case MacBookPro111:
    case MacBookPro112:
    case MacBookPro113:
    case MacBookPro114:
    case MacBookPro115:
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
      gFwFeaturesMask         = 0xFF1FFF3F;
      break;
          
    case MacBook91:
    case MacBook101:
    case MacBookPro131:
    case MacBookPro132:
    case MacBookPro133:
    case MacBookPro141:
    case MacBookPro142:
    case MacBookPro143:
    case iMac144:
    case iMac151:
    case iMac171:
    case iMac181:
    case iMac182:
    case iMac183:
    case MacPro61:
      gFwFeaturesMask         = 0xFF1FFF7F;
      break;
    case iMacPro11:
    case MacBookAir91:
      gFwFeaturesMask         = 0xFF9FFF3F;
      break;
    case iMac191:
    case iMac192:
    case MacMini81:
      gFwFeaturesMask         = 0xFFDFFF7F;
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
    case MacBookPro153:
    case MacBookPro154:
    case MacBookPro161:
    case MacBookAir31:
    case MacBookAir32:
    case MacBookAir71:
    case MacBookAir72:
    case MacBookAir81:
    case MacBookAir82:
    case MacMini41:
    case MacMini71:
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

    case MacPro71:
      gFwFeaturesMask         = 0xFF9FFF3F;
      break;

    default:
      gFwFeaturesMask         = 0xFFFFFFFF; //unknown - use oem SMBIOS value to be default
      break;
  }
  
  // PlatformFeature
  // the memory tab in About This Mac
  // by TheRacerMaster
  switch (Model) {
    // Verified list from ioreg
    case iMac171:
    case iMac181:
    case iMac182:
    case iMac183:
    case iMacPro11: // ' '
    case MacPro71:
      gPlatformFeature        = 0x00;
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
      gPlatformFeature        = 0x01;
      break;
    case MacBookPro111:
    case MacBookPro112:
    case MacBookPro113:
    case MacBookPro114:
    case MacBookPro115:
    case MacBookPro121:
    case MacBookPro151: // '2'
    case MacBookPro152: // '2'
      gPlatformFeature        = 0x02;
      break;
    case MacMini71:
    case iMac161:
    case iMac162:
      gPlatformFeature        = 0x03;
      break;
    case MacPro61:
      gPlatformFeature        = 0x04;
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
      gPlatformFeature        = 0x1A;
      break;
    case MacMini81:
      gPlatformFeature        = 0x20;
      break;
    case iMac191:
    case iMac192:
      gPlatformFeature        = 0x22;
      break;
    case MacBookAir81:
    case MacBookAir82:
    case MacBookAir91:
      gPlatformFeature        = 0x3A;
      break;
          
    // Verified list from Users
    case MacBookPro153:
    case MacBookPro154:
    case MacBookPro161:
      gPlatformFeature        = 0x02;
      break;

    default:
      gPlatformFeature        = 0xFFFF; // disabled
      break;
  }

  if (Model > MacPro31) {
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
    case MacBookPro153:
    case MacBookPro154:
    case MacBookPro161:
    case MacBookAir71:
    case MacBookAir72:
    case MacBookAir81:
    case MacBookAir82:
    case MacBookAir91:
    case MacMini81:
    case iMac161:
    case iMac162:
    case iMac171:
    case iMac181:
    case iMac182:
    case iMac183:
    case iMac191:
    case iMac192:
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
        case iMac191:
        case iMac192:
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
    case MacPro71:
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
  MACHINE_TYPES i;

  for (i = (MACHINE_TYPES)(0); i < MaxMachineType; i = (MACHINE_TYPES)(i + 1)) {
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
//  MemSet(gSettings.BooterCfgStr, 64, 0);
//  AsciiStrCpyS(gSettings.BooterCfgStr, 64, "log=0");
  CHAR8 *OldCfgStr = (__typeof__(OldCfgStr))GetNvramVariable (L"bootercfg", &gEfiAppleBootGuid, NULL, NULL);
  if (OldCfgStr) {
    AsciiStrCpyS(gSettings.BooterCfgStr, 64, OldCfgStr);
    FreePool(OldCfgStr);
  }
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
  //MsgLog ("Turbo default value: %s\n", gCPUStructure.Turbo ? "Yes" : "No");
  //msr                            = AsmReadMsr64(MSR_IA32_MISC_ENABLE);
  //force enable EIST
  //msr                            |= (1<<16);
  //AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, msr);
  //gSettings.Turbo                = ((msr & (1ULL<<38)) == 0);
  //gSettings.EnableISS            = ((msr & (1ULL<<16)) != 0);

  //Fill ACPI table list
  //  GetAcpiTablesList ();
}
