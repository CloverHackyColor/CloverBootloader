/* icon_manager.h - gfxmenu icon manager. */
/*
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

#ifndef GRUB_ICON_MANAGER_HEADER
#define GRUB_ICON_MANAGER_HEADER 1

#include <grub/menu.h>
#include <grub/bitmap.h>

/* Forward declaration of opaque structure handle type.  */
typedef struct grub_gfxmenu_icon_manager *grub_gfxmenu_icon_manager_t;

grub_gfxmenu_icon_manager_t grub_gfxmenu_icon_manager_new (void);
void grub_gfxmenu_icon_manager_destroy (grub_gfxmenu_icon_manager_t mgr);
void grub_gfxmenu_icon_manager_clear_cache (grub_gfxmenu_icon_manager_t mgr);
void grub_gfxmenu_icon_manager_set_theme_path (grub_gfxmenu_icon_manager_t mgr,
                                               const char *path);
void grub_gfxmenu_icon_manager_set_icon_size (grub_gfxmenu_icon_manager_t mgr,
                                              int width, int height);
struct grub_video_bitmap *
grub_gfxmenu_icon_manager_get_icon (grub_gfxmenu_icon_manager_t mgr,
                                    grub_menu_entry_t entry);

#endif /* GRUB_ICON_MANAGER_HEADER */

