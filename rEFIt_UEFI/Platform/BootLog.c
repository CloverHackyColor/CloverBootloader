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
  if (!GlobalConfig.NoLogging){
    Status = SaveBooterLog(SelfRootDir, BOOT_LOG);
    if (EFI_ERROR(Status)) {
      Status = SaveBooterLog(NULL, BOOT_LOG);
    }
  }
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
   if (/*GlobalConfig.NoLogging || */
       FormatString == NULL || DebugMode < 0)
   {
     return;
   }

   // Print message to log buffer
   VA_START(Marker, FormatString);
   MemLogVA(FormatString, Marker);
   VA_END(Marker);
  /*
   VA_START(Marker, FormatString);
   offset = AsciiVSPrint(msgCursor, (MSG_LOG_SIZE-(msgCursor-msgbuf)), FormatString, Marker);
   VA_END(Marker);

   // Don't continue if nothing written
   if (!offset)
      return;

   // Print message to console
   if (DebugMode >= 2)
      AsciiPrint(msgCursor);

   // Print message to log file
   if ((DebugMode >= 1) && !GlobalConfig.NoLogging)
   {
      UINTN LogSize = offset;
      EFI_FILE_HANDLE LogFile = NULL;
      // Check to make sure we have a root directory
      if (SelfRootDir)
      {
         // Open log file from current root
         EFI_STATUS Status = SelfRootDir->Open(SelfRootDir, &LogFile, SYSTEM_LOG,
                                               EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
         // If the log file is not found try to create it
         if (Status == EFI_NOT_FOUND)
         {
            Status = SelfRootDir->Open(SelfRootDir, &LogFile, SYSTEM_LOG,
                                       EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
         }
         // Write to the log file
         if (!EFI_ERROR(Status) && LogFile)
         {
            // Advance to the EOF so we append
            EFI_FILE_INFO *Info = EfiLibFileInfo(LogFile);
            if (Info)
            {
               LogFile->SetPosition(LogFile, Info->FileSize);
               // If we haven't had root before this write out whole log
               if (!msgLast)
               {
                  msgLast = msgbuf;
               }
               if (msgLast < msgCursor)
               {
                  LogSize += (msgCursor - msgLast);
                  LogFile->Write(LogFile, &LogSize, msgLast);
               }
               else
               {
                  // Write out this message
                  LogFile->Write(LogFile, &LogSize, msgCursor);
               }
            }
            LogFile->Close(LogFile);
         }
      }
   }
   // Advance log buffer
   msgCursor += offset;
   if (SelfRootDir)
   {
      msgLast = msgCursor;
   }
   */
}
