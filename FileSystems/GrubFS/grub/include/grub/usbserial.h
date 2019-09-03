/* serial.h - serial device interface */
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

#ifndef GRUB_USBSERIAL_HEADER
#define GRUB_USBSERIAL_HEADER	1

void grub_usbserial_fini (struct grub_serial_port *port);

void grub_usbserial_detach (grub_usb_device_t usbdev, int configno,
			    int interfno);

int
grub_usbserial_attach (grub_usb_device_t usbdev, int configno, int interfno,
		       struct grub_serial_driver *driver, int in_endp,
		       int out_endp);
enum
  {
    GRUB_USB_SERIAL_ENDPOINT_LAST_MATCHING = -1
  };
int
grub_usbserial_fetch (struct grub_serial_port *port, grub_size_t header_size);

#endif
