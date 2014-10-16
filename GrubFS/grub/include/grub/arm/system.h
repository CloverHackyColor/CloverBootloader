#ifndef GRUB_SYSTEM_CPU_HEADER
#define GRUB_SYSTEM_CPU_HEADER

#include <grub/types.h>

enum
  {
    GRUB_ARM_MACHINE_TYPE_RASPBERRY_PI = 3138,
    GRUB_ARM_MACHINE_TYPE_FDT = 0xFFFFFFFF
  };

void EXPORT_FUNC(grub_arm_disable_caches_mmu) (void);
void grub_arm_enable_caches_mmu (void);
void grub_arm_enable_mmu (grub_uint32_t *mmu_tables);
void grub_arm_clear_mmu_v6 (void);

#endif /* ! GRUB_SYSTEM_CPU_HEADER */

