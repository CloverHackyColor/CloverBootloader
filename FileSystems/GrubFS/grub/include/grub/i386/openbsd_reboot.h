/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2008,2009  Free Software Foundation, Inc.
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

/*      $OpenBSD: reboot.h,v 1.13 2004/03/10 23:02:53 tom Exp $ */
/*      $NetBSD: reboot.h,v 1.9 1996/04/22 01:23:25 christos Exp $      */

/*
 * Copyright (c) 1982, 1986, 1988, 1993, 1994
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *      @(#)reboot.h    8.2 (Berkeley) 7/10/94
 */

#ifndef GRUB_OPENBSD_REBOOT_CPU_HEADER
#define GRUB_OPENBSD_REBOOT_CPU_HEADER	1

#define OPENBSD_RB_ASKNAME	(1 << 0)  /* ask for file name to reboot from */
#define OPENBSD_RB_SINGLE	(1 << 1)  /* reboot to single user only */
#define OPENBSD_RB_NOSYNC	(1 << 2)  /* dont sync before reboot */
#define OPENBSD_RB_HALT		(1 << 3)  /* don't reboot, just halt */
#define OPENBSD_RB_INITNAME	(1 << 4)  /* name given for /etc/init (unused) */
#define OPENBSD_RB_DFLTROOT	(1 << 5)  /* use compiled-in rootdev */
#define OPENBSD_RB_KDB		(1 << 6)  /* give control to kernel debugger */
#define OPENBSD_RB_RDONLY	(1 << 7)  /* mount root fs read-only */
#define OPENBSD_RB_DUMP		(1 << 8)  /* dump kernel memory before reboot */
#define OPENBSD_RB_MINIROOT	(1 << 9)  /* mini-root present in memory at boot time */
#define OPENBSD_RB_CONFIG	(1 << 10) /* change configured devices */
#define OPENBSD_RB_TIMEBAD	(1 << 11) /* don't call resettodr() in boot() */
#define OPENBSD_RB_POWERDOWN	(1 << 12) /* attempt to power down machine */
#define OPENBSD_RB_SERCONS	(1 << 13) /* use serial console if available */
#define OPENBSD_RB_USERREQ	(1 << 14) /* boot() called at user request (e.g. ddb) */

#define OPENBSD_B_DEVMAGIC	0xa0000000
#define OPENBSD_B_ADAPTORSHIFT	24
#define OPENBSD_B_CTRLSHIFT	20
#define OPENBSD_B_UNITSHIFT	16
#define OPENBSD_B_PARTSHIFT	8
#define OPENBSD_B_TYPESHIFT	0

#endif
