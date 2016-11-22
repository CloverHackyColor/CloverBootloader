/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2004,2007,2009  Free Software Foundation, Inc.
 *
 *  GRUB is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GRUB is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GRUB.  If not, see <http://www.gnu.org/licenses/>.
 */

/* We need to include config-util.h.in for HAVE_*.  */
#ifndef __STDC_VERSION__
#define __STDC_VERSION__ 0
#endif
#include <config-util.h>

/* On x86 these functions aren't really needed. Save some space.  */
#if !defined (__i386__) && !defined (__x86_64__)
# ifdef HAVE___ASHLDI3
void EXPORT_FUNC (__ashldi3) (void);
# endif
# ifdef HAVE___ASHRDI3
void EXPORT_FUNC (__ashrdi3) (void);
# endif
# ifdef HAVE___LSHRDI3
void EXPORT_FUNC (__lshrdi3) (void);
# endif
# ifdef HAVE___UCMPDI2
void EXPORT_FUNC (__ucmpdi2) (void);
# endif
# ifdef HAVE___BSWAPSI2
void EXPORT_FUNC (__bswapsi2) (void);
# endif
# ifdef HAVE___BSWAPDI2
void EXPORT_FUNC (__bswapdi2) (void);
# endif
#endif

#ifdef HAVE__RESTGPR_14_X
void EXPORT_FUNC (_restgpr_14_x) (void);
void EXPORT_FUNC (_restgpr_15_x) (void);
void EXPORT_FUNC (_restgpr_16_x) (void);
void EXPORT_FUNC (_restgpr_17_x) (void);
void EXPORT_FUNC (_restgpr_18_x) (void);
void EXPORT_FUNC (_restgpr_19_x) (void);
void EXPORT_FUNC (_restgpr_20_x) (void);
void EXPORT_FUNC (_restgpr_21_x) (void);
void EXPORT_FUNC (_restgpr_22_x) (void);
void EXPORT_FUNC (_restgpr_23_x) (void);
void EXPORT_FUNC (_restgpr_24_x) (void);
void EXPORT_FUNC (_restgpr_25_x) (void);
void EXPORT_FUNC (_restgpr_26_x) (void);
void EXPORT_FUNC (_restgpr_27_x) (void);
void EXPORT_FUNC (_restgpr_28_x) (void);
void EXPORT_FUNC (_restgpr_29_x) (void);
void EXPORT_FUNC (_restgpr_30_x) (void);
void EXPORT_FUNC (_restgpr_31_x) (void);
void EXPORT_FUNC (_savegpr_14) (void);
void EXPORT_FUNC (_savegpr_15) (void);
void EXPORT_FUNC (_savegpr_16) (void);
void EXPORT_FUNC (_savegpr_17) (void);
void EXPORT_FUNC (_savegpr_18) (void);
void EXPORT_FUNC (_savegpr_19) (void);
void EXPORT_FUNC (_savegpr_20) (void);
void EXPORT_FUNC (_savegpr_21) (void);
void EXPORT_FUNC (_savegpr_22) (void);
void EXPORT_FUNC (_savegpr_23) (void);
void EXPORT_FUNC (_savegpr_24) (void);
void EXPORT_FUNC (_savegpr_25) (void);
void EXPORT_FUNC (_savegpr_26) (void);
void EXPORT_FUNC (_savegpr_27) (void);
void EXPORT_FUNC (_savegpr_28) (void);
void EXPORT_FUNC (_savegpr_29) (void);
void EXPORT_FUNC (_savegpr_30) (void);
void EXPORT_FUNC (_savegpr_31) (void);
#endif

#if defined (__arm__)
void EXPORT_FUNC (__aeabi_lasr) (void);
void EXPORT_FUNC (__aeabi_llsl) (void);
void EXPORT_FUNC (__aeabi_llsr) (void);
void EXPORT_FUNC (__aeabi_ulcmp) (void);
#endif
