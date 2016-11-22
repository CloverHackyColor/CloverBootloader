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

#ifndef	GRUB_USB_H
#define	GRUB_USB_H	1

#include <grub/err.h>
#include <grub/usbdesc.h>
#include <grub/usbtrans.h>

typedef struct grub_usb_device *grub_usb_device_t;
typedef struct grub_usb_controller *grub_usb_controller_t;
typedef struct grub_usb_controller_dev *grub_usb_controller_dev_t;

typedef enum
  {
    GRUB_USB_ERR_NONE,
    GRUB_USB_ERR_WAIT,
    GRUB_USB_ERR_INTERNAL,
    GRUB_USB_ERR_STALL,
    GRUB_USB_ERR_DATA,
    GRUB_USB_ERR_NAK,
    GRUB_USB_ERR_BABBLE,
    GRUB_USB_ERR_TIMEOUT,
    GRUB_USB_ERR_BITSTUFF,
    GRUB_USB_ERR_UNRECOVERABLE,
    GRUB_USB_ERR_BADDEVICE
  } grub_usb_err_t;

typedef enum
  {
    GRUB_USB_SPEED_NONE,
    GRUB_USB_SPEED_LOW,
    GRUB_USB_SPEED_FULL,
    GRUB_USB_SPEED_HIGH
  } grub_usb_speed_t;

typedef int (*grub_usb_iterate_hook_t) (grub_usb_device_t dev, void *data);
typedef int (*grub_usb_controller_iterate_hook_t) (grub_usb_controller_t dev,
						   void *data);

/* Call HOOK with each device, until HOOK returns non-zero.  */
int grub_usb_iterate (grub_usb_iterate_hook_t hook, void *hook_data);

grub_usb_err_t grub_usb_device_initialize (grub_usb_device_t dev);

grub_usb_err_t grub_usb_get_descriptor (grub_usb_device_t dev,
					grub_uint8_t type, grub_uint8_t index,
					grub_size_t size, char *data);

grub_usb_err_t grub_usb_clear_halt (grub_usb_device_t dev, int endpoint);


grub_usb_err_t grub_usb_set_configuration (grub_usb_device_t dev,
					   int configuration);

void grub_usb_controller_dev_register (grub_usb_controller_dev_t usb);

void grub_usb_controller_dev_unregister (grub_usb_controller_dev_t usb);

int grub_usb_controller_iterate (grub_usb_controller_iterate_hook_t hook,
				 void *hook_data);


grub_usb_err_t grub_usb_control_msg (grub_usb_device_t dev, grub_uint8_t reqtype,
				     grub_uint8_t request, grub_uint16_t value,
				     grub_uint16_t index, grub_size_t size,
				     char *data);

grub_usb_err_t
grub_usb_bulk_read (grub_usb_device_t dev,
		    struct grub_usb_desc_endp *endpoint,
		    grub_size_t size, char *data);
grub_usb_err_t
grub_usb_bulk_write (grub_usb_device_t dev,
		     struct grub_usb_desc_endp *endpoint,
		     grub_size_t size, char *data);

grub_usb_err_t
grub_usb_root_hub (grub_usb_controller_t controller);



/* XXX: All handled by libusb for now.  */
struct grub_usb_controller_dev
{
  /* The device name.  */
  const char *name;

  int (*iterate) (grub_usb_controller_iterate_hook_t hook, void *hook_data);

  grub_usb_err_t (*setup_transfer) (grub_usb_controller_t dev,
				    grub_usb_transfer_t transfer);

  grub_usb_err_t (*check_transfer) (grub_usb_controller_t dev,
				    grub_usb_transfer_t transfer,
				    grub_size_t *actual);

  grub_usb_err_t (*cancel_transfer) (grub_usb_controller_t dev,
				     grub_usb_transfer_t transfer);

  int (*hubports) (grub_usb_controller_t dev);

  grub_usb_err_t (*portstatus) (grub_usb_controller_t dev, unsigned int port,
				unsigned int enable);

  grub_usb_speed_t (*detect_dev) (grub_usb_controller_t dev, int port, int *changed);

  /* Per controller flag - port reset pending, don't do another reset */
  grub_uint64_t pending_reset;

  /* Max. number of transfer descriptors used per one bulk transfer */
  /* The reason is to prevent "exhausting" of TD by large bulk */
  /* transfer - number of TD is limited in USB host driver */
  /* Value is calculated/estimated in driver - some TDs should be */
  /* reserved for posible concurrent control or "interrupt" transfers */
  grub_size_t max_bulk_tds;
  
  /* The next host controller.  */
  struct grub_usb_controller_dev *next;
};

struct grub_usb_controller
{
  /* The underlying USB Host Controller device.  */
  grub_usb_controller_dev_t dev;

  /* Data used by the USB Host Controller Driver.  */
  void *data;
};


struct grub_usb_interface
{
  struct grub_usb_desc_if *descif;

  struct grub_usb_desc_endp *descendp;

  /* A driver is handling this interface. Do we need to support multiple drivers
     for single interface?
   */
  int attached;

  void (*detach_hook) (struct grub_usb_device *dev, int config, int interface);

  void *detach_data;
};

struct grub_usb_configuration
{
  /* Configuration descriptors .  */
  struct grub_usb_desc_config *descconf;

  /* Interfaces associated to this configuration.  */
  struct grub_usb_interface interf[32];
};

struct grub_usb_hub_port
{
  grub_uint64_t soft_limit_time;
  grub_uint64_t hard_limit_time;
  enum { 
    PORT_STATE_NORMAL = 0,
    PORT_STATE_WAITING_FOR_STABLE_POWER = 1,
    PORT_STATE_FAILED_DEVICE = 2,
    PORT_STATE_STABLE_POWER = 3,
  } state;
};

struct grub_usb_device
{
  /* The device descriptor of this device.  */
  struct grub_usb_desc_device descdev;

  /* The controller the device is connected to.  */
  struct grub_usb_controller controller;

  /* Device configurations (after opening the device).  */
  struct grub_usb_configuration config[8];

  /* Device address.  */
  int addr;

  /* Device speed.  */
  grub_usb_speed_t speed;

  /* All descriptors are read if this is set to 1.  */
  int initialized;

  /* Data toggle values (used for bulk transfers only).  */
  int toggle[256];

  /* Used by libusb wrapper.  Schedulded for removal. */
  void *data;

  /* Hub information.  */

  /* Array of children for a hub.  */
  grub_usb_device_t *children;

  /* Number of hub ports.  */
  unsigned nports;

  struct grub_usb_hub_port *ports;

  grub_usb_transfer_t hub_transfer;

  grub_uint32_t statuschange;

  struct grub_usb_desc_endp *hub_endpoint;

  /* EHCI Split Transfer information */
  int split_hubport;

  int split_hubaddr;
};



typedef enum grub_usb_ep_type
  {
    GRUB_USB_EP_CONTROL,
    GRUB_USB_EP_ISOCHRONOUS,
    GRUB_USB_EP_BULK,
    GRUB_USB_EP_INTERRUPT
  } grub_usb_ep_type_t;

static inline enum grub_usb_ep_type
grub_usb_get_ep_type (struct grub_usb_desc_endp *ep)
{
  return ep->attrib & 3;
}

typedef enum
  {
    GRUB_USB_CLASS_NOTHERE,
    GRUB_USB_CLASS_AUDIO,
    GRUB_USB_CLASS_COMMUNICATION,
    GRUB_USB_CLASS_HID,
    GRUB_USB_CLASS_XXX,
    GRUB_USB_CLASS_PHYSICAL,
    GRUB_USB_CLASS_IMAGE,
    GRUB_USB_CLASS_PRINTER,
    GRUB_USB_CLASS_MASS_STORAGE,
    GRUB_USB_CLASS_HUB,
    GRUB_USB_CLASS_DATA_INTERFACE,
    GRUB_USB_CLASS_SMART_CARD,
    GRUB_USB_CLASS_CONTENT_SECURITY,
    GRUB_USB_CLASS_VIDEO
  } grub_usb_classes_t;

typedef enum
  {
    GRUB_USBMS_SUBCLASS_BULK = 0x06,
  	/* Experimental support for non-pure SCSI devices */
    GRUB_USBMS_SUBCLASS_RBC = 0x01,
    GRUB_USBMS_SUBCLASS_MMC2 = 0x02,
    GRUB_USBMS_SUBCLASS_UFI = 0x04,
    GRUB_USBMS_SUBCLASS_SFF8070 = 0x05
  } grub_usbms_subclass_t;

typedef enum
  {
    GRUB_USBMS_PROTOCOL_BULK = 0x50,
    /* Experimental support for Control/Bulk/Interrupt (CBI) devices */
    GRUB_USBMS_PROTOCOL_CBI = 0x00, /* CBI with interrupt */
    GRUB_USBMS_PROTOCOL_CB = 0x01  /* CBI wthout interrupt */
  } grub_usbms_protocol_t;

static inline struct grub_usb_desc_if *
grub_usb_get_config_interface (struct grub_usb_desc_config *config)
{
  struct grub_usb_desc_if *interf;

  interf = (struct grub_usb_desc_if *) (sizeof (*config) + (char *) config);
  return interf;
}

typedef int (*grub_usb_attach_hook_class) (grub_usb_device_t usbdev,
					   int configno, int interfno);

struct grub_usb_attach_desc
{
  struct grub_usb_attach_desc *next;
  struct grub_usb_attach_desc **prev;
  int class;
  grub_usb_attach_hook_class hook;
};

void grub_usb_register_attach_hook_class (struct grub_usb_attach_desc *desc);
void grub_usb_unregister_attach_hook_class (struct grub_usb_attach_desc *desc);

void grub_usb_poll_devices (int wait_for_completion);

void grub_usb_device_attach (grub_usb_device_t dev);
grub_usb_err_t
grub_usb_bulk_read_extended (grub_usb_device_t dev,
			     struct grub_usb_desc_endp *endpoint,
			     grub_size_t size, char *data,
			     int timeout, grub_size_t *actual);
grub_usb_transfer_t
grub_usb_bulk_read_background (grub_usb_device_t dev,
			       struct grub_usb_desc_endp *endpoint,
			       grub_size_t size, void *data);
grub_usb_err_t
grub_usb_check_transfer (grub_usb_transfer_t trans, grub_size_t *actual);
void
grub_usb_cancel_transfer (grub_usb_transfer_t trans);

#endif /* GRUB_USB_H */
