/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2013  Free Software Foundation, Inc.
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

#ifndef GRUB_XEN_HEADER
#define GRUB_XEN_HEADER 1

#define __XEN_INTERFACE_VERSION__ 0x0003020a

#ifdef ASM_FILE
#define __ASSEMBLY__
#include <xen/xen.h>
#else

#include <grub/symbol.h>
#include <grub/types.h>
#include <grub/err.h>

#ifndef GRUB_SYMBOL_GENERATOR
#include <stdint.h>
#include <xen/xen.h>

#include <xen/sched.h>
#include <xen/grant_table.h>
#include <xen/io/console.h>
#include <xen/io/xs_wire.h>
#include <xen/io/xenbus.h>
#include <xen/io/protocols.h>
#endif

#include <grub/cpu/xen/hypercall.h>

extern grub_size_t EXPORT_VAR (grub_xen_n_allocated_shared_pages);


#define GRUB_XEN_LOG_PAGE_SIZE 12
#define GRUB_XEN_PAGE_SIZE (1 << GRUB_XEN_LOG_PAGE_SIZE)

extern volatile struct xencons_interface *grub_xen_xcons;
extern volatile struct shared_info *EXPORT_VAR (grub_xen_shared_info);
extern volatile struct xenstore_domain_interface *grub_xen_xenstore;
extern volatile grant_entry_v1_t *grub_xen_grant_table;

void EXPORT_FUNC (grub_xen_store_send) (const void *buf_, grub_size_t len);
void EXPORT_FUNC (grub_xen_store_recv) (void *buf_, grub_size_t len);
grub_err_t
EXPORT_FUNC (grub_xenstore_dir) (const char *dir,
				 int (*hook) (const char *dir,
					      void *hook_data),
				 void *hook_data);
void *EXPORT_FUNC (grub_xenstore_get_file) (const char *dir,
					    grub_size_t * len);
grub_err_t EXPORT_FUNC (grub_xenstore_write_file) (const char *dir,
						   const void *buf,
						   grub_size_t len);

typedef unsigned int grub_xen_grant_t;

void *EXPORT_FUNC (grub_xen_alloc_shared_page) (domid_t dom,
						grub_xen_grant_t * grnum);
void EXPORT_FUNC (grub_xen_free_shared_page) (void *ptr);

#define mb() asm volatile("mfence;sfence;" : : : "memory");
extern struct start_info *EXPORT_VAR (grub_xen_start_page_addr);

void grub_console_init (void);

void grub_xendisk_fini (void);
void grub_xendisk_init (void);

#ifdef __x86_64__
typedef grub_uint64_t grub_xen_mfn_t;
#else
typedef grub_uint32_t grub_xen_mfn_t;
#endif
typedef unsigned int grub_xen_evtchn_t;
#endif

#endif
