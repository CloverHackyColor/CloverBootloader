/*
 * Pci.h
 *
 *  Created on: 16 Apr 2020
 *      Author: jief
 */

#ifndef INCLUDE_PCI_H_
#define INCLUDE_PCI_H_

#include <stdint.h>
#include "../cpp_foundation/XBool.h"

/* PCI */
#define PCI_BASE_ADDRESS_0          0x10    /* 32 bits */
#define PCI_BASE_ADDRESS_1          0x14    /* 32 bits [htype 0,1 only] */
#define PCI_BASE_ADDRESS_2          0x18    /* 32 bits [htype 0 only] */
#define PCI_BASE_ADDRESS_3          0x1c    /* 32 bits */
#define PCI_BASE_ADDRESS_4          0x20    /* 32 bits */
#define PCI_BASE_ADDRESS_5          0x24    /* 32 bits */

#define PCI_CLASS_MEDIA_HDA         0x03


#define PCIADDR(bus, dev, func)      ((1 << 31) | ((bus) << 16) | ((dev) << 11) | ((func) << 8))


typedef struct {
	uint32_t		:2;
	uint32_t	reg :6;
	uint32_t	func:3;
	uint32_t	dev :5;
	uint32_t	bus :8;
	uint32_t		:7;
	uint32_t	eb	:1;
} pci_addr_t;

typedef union pci_dev_t {
  uint32_t    addr = 0;
	pci_addr_t	bits;
} pci_dev_t;

typedef struct pci_dt_t {
//  EFI_PCI_IO_PROTOCOL		*PciIo;
//  PCI_TYPE00            Pci;
  EFI_HANDLE    DeviceHandle = NULL;
  UINT8*        regs         = NULL;
	pci_dev_t			dev          = pci_dev_t();

	UINT16				vendor_id    = 0;
	UINT16				device_id    = 0;

	union {
		struct {
			UINT16	  vendor_id;
			UINT16	  device_id;
		} subsys;
		UINT32	    subsys_id;
  } subsys_id_union = { {0, 0} };
	UINT8		      revision      = 0;
	UINT8		      subclass      = 0;
	UINT16				class_id      = 0;

	struct pci_dt_t	*parent     = NULL;
	struct pci_dt_t	*children   = NULL;
	struct pci_dt_t	*next       = NULL;
  XBool           used       = false;
} pci_dt_t;

#if 0

//
// Definitions of PCI class bytes and manipulation macros.
//
#define PCI_CLASS_OLD                 0x00
#define   PCI_CLASS_OLD_OTHER           0x00
#define   PCI_CLASS_OLD_VGA             0x01

#define PCI_CLASS_MASS_STORAGE        0x01
#define   PCI_CLASS_MASS_STORAGE_SCSI   0x00
#define     PCI_IF_MASS_STORAGE_SCSI_VENDOR_SPECIFIC          0x00
#define     PCI_IF_MASS_STORAGE_SCSI_DEVICE_PQI               0x11
#define     PCI_IF_MASS_STORAGE_SCSI_CONTROLLER_PQI           0x12
#define     PCI_IF_MASS_STORAGE_SCSI_DEVICE_CONTROLLER_PQI    0x13
#define     PCI_IF_MASS_STORAGE_SCSI_DEVICE_NVM_EXPRESS       0x21
#define   PCI_CLASS_MASS_STORAGE_IDE    0x01
#define   PCI_CLASS_MASS_STORAGE_FLOPPY 0x02
#define   PCI_CLASS_MASS_STORAGE_IPI    0x03
#define   PCI_CLASS_MASS_STORAGE_RAID   0x04
#define   PCI_CLASS_MASS_STORAGE_ATA       0x05
#define     PCI_IF_MASS_STORAGE_SINGLE_DMA   0x20
#define     PCI_IF_MASS_STORAGE_CHAINED_DMA  0x30
#define   PCI_CLASS_MASS_STORAGE_SATADPA   0x06
#define     PCI_IF_MASS_STORAGE_SATA         0x00
#define     PCI_IF_MASS_STORAGE_AHCI         0x01
#define     PCI_IF_MASS_STORAGE_SATA_SERIAL_BUS   0x02
#define   PCI_CLASS_MASS_STORAGE_SAS                        0x07
#define     PCI_IF_MASS_STORAGE_SAS                           0x00
#define     PCI_IF_MASS_STORAGE_SAS_SERIAL_BUS                0x01
#define   PCI_CLASS_MASS_STORAGE_SOLID_STATE                0x08
#define     PCI_IF_MASS_STORAGE_SOLID_STATE                   0x00
#define     PCI_IF_MASS_STORAGE_SOLID_STATE_NVMHCI            0x01
#define     PCI_IF_MASS_STORAGE_SOLID_STATE_ENTERPRISE_NVMHCI 0x02
#define   PCI_CLASS_MASS_STORAGE_OTHER  0x80

#define PCI_CLASS_NETWORK             0x02
#define   PCI_CLASS_NETWORK_ETHERNET    0x00
#define   PCI_CLASS_NETWORK_TOKENRING   0x01
#define   PCI_CLASS_NETWORK_FDDI        0x02
#define   PCI_CLASS_NETWORK_ATM         0x03
#define   PCI_CLASS_NETWORK_ISDN        0x04
#define   PCI_CLASS_NETWORK_WORLDFIP              0x05
#define   PCI_CLASS_NETWORK_PICMG_MULTI_COMPUTING 0x06
#define   PCI_CLASS_NETWORK_INFINIBAND  0x07
#define   PCI_CLASS_NETWORK_OTHER       0x80

#define PCI_CLASS_DISPLAY             0x03
#define   PCI_CLASS_DISPLAY_VGA         0x00
#define     PCI_IF_VGA_VGA                0x00
#define     PCI_IF_VGA_8514               0x01
#define   PCI_CLASS_DISPLAY_XGA         0x01
#define   PCI_CLASS_DISPLAY_3D          0x02
#define   PCI_CLASS_DISPLAY_OTHER       0x80

#define PCI_CLASS_MEDIA               0x04
#define   PCI_CLASS_MEDIA_VIDEO         0x00
#define   PCI_CLASS_MEDIA_AUDIO         0x01
#define   PCI_CLASS_MEDIA_TELEPHONE     0x02
#define   PCI_CLASS_MEDIA_HDA           0x03
#define   PCI_CLASS_MEDIA_MIXED_MODE    0x03  //other name
#define   PCI_CLASS_MEDIA_OTHER         0x80

#define PCI_CLASS_MEMORY_CONTROLLER   0x05
#define   PCI_CLASS_MEMORY_RAM          0x00
#define   PCI_CLASS_MEMORY_FLASH        0x01
#define   PCI_CLASS_MEMORY_OTHER        0x80

#define PCI_CLASS_BRIDGE              0x06
#define   PCI_CLASS_BRIDGE_HOST         0x00
#define   PCI_CLASS_BRIDGE_ISA          0x01
#define   PCI_CLASS_BRIDGE_EISA         0x02
#define   PCI_CLASS_BRIDGE_MCA          0x03
#define   PCI_CLASS_BRIDGE_P2P          0x04
#define     PCI_IF_BRIDGE_P2P             0x00
#define     PCI_IF_BRIDGE_P2P_SUBTRACTIVE 0x01
#define   PCI_CLASS_BRIDGE_PCMCIA       0x05
#define   PCI_CLASS_BRIDGE_NUBUS        0x06
#define   PCI_CLASS_BRIDGE_CARDBUS      0x07
#define   PCI_CLASS_BRIDGE_RACEWAY      0x08
#define   PCI_CLASS_BRIDGE_SEMI_TRANSPARENT_P2P        0x09
#define     PCI_IF_BRIDGE_SEMI_TRANSPARENT_P2P_PRIMARY   0x40
#define     PCI_IF_BRIDGE_SEMI_TRANSPARENT_P2P_SECONDARY 0x80
#define   PCI_CLASS_BRIDGE_INFINIBAND_TO_PCI           0x0A
#define   PCI_CLASS_BRIDGE_ADVANCED_SWITCHING_TO_PCI   0x0B
#define     PCI_IF_BRIDGE_ADVANCED_SWITCHING_TO_PCI_CUSTOM  0x00
#define     PCI_IF_BRIDGE_ADVANCED_SWITCHING_TO_PCI_ASI_SIG 0x01
#define   PCI_CLASS_BRIDGE_OTHER        0x80
#define   PCI_CLASS_BRIDGE_ISA_PDECODE  0x80

#define PCI_CLASS_SCC                 0x07  ///< Simple communications controllers
#define   PCI_SUBCLASS_SERIAL           0x00
#define     PCI_IF_GENERIC_XT             0x00
#define     PCI_IF_16450                  0x01
#define     PCI_IF_16550                  0x02
#define     PCI_IF_16650                  0x03
#define     PCI_IF_16750                  0x04
#define     PCI_IF_16850                  0x05
#define     PCI_IF_16950                  0x06
#define   PCI_SUBCLASS_PARALLEL         0x01
#define     PCI_IF_PARALLEL_PORT          0x00
#define     PCI_IF_BI_DIR_PARALLEL_PORT   0x01
#define     PCI_IF_ECP_PARALLEL_PORT      0x02
#define     PCI_IF_1284_CONTROLLER        0x03
#define     PCI_IF_1284_DEVICE            0xFE
#define   PCI_SUBCLASS_MULTIPORT_SERIAL 0x02
#define   PCI_SUBCLASS_MODEM            0x03
#define     PCI_IF_GENERIC_MODEM          0x00
#define     PCI_IF_16450_MODEM            0x01
#define     PCI_IF_16550_MODEM            0x02
#define     PCI_IF_16650_MODEM            0x03
#define     PCI_IF_16750_MODEM            0x04
#define   PCI_SUBCLASS_GPIB             0x04
#define   PCI_SUBCLASS_SMART_CARD       0x05
#define   PCI_SUBCLASS_SCC_OTHER        0x80

#define PCI_CLASS_SYSTEM_PERIPHERAL   0x08
#define   PCI_SUBCLASS_PIC              0x00
#define     PCI_IF_8259_PIC               0x00
#define     PCI_IF_ISA_PIC                0x01
#define     PCI_IF_EISA_PIC               0x02
#define     PCI_IF_HPET                   0x03
#define     PCI_IF_APIC_CONTROLLER        0x10  ///< I/O APIC interrupt controller , 32 bye none-prefectable memory.
#define     PCI_IF_APIC_CONTROLLER2       0x20
#define   PCI_SUBCLASS_DMA              0x01
#define     PCI_IF_8237_DMA               0x00
#define     PCI_IF_ISA_DMA                0x01
#define     PCI_IF_EISA_DMA               0x02
#define   PCI_SUBCLASS_TIMER            0x02
#define     PCI_IF_8254_TIMER             0x00
#define     PCI_IF_ISA_TIMER              0x01
#define     PCI_IF_EISA_TIMER             0x02
#define   PCI_SUBCLASS_RTC              0x03
#define     PCI_IF_GENERIC_RTC            0x00
#define     PCI_IF_ISA_RTC                0x01
#define   PCI_SUBCLASS_PNP_CONTROLLER   0x04    ///< HotPlug Controller
#define   PCI_SUBCLASS_SD_HOST_CONTROLLER 0x05
#define   PCI_SUBCLASS_IOMMU              0x06
#define   PCI_SUBCLASS_PERIPHERAL_OTHER 0x80

#define PCI_CLASS_INPUT_DEVICE        0x09
#define   PCI_SUBCLASS_KEYBOARD         0x00
#define   PCI_SUBCLASS_PEN              0x01
#define   PCI_SUBCLASS_MOUSE_CONTROLLER 0x02
#define   PCI_SUBCLASS_SCAN_CONTROLLER  0x03
#define   PCI_SUBCLASS_GAMEPORT         0x04
#define     PCI_IF_GAMEPORT               0x00
#define     PCI_IF_GAMEPORT1              0x10
#define   PCI_SUBCLASS_INPUT_OTHER      0x80

#define PCI_CLASS_DOCKING_STATION     0x0A
#define   PCI_SUBCLASS_DOCKING_GENERIC  0x00
#define   PCI_SUBCLASS_DOCKING_OTHER    0x80

#define PCI_CLASS_PROCESSOR           0x0B
#define   PCI_SUBCLASS_PROC_386         0x00
#define   PCI_SUBCLASS_PROC_486         0x01
#define   PCI_SUBCLASS_PROC_PENTIUM     0x02
#define   PCI_SUBCLASS_PROC_ALPHA       0x10
#define   PCI_SUBCLASS_PROC_POWERPC     0x20
#define   PCI_SUBCLASS_PROC_MIPS        0x30
#define   PCI_SUBCLASS_PROC_CO_PORC     0x40 ///< Co-Processor
#define   PCI_SUBCLASS_PROC_OTHER       0x80

#define PCI_CLASS_SERIAL              0x0C
#define   PCI_CLASS_SERIAL_FIREWIRE     0x00
#define     PCI_IF_1394                   0x00
#define     PCI_IF_1394_OPEN_HCI          0x10
#define   PCI_CLASS_SERIAL_ACCESS_BUS   0x01
#define   PCI_CLASS_SERIAL_SSA          0x02
#define   PCI_CLASS_SERIAL_USB          0x03
#define     PCI_IF_UHCI                   0x00
#define     PCI_IF_OHCI                   0x10
#define     PCI_IF_EHCI                   0x20
#define     PCI_IF_XHCI                   0x30
#define     PCI_IF_USB_OTHER              0x80
#define     PCI_IF_USB_DEVICE             0xFE
#define   PCI_CLASS_SERIAL_FIBRECHANNEL 0x04
#define   PCI_CLASS_SERIAL_SMB          0x05
#define   PCI_CLASS_SERIAL_IB           0x06
#define   PCI_CLASS_SERIAL_IPMI         0x07
#define     PCI_IF_IPMI_SMIC              0x00
#define     PCI_IF_IPMI_KCS               0x01 ///< Keyboard Controller Style
#define     PCI_IF_IPMI_BT                0x02 ///< Block Transfer
#define   PCI_CLASS_SERIAL_SERCOS       0x08
#define   PCI_CLASS_SERIAL_CANBUS       0x09
#define   PCI_CLASS_SERIAL_OTHER        0x80



#define PCI_CLASS_WIRELESS            0x0D
#define   PCI_SUBCLASS_IRDA             0x00
#define   PCI_SUBCLASS_IR               0x01
#define   PCI_SUBCLASS_RF               0x10
#define   PCI_SUBCLASS_BLUETOOTH        0x11
#define   PCI_SUBCLASS_BROADBAND        0x12
#define   PCI_SUBCLASS_ETHERNET_80211A  0x20
#define   PCI_SUBCLASS_ETHERNET_80211B  0x21
#define   PCI_SUBCLASS_WIRELESS_OTHER   0x80

#define PCI_CLASS_INTELLIGENT_IO      0x0E

#define PCI_CLASS_SATELLITE           0x0F
#define   PCI_SUBCLASS_TV               0x01
#define   PCI_SUBCLASS_AUDIO            0x02
#define   PCI_SUBCLASS_VOICE            0x03
#define   PCI_SUBCLASS_DATA             0x04
#define   PCI_SUBCLASS_SATELLITE_OTHER  0x80

#define PCI_SECURITY_CONTROLLER       0x10   ///< Encryption and decryption controller
#define   PCI_SUBCLASS_NET_COMPUT       0x00
#define   PCI_SUBCLASS_ENTERTAINMENT    0x10
#define   PCI_SUBCLASS_SECURITY_OTHER   0x80

#define PCI_CLASS_DPIO                0x11
#define   PCI_SUBCLASS_DPIO             0x00
#define   PCI_SUBCLASS_PERFORMANCE_COUNTERS          0x01
#define   PCI_SUBCLASS_COMMUNICATION_SYNCHRONIZATION 0x10
#define   PCI_SUBCLASS_MANAGEMENT_CARD               0x20
#define   PCI_SUBCLASS_DPIO_OTHER       0x80

#define PCI_CLASS_PROCESSING_ACCELERATOR  0x12

#endif


#endif /* INCLUDE_PCI_H_ */
