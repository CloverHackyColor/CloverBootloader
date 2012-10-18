/*
 * refit/config.c
 * Configuration file functions
 *
 * Copyright (c) 2006 Christoph Pfisterer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *  * Neither the name of Christoph Pfisterer nor the names of the
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Platform.h"

#ifndef DEBUG_ALL
#define DEBUG_CFG 1
#else
#define DEBUG_CFG DEBUG_ALL
#endif

#if DEBUG_CFG == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_CFG, __VA_ARGS__)
#endif


// constants

#define CONFIG_FILE_NAME    L"refit.conf"
#define MAXCONFIGFILESIZE   (256*1024)

#define ENCODING_ISO8859_1  (0)
#define ENCODING_UTF8       (1)
#define ENCODING_UTF16_LE   (2)


CHAR16*  AnimeName[MAX_ANIME];     // = {NULL};
INTN     AnimeFrames[MAX_ANIME];   //  = {0, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
INTN     AnimeFrameTime[MAX_ANIME];//  = {0, 100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// global configuration with default values

REFIT_CONFIG        GlobalConfig = { FALSE, -1, 0, 0, 0, FALSE, FALSE, FALSE, FALSE, FONT_ALFA, 7, 0xFFFFFF00, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

//
// read a file into a buffer
//

static EFI_STATUS ReadFile(IN EFI_FILE_HANDLE BaseDir, CHAR16 *FileName, REFIT_FILE *File)
{
  EFI_STATUS      Status;
  EFI_FILE_HANDLE FileHandle;
  EFI_FILE_INFO   *FileInfo;
  UINT64          ReadSize;
  
  File->Buffer = NULL;
  File->BufferSize = 0;
  
  // read the file, allocating a buffer on the fly
  Status = BaseDir->Open(BaseDir, &FileHandle, FileName, EFI_FILE_MODE_READ, 0);
  if (CheckError(Status, L"while loading the configuration file"))
    return Status;
  
  FileInfo = EfiLibFileInfo(FileHandle);
  if (FileInfo == NULL) {
    // TODO: print and register the error
    FileHandle->Close(FileHandle);
    return EFI_LOAD_ERROR;
  }
  ReadSize = FileInfo->FileSize;
  if (ReadSize > MAXCONFIGFILESIZE)
    ReadSize = MAXCONFIGFILESIZE;
  FreePool(FileInfo);
  
  File->BufferSize = (UINTN)ReadSize;   // was limited to a few K before, so this is safe
  File->Buffer = AllocateZeroPool (File->BufferSize);
  Status = FileHandle->Read(FileHandle, &File->BufferSize, File->Buffer);
  if (CheckError(Status, L"while loading the configuration file")) {
    FreePool(File->Buffer);
    File->Buffer = NULL;
    FileHandle->Close(FileHandle);
    return Status;
  }
  Status = FileHandle->Close(FileHandle);
  
  // setup for reading
  File->Current8Ptr  = (CHAR8 *)File->Buffer;
  File->End8Ptr      = File->Current8Ptr + File->BufferSize;
  File->Current16Ptr = (CHAR16 *)File->Buffer;
  File->End16Ptr     = File->Current16Ptr + (File->BufferSize >> 1);
  
  // detect encoding
  File->Encoding = ENCODING_ISO8859_1;   // default: 1:1 translation of CHAR8 to CHAR16
  if (File->BufferSize >= 4) {
    if (File->Buffer[0] == 0xFF && File->Buffer[1] == 0xFE) {
      // BOM in UTF-16 little endian (or UTF-32 little endian)
      File->Encoding = ENCODING_UTF16_LE;   // use CHAR16 as is
      File->Current16Ptr++;
    } else if (File->Buffer[0] == 0xEF && File->Buffer[1] == 0xBB && File->Buffer[2] == 0xBF) {
      // BOM in UTF-8
      File->Encoding = ENCODING_UTF8;       // translate from UTF-8 to UTF-16
      File->Current8Ptr += 3;
    } else if (File->Buffer[1] == 0 && File->Buffer[3] == 0) {
      File->Encoding = ENCODING_UTF16_LE;   // use CHAR16 as is
    }
    // TODO: detect other encodings as they are implemented
  }
  
  return EFI_SUCCESS;
}

//
// get a single line of text from a file
//

static CHAR16 *ReadLine(REFIT_FILE *File)
{
  CHAR16  *Line, *q;
  UINTN   LineLength;
  
  if (File->Buffer == NULL)
    return NULL;
  
  if (File->Encoding == ENCODING_ISO8859_1 || File->Encoding == ENCODING_UTF8) {
    
    CHAR8 *p, *LineStart, *LineEnd;
    
    p = File->Current8Ptr;
    if (p >= File->End8Ptr)
      return NULL;
    
    LineStart = p;
    for (; p < File->End8Ptr; p++)
      if (*p == 13 || *p == 10)
        break;
    LineEnd = p;
    for (; p < File->End8Ptr; p++)
      if (*p != 13 && *p != 10)
        break;
    File->Current8Ptr = p;
    
    LineLength = (UINTN)(LineEnd - LineStart) + 1;
    Line = AllocatePool(LineLength * sizeof(CHAR16));
    if (Line == NULL)
      return NULL;
    
    q = Line;
    if (File->Encoding == ENCODING_ISO8859_1) {
      for (p = LineStart; p < LineEnd; )
        *q++ = *p++;
    } else if (File->Encoding == ENCODING_UTF8) {
      // TODO: actually handle UTF-8
      for (p = LineStart; p < LineEnd; )
        *q++ = *p++;
    }
    *q = 0;
    
  } else if (File->Encoding == ENCODING_UTF16_LE) {
    
    CHAR16 *p, *LineStart, *LineEnd;
    
    p = File->Current16Ptr;
    if (p >= File->End16Ptr)
      return NULL;
    
    LineStart = p;
    for (; p < File->End16Ptr; p++)
      if (*p == 13 || *p == 10)
        break;
    LineEnd = p;
    for (; p < File->End16Ptr; p++)
      if (*p != 13 && *p != 10)
        break;
    File->Current16Ptr = p;
    
    LineLength = (UINTN)(LineEnd - LineStart) + 1;
    Line = AllocatePool(LineLength * sizeof(CHAR16));
    if (Line == NULL)
      return NULL;
    
    for (p = LineStart, q = Line; p < LineEnd; )
      *q++ = *p++;
    *q = 0;
    
  } else
    return NULL;   // unsupported encoding
  
  return Line;
}

//
// get a line of tokens from a file
//

static VOID ReadTokenLine(IN REFIT_FILE *File, OUT CHAR16 ***TokenList, OUT UINTN *TokenCount)
{
  BOOLEAN         LineFinished;
  CHAR16          *Line, *Token, *p;
  
  *TokenCount = 0;
  *TokenList = NULL;
  
  while (*TokenCount == 0) {
    Line = ReadLine(File);
    if (Line == NULL)
      return;
    
    p = Line;
    LineFinished = FALSE;
    while (!LineFinished) {
      // skip whitespace
      while (*p == ' ' || *p == '\t' || *p == '=')
        p++;
      if (*p == 0 || *p == '#')
        break;
      
      Token = p;
      
      // find end of token
      while (*p && *p != ' ' && *p != '\t' && *p != '=' && *p != '#')
        p++;
      if (*p == 0 || *p == '#')
        LineFinished = TRUE;
      *p++ = 0;
      
      AddListElement((VOID ***)TokenList, TokenCount, (VOID *)EfiStrDuplicate(Token));
    }
    
    FreePool(Line);
  }
}

static VOID FreeTokenLine(IN OUT CHAR16 ***TokenList, IN OUT UINTN *TokenCount)
{
    // TODO: also free the items
  FreeList((VOID ***)TokenList, TokenCount);
}

//
// handle a parameter with a single integer argument
//

static VOID HandleInt(IN CHAR16 **TokenList, IN UINTN TokenCount, OUT INTN *Value)
{
  if (TokenCount < 2) {
    return;
  }
  if (TokenCount > 2) {
    return;
  }
  *Value = (INTN)StrDecimalToUintn(TokenList[1]);
}

static VOID HandleHex(IN CHAR16 **TokenList, IN UINTN TokenCount, OUT UINTN *Value)
{
  if (TokenCount < 2) {
    return;
  }
  if (TokenCount > 2) {
    return;
  }
  *Value = StrHexToUintn(TokenList[1]);
}

//
// handle a parameter with a single string argument
//

static VOID HandleString(IN CHAR16 **TokenList, IN UINTN TokenCount, OUT CHAR16 **Value)
{
  if (TokenCount < 2) {
    return;
  }
  if (TokenCount > 2) {
    return;
  }
  *Value = EfiStrDuplicate(TokenList[1]);
}

//
// handle an enumeration parameter
//

static VOID HandleEnum(IN CHAR16 **TokenList, IN UINTN TokenCount, IN CHAR16 **EnumList, IN UINTN EnumCount, OUT UINTN *Value)
{
  UINTN i;
  
  if (TokenCount < 2) {
    return;
  }
  if (TokenCount > 2) {
    return;
  }
  // look for the enum value
  for (i = 0; i < EnumCount; i++)
    if (StriCmp(EnumList[i], TokenList[1]) == 0) {
      *Value = i;
      return;
    }
  // try to handle an INTN instead
  *Value = StrDecimalToUintn(TokenList[1]);
}

//
// read config file
//

static CHAR16 *HideBadgesEnum[] = { L"none", L"internal", L"all", L"swap", L"drive"};

VOID ReadConfig(VOID)
{
  EFI_STATUS      Status;
  REFIT_FILE      File;
  CHAR16          **TokenList;
  CHAR16          *FlagName;
  UINTN           TokenCount, i;
  INTN            ID;
  
  if (!FileExists(SelfDir, CONFIG_FILE_NAME))
    return;
  
  //    DBG("Reading configuration file...\n");
  Status = ReadFile(SelfDir, CONFIG_FILE_NAME, &File);
  if (EFI_ERROR(Status))
    return;
  //    DBG("Reading refit.conf file OK!\n");
  for (;;) {
    ReadTokenLine(&File, &TokenList, &TokenCount);
    if (TokenCount == 0)
      break;
    
    if (StriCmp(TokenList[0], L"timeout") == 0) {
      // default is -1
      if (TokenCount == 2 && TokenList[1][0] != L'-') {
        HandleInt(TokenList, TokenCount, &(GlobalConfig.Timeout));
      }
      
    } else if (StriCmp(TokenList[0], L"char_width") == 0) {  
      HandleInt(TokenList, TokenCount, &(GlobalConfig.CharWidth));
      
    } else if (StriCmp(TokenList[0], L"selection_color") == 0) {  
      HandleHex(TokenList, TokenCount, &(GlobalConfig.SelectionColor));
      
    } else if (StriCmp(TokenList[0], L"disable") == 0) {
      for (i = 1; i < TokenCount; i++) {
        FlagName = TokenList[i];
        if (StriCmp(FlagName, L"shell") == 0) {
          GlobalConfig.DisableFlags |= DISABLE_FLAG_SHELL;
        } else if (StriCmp(FlagName, L"tools") == 0) {
          GlobalConfig.DisableFlags |= DISABLE_FLAG_TOOLS;
        } else if (StriCmp(FlagName, L"optical") == 0) {
          GlobalConfig.DisableFlags |= DISABLE_FLAG_OPTICAL;
        } else if (StriCmp(FlagName, L"external") == 0) {
          GlobalConfig.DisableFlags |= DISABLE_FLAG_EXTERNAL;
        } else if (StriCmp(FlagName, L"internal") == 0) {
          GlobalConfig.DisableFlags |= DISABLE_FLAG_INTERNAL;
        } else if (StriCmp(FlagName, L"singleuser") == 0) {
          GlobalConfig.DisableFlags |= DISABLE_FLAG_SINGLEUSER;
        } else if (StriCmp(FlagName, L"hwtest") == 0) {
          GlobalConfig.DisableFlags |= DISABLE_FLAG_HWTEST;
        } else if (StriCmp(FlagName, L"all") == 0) {
          GlobalConfig.DisableFlags = DISABLE_ALL;
        } else {
          DBG(" unknown disable flag: %s\n", FlagName);
        }
      }
      
    } else if (StriCmp(TokenList[0], L"disableopticalboot") == 0) {
      GlobalConfig.DisableFlags |= DISABLE_FLAG_OPTICAL;
    } else if (StriCmp(TokenList[0], L"disableexternalboot") == 0) {
      GlobalConfig.DisableFlags |= DISABLE_FLAG_EXTERNAL;
    } else if (StriCmp(TokenList[0], L"disableinternalboot") == 0) {
      GlobalConfig.DisableFlags |= DISABLE_FLAG_INTERNAL;
      
    } else if (StriCmp(TokenList[0], L"hidebadges") == 0) {
      HandleEnum(TokenList, TokenCount, HideBadgesEnum, 5, &(GlobalConfig.HideBadges));
      
    } else if (StriCmp(TokenList[0], L"hideui") == 0) {
      for (i = 1; i < TokenCount; i++) {
        FlagName = TokenList[i];
        if (StriCmp(FlagName, L"banner") == 0) {
          GlobalConfig.HideUIFlags |= HIDEUI_FLAG_BANNER;
        } else if (StriCmp(FlagName, L"shell") == 0) {
          GlobalConfig.DisableFlags |= DISABLE_FLAG_SHELL;
        } else if (StriCmp(FlagName, L"tools") == 0) {
          GlobalConfig.DisableFlags |= DISABLE_FLAG_TOOLS;
        } else if (StriCmp(FlagName, L"funcs") == 0) {
          GlobalConfig.HideUIFlags |= HIDEUI_FLAG_FUNCS;
        } else if (StriCmp(FlagName, L"label") == 0) {
          GlobalConfig.HideUIFlags |= HIDEUI_FLAG_LABEL;
        } else if (StriCmp(FlagName, L"revision") == 0) {
          GlobalConfig.HideUIFlags |= HIDEUI_FLAG_REVISION;
        } else if (StriCmp(FlagName, L"hdbadges") == 0) {
          if (GlobalConfig.HideBadges < 1)
            GlobalConfig.HideBadges = 1;
        } else if (StriCmp(FlagName, L"badges") == 0) {
          if (GlobalConfig.HideBadges < 2)
            GlobalConfig.HideBadges = 2;
        } else if (StriCmp(FlagName, L"all") == 0) {
          GlobalConfig.HideUIFlags = HIDEUI_ALL;
          GlobalConfig.DisableFlags |= DISABLE_FLAG_SHELL | DISABLE_FLAG_TOOLS;
          if (GlobalConfig.HideBadges < 1)
            GlobalConfig.HideBadges = 1;
        } else {
          DBG(" unknown hideui flag: %s\n", FlagName);
        }
      }
      
    } else if (StriCmp(TokenList[0], L"theme") == 0) {
      HandleString(TokenList, TokenCount, &(GlobalConfig.Theme));
      
    } else if (StriCmp(TokenList[0], L"font") == 0) {
      FlagName = TokenList[1];
      if (StriCmp(FlagName, L"alfa") == 0) {
        GlobalConfig.Font = FONT_ALFA;
      } else if (StriCmp(FlagName, L"gray") == 0) {
        GlobalConfig.Font = FONT_GRAY;
      } else if (StriCmp(FlagName, L"load") == 0) {
        GlobalConfig.Font = FONT_LOAD;
      }  else {
        DBG(" unknown font type: %s\n", FlagName);
      }
      
    } else if (StriCmp(TokenList[0], L"anime") == 0) {
      ID = (INTN)StrDecimalToUintn(TokenList[1]);   //AnimationID
      AnimeName[ID] = EfiStrDuplicate(TokenList[2]);
      if (TokenCount <= 3) {
        AnimeFrames[ID] = 100; //as we have name we propose many frames here
      } else {
        AnimeFrames[ID] = (INTN)StrDecimalToUintn(TokenList[3]);
        if (TokenCount <= 4) {
          AnimeFrameTime[ID] = 100; //ms
        } else {
          AnimeFrameTime[ID] = (INTN)StrDecimalToUintn(TokenList[4]);
        }
      }
      
    } else if (StriCmp(TokenList[0], L"font_file_name") == 0) {
      HandleString(TokenList, TokenCount, &(GlobalConfig.FontFileName));
      
    } else if (StriCmp(TokenList[0], L"banner") == 0) {
      HandleString(TokenList, TokenCount, &(GlobalConfig.BannerFileName));
      
    } else if (StriCmp(TokenList[0], L"selection_small") == 0) {
      HandleString(TokenList, TokenCount, &(GlobalConfig.SelectionSmallFileName));
      
    } else if (StriCmp(TokenList[0], L"selection_big") == 0) {
      HandleString(TokenList, TokenCount, &(GlobalConfig.SelectionBigFileName));
      
    } else if (StriCmp(TokenList[0], L"default_selection") == 0) {
      HandleString(TokenList, TokenCount, &(GlobalConfig.DefaultSelection));
      
    } else if (StriCmp(TokenList[0], L"textonly") == 0) {
      GlobalConfig.TextOnly = TRUE;
      
    } else if (StriCmp(TokenList[0], L"legacyfirst") == 0) {
      GlobalConfig.LegacyFirst = TRUE;
      
    } else if (StriCmp(TokenList[0], L"quiet") == 0) {
      GlobalConfig.Quiet = TRUE;
      
    } else if (StriCmp(TokenList[0], L"nolegacy") == 0) {
      GlobalConfig.NoLegacy = TRUE;
      
    } else if (StriCmp(TokenList[0], L"screen_resolution") == 0) {
      HandleString(TokenList, TokenCount, &(GlobalConfig.ScreenResolution));
      
    } else if (StriCmp(TokenList[0], L"nolog") == 0) {
      GlobalConfig.NoLogging = TRUE;
    } else {
      DBG(" unknown configuration command: %s\n", TokenList[0]);
    }
    
    FreeTokenLine(&TokenList, &TokenCount);
  }
  
  FreePool(File.Buffer);
}
