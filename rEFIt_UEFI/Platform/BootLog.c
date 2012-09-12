/*
 *  BootLog.c
 *  
 *
 *  Created by Slice on 19.08.11.
 *  Edited by apianti 2012-09-08
 *  Initial idea from Kabyl
 */


#include "Platform.h"
extern  EFI_GUID  gEfiMiscSubClassGuid;

CHAR8 *msgbuf = 0;
CHAR8 *msgCursor = 0;
BOOLEAN msgHadSelfRoot = FALSE;

VOID InitBooterLog(VOID)
{
  EFI_STATUS		Status	= EFI_SUCCESS;
  MESSAGE_LOG_PROTOCOL*         Msg;
  INTN  N;
   
   // Allocate MsgLog
	msgbuf = AllocateZeroPool(MSG_LOG_SIZE);
	msgCursor = msgbuf;
  
   // Search for Clover log and copy to MsgLog
  Status = gBS->LocateProtocol (&gMsgLogProtocolGuid, NULL, (VOID **)&Msg);
   
   if (!EFI_ERROR (Status)) 
   {
     if (Msg->Log && Msg->Cursor) {
       
       N =(INTN)((INTN)Msg->Cursor - (INTN)Msg->Log);
       MsgLog("Log from Clover size=%d:\n", N);
       N &= 0xFFF;
       if ((N > 0) && (N < MSG_LOG_SIZE)) {
         CopyMem(msgCursor, Msg->Log, N);
         msgCursor += N;
         *msgCursor = 0;
         FreePool(Msg->Log);         
       }
       else {
         MsgLog("no BootLog from CloverEFI\n");
       }
     }
   }
}

EFI_STATUS SetupBooterLog(VOID)
{
	EFI_STATUS		Status	= EFI_SUCCESS;
	UINTN			LogSize;
  //CHAR16*    BootLogName = L"EFI\\misc\\boot.log";
	if (!msgbuf || !msgCursor ||
       (msgCursor < msgbuf) ||
       (msgCursor >= (msgbuf + MSG_LOG_SIZE)))
		return EFI_NOT_FOUND;
	LogSize  = msgCursor - msgbuf;
	Status =  LogDataHub(&gEfiMiscSubClassGuid, L"boot-log", msgbuf, (UINT32)LogSize);
   // Save BOOT_LOG only once on successful boot
   Status = SaveBooterLog(SelfRootDir, BOOT_LOG);
  if (EFI_ERROR(Status)) {
    Status = SaveBooterLog(NULL, BOOT_LOG);
  }
   /*
  Status = egSaveFile(SelfRootDir, BootLogName, (UINT8*)msgbuf, LogSize);
  if (EFI_ERROR(Status)) {
    Status = egSaveFile(NULL, BootLogName, (UINT8*)msgbuf, LogSize);
  }
  */
   
	return Status;
}
/* Kabyl: !BooterLog */

// Made msgbuf and msgCursor private to this source
// so we need a different way of saving the msg log - apianti
EFI_STATUS SaveBooterLog(IN EFI_FILE_HANDLE BaseDir OPTIONAL, IN CHAR16 *FileName)
{
   UINTN LogSize = 0;
   if ((msgCursor > msgbuf) && (msgCursor <= msgbuf + (MSG_LOG_SIZE)))
      LogSize = (UINTN)(msgCursor - msgbuf);
   return egSaveFile(BaseDir, FileName, (UINT8*)msgbuf, LogSize);
}

// Changed MsgLog(...) it now calls this function
//  with DebugMode == 0. - apianti
// DebugMode==0 Prints to msg log, only output to log on SaveBooterLog
// DebugMode==1 Prints to msg log and SYSTEM_LOG
// DebugMode==2 Prints to msg log, SYSTEM_LOG and display console
VOID DebugLog(IN INTN DebugMode, IN CONST CHAR8 *FormatString, ...)
{
   VA_LIST Marker;
   UINTN offset = 0;
   
   // Make sure the buffer is intact for writing
   if (GlobalConfig.NoLogging ||
       !FormatString || !msgbuf || !msgCursor ||
       (msgCursor < msgbuf) ||
       (msgCursor >= (msgbuf + MSG_LOG_SIZE)) ||
       (DebugMode < 0))
      return;

   // Print message to log buffer
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
   if (DebugMode >= 1)
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
               if (!msgHadSelfRoot)
               {
                  LogSize += (msgCursor - msgbuf);
                  LogFile->Write(LogFile, &LogSize, msgbuf);
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
   if (SelfRootDir)
   {
      msgHadSelfRoot = TRUE;
   }

   // Advance log buffer
   msgCursor += offset;
}
