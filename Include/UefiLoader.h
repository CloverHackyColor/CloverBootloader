/*
 * dmazar: UEFI wrapper for Mach-O definitions
 * xnu/EXTERNAL_HEADERS/mach-o/loader.h
 *
 */

#ifndef _UEFI_MACHO_LOADER_H_
#define _UEFI_MACHO_LOADER_H_

//
// Base UEFI types
//
#include <Base.h>

//
// From xnu/EXTERNAL_HEADERS/stdint.h:
//  typedef u_int8_t              uint8_t;   /* u_int8_t is defined in <machine/types.h> */
//  typedef u_int16_t            uint16_t;   /* u_int16_t is defined in <machine/types.h> */
//  typedef u_int32_t            uint32_t;   /* u_int32_t is defined in <machine/types.h> */
//  typedef u_int64_t            uint64_t;   /* u_int64_t is defined in <machine/types.h> */
//

typedef UINT8             uint8_t;
typedef UINT16            uint16_t;
typedef UINT32            uint32_t;
typedef UINT64            uint64_t;

//
// From xnu/osfmk/mach/machine.h:
//  typedef integer_t	cpu_type_t;
//  typedef integer_t	cpu_subtype_t;
//
// From xnu/osfmk/mach/i386/vm_types.h:
//  typedef int			integer_t;
//
typedef INT32	cpu_type_t;
typedef INT32	cpu_subtype_t;

//
// From xnu/osfmk/mach/vm_prot.h:
// typedef int		vm_prot_t;
typedef INT32		vm_prot_t;

//
// And finally the xnu/EXTERNAL_HEADERS/mach-o/loader.h
//
//#include "loader.h"
#include <IndustryStandard/MachO-loader.h>

//
// Additionally, only needed thread state definitions for LC_UNIXTHREAD
// are included here to avoid more header files.
//

//
// From xnu/bsd/i386/types.h:
//  typedef unsigned long long	__uint64_t;
//
typedef UINT64	__uint64_t;

//
// From xnu/osfmk/mach/i386/_structs.h:
//

//
//#define	_STRUCT_X86_THREAD_STATE32	struct __darwin_i386_thread_state
//_STRUCT_X86_THREAD_STATE32
//{
//    // all fields are unsigned int in xnu rources
//	  UINT32	eax;
//    UINT32	ebx;
//    UINT32	ecx;
//    UINT32	edx;
//    UINT32	edi;
//    UINT32	esi;
//    UINT32	ebp;
//    UINT32	esp;
//    UINT32	ss;
//    UINT32	eflags;
//    UINT32	eip;
//    UINT32	cs;
//    UINT32	ds;
//    UINT32	es;
//    UINT32	fs;
//    UINT32	gs;
//};
//
//#define	_STRUCT_X86_THREAD_STATE64	struct __darwin_x86_thread_state64
//_STRUCT_X86_THREAD_STATE64
//{
//	__uint64_t	rax;
//	__uint64_t	rbx;
//	__uint64_t	rcx;
//	__uint64_t	rdx;
//	__uint64_t	rdi;
//	__uint64_t	rsi;
//	__uint64_t	rbp;
//	__uint64_t	rsp;
//	__uint64_t	r8;
//	__uint64_t	r9;
//	__uint64_t	r10;
//	__uint64_t	r11;
//	__uint64_t	r12;
//	__uint64_t	r13;
//	__uint64_t	r14;
//	__uint64_t	r15;
//	__uint64_t	rip;
//	__uint64_t	rflags;
//	__uint64_t	cs;
//	__uint64_t	fs;
//	__uint64_t	gs;
//};
//
////
//// From xnu/osfmk/mach/i386/thread_status.h:
////
//typedef _STRUCT_X86_THREAD_STATE32 i386_thread_state_t;
//typedef _STRUCT_X86_THREAD_STATE64 x86_thread_state64_t;
//


#endif /* _UEFI_MACHO_LOADER_H_ */
