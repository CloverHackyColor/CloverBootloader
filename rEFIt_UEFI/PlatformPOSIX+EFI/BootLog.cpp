/*
 *  BootLog.c
 *  
 *
 *  Created by Slice on 19.08.11.
 *  Edited by apianti 2012-09-08
 *  Initial idea from Kabyl
 */


#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include <Efi.h>

/** Prints Number of bytes in a row (hex and ascii). Row size is MaxNumber. */
void
PrintBytesRow(IN UINT8 *Bytes, IN UINTN Number, IN UINTN MaxNumber)
{
	UINTN	Index;
	
	// print hex vals
	for (Index = 0; Index < Number; Index++) {
		DebugLog(1, "%02hhX ", Bytes[Index]);
	}
	
	// pad to MaxNumber if needed
	for (; Index < MaxNumber; Index++) {
		DebugLog(1, "   ");
	}
	
	DebugLog(1, "| ");
	
	// print ASCII
	for (Index = 0; Index < Number; Index++) {
		if (Bytes[Index] >= 0x20 && Bytes[Index] <= 0x7e) {
			DebugLog(1, "%c", (CHAR16)Bytes[Index]);
		} else {
			DebugLog(1, "%c", L'.');
		}
	}
	
	DebugLog(1, "\n");
}

/** Prints series of bytes. */
void
PrintBytes(IN void *Bytes, IN UINTN Number)
{
	UINTN	Index;
	
	for (Index = 0; Index < Number; Index += 16) {
		PrintBytesRow((UINT8*)Bytes + Index, ((Index + 16 < Number) ? 16 : (Number - Index)), 16);
	}
}

bool gEnableCloverLog = false;

// Changed MsgLog(...) it now calls this function
//  with DebugMode == 0. - apianti
// DebugMode==0 Prints to msg log, only output to log on SaveBooterLog
// DebugMode==1 Prints to msg log and DEBUG_LOG
// DebugMode==2 Prints to msg log, DEBUG_LOG and display console
void EFIAPI DebugLog(IN INTN DebugMode, IN CONST CHAR8 *FormatString, ...)
{
  if ( !gEnableCloverLog ) return;
  
  VA_LIST Marker;
  // Make sure the buffer is intact for writing
  if (FormatString == NULL || DebugMode < 0) {
    return;
  }

  // Print message to log buffer
  VA_START(Marker, FormatString);
  #if __WCHAR_MAX__ < 0xffff
  #else
    vprintf(FormatString, Marker);
  #endif
  VA_END(Marker);
}



void DbgHeader(CONST CHAR8 *str)
{
  CHAR8 strLog[50];
  size_t len;
  int end = snprintf(strLog, 50, "=== [ %s ] ", str);
  if ( end < 0 ) return;
  len = 50 - (unsigned int)end;

  memset(&strLog[end], '=', len);
  strLog[49] = '\0';
  DebugLog (1, "%s\n", strLog);
}


void closeDebugLog()
{
}

EFI_STATUS
SaveBooterLog (
  const EFI_FILE* BaseDir  OPTIONAL,
  const CHAR16 *FileName
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
SetupBooterLog (
  BOOLEAN AllowGrownSize
  )
{
  return EFI_SUCCESS;
}


void
InitBooterLog (void)
{
}


void EFIAPI DebugLogForOC(IN INTN DebugLevel, IN CONST CHAR8 *FormatString, ...)
{
  VA_LIST Marker;
  //UINTN offset = 0;
  
  // Make sure the buffer is intact for writing
  if (FormatString == NULL ) {
    return;
  }

  // Print message to log buffer
  VA_START(Marker, FormatString);
  printf(FormatString, Marker);
  VA_END(Marker);
}

