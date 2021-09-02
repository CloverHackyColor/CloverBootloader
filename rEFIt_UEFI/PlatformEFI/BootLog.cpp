/*
 *  BootLog.c
 *  
 *
 *  Created by Slice on 19.08.11.
 *  Edited by apianti 2012-09-08
 *  Initial idea from Kabyl
 */


#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
//#include <Library/MemLogLib.h>
#include "../Platform/DataHubCpu.h"
#include "../Platform/Settings.h"
#include "../Settings/Self.h"
#include "../Platform/guid.h"

#ifndef DEBUG_ALL
#define DEBUG_BOOTLOG 0
#else
#define DEBUG_BOOTLOG DEBUG_ALL
#endif

#if DEBUG_BOOTLOG == 0
#define DBG(...)
#else
#define DBG(...) DebugLog (DEBUG_BOOTLOG, __VA_ARGS__)
#endif


void EFIAPI MemLogCallback(IN INTN DebugMode, IN CHAR8 *LastMessage);


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

static XStringW debugLogFileName;
static EFI_FILE_PROTOCOL* gLogFile = NULL;
// Do not keep a pointer to MemLogBuffer. Because a reallocation, it could become invalid.

int g_OpeningLogFile = 0;


// Avoid debug looping. To be able to call DBG from inside function that DBG calls, we need to suspend callback to avoid a loop.
// Just instanciante this, the destructor will restore the callback.
class SuspendMemLogCallback
{
  MEM_LOG_CALLBACK memlogCallBack_saved = NULL;
public:
  SuspendMemLogCallback() {
    memlogCallBack_saved = GetMemLogCallback();
    SetMemLogCallback(NULL);
  };
  ~SuspendMemLogCallback() { SetMemLogCallback(memlogCallBack_saved); };
};

#if DEBUG_BOOTLOG == 0
#define DGB_nbCallback(...)
#else
#define DGB_nbCallback(...) do { SuspendMemLogCallback smc; DBG(__VA_ARGS__); } while (0)
#endif

void closeDebugLog()
{
//  EFI_STATUS          Status;

  if ( !gLogFile ) return;

  SuspendMemLogCallback smc;

  /*Status =*/ gLogFile->Close(gLogFile);
  gLogFile = NULL;
  //DGB_nbCallback("closeDebugLog() -> %s\n", efiStrError(Status));
}

/*
 * Use (or not) self.getCloverDir() to open log file.
 * Not using self has the advantage of being able to generate a log even after uninitreflib().
 * The only thing needed is gImageHandle. But because it's a parameter to main entry point, value can't be wrong.
 * Drawback is that code to find current working directory has to be duplicated.
 */
//#define USE_SELF_INSTANCE
static UINTN GetDebugLogFile()
{
  EFI_STATUS          Status;
  EFI_FILE_PROTOCOL   *LogFile;

  if ( gLogFile ) return 0;

  #ifdef USE_SELF_INSTANCE
    if ( !self.isInitialized() ) return 0;
  #endif

  g_OpeningLogFile = 1;

  EFI_TIME          Now;
  Status = gRT->GetTime(&Now, NULL);
  if ( EFI_ERROR(Status) ) {
    DBG("GetTime return %s\n", efiStrError(Status));
  }

  #ifdef USE_SELF_INSTANCE
    const EFI_FILE_PROTOCOL& CloverDir = self.getCloverDir();
    const XString& efiFileName = self.getCloverEfiFileName();
  #else
    XStringW efiFileName;
    const EFI_FILE_PROTOCOL* CloverDirPtr = Self::getCloverDirAndEfiFileName(gImageHandle, &efiFileName);
    if ( CloverDirPtr == NULL ) return 0;
    const EFI_FILE_PROTOCOL& CloverDir = *CloverDirPtr;
  #endif

  if ( debugLogFileName.isEmpty() )
  {
    debugLogFileName = S8Printf("misc\\%04d-%02d-%02d_%02d-%02d_%ls.log", Now.Year, Now.Month, Now.Day, Now.Hour, Now.Minute,  efiFileName.wc_str());
    Status = CloverDir.Open(&CloverDir, &LogFile, debugLogFileName.wc_str(), EFI_FILE_MODE_READ, 0);
    if ( !EFI_ERROR(Status) ) LogFile->Close(LogFile); // DO NOT modify Status here.
    INTN i=1;
    while ( Status != EFI_NOT_FOUND  &&  (i < MAX_INTN) ) {
      debugLogFileName = S8Printf("misc\\%04d-%02d-%02d_%02d-%02d_%ls(%lld).log", Now.Year, Now.Month, Now.Day, Now.Hour, Now.Minute,  efiFileName.wc_str(), i);
      Status = CloverDir.Open(&CloverDir, &LogFile, debugLogFileName.wc_str(), EFI_FILE_MODE_READ, 0);
      if ( !EFI_ERROR(Status) ) LogFile->Close(LogFile); // DO NOT modify Status here.
    }
    if ( Status != EFI_NOT_FOUND ) {
      DBG("Cannot find a free debug log file name\n"); // I can't imagine that to happen...
      debugLogFileName.setEmpty(); // To allow to retry at the next call
      g_OpeningLogFile = 0;
      return 0;
    }
    Status = CloverDir.Open(&CloverDir, &LogFile, debugLogFileName.wc_str(), EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
    gLogFile = LogFile;
    g_OpeningLogFile = 0;
    return 0;
  }else{
    Status = CloverDir.Open(&CloverDir, &LogFile, debugLogFileName.wc_str(), EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
    g_OpeningLogFile = 0;

////   Jief : Instead of EfiLibFileInfo, let's use SetPosition to get the size.
//    if (!EFI_ERROR(Status)) {
//      EFI_FILE_INFO *Info = EfiLibFileInfo(LogFile);
//      if (Info) {
//        Status = LogFile->SetPosition(LogFile, Info->FileSize);
//        if ( EFI_ERROR(Status) ) {
//          DBG("SaveMessageToDebugLogFile SetPosition error %s\n", efiStrError(Status));
//        }
//      }
//    }

    if (!EFI_ERROR(Status)) {
      Status = LogFile->SetPosition(LogFile, 0xFFFFFFFFFFFFFFFFULL);
      if ( EFI_ERROR (Status) ) {
        DGB_nbCallback("GetDebugLogFile() -> Cannot set log position to 0xFFFFFFFFFFFFFFFFULL : %s\n", efiStrError(Status));
        LogFile->Close(LogFile);
      }else{
        UINTN size;
        Status = LogFile->GetPosition(LogFile, &size);
        if ( EFI_ERROR (Status) ) {
          DGB_nbCallback("GetDebugLogFile() -> Cannot get log position : %s\n", efiStrError(Status));
          LogFile->Close(LogFile);
        }else{
          //DGB_nbCallback("GetDebugLogFile() -> opened. log position = %lld (lwo %lld)\n", size, lastWrittenOffset);
          gLogFile = LogFile;
          return size;
        }
      }
    }
    return 0;
  }
}

VOID SaveMessageToDebugLogFile(IN CHAR8 *LastMessage)
{
  EFI_STATUS Status;

  UINTN lastWrittenOffset = GetDebugLogFile();

  if ( gLogFile == NULL ) return;

  // Write out this message
  const char* lastWrittenPointer = GetMemLogBuffer() + lastWrittenOffset;
  UINTN TextLen = strlen(lastWrittenPointer);
  UINTN TextLen2 = TextLen;

  Status = gLogFile->Write(gLogFile, &TextLen2, lastWrittenPointer);
  lastWrittenOffset += TextLen2;
  if ( EFI_ERROR(Status) ) {
    DGB_nbCallback("SaveMessageToDebugLogFile write error %s\n", efiStrError(Status));
    closeDebugLog();
  }else{
    if ( TextLen2 != TextLen ) {
      DGB_nbCallback("SaveMessageToDebugLogFile TextLen2(%lld) != TextLen(%lld)\n", TextLen2, TextLen);
      closeDebugLog();
    }else{
      // Not all Firmware implements Flush. So we have to close everytime to force flush. Let's Close() instead of Flush()
      // Is there a performance difference ? Is it worth to create a setting ? Probably not...
//          Status = LogFile->Flush(LogFile);
//          if ( EFI_ERROR(Status) ) {
//            DGB_nbCallback("SaveMessageToDebugLogFile Cannot flush error %s\n", efiStrError(Status));
//            closeDebugLog();
//          }
    }
  }
  // Not all Firmware implements Flush. So we have to close every time to force flush.
  closeDebugLog();
}

void EFIAPI MemLogCallback(IN INTN DebugMode, IN CHAR8 *LastMessage)
{
  // Print message to console
  if (DebugMode >= 2) {
    printf("%s", LastMessage);
  }
  
  if ((DebugMode >= 1) && gSettings.Boot.DebugLog) {
    SuspendMemLogCallback smc;
    SaveMessageToDebugLogFile(LastMessage);
  }
}

// Changed MsgLog(...) it now calls this function
//  with DebugMode == 0. - apianti
// DebugMode==0 Prints to msg log, only output to log on SaveBooterLog
// DebugMode==1 Prints to msg log and DEBUG_LOG
// DebugMode==2 Prints to msg log, DEBUG_LOG and display console
void EFIAPI DebugLog(IN INTN DebugMode, IN CONST CHAR8 *FormatString, ...)
{
   VA_LIST Marker;
   //UINTN offset = 0;
   
   // Make sure the buffer is intact for writing
   if (FormatString == NULL || DebugMode < 0) {
     return;
   }

   // Print message to log buffer
   VA_START(Marker, FormatString);
   MemLogfVA(TRUE, DebugMode, FormatString, Marker);
   VA_END(Marker);
}

void InitBooterLog(void)
{
  SetMemLogCallback(MemLogCallback);
}


EFI_STATUS SetupBooterLog(BOOLEAN AllowGrownSize)
{
  EFI_STATUS              Status = EFI_SUCCESS;
  CHAR8                   *MemLogBuffer;
  UINTN                   MemLogLen;
  
  MemLogBuffer = GetMemLogBuffer();
  MemLogLen = GetMemLogLen();
  
  if (MemLogBuffer == NULL || MemLogLen == 0) {
		return EFI_NOT_FOUND;
  }
  
  if (MemLogLen > MEM_LOG_INITIAL_SIZE && !AllowGrownSize) {
    CHAR8 PrevChar = MemLogBuffer[MEM_LOG_INITIAL_SIZE-1];
    MemLogBuffer[MEM_LOG_INITIAL_SIZE-1] = '\0';
    Status = LogDataHub(&gEfiMiscSubClassGuid, L"boot-log", MemLogBuffer, MEM_LOG_INITIAL_SIZE);
    MemLogBuffer[MEM_LOG_INITIAL_SIZE-1] = PrevChar;
  } else {
    Status = LogDataHub(&gEfiMiscSubClassGuid, L"boot-log", MemLogBuffer, (UINT32)MemLogLen);
  }
  
	return Status;
}

// Made msgbuf and msgCursor private to this source
// so we need a different way of saving the msg log - apianti
EFI_STATUS SaveBooterLog(const EFI_FILE* BaseDir OPTIONAL, IN CONST CHAR16 *FileName)
{
  CHAR8                   *MemLogBuffer;
  UINTN                   MemLogLen;
  
  MemLogBuffer = GetMemLogBuffer();
  MemLogLen = GetMemLogLen();
  
  if (MemLogBuffer == NULL || MemLogLen == 0) {
		return EFI_NOT_FOUND;
  }
  
  return egSaveFile(BaseDir, FileName, (UINT8*)MemLogBuffer, MemLogLen);
}

void DbgHeader(CONST CHAR8 *str)
{
  CHAR8 strLog[50];
  INTN len;
  UINTN end = snprintf(strLog, 50, "=== [ %s ] ", str);
  len = 50 - end;

  SetMem(&strLog[end], len , '=');
  strLog[49] = '\0';
  DebugLog (1, "%s\n", strLog);
}



/*
 * Redirection of OpenCore log to Clover Log.
 */

/*
 * This function is called from OpenCore when there is a DEBUG ((expression))
 * Mapping from DEBUG to DebugLogForOC is made in OpenCoreFromClover.h
 */
void EFIAPI DebugLogForOC(IN INTN DebugLevel, IN CONST CHAR8 *FormatString, ...)
{
   VA_LIST Marker;

   if (FormatString == NULL ) return;

   // Print message to log buffer
   VA_START(Marker, FormatString);
   MemLogVA(TRUE, 1, FormatString, Marker);
   VA_END(Marker);
}
