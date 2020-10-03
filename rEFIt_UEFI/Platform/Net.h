/*
 * Net.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef PLATFORM_NET_H_
#define PLATFORM_NET_H_



void
GetMacAddress(void);

extern UINTN                           nLanCards;      // number of LAN cards
extern UINT16                          gLanVendor[4];  // their vendors
extern UINT8                           *gLanMmio[4];   // their MMIO regions
extern UINT8                           gLanMac[4][6];  // their MAC addresses
extern UINTN                           nLanPaths;      // number of UEFI LAN


#endif /* PLATFORM_NET_H_ */
