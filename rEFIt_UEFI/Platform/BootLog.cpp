/*
 *  BootLog.c
 *  
 *
 *  Created by Slice on 19.08.11.
 *  Edited by apianti 2012-09-08
 *  Initial idea from Kabyl
 */


#include "Platform.h"
//#include <Library/MemLogLib.h>
#include "DataHubCpu.h"

extern  EFI_GUID  gEfiMiscSubClassGuid;


/** Prints Number of bytes in a row (hex and ascii). Row size is MaxNumber. */
VOID
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
VOID
PrintBytes(IN VOID *Bytes, IN UINTN Number)
{
	UINTN	Index;
	
	for (Index = 0; Index < Number; Index += 16) {
		PrintBytesRow((UINT8*)Bytes + Index, ((Index + 16 < Number) ? 16 : (Number - Index)), 16);
	}
}



EFI_FILE_PROTOCOL* GetDebugLogFile()
{
  EFI_STATUS          Status;
  EFI_LOADED_IMAGE    *LoadedImage;
  EFI_FILE_PROTOCOL   *RootDir;
  EFI_FILE_PROTOCOL   *LogFile;
  
  // get RootDir from device we are loaded from
  Status = gBS->HandleProtocol(gImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &LoadedImage);
  if (EFI_ERROR(Status)) {
    return NULL;
  }
  RootDir = EfiLibOpenRoot(LoadedImage->DeviceHandle);
  if (RootDir == NULL) {
    return NULL;
  }
  
  // Open log file from current root
  Status = RootDir->Open(RootDir, &LogFile, DEBUG_LOG,
                         EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);

  // If the log file is not found try to create it
  if (Status == EFI_NOT_FOUND) {
    Status = RootDir->Open(RootDir, &LogFile, DEBUG_LOG,
                           EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
  }
  RootDir->Close(RootDir);
  RootDir = NULL;
  
  if (EFI_ERROR(Status)) {
    // try on first EFI partition
    Status = egFindESP(&RootDir);
    if (!EFI_ERROR(Status)) {
      Status = RootDir->Open(RootDir, &LogFile, DEBUG_LOG,
                             EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
      // If the log file is not found try to create it
      if (Status == EFI_NOT_FOUND) {
        Status = RootDir->Open(RootDir, &LogFile, DEBUG_LOG,
                               EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
      }
      RootDir->Close(RootDir);
      RootDir = NULL;
    }
  }
  
  if (EFI_ERROR(Status)) {
    LogFile = NULL;
  }
  
  return LogFile;
}


VOID SaveMessageToDebugLogFile(IN CHAR8 *LastMessage)
{
  STATIC BOOLEAN          FirstTimeSave = TRUE;
//  STATIC UINTN            Position = 0;
  CHAR8                   *MemLogBuffer;
  UINTN                   MemLogLen;
  CHAR8                   *Text;
  UINTN                   TextLen;
  EFI_FILE_HANDLE         LogFile;
  
  MemLogBuffer = GetMemLogBuffer();
  MemLogLen = GetMemLogLen();
  Text = LastMessage;
  TextLen = AsciiStrLen(LastMessage);

  LogFile = GetDebugLogFile();
  
  // Write to the log file
  if (LogFile != NULL) {
    // Advance to the EOF so we append
    EFI_FILE_INFO *Info = EfiLibFileInfo(LogFile);
    if (Info) {
      LogFile->SetPosition(LogFile, Info->FileSize);
      // If we haven't had root before this write out whole log
      if (FirstTimeSave) {
        Text = MemLogBuffer;
        TextLen = MemLogLen;
        FirstTimeSave = FALSE;
      }
      // Write out this message
      LogFile->Write(LogFile, &TextLen, Text);
    }
    LogFile->Close(LogFile);
  }
}

VOID EFIAPI MemLogCallback(IN INTN DebugMode, IN CHAR8 *LastMessage)
{
  // Print message to console
  if (DebugMode >= 2) {
    printf("%s", LastMessage);
  }
  
  if ((DebugMode >= 1) && GlobalConfig.DebugLog) {
    SaveMessageToDebugLogFile(LastMessage);
  }
}

// Changed MsgLog(...) it now calls this function
//  with DebugMode == 0. - apianti
// DebugMode==0 Prints to msg log, only output to log on SaveBooterLog
// DebugMode==1 Prints to msg log and DEBUG_LOG
// DebugMode==2 Prints to msg log, DEBUG_LOG and display console
VOID EFIAPI DebugLog(IN INTN DebugMode, IN CONST CHAR8 *FormatString, ...)
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

VOID InitBooterLog(VOID)
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
EFI_STATUS SaveBooterLog(IN EFI_FILE_HANDLE BaseDir OPTIONAL, IN CONST CHAR16 *FileName)
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

