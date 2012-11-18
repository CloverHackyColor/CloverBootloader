/****************************************************************************/	
/*Portion I: Definitions  shared between VBIOS and Driver                   */
/****************************************************************************/


#ifndef _SHORT_ATOMBIOS_H
#define _SHORT_ATOMBIOS_H

#define ATOM_VERSION_MAJOR                   0x00020000
#define ATOM_VERSION_MINOR                   0x00000002

#define ATOM_HEADER_VERSION (ATOM_VERSION_MAJOR | ATOM_VERSION_MINOR)

typedef unsigned char		BOOLEAN;
typedef signed char			INT8;
typedef unsigned char		UINT8;
typedef signed short		INT16;
typedef unsigned short		UINT16;
typedef signed long			INT32;
typedef unsigned long		UINT32;
typedef unsigned char		CHAR8;
typedef unsigned short		CHAR16;
typedef unsigned short		USHORT;
typedef unsigned char		UCHAR;
typedef	unsigned long		ULONG;

#pragma pack(1)                                       /* BIOS data must use byte aligment */

/*  Define offset to location of ROM header. */

#define OFFSET_TO_POINTER_TO_ATOM_ROM_HEADER		0x00000048L
#define OFFSET_TO_ATOM_ROM_IMAGE_SIZE				    0x00000002L

typedef struct _ATOM_COMMON_TABLE_HEADER
{
	USHORT usStructureSize;
	UCHAR  ucTableFormatRevision;   /*Change it when the Parser is not backward compatible */
	UCHAR  ucTableContentRevision;  /*Change it only when the table needs to change but the firmware */
                                  /*Image can't be updated, while Driver needs to carry the new table! */
} ATOM_COMMON_TABLE_HEADER;

typedef struct _ATOM_ROM_HEADER
{
	ATOM_COMMON_TABLE_HEADER		sHeader;
	UCHAR	 uaFirmWareSignature[4];    /*Signature to distinguish between Atombios and non-atombios, 
											 +                                      atombios should init it as "ATOM", don't change the position */
	USHORT usBiosRuntimeSegmentAddress;
	USHORT usProtectedModeInfoOffset;
	USHORT usConfigFilenameOffset;
	USHORT usCRC_BlockOffset;
	USHORT usBIOS_BootupMessageOffset;
	USHORT usInt10Offset;
	USHORT usPciBusDevInitCode;
	USHORT usIoBaseAddress;
	USHORT usSubsystemVendorID;
	USHORT usSubsystemID;
	USHORT usPCI_InfoOffset; 
	USHORT usMasterCommandTableOffset; /*Offset for SW to get all command table offsets, Don't change the position */
	USHORT usMasterDataTableOffset;   /*Offset for SW to get all data table offsets, Don't change the position */
	UCHAR  ucExtendedFunctionCode;
	UCHAR  ucReserved;
} ATOM_ROM_HEADER;

/****************************************************************************/	
// Structure used in Data.mtb
/****************************************************************************/	
typedef struct _ATOM_MASTER_LIST_OF_DATA_TABLES
{
	USHORT        UtilityPipeLine;	        // Offest for the utility to get parser info,Don't change this position!
	USHORT        MultimediaCapabilityInfo; // Only used by MM Lib,latest version 1.1, not configuable from Bios, need to include the table to build Bios 
	USHORT        MultimediaConfigInfo;     // Only used by MM Lib,latest version 2.1, not configuable from Bios, need to include the table to build Bios
	USHORT        StandardVESA_Timing;      // Only used by Bios
	USHORT        FirmwareInfo;             // Shared by various SW components,latest version 1.4
	USHORT        DAC_Info;                 // Will be obsolete from R600
	USHORT        LVDS_Info;                // Shared by various SW components,latest version 1.1 
	USHORT        TMDS_Info;                // Will be obsolete from R600
	USHORT        AnalogTV_Info;            // Shared by various SW components,latest version 1.1 
	USHORT        SupportedDevicesInfo;     // Will be obsolete from R600
	USHORT        GPIO_I2C_Info;            // Shared by various SW components,latest version 1.2 will be used from R600           
	USHORT        VRAM_UsageByFirmware;     // Shared by various SW components,latest version 1.3 will be used from R600
	USHORT        GPIO_Pin_LUT;             // Shared by various SW components,latest version 1.1
	USHORT        VESA_ToInternalModeLUT;   // Only used by Bios
	USHORT        ComponentVideoInfo;       // Shared by various SW components,latest version 2.1 will be used from R600
	USHORT        PowerPlayInfo;            // Shared by various SW components,latest version 2.1,new design from R600
	USHORT        CompassionateData;        // Will be obsolete from R600
	USHORT        SaveRestoreInfo;          // Only used by Bios
	USHORT        PPLL_SS_Info;             // Shared by various SW components,latest version 1.2, used to call SS_Info, change to new name because of int ASIC SS info
	USHORT        OemInfo;                  // Defined and used by external SW, should be obsolete soon
	USHORT        XTMDS_Info;               // Will be obsolete from R600
	USHORT        MclkSS_Info;              // Shared by various SW components,latest version 1.1, only enabled when ext SS chip is used
	USHORT        Object_Header;            // Shared by various SW components,latest version 1.1
	USHORT        IndirectIOAccess;         // Only used by Bios,this table position can't change at all!!
	USHORT        MC_InitParameter;         // Only used by command table
	USHORT        ASIC_VDDC_Info;						// Will be obsolete from R600
	USHORT        ASIC_InternalSS_Info;			// New tabel name from R600, used to be called "ASIC_MVDDC_Info"
	USHORT        TV_VideoMode;							// Only used by command table
	USHORT        VRAM_Info;								// Only used by command table, latest version 1.3
	USHORT        MemoryTrainingInfo;				// Used for VBIOS and Diag utility for memory training purpose since R600. the new table rev start from 2.1
	USHORT        IntegratedSystemInfo;			// Shared by various SW components
	USHORT        ASIC_ProfilingInfo;				// New table name from R600, used to be called "ASIC_VDDCI_Info" for pre-R600
	USHORT        VoltageObjectInfo;				// Shared by various SW components, latest version 1.1
	USHORT				PowerSourceInfo;					// Shared by various SW components, latest versoin 1.1
} ATOM_MASTER_LIST_OF_DATA_TABLES;

typedef struct _ATOM_MASTER_DATA_TABLE
{ 
	ATOM_COMMON_TABLE_HEADER sHeader;  
	ATOM_MASTER_LIST_OF_DATA_TABLES   ListOfDataTables;
} ATOM_MASTER_DATA_TABLE;

typedef union _ATOM_MODE_MISC_INFO_ACCESS
{ 
	USHORT              usAccess;
} ATOM_MODE_MISC_INFO_ACCESS;

/****************************************************************************/	
// Structure used in StandardVESA_TimingTable
//                   AnalogTV_InfoTable 
//                   ComponentVideoInfoTable
/****************************************************************************/	
typedef struct _ATOM_MODE_TIMING
{
	USHORT  usCRTC_H_Total;
	USHORT  usCRTC_H_Disp;
	USHORT  usCRTC_H_SyncStart;
	USHORT  usCRTC_H_SyncWidth;
	USHORT  usCRTC_V_Total;
	USHORT  usCRTC_V_Disp;
	USHORT  usCRTC_V_SyncStart;
	USHORT  usCRTC_V_SyncWidth;
	USHORT  usPixelClock;					                 //in 10Khz unit
	ATOM_MODE_MISC_INFO_ACCESS  susModeMiscInfo;
	USHORT  usCRTC_OverscanRight;
	USHORT  usCRTC_OverscanLeft;
	USHORT  usCRTC_OverscanBottom;
	USHORT  usCRTC_OverscanTop;
	USHORT  usReserve;
	UCHAR   ucInternalModeNumber;
	UCHAR   ucRefreshRate;
} ATOM_MODE_TIMING;

typedef struct _ATOM_DTD_FORMAT
{
	USHORT  usPixClk;
	USHORT  usHActive;
	USHORT  usHBlanking_Time;
	USHORT  usVActive;
	USHORT  usVBlanking_Time;			
	USHORT  usHSyncOffset;
	USHORT  usHSyncWidth;
	USHORT  usVSyncOffset;
	USHORT  usVSyncWidth;
	USHORT  usImageHSize;
	USHORT  usImageVSize;
	UCHAR   ucHBorder;
	UCHAR   ucVBorder;
	ATOM_MODE_MISC_INFO_ACCESS susModeMiscInfo;
	UCHAR   ucInternalModeNumber;
	UCHAR   ucRefreshRate;
} ATOM_DTD_FORMAT;

typedef struct _ATOM_LVDS_INFO_V12
{
	ATOM_COMMON_TABLE_HEADER sHeader;  
	ATOM_DTD_FORMAT     sLCDTiming;
	USHORT              usExtInfoTableOffset;
	USHORT              usSupportedRefreshRate;     //Refer to panel info table in ATOMBIOS extension Spec.
	USHORT              usOffDelayInMs;
	UCHAR               ucPowerSequenceDigOntoDEin10Ms;
	UCHAR               ucPowerSequenceDEtoBLOnin10Ms;
	UCHAR               ucLVDS_Misc;            // Bit0:{=0:single, =1:dual},Bit1 {=0:666RGB, =1:888RGB},Bit2:3:{Grey level}
												// Bit4:{=0:LDI format for RGB888, =1 FPDI format for RGB888}
	                                            // Bit5:{=0:Spatial Dithering disabled;1 Spatial Dithering enabled}
	                                            // Bit6:{=0:Temporal Dithering disabled;1 Temporal Dithering enabled}
	UCHAR               ucPanelDefaultRefreshRate;
	UCHAR               ucPanelIdentification;
	UCHAR               ucSS_Id;
	USHORT              usLCDVenderID;
	USHORT              usLCDProductID;
	UCHAR               ucLCDPanel_SpecialHandlingCap; 
	UCHAR								ucPanelInfoSize;					//  start from ATOM_DTD_FORMAT to end of panel info, include ExtInfoTable
	UCHAR               ucReserved[2];
} ATOM_LVDS_INFO_V12;
	
	
typedef struct _ATOM_STANDARD_VESA_TIMING
{
	ATOM_COMMON_TABLE_HEADER sHeader;  
	char * 				 aModeTimings;      // 16 is not the real array number, just for initial allocation
} ATOM_STANDARD_VESA_TIMING;

#endif