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

#ifndef	GRUB_USBDESC_H
#define	GRUB_USBDESC_H	1

#include <grub/types.h>
#include <grub/symbol.h>

typedef enum {
  GRUB_USB_DESCRIPTOR_DEVICE = 1,
  GRUB_USB_DESCRIPTOR_CONFIG,
  GRUB_USB_DESCRIPTOR_STRING,
  GRUB_USB_DESCRIPTOR_INTERFACE,
  GRUB_USB_DESCRIPTOR_ENDPOINT,
  GRUB_USB_DESCRIPTOR_DEBUG = 10,
  GRUB_USB_DESCRIPTOR_HUB = 0x29
} grub_usb_descriptor_t;

struct grub_usb_desc
{
  grub_uint8_t length;
  grub_uint8_t type;
} GRUB_PACKED;

struct grub_usb_desc_device
{
  grub_uint8_t length;
  grub_uint8_t type;
  grub_uint16_t usbrel;
  grub_uint8_t class;
  grub_uint8_t subclass;
  grub_uint8_t protocol;
  grub_uint8_t maxsize0;
  grub_uint16_t vendorid;
  grub_uint16_t prodid;
  grub_uint16_t devrel;
  grub_uint8_t strvendor;
  grub_uint8_t strprod;
  grub_uint8_t strserial;
  grub_uint8_t configcnt;
} GRUB_PACKED;

struct grub_usb_desc_config
{
  grub_uint8_t length;
  grub_uint8_t type;
  grub_uint16_t totallen;
  grub_uint8_t numif;
  grub_uint8_t config;
  grub_uint8_t strconfig;
  grub_uint8_t attrib;
  grub_uint8_t maxpower;
} GRUB_PACKED;

#if 0
struct grub_usb_desc_if_association
{
  grub_uint8_t length;
  grub_uint8_t type;
  grub_uint8_t firstif;
  grub_uint8_t ifcnt;
  grub_uint8_t class;
  grub_uint8_t subclass;
  grub_uint8_t protocol;
  grub_uint8_t function;
} GRUB_PACKED;
#endif

struct grub_usb_desc_if
{
  grub_uint8_t length;
  grub_uint8_t type;
  grub_uint8_t ifnum;
  grub_uint8_t altsetting;
  grub_uint8_t endpointcnt;
  grub_uint8_t class;
  grub_uint8_t subclass;
  grub_uint8_t protocol;
  grub_uint8_t strif;
} GRUB_PACKED;

struct grub_usb_desc_endp
{
  grub_uint8_t length;
  grub_uint8_t type;
  grub_uint8_t endp_addr;
  grub_uint8_t attrib;
  grub_uint16_t maxpacket;
  grub_uint8_t interval;
} GRUB_PACKED;

struct grub_usb_desc_str
{
  grub_uint8_t length;
  grub_uint8_t type;
  grub_uint16_t str[0];
} GRUB_PACKED;

struct grub_usb_desc_debug
{
  grub_uint8_t length;
  grub_uint8_t type;
  grub_uint8_t in_endp;
  grub_uint8_t out_endp;
} GRUB_PACKED;

struct grub_usb_usb_hubdesc
{
  grub_uint8_t length;
  grub_uint8_t type;
  grub_uint8_t portcnt;
  grub_uint16_t characteristics;
  grub_uint8_t pwdgood;
  grub_uint8_t current;
  /* Removable and power control bits follow.  */
} GRUB_PACKED;

#endif /* GRUB_USBDESC_H */
