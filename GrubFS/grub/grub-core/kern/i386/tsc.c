/* kern/i386/tsc.c - x86 TSC time source implementation
 * Requires Pentium or better x86 CPU that supports the RDTSC instruction.
 * This module uses the RTC (via grub_get_rtc()) to calibrate the TSC to
 * real time.
 *
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2008  Free Software Foundation, Inc.
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

#include <grub/types.h>
#include <grub/time.h>
#include <grub/misc.h>
#include <grub/i386/tsc.h>
#include <grub/i386/cpuid.h>
#ifdef GRUB_MACHINE_XEN
#include <grub/xen.h>
#else
#include <grub/i386/pit.h>
#endif
#include <grub/cpu/io.h>

/* This defines the value TSC had at the epoch (that is, when we calibrated it). */
static grub_uint64_t tsc_boot_time;

/* Calibrated TSC rate.  (In ms per 2^32 ticks) */
/* We assume that the tick is less than 1 ms and hence this value fits
   in 32-bit.  */
grub_uint32_t grub_tsc_rate;

/* Read the TSC value, which increments with each CPU clock cycle. */
static __inline grub_uint64_t
grub_get_tsc (void)
{
  grub_uint32_t lo, hi;
  grub_uint32_t a,b,c,d;

  /* The CPUID instruction is a 'serializing' instruction, and
     avoids out-of-order execution of the RDTSC instruction. */
  grub_cpuid (0,a,b,c,d);
  /* Read TSC value.  We cannot use "=A", since this would use
     %rax on x86_64. */
  __asm__ __volatile__ ("rdtsc":"=a" (lo), "=d" (hi));

  return (((grub_uint64_t) hi) << 32) | lo;
}

static __inline int
grub_cpu_is_tsc_supported (void)
{
  grub_uint32_t a,b,c,d;
  if (! grub_cpu_is_cpuid_supported ())
    return 0;

  grub_cpuid(1,a,b,c,d);

  return (d & (1 << 4)) != 0;
}

#ifndef GRUB_MACHINE_XEN

static void
grub_pit_wait (grub_uint16_t tics)
{
  /* Disable timer2 gate and speaker.  */
  grub_outb (grub_inb (GRUB_PIT_SPEAKER_PORT)
	     & ~ (GRUB_PIT_SPK_DATA | GRUB_PIT_SPK_TMR2),
             GRUB_PIT_SPEAKER_PORT);

  /* Set tics.  */
  grub_outb (GRUB_PIT_CTRL_SELECT_2 | GRUB_PIT_CTRL_READLOAD_WORD,
	     GRUB_PIT_CTRL);
  grub_outb (tics & 0xff, GRUB_PIT_COUNTER_2);
  grub_outb (tics >> 8, GRUB_PIT_COUNTER_2);

  /* Enable timer2 gate, keep speaker disabled.  */
  grub_outb ((grub_inb (GRUB_PIT_SPEAKER_PORT) & ~ GRUB_PIT_SPK_DATA)
	     | GRUB_PIT_SPK_TMR2,
             GRUB_PIT_SPEAKER_PORT);

  /* Wait.  */
  while ((grub_inb (GRUB_PIT_SPEAKER_PORT) & GRUB_PIT_SPK_TMR2_LATCH) == 0x00);

  /* Disable timer2 gate and speaker.  */
  grub_outb (grub_inb (GRUB_PIT_SPEAKER_PORT)
	     & ~ (GRUB_PIT_SPK_DATA | GRUB_PIT_SPK_TMR2),
             GRUB_PIT_SPEAKER_PORT);
}
#endif

static grub_uint64_t
grub_tsc_get_time_ms (void)
{
  grub_uint64_t a = grub_get_tsc () - tsc_boot_time;
  grub_uint64_t ah = a >> 32;
  grub_uint64_t al = a & 0xffffffff;

  return ((al * grub_tsc_rate) >> 32) + ah * grub_tsc_rate;
}

#ifndef GRUB_MACHINE_XEN
/* Calibrate the TSC based on the RTC.  */
static void
calibrate_tsc (void)
{
  /* First calibrate the TSC rate (relative, not absolute time). */
  grub_uint64_t end_tsc;

  tsc_boot_time = grub_get_tsc ();
  grub_pit_wait (0xffff);
  end_tsc = grub_get_tsc ();

  grub_tsc_rate = grub_divmod64 ((55ULL << 32), end_tsc - tsc_boot_time, 0);
}
#endif

void
grub_tsc_init (void)
{
#ifdef GRUB_MACHINE_XEN
  grub_uint64_t t;
  tsc_boot_time = grub_get_tsc ();
  t = grub_xen_shared_info->vcpu_info[0].time.tsc_to_system_mul;
  if (grub_xen_shared_info->vcpu_info[0].time.tsc_shift > 0)
    t <<= grub_xen_shared_info->vcpu_info[0].time.tsc_shift;
  else
    t >>= -grub_xen_shared_info->vcpu_info[0].time.tsc_shift;
  grub_tsc_rate = grub_divmod64 (t, 1000000, 0);
  grub_install_get_time_ms (grub_tsc_get_time_ms);
#else
  if (grub_cpu_is_tsc_supported ())
    {
      calibrate_tsc ();
      grub_install_get_time_ms (grub_tsc_get_time_ms);
    }
  else
    {
#if defined (GRUB_MACHINE_PCBIOS) || defined (GRUB_MACHINE_IEEE1275)
      grub_install_get_time_ms (grub_rtc_get_time_ms);
#else
      grub_fatal ("no TSC found");
#endif
    }
#endif
}
