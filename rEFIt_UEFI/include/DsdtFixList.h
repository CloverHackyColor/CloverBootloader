/*
 * DsdtFixList.h
 *
 *  Created on: Feb 6, 2021
 *      Author: jief
 */

#ifndef INCLUDE_DSDTFIXLIST_H_
#define INCLUDE_DSDTFIXLIST_H_

#include "../include/OneLinerMacros.h"


//DSDT fixes MASK
//0x00FF
#define FIX_DTGP      bit(0)
#define FIX_WARNING   bit(1)
#define FIX_SHUTDOWN  bit(2)
#define FIX_MCHC      bit(3)
#define FIX_HPET      bit(4)
#define FIX_LPC       bit(5)
#define FIX_IPIC      bit(6)
#define FIX_SBUS      bit(7)
//0xFF00
#define FIX_DISPLAY   bit(8)
#define FIX_IDE       bit(9)
#define FIX_SATA      bit(10)
#define FIX_FIREWIRE  bit(11)
#define FIX_USB       bit(12)
#define FIX_LAN       bit(13)
#define FIX_WIFI      bit(14)
#define FIX_HDA       bit(15)
//new bits 16-31 0xFFFF0000
//#define FIX_NEW_WAY   bit(31) will be reused
#define FIX_DARWIN    bit(16)
#define FIX_RTC       bit(17)
#define FIX_TMR       bit(18)
#define FIX_IMEI      bit(19)
#define FIX_INTELGFX  bit(20)
#define FIX_WAK       bit(21)
#define FIX_UNUSED    bit(22)
#define FIX_ADP1      bit(23)
#define FIX_PNLF      bit(24)
#define FIX_S3D       bit(25)
#define FIX_ACST      bit(26)
#define FIX_HDMI      bit(27)
#define FIX_REGIONS   bit(28)
#define FIX_HEADERS_DEPRECATED   bit(29) // deprecated !!
#define FIX_MUTEX     bit(30)




#endif /* INCLUDE_DSDTFIXLIST_H_ */
