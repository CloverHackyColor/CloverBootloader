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

#ifndef GRUB_CPU_PXE_H
#define GRUB_CPU_PXE_H

#include <grub/types.h>

#define GRUB_PXENV_TFTP_OPEN			0x0020
#define GRUB_PXENV_TFTP_CLOSE			0x0021
#define GRUB_PXENV_TFTP_READ			0x0022
#define GRUB_PXENV_TFTP_READ_FILE		0x0023
#define GRUB_PXENV_TFTP_READ_FILE_PMODE		0x0024
#define GRUB_PXENV_TFTP_GET_FSIZE		0x0025

#define GRUB_PXENV_UDP_OPEN			0x0030
#define GRUB_PXENV_UDP_CLOSE			0x0031
#define GRUB_PXENV_UDP_READ			0x0032
#define GRUB_PXENV_UDP_WRITE			0x0033

#define GRUB_PXENV_START_UNDI			0x0000
#define GRUB_PXENV_UNDI_STARTUP			0x0001
#define GRUB_PXENV_UNDI_CLEANUP			0x0002
#define GRUB_PXENV_UNDI_INITIALIZE		0x0003
#define GRUB_PXENV_UNDI_RESET_NIC		0x0004
#define GRUB_PXENV_UNDI_SHUTDOWN		0x0005
#define GRUB_PXENV_UNDI_OPEN			0x0006
#define GRUB_PXENV_UNDI_CLOSE			0x0007
#define GRUB_PXENV_UNDI_TRANSMIT		0x0008
#define GRUB_PXENV_UNDI_SET_MCAST_ADDR		0x0009
#define GRUB_PXENV_UNDI_SET_STATION_ADDR	0x000A
#define GRUB_PXENV_UNDI_SET_PACKET_FILTER	0x000B
#define GRUB_PXENV_UNDI_GET_INFORMATION		0x000C
#define GRUB_PXENV_UNDI_GET_STATISTICS		0x000D
#define GRUB_PXENV_UNDI_CLEAR_STATISTICS	0x000E
#define GRUB_PXENV_UNDI_INITIATE_DIAGS		0x000F
#define GRUB_PXENV_UNDI_FORCE_INTERRUPT		0x0010
#define GRUB_PXENV_UNDI_GET_MCAST_ADDR		0x0011
#define GRUB_PXENV_UNDI_GET_NIC_TYPE		0x0012
#define GRUB_PXENV_UNDI_GET_IFACE_INFO		0x0013
#define GRUB_PXENV_UNDI_ISR			0x0014
#define	GRUB_PXENV_STOP_UNDI			0x0015
#define GRUB_PXENV_UNDI_GET_STATE		0x0015

#define GRUB_PXENV_UNLOAD_STACK			0x0070
#define GRUB_PXENV_GET_CACHED_INFO		0x0071
#define GRUB_PXENV_RESTART_DHCP			0x0072
#define GRUB_PXENV_RESTART_TFTP			0x0073
#define GRUB_PXENV_MODE_SWITCH			0x0074
#define GRUB_PXENV_START_BASE			0x0075
#define GRUB_PXENV_STOP_BASE			0x0076

#define GRUB_PXENV_EXIT_SUCCESS			0x0000
#define GRUB_PXENV_EXIT_FAILURE			0x0001

#define GRUB_PXENV_STATUS_SUCCESS				0x00
#define GRUB_PXENV_STATUS_FAILURE				0x01
#define GRUB_PXENV_STATUS_BAD_FUNC				0x02
#define GRUB_PXENV_STATUS_UNSUPPORTED				0x03
#define GRUB_PXENV_STATUS_KEEP_UNDI				0x04
#define GRUB_PXENV_STATUS_KEEP_ALL				0x05
#define GRUB_PXENV_STATUS_OUT_OF_RESOURCES			0x06
#define GRUB_PXENV_STATUS_ARP_TIMEOUT				0x11
#define GRUB_PXENV_STATUS_UDP_CLOSED				0x18
#define GRUB_PXENV_STATUS_UDP_OPEN				0x19
#define GRUB_PXENV_STATUS_TFTP_CLOSED				0x1A
#define GRUB_PXENV_STATUS_TFTP_OPEN				0x1B
#define GRUB_PXENV_STATUS_MCOPY_PROBLEM				0x20
#define GRUB_PXENV_STATUS_BIS_INTEGRITY_FAILURE			0x21
#define GRUB_PXENV_STATUS_BIS_VALIDATE_FAILURE			0x22
#define GRUB_PXENV_STATUS_BIS_INIT_FAILURE			0x23
#define GRUB_PXENV_STATUS_BIS_SHUTDOWN_FAILURE			0x24
#define GRUB_PXENV_STATUS_BIS_GBOA_FAILURE			0x25
#define GRUB_PXENV_STATUS_BIS_FREE_FAILURE			0x26
#define GRUB_PXENV_STATUS_BIS_GSI_FAILURE			0x27
#define GRUB_PXENV_STATUS_BIS_BAD_CKSUM				0x28
#define GRUB_PXENV_STATUS_TFTP_CANNOT_ARP_ADDRESS		0x30
#define GRUB_PXENV_STATUS_TFTP_OPEN_TIMEOUT			0x32

#define GRUB_PXENV_STATUS_TFTP_UNKNOWN_OPCODE			0x33
#define GRUB_PXENV_STATUS_TFTP_READ_TIMEOUT			0x35
#define GRUB_PXENV_STATUS_TFTP_ERROR_OPCODE			0x36
#define GRUB_PXENV_STATUS_TFTP_CANNOT_OPEN_CONNECTION		0x38
#define GRUB_PXENV_STATUS_TFTP_CANNOT_READ_FROM_CONNECTION	0x39
#define GRUB_PXENV_STATUS_TFTP_TOO_MANY_PACKAGES		0x3A
#define GRUB_PXENV_STATUS_TFTP_FILE_NOT_FOUND			0x3B
#define GRUB_PXENV_STATUS_TFTP_ACCESS_VIOLATION			0x3C
#define GRUB_PXENV_STATUS_TFTP_NO_MCAST_ADDRESS			0x3D
#define GRUB_PXENV_STATUS_TFTP_NO_FILESIZE			0x3E
#define GRUB_PXENV_STATUS_TFTP_INVALID_PACKET_SIZE		0x3F
#define GRUB_PXENV_STATUS_DHCP_TIMEOUT				0x51
#define GRUB_PXENV_STATUS_DHCP_NO_IP_ADDRESS			0x52
#define GRUB_PXENV_STATUS_DHCP_NO_BOOTFILE_NAME			0x53
#define GRUB_PXENV_STATUS_DHCP_BAD_IP_ADDRESS			0x54
#define GRUB_PXENV_STATUS_UNDI_INVALID_FUNCTION			0x60
#define GRUB_PXENV_STATUS_UNDI_MEDIATEST_FAILED			0x61
#define GRUB_PXENV_STATUS_UNDI_CANNOT_INIT_NIC_FOR_MCAST	0x62
#define GRUB_PXENV_STATUS_UNDI_CANNOT_INITIALIZE_NIC		0x63
#define GRUB_PXENV_STATUS_UNDI_CANNOT_INITIALIZE_PHY		0x64
#define GRUB_PXENV_STATUS_UNDI_CANNOT_READ_CONFIG_DATA		0x65
#define GRUB_PXENV_STATUS_UNDI_CANNOT_READ_INIT_DATA		0x66
#define GRUB_PXENV_STATUS_UNDI_BAD_MAC_ADDRESS			0x67
#define GRUB_PXENV_STATUS_UNDI_BAD_EEPROM_CHECKSUM		0x68
#define GRUB_PXENV_STATUS_UNDI_ERROR_SETTING_ISR		0x69
#define GRUB_PXENV_STATUS_UNDI_INVALID_STATE			0x6A
#define GRUB_PXENV_STATUS_UNDI_TRANSMIT_ERROR			0x6B
#define GRUB_PXENV_STATUS_UNDI_INVALID_PARAMETER		0x6C
#define GRUB_PXENV_STATUS_BSTRAP_PROMPT_MENU			0x74
#define GRUB_PXENV_STATUS_BSTRAP_MCAST_ADDR			0x76
#define GRUB_PXENV_STATUS_BSTRAP_MISSING_LIST			0x77
#define GRUB_PXENV_STATUS_BSTRAP_NO_RESPONSE			0x78
#define GRUB_PXENV_STATUS_BSTRAP_FILE_TOO_BIG			0x79
#define GRUB_PXENV_STATUS_BINL_CANCELED_BY_KEYSTROKE		0xA0
#define GRUB_PXENV_STATUS_BINL_NO_PXE_SERVER			0xA1
#define GRUB_PXENV_STATUS_NOT_AVAILABLE_IN_PMODE		0xA2
#define GRUB_PXENV_STATUS_NOT_AVAILABLE_IN_RMODE		0xA3
#define GRUB_PXENV_STATUS_BUSD_DEVICE_NOT_SUPPORTED		0xB0
#define GRUB_PXENV_STATUS_LOADER_NO_FREE_BASE_MEMORY		0xC0
#define GRUB_PXENV_STATUS_LOADER_NO_BC_ROMID			0xC1
#define GRUB_PXENV_STATUS_LOADER_BAD_BC_ROMID			0xC2
#define GRUB_PXENV_STATUS_LOADER_BAD_BC_RUNTIME_IMAGE		0xC3
#define GRUB_PXENV_STATUS_LOADER_NO_UNDI_ROMID			0xC4
#define GRUB_PXENV_STATUS_LOADER_BAD_UNDI_ROMID			0xC5
#define GRUB_PXENV_STATUS_LOADER_BAD_UNDI_DRIVER_IMAGE		0xC6
#define GRUB_PXENV_STATUS_LOADER_NO_PXE_STRUCT			0xC8
#define GRUB_PXENV_STATUS_LOADER_NO_PXENV_STRUCT		0xC9
#define GRUB_PXENV_STATUS_LOADER_UNDI_START			0xCA
#define GRUB_PXENV_STATUS_LOADER_BC_START			0xCB

#define GRUB_PXENV_PACKET_TYPE_DHCP_DISCOVER	1
#define GRUB_PXENV_PACKET_TYPE_DHCP_ACK		2
#define GRUB_PXENV_PACKET_TYPE_CACHED_REPLY	3

#define GRUB_PXE_BOOTP_REQ	1
#define GRUB_PXE_BOOTP_REP	2

#define GRUB_PXE_BOOTP_BCAST	0x8000

#if 1
#define GRUB_PXE_BOOTP_SIZE	(1024 + 236)	/* DHCP extended vendor field size.  */
#else
#define GRUB_PXE_BOOTP_SIZE	(312 + 236)	/* DHCP standard vendor field size.  */
#endif

#define GRUB_PXE_TFTP_PORT	69

#define GRUB_PXE_ERR_LEN	0xFFFFFFFF

#ifndef ASM_FILE

#define GRUB_PXE_SIGNATURE "PXENV+"

struct grub_pxenv
{
  grub_uint8_t signature[6];	/* 'PXENV+'.  */
  grub_uint16_t version;	/* MSB = major, LSB = minor.  */
  grub_uint8_t length;		/* structure length.  */
  grub_uint8_t checksum;	/* checksum pad.  */
  grub_uint32_t rm_entry;	/* SEG:OFF to PXE entry point.  */
  grub_uint32_t	pm_offset;	/* Protected mode entry.  */
  grub_uint16_t pm_selector;	/* Protected mode selector.  */
  grub_uint16_t stack_seg;	/* Stack segment address.  */
  grub_uint16_t	stack_size;	/* Stack segment size (bytes).  */
  grub_uint16_t bc_code_seg;	/* BC Code segment address.  */
  grub_uint16_t	bc_code_size;	/* BC Code segment size (bytes).  */
  grub_uint16_t	bc_data_seg;	/* BC Data segment address.  */
  grub_uint16_t	bc_data_size;	/* BC Data segment size (bytes).  */
  grub_uint16_t	undi_data_seg;	/* UNDI Data segment address.  */
  grub_uint16_t	undi_data_size;	/* UNDI Data segment size (bytes).  */
  grub_uint16_t	undi_code_seg;	/* UNDI Code segment address.  */
  grub_uint16_t	undi_code_size;	/* UNDI Code segment size (bytes).  */
  grub_uint32_t pxe_ptr;	/* SEG:OFF to !PXE struct.  */
} GRUB_PACKED;

struct grub_pxe_bangpxe
{
  grub_uint8_t signature[4];
#define GRUB_PXE_BANGPXE_SIGNATURE "!PXE"
  grub_uint8_t length;
  grub_uint8_t chksum;
  grub_uint8_t rev;
  grub_uint8_t reserved;
  grub_uint32_t undiromid;
  grub_uint32_t baseromid;
  grub_uint32_t rm_entry;
} GRUB_PACKED;

struct grub_pxenv_get_cached_info
{
  grub_uint16_t status;
  grub_uint16_t packet_type;
  grub_uint16_t buffer_size;
  grub_uint32_t buffer;
  grub_uint16_t buffer_limit;
} GRUB_PACKED;

struct grub_pxenv_tftp_open
{
  grub_uint16_t status;
  grub_uint32_t server_ip;
  grub_uint32_t gateway_ip;
  grub_uint8_t filename[128];
  grub_uint16_t tftp_port;
  grub_uint16_t packet_size;
} GRUB_PACKED;

struct grub_pxenv_tftp_close
{
  grub_uint16_t status;
} GRUB_PACKED;

struct grub_pxenv_tftp_read
{
  grub_uint16_t status;
  grub_uint16_t packet_number;
  grub_uint16_t buffer_size;
  grub_uint32_t buffer;
} GRUB_PACKED;

struct grub_pxenv_tftp_get_fsize
{
  grub_uint16_t status;
  grub_uint32_t server_ip;
  grub_uint32_t gateway_ip;
  grub_uint8_t filename[128];
  grub_uint32_t file_size;
} GRUB_PACKED;

struct grub_pxenv_udp_open
{
  grub_uint16_t status;
  grub_uint32_t src_ip;
} GRUB_PACKED;

struct grub_pxenv_udp_close
{
  grub_uint16_t status;
} GRUB_PACKED;

struct grub_pxenv_udp_write
{
  grub_uint16_t status;
  grub_uint32_t ip;
  grub_uint32_t gateway;
  grub_uint16_t src_port;
  grub_uint16_t dst_port;
  grub_uint16_t buffer_size;
  grub_uint32_t buffer;
} GRUB_PACKED;

struct grub_pxenv_udp_read
{
  grub_uint16_t status;
  grub_uint32_t src_ip;
  grub_uint32_t dst_ip;
  grub_uint16_t src_port;
  grub_uint16_t dst_port;
  grub_uint16_t buffer_size;
  grub_uint32_t buffer;
} GRUB_PACKED;

struct grub_pxenv_unload_stack
{
  grub_uint16_t status;
  grub_uint8_t reserved[10];
} GRUB_PACKED;

int EXPORT_FUNC(grub_pxe_call) (int func, void * data, grub_uint32_t pxe_rm_entry) __attribute__ ((regparm(3)));

extern struct grub_pxe_bangpxe *grub_pxe_pxenv;

void *
grub_pxe_get_cached (grub_uint16_t type);

#endif

#endif /* GRUB_CPU_PXE_H */
