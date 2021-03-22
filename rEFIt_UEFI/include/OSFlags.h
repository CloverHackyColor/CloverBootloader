/*
 * OSFlags.h
 *
 *  Created on: Feb 4, 2021
 *      Author: jief
 */

#ifndef INCLUDE_OSFLAGS_H_
#define INCLUDE_OSFLAGS_H_



#define OSFLAG_ISSET(flags, flag) ((flags & flag) == flag)
#define OSFLAG_ISUNSET(flags, flag) ((flags & flag) != flag)
#define OSFLAG_SET(flags, flag) (flags | flag)
#define OSFLAG_UNSET(flags, flag) (flags & (~flag))
#define OSFLAG_TOGGLE(flags, flag) (flags ^ flag)
#define OSFLAG_USEGRAPHICS    (1 << 0)
//#define OSFLAG_WITHKEXTS      (1 << 1)  // Jief not used, maybe sincee OC integration
//#define OSFLAG_CHECKFAKESMC   (1 << 2)  // Jief : not used since 4202, I think
#define OSFLAG_NOCACHES       (1 << 3)
#define OSFLAG_NODEFAULTARGS  (1 << 4)
#define OSFLAG_NODEFAULTMENU  (1 << 5)
//#define OSFLAG_HIDDEN         (1 << 6)
#define OSFLAG_DISABLED       (1 << 7)
#define OSFLAG_HIBERNATED     (1 << 8)
#define OSFLAG_NOSIP          (1 << 9)



#endif /* INCLUDE_OSFLAGS_H_ */
