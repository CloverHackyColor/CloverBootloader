/*
 * OneLinerMacros.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef INCLUDE_ONELINERMACROS_H_
#define INCLUDE_ONELINERMACROS_H_



/* Decimal powers: */
#define Kilo (1000ULL)
#define Mega (Kilo * Kilo)
#define Giga (Kilo * Mega)
#define Tera (Kilo * Giga)
#define Peta (Kilo * Tera)

#define bit(n)                  (1UL << (n))
#define _Bit(n)                 (1ULL << (n))
#define _HBit(n)                (1ULL << ((n)+32))

#define bitmask(h,l)            ((bit(h)|(bit(h)-1)) & ~(bit(l)-1))
#define bitfield(x,h,l)          RShiftU64(((x) & bitmask((h),(l))), (l))
#define quad(hi,lo)             ((LShiftU64((hi), 32) | (lo)))

#define REG8(base, reg)              ((volatile UINT8 *)(UINTN)(base))[(reg)]
#define REG16(base, reg)             ((volatile UINT16 *)(UINTN)(base))[(reg) >> 1]
//#define REG32(base, reg)             ((volatile UINT32 *)(UINTN)(base))[(reg) >> 2]
#define REG32(base, reg)             (*(volatile UINT32 *)((UINTN)base + reg))
#define WRITEREG32(base, reg, value) REG32((base), (reg)) = value


#define CONCAT2(x, y) x ## y
#define CONCAT(x, y) CONCAT2(x, y)
#define STRINGIZE(x) #x


#ifdef __cplusplus

#include "../cpp_util/remove_ref.h"

#ifdef _MSC_VER
#define __typeof__(x) decltype(x)
#endif
#define __typeof_am__(x) _typeofam_remove_ref<decltype(x)>::type

#endif


#endif /* INCLUDE_ONELINERMACROS_H_ */
