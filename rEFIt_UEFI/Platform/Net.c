// Slice 2014

#include "Platform.h"
#include <Library/NetLib.h>

#ifndef DEBUG_NET
#ifndef DEBUG_ALL
#define DEBUG_NET 1
#else
#define DEBUG_NET DEBUG_ALL
#endif
#endif

#if DEBUG_NET == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_NET, __VA_ARGS__)
#endif


VOID
GetMacAddress()
{
  EFI_STATUS                   Status;
  EFI_MAC_ADDRESS              MacAddr;
  UINT8                        *HwAddress;
  UINTN                        AddrSize;
  UINTN                        Index, Index2;
  EFI_HANDLE                   *HandleBuffer;
  UINTN                        NumberOfHandles;


  //
  // Locate Service Binding handles.
  //
  NumberOfHandles = 0;
  HandleBuffer = NULL;
  Status = gBS->LocateHandleBuffer (
                                    ByProtocol,
                                    &gEfiManagedNetworkServiceBindingProtocolGuid,
                                    NULL,
                                    &NumberOfHandles,
                                    &HandleBuffer
                                    );
  if (EFI_ERROR (Status)) {
    DBG("Network protocol is not installed\n");
    return;
  }

  for (Index = 0; Index < NumberOfHandles; Index++) {
    //
    // Get MAC address.
    //
    AddrSize = 0;
    ZeroMem (&MacAddr, sizeof(EFI_MAC_ADDRESS));
    Status = NetLibGetMacAddress (HandleBuffer[Index], &MacAddr, &AddrSize);
    if (EFI_ERROR (Status)) {
      DBG("MAC address of LAN %d is not accessible\n", Index);
      continue;
    }
    if (AddrSize != 6) {
      DBG(" WTH! MacAddress size =%d\n", AddrSize);
      continue; //do nothing for security
    }

    DBG("MAC address of LAN %d:", Index);
    HwAddress = (UINT8*)&MacAddr.Addr[0];
    for (Index2 = 0; Index2 < AddrSize; Index2++) {
      DBG("%02x:", *HwAddress++);
    }
    DBG("\n");
  }

}