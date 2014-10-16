#include <grub/dl.h>
#include <grub/cache.h>
#include <grub/arm/system.h>
#ifdef GRUB_MACHINE_UBOOT
#include <grub/uboot/uboot.h>
#include <grub/uboot/api_public.h>
#include <grub/mm.h>
#endif

/* This is only about cache architecture. It doesn't imply
   the CPU architecture.  */
static enum
  {
    ARCH_UNKNOWN,
    ARCH_ARMV5_WRITE_THROUGH,
    ARCH_ARMV6,
    ARCH_ARMV6_UNIFIED,
    ARCH_ARMV7
  } type = ARCH_UNKNOWN;

static int is_v6_mmu;

static grub_uint32_t grub_arch_cache_dlinesz;
static grub_uint32_t grub_arch_cache_ilinesz;
static grub_uint32_t grub_arch_cache_max_linesz;

/* Prototypes for asm functions.  */
void grub_arm_clean_dcache_range_armv6 (grub_addr_t start, grub_addr_t end,
					grub_addr_t dlinesz);
void grub_arm_clean_dcache_range_armv7 (grub_addr_t start, grub_addr_t end,
					grub_addr_t dlinesz);
void grub_arm_invalidate_icache_range_armv6 (grub_addr_t start, grub_addr_t end,
					     grub_addr_t dlinesz);
void grub_arm_invalidate_icache_range_armv7 (grub_addr_t start, grub_addr_t end,
					     grub_addr_t dlinesz);
void grub_arm_disable_caches_mmu_armv6 (void);
void grub_arm_disable_caches_mmu_armv7 (void);
grub_uint32_t grub_arm_main_id (void);
grub_uint32_t grub_arm_cache_type (void);

static void
probe_caches (void)
{
  grub_uint32_t main_id, cache_type;

  /* Read main ID Register */
  main_id = grub_arm_main_id ();

  switch ((main_id >> 16) & 0xf)
    {
    case 0x3:
    case 0x4:
    case 0x5:
    case 0x6:
      is_v6_mmu = 0;
      break;
    case 0x7:
    case 0xf:
      is_v6_mmu = 1;
      break;
    default:
      grub_fatal ("Unsupported ARM ID 0x%x", main_id);
    }

  /* Read Cache Type Register */
  cache_type = grub_arm_cache_type ();

  switch (cache_type >> 24)
    {
    case 0x00:
    case 0x01:
      grub_arch_cache_dlinesz = 8 << ((cache_type >> 12) & 3);
      grub_arch_cache_ilinesz = 8 << (cache_type & 3);
      type = ARCH_ARMV5_WRITE_THROUGH;
      break;
    case 0x04:
    case 0x0a:
    case 0x0c:
    case 0x0e:
    case 0x1c:
      grub_arch_cache_dlinesz = 8 << ((cache_type >> 12) & 3);
      grub_arch_cache_ilinesz = 8 << (cache_type & 3);
      type = ARCH_ARMV6_UNIFIED;
      break;
    case 0x05:
    case 0x0b:
    case 0x0d:
    case 0x0f:
    case 0x1d:
      grub_arch_cache_dlinesz = 8 << ((cache_type >> 12) & 3);
      grub_arch_cache_ilinesz = 8 << (cache_type & 3);
      type = ARCH_ARMV6;
      break;
    case 0x80 ... 0x8f:
      grub_arch_cache_dlinesz = 4 << ((cache_type >> 16) & 0xf);
      grub_arch_cache_ilinesz = 4 << (cache_type & 0xf);
      type = ARCH_ARMV7;
      break;
    default:
      grub_fatal ("Unsupported cache type 0x%x", cache_type);
    }
  if (grub_arch_cache_dlinesz > grub_arch_cache_ilinesz)
    grub_arch_cache_max_linesz = grub_arch_cache_dlinesz;
  else
    grub_arch_cache_max_linesz = grub_arch_cache_ilinesz;
}

#ifdef GRUB_MACHINE_UBOOT

static void subdivide (grub_uint32_t *table, grub_uint32_t *subtable,
		       grub_uint32_t addr)
{
  grub_uint32_t j;
  addr = addr >> 20 << 20;
  table[addr >> 20] = (grub_addr_t) subtable | 1;
  for (j = 0; j < 256; j++)
    subtable[j] = addr | (j << 12)
      | (3 << 4) | (3 << 6) | (3 << 8) | (3 << 10)
      | (0 << 3) | (1 << 2) | 2;
}

void
grub_arm_enable_caches_mmu (void)
{
  grub_uint32_t *table;
  grub_uint32_t i;
  grub_uint32_t border_crossing = 0;
  grub_uint32_t *subtable;
  struct sys_info *si = grub_uboot_get_sys_info ();

  if (!si || (si->mr_no == 0))
    {
      grub_printf ("couldn't get memory map, not enabling caches");
      grub_errno = GRUB_ERR_NONE;
      return;
    }

  if (type == ARCH_UNKNOWN)
    probe_caches ();

  for (i = 0; (signed) i < si->mr_no; i++)
    {
      if (si->mr[i].start & ((1 << 20) - 1))
	border_crossing++;
      if ((si->mr[i].start + si->mr[i].size) & ((1 << 20) - 1))
	border_crossing++;
    }

  grub_printf ("%d crossers\n", border_crossing);

  table = grub_memalign (1 << 14, (1 << 14) + (border_crossing << 10));
  if (!table)
    {
      grub_printf ("couldn't allocate place for MMU table, not enabling caches");
      grub_errno = GRUB_ERR_NONE;
      return;
    }

  subtable = table + (1 << 12);
  /* Map all unknown as device.  */
  for (i = 0; i < (1 << 12); i++)
    table[i] = (i << 20) | (3 << 10) | (0 << 3) | (1 << 2) | 2;
  /*
    Device: TEX= 0, C=0, B=1
    normal: TEX= 0, C=1, B=1
    AP = 3
    IMP = 0
    Domain = 0
*/

  for (i = 0; (signed) i < si->mr_no; i++)
    {
      if (si->mr[i].start & ((1 << 20) - 1))
	{
	  subdivide (table, subtable, si->mr[i].start);
	  subtable += (1 << 8);
	}
      if ((si->mr[i].start + si->mr[i].size) & ((1 << 20) - 1))
	{
	  subdivide (table, subtable, si->mr[i].start + si->mr[i].size);
	  subtable += (1 << 8);
	}
    }

  for (i = 0; (signed) i < si->mr_no; i++)
    if ((si->mr[i].flags & MR_ATTR_MASK) == MR_ATTR_DRAM
	|| (si->mr[i].flags & MR_ATTR_MASK) == MR_ATTR_SRAM
	|| (si->mr[i].flags & MR_ATTR_MASK) == MR_ATTR_FLASH)
      {
	grub_uint32_t cur, end;
	cur = si->mr[i].start;
	end = si->mr[i].start + si->mr[i].size;
	while (cur < end)
	  {
	    grub_uint32_t *st;
	    if ((table[cur >> 20] & 3) == 2)
	      {
		cur = cur >> 20 << 20;
		table[cur >> 20] = cur | (3 << 10) | (1 << 3) | (1 << 2) | 2;
		cur += (1 << 20);
		continue;
	      }
	    cur = cur >> 12 << 12;
	    st = (grub_uint32_t *) (table[cur >> 20] & ~0x3ff);
	    st[(cur >> 12) & 0xff] = cur | (3 << 4) | (3 << 6)
	      | (3 << 8) | (3 << 10)
	      | (1 << 3) | (1 << 2) | 2;
	    cur += (1 << 12);
	  }
      }

  grub_printf ("MMU tables generated\n");
  if (is_v6_mmu)
    grub_arm_clear_mmu_v6 ();

  grub_printf ("enabling MMU\n");
  grub_arm_enable_mmu (table);
  grub_printf ("MMU enabled\n");
}

#endif

void
grub_arch_sync_caches (void *address, grub_size_t len)
{
  grub_addr_t start = (grub_addr_t) address;
  grub_addr_t end = start + len;

  if (type == ARCH_UNKNOWN)
    probe_caches ();
  start = ALIGN_DOWN (start, grub_arch_cache_max_linesz);
  end = ALIGN_UP (end, grub_arch_cache_max_linesz);
  switch (type)
    {
    case ARCH_ARMV6:
      grub_arm_clean_dcache_range_armv6 (start, end, grub_arch_cache_dlinesz);
      grub_arm_invalidate_icache_range_armv6 (start, end,
					      grub_arch_cache_ilinesz);
      break;
    case ARCH_ARMV7:
      grub_arm_clean_dcache_range_armv7 (start, end, grub_arch_cache_dlinesz);
      grub_arm_invalidate_icache_range_armv7 (start, end,
					      grub_arch_cache_ilinesz);
      break;
      /* Nothing to do.  */
    case ARCH_ARMV5_WRITE_THROUGH:
    case ARCH_ARMV6_UNIFIED:
      break;
      /* Pacify GCC.  */
    case ARCH_UNKNOWN:
      break;
    }
}

void
grub_arm_disable_caches_mmu (void)
{
  if (type == ARCH_UNKNOWN)
    probe_caches ();
  switch (type)
    {
    case ARCH_ARMV5_WRITE_THROUGH:
    case ARCH_ARMV6_UNIFIED:
    case ARCH_ARMV6:
      grub_arm_disable_caches_mmu_armv6 ();
      break;
    case ARCH_ARMV7:
      grub_arm_disable_caches_mmu_armv7 ();
      break;
      /* Pacify GCC.  */
    case ARCH_UNKNOWN:
      break;
    }
}
