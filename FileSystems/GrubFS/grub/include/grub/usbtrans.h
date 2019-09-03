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

#ifndef	GRUB_USBTRANS_H
#define	GRUB_USBTRANS_H	1

#define MAX_USB_TRANSFER_LEN 0x0800

typedef enum
  {
    GRUB_USB_TRANSFER_TYPE_IN,
    GRUB_USB_TRANSFER_TYPE_OUT,
    GRUB_USB_TRANSFER_TYPE_SETUP
  } grub_transfer_type_t;

typedef enum
  {
    GRUB_USB_TRANSACTION_TYPE_CONTROL,
    GRUB_USB_TRANSACTION_TYPE_BULK
  } grub_transaction_type_t;

struct grub_usb_transaction
{
  int size;
  int toggle;
  grub_transfer_type_t pid;
  grub_uint32_t data;
  grub_size_t preceding;
};
typedef struct grub_usb_transaction *grub_usb_transaction_t;

struct grub_usb_transfer
{
  int devaddr;

  int endpoint;

  int size;

  int transcnt;

  int max;

  grub_transaction_type_t type;

  grub_transfer_type_t dir;

  struct grub_usb_device *dev;

  struct grub_usb_transaction *transactions;
  
  int last_trans;
  /* Index of last processed transaction in OHCI/UHCI driver. */

  void *controller_data;

  /* Used when finishing transfer to copy data back.  */
  struct grub_pci_dma_chunk *data_chunk;
  void *data;
};
typedef struct grub_usb_transfer *grub_usb_transfer_t;



enum
  {
    GRUB_USB_REQTYPE_TARGET_DEV = (0 << 0),
    GRUB_USB_REQTYPE_TARGET_INTERF = (1 << 0),
    GRUB_USB_REQTYPE_TARGET_ENDP = (2 << 0),
    GRUB_USB_REQTYPE_TARGET_OTHER = (3 << 0),
    GRUB_USB_REQTYPE_STANDARD = (0 << 5),
    GRUB_USB_REQTYPE_CLASS = (1 << 5),
    GRUB_USB_REQTYPE_VENDOR = (2 << 5),
    GRUB_USB_REQTYPE_OUT = (0 << 7),
    GRUB_USB_REQTYPE_IN	= (1 << 7),
    GRUB_USB_REQTYPE_CLASS_INTERFACE_OUT = GRUB_USB_REQTYPE_TARGET_INTERF
    | GRUB_USB_REQTYPE_CLASS | GRUB_USB_REQTYPE_OUT,
    GRUB_USB_REQTYPE_VENDOR_OUT = GRUB_USB_REQTYPE_VENDOR | GRUB_USB_REQTYPE_OUT,
    GRUB_USB_REQTYPE_CLASS_INTERFACE_IN = GRUB_USB_REQTYPE_TARGET_INTERF
    | GRUB_USB_REQTYPE_CLASS | GRUB_USB_REQTYPE_IN,
    GRUB_USB_REQTYPE_VENDOR_IN = GRUB_USB_REQTYPE_VENDOR | GRUB_USB_REQTYPE_IN
  };

enum
  {
    GRUB_USB_REQ_GET_STATUS = 0x00,
    GRUB_USB_REQ_CLEAR_FEATURE = 0x01,
    GRUB_USB_REQ_SET_FEATURE = 0x03,
    GRUB_USB_REQ_SET_ADDRESS = 0x05,
    GRUB_USB_REQ_GET_DESCRIPTOR = 0x06,
    GRUB_USB_REQ_SET_DESCRIPTOR	= 0x07,
    GRUB_USB_REQ_GET_CONFIGURATION = 0x08,
    GRUB_USB_REQ_SET_CONFIGURATION = 0x09,
    GRUB_USB_REQ_GET_INTERFACE = 0x0A,
    GRUB_USB_REQ_SET_INTERFACE = 0x0B,
    GRUB_USB_REQ_SYNC_FRAME = 0x0C
  };

#define GRUB_USB_FEATURE_ENDP_HALT	0x00
#define GRUB_USB_FEATURE_DEV_REMOTE_WU	0x01
#define GRUB_USB_FEATURE_TEST_MODE	0x02

enum
  {
    GRUB_USB_HUB_FEATURE_PORT_RESET = 0x04,
    GRUB_USB_HUB_FEATURE_PORT_POWER = 0x08,
    GRUB_USB_HUB_FEATURE_C_PORT_CONNECTED = 0x10,
    GRUB_USB_HUB_FEATURE_C_PORT_ENABLED = 0x11,
    GRUB_USB_HUB_FEATURE_C_PORT_SUSPEND = 0x12,
    GRUB_USB_HUB_FEATURE_C_PORT_OVERCURRENT = 0x13,
    GRUB_USB_HUB_FEATURE_C_PORT_RESET = 0x14
  };

enum
  {
    GRUB_USB_HUB_STATUS_PORT_CONNECTED = (1 << 0),
    GRUB_USB_HUB_STATUS_PORT_ENABLED = (1 << 1),
    GRUB_USB_HUB_STATUS_PORT_SUSPEND = (1 << 2),
    GRUB_USB_HUB_STATUS_PORT_OVERCURRENT = (1 << 3),
    GRUB_USB_HUB_STATUS_PORT_POWERED = (1 << 8),
    GRUB_USB_HUB_STATUS_PORT_LOWSPEED = (1 << 9),
    GRUB_USB_HUB_STATUS_PORT_HIGHSPEED = (1 << 10),
    GRUB_USB_HUB_STATUS_C_PORT_CONNECTED = (1 << 16),
    GRUB_USB_HUB_STATUS_C_PORT_ENABLED = (1 << 17),
    GRUB_USB_HUB_STATUS_C_PORT_SUSPEND = (1 << 18),
    GRUB_USB_HUB_STATUS_C_PORT_OVERCURRENT = (1 << 19),
    GRUB_USB_HUB_STATUS_C_PORT_RESET = (1 << 20)
  };

struct grub_usb_packet_setup
{
  grub_uint8_t reqtype;
  grub_uint8_t request;
  grub_uint16_t value;
  grub_uint16_t index;
  grub_uint16_t length;
} GRUB_PACKED;


#endif /* GRUB_USBTRANS_H */
