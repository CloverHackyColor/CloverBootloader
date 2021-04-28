/*
 * Net.h
 *
 *  Created on: Apr 27, 2021
 *      Author: jief
 */

#ifndef INCLUDE_NET_H_
#define INCLUDE_NET_H_



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



#endif /* INCLUDE_NET_H_ */
