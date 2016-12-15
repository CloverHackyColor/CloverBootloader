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

UINT32  gFwFeatures             = 0xE001f537;             //default values for iMac13,1

// All SMBIOS data were updated by Sherlocks.
// FredWst supported SmcExtract.

CHAR8   *AppleFirmwareVersion[] =
{
  "MB11.88Z.0061.B03.0610121324",   // MB11		// 1.83 GHz Core Duo (T2400)
  "MB21.88Z.00A5.B07.0706270922",   // MB21		// 1.83 GHz Core 2 Duo (T5600)
  "MB31.88Z.008E.B02.0803051832",   // MB31		// 2.2 GHz Core 2 Duo (T7500)
  "MB41.88Z.00C1.B00.0802091535",   // MB41		// 2.1 GHz Core 2 Duo (T8100)
  "MB51.88Z.007D.B03.0904271443",   // MB51		// 2.0 GHz Core 2 Duo (P7350)
  "MB52.88Z.0088.B05.0904162222",   // MB52		// 2.13 GHz Core 2 Duo (P7450)
  "MB61.88Z.00C8.B02.1003151501",   // MB61		// 2.26 GHz Core 2 Duo (P7550)
  "MB71.88Z.0039.B0E.1111071359",   // MB71		// 2.4 GHz Core 2 Duo (P8600)
  "MB81.88Z.0164.B19.1610201519",   // MB81		// 1.2 GHz Core M (M-5Y51)
  "MB91.88Z.0154.B09.1611151456",   // MB91		// 1.2 GHz Core m5 (M5-6Y54)
  "MBP11.88Z.0055.B08.0610121325",  // MBP11	// 1.67 GHz Core Duo (L2400)
  "MBP21.88Z.00A5.B08.0708131242",  // MBP21	// 2.33 GHz Core 2 Duo (T7600)
  "MBP22.88Z.00A5.B07.0708131242",  // MBP22	// 2.16 GHz Core 2 Duo (T7400)
  "MBP31.88Z.0070.B07.0803051658",  // MBP31	// 2.2 GHz Core 2 Duo (T7500)
  "MBP41.88Z.00C1.B03.0802271651",  // MBP41	// 2.4 GHz Core 2 Duo (T8300)
  "MBP51.88Z.007E.B06.1202061253",  // MBP51	// 2.4 GHz Core 2 Duo (P8600)
  "MBP53.88Z.00AC.B03.0906151647",  // MBP53	// 2.66 GHz Core 2 Duo (P8800)
  "MBP55.88Z.00AC.B03.0906151708",  // MBP55	// 2.53 GHz Core 2 Duo (P8700)
  "MBP61.88Z.0057.B11.1509232013",  // MBP61	// 2.53 GHz Core i5 (I5-540M)
  "MBP61.88Z.0057.B11.1509232013",  // MBP62	// 2.4 GHz Core i5 (I5-520M)
  "MBP71.88Z.0039.B0E.1111071400",  // MBP71	// 2.4 GHz Core 2 Duo (P8600)
  "MBP81.88Z.0047.B2D.1610201938",  // MBP81	// 2.3 GHz Core i5 (I5-2415M)
  "MBP81.88Z.0047.B2D.1610201938",  // MBP82	// 2.3 GHz Core i7 (I7-2820QM)
  "MBP81.88Z.0047.B2D.1610201938",  // MBP83	// 2.2 GHz Core i7 (I7-2720QM)
  "MBP91.88Z.00D3.B0E.1610201614",  // MBP91	// 2.3 GHz Core i7 (I7-3615QM)
  "MBP91.88Z.00D3.B0E.1610201614",  // MBP92	// 2.5 GHz Core i5 (I5-3210M)
  "MBP101.88Z.00EE.B0B.1610201442", // MBP101	// 2.3 GHz Core i7 (I7-3615QM)
  "MBP102.88Z.0106.B0B.1610201916", // MBP102	// 2.5 GHz Core i5 (I5-3210M)
  "MBP111.88Z.0138.B18.1610201920", // MBP111	// 2.4 GHz Core i5 (I5-4258U)
  "MBP112.88Z.0138.B18.1610201654", // MBP112	// 2.0 GHz Core i7 (I7-4750HQ)
  "MBP112.88Z.0138.B18.1610201654", // MBP113	// 2.6 GHz Core i7 (I7-4980HQ)
  "MBP114.88Z.0172.B10.1610201519", // MBP115	// 2.3 GHz Core i7 (I7-4980HQ)
  "MBP121.88Z.0167.B18.1610241407", // MBP121	// 2.7 GHz Core i5 (I5-5257U)
  "MBP131.88Z.0205.B07.1611151504", // MBP131	// 2.0 GHz Core i5 (I5-6360U)
  "MBP132.88Z.0226.B08.1611301700", // MBP132	// 2.9 GHz Core i5 (I5-6267U)
  "MBP133.88Z.0226.B08.1611301700", // MBP133	// 2.9 GHz Core i7 (I7-6920HQ)
  "MBA11.88Z.00BB.B03.0803171226",  // MBA11	// 1.6 GHz Core 2 Duo (P7500)
  "MBA21.88Z.0075.B05.1003051506",  // MBA21	// 1.6 GHz Core 2 Duo (SL9300)
  "MBA31.88Z.0061.B07.1201241641",  // MBA31	// 1.4 GHz Core 2 Duo (SU9400)
  "MBA41.88Z.0077.B15.1610201441",  // MBA41	// 1.6 GHz Core i5 (I5-2467M)
  "MBA41.88Z.0077.B15.1610201441",  // MBA42	// 1.8 GHz Core i7 (I7-2677M)
  "MBA51.88Z.00EF.B05.1610241034",  // MBA51	// 1.7 GHz Core i5 (I5-3317U)
  "MBA51.88Z.00EF.B05.1610241034",  // MBA52	// 1.8 GHz Core i5 (I5-3427U)
  "MBA61.88Z.0099.B23.1610201523",  // MBA61	// 1.3 GHz Core i5 (I5-4250U)
  "MBA61.88Z.0099.B23.1610201523",  // MBA62	// 1.7 GHz Core i7 (I7-4650U)
  "MBA71.88Z.0166.B13.1611031424",  // MBA71	// 1.6 GHz Core i5 (I5-5250U)
  "MBA71.88Z.0166.B13.1611031424",  // MBA72	// 2.2 GHz Core i7 (I7-5650U)
  "MM11.88Z.0055.B08.0610121326",   // MM11		// 1.66 GHz Core Duo (T2300)
  "MM21.88Z.009A.B00.0706281359",   // MM21		// 1.83 GHz Core 2 Duo (T5600)
  "MM31.88Z.0081.B06.0904271717",   // MM31		// 2.0 GHz Core 2 Duo (P7350)
  "MM41.88Z.0042.B03.1111072100",   // MM41		// 2.4 GHz Core 2 Duo (P8600)
  "MM51.88Z.0077.B15.1610201614",   // MM51		// 2.3 GHz Core i5 (I5-2415M)
  "MM61.88Z.0106.B0B.1610201654",   // MM61		// 2.5 GHz Core i5 (I5-3210M)
  "MM61.88Z.0106.B0B.1610201654",   // MM62		// 2.6 GHz Core i7 (I7-3720QM)
  "MM71.88Z.0220.B08.1610201519",   // MM71		// 1.4 GHz Core i5 (I5-4260U)
  "IM41.88Z.0055.B08.0609061538",   // IM41		// 1.83 GHz Core Duo (T2400)
  "IM51.88Z.0090.B09.0706270921",   // IM51		// 2.0 GHz Core 2 Duo (T7200)
  "IM61.88Z.0093.B07.0804281538",   // IM61		// 2.16 GHz Core 2 Duo (T7400)
  "IM71.88Z.007A.B03.0803051705",   // IM71		// 2.0 GHz Core 2 Duo (T7300)
  "IM81.88Z.00C1.B00.0802091538",   // IM81		// 2.4 GHz Core 2 Duo (E8135)
  "IM91.88Z.008D.B08.0904271717",   // IM91		// 3.06 GHz Core 2 Duo (E8435)
  "IM101.88Z.00CC.B00.0909031926",  // IM101	// 3.06 GHz Core 2 Duo (E7600)
  "IM111.88Z.0034.B04.1509231906",  // IM111	// 2.66 GHz Core i5 (I5-750)
  "IM112.88Z.0057.B03.1509231647",  // IM112	// 3.06 GHz Core i3 (I3-540)
  "IM112.88Z.0057.B03.1509231647",  // IM113	// 3.2 GHz Core i3 (I3-550)
  "IM121.88Z.0047.B25.1611011518",  // IM121	// 2.5 GHz Core i5 (I5-2400S)
  "IM121.88Z.0047.B25.1611011518",  // IM122	// 3.4 GHz Core i7 (I7-2600)
  "IM131.88Z.010A.B0A.1610201916",  // IM131	// 2.9 GHz Core i5 (I5-3470S)
  "IM131.88Z.010A.B0A.1610201916",  // IM132	// 3.4 GHz Core i7 (I7-3770)
  "IM141.88Z.0118.B14.1610201916",  // IM141	// 2.7 GHz Core i5 (I5-4570R)
  "IM142.88Z.0118.B14.1610201615",  // IM142	// 3.5 GHz Core i7 (I7-4771)
  "IM143.88Z.0118.B14.1610201916",  // IM143	// 2.9 GHz Core i5 (I5-4570S)
  "IM144.88Z.0179.B13.1602221719",  // IM144	// 1.4 GHz Core i5 (I5-4260U)
  "IM151.88Z.0207.B08.1610201922",  // IM151	// 3.5 GHz Core i5 (I5-4690)
  "IM161.88Z.0207.B04.1610201519",  // IM161	// 1.6 GHz Core i5 (I5-5250U)
  "IM162.88Z.0207.B04.1610201519",  // IM162	// 3.3 GHz Core i5 (i7-5775R)
  "IM171.88Z.0105.B11.1610251139",  // IM171	// 3.3 GHz Core i5 (I5-6600)
  "MP11.88Z.005C.B08.0707021221",   // MP11		// 2.0 GHz D. Core Xeon 5130 x2
  "MP21.88Z.007F.B06.0707021348",   // MP21		// 3.0 GHz Q. Core Xeon X5365 x2
  "MP31.88Z.006C.B05.0802291410",   // MP31		// 2.8 GHz Q. Core Xeon E5462 x2
  "MP41.88Z.0081.B07.0910130729",   // MP41		// 2.26 GHz Q. Core Xeon E5520 x2
  "MP51.88Z.007F.B03.1010071432",   // MP51		// 2.93 GHz 6 Core Xeon X5670 x2
  "MP61.88Z.0116.B21.1610201524",   // MP61		// 2.7 GHz 12 Core Xeon E5-2697v2
};

CHAR8* AppleBoardID[] =    //Lion DR1 compatible
{
  "Mac-F4208CC8",          // MB11
  "Mac-F4208CA9",          // MB21
  "Mac-F22788C8",          // MB31
  "Mac-F22788A9",          // MB41
  "Mac-F42D89C8",          // MB51
  "Mac-F22788AA",          // MB52
  "Mac-F22C8AC8",          // MB61
  "Mac-F22C89C8",          // MB71
  "Mac-BE0E8AC46FE800CC",  // MB81
  "Mac-9AE82516C7C6B903",  // MB91
  "Mac-F425BEC8",          // MBP11
  "Mac-F42189C8",          // MBP21
  "Mac-F42187C8",          // MBP22
  "Mac-F4238BC8",          // MBP31
  "Mac-F42C89C8",          // MBP41
  "Mac-F42D86C8",          // MBP51
  "Mac-F22587C8",          // MBP53
  "Mac-F2268AC8",          // MBP55
  "Mac-F22589C8",          // MBP61
  "Mac-F22586C8",          // MBP62
  "Mac-F222BEC8",          // MBP71
  "Mac-94245B3640C91C81",  // MBP81
  "Mac-94245A3940C91C80",  // MBP82
  "Mac-942459F5819B171B",  // MBP83
  "Mac-4B7AC7E43945597E",  // MBP91
  "Mac-6F01561E16C75D06",  // MBP92
  "Mac-C3EC7CD22292981F",  // MBP101
  "Mac-AFD8A9D944EA4843",  // MBP102
  "Mac-189A3D4F975D5FFC",  // MBP111
  "Mac-3CBD00234E554E41",  // MBP112
  "Mac-2BD1B31983FE1663",  // MBP113
  "Mac-06F11F11946D27C5",  // MBP115
  "Mac-E43C1C25D4880AD6",  // MBP121
  "Mac-473D31EABEB93F9B",  // MBP131
  "Mac-66E35819EE2D0D05",  // MBP132
  "Mac-A5C67F76ED83108C",  // MBP133
  "Mac-F42C8CC8",          // MBA11
  "Mac-F42D88C8",          // MBA21
  "Mac-942452F5819B1C1B",  // MBA31
  "Mac-C08A6BB70A942AC2",  // MBA41
  "Mac-742912EFDBEE19B3",  // MBA42
  "Mac-66F35F19FE2A0D05",  // MBA51
  "Mac-2E6FAB96566FE58C",  // MBA52
  "Mac-35C1E88140C3E6CF",  // MBA61
  "Mac-7DF21CB3ED6977E5",  // MBA62
  "Mac-9F18E312C5C2BF0B",  // MBA71
  "Mac-937CB26E2E02BB01",  // MBA72
  "Mac-F4208EC8",          // MM11
  "Mac-F4208EAA",          // MM21
  "Mac-F22C86C8",          // MM31
  "Mac-F2208EC8",          // MM41
  "Mac-8ED6AF5B48C039E1",  // MM51
  "Mac-031AEE4D24BFF0B1",  // MM61
  "Mac-F65AE981FFA204ED",  // MM62
  "Mac-35C5E08120C7EEAF",  // MM71
  "Mac-F42787C8",          // IM41
  "Mac-F4228EC8",          // IM51
  "Mac-F4218FC8",          // IM61
  "Mac-F42386C8",          // IM71
  "Mac-F227BEC8",          // IM81
  "Mac-F2218FA9",          // IM91
  "Mac-F2268CC8",          // IM101
  "Mac-F2268DAE",          // IM111
  "Mac-F2238AC8",          // IM112
  "Mac-F2238BAE",          // IM113
  "Mac-942B5BF58194151B",  // IM121
  "Mac-942B59F58194171B",  // IM122
  "Mac-00BE6ED71E35EB86",  // IM131
  "Mac-FC02E91DDD3FA6A4",  // IM132
  "Mac-031B6874CF7F642A",  // IM141
  "Mac-27ADBB7B4CEE8E61",  // IM142
  "Mac-77EB7D7DAF985301",  // IM143
  "Mac-81E3E92DD6088272",  // IM144
  "Mac-42FD25EABCABB274",  // IM151
  "Mac-A369DDC4E67F1C45",  // IM161
  "Mac-FFE5EF870D7BA81A",  // IM162
  "Mac-DB15BD556843C820",  // IM171
  "Mac-F4208DC8",          // MP11
  "Mac-F4208DA9",          // MP21
  "Mac-F42C88C8",          // MP31
  "Mac-F4238CC8",          // MP41
  "Mac-F221BEC8",          // MP51
  "Mac-F60DEB81FF30ACF6",  // MP61
};

CHAR8* AppleReleaseDate[] =
{
  "10/12/2006",  // MB11
  "06/27/2007",  // MB21
  "03/05/2008",  // MB31
  "02/09/2008",  // MB41
  "04/27/2009",  // MB51
  "04/16/2009",  // MB52
  "03/15/2010",  // MB61
  "11/07/2011",  // MB71
  "10/20/2016",  // MB81
  "11/15/2016",  // MB91
  "10/12/2006",  // MBP11
  "08/13/2007",  // MBP21
  "08/13/2007",  // MBP22
  "03/05/2008",  // MBP31
  "02/27/2008",  // MBP41
  "02/06/2012",  // MBP51
  "06/15/2009",  // MBP52
  "06/15/2009",  // MBP53
  "09/23/2015",  // MBP61
  "09/23/2015",  // MBP62
  "11/07/2011",  // MBP71
  "10/20/2016",  // MBP81
  "10/20/2016",  // MBP82
  "10/20/2016",  // MBP83
  "10/20/2016",  // MBP91
  "10/20/2016",  // MBP92
  "10/20/2016",  // MBP101
  "10/20/2016",  // MBP102
  "10/20/2016",  // MBP111
  "10/20/2016",  // MBP112
  "10/20/2016",  // MBP113
  "10/20/2016",  // MBP115
  "10/24/2016",  // MBP121
  "11/15/2016",  // MBP131
  "11/30/2016",  // MBP132
  "11/30/2016",  // MBP133
  "03/17/2008",  // MBA11
  "03/05/2010",  // MBA21
  "01/24/2012",  // MBA31
  "10/20/2016",  // MBA41
  "10/20/2016",  // MBA42
  "10/24/2016",  // MBA51
  "10/24/2016",  // MBA52
  "10/20/2016",  // MBA61
  "10/20/2016",  // MBA62
  "11/03/2016",  // MBA71
  "11/03/2016",  // MBA72
  "10/12/2006",  // MM11
  "06/28/2007",  // MM21
  "04/27/2009",  // MM31
  "11/07/2011",  // MM41
  "10/20/2016",  // MM51
  "10/20/2016",  // MM61
  "10/20/2016",  // MM62
  "10/20/2016",  // MM71
  "09/06/2006",  // IM41
  "06/27/2007",  // IM51
  "04/28/2008",  // IM61
  "03/05/2008",  // IM71
  "02/09/2008",  // IM81
  "04/27/2009",  // IM91
  "09/03/2009",  // IM101
  "09/23/2015",  // IM111
  "09/23/2015",  // IM112
  "09/23/2015",  // IM113
  "11/01/2016",  // IM121
  "11/01/2016",  // IM122
  "10/20/2016",  // IM131
  "10/20/2015",  // IM132
  "10/20/2016",  // IM141
  "10/20/2016",  // IM142
  "10/20/2016",  // IM143
  "02/22/2016",  // IM144
  "10/20/2016",  // IM151
  "10/20/2015",  // IM161
  "10/20/2015",  // IM162
  "10/25/2016",  // IM171
  "07/02/2007",  // MP11
  "07/02/2007",  // MP21
  "02/29/2008",  // MP31
  "10/13/2009",  // MP41
  "10/07/2010",  // MP51
  "10/20/2016",  // MP61
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
  "MacBookPro1,1",
  "MacBookPro2,1",
  "MacBookPro2,2",
  "MacBookPro3,1",
  "MacBookPro4,1",
  "MacBookPro5,1",
  "MacBookPro5,3",
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
  "MacBookPro11,5",
  "MacBookPro12,1",
  "MacBookPro13,1",
  "MacBookPro13,2",
  "MacBookPro13,3",
  "MacBookAir1,1",
  "MacBookAir2,1",
  "MacBookAir3,1",
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
  "Macmini6,1",
  "Macmini6,2",
  "Macmini7,1",
  "iMac4,1",
  "iMac5,1",
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
  "iMac14,1",
  "iMac14,2",
  "iMac14,3",
  "iMac14,4",
  "iMac15,1",
  "iMac16,1",
  "iMac16,2",
  "iMac17,1",
  "MacPro1,1",
  "MacPro2,1",
  "MacPro3,1",
  "MacPro4,1",
  "MacPro5,1",
  "MacPro6,1",
};

CHAR8* AppleFamilies[] =
{
  "MacBook",       // MB11
  "MacBook",       // MB21
  "MacBook",       // MB31
  "MacBook",       // MB41
  "MacBook",       // MB51
  "MacBook",       // MB52
  "MacBook",       // MB61
  "MacBook",       // MB71
  "MacBook",       // MB81
  "MacBook",       // MB91
  "MacBook Pro",   // MBP11
  "MacBook Pro",   // MBP21
  "MacBook Pro",   // MBP22
  "MacBook Pro",   // MBP31
  "MacBook Pro",   // MBP41
  "MacBook Pro",   // MBP51
  "MacBook Pro",   // MBP53
  "MacBook Pro",   // MBP55
  "MacBook Pro",   // MBP61
  "MacBook Pro",   // MBP62
  "MacBook Pro",   // MBP71
  "MacBook Pro",   // MBP81
  "MacBook Pro",   // MBP82
  "MacBook Pro",   // MBP83
  "MacBook Pro",   // MBP91
  "MacBook Pro",   // MBP92
  "MacBook Pro",   // MBP101
  "MacBook Pro",   // MBP102
  "MacBook Pro",   // MBP111
  "MacBook Pro",   // MBP112
  "MacBook Pro",   // MBP113
  "MacBook Pro",   // MBP115
  "MacBook Pro",   // MBP121
  "MacBook Pro",   // MBP131
  "MacBook Pro",   // MBP132
  "MacBook Pro",   // MBP133
  "MacBook Air",   // MBA11
  "MacBook Air",   // MBA21
  "MacBook Air",   // MBA31
  "MacBook Air",   // MBA41
  "MacBook Air",   // MBA42
  "MacBook Air",   // MBA51
  "MacBook Air",   // MBA52
  "MacBook Air",   // MBA61
  "MacBook Air",   // MBA62
  "MacBook Air",   // MBA71
  "MacBook Air",   // MBA72
  "Mac mini",      // MM11
  "Mac mini",      // MM21
  "Mac mini",      // MM31
  "Mac mini",      // MM41
  "Mac mini",      // MM51
  "Mac mini",      // MM61
  "Mac mini",      // MM62
  "Mac mini",      // MM71
  "iMac",          // IM41
  "iMac",          // IM51
  "iMac",          // IM61
  "iMac",          // IM71
  "iMac",          // IM81
  "iMac",          // IM91
  "iMac",          // IM101
  "iMac",          // IM111
  "iMac",          // IM112
  "iMac",          // IM113
  "iMac",          // IM121
  "iMac",          // IM122
  "iMac",          // IM131
  "iMac",          // IM132
  "iMac",          // IM141
  "iMac",          // IM142
  "iMac",          // IM143
  "iMac",          // IM144
  "iMac",          // IM151
  "iMac",          // IM161
  "iMac",          // IM162
  "iMac",          // IM171
  "MacPro",        // MP11
  "MacPro",        // MP21
  "MacPro",        // MP31
  "MacPro",        // MP41
  "MacPro",        // MP51
  "MacPro",        // MP61
};


CHAR8* AppleSystemVersion[] =
{
  "1.1",  // MB11
  "1.2",  // MB21
  "1.3",  // MB31
  "1.3",  // MB41
  "1.3",  // MB51
  "1.3",  // MB52
  "1.0",  // MB61
  "1.0",  // MB71
  "1.0",  // MB81
  "1.0",  // MB91
  "1.0",  // MBP11
  "1.0",  // MBP21
  "1.0",  // MBP22
  "1.0",  // MBP31
  "1.0",  // MBP41
  "1.0",  // MBP51
  "1.0",  // MBP53
  "1.0",  // MBP55
  "1.0",  // MBP61
  "1.0",  // MBP62
  "1.0",  // MBP71
  "1.0",  // MBP81
  "1.0",  // MBP82
  "1.0",  // MBP83
  "1.0",  // MBP91
  "1.0",  // MBP92
  "1.0",  // MBP101
  "1.0",  // MBP102
  "1.0",  // MBP111
  "1.0",  // MBP112
  "1.0",  // MBP113
  "1.0",  // MBP115
  "1.0",  // MBP121
  "1.0",  // MBP131
  "1.0",  // MBP132
  "1.0",  // MBP133
  "1.0",  // MBA11
  "1.0",  // MBA21
  "1.0",  // MBA31
  "1.0",  // MBA41
  "1.0",  // MBA42
  "1.0",  // MBA51
  "1.0",  // MBA52 
  "1.0",  // MBA61
  "1.0",  // MBA62
  "1.0",  // MBA71
  "1.0",  // MBA72
  "1.0",  // MM11
  "1.1",  // MM21
  "1.0",  // MM31
  "1.0",  // MM41
  "1.0",  // MM51
  "1.0",  // MM61
  "1.0",  // MM62
  "1.0",  // MM71
  "1.0",  // IM41
  "1.0",  // IM51
  "1.0",  // IM61
  "1.0",  // IM71
  "1.3",  // IM81
  "1.0",  // IM91
  "1.0",  // IM101
  "1.0",  // IM111
  "1.2",  // IM112
  "1.0",  // IM113
  "1.9",  // IM121
  "1.9",  // IM122
  "1.0",  // IM131
  "1.0",  // IM132
  "1.0",  // IM141
  "1.0",  // IM142
  "1.0",  // IM143
  "1.0",  // IM144
  "1.0",  // IM151
  "1.0",  // IM161
  "1.0",  // IM162
  "1.0",  // IM171
  "1.0",  // MP11
  "1.0",  // MP21
  "1.3",  // MP31
  "1.4",  // MP41
  "1.2",  // MP51
  "1.0",  // MP61
};

CHAR8* AppleSerialNumber[] = //random generated
{
  "W80A041AU9B",  // MB11
  "W88A041AWGP",  // MB21
  "W874HACKZ62",	// MB31
  "W88A041A0P0",  // MB41
  "W8944T1S1AQ",  // MB51
  "W88AAAAA9GU",  // MB52
  "451131JCGAY",  // MB61
  "451211MEF5X",  // MB71
  "C02RCE58GCN3", // MB81
  "C02RM408HDNK", // MB91
  "W884857JVJ1",  // MBP11
  "W88130WUW0H",	// MBP21
  "W8827B4CW0L",  // MBP22
  "W8841OHZX91",  // MBP31
  "W88484F2YP4",  // MBP41
  "W88439FE1G0",  // MBP51
  "W8035TG97XK",  // MBP53
  "W8035TG966D",  // MBP55
  "C02G5834DC79", // MBP61
  "CK132A91AGW",  // MBP62
  "CK145C7NATM",  // MBP71
  "W89F9196DH2G", // MBP81
  "C02HL0FGDF8X", // MBP82
  "W88F9CDEDF93", // MBP83
  "C02LW984F1G4", // MBP91
  "C02HA041DTY3", // MBP92
  "C02K2HACDKQ1", // MBP101
  "C02K2HACG4N7", // MBP102
  "C02LSHACFH00", // MBP111
  "C02LSHACG86R", // MBP112
  "C02LSHACFR1M", // MBP113
  "C02LSHACG85Y", // MBP115
  "C02Q51OSH1DP", // MBP121
  "C02SLHACGVC1", // MBP131
  "C02SLHACGYFH", // MBP132
  "C02SLHACGTFN", // MBP133
  "W864947A18X",  // MBA11
  "W86494769A7",  // MBA21
  "C02FLHACD0QX", // MBA31
  "C02KGHACDRV9", // MBA41
  "C02GLHACDJWT", // MBA42
  "C02J6HACDRV6", // MBA51
  "C02HA041DRVC", // MBA52
  "C02KTHACF5NT", // MBA61
  "C02HACKUF5V7", // MBA62
  "C02PCLGFH569", // MBA71
  "C02Q1HACG940", // MBA72
  "W8702N1JU35",  // MM11
  "W8705W9LYL2",  // MM21
  "W8905BBE19X",  // MM31
  "C02FHBBEDD6H", // MM41
  "C07GA041DJD0", // MM51
  "C07JNHACDY3H", // MM61
  "C07JD041DWYN", // MM62
  "C02NN7NHG1J0", // MM71
  "W862HACFU2N",	// IM41
  "W871HACHVUX",	// IM51
  "W871HACKVGN",	// IM61
  "W874HACBX85",	// IM71
  "W89A00AAX88",  // IM81
  "W89A00A36MJ",  // IM91
  "W80AA98A5PE",  // IM101
  "G8942B1V5PJ",  // IM111
  "W8034342DB7",  // IM112
  "QP0312PBDNR",  // IM113
  "W80CF65ADHJF", // IM121
  "W88GG136DHJQ", // IM122
  "C02JA041DNCT", // IM131
  "C02JB041DNCW", // IM132
  "D25LHACKF8J2", // IM141
  "D25LHACKF8JC", // IM142
  "D25LHACKF8J3", // IM143
  "D25LHACKFY0T", // IM144
  "C02ND2VTGL1V", // IM151
  "C02QQHACGF1J",	// IM161
  "C02PNHACGG7G",	// IM162
  "C02QFHACGG7L", // IM171
  "W88A77AXUPZ",  // MP11
  "W8930518UPZ",  // MP21
  "W88A77AA5J4",  // MP31
  "CT93051DK9Y",  // MP41
  "C07J77F7F4MC", // MP51  - C07J50F7F4MC  CK04000AHFC  "CG154TB9WU3"
  "F5KLA770F9VM", // MP61
};

//no! ChassisVersion == BoardID
CHAR8* AppleChassisAsset[] =
{
  "MacBook-White",      // MB11
  "MacBook-White",      // MB21
  "MacBook-White",      // MB31
  "MacBook-Black",      // MB41
  "MacBook-Black",      // MB51
  "MacBook-Black",      // MB52
  "MacBook-White",      // MB61
  "MacBook-White",      // MB71
  "MacBook-Aluminum",   // MB81
  "MacBook-Aluminum",   // MB91
  "MacBook-Aluminum",   // MBP11
  "MacBook-Aluminum",   // MBP21
  "MacBook-Aluminum",   // MBP22
  "MacBook-Aluminum",   // MBP31
  "MacBook-Aluminum",   // MBP41
  "MacBook-Aluminum",   // MBP51
  "MacBook-Aluminum",   // MBP53
  "MacBook-Aluminum",   // MBP55
  "MacBook-Aluminum",   // MBP61
  "MacBook-Aluminum",   // MBP62
  "MacBook-Aluminum",   // MBP71
  "MacBook-Aluminum",   // MBP81
  "MacBook-Aluminum",   // MBP82
  "MacBook-Aluminum",   // MBP83
  "MacBook-Aluminum",   // MBP91
  "MacBook-Aluminum",   // MBP92
  "MacBook-Aluminum",   // MBP101
  "MacBook-Aluminum",   // MBP102
  "MacBook-Aluminum",   // MBP111
  "MacBook-Aluminum",   // MBP112
  "MacBook-Aluminum",   // MBP113
  "MacBook-Aluminum",   // MBP115
  "MacBook-Aluminum",   // MBP121
  "MacBook-Aluminum",   // MBP131
  "MacBook-Aluminum",   // MBP132
  "MacBook-Aluminum",   // MBP133
  "Air-Enclosure",      // MBA11
  "Air-Enclosure",      // MBA21
  "Air-Enclosure",      // MBA31
  "Air-Enclosure",      // MBA41
  "Air-Enclosure",      // MBA42
  "Air-Enclosure",      // MBA51
  "Air-Enclosure",      // MBA52 
  "Air-Enclosure",      // MBA61
  "Air-Enclosure",      // MBA62
  "Air-Enclosure",      // MBA71
  "Air-Enclosure",      // MBA72
  "Mini-Aluminum",      // MM11
  "Mini-Aluminum",      // MM21
  "Mini-Aluminum",      // MM31
  "Mini-Aluminum",      // MM41
  "Mini-Aluminum",      // MM51
  "Mini-Aluminum",      // MM61
  "Mini-Aluminum",      // MM62
  "Mini-Aluminum",      // MM71
  "iMac",				// IM41
  "iMac",				// IM51
  "iMac",				// IM61
  "iMac-Aluminum",      // IM71
  "iMac-Aluminum",      // IM81
  "iMac-Aluminum",      // IM91
  "iMac-Aluminum",      // IM101
  "iMac-Aluminum",      // IM111
  "iMac-Aluminum",      // IM112
  "iMac-Aluminum",      // IM113
  "iMac-Aluminum",      // IM121
  "iMac-Aluminum",      // IM122
  "iMac-Aluminum",      // IM131
  "iMac-Aluminum",      // IM132
  "iMac-Aluminum",      // IM141
  "iMac-Aluminum",      // IM142
  "iMac-Aluminum",      // IM143
  "iMac-Aluminum",      // IM144
  "iMac-Aluminum",      // IM151
  "iMac-Aluminum",      // IM161
  "iMac-Aluminum",      // IM162
  "iMac-Aluminum",      // IM171
  "Pro-Enclosure",      // MP11
  "Pro-Enclosure",      // MP21
  "Pro-Enclosure",      // MP31
  "Pro-Enclosure",      // MP41
  "Pro-Enclosure",      // MP51
  "Pro-Enclosure",      // MP61
};

//TODO - find more information and correct all SMC arrays
//RPlt
CHAR8* SmcPlatform[] =
{
  "m70",  // MacBook1,1,
  "m75",  // MacBook2,1,
  "k36",  // MacBook3,1,
  "m82",  // MacBook4,1,
  "m97",  // MacBook5,1,
  "k36b", // MacBook5,2,
  "NA",   // MacBook6,1, // need to find RPlt key
  "k87",  // MacBook7,1,
  "j92",  // MacBook8,1,
  "j93",  // MacBook9,1,
  "m1",   // MacBookPro1,1,
  "NA",   // MacBookPro2,1,		// need to find RPlt key
  "NA",   // MacBookPro2,2,   // need to find RPlt key
  "NA",   // MacBookPro3,1,   // need to find RPlt key
  "m87",  // MacBookPro4,1,
  "m98",  // MacBookPro5,1,
  "NA",   // MacBookPro5,3,   // need to find RPlt key
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
  "NA",   // MacBookPro11,5,  // need to find RPlt key
  "j52",  // MacBookPro12,1,
  "NA",   // MacBookPro13,1, // need to find RPlt key
  "j79",  // MacBookPro13,2,
  "NA",   // MacBookPro13,3,  // need to find RPlt key
  "NA",   // MacBookAir1,1,   // need to find RPlt key
  "NA",   // MacBookAir2,1,   // need to find RPlt key
  "k99",  // MacBookAir3,1,
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
  "NA",   // Macmini6,1,      // need to find RPlt key
  "j50s", // Macmini6,2,
  "j64",  // Macmini7,1,
  "m38",  // iMac4,1,
  "NA",   // iMac5,1,			// need to find RPlt key
  "NA",   // iMac6,1,			// need to find RPlt key
  "NA",   // iMac7,1,			// need to find RPlt key
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
  "j16",  // iMac14,1,
  "j17",  // iMac14,2,
  "j16g", // iMac14,3,
  "j70",  // iMac14,4,
  "j78",  // iMac15,1,
  "j117", // iMac16,1,
  "j94",  // iMac16,2,
  "j95",  // iMac17,1,
  "m43",  // MacPro1,1,
  "m43a", // MacPro2,1,
  "m86",  // MacPro3,1,
  "NA",   // MacPro4,1, // need to find RPlt key
  "k5",   // MacPro5,1,
  "j90",  // MacPro6,1,
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
  { 0x02, 0x35, 0x0F, 0, 1, 0x02 },   // MacBook9,1,
  { 0x01, 0x02, 0x0F, 0, 0, 0x10 },   // MacBookPro1,1,
  { 0x01, 0x14, 0x0F, 0, 0, 0x05 },   // MacBookPro2,1,
  { 0x01, 0x13, 0x0F, 0, 0, 0x03 },   // MacBookPro2,2,
  { 0x01, 0x16, 0x0F, 0, 0, 0x11 },   // MacBookPro3,1,
  { 0x01, 0x27, 0x0F, 0, 0, 0x03 },   // MacBookPro4,1,
  { 0x01, 0x33, 0x0F, 0, 0, 0x08 },   // MacBookPro5,1,
  { 0x01, 0x48, 0x0F, 0, 0, 0x02 },   // MacBookPro5,3,
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
  { 0x02, 0x30, 0x0F, 0, 0, 0x02 },   // MacBookPro11,5,
  { 0x02, 0x28, 0x0F, 0, 0, 0x07 },   // MacBookPro12,1,
  { 0x02, 0x36, 0x0F, 0, 0, 0x93 },   // MacBookPro13,1,
  { 0x02, 0x37, 0x0F, 0, 0, 0x18 },   // MacBookPro13,2,
  { 0x02, 0x38, 0x0F, 0, 0, 0x05 },   // MacBookPro13,3,
  { 0x01, 0x23, 0x0F, 0, 0, 0x20 },   // MacBookAir1,1,
  { 0x01, 0x34, 0x0F, 0, 0, 0x08 },   // MacBookAir2,1,
  { 0x01, 0x67, 0x0F, 0, 0, 0x10 },   // MacBookAir3,1,
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
  { 0x02, 0x08, 0x0F, 0, 0, 0x00 },   // Macmini6,1,
  { 0x02, 0x08, 0x0F, 0, 0, 0x00 },   // Macmini6,2,
  { 0x02, 0x24, 0x0F, 0, 0, 0x32 },   // Macmini7,1,
  { 0x01, 0x01, 0x0F, 0, 0, 0x05 },   // iMac4,1,
  { 0x01, 0x08, 0x0F, 0, 0, 0x02 },   // iMac5,1,
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
  { 0x02, 0x11, 0x0F, 0, 0, 0x14 },   // iMac13,1,
  { 0x02, 0x11, 0x0F, 0, 0, 0x14 },   // iMac13,2,
  { 0x02, 0x14, 0x0F, 0, 0, 0x24 },   // iMac14,1,
  { 0x02, 0x15, 0x0F, 0, 0, 0x07 },   // iMac14,2,
  { 0x02, 0x17, 0x0F, 0, 0, 0x07 },   // iMac14,3,
  { 0x02, 0x17, 0x0F, 0, 0, 0x92 },   // iMac14,4,
  { 0x02, 0x22, 0x0F, 0, 0, 0x16 },   // iMac15,1,
  { 0x02, 0x31, 0x0F, 0, 0, 0x36 },   // iMac16,1,
  { 0x02, 0x32, 0x0F, 0, 0, 0x20 },   // iMac16,2,
  { 0x02, 0x33, 0x0F, 0, 0, 0x10 },   // iMac17,1,
  { 0x01, 0x07, 0x0F, 0, 0, 0x10 },   // MacPro1,1,
  { 0x01, 0x15, 0x0F, 0, 0, 0x03 },   // MacPro2,1,
  { 0x01, 0x30, 0x0F, 0, 0, 0x03 },   // MacPro3,1,
  { 0x01, 0x39, 0x0F, 0, 0, 0x05 },   // MacPro4,1,
  { 0x01, 0x39, 0x0F, 0, 0, 0x11 },   // MacPro5,1,
  { 0x02, 0x20, 0x0F, 0, 0, 0x18 },   // MacPro6,1,
};

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
  "m1",        // MacBookPro1,1,
  "NA",        // MacBookPro2,1,   // need to find RBr key
  "NA",        // MacBookPro2,2,   // need to find RBr key
  "NA",        // MacBookPro3,1,   // need to find RBr key
  "m87",       // MacBookPro4,1,
  "m98",       // MacBookPro5,1,
  "NA",        // MacBookPro5,3,   // need to find RBr key
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
  "NA",        // MacBookPro11,5,  // need to find RBr key
  "j52",       // MacBookPro12,1,
  "NA",        // MacBookPro13,1,  // need to find RBr key
  "j79",       // MacBookPro13,2,
  "NA",        // MacBookPro13,3,  // need to find RBr key
  "NA",        // MacBookAir1,1,   // need to find RBr key
  "NA",        // MacBookAir2,1,   // need to find RBr key
  "k16k99",    // MacBookAir3,1,
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
  "NA",        // Macmini6,1,      // need to find RBr key
  "j50s",      // Macmini6,2,
  "j64",       // Macmini7,1,
  "m38m39",    // iMac4,1,
  "NA",        // iMac5,1,         // need to find RBr key
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
  "j16j17",    // iMac14,1,
  "j16j17",    // iMac14,2,
  "j16g",      // iMac14,3,
  "j70",       // iMac14,4,
  "j78j78am",  // iMac15,1,
  "j117",      // iMac16,1,
  "j94",       // iMac16,2,
  "j95j95am",  // iMac17,1,
  "m43",       // MacPro1,1,
  "m43a",      // MacPro2,1,
  "m86",       // MacPro3,1,
  "NA",        // MacPro4,1,       // need to find RBr key
  "k5",        // MacPro5,1,
  "j90",       // MacPro6,1,
};

//EPCI
UINT32 SmcConfig[] =
{
  0x71001,  //"MacBook1,1",
  0x72001,  //"MacBook2,1",
  0x72001,  //"MacBook3,1",			// need to find EPCI key
  0x74001,  //"MacBook4,1",
  0x7a002,  //"MacBook5,1",      // need to find EPCI key
  0x7a002,  //"MacBook5,2",
  0x7a002,  //"MacBook6,1", // need to find EPCI key
  0x7a002,  //"MacBook7,1", // need to find EPCI key
  0xf0e007, //"MacBook8,1",
  0xf0e007, //"MacBook9,1",      // need to find EPCI key
  0x7b002,  //"MacBookPro1,1",   // need to find EPCI key
  0x7b002,  //"MacBookPro2,1",		// need to find EPCI key
  0x7b002,  //"MacBookPro2,2",   // need to find EPCI key
  0x7b002,  //"MacBookPro3,1",   // need to find EPCI key
  0x7b002,  //"MacBookPro4,1",   // need to find EPCI key
  0x7b002,  //"MacBookPro5,1",
  0x7b002,  //"MacBookPro5,3",   // need to find EPCI key
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
  0xf0b007, //"MacBookPro11,5",  // need to find EPCI key
  0xf01008, //"MacBookPro12,1",
  0xf02009, //"MacBookPro13,1",  // need to find EPCI key
  0xf02009, //"MacBookPro13,2",
  0xf02009, //"MacBookPro13,3",  // need to find EPCI key
  0x76005,  //"MacBookAir1,1",   // need to find EPCI key
  0x76005,  //"MacBookAir2,1",   // need to find EPCI key
  0x76005,  //"MacBookAir3,1",
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
  0x7d006,  //"Macmini6,1",      // need to find EPCI key
  0x7d006,  //"Macmini6,2",
  0xf04008, //"Macmini7,1",
  0x73002,  //"iMac4,1",			// need to find EPCI key
  0x73002,  //"iMac5,1",			// need to find EPCI key
  0x73002,  //"iMac6,1",			// need to find EPCI key
  0x73002,  //"iMac7,1",			// need to find EPCI key
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
  0x79007,  //"iMac14,1",
  0x7a007,  //"iMac14,2",
  0x7a007,  //"iMac14,3",        // need to find EPCI key
  0x7a007,  //"iMac14,4",        // need to find EPCI key
  0xf00008, //"iMac15,1",
  0xf00008, //"iMac16,1",			// need to find EPCI key
  0xf00008, //"iMac16,2",			// need to find EPCI key
  0xf0c008, //"iMac17,1",
  0x79001,  //"MacPro1,1",      // need to find EPCI key
  0x79001,  //"MacPro2,1",      // need to find EPCI key
  0x79001,  //"MacPro3,1",
  0x7c002,  //"MacPro4,1",
  0x7c002,  //"MacPro5,1",
  0xf0f006, //"MacPro6,1",
};


CHAR8 *AppleBoardSN       = "C02140302D5DMT31M";
CHAR8 *AppleBoardLocation = "Part Component";

VOID
SetDMISettingsForModel (MACHINE_TYPES Model, BOOLEAN Redefine)
{
  AsciiStrCpy (gSettings.VendorName,           BiosVendor);
  AsciiStrCpy (gSettings.RomVersion,           AppleFirmwareVersion[Model]);
  AsciiStrCpy (gSettings.ReleaseDate,          AppleReleaseDate[Model]);
  AsciiStrCpy (gSettings.ManufactureName,      BiosVendor);
  if (Redefine) {
    AsciiStrCpy (gSettings.ProductName,          AppleProductName[Model]);
  }
  AsciiStrCpy (gSettings.VersionNr,            AppleSystemVersion[Model]);
  AsciiStrCpy (gSettings.SerialNr,             AppleSerialNumber[Model]);
  AsciiStrCpy (gSettings.FamilyName,           AppleFamilies[Model]);
  AsciiStrCpy (gSettings.BoardManufactureName, BiosVendor);
  AsciiStrCpy (gSettings.BoardSerialNumber,    AppleBoardSN);
  AsciiStrCpy (gSettings.BoardNumber,          AppleBoardID[Model]);
  AsciiStrCpy (gSettings.BoardVersion,         AppleProductName[Model]);
  AsciiStrCpy (gSettings.LocationInChassis,    AppleBoardLocation);
  AsciiStrCpy (gSettings.ChassisManufacturer,  BiosVendor);
  AsciiStrCpy (gSettings.ChassisAssetTag,      AppleChassisAsset[Model]);

  if (Model >= MacPro31) {
    gSettings.BoardType = BaseBoardTypeProcessorMemoryModule; //11;
  } else {
    gSettings.BoardType = BaseBoardTypeMotherBoard; //10;
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
    case MacBook81:
    case MacBook91:
    case MacBookAir11:
    case MacBookAir21:
    case MacBookAir31:
    case MacBookAir41:
    case MacBookAir42:
    case MacBookAir51:
    case MacBookAir52:
    case MacBookAir61:
    case MacBookAir62:
    case MacBookAir71:
    case MacBookAir72:
    case MacBookPro111:
    case MacBookPro112:
    case MacBookPro113:
    case MacBookPro115:
    case MacBookPro121:
    case MacBookPro131:
    case MacBookPro132:
    case MacBookPro133:
      gSettings.ChassisType = MiscChassisTypeNotebook; //10;
      gSettings.Mobile      = TRUE;
      break;

    case MacBookPro11:
    case MacBookPro21:
    case MacBookPro22:
    case MacBookPro31:
    case MacBookPro41:
    case MacBookPro51:
    case MacBookPro53:
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
      gSettings.ChassisType = MiscChassisTypePortable; //08;
      gSettings.Mobile      = TRUE;
      break;

    case iMac41:
    case iMac51:
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
    case iMac141:
    case iMac142:
    case iMac143:
    case iMac144:
    case iMac151:
    case iMac161:
    case iMac162:
    case iMac171:
      gSettings.ChassisType = MiscChassisTypeAllInOne; //13;
      gSettings.Mobile      = FALSE;
      break;

    case MacMini11:
    case MacMini21:
    case MacMini31:
    case MacMini41:
    case MacMini51:
    case MacMini61:
    case MacMini62:
    case MacMini71:
      gSettings.ChassisType = MiscChassisTypeLunchBox; //16;
      gSettings.Mobile      = FALSE;
      break;

    case MacPro11:
    case MacPro21:
    case MacPro31:
    case MacPro41:
    case MacPro51:
      gSettings.ChassisType = MiscChassisTypeMiniTower; //06;
      gSettings.Mobile      = FALSE;
      break;

    case MacPro61:
      gSettings.ChassisType = MiscChassisTypeUnknown;  //02; this is a joke but think different!
      gSettings.Mobile      = FALSE;
      break;

    default: //unknown - use oem SMBIOS value to be default
      gSettings.Mobile      = gMobile;
      gSettings.ChassisType = 0; //let SMBIOS value to be
      /*      if (gMobile) {
       gSettings.ChassisType = 10; //notebook
       } else {
       gSettings.ChassisType = MiscChassisTypeDeskTop; //03;
       } */
      break;
  }
  //RPlt helper
  if (SmcPlatform[Model][0] != 'N') {
    AsciiStrCpy (gSettings.RPlt, SmcPlatform[Model]);
  } else {
    switch (gCPUStructure.Model) {
      case CPU_MODEL_PENTIUM_M:
      case CPU_MODEL_CELERON:
        AsciiStrCpy (gSettings.RPlt, "m70");
        break;

      case CPU_MODEL_YONAH:
        AsciiStrCpy (gSettings.RPlt, "k22");
        break;

      case CPU_MODEL_MEROM: //TODO check for mobile
        AsciiStrCpy (gSettings.RPlt, "m75");
        break;

      case CPU_MODEL_PENRYN:
        if (gSettings.Mobile) {
          AsciiStrCpy (gSettings.RPlt, "m82");
        } else {
          AsciiStrCpy (gSettings.RPlt, "k36");
        }
        break;

      case CPU_MODEL_SANDY_BRIDGE:
        if (gSettings.Mobile) {
          AsciiStrCpy (gSettings.RPlt, "k90i");
        } else {
          AsciiStrCpy (gSettings.RPlt, "k60");
        }
        break;

      case CPU_MODEL_IVY_BRIDGE:
        AsciiStrCpy (gSettings.RPlt, "j30");
        break;

      case CPU_MODEL_IVY_BRIDGE_E5:
        AsciiStrCpy (gSettings.RPlt, "j90");
        break;

      case CPU_MODEL_HASWELL_ULT:
        AsciiStrCpy (gSettings.RPlt, "j44");
        break;

      case CPU_MODEL_SKYLAKE_D:
        AsciiStrCpy (gSettings.RPlt, "j95");
        break;

      case CPU_MODEL_SKYLAKE_U:
        AsciiStrCpy (gSettings.RPlt, "j79");
        break;

      default:
        AsciiStrCpy (gSettings.RPlt, "t9");
        break;
    }
  }
    
  //RBr helper
  if (SmcBranch[Model][0] != 'N') {
    AsciiStrCpy (gSettings.RBr, SmcBranch[Model]);
  } else {
    switch (gCPUStructure.Model) {
      case CPU_MODEL_PENTIUM_M:
      case CPU_MODEL_CELERON:
        AsciiStrCpy (gSettings.RBr, "m70");
        break;
                
      case CPU_MODEL_YONAH:
        AsciiStrCpy (gSettings.RBr, "k22");
        break;
                
      case CPU_MODEL_MEROM: //TODO check for mobile
        AsciiStrCpy (gSettings.RBr, "m75");
        break;
                
      case CPU_MODEL_PENRYN:
        if (gSettings.Mobile) {
          AsciiStrCpy (gSettings.RBr, "m82");
        } else {
          AsciiStrCpy (gSettings.RBr, "k36");
        }
        break;
                
      case CPU_MODEL_SANDY_BRIDGE:
        if (gSettings.Mobile) {
          AsciiStrCpy (gSettings.RBr, "k90i");
        } else {
          AsciiStrCpy (gSettings.RBr, "k60");
        }
        break;
                
      case CPU_MODEL_IVY_BRIDGE:
        AsciiStrCpy (gSettings.RBr, "j30");
        break;
                
      case CPU_MODEL_IVY_BRIDGE_E5:
        AsciiStrCpy (gSettings.RBr, "j90");
        break;
                
      case CPU_MODEL_HASWELL_ULT:
        AsciiStrCpy (gSettings.RBr, "j44");
        break;
                
      case CPU_MODEL_SKYLAKE_D:
        AsciiStrCpy (gSettings.RBr, "j95");
        break;
                
      case CPU_MODEL_SKYLAKE_U:
        AsciiStrCpy (gSettings.RBr, "j79");
        break;
                
      default:
        AsciiStrCpy (gSettings.RBr, "t9");
        break;
    }
  }

  CopyMem (gSettings.REV,  SmcRevision[Model], 6);
  CopyMem (gSettings.EPCI, &SmcConfig[Model],  4);
}

//Other info
/*
 MacBookPro7,1 - penryn P8800 RPlt=k6 REV=1.62f5
 MacBookPro6,2 - i5 M520 arrandale
 */

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
  gSettings.DualLink             = 1;
  gSettings.HDAInjection         = TRUE;
  //gSettings.HDALayoutId          = 0;
  gSettings.USBInjection         = TRUE; // enabled by default to have the same behavior as before
  StrCpy (gSettings.DsdtName, L"DSDT.aml");
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
