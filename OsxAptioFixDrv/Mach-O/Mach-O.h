/**

  Basic functions for parsing Mach-O kernel.
  
  by dmazar

**/



/** Returns Mach-O entry point from LC_UNIXTHREAD loader command. */
UINTN
EFIAPI
MachOGetEntryAddress(IN VOID *MachOImage);