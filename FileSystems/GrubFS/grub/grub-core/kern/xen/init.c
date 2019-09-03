/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2011  Free Software Foundation, Inc.
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

#include <grub/xen.h>
#include <grub/term.h>
#include <grub/misc.h>
#include <grub/env.h>
#include <grub/mm.h>
#include <grub/kernel.h>
#include <grub/offsets.h>
#include <grub/memory.h>
#include <grub/i386/tsc.h>
#include <grub/term.h>
#include <grub/loader.h>

grub_addr_t grub_modbase;
struct start_info *grub_xen_start_page_addr;
volatile struct xencons_interface *grub_xen_xcons;
volatile struct shared_info *grub_xen_shared_info;
volatile struct xenstore_domain_interface *grub_xen_xenstore;
volatile grant_entry_v1_t *grub_xen_grant_table;
static const grub_size_t total_grants =
  GRUB_XEN_PAGE_SIZE / sizeof (grub_xen_grant_table[0]);
grub_size_t grub_xen_n_allocated_shared_pages;

static grub_xen_mfn_t
grub_xen_ptr2mfn (void *ptr)
{
  grub_xen_mfn_t *mfn_list =
    (grub_xen_mfn_t *) grub_xen_start_page_addr->mfn_list;
  return mfn_list[(grub_addr_t) ptr >> GRUB_XEN_LOG_PAGE_SIZE];
}

void *
grub_xen_alloc_shared_page (domid_t dom, grub_xen_grant_t * grnum)
{
  void *ret;
  grub_xen_mfn_t mfn;
  volatile grant_entry_v1_t *entry;

  /* Avoid 0.  */
  for (entry = grub_xen_grant_table;
       entry < grub_xen_grant_table + total_grants; entry++)
    if (!entry->flags)
      break;

  if (entry == grub_xen_grant_table + total_grants)
    {
      grub_error (GRUB_ERR_OUT_OF_MEMORY, "out of grant entries");
      return NULL;
    }
  ret = grub_memalign (GRUB_XEN_PAGE_SIZE, GRUB_XEN_PAGE_SIZE);
  if (!ret)
    return NULL;
  mfn = grub_xen_ptr2mfn (ret);
  entry->frame = mfn;
  entry->domid = dom;
  mb ();
  entry->flags = GTF_permit_access;
  mb ();
  *grnum = entry - grub_xen_grant_table;
  grub_xen_n_allocated_shared_pages++;
  return ret;
}

void
grub_xen_free_shared_page (void *ptr)
{
  grub_xen_mfn_t mfn;
  volatile grant_entry_v1_t *entry;

  mfn = grub_xen_ptr2mfn (ptr);
  for (entry = grub_xen_grant_table + 1;
       entry < grub_xen_grant_table + total_grants; entry++)
    if (entry->flags && entry->frame == mfn)
      {
	mb ();
	entry->flags = 0;
	mb ();
	entry->frame = 0;
	mb ();
      }
  grub_xen_n_allocated_shared_pages--;
}

void
grub_machine_get_bootlocation (char **device __attribute__ ((unused)),
			       char **path __attribute__ ((unused)))
{
}

static grub_uint8_t window[GRUB_XEN_PAGE_SIZE]
  __attribute__ ((aligned (GRUB_XEN_PAGE_SIZE)));

#ifdef __x86_64__
#define NUMBER_OF_LEVELS 4
#else
#define NUMBER_OF_LEVELS 3
#endif

#define LOG_POINTERS_PER_PAGE 9
#define POINTERS_PER_PAGE (1 << LOG_POINTERS_PER_PAGE)

void
grub_xen_store_send (const void *buf_, grub_size_t len)
{
  const grub_uint8_t *buf = buf_;
  struct evtchn_send send;
  int event_sent = 0;
  while (len)
    {
      grub_size_t avail, inbuf;
      grub_size_t prod, cons;
      mb ();
      prod = grub_xen_xenstore->req_prod;
      cons = grub_xen_xenstore->req_cons;
      if (prod >= cons + sizeof (grub_xen_xenstore->req))
	{
	  if (!event_sent)
	    {
	      send.port = grub_xen_start_page_addr->store_evtchn;
	      grub_xen_event_channel_op (EVTCHNOP_send, &send);
	      event_sent = 1;
	    }
	  grub_xen_sched_op (SCHEDOP_yield, 0);
	  continue;
	}
      event_sent = 0;
      avail = cons + sizeof (grub_xen_xenstore->req) - prod;
      inbuf = (~prod & (sizeof (grub_xen_xenstore->req) - 1)) + 1;
      if (avail > inbuf)
	avail = inbuf;
      if (avail > len)
	avail = len;
      grub_memcpy ((void *) &grub_xen_xenstore->req[prod & (sizeof (grub_xen_xenstore->req) - 1)],
		   buf, avail);
      buf += avail;
      len -= avail;
      mb ();
      grub_xen_xenstore->req_prod += avail;
      mb ();
      if (!event_sent)
	{
	  send.port = grub_xen_start_page_addr->store_evtchn;
	  grub_xen_event_channel_op (EVTCHNOP_send, &send);
	  event_sent = 1;
	}
      grub_xen_sched_op (SCHEDOP_yield, 0);
    }
}

void
grub_xen_store_recv (void *buf_, grub_size_t len)
{
  grub_uint8_t *buf = buf_;
  struct evtchn_send send;
  int event_sent = 0;
  while (len)
    {
      grub_size_t avail, inbuf;
      grub_size_t prod, cons;
      mb ();
      prod = grub_xen_xenstore->rsp_prod;
      cons = grub_xen_xenstore->rsp_cons;
      if (prod <= cons)
	{
	  if (!event_sent)
	    {
	      send.port = grub_xen_start_page_addr->store_evtchn;
	      grub_xen_event_channel_op (EVTCHNOP_send, &send);
	      event_sent = 1;
	    }
	  grub_xen_sched_op (SCHEDOP_yield, 0);
	  continue;
	}
      event_sent = 0;
      avail = prod - cons;
      inbuf = (~cons & (sizeof (grub_xen_xenstore->req) - 1)) + 1;
      if (avail > inbuf)
	avail = inbuf;
      if (avail > len)
	avail = len;
      grub_memcpy (buf,
		   (void *) &grub_xen_xenstore->rsp[cons & (sizeof (grub_xen_xenstore->rsp) - 1)],
		   avail);
      buf += avail;
      len -= avail;
      mb ();
      grub_xen_xenstore->rsp_cons += avail;
      mb ();
      if (!event_sent)
	{
	  send.port = grub_xen_start_page_addr->store_evtchn;
	  grub_xen_event_channel_op(EVTCHNOP_send, &send);
	  event_sent = 1;
	}
      grub_xen_sched_op(SCHEDOP_yield, 0);
    }
}

void *
grub_xenstore_get_file (const char *dir, grub_size_t *len)
{
  struct xsd_sockmsg msg;
  char *buf;
  grub_size_t dirlen = grub_strlen (dir) + 1;

  if (len)
    *len = 0;

  grub_memset (&msg, 0, sizeof (msg));
  msg.type = XS_READ;
  msg.len = dirlen;
  grub_xen_store_send (&msg, sizeof (msg));
  grub_xen_store_send (dir, dirlen);
  grub_xen_store_recv (&msg, sizeof (msg));
  buf = grub_malloc (msg.len + 1);
  if (!buf)
    return NULL;
  grub_dprintf ("xen", "msg type = %d, len = %d\n", msg.type, msg.len);
  grub_xen_store_recv (buf, msg.len);
  buf[msg.len] = '\0';
  if (msg.type == XS_ERROR)
    {
      grub_error (GRUB_ERR_IO, "couldn't read xenstorage `%s': %s", dir, buf);
      grub_free (buf);
      return NULL;
    }
  if (len)
    *len = msg.len;
  return buf;
}

grub_err_t
grub_xenstore_write_file (const char *dir, const void *buf, grub_size_t len)
{
  struct xsd_sockmsg msg;
  grub_size_t dirlen = grub_strlen (dir) + 1;
  char *resp;

  grub_memset (&msg, 0, sizeof (msg));
  msg.type = XS_WRITE;
  msg.len = dirlen + len;
  grub_xen_store_send (&msg, sizeof (msg));
  grub_xen_store_send (dir, dirlen);
  grub_xen_store_send (buf, len);
  grub_xen_store_recv (&msg, sizeof (msg));
  resp = grub_malloc (msg.len + 1);
  if (!resp)
    return grub_errno;
  grub_dprintf ("xen", "msg type = %d, len = %d\n", msg.type, msg.len);
  grub_xen_store_recv (resp, msg.len);
  resp[msg.len] = '\0';
  if (msg.type == XS_ERROR)
    {
      grub_dprintf ("xen", "error = %s\n", resp);
      grub_error (GRUB_ERR_IO, "couldn't read xenstorage `%s': %s",
		  dir, resp);
      grub_free (resp);
      return grub_errno;
    }
  grub_free (resp);
  return GRUB_ERR_NONE;
}

/* FIXME: error handling.  */
grub_err_t
grub_xenstore_dir (const char *dir,
		   int (*hook) (const char *dir, void *hook_data),
		   void *hook_data)
{
  struct xsd_sockmsg msg;
  char *buf;
  char *ptr;
  grub_size_t dirlen = grub_strlen (dir) + 1;

  grub_memset (&msg, 0, sizeof (msg));
  msg.type = XS_DIRECTORY;
  msg.len = dirlen;
  grub_xen_store_send (&msg, sizeof (msg));
  grub_xen_store_send (dir, dirlen);
  grub_xen_store_recv (&msg, sizeof (msg));
  buf = grub_malloc (msg.len + 1);
  if (!buf)
    return grub_errno;
  grub_dprintf ("xen", "msg type = %d, len = %d\n", msg.type, msg.len);
  grub_xen_store_recv (buf, msg.len);
  buf[msg.len] = '\0';
  if (msg.type == XS_ERROR)
    {
      grub_err_t err;
      err = grub_error (GRUB_ERR_IO, "couldn't read xenstorage `%s': %s",
			dir, buf);
      grub_free (buf);
      return err;
    }
  for (ptr = buf; ptr < buf + msg.len; ptr += grub_strlen (ptr) + 1)
    if (hook (ptr, hook_data))
      break;
  grub_free (buf);
  return grub_errno;
}

unsigned long gntframe = 0;

#define MAX_N_UNUSABLE_PAGES 4

static int
grub_xen_is_page_usable (grub_xen_mfn_t mfn)
{
  if (mfn == grub_xen_start_page_addr->console.domU.mfn)
    return 0;
  if (mfn == grub_xen_start_page_addr->shared_info)
    return 0;
  if (mfn == grub_xen_start_page_addr->store_mfn)
    return 0;
  if (mfn == gntframe)
    return 0;
  return 1;
}

static grub_uint64_t
page2offset (grub_uint64_t page)
{
  return page << 12;
}

#if defined (__x86_64__) && defined (__code_model_large__)
#define MAX_TOTAL_PAGES (1LL << (64 - 12))
#elif defined (__x86_64__)
#define MAX_TOTAL_PAGES (1LL << (31 - 12))
#else
#define MAX_TOTAL_PAGES (1LL << (32 - 12))
#endif

static void
map_all_pages (void)
{
  grub_uint64_t total_pages = grub_xen_start_page_addr->nr_pages;
  grub_uint64_t i, j;
  grub_xen_mfn_t *mfn_list =
    (grub_xen_mfn_t *) grub_xen_start_page_addr->mfn_list;
  grub_uint64_t *pg = (grub_uint64_t *) window;
  grub_uint64_t oldpgstart, oldpgend;
  struct gnttab_setup_table gnttab_setup;
  struct gnttab_set_version gnttab_setver;
  grub_size_t n_unusable_pages = 0;
  struct mmu_update m2p_updates[2 * MAX_N_UNUSABLE_PAGES];

  if (total_pages > MAX_TOTAL_PAGES - 4)
    total_pages = MAX_TOTAL_PAGES - 4;

  grub_memset (&gnttab_setver, 0, sizeof (gnttab_setver));

  gnttab_setver.version = 1;
  grub_xen_grant_table_op (GNTTABOP_set_version, &gnttab_setver, 1);

  grub_memset (&gnttab_setup, 0, sizeof (gnttab_setup));
  gnttab_setup.dom = DOMID_SELF;
  gnttab_setup.nr_frames = 1;
  gnttab_setup.frame_list.p = &gntframe;

  grub_xen_grant_table_op (GNTTABOP_setup_table, &gnttab_setup, 1);

  for (j = 0; j < total_pages - n_unusable_pages; j++)
    while (!grub_xen_is_page_usable (mfn_list[j]))
      {
	grub_xen_mfn_t t;
	if (n_unusable_pages >= MAX_N_UNUSABLE_PAGES)
	  {
	    struct sched_shutdown arg;
	    arg.reason = SHUTDOWN_crash;
	    grub_xen_sched_op (SCHEDOP_shutdown, &arg);
	    while (1);
	  }
	t = mfn_list[j];
	mfn_list[j] = mfn_list[total_pages - n_unusable_pages - 1];
	mfn_list[total_pages - n_unusable_pages - 1] = t;

	m2p_updates[2 * n_unusable_pages].ptr
	  = page2offset (mfn_list[j]) | MMU_MACHPHYS_UPDATE;
	m2p_updates[2 * n_unusable_pages].val = j;
	m2p_updates[2 * n_unusable_pages + 1].ptr
	  = page2offset (mfn_list[total_pages - n_unusable_pages - 1])
	  | MMU_MACHPHYS_UPDATE;
	m2p_updates[2 * n_unusable_pages + 1].val = total_pages
	  - n_unusable_pages - 1;

	n_unusable_pages++;
      }

  grub_xen_mmu_update (m2p_updates, 2 * n_unusable_pages, NULL, DOMID_SELF);

  total_pages += 4;

  grub_uint64_t lx[NUMBER_OF_LEVELS], nlx;
  grub_uint64_t paging_start = total_pages - 4 - n_unusable_pages, curpage;

  for (nlx = total_pages, i = 0; i < (unsigned) NUMBER_OF_LEVELS; i++)
    {
      nlx = (nlx + POINTERS_PER_PAGE - 1) >> LOG_POINTERS_PER_PAGE;
      /* PAE wants all 4 root directories present.  */
#ifdef __i386__
      if (i == 1)
	nlx = 4;
#endif
      lx[i] = nlx;
      paging_start -= nlx;
    }

  oldpgstart = grub_xen_start_page_addr->pt_base >> 12;
  oldpgend = oldpgstart + grub_xen_start_page_addr->nr_pt_frames;

  curpage = paging_start;

  int l;

  for (l = NUMBER_OF_LEVELS - 1; l >= 1; l--)
    {
      for (i = 0; i < lx[l]; i++)
	{
	  grub_xen_update_va_mapping (&window,
				      page2offset (mfn_list[curpage + i]) | 7,
				      UVMF_INVLPG);
	  grub_memset (&window, 0, sizeof (window));

	  for (j = i * POINTERS_PER_PAGE;
	       j < (i + 1) * POINTERS_PER_PAGE && j < lx[l - 1]; j++)
	    pg[j - i * POINTERS_PER_PAGE] =
	      page2offset (mfn_list[curpage + lx[l] + j])
#ifdef __x86_64__
	      | 4
#endif
	      | 3;
	}
      curpage += lx[l];
    }

  for (i = 0; i < lx[0]; i++)
    {
      grub_xen_update_va_mapping (&window,
				  page2offset (mfn_list[curpage + i]) | 7,
				  UVMF_INVLPG);
      grub_memset (&window, 0, sizeof (window));

      for (j = i * POINTERS_PER_PAGE;
	   j < (i + 1) * POINTERS_PER_PAGE && j < total_pages; j++)
	if (j < paging_start && !(j >= oldpgstart && j < oldpgend))
	  pg[j - i * POINTERS_PER_PAGE] = page2offset (mfn_list[j]) | 0x7;
	else if (j < grub_xen_start_page_addr->nr_pages)
	  pg[j - i * POINTERS_PER_PAGE] = page2offset (mfn_list[j]) | 5;
	else if (j == grub_xen_start_page_addr->nr_pages)
	  {
	    pg[j - i * POINTERS_PER_PAGE] =
	      page2offset (grub_xen_start_page_addr->console.domU.mfn) | 7;
	    grub_xen_xcons = (void *) (grub_addr_t) page2offset (j);
	  }
	else if (j == grub_xen_start_page_addr->nr_pages + 1)
	  {
	    pg[j - i * POINTERS_PER_PAGE] =
	      grub_xen_start_page_addr->shared_info | 7;
	    grub_xen_shared_info = (void *) (grub_addr_t) page2offset (j);
	  }
	else if (j == grub_xen_start_page_addr->nr_pages + 2)
	  {
	    pg[j - i * POINTERS_PER_PAGE] =
	      page2offset (grub_xen_start_page_addr->store_mfn) | 7;
	    grub_xen_xenstore = (void *) (grub_addr_t) page2offset (j);
	  }
	else if (j == grub_xen_start_page_addr->nr_pages + 3)
	  {
	    pg[j - i * POINTERS_PER_PAGE] = page2offset (gntframe) | 7;
	    grub_xen_grant_table = (void *) (grub_addr_t) page2offset (j);
	  }
    }

  grub_xen_update_va_mapping (&window, 0, UVMF_INVLPG);

  mmuext_op_t op[3];

  op[0].cmd = MMUEXT_PIN_L1_TABLE + (NUMBER_OF_LEVELS - 1);
  op[0].arg1.mfn = mfn_list[paging_start];
  op[1].cmd = MMUEXT_NEW_BASEPTR;
  op[1].arg1.mfn = mfn_list[paging_start];
  op[2].cmd = MMUEXT_UNPIN_TABLE;
  op[2].arg1.mfn = mfn_list[oldpgstart];

  grub_xen_mmuext_op (op, 3, NULL, DOMID_SELF);

  for (i = oldpgstart; i < oldpgend; i++)
    grub_xen_update_va_mapping ((void *) (grub_addr_t) page2offset (i),
				page2offset (mfn_list[i]) | 7, UVMF_INVLPG);
  void *new_start_page, *new_mfn_list;
  new_start_page = (void *) (grub_addr_t) page2offset (paging_start - 1);
  grub_memcpy (new_start_page, grub_xen_start_page_addr, 4096);
  grub_xen_start_page_addr = new_start_page;
  new_mfn_list = (void *) (grub_addr_t)
    page2offset (paging_start - 1
		 - ((grub_xen_start_page_addr->nr_pages
		     * sizeof (grub_uint64_t) + 4095) / 4096));
  grub_memcpy (new_mfn_list, mfn_list, grub_xen_start_page_addr->nr_pages
	       * sizeof (grub_uint64_t));
  grub_xen_start_page_addr->pt_base = page2offset (paging_start);
  grub_xen_start_page_addr->mfn_list = (grub_addr_t) new_mfn_list;

  grub_addr_t heap_start = grub_modules_get_end ();
  grub_addr_t heap_end = (grub_addr_t) new_mfn_list;

  grub_mm_init_region ((void *) heap_start, heap_end - heap_start);
}

extern char _end[];

void
grub_machine_init (void)
{
#ifdef __i386__
  grub_xen_vm_assist (VMASST_CMD_enable, VMASST_TYPE_pae_extended_cr3);
#endif

  grub_modbase = ALIGN_UP ((grub_addr_t) _end
			   + GRUB_KERNEL_MACHINE_MOD_GAP,
			   GRUB_KERNEL_MACHINE_MOD_ALIGN);

  map_all_pages ();

  grub_console_init ();

  grub_tsc_init ();

  grub_xendisk_init ();

  grub_boot_init ();
}

void
grub_exit (void)
{
  struct sched_shutdown arg;

  arg.reason = SHUTDOWN_poweroff;
  grub_xen_sched_op (SCHEDOP_shutdown, &arg);
  while (1);
}

void
grub_machine_fini (int flags __attribute__ ((unused)))
{
  grub_xendisk_fini ();
  grub_boot_fini ();
}

grub_err_t
grub_machine_mmap_iterate (grub_memory_hook_t hook, void *hook_data)
{
  grub_uint64_t total_pages = grub_xen_start_page_addr->nr_pages;
  grub_uint64_t usable_pages = grub_xen_start_page_addr->pt_base >> 12;
  if (hook (0, page2offset (usable_pages), GRUB_MEMORY_AVAILABLE, hook_data))
    return GRUB_ERR_NONE;

  hook (page2offset (usable_pages), page2offset (total_pages - usable_pages),
	GRUB_MEMORY_RESERVED, hook_data);

  return GRUB_ERR_NONE;
}
