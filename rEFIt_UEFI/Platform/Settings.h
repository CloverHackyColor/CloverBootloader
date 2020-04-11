#ifndef __SETTINGS_H__
#define __SETTINGS_H__

extern UINTN       AudioNum;
extern HDA_OUTPUTS AudioList[20];

extern CONST CHAR16* ThemesList[100]; //no more then 100 themes?
extern CHAR16*       ConfigsList[20];
extern CHAR16*       DsdtsList[20];
extern UINTN DsdtsNum;
extern UINTN ThemesNum;
extern UINTN ConfigsNum;
extern INTN                     ScrollButtonsHeight;
extern INTN    ScrollBarDecorationsHeight;
extern INTN    ScrollScrollDecorationsHeight;
extern INTN LayoutBannerOffset;
extern INTN LayoutButtonOffset;
extern INTN LayoutTextOffset;
// this should go in a globals, not in settings

extern INTN                            OldChosenTheme;
extern INTN                            OldChosenConfig;
extern INTN                            OldChosenDsdt;
extern UINTN                            OldChosenAudio;
extern BOOLEAN                        SavePreBootLog;
extern UINT8                            DefaultAudioVolume;


EFI_STATUS
SetFSInjection (
  IN LOADER_ENTRY *Entry
  );

VOID
SetDevices (
  LOADER_ENTRY *Entry
  );
//
// check if this entry corresponds to Boot# variable and then set BootCurrent
//
VOID
SetBootCurrent(REFIT_MENU_ITEM_BOOTNUM *LoadedEntry);


CHAR8
*GetOSVersion (
  IN  LOADER_ENTRY *Entry
  );


VOID GetListOfThemes(VOID);
VOID GetListOfConfigs(VOID);
VOID GetListOfACPI(VOID);
VOID GetListOfDsdts(VOID);

// syscl - get list of inject kext(s)
VOID GetListOfInjectKext(CHAR16 *);


#endif
