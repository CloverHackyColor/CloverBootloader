#ifndef GRUB_MACHINE_EMU
#error "This source is only meant for grub-emu platform"
#endif

#include <grub/cache.h>

#if defined(__ia64__)
#include "../ia64/cache.c"
#elif defined (__arm__) || defined (__aarch64__)

void __clear_cache (char *beg, char *end);

void
grub_arch_sync_caches (void *address, grub_size_t len)
{
  __clear_cache (address, (char *) address + len);
}

#elif defined (__mips__)
void _flush_cache (void *address, grub_size_t len, int type);

void
grub_arch_sync_caches (void *address, grub_size_t len)
{
  return _flush_cache (address, len, 0);
}

#endif

