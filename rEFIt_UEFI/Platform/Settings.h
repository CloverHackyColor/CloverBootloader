
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
SetBootCurrent(REFIT_MENU_ENTRY_LOADER *LoadedEntry);


CHAR8
*GetOSVersion (
  IN  LOADER_ENTRY *Entry
  );
