/*
 * Copyright 2010 AsereBLN. All rights reserved. <aserebln@googlemail.com>
 *
 * spd.h
 */

#ifndef __LIBSAIO_SPD_H
#define __LIBSAIO_SPD_H

/*
 * Serial Presence Detect (SPD) data stored on SDRAM modules.
 *
 * Datasheet:
 *  -- the link is broken!
 *   - Name: PC SDRAM Serial Presence Detect (SPD) Specification
 *           Revision 1.2A, December, 1997
 *   - PDF: http://www.intel.com/design/chipsets/memory/spdsd12a.pdf
 *
 * Datasheet (alternative):
 *  -- the link is not accessible without registration
 *   - Name: SERIAL PRESENCE DETECT STANDARD, General Standard
 *           JEDEC Standard No. 21-C
 *   - PDF: http://www.jedec.org/download/search/4_01_02_00R9.PDF
 */


/* Byte numbers. */
#define SPD_NUM_MANUFACTURER_BYTES          0  /* Number of bytes used by module manufacturer */
#define SPD_TOTAL_SPD_MEMORY_SIZE           1  /* Total SPD memory size | Byte 1 (0x001): SPD Revision*/ 
#define SPD_MEMORY_TYPE                     2  /* (Fundamental) memory type */
#define SPD_NUM_ROWS                        3  /* Number of row address bits */
#define SPD_NUM_COLUMNS                     4  /* Number of column address bits */
#define SPD_NUM_DIMM_BANKS                  5  /* Number of module rows (banks) */
#define SPD_MODULE_DATA_WIDTH_LSB           6  /* Module data width (LSB) */
#define SPD_MODULE_DATA_WIDTH_MSB           7  /* Module data width (MSB) */
#define SPD_MODULE_VOLTAGE                  8  /* Module interface signal levels */
#define SPD_MIN_CYCLE_TIME_AT_CAS_MAX       9  /* SDRAM cycle time (highest CAS latency), RAS access time (tRAC) */
#define SPD_ACCESS_TIME_FROM_CLOCK          10 /* SDRAM access time from clock (highest CAS latency), CAS access time (Tac, tCAC) */
#define SPD_DIMM_CONFIG_TYPE                11 /* Module configuration type */
#define SPD_REFRESH                         12 /* Refresh rate/type */
#define SPD_PRIMARY_SDRAM_WIDTH             13 /* SDRAM width (primary SDRAM) */
#define SPD_ERROR_CHECKING_SDRAM_WIDTH      14 /* Error checking SDRAM (data) width */
#define SPD_MIN_CLOCK_DELAY_B2B_RAND_COLUMN 15 /* SDRAM device attributes, minimum clock delay for back to back random column */
#define SPD_SUPPORTED_BURST_LENGTHS         16 /* SDRAM device attributes, burst lengths supported */
#define SPD_NUM_BANKS_PER_SDRAM             17 /* SDRAM device attributes, number of banks on SDRAM device */
#define SPD_ACCEPTABLE_CAS_LATENCIES        18 /* SDRAM device attributes, CAS latency */
#define SPD_CS_LATENCY                      19 /* SDRAM device attributes, CS latency */
#define SPD_WE_LATENCY                      20 /* SDRAM device attributes, WE latency */
#define SPD_MODULE_ATTRIBUTES               21 /* SDRAM module attributes */
#define SPD_DEVICE_ATTRIBUTES_GENERAL       22 /* SDRAM device attributes, general */
#define SPD_SDRAM_CYCLE_TIME_2ND            23 /* SDRAM cycle time (2nd highest CAS latency) */
#define SPD_ACCESS_TIME_FROM_CLOCK_2ND      24 /* SDRAM access from clock (2nd highest CAS latency) */
#define SPD_SDRAM_CYCLE_TIME_3RD            25 /* SDRAM cycle time (3rd highest CAS latency) */
#define SPD_ACCESS_TIME_FROM_CLOCK_3RD      26 /* SDRAM access from clock (3rd highest CAS latency) */
#define SPD_MIN_ROW_PRECHARGE_TIME          27 /* Minimum row precharge time (Trp) */
#define SPD_MIN_ROWACTIVE_TO_ROWACTIVE      28 /* Minimum row active to row active (Trrd) */
#define SPD_MIN_RAS_TO_CAS_DELAY            29 /* Minimum RAS to CAS delay (Trcd) */
#define SPD_MIN_ACTIVE_TO_PRECHARGE_DELAY   30 /* Minimum RAS pulse width (Tras) */
#define SPD_DENSITY_OF_EACH_ROW_ON_MODULE   31 /* Density of each row on module */
#define SPD_CMD_SIGNAL_INPUT_SETUP_TIME     32 /* Command and address signal input setup time */
#define SPD_CMD_SIGNAL_INPUT_HOLD_TIME      33 /* Command and address signal input hold time */
#define SPD_DATA_SIGNAL_INPUT_SETUP_TIME    34 /* Data signal input setup time */
#define SPD_DATA_SIGNAL_INPUT_HOLD_TIME     35 /* Data signal input hold time */
#define SPD_WRITE_RECOVERY_TIME             36 /* Write recovery time (tWR) */
#define SPD_INT_WRITE_TO_READ_DELAY         37 /* Internal write to read command delay (tWTR) */
#define SPD_INT_READ_TO_PRECHARGE_DELAY     38 /* Internal read to precharge command delay (tRTP) */
#define SPD_MEM_ANALYSIS_PROBE_PARAMS       39 /* Memory analysis probe characteristics */
#define SPD_BYTE_41_42_EXTENSION            40 /* Extension of byte 41 (tRC) and byte 42 (tRFC) */
#define SPD_MIN_ACT_TO_ACT_AUTO_REFRESH     41 /* Minimum active to active auto refresh (tRCmin) */
#define SPD_MIN_AUTO_REFRESH_TO_ACT         42 /* Minimum auto refresh to active/auto refresh (tRFC) */
#define SPD_MAX_DEVICE_CYCLE_TIME           43 /* Maximum device cycle time (tCKmax) */
#define SPD_MAX_DQS_DQ_SKEW                 44 /* Maximum skew between DQS and DQ (tDQSQ) */
#define SPD_MAX_READ_DATAHOLD_SKEW          45 /* Maximum read data-hold skew factor (tQHS) */
#define SPD_PLL_RELOCK_TIME                 46 /* PLL relock time */
#define SPD_SPD_DATA_REVISION_CODE          62 /* SPD data revision code */
#define SPD_CHECKSUM_FOR_BYTES_0_TO_62      63 /* Checksum for bytes 0-62 */
#define SPD_MANUFACTURER_JEDEC_ID_CODE      64 /* Manufacturer's JEDEC ID code, per EIA/JEP106 (bytes 64-71) */
#define SPD_MANUFACTURING_LOCATION          72 /* Manufacturing location */
#define SPD_MANUFACTURER_PART_NUMBER        73 /* Manufacturer's part number, in 6-bit ASCII (bytes 73-90) */
#define SPD_REVISION_CODE                   91 /* Revision code (bytes 91-92) */
#define SPD_MANUFACTURING_DATE              93 /* Manufacturing date (byte 93: year, byte 94: week) */
#define SPD_ASSEMBLY_SERIAL_NUMBER          95 /* Assembly serial number (bytes 95-98) */
#define SPD_MANUFACTURER_SPECIFIC_DATA      99 /* Manufacturer specific data (bytes 99-125) */
#define SPD_INTEL_SPEC_FOR_FREQUENCY       126 /* Intel specification for frequency */
#define SPD_INTEL_SPEC_100_MHZ             127 /* Intel specification details for 100MHz support */
#define SPD_DDR3_MEMORY_BANK			   0x75
#define SPD_DDR3_MEMORY_CODE			   0x76

#define SPD_DDR4_MANUFACTURER_ID_BANK       0x140 /* Manufacturer's JEDEC ID code (bytes 140-141) */
#define SPD_DDR4_MANUFACTURER_ID_CODE       0x141 /* Manufacturer's JEDEC ID code (bytes 140-141) */
#define SPD_DDR4_MANUFACTURING_LOCATION     0x142 /* Manufacturing location */
#define SPD_DDR4_MANUFACTURING_DATE         0x143 /* bytes 143-144 */
#define SPD_DDR4_SERIAL_NUMBER              0x145 /* Assembly serial number (bytes 145-148) */
#define SPD_DDR4_MANUFACTURER_PART_NUMBER   0x149 /*  in 6-bit ASCII (bytes 149-15c) */
#define SPD_DDR4_REVISION_CODE              0x15d /* Revision code  */

/* DRAM specifications use the following naming conventions for SPD locations */
#define SPD_tRP                             SPD_MIN_ROW_PRECHARGE_TIME
#define SPD_tRRD                            SPD_MIN_ROWACTIVE_TO_ROWACTIVE
#define SPD_tRCD                            SPD_MIN_RAS_TO_CAS_DELAY
#define SPD_tRAS                            SPD_MIN_ACTIVE_TO_PRECHARGE_DELAY
#define SPD_BANK_DENSITY                    SPD_DENSITY_OF_EACH_ROW_ON_MODULE
#define SPD_ADDRESS_CMD_HOLD                SPD_CMD_SIGNAL_INPUT_HOLD_TIME
#define SPD_tRC								41	/* SDRAM Device Minimum Active to Active/Auto Refresh Time (tRC) */
#define SPD_tRFC							42	/* SDRAM Device Minimum Auto Refresh to Active/Auto Refresh (tRFC) */


/* SPD_MEMORY_TYPE values. */
#define SPD_MEMORY_TYPE_FPM_DRAM            1
#define SPD_MEMORY_TYPE_EDO                 2
#define SPD_MEMORY_TYPE_PIPELINED_NIBBLE    3
#define SPD_MEMORY_TYPE_SDRAM               4
#define SPD_MEMORY_TYPE_MULTIPLEXED_ROM     5
#define SPD_MEMORY_TYPE_SGRAM_DDR           6
#define SPD_MEMORY_TYPE_SDRAM_DDR           7
#define SPD_MEMORY_TYPE_SDRAM_DDR2          8
#define SPD_MEMORY_TYPE_SDRAM_DDR2_FB_DIMM  9
#define SPD_MEMORY_TYPE_SDRAM_DDR2_FB_PROBE 0xa
#define SPD_MEMORY_TYPE_SDRAM_DDR3          0xb
#define SPD_MEMORY_TYPE_SDRAM_DDR4          0xc

/* SPD_MODULE_VOLTAGE values. */
#define SPD_VOLTAGE_TTL						0 /* 5.0 Volt/TTL */
#define SPD_VOLTAGE_LVTTL					1 /* LVTTL */
#define SPD_VOLTAGE_HSTL					2 /* HSTL 1.5 */
#define SPD_VOLTAGE_SSTL3					3 /* SSTL 3.3 */
#define SPD_VOLTAGE_SSTL2					4 /* SSTL 2.5 */

/* SPD_DIMM_CONFIG_TYPE values. */
#define ERROR_SCHEME_NONE					0
#define ERROR_SCHEME_PARITY				1
#define ERROR_SCHEME_ECC					2

/* SPD_ACCEPTABLE_CAS_LATENCIES values. */
// TODO: Check values.
#define SPD_CAS_LATENCY_1_0					0x01
#define SPD_CAS_LATENCY_1_5					0x02
#define SPD_CAS_LATENCY_2_0					0x04
#define SPD_CAS_LATENCY_2_5					0x08
#define SPD_CAS_LATENCY_3_0					0x10
#define SPD_CAS_LATENCY_3_5					0x20
#define SPD_CAS_LATENCY_4_0					0x40

#define SPD_CAS_LATENCY_DDR2_3				(1 << 3)
#define SPD_CAS_LATENCY_DDR2_4				(1 << 4)
#define SPD_CAS_LATENCY_DDR2_5				(1 << 5)
#define SPD_CAS_LATENCY_DDR2_6				(1 << 6)

/* SPD_SUPPORTED_BURST_LENGTHS values. */
#define SPD_BURST_LENGTH_1					1
#define SPD_BURST_LENGTH_2					2
#define SPD_BURST_LENGTH_4					4
#define SPD_BURST_LENGTH_8					8
#define SPD_BURST_LENGTH_PAGE				(1 << 7)

/* SPD_MODULE_ATTRIBUTES values. */
#define MODULE_BUFFERED						1
#define MODULE_REGISTERED					2

//DDR4 specification
/*
Block 2, upper half: Manufacturing Information
Bytes 320~383 (0x140~0x17F) The following table details the location of each byte in this block.
Byte Number Function Described Notes
320 0x140 Module Manufacturer’s ID Code, Least Significant Byte
321 0x141 Module Manufacturer’s ID Code, Most Significant Byte
322 0x142 Module Manufacturing Location
323~324 0x143~0x144 Module Manufacturing Date
325~328 0x145~0x148 Module Serial Number
329~348 0x149~0x15C Module Part Number
349 0x15D Module Revision Code
350 0x15E DRAM Manufacturer’s ID Code, Least Significant Byte
351 0x15F DRAM Manufacturer’s ID Code, Most Significant Byte
352 0x160 DRAM Stepping
353~381 0x161~0x17D Module Manufacturer’s Specific Data
382~383 0x17E~0x17F Reserved; must be coded as 0x00
 */


VOID
ScanSPD (VOID);

#endif /* !__LIBSAIO_SPD_H */
