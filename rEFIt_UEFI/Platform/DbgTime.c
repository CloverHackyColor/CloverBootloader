/*
 *  DbgTime.c
 *  
 *  Created by dmazar on 19.06.12.
 *  
 *  Simple functions for profiling code execution.
 *
 * Usage:
 * 
 *   #define DBGT(...) { DBG("[%a] ", DbgTime()); DBG(__VA_ARGS__) }
 *   ...
 *   // init debug time - must be after PrepatchSmbios()
 *   DbgTimeInit();
 *   ...
 *   DBGT("Some debug text\n");
 *   ...
 *   DBGT("Some debug text %d, %s\n", Int, String);
 */


#include "Platform.h"


//
// Globals
//

// Start debug ticks.
UINT64     DbgTStartTsc = 0;

// Last debug ticks.
UINT64     DbgTLastTsc = 0;

// Last debug ticks.
UINT64     DbgTFreqDivMs = 0;

// Buffer for debug time.
CHAR8    DbgTTxt[32];


//
// Functions
//

// Inits debug time. Must be called after PrepatchSmbios().
VOID DbgTimeInit(VOID)
{
	DbgTStartTsc = AsmReadTsc();
	DbgTLastTsc = DbgTStartTsc;
	// note: depending on a CurrentSpeed obtained from PrepatchSmbios()
	DbgTFreqDivMs = MultU64x32(gCPUStructure.CurrentSpeed, kilo);
}

// Returns debug time as string for print:
//   Start      - Last call
//   secs:milis - sec:milis
// Returned buffer should not be released.
CHAR8* DbgTime(VOID)
{
	UINT64    dTStart;
	UINT64    dTLast;
	UINT64    CurrentTsc;
   UINT64    start, start2, last, last2;
	
	DbgTTxt[0] = '\0';
	
	if (DbgTFreqDivMs) {
		CurrentTsc = AsmReadTsc();
		dTStart = DivU64x64Remainder((CurrentTsc - DbgTStartTsc), DbgTFreqDivMs, 0);
		dTLast = DivU64x64Remainder((CurrentTsc - DbgTLastTsc), DbgTFreqDivMs, 0);
      start = DivU64x64Remainder(dTStart, 1000, &start2);
      last = DivU64x64Remainder(dTLast, 1000, &last2);
		AsciiSPrint(DbgTTxt, sizeof(DbgTTxt),
					"%ld:%03ld - %ld:%03ld", start, start2, last, last2);
		DbgTLastTsc = CurrentTsc;
	}
	
	return DbgTTxt;
}

