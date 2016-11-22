#ifndef GRUB_VESA_MODE_TABLE_HEADER
#define GRUB_VESA_MODE_TABLE_HEADER 1

#include <grub/types.h>

#define GRUB_VESA_MODE_TABLE_START 0x300
#define GRUB_VESA_MODE_TABLE_END 0x373

struct grub_vesa_mode_table_entry {
  grub_uint16_t width;
  grub_uint16_t height;
  grub_uint8_t depth;
};

extern struct grub_vesa_mode_table_entry
grub_vesa_mode_table[GRUB_VESA_MODE_TABLE_END
		     - GRUB_VESA_MODE_TABLE_START + 1];

#endif
