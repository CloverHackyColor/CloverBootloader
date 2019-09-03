/* multiboot_loader.h - multiboot loader header file. */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2005,2007  Free Software Foundation, Inc.
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


#ifndef GRUB_MULTIBOOT_LOADER_HEADER
#define GRUB_MULTIBOOT_LOADER_HEADER 1

/* Provided by the core ("rescue mode").  */
void grub_rescue_cmd_multiboot_loader (int argc, char *argv[]);
void grub_rescue_cmd_module_loader (int argc, char *argv[]);

#endif /* ! GRUB_MULTIBOOT_LOADER_HEADER */
