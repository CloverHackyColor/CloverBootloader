/**

  funcs for checking decoded kernel in memory
  
  dmazar

**/


extern EFI_PHYSICAL_ADDRESS gRelocBase;

//EFI_STATUS EFIAPI CheckDecodedKernel (VOID);
VOID EFIAPI DumpStack (UINT64 RSP);
