/*
 * OSType.h
 *
 *  Created on: Nov 12, 2020
 *      Author: jief
 */

#ifndef INCLUDE_OSTYPE_H_
#define INCLUDE_OSTYPE_H_



#define OSTYPE_OSX              (1)
#define OSTYPE_WIN              (2)
#define OSTYPE_VAR              (3)
#define OSTYPE_LIN              (4)
#define OSTYPE_LINEFI           (5)
#define OSTYPE_EFI              (6)
#define OSTYPE_WINEFI           (7)
//#define OSTYPE_BOOT_OSX         (9)
#define OSTYPE_RECOVERY         (10)
#define OSTYPE_OSX_INSTALLER    (11)
/*#define OSTYPE_TIGER            (14)
 #define OSTYPE_LEO              (15)
 #define OSTYPE_SNOW             (16)
 #define OSTYPE_LION             (17)
 #define OSTYPE_ML               (18)
 #define OSTYPE_MAV              (19)*/
#define OSTYPE_OTHER            (99)
//#define OSTYPE_HIDE             (100)

#define OSTYPE_IS_OSX(type) ((type == OSTYPE_OSX) /*|| (type == OSTYPE_BOOT_OSX) || ((type >= OSTYPE_TIGER) && (type <= OSTYPE_MAV))*/ || (type == OSTYPE_VAR))
#define OSTYPE_IS_OSX_RECOVERY(type) ((type == OSTYPE_RECOVERY) /*|| ((type >= OSTYPE_TIGER) && (type <= OSTYPE_MAV))*/ || (type == OSTYPE_VAR))
#define OSTYPE_IS_OSX_INSTALLER(type) ((type == OSTYPE_OSX_INSTALLER) /*|| ((type >= OSTYPE_TIGER) && (type <= OSTYPE_MAV))*/ || (type == OSTYPE_VAR))
#define OSTYPE_IS_WINDOWS(type) ((type == OSTYPE_WIN) || (type == OSTYPE_WINEFI) || (type == OSTYPE_EFI) || (type == OSTYPE_VAR))
#define OSTYPE_IS_LINUX(type) ((type == OSTYPE_LIN) || (type == OSTYPE_EFI) || (type == OSTYPE_VAR))
#define OSTYPE_IS_OTHER(type) ((type == OSTYPE_OTHER) || (type == OSTYPE_EFI) || (type == OSTYPE_VAR))
#define OSTYPE_COMPARE_IMP(comparator, type1, type2) (comparator(type1) && comparator(type2))
#define OSTYPE_COMPARE(type1, type2) (OSTYPE_COMPARE_IMP(OSTYPE_IS_OSX, type1, type2) || OSTYPE_COMPARE_IMP(OSTYPE_IS_OSX_RECOVERY, type1, type2) || \
OSTYPE_COMPARE_IMP(OSTYPE_IS_OSX_INSTALLER, type1, type2) || OSTYPE_COMPARE_IMP(OSTYPE_IS_WINDOWS, type1, type2) || \
OSTYPE_COMPARE_IMP(OSTYPE_IS_LINUX, type1, type2) || OSTYPE_COMPARE_IMP(OSTYPE_IS_OTHER, type1, type2))



#endif /* INCLUDE_OSTYPE_H_ */
