/*
kext injection
*/

#ifndef __KEXT_INJECT_H__
#define __KEXT_INJECT_H__

#include "LoaderUefi.h"
//#include "device_tree.h"
#include "kernel_patcher.h"

////////////////////
// defines
////////////////////
#define RoundPage(x)  ((((UINTN)(x)) + EFI_PAGE_SIZE - 1) & ~(EFI_PAGE_SIZE - 1))

#define KEXT_SIGNATURE SIGNATURE_32('M','O','S','X')

/*
 * Capability bits used in the definition of cpu_type.
 */
#define	CPU_ARCH_MASK	0xff000000		/* mask for architecture bits */
#define CPU_ARCH_ABI64	0x01000000		/* 64 bit ABI */

/*
 *	Machine types known by all.
 */
 
#define CPU_TYPE_ANY		((cpu_type_t) -1)

#define CPU_TYPE_VAX		((cpu_type_t) 1)
/* skip				((cpu_type_t) 2)	*/
/* skip				((cpu_type_t) 3)	*/
/* skip				((cpu_type_t) 4)	*/
/* skip				((cpu_type_t) 5)	*/
#define	CPU_TYPE_MC680x0	((cpu_type_t) 6)
#define CPU_TYPE_X86		((cpu_type_t) 7)
#define CPU_TYPE_I386		CPU_TYPE_X86		/* compatibility */
#define	CPU_TYPE_X86_64		(CPU_TYPE_X86 | CPU_ARCH_ABI64)

/* skip CPU_TYPE_MIPS		((cpu_type_t) 8)	*/
/* skip 			((cpu_type_t) 9)	*/
#define CPU_TYPE_MC98000	((cpu_type_t) 10)
#define CPU_TYPE_HPPA           ((cpu_type_t) 11)
#define CPU_TYPE_ARM		((cpu_type_t) 12)
#define CPU_TYPE_MC88000	((cpu_type_t) 13)
#define CPU_TYPE_SPARC		((cpu_type_t) 14)
#define CPU_TYPE_I860		((cpu_type_t) 15)
/* skip	CPU_TYPE_ALPHA		((cpu_type_t) 16)	*/
/* skip				((cpu_type_t) 17)	*/
#define CPU_TYPE_POWERPC		((cpu_type_t) 18)
#define CPU_TYPE_POWERPC64		(CPU_TYPE_POWERPC | CPU_ARCH_ABI64)

#define FAT_MAGIC	0xcafebabe
#define FAT_CIGAM	0xbebafeca	/* NXSwapLong(FAT_MAGIC) */

#define THIN_IA32 0xfeedface
#define THIN_X64  0xfeedfacf


////////////////////
// types
////////////////////
typedef struct 
{
	UINT32   magic;          /* FAT_MAGIC */
	UINT32   nfat_arch;      /* number of structs that follow */

} FAT_HEADER;

typedef struct 
{
	UINT32	cputype;        /* cpu specifier (int) */
	UINT32	cpusubtype;     /* machine specifier (int) */
	UINT32	offset;         /* file offset to this object file */
	UINT32	size;           /* size of this object file */
	UINT32	align;          /* alignment as a power of 2 */

} FAT_ARCH;

typedef struct
{
	UINT32				Signature;
	LIST_ENTRY			Link;
	_DeviceTreeBuffer	kext;
} KEXT_ENTRY;


////////////////////
// functions
////////////////////
class LOADER_ENTRY;
EFI_STATUS LoadKexts(IN LOADER_ENTRY *Entry);
EFI_STATUS InjectKexts(IN UINT32 deviceTreeP, IN UINT32* deviceTreeLength, LOADER_ENTRY *Entry);

VOID EFIAPI KernelBooterExtensionsPatch(IN UINT8 *KernelData, LOADER_ENTRY *Entry);

#endif