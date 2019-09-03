/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2010  Free Software Foundation, Inc.
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

void EXPORT_FUNC (usb_bulk_write) (void);
void EXPORT_FUNC (usb_find_busses) (void);
void EXPORT_FUNC (usb_init) (void);
void EXPORT_FUNC (usb_find_devices) (void);
void EXPORT_FUNC (usb_open) (void);
void EXPORT_FUNC (usb_get_busses) (void);
void EXPORT_FUNC (usb_control_msg) (void);
void EXPORT_FUNC (usb_release_interface) (void);
void EXPORT_FUNC (usb_close) (void);
void EXPORT_FUNC (usb_bulk_read) (void);
void EXPORT_FUNC (usb_claim_interface) (void);
