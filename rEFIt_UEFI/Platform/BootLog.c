/*
 *  BootLog.c
 *  
 *
 *  Created by Slice on 19.08.11.
 *  Edited by apianti 2012-09-08
 *  Initial idea from Kabyl
 */


#include "Platform.h"
#include <Library/MemLogLib.h>


extern  EFI_GUID  gEfiMiscSubClassGuid;


EFI_FILE_PROTOCOL* GetSystemLogFile(BOOLEAN FirstTimeSave)
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
  Status = RootDir->Open(RootDir, &LogFile, SYSTEM_LOG,
                         EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
  /*
  if (FirstTimeSave && Status == EFI_SUCCESS) {
    LogFile->Delete(LogFile);
    Status = EFI_NOT_FOUND;
  }
   */
  // If the log file is not found try to create it
  if (Status == EFI_NOT_FOUND) {
    Status = RootDir->Open(RootDir, &LogFile, SYSTEM_LOG,
                           EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
  }
  RootDir->Close(RootDir);
  RootDir = NULL;
  
  if (EFI_ERROR(Status)) {
    // try on first EFI partition
    Status = egFindESP(&RootDir);
    if (!EFI_ERROR(Status)) {
      Status = RootDir->Open(RootDir, &LogFile, SYSTEM_LOG,
                             EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
      /*
      if (FirstTimeSave && Status == EFI_SUCCESS) {
        LogFile->Delete(LogFile);
        Status = EFI_NOT_FOUND;
      }
       */
      // If the log file is not found try to create it
      if (Status == EFI_NOT_FOUND) {
        Status = RootDir->Open(RootDir, &LogFile, SYSTEM_LOG,
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


VOID SaveMessageToSystemLogFile(IN CHAR8 *LastMessage)
{
  static BOOLEAN          FirstTimeSave = TRUE;
  CHAR8                   *MemLogBuffer;
  UINTN                   MemLogLen;
  CHAR8                   *Text;
  UINTN                   TextLen;
  EFI_FILE_HANDLE         LogFile;
  
  MemLogBuffer = GetMemLogBuffer();
  MemLogLen = GetMemLogLen();
  Text = LastMessage;
  TextLen = AsciiStrLen(LastMessage);

  
  LogFile = GetSystemLogFile(FirstTimeSave);
  // Write to the log file
  if (LogFile != NULL)
  {
    // Advance to the EOF so we append
    EFI_FILE_INFO *Info = EfiLibFileInfo(LogFile);
    if (Info)
    {
      LogFile->SetPosition(LogFile, Info->FileSize);
      // If we haven't had root before this write out whole log
      if (FirstTimeSave)
      {
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
    AsciiPrint(LastMessage);
  }
  
  if ((DebugMode >= 1) && GlobalConfig.SystemLog) {
    SaveMessageToSystemLogFile(LastMessage);
  }
}

// Changed MsgLog(...) it now calls this function
//  with DebugMode == 0. - apianti
// DebugMode==0 Prints to msg log, only output to log on SaveBooterLog
// DebugMode==1 Prints to msg log and SYSTEM_LOG
// DebugMode==2 Prints to msg log, SYSTEM_LOG and display console
VOID DebugLog(IN INTN DebugMode, IN CONST CHAR8 *FormatString, ...)
{
   VA_LIST Marker;
   //UINTN offset = 0;
   
   // Make sure the buffer is intact for writing
   if (FormatString == NULL || DebugMode < 0)
   {
     return;
   }

   // Print message to log buffer
   VA_START(Marker, FormatString);
   MemLogVA(DebugMode, FormatString, Marker);
   VA_END(Marker);
}

VOID InitBooterLog(VOID)
{
  SetMemLogCallback(MemLogCallback);
}

EFI_STATUS SetupBooterLog(VOID)
{
	EFI_STATUS              Status	= EFI_SUCCESS;
  CHAR8                   *MemLogBuffer;
  UINTN                   MemLogLen;
  
  MemLogBuffer = GetMemLogBuffer();
  MemLogLen = GetMemLogLen();
  
  if (MemLogBuffer == NULL || MemLogLen == 0) {
		return EFI_NOT_FOUND;
  }
  
	Status =  LogDataHub(&gEfiMiscSubClassGuid, L"boot-log", MemLogBuffer, (UINT32)MemLogLen);
  
  // Save BOOT_LOG only once on successful boot
  /*
  if (GlobalConfig.SystemLog){
    Status = SaveBooterLog(SelfRootDir, BOOT_LOG);
    if (EFI_ERROR(Status)) {
      Status = SaveBooterLog(NULL, BOOT_LOG);
    }
  }
   */
	return Status;
}

// Made msgbuf and msgCursor private to this source
// so we need a different way of saving the msg log - apianti
EFI_STATUS SaveBooterLog(IN EFI_FILE_HANDLE BaseDir OPTIONAL, IN CHAR16 *FileName)
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

