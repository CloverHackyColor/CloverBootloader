/*
 * spd.c - serial presence detect memory information
 implementation for reading memory spd

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 *
 * Originally restored from pcefi10.5 by netkas
 * Dynamic mem detection original impl. by Rekursor
 * System profiler fix and other fixes by Mozodojo.
 * Slice 2011  remade for UEFI
 * XMP detection - apianti
 */

//
#include "Platform.h"
#include "spd.h"
#include "memvendors.h"


#ifndef DEBUG_SPD
#ifndef DEBUG_ALL
#define DEBUG_SPD 0
#else
#define DEBUG_SPD DEBUG_ALL
#endif
#endif

#if DEBUG_SPD == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_SPD, __VA_ARGS__)
#endif

//extern EFI_DATA_HUB_PROTOCOL			*gDataHub;

extern MEM_STRUCTURE		gRAM;
//extern DMI*					gDMI;

//==>
extern UINT16 TotalCount;
//<==

BOOLEAN             smbIntel;
UINT8				smbPage;

CONST CHAR8 *spd_memory_types[] =
{
  "RAM",          /* 00h  Undefined */
  "FPM",          /* 01h  FPM */
  "EDO",          /* 02h  EDO */
  "",				/* 03h  PIPELINE NIBBLE */
  "SDRAM",        /* 04h  SDRAM */
  "",				/* 05h  MULTIPLEXED ROM */
  "DDR SGRAM",	/* 06h  SGRAM DDR */
  "DDR SDRAM",	/* 07h  SDRAM DDR */
  "DDR2 SDRAM",   /* 08h  SDRAM DDR 2 */
  "",				/* 09h  Undefined */
  "",				/* 0Ah  Undefined */
  "DDR3 SDRAM",	/* 0Bh  SDRAM DDR 3 */
  "DDR4 SDRAM" /* 0Ch SDRAM DDR 4 */
};

#define UNKNOWN_MEM_TYPE 2
UINT8 spd_mem_to_smbios[] =
{
  UNKNOWN_MEM_TYPE,		/* 00h  Undefined */
  UNKNOWN_MEM_TYPE,		/* 01h  FPM */
  UNKNOWN_MEM_TYPE,		/* 02h  EDO */
  UNKNOWN_MEM_TYPE,		/* 03h  PIPELINE NIBBLE */
  SMB_MEM_TYPE_SDRAM,	/* 04h  SDRAM */
  SMB_MEM_TYPE_ROM,		/* 05h  MULTIPLEXED ROM */
  SMB_MEM_TYPE_SGRAM,	/* 06h  SGRAM DDR */
  SMB_MEM_TYPE_DDR,		/* 07h  SDRAM DDR */
  SMB_MEM_TYPE_DDR2,	/* 08h  SDRAM DDR 2 */
  UNKNOWN_MEM_TYPE,		/* 09h  Undefined */
  UNKNOWN_MEM_TYPE,		/* 0Ah  Undefined */
  SMB_MEM_TYPE_DDR3,	/* 0Bh  SDRAM DDR 3 */
  SMB_MEM_TYPE_DDR4   /* 0Ch  SDRAM DDR 4 */
};
#define SPD_TO_SMBIOS_SIZE (sizeof(spd_mem_to_smbios)/sizeof(UINT8))

//define rdtsc(low,high) UINT64=AsmReadTsc()
//define outb(port, val)   IoWrite8(port, val)
//define val=inb(port) val=IoRead(port)

// Intel SMB reg offsets
#define SMBHSTSTS 0
#define SMBHSTCNT 2
#define SMBHSTCMD 3
#define SMBHSTADD 4
#define SMBHSTDAT 5
#define SMBHSTDAT1 6
#define SBMBLKDAT 7
// MCP and nForce SMB reg offsets
#define SMBHPRTCL_NV 0 /* protocol, PEC */
#define SMBHSTSTS_NV 1 /* status */
#define SMBHSTADD_NV 2 /* address */
#define SMBHSTCMD_NV 3 /* command */
#define SMBHSTDAT_NV 4 /* 32 data registers */
//

// XMP memory profile
#define SPD_XMP_SIG1 176
#define SPD_XMP_SIG1_VALUE 0x0C
#define SPD_XMP_SIG2 177
#define SPD_XMP_SIG2_VALUE 0x4A
#define SPD_XMP_PROFILES 178
#define SPD_XMP_VERSION 179
#define SPD_XMP_PROF1_DIVISOR 180
#define SPD_XMP_PROF1_DIVIDEND 181
#define SPD_XMP_PROF2_DIVISOR 182
#define SPD_XMP_PROF2_DIVIDEND 183
#define SPD_XMP_PROF1_RATIO 186
#define SPD_XMP_PROF2_RATIO 221


#define SPD_XMP20_SIG1 0x180
#define SPD_XMP20_SIG2 0x181
#define SPD_XMP20_PROFILES 0x182
#define SPD_XMP20_VERSION 0x183
/* 0x189 */
#define SPD_XMP20_PROF1_MINCYCLE 0x18C
#define SPD_XMP20_PROF1_FINEADJUST 0x1AF
/* 0x1B8 */
#define SPD_XMP20_PROF2_MINCYCLE 0x1BB
#define SPD_XMP20_PROF2_FINEADJUST 0x1DE
/* 0x1E7 */

UINT16 spd_indexes_ddr[] = {
  /* 3 */ SPD_NUM_ROWS,  /* ModuleSize */
  /* 4 */ SPD_NUM_COLUMNS,
  /* 5 */ SPD_NUM_DIMM_BANKS,
  /* 17 */ SPD_NUM_BANKS_PER_SDRAM,
  9, /* Frequency */
  64, /* Manufacturer */
  95,96,97,98, /* UIS */
  0
};

UINT16 spd_indexes_ddr3[] = {
  4,7,8, /* ModuleSize */
  10,11,12, /* Frequency */
  /* 0x75, 0x76 */ SPD_DDR3_MEMORY_BANK, SPD_DDR3_MEMORY_CODE, /* Manufacturer */
  122,123,124,125, /* UIS */
  /* XMP */
  SPD_XMP_SIG1,
  SPD_XMP_SIG2,
  SPD_XMP_PROFILES,
  SPD_XMP_VERSION,
  SPD_XMP_PROF1_DIVISOR,
  SPD_XMP_PROF1_DIVIDEND,
  SPD_XMP_PROF2_DIVISOR,
  SPD_XMP_PROF2_DIVIDEND,
  SPD_XMP_PROF1_RATIO,
  SPD_XMP_PROF2_RATIO,
  0
};

UINT16 spd_indexes_ddr4[] = {
  4,6,12,13, /* ModuleSize */
  18,125, /* Frequency */
  SPD_DDR4_MANUFACTURER_ID_BANK, SPD_DDR4_MANUFACTURER_ID_CODE, /* Manufacturer */
  325,326,327,328, /* UIS */
  /* XMP 2.0 */
  SPD_XMP20_SIG1,
  SPD_XMP20_SIG2,
  SPD_XMP20_PROFILES,
  SPD_XMP20_VERSION,
  SPD_XMP20_PROF1_MINCYCLE,
  SPD_XMP20_PROF1_FINEADJUST,
  SPD_XMP20_PROF2_MINCYCLE,
  SPD_XMP20_PROF2_FINEADJUST,
  0
};

/** Read one byte from i2c, used for reading SPD */

UINT8 smb_read_byte(UINT32 base, UINT8 adr, UINT16 cmd)
{
  //   INTN l1, h1, l2, h2;
  UINT64 t, t1, t2;
  UINT8 page;
  UINT8 c;
  //	UINT8 p;

  if (smbIntel) {
    IoWrite8(base + SMBHSTSTS, 0x1f);				// reset SMBus Controller (set busy)
    IoWrite8(base + SMBHSTDAT, 0xff);

    t1 = AsmReadTsc(); //rdtsc(l1, h1);
    while ( IoRead8(base + SMBHSTSTS) & 0x01) {   // wait until host is not busy
      t2 = AsmReadTsc(); //rdtsc(l2, h2);
      t = DivU64x64Remainder((t2 - t1),
                             DivU64x32(gCPUStructure.TSCFrequency, 1000),
                             0);
      if (t > 5) {
        DBG("host is busy for too long for byte %2X:%d!\n", adr, cmd);
        return 0xFF;                  // break
      }
    }

    page = (cmd >> 8) & 1;
    if (page != smbPage) {
      // p = 0xFF;
      IoWrite8(base + SMBHSTCMD, 0x00);
      IoWrite8(base + SMBHSTADD, 0x6C + (page << 1)); // Set SPD Page Address
#if 0
      IoWrite8(base + SMBHSTCNT, 0x48); // Start + Byte Data Write
      // Don't use "Byte Data Write" because status goes from 0x41 (Busy) -> 0x44 (Error)
#else
      IoWrite8(base + SMBHSTCNT, 0x40); // Start + Quick Write
      // status goes from 0x41 (Busy) -> 0x42 (Completed)
#endif
      smbPage = page;

      t1 = AsmReadTsc();
      while (!( (c=IoRead8(base + SMBHSTSTS)) & 0x02)) {	// wait until command finished
        t2 = AsmReadTsc();
        t = DivU64x64Remainder((t2 - t1), DivU64x32(gCPUStructure.TSCFrequency, 1000), 0);
        /*
         if (c != p) {
         DBG("%02d %02X spd page change status %2X\n", t, cmd, c);
         p = c;
         }
         */
        if (c & 4) {
          DBG("spd page change error for byte %2X:%d!\n", adr, cmd);
          break;
        }
        if (t > 5) {
          DBG("spd page change taking too long for byte %2X:%d!\n", adr, cmd);
          break;									// break after 5ms
        }
      }
      return smb_read_byte(base, adr, cmd);
    }

   	// p = 0xFF;
    IoWrite8(base + SMBHSTCMD, (UINT8)(cmd & 0xFF)); // SMBus uses 8 bit commands
    IoWrite8(base + SMBHSTADD, (adr << 1) | 0x01 ); // read from spd
    IoWrite8(base + SMBHSTCNT, 0x48 ); // Start + Byte Data Read
    // status goes from 0x41 (Busy) -> 0x42 (Completed) or 0x44 (Error)

    t1 = AsmReadTsc();
    while (!( (c=IoRead8(base + SMBHSTSTS)) & 0x02)) {	// wait until command finished
      t2 = AsmReadTsc();
      t = DivU64x64Remainder((t2 - t1), DivU64x32(gCPUStructure.TSCFrequency, 1000), 0);
      /*
       if (c != p) {
       DBG("%02d %02X spd read status %2X\n", t, cmd, c);
       p = c;
       }
       */
      if (c & 4) {
        // This alway happens when trying to read the memory type (cmd 2) of an empty slot
        // DBG("spd byte read error for byte %2X:%d!\n", adr, cmd);
        break;
      }
      if (t > 5) {
        // if (cmd != 2)
        DBG("spd byte read taking too long for byte %2X:%d!\n", adr, cmd);
        break;									// break after 5ms
      }
    }
    return IoRead8(base + SMBHSTDAT);
  }
  else {
    IoWrite8(base + SMBHSTSTS_NV, 0x1f);			// reset SMBus Controller
    IoWrite8(base + SMBHSTDAT_NV, 0xff);

    t1 = AsmReadTsc(); //rdtsc(l1, h1);
    while ( IoRead8(base + SMBHSTSTS_NV) & 0x01) {    // wait until read
      t2 = AsmReadTsc(); //rdtsc(l2, h2);
      t = DivU64x64Remainder((t2 - t1),
                             DivU64x32(gCPUStructure.TSCFrequency, 1000),
                             0);
      if (t > 5)
        return 0xFF;                  // break
    }

    IoWrite8(base + SMBHSTSTS_NV, 0x00); // clear status register
    IoWrite16(base + SMBHSTCMD_NV, cmd);
    IoWrite8(base + SMBHSTADD_NV, (adr << 1) | 0x01 );
    IoWrite8(base + SMBHPRTCL_NV, 0x07 );
    t1 = AsmReadTsc();

    while (!( IoRead8(base + SMBHSTSTS_NV) & 0x9F)) {		// wait till command finished
      t2 = AsmReadTsc();
      t = DivU64x64Remainder((t2 - t1),
                             DivU64x32(gCPUStructure.TSCFrequency, 1000),
                             0);
      if (t > 5)
        break; // break after 5ms
    }
    return IoRead8(base + SMBHSTDAT_NV);
  }
}

/* SPD i2c read optimization: prefetch only what we need, read non prefetcheable bytes on the fly */
#define READ_SPD(spd, base, slot, x) spd[x] = smb_read_byte(base, 0x50 + slot, x)


/** Read from spd *used* values only*/
VOID init_spd(UINT16* spd_indexes, UINT8* spd, UINT32 base, UINT8 slot)
{
  UINT16 i;
  for (i=0; spd_indexes[i]; i++) {
    READ_SPD(spd, base, slot, spd_indexes[i]);
  }

#if 0
  DBG("Reading entire spd data\n");
  for (i = 0; i < 512; i++) {
    UINT8 b = smb_read_byte(base, 0x50 + slot, i);
    DBG("%02X", b);
  }
  DBG(".\n");
#endif
}

// Get Vendor Name from spd, 3 cases handled DDR3, DDR4 and DDR2,
// have different formats, always return a valid ptr.
CONST CHAR8* getVendorName(RAM_SLOT_INFO* slot, UINT8 *spd, UINT32 base, UINT8 slot_num)
{
  UINT8 bank = 0;
  UINT8 code = 0;
  UINT8 parity;
  UINT8 testbit;
  //UINT8 * spd = (UINT8 *) slot->spd;
  if (spd[SPD_MEMORY_TYPE]==SPD_MEMORY_TYPE_SDRAM_DDR4) { // DDR4
    bank = spd[SPD_DDR4_MANUFACTURER_ID_BANK];
    code = spd[SPD_DDR4_MANUFACTURER_ID_CODE];
    parity = bank;
    testbit = bank;
    for (INTN i=6; i >= 0; i--) { parity ^= (testbit <<= 1); }
    if ( (parity & 0x80) == 0 ) {
      DBG("Bad parity bank=0x%2X code=0x%2X\n", bank, code);
    }
    bank &= 0x7f;
    for (UINTN i=0; i < VEN_MAP_SIZE; i++) {
      if (bank==vendorMap[i].bank && code==vendorMap[i].code) {
        return vendorMap[i].name;
      }
    }
    DBG("Unknown vendor bank=0x%2X code=0x%2X\n", bank, code);
  } else if (spd[SPD_MEMORY_TYPE]==SPD_MEMORY_TYPE_SDRAM_DDR3) { // DDR3
    bank = spd[SPD_DDR3_MEMORY_BANK]; // constructors like Patriot use b7=1
    code = spd[SPD_DDR3_MEMORY_CODE];
    parity = bank;
    testbit = bank;
    for (INTN i=6; i >= 0; i--) { parity ^= (testbit <<= 1); }
    if ( (parity & 0x80) == 0 ) {
      DBG("Bad parity bank=0x%2X code=0x%2X\n", bank, code);
    }
    bank &= 0x7f;

    for (UINTN i=0; i < VEN_MAP_SIZE; i++) {
      if (bank==vendorMap[i].bank && code==vendorMap[i].code) {
        return vendorMap[i].name;
      }
    }
  } else if (spd[SPD_MEMORY_TYPE]==SPD_MEMORY_TYPE_SDRAM_DDR2 ||
             spd[SPD_MEMORY_TYPE]==SPD_MEMORY_TYPE_SDRAM_DDR) {
    if(spd[64]==0x7f) {
      UINTN i;
      for (i=64; i<72 && spd[i]==0x7f;i++) {
        bank++;
        READ_SPD(spd, base, slot_num, (UINT8)(i+1)); // prefetch next spd byte to read for next loop
      }
      READ_SPD(spd, base, slot_num, (UINT8)i);
      code = spd[i];
    } else {
      code = spd[64];
      bank = 0;
    }
    for (UINTN i=0; i < VEN_MAP_SIZE; i++) {
      if (bank==vendorMap[i].bank && code==vendorMap[i].code) {
        return vendorMap[i].name;
      }
    }
  }
  /* OK there is no vendor id here lets try to match the partnum if it exists */
  if (AsciiStrStr(slot->PartNo,"GU332") == slot->PartNo) { // Unifosa fingerprint
    return "Unifosa";
  }
  return "NoName";
}

/** Get Default Memory Module Speed (no overclocking handled) */
UINT16 getDDRspeedMhz(UINT8 * spd)
{
  UINT16 frequency = 0; // default freq for unknown types //shit! DDR1 = 533
  UINT16 xmpFrequency1 = 0, xmpFrequency2 = 0;
  UINT8 xmpVersion = 0;
  UINT8 xmpProfiles = 0;

  if (spd[SPD_MEMORY_TYPE] == SPD_MEMORY_TYPE_SDRAM_DDR4) {
    UINT16 mincycle = spd[18];
    INT8 fineadjust = spd[125];
    frequency = (UINT16)(2000000 / (mincycle * 125 + fineadjust));

    // Check if module supports XMP
    if ((spd[SPD_XMP20_SIG1] == SPD_XMP_SIG1_VALUE) &&
        (spd[SPD_XMP20_SIG2] == SPD_XMP_SIG2_VALUE)) {
      xmpVersion = spd[SPD_XMP20_VERSION];
      xmpProfiles = spd[SPD_XMP20_PROFILES] & 3;

      if ((xmpProfiles & 1) == 1) {
        // Check the first profile
        mincycle = spd[SPD_XMP20_PROF1_MINCYCLE];
        fineadjust = spd[SPD_XMP20_PROF1_FINEADJUST];
        xmpFrequency1 = (UINT16)(2000000 / (mincycle * 125 + fineadjust));
        DBG("XMP Profile1: %d*125 %d ns\n", mincycle, fineadjust);
      }
      if ((xmpProfiles & 2) == 2) {
        // Check the second profile
        mincycle = spd[SPD_XMP20_PROF2_MINCYCLE];
        fineadjust = spd[SPD_XMP20_PROF2_FINEADJUST];
        xmpFrequency2 = (UINT16)(2000000 / (mincycle * 125 + fineadjust));
        DBG("XMP Profile2: %d*125 %d ns\n", mincycle, fineadjust);
      }
    }
  } else if (spd[SPD_MEMORY_TYPE] == SPD_MEMORY_TYPE_SDRAM_DDR3) {
    // This should be multiples of MTB converted to MHz- apianti
    UINT16 divisor = spd[10];
    UINT16 dividend = spd[11];
    UINT16 ratio = spd[12];
    frequency = (((dividend != 0) && (divisor != 0) && (ratio != 0)) ?
                 ((2000 * dividend) / (divisor * ratio)) : 0);

    // Check if module supports XMP
    if ((spd[SPD_XMP_SIG1] == SPD_XMP_SIG1_VALUE) &&
        (spd[SPD_XMP_SIG2] == SPD_XMP_SIG2_VALUE)) {
      xmpVersion = spd[SPD_XMP_VERSION];
      xmpProfiles = spd[SPD_XMP_PROFILES] & 3;

      if ((xmpProfiles & 1) == 1) {
        // Check the first profile
        divisor = spd[SPD_XMP_PROF1_DIVISOR];
        dividend = spd[SPD_XMP_PROF1_DIVIDEND];
        ratio = spd[SPD_XMP_PROF1_RATIO];
        xmpFrequency1 = (((dividend != 0) && (divisor != 0) && (ratio != 0)) ?
                         ((2000 * dividend) / (divisor * ratio)) : 0);
        DBG("XMP Profile1: %d*%d/%dns\n", ratio, divisor, dividend);
      }
      if ((xmpProfiles & 2) == 2) {
        // Check the second profile
        divisor = spd[SPD_XMP_PROF2_DIVISOR];
        dividend = spd[SPD_XMP_PROF2_DIVIDEND];
        ratio = spd[SPD_XMP_PROF2_RATIO];
        xmpFrequency2 = (((dividend != 0) && (divisor != 0) && (ratio != 0)) ?
                         ((2000 * dividend) / (divisor * ratio)) : 0);
        DBG("XMP Profile2: %d*%d/%dns\n", ratio, divisor, dividend);
      }
    }
  } else if ((spd[SPD_MEMORY_TYPE] == SPD_MEMORY_TYPE_SDRAM_DDR2) ||
             (spd[SPD_MEMORY_TYPE] == SPD_MEMORY_TYPE_SDRAM_DDR)) {
    switch(spd[9]) {
      case 0x50:
        return 400;
      case 0x3d:
        return 533;
      case 0x30:
        return 667;
      case 0x25:
      default:
        return 800;
      case 0x1E:
        return 1066;
    }
  }

  if (xmpProfiles) {
    DBG("Found module with XMP version %d.%d\n", (xmpVersion >> 4) & 0xF, xmpVersion & 0xF);

    switch (gSettings.XMPDetection) {
      case 0:
        // Detect the better XMP profile
        if (xmpFrequency1 >= xmpFrequency2) {
          if (xmpFrequency1 >= frequency) {
            DBG("Using XMP Profile1 instead of standard frequency %dMHz\n", frequency);
            frequency = xmpFrequency1;
          }
        } else if (xmpFrequency2 >= frequency) {
          DBG("Using XMP Profile2 instead of standard frequency %dMHz\n", frequency);
          frequency = xmpFrequency2;
        }
        break;

      case 1:
        // Use first profile if present
        if ((xmpProfiles & 1) == 1) {
          frequency = xmpFrequency1;
          DBG("Using XMP Profile1 instead of standard frequency %dMHz\n", frequency);
        } else {
          DBG("Not using XMP Profile1 because it is not present\n");
        }
        break;

      case 2:
        // Use second profile
        if ((xmpProfiles & 2) == 2) {
          frequency = xmpFrequency2;
          DBG("Using XMP Profile2 instead of standard frequency %dMHz\n", frequency);
        } else {
          DBG("Not using XMP Profile2 because it is not present\n");
        }
        break;

      default:
        break;
    }
  } else {
    // Print out XMP not detected
    switch (gSettings.XMPDetection) {
      case 0:
        DBG("Not using XMP because it is not present\n");
        break;

      case 1:
      case 2:
        DBG("Not using XMP Profile%d because it is not present\n", gSettings.XMPDetection);
        break;

      default:
        break;
    }
  }
  return frequency;
}

#define SMST(a) ((UINT8)((spd[a] & 0xf0) >> 4))
#define SLST(a) ((UINT8)(spd[a] & 0x0f))

/** Get DDR3 or DDR2 serial number, 0 most of the times, always return a valid ptr */
CHAR8* getDDRSerial(UINT8* spd)
{
  CHAR8* asciiSerial; //[16];
  asciiSerial = (__typeof__(asciiSerial))AllocatePool(17);
  if (spd[SPD_MEMORY_TYPE]==SPD_MEMORY_TYPE_SDRAM_DDR4) { // DDR4
    AsciiSPrint(asciiSerial, 17, "%2X%2X%2X%2X%2X%2X%2X%2X", SMST(325) /*& 0x7*/, SLST(325), SMST(326), SLST(326), SMST(327), SLST(327), SMST(328), SLST(328));
  } else if (spd[SPD_MEMORY_TYPE]==SPD_MEMORY_TYPE_SDRAM_DDR3) { // DDR3
    AsciiSPrint(asciiSerial, 17, "%2X%2X%2X%2X%2X%2X%2X%2X", SMST(122) /*& 0x7*/, SLST(122), SMST(123), SLST(123), SMST(124), SLST(124), SMST(125), SLST(125));
  } else if (spd[SPD_MEMORY_TYPE]==SPD_MEMORY_TYPE_SDRAM_DDR2 ||
             spd[SPD_MEMORY_TYPE]==SPD_MEMORY_TYPE_SDRAM_DDR) {  // DDR2 or DDR
    AsciiSPrint(asciiSerial, 17, "%2X%2X%2X%2X%2X%2X%2X%2X", SMST(95) /*& 0x7*/, SLST(95), SMST(96), SLST(96), SMST(97), SLST(97), SMST(98), SLST(98));
  } else {
    AsciiStrCpyS(asciiSerial, 17, "0000000000000000");
  }

  return asciiSerial;
}

/** Get DDR2 or DDR3 or DDR4 Part Number, always return a valid ptr */
CHAR8* getDDRPartNum(UINT8* spd, UINT32 base, UINT8 slot)
{
  UINT16 i, start=0, index = 0;
  CHAR8 c;
  CHAR8* asciiPartNo = (__typeof__(asciiPartNo))AllocatePool(32); //[32];

  if (spd[SPD_MEMORY_TYPE] == SPD_MEMORY_TYPE_SDRAM_DDR4) {
    start = 329;
  } else if (spd[SPD_MEMORY_TYPE] == SPD_MEMORY_TYPE_SDRAM_DDR3) {
    start = 128;
  } else if (spd[SPD_MEMORY_TYPE] == SPD_MEMORY_TYPE_SDRAM_DDR2 ||
             spd[SPD_MEMORY_TYPE] == SPD_MEMORY_TYPE_SDRAM_DDR) {
    start = 73;
  }

  // Check that the spd part name is zero terminated and that it is ascii:
  ZeroMem(asciiPartNo, 32);  //sizeof(asciiPartNo));
  for (i = start; i < start + 20; i++) {
    READ_SPD(spd, base, slot, (UINT16)i); // only read once the corresponding model part (ddr3 or ddr2)
    c = spd[i];
    if (IS_ALFA(c) || IS_DIGIT(c) || IS_PUNCT(c)) // It seems that System Profiler likes only letters and digits...
      asciiPartNo[index++] = c;
    else if (c < 0x20)
      break;
  }

  return asciiPartNo;
}

//INTN mapping []= {0,2,1,3,4,6,5,7,8,10,9,11};
#define PCI_COMMAND_OFFSET                          0x04

/** Read from smbus the SPD content and interpret it for detecting memory attributes */
STATIC VOID read_smb(EFI_PCI_IO_PROTOCOL *PciIo, UINT16	vid, UINT16	did)
{
  //	EFI_STATUS	Status;
  UINT16      speed;
  UINT8       i;// spd_size, spd_type;
  UINT32			base, mmio, hostc;
  UINT16			Command;
  //RAM_SLOT_INFO*  slot;
  //BOOLEAN			fullBanks;
  UINT8*			spdbuf;
//  UINT16			vid, did;

  UINT8                  TotalSlotsCount;

  smbPage = 0; // valid pages are 0 and 1; assume the first page (page 0) is already selected
//  vid = gPci->Hdr.VendorId;
//  did = gPci->Hdr.DeviceId;

  /*Status = */PciIo->Pci.Read (
                                PciIo,
                                EfiPciIoWidthUint16,
                                PCI_COMMAND_OFFSET,
                                1,
                                &Command
                                );

  Command |= 1;
  /*Status = */PciIo->Pci.Write (
                                 PciIo,
                                 EfiPciIoWidthUint16,
                                 PCI_COMMAND_OFFSET,
                                 1,
                                 &Command
                                 );

  DBG("SMBus CmdReg: 0x%X\n", Command);

  /*Status = */PciIo->Pci.Read (
                                PciIo,
                                EfiPciIoWidthUint32,
                                0x10,
                                1,
                                &mmio
                                );
  if (vid == 0x8086) {
    /*Status = */PciIo->Pci.Read (
                                  PciIo,
                                  EfiPciIoWidthUint32,
                                  0x20,
                                  1,
                                  &base
                                  );

    base &= 0xFFFE;
    smbIntel = TRUE;
  }
  else {
    /*Status = */PciIo->Pci.Read (
                                  PciIo,
                                  EfiPciIoWidthUint32,
                                  0x24, // iobase offset 0x24 on MCP
                                  1,
                                  &base
                                  );
    base &= 0xFFFC;
    smbIntel = FALSE;
  }
  /*Status = */PciIo->Pci.Read (
                                PciIo,
                                EfiPciIoWidthUint32,
                                0x40,
                                1,
                                &hostc
                                );


  MsgLog("Scanning SMBus [%04X:%04X], mmio: 0x%X, ioport: 0x%X, hostc: 0x%X\n",
         vid, did, mmio, base, hostc);

  // needed at least for laptops
  //fullBanks = (gDMI->MemoryModules == gDMI->CntMemorySlots);

  spdbuf = (__typeof__(spdbuf))AllocateZeroPool(MAX_SPD_SIZE);

  // Search MAX_RAM_SLOTS slots
  //==>
  /*  TotalSlotsCount = (UINT8) TotalCount;
   if (!TotalSlotsCount) {
   TotalSlotsCount = MAX_RAM_SLOTS;
   } */
  TotalSlotsCount = 8; //MAX_RAM_SLOTS;  -- spd can read only 8 slots
  DBG("Slots to scan [%d]...\n", TotalSlotsCount);
  for (i = 0; i <  TotalSlotsCount; i++){
    //<==
    ZeroMem(spdbuf, MAX_SPD_SIZE);
    READ_SPD(spdbuf, base, i, SPD_MEMORY_TYPE);
    if (spdbuf[SPD_MEMORY_TYPE] == 0xFF) {
      //DBG("SPD[%d]: Empty\n", i);
      continue;
    }
    else if (spdbuf[SPD_MEMORY_TYPE] == 0) {
      // First 0x40 bytes of DDR4 spd second page is 0. Maybe we need to change page, so do that and retry.
      DBG("SPD[%d]: Got invalid type %d @0x%X. Will set page and retry.\n", i, spdbuf[SPD_MEMORY_TYPE], 0x50 + i);
      smbPage = 0xFF; // force page to be set
      READ_SPD(spdbuf, base, i, SPD_MEMORY_TYPE);
    }
    
    // Copy spd data into buffer
    DBG("SPD[%d]: Type %d @0x%X\n", i, spdbuf[SPD_MEMORY_TYPE], 0x50 + i);
    switch (spdbuf[SPD_MEMORY_TYPE])  {
      case SPD_MEMORY_TYPE_SDRAM_DDR:
        init_spd(spd_indexes_ddr, spdbuf, base, i);

        gRAM.SPD[i].Type = MemoryTypeDdr;
        gRAM.SPD[i].ModuleSize = (((1 << ((spdbuf[SPD_NUM_ROWS] & 0x0f)
                                          + (spdbuf[SPD_NUM_COLUMNS] & 0x0f) - 17)) *
                                   ((spdbuf[SPD_NUM_DIMM_BANKS] & 0x7) + 1) *
                                   spdbuf[SPD_NUM_BANKS_PER_SDRAM])/3)*2;
        break;

      case SPD_MEMORY_TYPE_SDRAM_DDR2:
        init_spd(spd_indexes_ddr, spdbuf, base, i);

        gRAM.SPD[i].Type = MemoryTypeDdr2;
        gRAM.SPD[i].ModuleSize = ((1 << ((spdbuf[SPD_NUM_ROWS] & 0x0f)
                                         + (spdbuf[SPD_NUM_COLUMNS] & 0x0f) - 17)) *
                                  ((spdbuf[SPD_NUM_DIMM_BANKS] & 0x7) + 1) *
                                  spdbuf[SPD_NUM_BANKS_PER_SDRAM]);
        break;

      case SPD_MEMORY_TYPE_SDRAM_DDR3:
        init_spd(spd_indexes_ddr3, spdbuf, base, i);

        gRAM.SPD[i].Type = MemoryTypeDdr3;
        gRAM.SPD[i].ModuleSize = ((spdbuf[4] & 0x0f) + 28 ) + ((spdbuf[8] & 0x7)  + 3 );
        gRAM.SPD[i].ModuleSize -= (spdbuf[7] & 0x7) + 25;
        gRAM.SPD[i].ModuleSize = ((1 << gRAM.SPD[i].ModuleSize) * (((spdbuf[7] >> 3) & 0x1f) + 1));

        break;

      case SPD_MEMORY_TYPE_SDRAM_DDR4:
        init_spd(spd_indexes_ddr4, spdbuf, base, i);

        gRAM.SPD[i].Type = MemoryTypeDdr4;

        gRAM.SPD[i].ModuleSize
        = (1 << ((spdbuf[4] & 0x0f) + 8 /* Mb */ - 3 /* MB */)) // SDRAM Capacity
        * (1 << ((spdbuf[13] & 0x07) + 3)) // Primary Bus Width
        / (1 << ((spdbuf[12] & 0x07) + 2)) // SDRAM Width
        * (((spdbuf[12] >> 3) & 0x07) + 1) // Logical Ranks per DIMM
        * (((spdbuf[6] & 0x03) == 2) ? (((spdbuf[6] >> 4) & 0x07) + 1) : 1);

        /*
         Total = SDRAM Capacity / 8 * Primary Bus Width / SDRAM Width * Logical Ranks per DIMM
         where:
         : SDRAM Capacity = SPD byte 4 bits 3~0
         : Primary Bus Width = SPD byte 13 bits 2~0
         : SDRAM Width = SPD byte 12 bits 2~0
         : Logical Ranks per DIMM =
         for SDP, DDP, QDP: = SPD byte 12 bits 5~3
         for 3DS: = SPD byte 12 bits 5~3
         times SPD byte 6 bits 6~4 (Die Count)

         SDRAM Capacity

         0	0000 = 256 Mb
         1	0001 = 512 Mb
         2	0010 = 1 Gb
         3	0011 = 2 Gb
         4	0100 = 4 Gb
         5	0101 = 8 Gb
         6	0110 = 16 Gb
         7	0111 = 32 Gb


         Primary Bus Width

         000 = 8 bits
         001 = 16 bits
         010 = 32 bits
         011 = 64 bits


         SDRAM Device Width

         000 = 4 bits
         001 = 8 bits
         010 = 16 bits
         011 = 32 bits


         Logical Ranks per DIMM for SDP, DDP, QDP

         000 = 1 Package Rank
         001 = 2 Package Ranks
         010 = 3 Package Ranks
         011 = 4 Package Ranks


         Die Count for 3DS

         000 = Single die 001 = 2 die
         010 = 3 die
         011 = 4 die
         100 = 5 die
         101 = 6 die
         110 = 7 die
         111 = 8 die
         */
        break;

      default:
        gRAM.SPD[i].ModuleSize = 0;
        break;
    }

    if (gRAM.SPD[i].ModuleSize == 0) continue;
    //spd_type = (slot->spd[SPD_MEMORY_TYPE] < ((UINT8) 12) ? slot->spd[SPD_MEMORY_TYPE] : 0);
    //gRAM Type = spd_mem_to_smbios[spd_type];
    gRAM.SPD[i].PartNo = getDDRPartNum(spdbuf, base, i);
    gRAM.SPD[i].Vendor = getVendorName(&(gRAM.SPD[i]), spdbuf, base, i);
    gRAM.SPD[i].SerialNo = getDDRSerial(spdbuf);
    //XXX - when we can FreePool allocated for these buffers?
    // determine spd speed
    speed = getDDRspeedMhz(spdbuf);
    DBG("DDR speed %dMHz\n", speed);
    if (gRAM.SPD[i].Frequency<speed) gRAM.SPD[i].Frequency = speed;

#if 0
    // pci memory controller if available, is more reliable
    if (gRAM.Frequency > 0) {
      UINT32 freq = (UINT32)DivU64x32(gRAM.Frequency, 500000);
      // now round off special cases
      UINT32 fmod100 = freq %100;
      switch(fmod100) {
        case  1:	freq--;	break;
        case 32:	freq++;	break;
        case 65:	freq++; break;
        case 98:	freq+=2;break;
        case 99:	freq++; break;
      }
      gRAM.SPD[i].Frequency = freq;
      DBG("RAM speed %dMHz\n", freq);
    }
#endif

    MsgLog("Slot: %d Type %d %dMB %dMHz Vendor=%s PartNo=%s SerialNo=%s\n",
           i,
           (int)gRAM.SPD[i].Type,
           gRAM.SPD[i].ModuleSize,
           gRAM.SPD[i].Frequency,
           gRAM.SPD[i].Vendor,
           gRAM.SPD[i].PartNo,
           gRAM.SPD[i].SerialNo);

    gRAM.SPD[i].InUse = TRUE;
    ++(gRAM.SPDInUse);
    //}

    // laptops sometimes show slot 0 and 2 with slot 1 empty when only 2 slots are presents so:
    //gDMI->DIMM[i]= (UINT32)((i>0 && gRAM->DIMM[1].InUse==FALSE && !fullBanks && TotalCount == 2)?mapping[i] : i); // for laptops case, mapping setup would need to be more generic than this

    //slot->spd = NULL;

  } // for
  if (smbPage != 0) {
    READ_SPD(spdbuf, base, 0, 0); // force first page when we're done
  }
}

VOID ScanSPD()
{
  EFI_STATUS            Status;
  EFI_HANDLE            *HandleBuffer = NULL;
//  EFI_GUID              **ProtocolGuidArray;
  EFI_PCI_IO_PROTOCOL   *PciIo = NULL;
  UINTN                 HandleCount;
//  UINTN                 ArrayCount;
  UINTN                 Index;
//  UINTN                 ProtocolIndex;
  PCI_TYPE00            gPci;

  DbgHeader("ScanSPD");
  
  // Scan PCI handles
  Status = gBS->LocateHandleBuffer (
                                    ByProtocol,
                                    &gEfiPciIoProtocolGuid,
                                    NULL,
                                    &HandleCount,
                                    &HandleBuffer
                                    );
  
  if (!EFI_ERROR (Status)) {
    for (Index = 0; Index < HandleCount; ++Index) {
      Status = gBS->HandleProtocol(HandleBuffer[Index], &gEfiPciIoProtocolGuid, (VOID **)&PciIo);
      if (!EFI_ERROR (Status)) {
        // Read PCI BUS
        //PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
        Status = PciIo->Pci.Read (
                                  PciIo,
                                  EfiPciIoWidthUint32,
                                  0,
                                  sizeof (gPci) / sizeof (UINT32),
                                  &gPci
                                  );
        //SmBus controller has class = 0x0c0500
        if ((gPci.Hdr.ClassCode[2] == 0x0c) && (gPci.Hdr.ClassCode[1] == 5)
            && (gPci.Hdr.ClassCode[0] == 0) && (gPci.Hdr.VendorId == 0x8086 || gPci.Hdr.VendorId == 0x10DE)) {
          DBG ("SMBus device : %04X %04X class=%02X%02X%02X status=%s\n",
               gPci.Hdr.VendorId,
               gPci.Hdr.DeviceId,
               gPci.Hdr.ClassCode[2],
               gPci.Hdr.ClassCode[1],
               gPci.Hdr.ClassCode[0],
               strerror(Status)
               );
          read_smb(PciIo, gPci.Hdr.VendorId, gPci.Hdr.DeviceId);
        }
      }
    }
  }


  // Scan PCI BUS For SmBus controller 
/*  Status = gBS->LocateHandleBuffer(AllHandles,NULL,NULL,&HandleCount,&HandleBuffer);
  if (!EFI_ERROR(Status)) {
    for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
      Status = gBS->ProtocolsPerHandle(HandleBuffer[HandleIndex],&ProtocolGuidArray,&ArrayCount);
      if (!EFI_ERROR(Status)) {
        for (ProtocolIndex = 0; ProtocolIndex < ArrayCount; ProtocolIndex++) {
          if (CompareGuid(&gEfiPciIoProtocolGuid, ProtocolGuidArray[ProtocolIndex])) {
            Status = gBS->OpenProtocol(HandleBuffer[HandleIndex],&gEfiPciIoProtocolGuid,(VOID **)&PciIo,gImageHandle,NULL,EFI_OPEN_PROTOCOL_GET_PROTOCOL);
            
            if (!EFI_ERROR(Status)) {
              // Read PCI BUS 
              Status = PciIo->Pci.Read (
                                        PciIo,
                                        EfiPciIoWidthUint32,
                                        0,
                                        sizeof (gPci) / sizeof (UINT32),
                                        &gPci
                                        );
              
              //SmBus controller has class = 0x0c0500
              if ((gPci.Hdr.ClassCode[2] == 0x0c) && (gPci.Hdr.ClassCode[1] == 5) 
                  && (gPci.Hdr.ClassCode[0] == 0) && (gPci.Hdr.VendorId == 0x8086 || gPci.Hdr.VendorId == 0x10DE)) {
                read_smb(PciIo);
              }
            }
          }
        }
      }
    }
  }
 */
}
