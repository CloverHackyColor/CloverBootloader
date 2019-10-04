/**

  Hack configuration
  This is a gestalt of black art, do not edit.

  by vit9696

**/

#ifndef APTIOFIX_HACK_CONFIG_H
#define APTIOFIX_HACK_CONFIG_H

/**
 * Attempt to protect certain CSM memory regions from being used by the kernel (by Slice).
 * On older firmwares this caused wake issues.
 */
#ifndef APTIOFIX_PROTECT_CSM_REGION
#define APTIOFIX_PROTECT_CSM_REGION 1
#endif

/** Forces XNU to use old UEFI memory mapping after hibernation wake.
 *  May cause memory corruption. See FixHibernateWakeWithoutRelocBlock for details.
 */
#ifndef APTIOFIX_HIBERNATION_FORCE_OLD_MEMORYMAP
#define APTIOFIX_HIBERNATION_FORCE_OLD_MEMORYMAP 1
#endif

/** When attempting to reuse old UEFI memory mapping gBS->AllocatePool seems
 *  to produce the same addresses way more often, and thus the system will not reboot
 *  when accessing RTShims after waking from hibernation.
 *  However, gBS->AllocatePool is dangerous, because it may overlap with the kernel
 *  region and break aslr.
 */
#ifndef APTIOFIX_ALLOCATE_POOL_GIVES_STABLE_ADDR
#define APTIOFIX_ALLOCATE_POOL_GIVES_STABLE_ADDR APTIOFIX_HIBERNATION_FORCE_OLD_MEMORYMAP
#endif

/** boot.efi still tries to allocate runtime memory for reserved segments as of 10.13.3.
 *  This results in boot failure with "Couldn't allocate memory map" error.
 *  Proven by GA-H81N-D2H with a single reserved segment:
 *  000000009F800000-00000000DF9FFFFF 0000000000040200 8000000000000000
 */
#ifndef APTIOFIX_PROTECT_RESERVED_MEMORY
#define APTIOFIX_PROTECT_RESERVED_MEMORY 1
#endif

/** It is believed that boot.efi on Sandy & Ivy skips 0x10200000 bytes from 0x10000000
 *  to protect from IGPU bugs, yet if this memory is marked available, it will may be
 *  used by XNU. So far attempts to enable this did not show any pattern but boot failures.
 */

#define APTIOFIX_SLICE_OVERLAPPING_REGION_FIX 1

#ifndef APTIOFIX_PROTECT_IGPU_SANDY_IVY_RESERVED_MEMORY
#define APTIOFIX_PROTECT_IGPU_SANDY_IVY_RESERVED_MEMORY 1
#endif

/** Attempt to protect some memory region from being used by the kernel (by Slice).
 *  It is believed to cause sleep issues on some systems, because this region
 *  is generally marked as conventional memory.
 */
#ifndef APTIOFIX_UNMARKED_OVERLAPPING_REGION_FIX
#define APTIOFIX_UNMARKED_OVERLAPPING_REGION_FIX 1
#endif

/** Calculate aslr slide ourselves when some addresses are not available for XNU. */
#ifndef APTIOFIX_ALLOW_CUSTOM_ASLR_IMPLEMENTATION
#define APTIOFIX_ALLOW_CUSTOM_ASLR_IMPLEMENTATION 1
#endif

/** This is important for several boards that cannot boot with slide=0, which safe mode enforces. */
#ifndef APTIOFIX_ALLOW_ASLR_IN_SAFE_MODE
#define APTIOFIX_ALLOW_ASLR_IN_SAFE_MODE 1
#endif

/** Hide slide=x value from os for increased security when using custom aslr. */
#ifndef APTIOFIX_CLEANUP_SLIDE_BOOT_ARGUMENT
#define APTIOFIX_CLEANUP_SLIDE_BOOT_ARGUMENT APTIOFIX_ALLOW_CUSTOM_ASLR_IMPLEMENTATION
#endif

/**
 * Speculated maximum kernel size (in bytes) to use when looking for a free memory region.
 * Used by APTIOFIX_ALLOW_CUSTOM_ASLR_IMPLEMENTATION to determine valid slide values.
 * 10.12.6 allocates at least approximately 287 MBs, we round it to 384 MBs
 * This seems to work pretty well on X299. Yet it may be a good idea to make a boot-arg.
 */
#ifndef APTIOFIX_SPECULATED_KERNEL_SIZE
#define APTIOFIX_SPECULATED_KERNEL_SIZE ((UINTN)0x18000000)
#endif

/** Maximum number of supported runtime reloc protection areas */
#ifndef APTIFIX_MAX_RT_RELOC_NUM
#define APTIFIX_MAX_RT_RELOC_NUM ((UINTN)64)
#endif

/**
 * Perform invasive memory dumps when -aptiodump -v are passed to boot.efi.
 * This allows to reliably get the memory maps when in-OS dtrace script is broken.
 * Enable for development and testing purposes.
 */
#ifndef APTIOFIX_ALLOW_MEMORY_DUMP_ARG
#define APTIOFIX_ALLOW_MEMORY_DUMP_ARG 0
#endif

/**
 * Performing memory map dumps may alter memory map contents themselves, so it is important to ensure
 * no memory is allocated during the dump process. This is especially crtitical for most ASUS APTIO V
 * boards for Skylake and newer, which may crash after bootng.
 */
#ifndef APTIOFIX_CUSTOM_POOL_ALLOCATOR
#define APTIOFIX_CUSTOM_POOL_ALLOCATOR APTIOFIX_ALLOW_MEMORY_DUMP_ARG
#endif

/**
 * Maximum reserved area used by the custom pool allocator. This area must be large enough
 * to fit the screen buffer, but considerably small to avoid colliding with the kernel area.
 * 32 MB appears experimentally proven to be good enough for most of the boards.
 */
#ifndef APTIOFIX_CUSTOM_POOL_ALLOCATOR_SIZE
#define APTIOFIX_CUSTOM_POOL_ALLOCATOR_SIZE 0x2000000
#endif

#endif // APTIOFIX_HACK_CONFIG_H
