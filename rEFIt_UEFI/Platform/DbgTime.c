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

// TSC ticks per second.
UINT64     DbgTscFreqSec = 0;

// Buffer for debug time.
CHAR8    DbgTTxt[32];


//
// Functions
//

// Inits debug time. Must be called after PrepatchSmbios().
VOID DbgTimeInit(UINT64 TscTicksPerSecond, UINTN StartTsc)
{
	DbgTStartTsc = StartTsc;
	DbgTLastTsc = StartTsc;
	DbgTscFreqSec = TscTicksPerSecond;
}

// Returns debug time as string for print:
//   Start      - Last call
//   secs:milis - sec:milis
// Returned buffer should not be released.
CHAR8* DbgTime(VOID)
{
	UINT64    dTStartSec;
	UINT64    dTStartMs;
	UINT64    dTLastSec;
	UINT64    dTLastMs;
	UINT64    CurrentTsc;
	
	DbgTTxt[0] = '\0';
	
	if (DbgTscFreqSec) {
		CurrentTsc = AsmReadTsc();
        
		dTStartMs = DivU64x64Remainder(MultU64x32(CurrentTsc - DbgTStartTsc, 1000), DbgTscFreqSec, NULL);
		dTStartSec = DivU64x64Remainder(dTStartMs, 1000, &dTStartMs);
        
		dTLastMs = DivU64x64Remainder(MultU64x32(CurrentTsc - DbgTLastTsc, 1000), DbgTscFreqSec, NULL);
		dTLastSec = DivU64x64Remainder(dTLastMs, 1000, &dTLastMs);
        
		AsciiSPrint(DbgTTxt, sizeof(DbgTTxt),
					"%ld:%03ld - %ld:%03ld", dTStartSec, dTStartMs, dTLastSec, dTLastMs);
		DbgTLastTsc = CurrentTsc;
	}
	
	return DbgTTxt;
}

