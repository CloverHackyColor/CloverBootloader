/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2009  Free Software Foundation, Inc.
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

#ifndef GRUB_CPU_CPUID_HEADER
#define GRUB_CPU_CPUID_HEADER 1

extern unsigned char grub_cpuid_has_longmode;
extern unsigned char grub_cpuid_has_pae;

#ifdef __x86_64__

static __inline int
grub_cpu_is_cpuid_supported (void)
{
  grub_uint64_t id_supported;

  __asm__ ("pushfq\n\t"
           "popq %%rax             /* Get EFLAGS into EAX */\n\t"
           "movq %%rax, %%rcx      /* Save original flags in ECX */\n\t"
           "xorq $0x200000, %%rax  /* Flip ID bit in EFLAGS */\n\t"
           "pushq %%rax            /* Store modified EFLAGS on stack */\n\t"
           "popfq                  /* Replace current EFLAGS */\n\t"
           "pushfq                 /* Read back the EFLAGS */\n\t"
           "popq %%rax             /* Get EFLAGS into EAX */\n\t"
           "xorq %%rcx, %%rax      /* Check if flag could be modified */\n\t"
           : "=a" (id_supported)
           : /* No inputs.  */
           : /* Clobbered:  */ "%rcx");

  return id_supported != 0;
}

#else

static __inline int
grub_cpu_is_cpuid_supported (void)
{
  grub_uint32_t id_supported;

  __asm__ ("pushfl\n\t"
           "popl %%eax             /* Get EFLAGS into EAX */\n\t"
           "movl %%eax, %%ecx      /* Save original flags in ECX */\n\t"
           "xorl $0x200000, %%eax  /* Flip ID bit in EFLAGS */\n\t"
           "pushl %%eax            /* Store modified EFLAGS on stack */\n\t"
           "popfl                  /* Replace current EFLAGS */\n\t"
           "pushfl                 /* Read back the EFLAGS */\n\t"
           "popl %%eax             /* Get EFLAGS into EAX */\n\t"
           "xorl %%ecx, %%eax      /* Check if flag could be modified */\n\t"
           : "=a" (id_supported)
           : /* No inputs.  */
           : /* Clobbered:  */ "%rcx");

  return id_supported != 0;
}

#endif

#ifdef __PIC__
#define grub_cpuid(num,a,b,c,d) \
  asm volatile ("xchgl %%ebx, %1; cpuid; xchgl %%ebx, %1" \
                : "=a" (a), "=r" (b), "=c" (c), "=d" (d)  \
                : "0" (num))
#else
#define grub_cpuid(num,a,b,c,d) \
  asm volatile ("cpuid" \
                : "=a" (a), "=b" (b), "=c" (c), "=d" (d)  \
                : "0" (num))
#endif

#endif
