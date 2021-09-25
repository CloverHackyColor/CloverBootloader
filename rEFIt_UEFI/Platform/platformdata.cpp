/**
 platformdata.c
 **/

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "nvidia.h"
#include "smbios.h"
#include "cpu.h"
#include "Nvram.h"
#include "guid.h"

/* Machine Default Data */


#ifndef DEBUG_PLATFORMDATA
#ifndef DEBUG_ALL
#define DEBUG_PLATFORMDATA 1
#else
#define DEBUG_PLATFORMDATA DEBUG_ALL
#endif
#endif

#if DEBUG_PLATFORMDATA==0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_PLATFORMDATA, __VA_ARGS__)
#endif

// All SMBIOS data were updated by Sherlocks, PMheart.
// FredWst supported SmcExtract.

// Refactored to single data structure by RehabMan

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
  { "MacBook1,1"_XS8, "MB11.88Z.0061.B03.0610121324"_XS8, ""_XS8, "Mac-F4208CC8"_XS8, // Intel Core Duo T2500 @ 2.00 GHz
    "MacBook"_XS8, "1.1"_XS8, "4H625HACVTH"_XS8, "MacBook-White"_XS8,
    0x01, 0x04, 0x0f, 0, 0, 0x12, "branch"_XS8, "m70"_XS8, 0x71001 },
  //MacBook2,1 / MacBook (13-inch Late 2006)
  { "MacBook2,1"_XS8, "MB21.88Z.00A5.B07.0706270922"_XS8, ""_XS8, "Mac-F4208CA9"_XS8, // Intel Core 2 Duo T7200 @ 2.00 GHz
    "MacBook"_XS8, "1.2"_XS8, "W8713HACWGL"_XS8, "MacBook-White"_XS8,
    0x01, 0x13, 0x0f, 0, 0, 0x03, "branch"_XS8, "m75"_XS8, 0x72001 },
  //MacBook3,1 / MacBook (13-inch Late 2007)
  { "MacBook3,1"_XS8, "MB31.88Z.008E.B02.0803051832"_XS8, ""_XS8, "Mac-F22788C8"_XS8, // Intel Core 2 Duo T7500 @ 2.20 GHz
    "MacBook"_XS8, "1.3"_XS8, "W8747HACZ63"_XS8, "MacBook-White"_XS8,
    0x01, 0x24, 0x0f, 0, 0, 0x03, "branch"_XS8, "k36"_XS8, 0x72001 }, // need EPCI
  //MacBook4,1 / MacBook (13-inch, Early 2008)
  { "MacBook4,1"_XS8, "MB41.88Z.00C1.B00.0802091535"_XS8, ""_XS8, "Mac-F22788A9"_XS8, // Intel Core 2 Duo T8300 @ 2.40 GHz
    "MacBook"_XS8, "1.3"_XS8, "W88A041A0P0"_XS8, "MacBook-Black"_XS8,
    0x01, 0x31, 0x0f, 0, 0, 0x01, "branch"_XS8, "m82"_XS8, 0x74001 },
  //MacBook5,1 / MacBook (13-inch, Aluminum, Late 2008)
  { "MacBook5,1"_XS8, "MB51.88Z.007D.B03.0904271443"_XS8, ""_XS8, "Mac-F42D89C8"_XS8, // Intel Core 2 Duo P7350 @ 2.00 GHz
    "MacBook"_XS8, "1.3"_XS8, "W8944T1S1AQ"_XS8, "MacBook-Aluminum"_XS8,
    0x01, 0x32, 0x0f, 0, 0, 0x08, "branch"_XS8, "m97"_XS8, 0x7a002 }, // need EPCI
  //MacBook5,2 / MacBook (13-inch, Early 2009)
  { "MacBook5,2"_XS8, "MB52.88Z.0088.B05.0904162222"_XS8, ""_XS8, "Mac-F22788AA"_XS8, // Intel Core 2 Duo P7450 @ 2.13 GHz
    "MacBook"_XS8, "1.3"_XS8, "W8913HAC4R1"_XS8, "MacBook-Black"_XS8,
    0x01, 0x38, 0x0f, 0, 0, 0x05, "branch"_XS8, "k36b"_XS8, 0x7a002 },
  //MacBook6,1 / MacBook (13-inch, Late 2009)
  { "MacBook6,1"_XS8, "MB61.88Z.F000.B00.1906140014"_XS8, "209.0.0.0.0"_XS8, "Mac-F22C8AC8"_XS8, // Intel Core 2 Duo P7550 @ 2.26 GHz
    "MacBook"_XS8, "1.0"_XS8, "451131JCGAY"_XS8, "MacBook-White"_XS8,
    0x01, 0x51, 0x0f, 0, 0, 0x53, "k84"_XS8, "k84"_XS8, 0x72004 },
  //MacBook7,1 / MacBook (13-inch, Mid 2010)
  { "MacBook7,1"_XS8, "MB71.88Z.F000.B00.1906140026"_XS8, "68.0.0.0.0"_XS8, "Mac-F22C89C8"_XS8, // Intel Core 2 Duo P8600 @ 2.40 GHz
    "MacBook"_XS8, "1.0"_XS8, "451211MEF5X"_XS8, "MacBook-White"_XS8,
    0x01, 0x60, 0x0f, 0, 0, 0x06, "k87"_XS8, "k87"_XS8, 0x72005 },
  //MacBook8,1 / MacBook (Retina, 12-inch, Early 2015)
  { "MacBook8,1"_XS8, "MB81.88Z.F000.B00.2106131842"_XS8, "427.140.8.0.0"_XS8, "Mac-BE0E8AC46FE800CC"_XS8, // Intel Core M-5Y51 @ 1.20 GHz
    "MacBook"_XS8, "1.0"_XS8, "C02RCE58GCN3"_XS8, "MacBook-Aluminum"_XS8,
    0x02, 0x25, 0x0f, 0, 0, 0x87, "j92"_XS8, "j92"_XS8, 0xf0e007 },
  //MacBook9,1 / MacBook (Retina, 12-inch, Early 2016)
  { "MacBook9,1"_XS8, "MB91.88Z.F000.B00.2106131743"_XS8, "429.140.8.0.0"_XS8, "Mac-9AE82516C7C6B903"_XS8, // Intel Core m5-6Y54 @ 1.20 GHz
    "MacBook"_XS8, "1.0"_XS8, "C02RM408HDNK"_XS8, "MacBook-Aluminum"_XS8,
    0x02, 0x35, 0x0f, 0, 1, 0x06, "j93"_XS8, "j93"_XS8, 0xf0e007 }, // need EPCI
  //MacBook10,1 / MacBook (Retina, 12-inch, 2017)
  { "MacBook10,1"_XS8, "MB101.88Z.F000.B00.2106131834"_XS8, "429.140.8.0.0"_XS8, "Mac-EE2EBD4B90B839A8"_XS8, // Intel Core i5-7Y54 @ 1.30 GHz
    "MacBook"_XS8, "1.0"_XS8, "C02TQHACHH27"_XS8, "MacBook-Aluminum"_XS8,
    0x02, 0x42, 0x0f, 0, 0, 0x12, "j122"_XS8, "j122"_XS8, 0xf08009 }, // need EPCI
  //MacBookPro1,1 / MacBook Pro (15-inch Glossy)
  { "MacBookPro1,1"_XS8, "MBP11.88Z.0055.B08.0610121325"_XS8, ""_XS8, "Mac-F425BEC8"_XS8, // Intel Core Duo T2500 @ 2.00 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "W8634HACVWZ"_XS8, "MacBook-Aluminum"_XS8,
    0x01, 0x02, 0x0f, 0, 0, 0x10, "m1"_XS8, "m1"_XS8, 0x7b002 }, // need EPCI
  //MacBookPro1,2 / MacBook Pro (17-inch)
  { "MacBookPro1,2"_XS8, "MBP12.88Z.0061.B03.0610121334"_XS8, ""_XS8, "Mac-F42DBEC8"_XS8, // Intel Core Duo T2600 @ 2.17 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "W8629HACTHY"_XS8, "MacBook-Aluminum"_XS8,
    0x01, 0x05, 0x0f, 0, 0, 0x10, "NA"_XS8, "NA"_XS8, 0x7b002 }, // need rBR RPlt EPCI
  //MacBookPro2,1 / MacBook Pro (17-inch Core 2 Duo)
  { "MacBookPro2,1"_XS8, "MBP21.88Z.00A5.B08.0708131242"_XS8, ""_XS8, "Mac-F42189C8"_XS8, // Intel Core 2 Duo T7600 @ 2.33 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "W8715HACW0J"_XS8, "MacBook-Aluminum"_XS8,
    0x01, 0x14, 0x0f, 0, 0, 0x05, "NA"_XS8, "NA"_XS8, 0x7b002 }, // need rBR RPlt EPCI
  //MacBookPro2,2 / MacBook Pro (15-inch Core 2 Duo)
  { "MacBookPro2,2"_XS8, "MBP22.88Z.00A5.B07.0708131242"_XS8, ""_XS8, "Mac-F42187C8"_XS8, // Intel Core 2 Duo T7400 @ 2.16 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "W8827B4CW0L"_XS8, "MacBook-Aluminum"_XS8,
    0x01, 0x13, 0x0f, 0, 0, 0x03, "NA"_XS8, "NA"_XS8, 0x7b002 }, // need rBR RPlt EPCI
  //MacBookPro3,1 / MacBook Pro (17-inch 2.4GHZ) - not exists in server
  { "MacBookPro3,1"_XS8, "MBP31.88Z.0070.B07.0803051658"_XS8, ""_XS8, "Mac-F4238BC8"_XS8, // Intel Core 2 Duo T7700 @ 2.40 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "W8841OHZX91"_XS8, "MacBook-Aluminum"_XS8,
    0x01, 0x16, 0x0f, 0, 0, 0x11, "NA"_XS8, "NA"_XS8, 0x7b002 }, // need rBR RPlt EPCI
  //MacBookPro4,1 / MacBook Pro (17-inch, Early 2008)
  { "MacBookPro4,1"_XS8, "MBP41.88Z.00C1.B03.0802271651"_XS8, ""_XS8, "Mac-F42C89C8"_XS8, // Intel Core 2 Duo T9500 @ 2.60 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "W88484F2YP4"_XS8, "MacBook-Aluminum"_XS8,
    0x01, 0x27, 0x0f, 0, 0, 0x03, "m87"_XS8, "m87"_XS8, 0x7b002 }, // need EPCI
  //MacBookPro5,1 / MacBook Pro (15-inch, Late 2008)
  { "MacBookPro5,1"_XS8, "MBP51.88Z.007E.B06.1202061253"_XS8, ""_XS8, "Mac-F42D86C8"_XS8, // Intel Core 2 Duo P8600 @ 2.40 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "W88439FE1G0"_XS8, "MacBook-Aluminum"_XS8,
    0x01, 0x33, 0x0f, 0, 0, 0x08, "m98"_XS8, "m98"_XS8, 0x7b002 },
  //MacBookPro5,2 / MacBook Pro (17-inch, Early 2009)
  { "MacBookPro5,2"_XS8, "MBP52.88Z.008E.B05.0905042202"_XS8, ""_XS8, "Mac-F2268EC8"_XS8, // Intel Core 2 Duo T9600 @ 2.80 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "W8908HAC2QP"_XS8, "MacBook-Aluminum"_XS8,
    0x01, 0x42, 0x0f, 0, 0, 0x04, "NA"_XS8, "NA"_XS8, 0x7b002 }, // need rBR RPlt EPCI
  //MacBookPro5,3 / MacBook Pro (15-inch, Mid 2009)
  { "MacBookPro5,3"_XS8, "MBP53.88Z.00AC.B03.0906151647"_XS8, ""_XS8, "Mac-F22587C8"_XS8, // Intel Core 2 Duo P8800 @ 2.66 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "W89E6HAC64C"_XS8, "MacBook-Aluminum"_XS8,
    0x01, 0x48, 0x0f, 0, 0, 0x02, "NA"_XS8, "NA"_XS8, 0x7b002 }, // need rBR RPlt EPCI
  //MacBookPro5,4 / MacBook Pro (15-inch, 2.53GHz, Mid 2009)
  { "MacBookPro5,4"_XS8, "MBP53.88Z.00AC.B03.0906151647"_XS8, ""_XS8, "Mac-F22587A1"_XS8, // Intel Core 2 Duo P8700 @ 2.53 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "W8948HAC7XJ"_XS8, "MacBook-Aluminum"_XS8,
    0x01, 0x49, 0x0f, 0, 0, 0x02, "NA"_XS8, "NA"_XS8, 0x7b002 }, // need rBR RPlt EPCI
  //MacBookPro5,5 / MacBook Pro (13-inch, Mid 2009)
  { "MacBookPro5,5"_XS8, "MBP55.88Z.00AC.B03.0906151708"_XS8, ""_XS8, "Mac-F2268AC8"_XS8, // Intel Core 2 Duo P7550 @ 2.26 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "W8951HAC66E"_XS8, "MacBook-Aluminum"_XS8,
    0x01, 0x47, 0x0f, 0, 0, 0x02, "branch"_XS8, "k24"_XS8, 0x7a003 },
  //MacBookPro6,1 / MacBook Pro (17-inch, Mid 2010)
  { "MacBookPro6,1"_XS8, "MBP61.88Z.F000.B00.1906132235"_XS8, "99.0.0.0.0"_XS8, "Mac-F22589C8"_XS8, // Intel Core i5-540M @ 2.53 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "C02G5834DC79"_XS8, "MacBook-Aluminum"_XS8,
    0x01, 0x57, 0x0f, 0, 0, 0x18, "k17"_XS8, "k17"_XS8, 0x7a004 }, // need EPCI
  //MacBookPro6,2 / MacBook Pro (15-inch, Mid 2010)
  { "MacBookPro6,2"_XS8, "MBP61.88Z.F000.B00.1906132235"_XS8, "99.0.0.0.0"_XS8, "Mac-F22586C8"_XS8, // Intel Core i7-620M @ 2.66 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "CK132A91AGW"_XS8, "MacBook-Aluminum"_XS8,
    0x01, 0x58, 0x0f, 0, 0, 0x17, "k74"_XS8, "k74"_XS8, 0x7a004 },
  //MacBookPro7,1 / MacBook Pro (13-inch, Mid 2010)
  { "MacBookPro7,1"_XS8, "MBP71.88Z.F000.B00.1906132329"_XS8, "68.0.0.0.0"_XS8, "Mac-F222BEC8"_XS8, // Intel Core 2 Duo P8600 @ 2.40 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "CK145C7NATM"_XS8, "MacBook-Aluminum"_XS8,
    0x01, 0x62, 0x0f, 0, 0, 0x07, "branch"_XS8, "k6"_XS8, 0x7a004 }, // need EPCI
  //MacBookPro8,1 / MacBook Pro (13-inch, Early 2011)
  { "MacBookPro8,1"_XS8, "MBP81.88Z.F000.B00.1906132217"_XS8, "87.0.0.0.0"_XS8, "Mac-94245B3640C91C81"_XS8, // Intel Core i5-2415M @ 2.30 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "W89F9196DH2G"_XS8, "MacBook-Aluminum"_XS8,
    0x01, 0x68, 0x0f, 0, 0, 0x99, "k90i"_XS8, "k90i"_XS8, 0x7b005 },
  //MacBookPro8,2 / MacBook Pro (15-inch, Early 2011)
  { "MacBookPro8,2"_XS8, "MBP81.88Z.0050.B00.1906132217"_XS8, "87.0.0.0.0"_XS8, "Mac-94245A3940C91C80"_XS8, // Intel Core i7-2675QM @ 2.20 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "C02HL0FGDF8X"_XS8, "MacBook-Aluminum"_XS8,
    0x01, 0x69, 0x0f, 0, 0, 0x04, "trunk"_XS8, "k91f"_XS8, 0x7b005 }, // need EPCI
  //MacBookPro8,3 / MacBook Pro (17-inch, Early 2011)
  { "MacBookPro8,3"_XS8, "MBP81.88Z.0050.B00.1906132217"_XS8, "87.0.0.0.0"_XS8, "Mac-942459F5819B171B"_XS8, // Intel Core i7-2860QM @ 2.49 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "W88F9CDEDF93"_XS8, "MacBook-Aluminum"_XS8,
    0x01, 0x70, 0x0f, 0, 0, 0x06, "k92"_XS8, "k92"_XS8, 0x7c005 },
  //MacBookPro9,1 / MacBook Pro (15-inch, Mid 2012)
  { "MacBookPro9,1"_XS8, "MBP91.88Z.F000.B00.2106041716"_XS8, "422.0.0.0.0"_XS8, "Mac-4B7AC7E43945597E"_XS8, // Intel Core i7-3720QM @ 2.60 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "C02LW984F1G4"_XS8, "MacBook-Aluminum"_XS8,
    0x02, 0x01, 0x0f, 0, 1, 0x75, "j31"_XS8, "j31"_XS8, 0x76006 }, // need EPCI
  //MacBookPro9,2 / MacBook Pro (13-inch, Mid 2012)
  { "MacBookPro9,2"_XS8, "MBP91.88Z.F000.B00.2106041716"_XS8, "422.0.0.0.0"_XS8, "Mac-6F01561E16C75D06"_XS8, // Intel Core i5-3210M @ 2.50 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "C02HA041DTY3"_XS8, "MacBook-Aluminum"_XS8,
    0x02, 0x02, 0x0f, 0, 0, 0x44, "branch"_XS8, "j30"_XS8, 0x76006 },
  //MacBookPro10,1 / MacBook Pro (Retina, 15-inch, Early 2013)
  { "MacBookPro10,1"_XS8, "MBP101.88Z.F000.B00.2106041718"_XS8, "422.0.0.0.0"_XS8, "Mac-C3EC7CD22292981F"_XS8, // Intel Core i7-3740QM @ 2.70 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "C02LHHACFFT4"_XS8, "MacBook-Aluminum"_XS8,
    0x02, 0x03, 0x0f, 0, 0, 0x36, "d2"_XS8, "d2"_XS8, 0x74006 },
  //MacBookPro10,2 / MacBook Pro (Retina, 13-inch, Early 2013)
  { "MacBookPro10,2"_XS8, "MBP102.88Z.F000.B00.2106041717"_XS8, "422.0.0.0.0"_XS8, "Mac-AFD8A9D944EA4843"_XS8, // Intel Core i5-3230M @ 2.60 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "C02K4HACFFRP"_XS8, "MacBook-Aluminum"_XS8,
    0x02, 0x06, 0x0f, 0, 0, 0x59, "branch"_XS8, "d1"_XS8, 0x73007 },
  //MacBookPro11,1 / MacBook Pro (Retina, 13-inch, Mid 2014)
  { "MacBookPro11,1"_XS8, "MBP111.88Z.F000.B00.2106131844"_XS8, "431.140.6.0.0"_XS8, "Mac-189A3D4F975D5FFC"_XS8, // Intel Core i7-4558U @ 2.80 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "C17N4HACG3QJ"_XS8, "MacBook-Aluminum"_XS8,
    0x02, 0x16, 0x0f, 0, 0, 0x68, "j44"_XS8, "j44"_XS8, 0xf0b007 },
  //MacBookPro11,2 / MacBook Pro (Retina, 15-inch, Mid 2014)
  { "MacBookPro11,2"_XS8, "MBP112.88Z.F000.B00.2106131836"_XS8, "431.140.6.0.0"_XS8, "Mac-3CBD00234E554E41"_XS8, // Intel Core i7-4750HQ @ 2.00 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "C02P9HACG9FT"_XS8, "MacBook-Aluminum"_XS8,
    0x02, 0x18, 0x0f, 0, 0, 0x15, "j45"_XS8, "j45"_XS8, 0xf0b007 }, // need EPCI
  //MacBookPro11,3 / MacBook Pro (Retina, 15-inch, Mid 2014)
  { "MacBookPro11,3"_XS8, "MBP112.88Z.F000.B00.2106131836"_XS8, "431.140.6.0.0"_XS8, "Mac-2BD1B31983FE1663"_XS8, // Intel Core i7-4870HQ @ 2.50 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "C02NNHACG3QP"_XS8, "MacBook-Aluminum"_XS8,
    0x02, 0x19, 0x0f, 0, 0, 0x12, "j45g"_XS8, "j45g"_XS8, 0xf0d007 },
  //MacBookPro11,4 / MacBook Pro (Retina, 15-inch, Mid 2015)
  { "MacBookPro11,4"_XS8, "MBP114.88Z.F000.B00.2106131816"_XS8, "427.140.8.0.0"_XS8, "Mac-06F11FD93F0323C5"_XS8, // Intel Core i7-4770HQ @ 2.20 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "C02Q7HACG8WL"_XS8, "MacBook-Aluminum"_XS8,
    0x02, 0x29, 0x0f, 0, 0, 0x23, "j145"_XS8, "j145"_XS8, 0xf07008 },
  //MacBookPro11,5 / MacBook Pro (Retina, 15-inch, Mid 2015)
  { "MacBookPro11,5"_XS8, "MBP114.88Z.F000.B00.2106131816"_XS8, "427.140.8.0.0"_XS8, "Mac-06F11F11946D27C5"_XS8, // Intel Core i7-4870HQ @ 2.50 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "C02Q3HACG8WM"_XS8, "MacBook-Aluminum"_XS8,
    0x02, 0x30, 0x0f, 0, 0, 0x02, "NA"_XS8, "NA"_XS8, 0xf0b007 }, // need rBR RPlt EPCI
  //MacBookPro12,1 / MacBook Pro (Retina, 13-inch, Early 2015)
  { "MacBookPro12,1"_XS8, "MBP121.88Z.F000.B00.2106131845"_XS8, "427.140.8.0.0"_XS8, "Mac-E43C1C25D4880AD6"_XS8, // Intel Core i5-5257U @ 2.70 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "C02Q51OSH1DP"_XS8, "MacBook-Aluminum"_XS8,
    0x02, 0x28, 0x0f, 0, 0, 0x07, "j52"_XS8, "j52"_XS8, 0xf01008 },
  //MacBookPro13,1 / MacBook Pro (13-inch, 2016, Two Thunderbolt 3 ports)
  { "MacBookPro13,1"_XS8, "MBP131.88Z.F000.B00.2106131757"_XS8, "429.140.8.0.0"_XS8, "Mac-473D31EABEB93F9B"_XS8, // Intel Core i5-6360U @ 2.00 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "C02SLHACGVC1"_XS8, "MacBook-Aluminum"_XS8,
    0x02, 0x36, 0x0f, 0, 1, 0x02, "2016mb"_XS8, "j130"_XS8, 0xf02009 }, // need EPCI
  //MacBookPro13,2 / MacBook Pro (13-inch, 2016, Four Thunderbolt 3 Ports)
  { "MacBookPro13,2"_XS8, "MBP132.88Z.F000.B00.2106131817"_XS8, "429.140.8.0.0"_XS8, "Mac-66E35819EE2D0D05"_XS8, // Intel Core i5-6287U @ 3.10 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "C02SLHACGYFH"_XS8, "MacBook-Aluminum"_XS8,
    0x02, 0x37, 0x0f, 0, 0, 0x21, "2016mb"_XS8, "j79"_XS8, 0xf02009 },
  //MacBookPro13,3 / MacBook Pro (15-inch, 2016)
  { "MacBookPro13,3"_XS8, "MBP133.88Z.F000.B00.2106131759"_XS8, "429.140.8.0.0"_XS8, "Mac-A5C67F76ED83108C"_XS8, // Intel Core i7-6920HQ @ 2.90 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "C02SLHACGTFN"_XS8, "MacBook-Aluminum"_XS8,
    0x02, 0x38, 0x0f, 0, 0, 0x08, "2016mb"_XS8, "j80g"_XS8, 0xf04009 },
  //MacBookPro14,1 / MacBook Pro (13-inch, 2017, Two Thunderbolt 3 ports)
  { "MacBookPro14,1"_XS8, "MBP141.88Z.F000.B00.2106131822"_XS8, "429.140.8.0.0"_XS8, "Mac-B4831CEBD52A0C4C"_XS8, // Intel Core i5-7360U @ 2.30 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "C02TNHACHV29"_XS8, "MacBook-Aluminum"_XS8,
    0x02, 0x43, 0x0f, 0, 0, 0x07, "2017mbp"_XS8, "j130a"_XS8, 0xf0b009 },
  //MacBookPro14,2 / MacBook Pro (13-inch, 2017, Four Thunderbolt 3 Ports)
  { "MacBookPro14,2"_XS8, "MBP142.88Z.F000.B00.2106131822"_XS8, "429.140.8.0.0"_XS8, "Mac-CAD6701F7CEA0921"_XS8, // Intel Core i5-7267U @ 3.09 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "C02TQHACHV2N"_XS8, "MacBook-Aluminum"_XS8,
    0x02, 0x44, 0x0f, 0, 0, 0x02, "2017mbp"_XS8, "j79a"_XS8, 0xf09009 },
  //MacBookPro14,3 / MacBook Pro (15-inch, 2017)
  { "MacBookPro14,3"_XS8, "MBP143.88Z.F000.B00.2106131834"_XS8, "429.140.8.0.0"_XS8, "Mac-551B86E5744E2388"_XS8, // Intel Core i7-7700HQ @ 2.80 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "C02TQHACHTD5"_XS8, "MacBook-Aluminum"_XS8,
    0x02, 0x45, 0x0f, 0, 0, 0x01, "2017mbp"_XS8, "j80ga"_XS8, 0xf0a009 },
  //MacBookPro15,1 / MacBook Pro (15-inch, 2018)
  { "MacBookPro15,1"_XS8, "MBP151.88Z.F000.B00.2107050205"_XS8, "715.0.57.0.0"_XS8, "Mac-937A206F2EE63C01"_XS8, // Intel Core i9-8950HK @ 2.90 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "C02X1HACKGYG"_XS8, "MacBook-Aluminum"_XS8,
    0, 0, 0, 0, 0, 0, ""_XS8, "j680"_XS8, 0 },
  //MacBookPro15,2 / MacBook Pro (13-inch, 2018, Four Thunderbolt 3 Ports)
  { "MacBookPro15,2"_XS8, "MBP152.88Z.F000.B00.2107050204"_XS8, "1715.0.57.0.0"_XS8, "Mac-827FB448E656EC26"_XS8, // Intel Core i5-8259U @ 2.30 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "C02X1HACJHCD"_XS8, "MacBook-Aluminum"_XS8,
    0, 0, 0, 0, 0, 0, ""_XS8, "j132"_XS8, 0 },
  //MacBookPro15,3 / MacBook Pro (15-inch, 2019)
  { "MacBookPro15,3"_XS8, "MBP153.88Z.F000.B00.2107050204"_XS8, "1715.0.57.0.0"_XS8, "Mac-1E7E29AD0135F9BC"_XS8, // Intel Core i9-9980HK @ 2.40 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "C02X1HACLVCG"_XS8, "MacBook-Aluminum"_XS8,
    0, 0, 0, 0, 0, 0, ""_XS8, "j780"_XS8, 0 },
  //MacBookPro15,4 / MacBook Pro (13-inch, 2019, Two Thunderbolt 3 ports)
  { "MacBookPro15,4"_XS8, "MBP154.88Z.F000.B00.2107050236"_XS8, "1715.0.57.0.0"_XS8, "Mac-53FDB3D8DB8CA971"_XS8, // Intel Core i7-8557U @ 1.70 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "FVFYXHACL411"_XS8, "MacBook-Aluminum"_XS8,
    0, 0, 0, 0, 0, 0, ""_XS8, "j213"_XS8, 0 },
  //MacBookPro16,1 / MacBook Pro (16-inch, 2019)
  { "MacBookPro16,1"_XS8, "MBP161.88Z.F000.B00.2107050239"_XS8, "1715.0.57.0.0"_XS8, "Mac-E1008331FDC96864"_XS8, // Intel Core i9-9980HK @ 2.40 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "C02ZPHACPG8W"_XS8, "MacBook-Aluminum"_XS8,
    0, 0, 0, 0, 0, 0, ""_XS8, "j152f"_XS8, 0 },
  //MacBookPro16,2 / MacBook Pro (13-inch, 2020, Four Thunderbolt 3 ports)
  { "MacBookPro16,2"_XS8, "MBP162.88Z.F000.B00.2107050225"_XS8, "1715.0.57.0.0"_XS8, "Mac-5F9802EFE386AA28"_XS8, // Intel Core i7-1068NG7 @ 2.30 GHz type=0x060b
    "MacBook Pro"_XS8, "1.0"_XS8, "C02CLHACML7H"_XS8, "MacBook-Aluminum"_XS8,
    0, 0, 0, 0, 0, 0, ""_XS8, "j214k"_XS8, 0 },
  //MacBookPro16,3 / MacBook Pro (13-inch, 2020, Two Thunderbolt 3 ports)
  { "MacBookPro16,3"_XS8, "MBP163.88Z.F000.B00.2107050231"_XS8, "1715.0.57.0.0"_XS8, "Mac-E7203C0F68AA0004"_XS8, // Intel Core i7-8557U @ 1.70 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "C02CJHACP3XY"_XS8, "MacBook-Aluminum"_XS8,
    0, 0, 0, 0, 0, 0, ""_XS8, "j223"_XS8, 0 },
  //MacBookPro16,4 / MacBook Pro (16-inch, 2019)  AMD Radeon Pro 5600M
  { "MacBookPro16,4"_XS8, "MBP164.88Z.F000.B00.2107050237"_XS8, "1715.0.57.0.0"_XS8, "Mac-A61BADE1FDAD7B05"_XS8, // Intel Core i9-9880H @ 2.30 GHz
    "MacBook Pro"_XS8, "1.0"_XS8, "C02CWHACMD6T"_XS8, "MacBook-Aluminum"_XS8,
    0, 0, 0, 0, 0, 0, ""_XS8, "j215"_XS8, 0 },
  //MacBookAir1,1 / MacBook Air (Original)
  { "MacBookAir1,1"_XS8, "MBA11.88Z.00BB.B03.0803171226"_XS8, ""_XS8, "Mac-F42C8CC8"_XS8, // Intel Core 2 Duo P7500 @ 1.60 GHz
    "MacBook Air"_XS8, "1.0"_XS8, "W864947A18X"_XS8, "Air-Enclosure"_XS8,
    0x01, 0x23, 0x0f, 0, 0, 0x20, "NA"_XS8, "NA"_XS8, 0x76005 }, // need rBR RPlt EPCI
  //MacBookAir2,1 / MacBook Air (Mid 2009)
  { "MacBookAir2,1"_XS8, "MBA21.88Z.0075.B05.1003051506"_XS8, ""_XS8, "Mac-F42D88C8"_XS8, // Intel Core 2 Duo L9600 @ 2.13 GHz
    "MacBook Air"_XS8, "1.0"_XS8, "W86494769A7"_XS8, "Air-Enclosure"_XS8,
    0x01, 0x34, 0x0f, 0, 0, 0x08, "NA"_XS8, "NA"_XS8, 0x76005 }, // need rBR RPlt EPCI
  //MacBookAir3,1 / MacBook Air (11-inch, Late 2010)
  { "MacBookAir3,1"_XS8, "MBA31.88Z.F000.B00.1906132254"_XS8, "110.0.0.0.0"_XS8, "Mac-942452F5819B1C1B"_XS8, // Intel Core 2 Duo U9600 @ 1.60 GHz
    "MacBook Air"_XS8, "1.0"_XS8, "C02DNHACDDR0"_XS8, "Air-Enclosure"_XS8,
    0x01, 0x67, 0x0f, 0, 0, 0x10, "k16k99"_XS8, "k99"_XS8, 0x76005 },
  //MacBookAir3,2 / MacBook Air (13-inch, Late 2010)
  { "MacBookAir3,2"_XS8, "MBA31.88Z.F000.B00.1906132254"_XS8, "110.0.0.0.0"_XS8, "Mac-942C5DF58193131B"_XS8, // Intel Core 2 Duo L9600 @ 2.13 GHz
    "MacBook Air"_XS8, "1.0"_XS8, "C02DRHACDDR3"_XS8, "Air-Enclosure"_XS8,
    0x01, 0x66, 0x0f, 0, 0, 0x61, "NA"_XS8, "NA"_XS8, 0x76005 }, // need rBR RPlt EPCI
  //MacBookAir4,1 / MacBook Air (11-inch, Mid 2011)
  { "MacBookAir4,1"_XS8, "MBA41.88Z.F000.B00.1906140715"_XS8, "135.0.0.0.0"_XS8, "Mac-C08A6BB70A942AC2"_XS8, // Intel Core i7-2677M @ 1.80 GHz
    "MacBook Air"_XS8, "1.0"_XS8, "C02HVHACDJYC"_XS8, "Air-Enclosure"_XS8,
    0x01, 0x74, 0x0f, 0, 0, 0x04, "k21k78"_XS8, "k78"_XS8, 0x76005 }, // need EPCI
  //MacBookAir4,2 / MacBook Air (13-inch, Mid 2011)
  { "MacBookAir4,2"_XS8, "MBA41.88Z.F000.B00.1906140715"_XS8, "135.0.0.0.0"_XS8, "Mac-742912EFDBEE19B3"_XS8, // Intel Core i5-2557M @ 1.70 GHz
    "MacBook Air"_XS8, "1.0"_XS8, "C02GLHACDJWT"_XS8, "Air-Enclosure"_XS8,
    0x01, 0x73, 0x0f, 0, 0, 0x66, "k21k78"_XS8, "k21"_XS8, 0x76005 }, // need EPCI
  //MacBookAir5,1 / MacBook Air (11-inch, Mid 2012)
  { "MacBookAir5,1"_XS8, "MBA51.88Z.F000.B00.2106041717"_XS8, "422.0.0.0.0"_XS8, "Mac-66F35F19FE2A0D05"_XS8, // Intel Core i7-3667U @ 2.00 GHz
    "MacBook Air"_XS8, "1.0"_XS8, "C02J6HACDRV6"_XS8, "Air-Enclosure"_XS8,
    0x02, 0x04, 0x0f, 0, 0, 0x19, "j11j13"_XS8, "j11"_XS8, 0x7b006 }, // need EPCI
  //MacBookAir5,2 / MacBook Air (13-inch, Mid 2012)
  { "MacBookAir5,2"_XS8, "MBA51.88Z.F000.B00.2106041717"_XS8, "422.0.0.0.0"_XS8, "Mac-2E6FAB96566FE58C"_XS8, // Intel Core i5-3427U @ 1.80 GHz
    "MacBook Air"_XS8, "1.0"_XS8, "C02HA041DRVC"_XS8, "Air-Enclosure"_XS8,
    0x02, 0x05, 0x0f, 0, 0, 0x09, "j11j13"_XS8, "j13"_XS8, 0x7b006 },
  //MacBookAir6,1 / MacBook Air (11-inch, Mid 2013)
  { "MacBookAir6,1"_XS8, "MBA61.88Z.F000.B00.2106131852"_XS8, "431.140.6.0.0"_XS8, "Mac-35C1E88140C3E6CF"_XS8, // Intel Core i7-4650U @ 1.70 GHz
    "MacBook Air"_XS8, "1.0"_XS8, "C2QM6HACFKYN"_XS8, "Air-Enclosure"_XS8,
    0x02, 0x12, 0x0f, 0, 1, 0x43, "j41j43"_XS8, "j41"_XS8, 0x7b007 }, // need EPCI
  //MacBookAir6,2 / MacBook Air (13-inch, Mid 2013)
  { "MacBookAir6,2"_XS8, "MBA61.88Z.F000.B00.2106131852"_XS8, "431.140.6.0.0"_XS8, "Mac-7DF21CB3ED6977E5"_XS8, // Intel Core i5-4250U @ 1.30 GHz
    "MacBook Air"_XS8, "1.0"_XS8, "C02L9HACF5V7"_XS8, "Air-Enclosure"_XS8,
    0x02, 0x13, 0x0f, 0, 0, 0x15, "j41j43"_XS8, "j43"_XS8, 0x7b007 },
  //MacBookAir7,1 / MacBook Air (11-inch, Early 2015)
  { "MacBookAir7,1"_XS8, "MBA71.88Z.F000.B00.2106131820"_XS8, "427.140.8.0.0"_XS8, "Mac-9F18E312C5C2BF0B"_XS8, // Intel Core i5-5250U @ 1.60 GHz
    "MacBook Air"_XS8, "1.0"_XS8, "C02PVHACGFWL"_XS8, "Air-Enclosure"_XS8,
    0x02, 0x26, 0x0f, 0, 0, 0x02, "j110"_XS8, "j110"_XS8, 0x7b007 },
  //MacBookAir7,2 / MacBook Air (13-inch, Early 2015)
  { "MacBookAir7,2"_XS8, "MBA71.88Z.F000.B00.2106131820"_XS8, "427.140.8.0.0"_XS8, "Mac-937CB26E2E02BB01"_XS8, // Intel Core i7-5650U @ 2.20 GHz, i5-5250U CPU @ 1.60GHz
    "MacBook Air"_XS8, "1.0"_XS8, "C02Q1HACG940"_XS8, "Air-Enclosure"_XS8,
    0x02, 0x27, 0x0f, 0, 0, 0x02, "j113"_XS8, "j113"_XS8, 0xf0a008 },
  //MacBookAir8,1 / MacBook Air (Retina, 13-inch, 2018)
  { "MacBookAir8,1"_XS8, "MBA81.88Z.F000.B00.2107050205"_XS8, "1715.0.57.0.0"_XS8, "Mac-827FAC58A8FDFA22"_XS8, // Intel Core i5-8210Y @ 1.60 GHz
    "MacBook Air"_XS8, "1.0"_XS8, "FVFXJHACJK77"_XS8, "Air-Enclosure"_XS8,
    0, 0, 0, 0, 0, 0, ""_XS8, "j140k"_XS8, 0 },
  //MacBookAir8,2 / MacBook Air (Retina, 13-inch, 2019)
  { "MacBookAir8,2"_XS8, "MBA82.88Z.F000.B00.2107050207"_XS8, "1715.0.57.0.0"_XS8, "Mac-226CB3C6A851A671"_XS8, // Intel Core i5-8210Y @ 1.60 GHz
    "MacBook Air"_XS8, "1.0"_XS8, "FVFXJHACLYWM"_XS8, "Air-Enclosure"_XS8,
    0, 0, 0, 0, 0, 0, ""_XS8, "j140a"_XS8, 0 },
  //MacBookAir9,1 / MacBook Air (Retina, 13-inch, 2020)
  { "MacBookAir9,1"_XS8, "MBA91.88Z.F000.B00.2107050235"_XS8, "1715.0.57.0.0"_XS8, "Mac-0CFF9C7C2B63DF8D"_XS8, // Intel Core i5-1030NG7 @ 1.10 GHz
    "MacBook Air"_XS8, "1.0"_XS8, "FVFCCHACMNHP"_XS8, "Air-Enclosure"_XS8,
    0, 0, 0, 0, 0, 0, ""_XS8, "j230k"_XS8, 0 },
  //Macmini1,1 / Mac mini (Early 2006)
  { "Macmini1,1"_XS8, "MM11.88Z.0055.B08.0610121326"_XS8, ""_XS8, "Mac-F4208EC8"_XS8, // Intel Core 2 Duo T2300 @ 1.67 GHz
    "Mac mini"_XS8, "1.0"_XS8, "W8702N1JU35"_XS8, "Mini-Aluminum"_XS8,
    0x01, 0x03, 0x0f, 0, 0, 0x04, "m40"_XS8, "m40"_XS8, 0x78002 }, // need EPCI
  //Macmini2,1 / Mac mini (Mid 2007)
  { "Macmini2,1"_XS8, "MM21.88Z.009A.B00.0706281359"_XS8, ""_XS8, "Mac-F4208EAA"_XS8, // Intel Core 2 Duo T7200 @ 2.00 GHz
    "Mac mini"_XS8, "1.1"_XS8, "W8705W9LYL2"_XS8, "Mini-Aluminum"_XS8,
    0x01, 0x19, 0x0f, 0, 0, 0x02, "NA"_XS8, "NA"_XS8, 0x78002 }, // need rBR EPCI
  //Macmini3,1 / Mac mini (Late 2009)
  { "Macmini3,1"_XS8, "MM31.88Z.0081.B06.0904271717"_XS8, ""_XS8, "Mac-F22C86C8"_XS8, // Intel Core 2 Duo P8700 @ 2.53 GHz
    "Mac mini"_XS8, "1.0"_XS8, "YM003HAC9G6"_XS8, "Mini-Aluminum"_XS8,
    0x01, 0x35, 0x0f, 0, 0, 0x01, "NA"_XS8, "NA"_XS8, 0x78002 }, // need rBR RPlt EPCI
  //Macmini4,1 / Mac mini (Mid 2010)
  { "Macmini4,1"_XS8, "MM41.88Z.F000.B00.1906132344"_XS8, "76.0.0.0.0"_XS8, "Mac-F2208EC8"_XS8, // Intel Core 2 Duo P8800 @ 2.66 GHz
    "Mac mini"_XS8, "1.0"_XS8, "C02FHBBEDD6H"_XS8, "Mini-Aluminum"_XS8,
    0x01, 0x65, 0x0f, 0, 0, 0x02, "NA"_XS8, "NA"_XS8, 0x78002 }, // need rBR RPlt EPCI
  //Macmini5,1 / Mac mini (Mid 2011)
  { "Macmini5,1"_XS8, "MM51.88Z.F000.B00.1906131918"_XS8, "135.0.0.0.0"_XS8, "Mac-8ED6AF5B48C039E1"_XS8, // Intel Core i5-2415M @ 2.30 GHz
    "Mac mini"_XS8, "1.0"_XS8, "C07GA041DJD0"_XS8, "Mini-Aluminum"_XS8,
    0x01, 0x30, 0x0f, 0, 0, 0x03, "NA"_XS8, "NA"_XS8, 0x7d005 }, // need rBR EPCI
  //Macmini5,2 / Mac mini (Mid 2011)
  { "Macmini5,2"_XS8, "MM51.88Z.F000.B00.1906131918"_XS8, "135.0.0.0.0"_XS8, "Mac-4BC72D62AD45599E"_XS8, // Intel Core i7-2620M @ 2.70 GHz
    "Mac mini"_XS8, "1.0"_XS8, "C07HVHACDJD1"_XS8, "Mini-Aluminum"_XS8,
    0x01, 0x75, 0x0f, 0, 0, 0x00, "NA"_XS8, "NA"_XS8, 0x7d005 }, // need rBR RPlt EPCI
  //Macmini5,3 / Mac mini Server (Mid 2011)
  { "Macmini5,3"_XS8, "MM51.88Z.F000.B00.1906131918"_XS8, "135.0.0.0.0"_XS8, "Mac-7BA5B2794B2CDB12"_XS8, // Intel Core i7-2635QM @ 2.00 GHz
    "Mac mini"_XS8, "1.0"_XS8, "C07GWHACDKDJ"_XS8, "Mini-Aluminum"_XS8,
    0x01, 0x77, 0x0f, 0, 0, 0x00, "NA"_XS8, "NA"_XS8, 0x7d005 }, // need rBR RPlt EPCI
  //Macmini6,1 / Mac mini (Late 2012)
  { "Macmini6,1"_XS8, "MM61.88Z.F000.B00.2106041717"_XS8, "422.0.0.0.0"_XS8, "Mac-031AEE4D24BFF0B1"_XS8, // Intel Core i5-3210M @ 2.50 GHz
    "Mac mini"_XS8, "1.0"_XS8, "C07JNHACDY3H"_XS8, "Mini-Aluminum"_XS8,
    0x02, 0x07, 0x0f, 0, 0, 0x00, "NA"_XS8, "NA"_XS8, 0x7d006 }, // need rBR RPlt EPCI
  //Macmini6,2 / Mac mini Server (Late 2012)
  { "Macmini6,2"_XS8, "MM61.88Z.F000.B00.2106041717"_XS8, "422.0.0.0.0"_XS8, "Mac-F65AE981FFA204ED"_XS8, // Intel Core i7-3615QM @ 2.30 GHz
    "Mac mini"_XS8, "1.0"_XS8, "C07JD041DWYN"_XS8, "Mini-Aluminum"_XS8,
    0x02, 0x08, 0x0f, 0, 0, 0x01, "j50s"_XS8, "j50s"_XS8, 0x7d006 },
  //Macmini7,1 / Mac mini (Late 2014)
  { "Macmini7,1"_XS8, "MM71.88Z.F000.B00.2106131851"_XS8, "431.140.6.0.0"_XS8, "Mac-35C5E08120C7EEAF"_XS8, // Intel Core i5-4278U @ 2.60 GHz
    "Mac mini"_XS8, "1.0"_XS8, "C02NN7NHG1J0"_XS8, "Mini-Aluminum"_XS8,
    0x02, 0x24, 0x0f, 0, 0, 0x32, "j64"_XS8, "j64"_XS8, 0xf04008 },
  //Macmini8,1 / Mac mini (2018)
  { "Macmini8,1"_XS8, "MM81.88Z.F000.B00.2107050205"_XS8, "1715.0.57.0.0"_XS8, "Mac-7BA5B2DFE22DDD8C"_XS8, // Intel Core i7-8700B @ 3.20 GHz
    "Mac mini"_XS8, "1.0"_XS8, "C07XL9WEJYVX"_XS8, "Mini-Aluminum"_XS8,
    0, 0, 0, 0, 0, 0, ""_XS8, "j174"_XS8, 0 },
  //iMac4,1 / iMac (20-inch, Early 2006)
  { "iMac4,1"_XS8, "IM41.88Z.0055.B08.0609061538"_XS8, ""_XS8, "Mac-F42786C8"_XS8, // Intel Core 2 Duo T2500 @ 2.00 GHz
    "iMac"_XS8, "1.0"_XS8, "W8610HACVGM"_XS8, "iMac"_XS8,
    0x01, 0x01, 0x0f, 0, 0, 0x05, "m38m39"_XS8, "m38"_XS8, 0x73002 }, // need EPCI
  //iMac4,2 / iMac (17-inch, Late 2006 CD)
  { "iMac4,2"_XS8, "IM42.88Z.0071.B03.0610121320"_XS8, ""_XS8, "Mac-F4218EC8"_XS8, // Intel Core 2 Duo T2400 @ 1.83 GHz
    "iMac"_XS8, "1.0"_XS8, "W8727HACWH5"_XS8, "iMac"_XS8,
    0x01, 0x06, 0x0f, 0, 0, 0x00, "NA"_XS8, "NA"_XS8, 0x73002 }, // need rBR RPlt EPCI
  //iMac5,1 / iMac (20-inch, Late 2006)
  { "iMac5,1"_XS8, "IM51.88Z.0090.B09.0706270921"_XS8, ""_XS8, "Mac-F4228EC8"_XS8, // Intel Core 2 Duo T7400 @ 2.16 GHz
    "iMac"_XS8, "1.0"_XS8, "CK708HACVUW"_XS8, "iMac"_XS8,
    0x01, 0x08, 0x0f, 0, 0, 0x02, "NA"_XS8, "NA"_XS8, 0x73002 }, // need rBR RPlt EPCI
  //iMac5,2 / iMac (17-inch, Late 2006 CD)
  { "iMac5,2"_XS8, "IM52.88Z.0090.B09.0706270913"_XS8, ""_XS8, "Mac-F4218EC8"_XS8, // Intel Core 2 Duo T5600 @ 1.83 GHz
    "iMac"_XS8, "1.0"_XS8, "QP702HACWH4"_XS8, "iMac"_XS8,
    0x01, 0x06, 0x0f, 0, 0, 0x00, "NA"_XS8, "NA"_XS8, 0x73002 }, // need rBR RPlt EPCI
  //iMac6,1 / iMac (24-inch, Late 2006)
  { "iMac6,1"_XS8, "IM61.88Z.0093.B07.0804281538"_XS8, ""_XS8, "Mac-F4218FC8"_XS8, // Intel Core 2 Duo T7600 @ 2.33 GHz
    "iMac"_XS8, "1.0"_XS8, "QP708HACXA6"_XS8, "iMac"_XS8,
    0x01, 0x08, 0x0f, 0, 0, 0x02, "NA"_XS8, "NA"_XS8, 0x73002 }, // need rBR RPlt EPCI
  //iMac7,1 / iMac (20-inch, Mid 2007)
  { "iMac7,1"_XS8, "IM71.88Z.007A.B03.0803051705"_XS8, ""_XS8, "Mac-F42386C8"_XS8, // Intel Core 2 Extreme X7900 @ 2.80 GHz
    "iMac"_XS8, "1.0"_XS8, "W8739HACX85"_XS8, "iMac-Aluminum"_XS8,
    0x01, 0x20, 0x0f, 0, 0, 0x04, "NA"_XS8, "NA"_XS8, 0x73002 }, // need rBR RPlt EPCI
  //iMac8,1 / iMac (24-inch, Early 2008)
  { "iMac8,1"_XS8, "IM81.88Z.00C1.B00.0802091538"_XS8, ""_XS8, "Mac-F227BEC8"_XS8, // Intel Core 2 Duo E8235 @ 2.80 GHz
    "iMac"_XS8, "1.3"_XS8, "QP849HACZE7"_XS8, "iMac-Aluminum"_XS8,
    0x01, 0x29, 0x0f, 0, 0, 0x01, "k3"_XS8, "k3"_XS8, 0x73002 },
  //iMac9,1 / iMac (24-inch, Early 2009)
  { "iMac9,1"_XS8, "IM91.88Z.008D.B08.0904271717"_XS8, ""_XS8, "Mac-F2218FA9"_XS8, // Intel Core 2 Duo E8435 @ 3.06 GHz
    "iMac"_XS8, "1.0"_XS8, "W8919HAC0TG"_XS8, "iMac-Aluminum"_XS8,
    0x01, 0x36, 0x0f, 0, 0, 0x03, "NA"_XS8, "NA"_XS8, 0x73002 }, // need rBR RPlt EPCI
  //iMac10,1 / iMac (27-inch, Late 2009)
  { "iMac10,1"_XS8, "IM101.88Z.F000.B00.1906141458"_XS8, "215.0.0.0.0"_XS8, "Mac-F2268CC8"_XS8, // Intel Core 2 Duo E7600 @ 3.06 GHz
    "iMac"_XS8, "1.0"_XS8, "W80AA98A5PE"_XS8, "iMac-Aluminum"_XS8,
    0x01, 0x53, 0x0f, 0, 0, 0x13, "k22k23"_XS8, "k23"_XS8, 0x7b002 },
  //iMac11,1 / iMac (27-inch, Late 2009)
  { "iMac11,1"_XS8, "IM111.88Z.F000.B00.1906132358"_XS8, "63.0.0.0.0"_XS8, "Mac-F2268DAE"_XS8, // Intel Core i7-860 @ 2.80 GHz
    "iMac"_XS8, "1.0"_XS8, "G8942B1V5PJ"_XS8, "iMac-Aluminum"_XS8,
    0x01, 0x54, 0x0f, 0, 0, 0x36, "k23f"_XS8, "k23f"_XS8, 0x7b004 },
  //iMac11,2 / iMac (21.5-inch, Mid 2010)
  { "iMac11,2"_XS8, "IM112.88Z.F000.B00.1906132310"_XS8, "99.0.0.0.0"_XS8, "Mac-F2238AC8"_XS8, // Intel Core i3-540 @ 3.06 GHz
    "iMac"_XS8, "1.2"_XS8, "W8034342DB7"_XS8, "iMac-Aluminum"_XS8,
    0x01, 0x64, 0x0f, 0, 0, 0x05, "k74"_XS8, "k74"_XS8, 0x7c004 },
  //iMac11,3 / iMac (27-inch, Mid 2010)
  { "iMac11,3"_XS8, "IM112.88Z.F000.B00.1906132310"_XS8, "99.0.0.0.0"_XS8, "Mac-F2238BAE"_XS8, // Intel Core i5-760 @ 2.80 GHz
    "iMac"_XS8, "1.0"_XS8, "QP0312PBDNR"_XS8, "iMac-Aluminum"_XS8,
    0x01, 0x59, 0x0f, 0, 0, 0x02, "k74"_XS8, "k74"_XS8, 0x7d004 },
  //iMac12,1 / iMac (21.5-inch, Mid 2011)
  { "iMac12,1"_XS8, "IM121.88Z.F000.B00.1906140041"_XS8, "87.0.0.0.0"_XS8, "Mac-942B5BF58194151B"_XS8, // Intel Core i7-2600S @ 2.80 GHz
    "iMac"_XS8, "1.9"_XS8, "W80CF65ADHJF"_XS8, "iMac-Aluminum"_XS8,
    0x01, 0x71, 0x0f, 0, 0, 0x22, "k60"_XS8, "k60"_XS8, 0x73005 },
  //iMac12,2 / iMac (27-inch, Mid 2011)
  { "iMac12,2"_XS8, "IM121.88Z.F000.B00.1906140041"_XS8, "87.0.0.0.0"_XS8, "Mac-942B59F58194171B"_XS8, // Intel Core i5-2500S @ 2.70 GHz
    "iMac"_XS8, "1.9"_XS8, "W88GG136DHJQ"_XS8, "iMac-Aluminum"_XS8,
    0x01, 0x72, 0x0f, 0, 0, 0x02, "k62"_XS8, "k62"_XS8, 0x75005 },
  //iMac13,1 / iMac (21.5-inch, Late 2012)
  { "iMac13,1"_XS8, "IM131.88Z.F000.B00.2106041716"_XS8, "422.0.0.0.0"_XS8, "Mac-00BE6ED71E35EB86"_XS8, // Intel Core i7-3770S @ 3.10 GHz
    "iMac"_XS8, "1.0"_XS8, "C02JA041DNCT"_XS8, "iMac-Aluminum"_XS8,
    0x02, 0x09, 0x0f, 0, 0, 0x05, "d8"_XS8, "d8"_XS8, 0x78006 },
  //iMac13,2 / iMac (27-inch, Late 2012)
  { "iMac13,2"_XS8, "IM131.88Z.F000.B00.2106041716"_XS8, "422.0.0.0.0"_XS8, "Mac-FC02E91DDD3FA6A4"_XS8, // Intel Core i5-3470 @ 3.20 GHz
    "iMac"_XS8, "1.0"_XS8, "C02JB041DNCW"_XS8, "iMac-Aluminum"_XS8,
    0x02, 0x11, 0x0f, 0, 0, 0x16, "d8"_XS8, "d8"_XS8, 0x79006 },
  //iMac13,3 / iMac (21.5-inch, Early 2013) - not exists in server
  { "iMac13,3"_XS8, "IM131.88Z.F000.B00.2106041716"_XS8, "422.0.0.0.0"_XS8, "Mac-7DF2A3B5E5D671ED"_XS8, // Intel Core i3-3225 @ 3.30 GHz
    "iMac"_XS8, "1.0"_XS8, "C02KVHACFFYV"_XS8, "iMac-Aluminum"_XS8,
    0x02, 0x13, 0x0f, 0, 0, 0x15, "d8"_XS8, "d8"_XS8, 0x79006 }, // need EPCI
  //iMac14,1 / iMac (21.5-inch, Late 2013)
  { "iMac14,1"_XS8, "IM141.88Z.F000.B00.2106131845"_XS8, "431.140.6.0.0"_XS8, "Mac-031B6874CF7F642A"_XS8, // Intel Core i5-4570R @ 2.70 GHz
    "iMac"_XS8, "1.0"_XS8, "D25LHACKF8J2"_XS8, "iMac-Aluminum"_XS8,
    0x02, 0x14, 0x0f, 0, 0, 0x24, "j16j17"_XS8, "j16"_XS8, 0x79007 },
  //iMac14,2 / iMac (27-inch, Late 2013)
  { "iMac14,2"_XS8, "IM142.88Z.F000.B00.2106131851"_XS8, "431.140.6.0.0"_XS8, "Mac-27ADBB7B4CEE8E61"_XS8, // Intel Core i5-4570 @ 3.20 GHz
    "iMac"_XS8, "1.0"_XS8, "D25LHACKF8JC"_XS8, "iMac-Aluminum"_XS8,
    0x02, 0x15, 0x0f, 0, 0, 0x07, "j16j17"_XS8, "j17"_XS8, 0x7a007 },
  //iMac14,3 / iMac (21.5-inch, Late 2013)
  { "iMac14,3"_XS8, "IM143.88Z.F000.B00.2106131845"_XS8, "431.140.6.0.0"_XS8, "Mac-77EB7D7DAF985301"_XS8, // Intel Core i5-4570S @ 2.90 GHz
    "iMac"_XS8, "1.0"_XS8, "D25LHACKF8J3"_XS8, "iMac-Aluminum"_XS8,
    0x02, 0x17, 0x0f, 0, 0, 0x07, "j16g"_XS8, "j16g"_XS8, 0x7a007 }, // need EPCI
  //iMac14,4 / iMac (21.5-inch, Mid 2014)
  { "iMac14,4"_XS8, "IM144.88Z.F000.B00.2106131820"_XS8, "431.140.6.0.0"_XS8, "Mac-81E3E92DD6088272"_XS8, // Intel Core i5-4260U @ 1.40 GHz
    "iMac"_XS8, "1.0"_XS8, "D25LHACKFY0T"_XS8, "iMac-Aluminum"_XS8,
    0x02, 0x21, 0x0f, 0, 0, 0x92, "j70"_XS8, "j70"_XS8, 0x7a007 }, // need EPCI
  //iMac15,1 / iMac (Retina 5K, 27-inch, Mid 2015)
  { "iMac15,1"_XS8, "IM151.88Z.F000.B00.2106131818"_XS8, "431.140.6.0.0"_XS8, "Mac-42FD25EABCABB274"_XS8, // Intel Core i5-4690 @ 3.50 GHz
    "iMac"_XS8, "1.0"_XS8, "C02Q6HACFY10"_XS8, "iMac-Aluminum"_XS8, // i5: Mac-42FD25EABCABB274, i7: Mac-FA842E06C61E91C5
    0x02, 0x22, 0x0f, 0, 0, 0x16, "j78j78am"_XS8, "j78"_XS8, 0xf00008 }, // i5: 2.22f16, i7: 2.23f11
  //iMac16,1 / iMac (21.5-inch, Late 2015)
  { "iMac16,1"_XS8, "IM161.88Z.F000.B00.2106131802"_XS8, "427.140.8.0.0"_XS8, "Mac-A369DDC4E67F1C45"_XS8, // Intel Core i5-5250U @ 1.60 GHz
    "iMac"_XS8, "1.0"_XS8, "C02QQHACGF1J"_XS8, "iMac-Aluminum"_XS8,
    0x02, 0x31, 0x0f, 0, 0, 0x37, "j117"_XS8, "j117"_XS8, 0xf00008 }, // need EPCI
  //iMac16,2 / iMac (Retina 4K, 21.5-inch, Late 2015)
  { "iMac16,2"_XS8, "IM162.88Z.F000.B00.2106131845"_XS8, "427.140.8.0.0"_XS8, "Mac-FFE5EF870D7BA81A"_XS8, // Intel Core i5-5575R @ 2.80 GHz
    "iMac"_XS8, "1.0"_XS8, "C02PNHACGG78"_XS8, "iMac-Aluminum"_XS8,
    0x02, 0x32, 0x0f, 0, 0, 0x21, "j94"_XS8, "j94"_XS8, 0xf00008 }, // need EPCI
  //iMac17,1 / iMac (Retina 5K, 27-inch, Late 2015)
  { "iMac17,1"_XS8, "IM171.88Z.F000.B00.2106131747"_XS8, "429.140.8.0.0"_XS8, "Mac-B809C3757DA9BB8D"_XS8, // Intel Core i7-6700K @ 4.00 GHz
    "iMac17,1"_XS8, "1.0"_XS8, "C02QFHACGG7L"_XS8, "iMac-Aluminum"_XS8, // i5: Mac-65CE76090165799A/Mac-DB15BD556843C820, i7: Mac-B809C3757DA9BB8D
    0x02, 0x34, 0x0f, 0, 0, 0x03, "j95j95am"_XS8, "j95"_XS8, 0xf0c008 }, //Note: why? i5: 2.33f12 but for i7: 2.34f3
  //iMac18,1 / iMac (21.5-inch, 2017)
  { "iMac18,1"_XS8, "IM181.88Z.F000.B00.2106131817"_XS8, "429.140.8.0.0"_XS8, "Mac-4B682C642B45593E"_XS8, // Intel Core i5-7360U @ 2.30 GHz
    "iMac"_XS8, "1.0"_XS8, "C02TDHACH7JY"_XS8, "iMac-Aluminum"_XS8,
    0x02, 0x39, 0x0f, 0, 0, 0x40, "j133_4_5"_XS8, "j135"_XS8, 0xf07009 }, // need RPlt EPCI
  //iMac18,2 / iMac (Retina 4K, 21.5-inch, 2017)
  { "iMac18,2"_XS8, "IM183.88Z.F000.B00.2106131835"_XS8, "429.140.8.0.0"_XS8, "Mac-77F17D7DA9285301"_XS8, // Intel Core i5-7500 @ 3.40 GHz/i5-7400 @ 3.00GHz
    "iMac"_XS8, "1.0"_XS8, "C02TDHACJ1G5"_XS8, "iMac-Aluminum"_XS8,
    0x02, 0x40, 0x0f, 0, 0, 0x01, "j133_4_5"_XS8, "j134"_XS8, 0xf06009 },
  //iMac18,3 / iMac (Retina 5K, 27-inch, 2017)
  { "iMac18,3"_XS8, "IM183.88Z.F000.B00.2106131835"_XS8, "429.140.8.0.0"_XS8, "Mac-BE088AF8C5EB4FA2"_XS8, // Intel Core i7-7700K @ 4.20 GHz
    "iMac"_XS8, "1.0"_XS8, "C02TDHACJ1GJ"_XS8, "iMac-Aluminum"_XS8,
    0x02, 0x41, 0x0f, 0, 0, 0x02, "j133_4_5"_XS8, "j135"_XS8, 0xf07009 },
  //iMac19,1 / iMac (Retina 5K, 27-inch, 2019)
  { "iMac19,1"_XS8, "IM191.88Z.F000.B00.2106222356"_XS8, "1554.140.20.0.0"_XS8, "Mac-AA95B1DDAB278B95"_XS8, // Intel Core i9-9900K @ 3.60 GHz
    "iMac"_XS8, "1.0"_XS8, "C02Y9HACJV3P"_XS8, "iMac-Aluminum"_XS8,
    0x02, 0x46, 0x0f, 0, 0, 0x12, "j138_9"_XS8, "j138"_XS8, 0xf0d009 },
  //iMac19,2 / iMac (Retina 4K, 21.5-inch, 2019)
  { "iMac19,2"_XS8, "IM191.88Z.F000.B00.2106222356"_XS8, "1554.140.20.0.0"_XS8, "Mac-63001698E7A34814"_XS8, // Intel Core i7-8700B @ 3.20 GHz
    "iMac"_XS8, "1.0"_XS8, "C02Y9HACJWDW"_XS8, "iMac-Aluminum"_XS8,
    0x02, 0x47, 0x0f, 0, 0, 0x03, "j138_9"_XS8, "j138"_XS8, 0xf0d009 },
  //iMac20,1 / iMac (Retina 5K, 27-inch, 2020) Intel Core i5-10500 @ 3.10 GHz
  { "iMac20,1"_XS8, "IM201.88Z.F000.B00.2107050239"_XS8, "1715.0.57.0.0"_XS8, "Mac-CFF7D910A743CAAF"_XS8,
    "iMac"_XS8, "1.0"_XS8, "C02D3HACPN5T"_XS8, "iMac-Aluminum"_XS8,
    0,0,0,0,0,0,""_XS8, "j185"_XS8, 0},
//    0x02, 0x46, 0x0f, 0, 0, 0x12, "j185"_XS8, "j185"_XS8, 0xf0d009 },
  //iMac20,2 / iMac (Retina 5K, 27-inch, 2020) Intel Core i9-10910 @ 3.60 GHz
  { "iMac20,2"_XS8, "IM201.88Z.F000.B00.2107050239"_XS8, "1715.0.57.0.0"_XS8, "Mac-AF89B6D9451A490B"_XS8,
    "iMac"_XS8, "1.0"_XS8, "C02D2HAC046M"_XS8, "iMac-Aluminum"_XS8,
    0,0,0,0,0,0,""_XS8, "j185"_XS8, 0},
//    0x02, 0x47, 0x0f, 0, 0, 0x03, "j185f"_XS8, "j185f"_XS8, 0xf0d009 },
  //iMacPro1,1 /iMac Pro (2017)
  { "iMacPro1,1"_XS8, "IMP11.88Z.F000.B00.2107050205"_XS8, "1715.0.57.0.0"_XS8, "Mac-7BA5B2D9E42DDD94"_XS8, // Intel Xeon W-2140B CPU @ 3.20 GHz
    "iMac Pro"_XS8, "1.0"_XS8, "C02VVHACHX87"_XS8, "iMacPro-Aluminum"_XS8,
    0, 0, 0, 0, 0, 0, ""_XS8, "j137"_XS8, 0 },
  //MacPro1,1 / Mac Pro
  { "MacPro1,1"_XS8, "MP11.88Z.005C.B08.0707021221"_XS8, ""_XS8, "Mac-F4208DC8"_XS8, // Intel Xeon X5355 @ 2.66 GHz x2
    "MacPro"_XS8, "1.0"_XS8, "W88A7HACUQ2"_XS8, "Pro-Enclosure"_XS8,
    0x01, 0x07, 0x0f, 0, 0, 0x10, "m43"_XS8, "m43"_XS8, 0x79001 }, // need EPCI
  //MacPro2,1 / Mac Pro
  { "MacPro2,1"_XS8, "MP21.88Z.007F.B06.0707021348"_XS8, ""_XS8, "Mac-F4208DA9"_XS8, // Intel Xeon X5365 @ 2.99 GHz x2
    "MacPro"_XS8, "1.0"_XS8, "W8930518UPZ"_XS8, "Pro-Enclosure"_XS8,
    0x01, 0x15, 0x0f, 0, 0, 0x03, "m43a"_XS8, "m43a"_XS8, 0x79001 }, // need EPCI
  //MacPro3,1 / Mac Pro (Early 2008)
  { "MacPro3,1"_XS8, "MP31.88Z.006C.B05.0802291410"_XS8, ""_XS8, "Mac-F42C88C8"_XS8, // Intel Xeon E5462 @ 2.80 GHz x2
    "MacPro"_XS8, "1.3"_XS8, "W88A77AA5J4"_XS8, "Pro-Enclosure"_XS8,
    0x01, 0x30, 0x0f, 0, 0, 0x03, "m86"_XS8, "m86"_XS8, 0x79001 },
  //MacPro4,1 / Mac Pro (Early 2009)
  { "MacPro4,1"_XS8, "MP41.88Z.0081.B08.1001221313"_XS8, ""_XS8, "Mac-F221BEC8"_XS8, // Intel Xeon X5670 @ 2.93 GHz x2
    "MacPro"_XS8, "1.4"_XS8, "CT930HAC4PD"_XS8, "Pro-Enclosure"_XS8,
    0x01, 0x39, 0x0f, 0, 0, 0x05, "NA"_XS8, "NA"_XS8, 0x7c002 }, // need rBR RPlt
  //MacPro5,1 / Mac Pro (Mid 2012)
  { "MacPro5,1"_XS8, "MP51.88Z.F000.B00.1904121248"_XS8, "144.0.0.0.0"_XS8, "Mac-F221BEC8"_XS8, // Intel Xeon X5675 @ 3.06 GHz x2
    "MacPro"_XS8, "1.2"_XS8, "C07J77F7F4MC"_XS8, "Pro-Enclosure"_XS8, // Note: C07J50F7F4MC CK04000AHFC CG154TB9WU3
    0x01, 0x39, 0x0f, 0, 0, 0x11, "k5"_XS8, "k5"_XS8, 0x7c002 },
  //MacPro6,1 / Mac Pro (Late 2013)
  { "MacPro6,1"_XS8, "MP61.88Z.F000.B00.2106101804"_XS8, "428.140.7.0.0"_XS8, "Mac-F60DEB81FF30ACF6"_XS8, // Intel Xeon E5-1650 v2 @ 3.50 GHz
    "MacPro"_XS8, "1.0"_XS8, "F5KLA770F9VM"_XS8, "Pro-Enclosure"_XS8,
    0x02, 0x20, 0x0f, 0, 0, 0x18, "j90"_XS8, "j90"_XS8, 0xf0f006 },
  //MacPro7,1 / Mac Pro (2019)
  { "MacPro7,1"_XS8, "MP71.88Z.F000.B00.2107050205"_XS8, "1715.0.57.0.0"_XS8, "Mac-27AD2F918AE68F61"_XS8, // Intel Xeon W-3245M CPU @ 3.20 GHz
    "MacPro"_XS8, "1.0"_XS8, "F5KZNHACP7QM"_XS8, "Pro-Enclosure"_XS8,
    0, 0, 0, 0, 0, 0, ""_XS8, "j16O"_XS8, 0 },
  //Xserve1,1 / Xserve (Late 2006)
  { "Xserve1,1"_XS8, "XS11.88Z.0080.B01.0706271533"_XS8, ""_XS8, "Mac-F4208AC8"_XS8, // Intel Xeon E5345 @ 2.33 GHz x2
    "Xserve"_XS8, "1.0"_XS8, "CK703E1EV2Q"_XS8, "Xserve"_XS8,
    0x01, 0x11, 0x0f, 0, 0, 0x05, "NA"_XS8, "NA"_XS8, 0x79001 }, // need rBR RPlt EPCI
  //Xserve2,1 / Xserve (Early 2008)
  { "Xserve2,1"_XS8, "XS21.88Z.006C.B06.0804011317"_XS8, ""_XS8, "Mac-F42289C8"_XS8, // Intel Xeon E5472 @ 3.00 GHz x2
    "Xserve"_XS8, "1.0"_XS8, "CK830DLQX8S"_XS8, "Xserve"_XS8,
    0x01, 0x26, 0x0f, 0, 0, 0x03, "NA"_XS8, "NA"_XS8, 0x79001 }, // need rBR RPlt EPCI
  //Xserve3,1 / Xserve (Early 2009)
  { "Xserve3,1"_XS8, "XS31.88Z.0081.B06.0908061300"_XS8, ""_XS8, "Mac-F223BEC8"_XS8, // Intel Xeon E5520 @ 2.26 GHz
    "Xserve"_XS8, "1.0"_XS8, "CK933YJ16HS"_XS8, "Xserve"_XS8,
    0x01, 0x43, 0x0f, 0, 0, 0x04, "NA"_XS8, "NA"_XS8, 0x79001 }, // need rBR RPlt EPCI
  //MaxMachineType : default to iMac132
  { "iMac13,2"_XS8, "IM131.88Z.F000.B00.2004121616"_XS8, "291.0.0.0.0"_XS8, "Mac-FC02E91DDD3FA6A4"_XS8, // Intel Core i5-3470 @ 3.20 GHz
    "iMac"_XS8, "1.0"_XS8, "C02JB041DNCW"_XS8, "iMac-Aluminum"_XS8,
    0x02, 0x11, 0x0f, 0, 0, 0x16, "d8"_XS8, "d8"_XS8, 0x79006 },
};

// Firmware info for 10.13+
// by Sherlocks
uint32_t GetFwFeatures(MACHINE_TYPES Model)
{
  // Firmware info for 10.13+
  // by Sherlocks
  // FirmwareFeatures
  switch ( Model )
    {
    // Verified list from Firmware
    case MacBookPro91:
    case MacBookPro92:
      return 0xC00DE137;
      break;
    case MacBookAir41:
    case MacBookAir42:
    case MacMini51:
    case MacMini52:
    case MacMini53:
      return 0xD00DE137;
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
      return 0xE00DE137;
      break;
    case MacMini81:
      return 0xFD8FF467;
      break;
    case MacBookAir61:
    case MacBookAir62:
    case iMac141:
    case iMac142:
    case iMac143:
      return 0xE00FE137;
      break;
    case MacBookPro111:
    case MacBookPro112:
    case MacBookPro113:
    case MacBookPro114:
    case MacBookPro115:
      return 0xE80FE137;
      break;
    case iMac144:
      return 0xF00FE177;
      break;
    case iMac151:
      return 0xF80FE177;
      break;
    case MacBookPro131:
    case MacBookPro132:
    case MacBookPro141:
    case MacBookPro142:
    case iMac171:
    case iMac181:
    case iMac182:
    case iMac183:
      return 0xFC0FE177;
      break;
    case MacBook91:
    case MacBook101:
    case MacBookPro133:
    case MacBookPro143:
      return 0xFC0FE17F;
      break;
    case iMacPro11:
      return 0xFD8FF53F;
      break;
    case MacBookAir91:
      return 0xFD8FF42F;
      break;
    case iMac191:
    case iMac192:
    case iMac201:
    case iMac202:
      return 0xFD8FF577;
      break;
    case MacBookPro162:
    case MacBookPro163:
    case MacBookPro164:
      return 0xFDAFF067;
      break;
      // Verified list from Users
    case MacBookAir31:
    case MacBookAir32:
    case MacMini41:
      return 0xD00DE137;
      break;
    case MacBookAir71:
    case MacBookAir72:
      return 0xE00FE137;
      break;
    case iMac101:
    case iMac111:
    case iMac112:
    case iMac113:
    case iMac121:
    case iMac122:
    case MacMini71:
      return 0xE00DE137;
      break;
    case MacPro51:
      return 0xE80FE137;
      break;
    case MacPro61:
      return 0xE80FE177;
      break;
    case MacPro71:
      return 0xFD8FF53F;
      break;
    case MacBookPro61:
    case MacBookPro62:
    case MacBookPro71:
    case MacBookPro81:
    case MacBookPro82:
    case MacBookPro83:
      return 0xC00DE137;
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
      return 0xFC0FE137;
      break;
    case MacBook61:
    case MacBook71:
    case MacBook81:
      return 0xFC0FE13F;
      break;
    default:
      return 0xE907F537; //unknown - use oem SMBIOS value to be default
      break;
    }
}


uint64_t GetExtFwFeatures(MACHINE_TYPES Model)
{
  // FirmwareFeatures for 12+
  switch ( Model )
  {
    case MacBookPro131:
    case MacBookPro132:
    case MacBookPro141:
    case MacBookPro142:
    case iMac171:
    case iMac181:
    case iMac182:
    case iMac183:
      return 0x8FC0FE177ull;
      break;
    case MacBook91:
    case MacBook101:
    case MacBookPro133:
    case MacBookPro143:
      return 0x8FC0FE17Eull;
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
      return 0x8FC0FE137ull;
      break;
    case MacBook61:
    case MacBook71:
    case MacBook81:
      return 0x8FC0FE13Full;
      break;
    default:
      return (uint64_t)GetFwFeatures(Model); //unknown - use oem SMBIOS value to be default
      break;
  }
}

uint64_t GetExtFwFeaturesMask(MACHINE_TYPES Model)
{
  return (uint64_t)GetFwFeaturesMaskFromModel(Model) + 0xFF00000000ull;
}

bool GetMobile(MACHINE_TYPES Model)
{
  // Mobile: the battery tab in Energy Saver
  switch ( Model )
    {
    case MacBook11:
    case MacBook21:
    case MacBook31:
    case MacBook41:
    case MacBook51:
    case MacBook52:
    case MacBook61:
    case MacBook71:
    case MacBook81:
    case MacBook91:
    case MacBook101:
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
    case MacBookPro91:
    case MacBookPro92:
    case MacBookPro101:
    case MacBookPro102:
    case MacBookPro111:
    case MacBookPro112:
    case MacBookPro113:
    case MacBookPro114:
    case MacBookPro115:
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
    case MacBookPro162:
    case MacBookPro163:
    case MacBookPro164:
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
    case MacBookAir71:
    case MacBookAir72:
    case MacBookAir81:
    case MacBookAir82:
    case MacBookAir91:
      return TRUE;
    case MacMini11:
    case MacMini21:
    case MacMini31:
    case MacMini41:
    case MacMini51:
    case MacMini52:
    case MacMini53:
    case MacMini61:
    case MacMini62:
    case MacMini71:
    case MacMini81:
      return FALSE;
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
    case iMac161:
    case iMac162:
    case iMac171:
    case iMac181:
    case iMac182:
    case iMac183:
    case iMac191:
    case iMac192:
    case iMac201:
    case iMac202:
    case iMacPro11:
      return FALSE;
    case MacPro11:
    case MacPro21:
    case MacPro31:
    case MacPro41:
    case MacPro51:
    case MacPro61:
    case MacPro71:
      return FALSE;
    case Xserve11:
    case Xserve21:
    case Xserve31:
      return FALSE;
    case MaxMachineType: // currently a copy of iMac132
      return FALSE;
    default: // bug, unknown Apple model
      log_technical_bug("%s : cannot find model %d\n", __PRETTY_FUNCTION__, Model);
      return false;
    }
}

  // PlatformFeature
  // the memory tab in About This Mac
  // by TheRacerMaster
UINT64 GetPlatformFeature(MACHINE_TYPES Model)
{
  switch ( Model )
    {
    // Verified list from ioreg
    case iMac171:
    case iMac181:
    case iMac182:
    case iMac183:
    case MacPro71:
      return 0x00;
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
      return 0x01;
      break;
    case MacBookPro111:
    case MacBookPro112:
    case MacBookPro113:
    case MacBookPro114:
    case MacBookPro115:
    case MacBookPro121:
    case MacBookAir71:
    case MacBookAir72:
      return 0x02;
      break;
    case MacMini71:
    case iMac161:
    case iMac162:
      return 0x03;
      break;
    case MacPro61:
      return 0x04;
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
      return 0x1A;
      break;
    case iMacPro11:
    case MacMini81:
    case iMac191:
    case iMac192:
    case iMac201:
    case iMac202:
      return 0x20;
      break;
    case MacBookPro151:
    case MacBookPro152:
    case MacBookPro153:
    case MacBookPro154:
    case MacBookPro161:
    case MacBookPro162:
    case MacBookPro163:
    case MacBookPro164:
      return 0x32;
      break;
    case MacBookAir81:
    case MacBookAir82:
    case MacBookAir91:
      return 0x3A;
      break;
      // It is nonsense, ASCII code сharacter "2" = 0x32 != 0x02. Don't use ioreg, so that not to be confused. Use dmidecode dump.
      // Verified list from Users
      // case MacBookPro153:
      // case MacBookPro154:
      // case MacBookPro161:
      //   gSettings.Smbios.gPlatformFeature        = 0x02;
      //   break;
    default:
      return 0xFFFF; // disabled
      break;
    }
}

void getRBr(MACHINE_TYPES Model, UINT32 CPUModel, bool isMobile, char RBr[8])
{
  memset(RBr, 0, 8);
  if (ApplePlatformData[Model].smcBranch[0] != 'N') {
//    snprintf(RBr, 8, "%s", ApplePlatformData[Model].smcBranch.c_str());
    memcpy(RBr, ApplePlatformData[Model].smcBranch.c_str(), MIN(8, ApplePlatformData[Model].smcBranch.sizeInBytesIncludingTerminator()));
  } else {
    switch (CPUModel) {
    case CPU_MODEL_PENTIUM_M:
    case CPU_MODEL_CELERON:
      snprintf(RBr, 8, "%s", "m70");
      break;
      
    case CPU_MODEL_YONAH:
      snprintf(RBr, 8, "%s", "k22");
      break;
      
    case CPU_MODEL_MEROM: //TODO check for mobile
      snprintf(RBr, 8, "%s", "m75");
      break;
      
    case CPU_MODEL_PENRYN:
      if (isMobile) {
        snprintf(RBr, 8, "%s", "m82");
      } else {
        snprintf(RBr, 8, "%s", "k36");
      }
      break;
      
    case CPU_MODEL_SANDY_BRIDGE:
      if (isMobile) {
        snprintf(RBr, 8, "%s", "k90i");
      } else {
        snprintf(RBr, 8, "%s", "k60");
      }
      break;
      
    case CPU_MODEL_IVY_BRIDGE:
      snprintf(RBr, 8, "%s", "j30");
      break;
      
    case CPU_MODEL_IVY_BRIDGE_E5:
      snprintf(RBr, 8, "%s", "j90");
      break;
      
    case CPU_MODEL_HASWELL_ULT:
      snprintf(RBr, 8, "%s", "j44");
      break;
      
    case CPU_MODEL_HASWELL_U5: //Mobile - Broadwell
      snprintf(RBr, 8, "%s", "j52");
      break;
      
    case CPU_MODEL_SKYLAKE_D:
//      snprintf(RBr, 8, "%s", "j95j95am");
      memcpy(RBr, "j95j95am", 8);
      break;
      
    case CPU_MODEL_SKYLAKE_U:
      snprintf(RBr, 8, "%s", "2016mb");
      break;
      
    case CPU_MODEL_KABYLAKE1: //Mobile
      snprintf(RBr, 8, "%s", "2017mbp");
      break;
      
    case CPU_MODEL_KABYLAKE2: //Desktop
//      snprintf(RBr, 8, "%s", "j133_4_5");
      memcpy(RBr, "j133_4_5", 8);
      break;
      
    default:
      snprintf(RBr, 8, "%s", "t9");
      break;
    }
  }
}

void getRPlt(MACHINE_TYPES Model, UINT32 CPUModel, bool isMobile, char RPlt[8])
{
  memset(RPlt, 0, 8);
  if (ApplePlatformData[Model].smcPlatform[0] != 'N') {
    snprintf(RPlt, 8, "%s", ApplePlatformData[Model].smcPlatform.c_str());
//    memcpy(RPlt, ApplePlatformData[Model].smcPlatform.c_str(), 8);
  } else {
    switch (CPUModel) {
    case CPU_MODEL_PENTIUM_M:
    case CPU_MODEL_CELERON:
      snprintf(RPlt, 8, "m70");
      break;
      
    case CPU_MODEL_YONAH:
      snprintf(RPlt, 8, "k22");
      break;
      
    case CPU_MODEL_MEROM: //TODO check for mobile
      snprintf(RPlt, 8, "m75");
      break;
      
    case CPU_MODEL_PENRYN:
      if (isMobile) {
        snprintf(RPlt, 8, "m82");
      } else {
        snprintf(RPlt, 8, "k36");
      }
      break;
      
    case CPU_MODEL_SANDY_BRIDGE:
      if (isMobile) {
        snprintf(RPlt, 8, "k90i");
      } else {
        snprintf(RPlt, 8, "k60");
      }
      break;
      
    case CPU_MODEL_IVY_BRIDGE:
      snprintf(RPlt, 8, "j30");
      break;
      
    case CPU_MODEL_IVY_BRIDGE_E5:
      snprintf(RPlt, 8, "j90");
      break;
      
    case CPU_MODEL_HASWELL_ULT:
      snprintf(RPlt, 8, "j44");
      break;
      
    case CPU_MODEL_HASWELL_U5: //Mobile - Broadwell
      snprintf(RPlt, 8, "j52");
      break;
      
    case CPU_MODEL_SKYLAKE_D:
      snprintf(RPlt, 8, "j95");
      break;
      
    case CPU_MODEL_SKYLAKE_U:
      snprintf(RPlt, 8, "j79");
      break;
      
    case CPU_MODEL_KABYLAKE1: //Mobile
      snprintf(RPlt, 8, "j130a");
      break;
      
    case CPU_MODEL_KABYLAKE2: //Desktop
      snprintf(RPlt, 8, "j135");
      break;
      
    default:
      snprintf(RPlt, 8, "t9");
      break;
    }
  }
}

bool isReleaseDateWithYear20(MACHINE_TYPES Model)
{
  switch ( Model )
  {
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
    case Xserve31: {
      return false;
    }
    default: {
      return true;
    }
  }
}

// AppleReleaseDate
XString8 GetReleaseDate(MACHINE_TYPES Model)
{
  XString8 returnValue;

  const char* i = ApplePlatformData[Model].firmwareVersion.c_str();
  i += strlen(i);
  while ( *i != '.' ) i--;
  if ( isReleaseDateWithYear20(Model) ) {
    returnValue.S8Printf("%c%c/%c%c/20%c%c", i[3], i[4], i[5], i[6], i[1], i[2]);
  }else{
    returnValue.S8Printf("%c%c/%c%c/%c%c", i[3], i[4], i[5], i[6], i[1], i[2]);
  }
  return returnValue;
}

void SetDMISettingsForModel(MACHINE_TYPES Model, SETTINGS_DATA* settingsData, REFIT_CONFIG* liveConfig)
{
  liveConfig->BiosVersionUsed = ApplePlatformData[Model].firmwareVersion;
  liveConfig->ReleaseDateUsed = GetReleaseDate(Model); // AppleReleaseDate
  liveConfig->EfiVersionUsed.takeValueFrom(ApplePlatformData[Model].efiversion);

  settingsData->Smbios.BiosVendor = AppleBiosVendor;
  settingsData->Smbios.ManufactureName = settingsData->Smbios.BiosVendor;
  settingsData->Smbios.ProductName = ApplePlatformData[Model].productName;
  settingsData->Smbios.SystemVersion = ApplePlatformData[Model].systemVersion;
  settingsData->Smbios.SerialNr = ApplePlatformData[Model].serialNumber;
  settingsData->Smbios.FamilyName = ApplePlatformData[Model].productFamily;
  settingsData->Smbios.BoardManufactureName = settingsData->Smbios.BiosVendor;
  settingsData->Smbios.BoardSerialNumber = AppleBoardSN;
  settingsData->Smbios.BoardNumber = ApplePlatformData[Model].boardID;
  settingsData->Smbios.BoardVersion = ApplePlatformData[Model].productName;
  settingsData->Smbios.LocationInChassis = AppleBoardLocation;
  settingsData->Smbios.ChassisManufacturer = settingsData->Smbios.BiosVendor;
  settingsData->Smbios.ChassisAssetTag = ApplePlatformData[Model].chassisAsset;
  settingsData->Smbios.FirmwareFeatures = GetFwFeatures(Model);
  settingsData->Smbios.FirmwareFeaturesMask = GetFwFeaturesMaskFromModel(Model);
  settingsData->Smbios.ExtendedFirmwareFeatures = GetExtFwFeatures(Model);
  settingsData->Smbios.ExtendedFirmwareFeaturesMask = GetExtFwFeaturesMask(Model);
  settingsData->Smbios.gPlatformFeature = GetPlatformFeature(Model);
  if ((Model > MacPro31) && (Model < MacPro71)) {
    settingsData->Smbios.BoardType = BaseBoardTypeProcessorMemoryModule; //0xB;
  } else {
    settingsData->Smbios.BoardType = BaseBoardTypeMotherBoard; //0xA;
  }
  settingsData->Smbios.ChassisType = GetChassisTypeFromModel(Model);
  settingsData->Smbios.Mobile = GetMobile(Model); // Mobile: the battery tab in Energy Saver
}

MACHINE_TYPES GetModelFromString(const XString8& ProductName)
{
  MACHINE_TYPES i;

  for (i = (MACHINE_TYPES)(0); i < MaxMachineType; i = (MACHINE_TYPES)(i + 1)) {
    if ( ProductName == ApplePlatformData[i].productName ) {
      return i;
    }
  }
  // return ending enum as "not found"
  return MaxMachineType;
}

uint8_t GetChassisTypeFromModel(MACHINE_TYPES Model)
{

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
      return MiscChassisTypeNotebook; //0x0A;

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
    case MacBookPro162:
    case MacBookPro163:
    case MacBookPro164:
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
    case iMac201:
    case iMac202:
    case iMacPro11:
      return MiscChassisTypeLapTop; //0x09;

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
      return MiscChassisTypePortable; //0x08;

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
      return MiscChassisTypeAllInOne; //0x0D;

    case MacMini11:
    case MacMini21:
      return MiscChassisTypeLowProfileDesktop; //0x04;

    case MacMini31:
    case MacMini41:
    case MacMini51:
    case MacMini52:
    case MacMini53:
    case MacMini61:
    case MacMini62:
      return MiscChassisTypeLunchBox; //0x10;
      break;

    case MacPro41:
    case MacPro51:
    case MacPro71:
      return MiscChassisTypeTower; //0x07;

    case MacPro11:
    case MacPro21:
    case MacPro31:
    case MacPro61:
      return MiscChassisTypeUnknown; //0x02; this is a joke but think different!
          
    case Xserve11:
    case Xserve21:
    case Xserve31:
      return MiscChassisTypeRackMountChassis; //0x17;

    default: //unknown - use oem SMBIOS value to be default
      /*if (gMobile) {
        return 10; //notebook
      } else {
        return MiscChassisTypeDeskTop; //0x03;
      }*/
      return 0;
      break;
  }
}




//gFwFeaturesMask


uint32_t GetFwFeaturesMaskFromModel(MACHINE_TYPES Model)
{

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
      return 0xFF1FFF3F;
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
      return 0xFF1FFF7F;
      break;
    case iMacPro11:
    case MacBookAir91:
      return 0xFF9FFF3F;
      break;
    case iMac191:
    case iMac192:
    case iMac201:
    case iMac202:
    case MacMini81:
      return 0xFFDFFF7F;
      break;
    case MacBookPro162:
    case MacBookPro163:
    case MacBookPro164:
      return 0xFFFFFF7F;
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
      return 0xFF1FFF3F;
      break;

    case MacPro71:
      return 0xFF9FFF3F;
      break;

    default:
      return 0xFFFFFFFF; //unknown - use oem SMBIOS value to be default
      break;
  }
}

/*
 * parameters MUST contains at least a dot, followed by at lest 6 chars
 */
int compareBiosVersion(const XString8& version1, const XString8& version2)
{
  const CHAR8* v1p = version1.c_str();
  const CHAR8* v2p = version2.c_str();

  v1p += strlen(v1p);
  while (*v1p != '.') {
    v1p--;
  }

  v2p += strlen(v2p);
  while (*v2p != '.') {
    v2p--;
  }
  if ( strlen(v1p) < 7 ) {
    log_technical_bug("strlen(v1p) < 7");
    return false;
  }
  if ( strlen(v2p) < 7 ) {
    log_technical_bug("strlen(v2p) < 7");
    return false;
  }

  if (((v1p[1] > '0') && (v2p[1] == '0')) || ((v1p[1] >= v2p[1]) && (v1p[2] > v2p[2]))) {
    return 1;
  } else if ((v1p[1] == v2p[1]) && (v1p[2] == v2p[2])) {
    if (((v1p[3] > '0') && (v2p[3] == '0')) || ((v1p[3] >= v2p[3]) && (v1p[4] > v2p[4]))) {
      return 1;
    } else if ((v1p[3] == v2p[3]) && (v1p[4] == v2p[4])) {
      if (((v1p[5] > '0') && (v2p[5] == '0')) || ((v1p[5] > '1') && (v2p[5] == '1')) ||
          ((v1p[5] > '2') && (v2p[5] == '2')) || ((v1p[5] >= v2p[5]) && (v1p[6] > v2p[6]))) {
        return 1;
      } else if ((v1p[5] == v2p[5]) && (v1p[6] == v2p[6])) {
        // equal
        return 0;
      } else {
        return -1;
      }
    } else {
      return -1;
    }
  } else {
    return -1;
  }
}

bool is2ndBiosVersionGreaterThan1st(const XString8& version1, const XString8& version2)
{
  return compareBiosVersion(version1, version2) <= 0;
}
bool isBiosVersionEquel(const XString8& version1, const XString8& version2)
{
  return compareBiosVersion(version1, version2) == 0;
}


int compareReleaseDate(const XString8& date1, const XString8& date2)
{
  const CHAR8* i = date1.c_str();
  const CHAR8* j = date2.c_str();

  if ( (strlen(i) == 8) && (strlen(j) == 8) )
  {
    if ( ((i[6] > '0') && (j[6] == '0')) || ((i[6] >= j[6]) && (i[7] > j[7])) )
    {
      return 1;
    } else if ( (i[6] == j[6]) && (i[7] == j[7]) )
    {
      if ( ((i[0] > '0') && (j[0] == '0')) || ((i[0] >= j[0]) && (i[1] > j[1])) )
      {
        return 1;
      } else if ( (i[0] == j[0]) && (i[1] == j[1]) )
      {
        if ( ((i[3] > '0') && (j[3] == '0')) || ((i[3] > '1') && (j[3] == '1')) || ((i[3] > '2') && (j[3] == '2')) || ((i[3] >= j[3]) && (i[4] > j[4])) )
        {
          return 1;
        } else if ( (i[3] == j[3]) && (i[4] == j[4]) )
        {
          return 0;
        } else
        {
          return -1;
        }
      } else
      {
        return -1;
      }
    } else
    {
      return -1;
    }
  } else if ( (strlen(i) == 8) && (strlen(j) == 10) )
  {
    if ( ((i[6] > '0') && (j[8] == '0')) || ((i[6] >= j[8]) && (i[7] > j[9])) )
    {
      return 1;
    } else if ( (i[6] == j[8]) && (i[7] == j[9]) )
    {
      if ( ((i[0] > '0') && (j[0] == '0')) || ((i[0] >= j[0]) && (i[1] > j[1])) )
      {
        return 1;
      } else if ( (i[0] == j[0]) && (i[1] == j[1]) )
      {
        if ( ((i[3] > '0') && (j[3] == '0')) || ((i[3] > '1') && (j[3] == '1')) || ((i[3] > '2') && (j[3] == '2')) || ((i[3] >= j[3]) && (i[4] > j[4])) )
        {
          return 1;
        } else if ( (i[3] == j[3]) && (i[4] == j[4]) )
        {
          return 0;
        } else
        {
          return -1;
        }
      } else
      {
        return -1;
      }
    } else
    {
      return -1;
    }
  } else if ( (strlen(i) == 10) && (strlen(j) == 10) )
  {
    if ( ((i[8] > '0') && (j[8] == '0')) || ((i[8] >= j[8]) && (i[9] > j[9])) )
    {
      return 1;
    } else if ( (i[8] == j[8]) && (i[9] == j[9]) )
    {
      if ( ((i[0] > '0') && (j[0] == '0')) || ((i[0] >= j[0]) && (i[1] > j[1])) )
      {
        return 1;
      } else if ( (i[0] == j[0]) && (i[1] == j[1]) )
      {
        if ( ((i[3] > '0') && (j[3] == '0')) || ((i[3] > '1') && (j[3] == '1')) || ((i[3] > '2') && (j[3] == '2')) || ((i[3] >= j[3]) && (i[4] > j[4])) )
        {
          return 1;
        } else if ( (i[3] == j[3]) && (i[4] == j[4]) )
        {
          return 0;
        } else
        {
          return -1;
        }
      } else
      {
        return -1;
      }
    } else
    {
      return -1;
    }
  } else if ( (strlen(i) == 10) && (strlen(j) == 8) )
  {
    if ( ((i[8] > '0') && (j[6] == '0')) || ((i[8] >= j[6]) && (i[9] > j[7])) )
    {
      return 1;
    } else if ( (i[8] == j[6]) && (i[9] == j[7]) )
    {
      if ( ((i[0] > '0') && (j[0] == '0')) || ((i[0] >= j[0]) && (i[1] > j[1])) )
      {
        return 1;
      } else if ( (i[0] == j[0]) && (i[1] == j[1]) )
      {
        if ( ((i[3] > '0') && (j[3] == '0')) || ((i[3] > '1') && (j[3] == '1')) || ((i[3] > '2') && (j[3] == '2')) || ((i[3] >= j[3]) && (i[4] > j[4])) )
        {
          return 1;
        } else if ( (i[3] == j[3]) && (i[4] == j[4]) )
        {
          return 0;
        } else
        {
          return -1;
        }
      } else
      {
        return -1;
      }
    } else
    {
      return -1;
    }
  } else
  {
    return -2;
  }
}


