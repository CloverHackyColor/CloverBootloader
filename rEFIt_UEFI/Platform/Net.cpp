// Slice 2014

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include <Library/NetLib.h>
#include "../refit/lib.h"

#ifndef DEBUG_MAC
#ifndef DEBUG_ALL
#define DEBUG_MAC 1
#else
#define DEBUG_MAC DEBUG_ALL
#endif
#endif

#if DEBUG_MAC == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_MAC, __VA_ARGS__)
#endif

extern UINTN                           nLanCards;     // number of LAN cards
extern UINTN                           nLanPaths;     // number of UEFI LAN
extern UINT16                          gLanVendor[4]; // their vendors
extern UINT8                           *gLanMmio[4];   // their MMIO regions
extern UINT8                           gLanMac[4][6]; // their MAC addresses
extern BOOLEAN                         GetLegacyLanAddress;

//Marvell Yukon
#define B2_MAC_1    0x0100    /* NA reg MAC Address 1 */
#define B2_MAC_2    0x0108    /* NA reg MAC Address 2 */
#define B2_MAC_3    0x0110    /* NA reg MAC Address 3 */

//Atheros
#define L1C_STAD0                       0x1488
#define L1C_STAD1                       0x148C

//Intel
#define INTEL_MAC_1                     0x5400
#define INTEL_MAC_2                     0x54E0

// Broadcom MAC Address Registers
#define EMAC_MACADDR0_HI                  0x00000410
#define EMAC_MACADDR0_LO                  0x00000414
#define EMAC_MACADDR1_HI                  0x00000418
#define EMAC_MACADDR1_LO                  0x0000041C
#define EMAC_MACADDR2_HI                  0x00000420
#define EMAC_MACADDR2_LO                  0x00000424
#define EMAC_MACADDR3_HI                  0x00000428
#define EMAC_MACADDR3_LO                  0x0000042C

/*
 ///
 /// MAC Address Device Path SubType.
 ///
 #define MSG_MAC_ADDR_DP           0x0b
typedef struct {
  EFI_DEVICE_PATH_PROTOCOL        Header;
  ///
  /// The MAC address for a network interface padded with 0s.
  ///
  EFI_MAC_ADDRESS                 MacAddress;
  ///
  /// Network interface type(i.e. 802.3, FDDI).
  ///
  UINT8                           IfType;
} MAC_ADDR_DEVICE_PATH;
 
 */

VOID
GetMacAddress()
{
  EFI_STATUS                  Status;
  EFI_MAC_ADDRESS             MacAddr;
  UINT8                       *HwAddress;
  UINTN                       HwAddressSize;
  UINTN                       Index, Index2;
  EFI_HANDLE                  *HandleBuffer;
  UINTN                       NumberOfHandles;
  EFI_DEVICE_PATH_PROTOCOL    *Node;
  EFI_DEVICE_PATH_PROTOCOL    *DevicePath;
  MAC_ADDR_DEVICE_PATH        *MacAddressNode;
  BOOLEAN                     Found;
  BOOLEAN                     Swab;
  UINT16                      PreviousVendor = 0;
  UINT32                      Mac0, Mac4;
  UINTN                       Offset;

  HwAddressSize = 6;
  //
  // Locate Service Binding handles.
  //
  NumberOfHandles = 0;
  HandleBuffer = NULL;
  Status = gBS->LocateHandleBuffer (
                                    ByProtocol,
                                    &gEfiDevicePathProtocolGuid,
                                    NULL,
                                    &NumberOfHandles,
                                    &HandleBuffer
                                    );
  if (EFI_ERROR(Status)) {
    return;
  }

  DbgHeader("GetMacAddress");

  Found = FALSE;
  for (Index = 0; Index < NumberOfHandles; Index++) {
    Node = NULL;
    Status = gBS->HandleProtocol (
                                  HandleBuffer[Index],
                                  &gEfiDevicePathProtocolGuid,
                                  (VOID **) &Node
                                  );
    if (EFI_ERROR(Status)) {
      continue;
    }
    DevicePath = (EFI_DEVICE_PATH_PROTOCOL *) Node;

    //
    while (!IsDevicePathEnd (DevicePath)) {
      if ((DevicePathType (DevicePath) == MESSAGING_DEVICE_PATH) &&
          (DevicePathSubType (DevicePath) == MSG_MAC_ADDR_DP)) {
        //
        // Get MAC address.
        //
        MacAddressNode = (MAC_ADDR_DEVICE_PATH*)DevicePath;
        //HwAddressSize = sizeof (EFI_MAC_ADDRESS);
        //if (MacAddressNode->IfType == 0x01 || MacAddressNode->IfType == 0x00) {
        //  HwAddressSize = 6;
        //}
        CopyMem(&MacAddr, &MacAddressNode->MacAddress.Addr[0], HwAddressSize);
		  DBG("MAC address of LAN #%llu= ", nLanPaths);
        HwAddress = &MacAddressNode->MacAddress.Addr[0];
        for (Index2 = 0; Index2 < HwAddressSize; Index2++) {
          DBG("%02hhX:", *HwAddress++);
        }
        DBG("\n");
        Found = TRUE;
        CopyMem(&gLanMac[nLanPaths++], &MacAddressNode->MacAddress.Addr[0], HwAddressSize);
        break;
      }
      DevicePath = NextDevicePathNode (DevicePath);
    }
    if (nLanPaths > 3) {
      break;
    }
  }
  if (HandleBuffer != NULL) {
    FreePool(HandleBuffer);
  }
  if (!Found && GetLegacyLanAddress) {
    ////
    //
    //  Legacy boot. Get MAC-address from hardwaredirectly
    //
    ////
	  DBG(" get legacy LAN MAC, %llu card found\n", nLanCards);
    for (Index = 0; Index < nLanCards; Index++) {
      if (!gLanMmio[Index]) {  //security
        continue;
      }
      Offset = 0;
      Swab = FALSE;
      switch (gLanVendor[Index]) {
        case 0x11ab:   //Marvell Yukon
          if (PreviousVendor == gLanVendor[Index]) {
            Offset = B2_MAC_2;
          } else {
            Offset = B2_MAC_1;
          }
          CopyMem(&gLanMac[0][Index], gLanMmio[Index] + Offset, 6);
          goto done;

        case 0x10ec:   //Realtek
          Mac0 = IoRead32((UINTN)gLanMmio[Index]);
          Mac4 = IoRead32((UINTN)gLanMmio[Index] + 4);
          goto copy;
          
        case 0x14e4:   //Broadcom
          if (PreviousVendor == gLanVendor[Index]) {
            Offset = EMAC_MACADDR1_HI;
          } else {
            Offset = EMAC_MACADDR0_HI;
          }
          break;
        case 0x1969:   //Atheros
          Offset = L1C_STAD0;
          Swab = TRUE;
          break;
        case 0x8086:   //Intel
          if (PreviousVendor == gLanVendor[Index]) {
            Offset = INTEL_MAC_2;
          } else {
            Offset = INTEL_MAC_1;
          }
          break;
          
        default:
          break;
      }
      if (!Offset) {
        continue;
      }
      Mac0 = *(UINT32*)(gLanMmio[Index] + Offset);
      Mac4 = *(UINT32*)(gLanMmio[Index] + Offset + 4);
      if (Swab) {
        gLanMac[Index][0] = (UINT8)((Mac4 & 0xFF00) >> 8);
        gLanMac[Index][1] = (UINT8)(Mac4 & 0xFF);
        gLanMac[Index][2] = (UINT8)((Mac0 & 0xFF000000) >> 24);
        gLanMac[Index][3] = (UINT8)((Mac0 & 0x00FF0000) >> 16);
        gLanMac[Index][4] = (UINT8)((Mac0 & 0x0000FF00) >> 8);
        gLanMac[Index][5] = (UINT8)(Mac0 & 0x000000FF);
        goto done;
      }
    copy:
      CopyMem(&gLanMac[Index][0], &Mac0, 4);
      CopyMem(&gLanMac[Index][4], &Mac4, 2);
      
    done:
      PreviousVendor = gLanVendor[Index];
		DBG("Legacy MAC address of LAN #%llu= ", Index);
      HwAddress = &gLanMac[Index][0];
      for (Index2 = 0; Index2 < HwAddressSize; Index2++) {
        DBG("%02hhX:", *HwAddress++);
      }
      DBG("\n");

    }
  }
}
