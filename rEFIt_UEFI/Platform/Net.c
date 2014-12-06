// Slice 2014

#include "Platform.h"
#include <Library/NetLib.h>

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
  if (EFI_ERROR (Status)) {
    return;
  }


  for (Index = 0; Index < NumberOfHandles; Index++) {
    Node = NULL;
    Status = gBS->HandleProtocol (
                                  HandleBuffer[Index],
                                  &gEfiDevicePathProtocolGuid,
                                  (VOID **) &Node
                                  );
    if (EFI_ERROR (Status)) {
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
        HwAddressSize = sizeof (EFI_MAC_ADDRESS);
        if (MacAddressNode->IfType == 0x01 || MacAddressNode->IfType == 0x00) {
          HwAddressSize = 6;
        }
        CopyMem(&MacAddr, &MacAddressNode->MacAddress.Addr[0], HwAddressSize);
        DBG("MAC address of LAN #%d= ", Index);
        HwAddress = &MacAddressNode->MacAddress.Addr[0];
        for (Index2 = 0; Index2 < HwAddressSize; Index2++) {
          DBG("%02x:", *HwAddress++);
        }
        DBG("\n");
        break;
      }
      DevicePath = NextDevicePathNode (DevicePath);
    }
  }
  if (HandleBuffer != NULL) {
    FreePool (HandleBuffer);
  }

}

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

if (Buffer != NULL) {
  FreePool (Buffer);
}
*/

////
//
//  Legacy boot. Get MAC-address from hardwaredirectly
//
////
//
// different procedures

// Marvell Yukon
#define ETHER_ADDR_LEN 6
/* NA reg = 48 bit Network Address Register, 3x16 or 8x8 bit readable */
#define B2_MAC_1    0x0100    /* NA reg MAC Address 1 */
#define B2_MAC_2    0x0108    /* NA reg MAC Address 2 */
#define B2_MAC_3    0x0110    /* NA reg MAC Address 3 */

//csrbase is address of CSR registers located at PCI Base Address Range 0 (0x10)

//ptrB0 =(char *)csrBase + B2_MAC_1;
//bcopy (ptrB0, &addrMAC, ETH_ALEN);


// Realtek 8139
/*
RTL_IDR0            = 0x00;   // ID reg 0
RTL_IDR4            = 0x04;   // ID reg 4, four byte access only
inline UInt32 csrRead32( UInt16 offset )
{ return OSReadLittleInt32( csrBase, offset ); }


idr.int32 = OSSwapLittleToHostInt32(csrAccess->csrRead32(RTL_IDR0));
address->bytes[0] = idr.bytes[0];
address->bytes[1] = idr.bytes[1];
address->bytes[2] = idr.bytes[2];
address->bytes[3] = idr.bytes[3];

idr.int32 = OSSwapLittleToHostInt32(csrAccess->csrRead32(RTL_IDR4));
address->bytes[4] = idr.bytes[0];
address->bytes[5] = idr.bytes[1];
 */


// Realtek 8168
/*
 MAC0 = 0;

for (uchar i = 0; i < ETHER_ADDR_LEN; i++)
{
  addr->bytes[i] = ReadMMIO8(MAC0 + i);
}
*/


//Atheros

#define L1C_STAD0                       0x1488
#define L1C_STAD1                       0x148C

/*
alx_mem_r32(hw, L1C_STAD0, &mac0);
alx_mem_r32(hw, L1C_STAD1, &mac1);
*/


// Intel
/*
#define E1000_RAL(_i)  (((_i) <= 15) ? (0x05400 + ((_i) * 8)) : \
(0x054E0 + ((_i - 16) * 8)))
#define E1000_RAH(_i)  (((_i) <= 15) ? (0x05404 + ((_i) * 8)) : \
(0x054E4 + ((_i - 16) * 8)))
rar_high = E1000_READ_REG(hw, E1000_RAH(0));
rar_low = E1000_READ_REG(hw, E1000_RAL(0));

for (i = 0; i < E1000_RAL_MAC_ADDR_LEN; i++)
  hw->mac.perm_addr[i] = (u8)(rar_low >> (i*8));

for (i = 0; i < E1000_RAH_MAC_ADDR_LEN; i++)
  hw->mac.perm_addr[i+4] = (u8)(rar_high >> (i*8));

for (i = 0; i < ETH_ADDR_LEN; i++)
  hw->mac.addr[i] = hw->mac.perm_addr[i];

*/