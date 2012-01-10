/*
 * Copyright (c) 1999, 2000
 * Intel Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 * 
 *    This product includes software developed by Intel Corporation and
 *    its contributors.
 * 
 * 4. Neither the name of Intel Corporation or its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */


#ifndef _EFIDBG_H_
#define _EFIDBG_H_

#include "eficontext.h"
#include "efiser.h"

typedef struct _DEBUGPORT_16550_CONFIG_DATA {
        UINT32							PortAddress;
        UINT64                          BaudRate;
    	UINT32               			ReceiveFifoDepth;
    	UINT32               			Timeout;
        UINT8                           Parity;
        UINT8                           DataBits;
        UINT8                           StopBits;
	    UINT32                       	ControlMask;
        BOOLEAN							RtsCtsEnable;		// RTS, CTS control
} DEBUGPORT_16550_CONFIG_DATA;

typedef struct _DEBUGPORT_16550_DEVICE_PATH {
        EFI_DEVICE_PATH                 Header;
        DEBUGPORT_16550_CONFIG_DATA		ConfigData;
} DEBUGPORT_16550_DEVICE_PATH;

typedef union {
    EFI_DEVICE_PATH                     DevPath;
    DEBUGPORT_16550_DEVICE_PATH         Uart;
    // add new types of debugport device paths to this union...
} DEBUGPORT_DEV_PATH;


//
// Debug Support protocol {2755590C-6F3C-42FA-9EA4-A3BA543CDA25}
//

#define DEBUG_SUPPORT_PROTOCOL \
{ 0x2755590C, 0x6F3C, 0x42fa, 0x9E, 0xA4, 0xA3, 0xBA, 0x54, 0x3C, 0xDA, 0x25 }


typedef UINTN EXCEPTION_TYPE;

typedef
VOID
(*EXCEPTION_HANDLER) (
	IN EXCEPTION_TYPE ExceptionType,
    IN SYSTEM_CONTEXT *SystemContext
    );

typedef
EFI_STATUS
(EFIAPI *EFI_REGISTER_TIMER_TICK_CALLBACK) (
    IN struct _EFI_DEBUG_SUPPORT_INTERFACE  *This,
    IN EXCEPTION_HANDLER	                TimerTickCallback
    );

typedef
EFI_STATUS
(EFIAPI *EFI_REGISTER_EXCEPTION_HANDLER) (
    IN     struct _EFI_DEBUG_SUPPORT_INTERFACE  *This,
    IN     EXCEPTION_HANDLER                    ExceptionHandler,
    IN     EXCEPTION_TYPE                       ExceptionType
    );

typedef
EFI_STATUS
(EFIAPI *EFI_IP_CALL_TRACE) (
    IN     struct _EFI_DEBUG_SUPPORT_INTERFACE  *This
    );


#define EFI_DEBUG_SUPPORT_INTERFACE_REVISION     0x00010000

typedef struct _EFI_DEBUG_SUPPORT_INTERFACE {
    UINT32                          	Revision;
    EFI_REGISTER_TIMER_TICK_CALLBACK	RegisterTimerTickCallback;
    EFI_REGISTER_EXCEPTION_HANDLER  	RegisterExceptionHandler;
    EFI_IP_CALL_TRACE               	IpCallTrace;
} EFI_DEBUG_SUPPORT_INTERFACE;


//
// Debugport io protocol {EBA4E8D2-3858-41EC-A281-2647BA9660D0}
//

#define DEBUGPORT_IO_PROTOCOL \
{ 0XEBA4E8D2, 0X3858, 0X41EC, 0XA2, 0X81, 0X26, 0X47, 0XBA, 0X96, 0X60, 0XD0 }
 

typedef
EFI_STATUS
(EFIAPI *EFI_DEBUGPORT_IO_RESET) (
    IN struct _EFI_DEBUGPORT_IO_INTERFACE  	*This
    );

typedef
EFI_STATUS
(EFIAPI *EFI_DEBUGPORT_IO_READ) (
    IN     struct _EFI_DEBUGPORT_IO_INTERFACE	*This,
    IN OUT UINTN                    		*BufferSize,
    OUT VOID                         		*Buffer
    );

typedef
EFI_STATUS
(EFIAPI *EFI_DEBUGPORT_IO_WRITE) (
    IN     struct _EFI_DEBUGPORT_IO_INTERFACE *This,
    IN OUT UINTN                    		*BufferSize,
    IN VOID                         		*Buffer
    );

#define EFI_DEBUGPORT_IO_INTERFACE_REVISION   0x00010000

typedef struct _EFI_DEBUGPORT_IO_INTERFACE {
    UINT32                          		Revision;
    EFI_DEBUGPORT_IO_READ					Read;
    EFI_DEBUGPORT_IO_WRITE					Write;
    EFI_DEBUGPORT_IO_RESET					Reset;
} EFI_DEBUGPORT_IO_INTERFACE;


//
// Debugport UART16550 control protocol {628EA978-4C26-4605-BC02-A42A496917DD}
//

#define DEBUGPORT_UART16550_CONTROL_PROTOCOL \
{ 0X628EA978, 0X4C26, 0X4605, 0XBC, 0X2, 0XA4, 0X2A, 0X49, 0X69, 0X17, 0XDD }
 
// Note: The definitions for EFI_PARITY_TYPE, EFI_STOP_BITS_TYPE, and 
// SERIAL_IO_MODE are included from efiser.h

typedef
EFI_STATUS
(EFIAPI *EFI_UART16550_SET_ATTRIBUTES) (
    IN struct _EFI_DEBUGPORT_UART16550_CONTROL_INTERFACE  	*This,
    IN UINT64                       	BaudRate,
    IN UINT32                       	ReceiveFifoDepth,
    IN UINT32                       	Timeout,
    IN EFI_PARITY_TYPE       			Parity,
    IN UINT8                        	DataBits,
    IN EFI_STOP_BITS_TYPE    			StopBits
    );

typedef
EFI_STATUS
(EFIAPI *EFI_UART16550_SET_CONTROL_BITS) (
    IN struct _EFI_DEBUGPORT_UART16550_CONTROL_INTERFACE  	*This,
    IN UINT32                       	Control
    );

typedef
EFI_STATUS
(EFIAPI *EFI_UART16550_GET_CONTROL_BITS) (
    IN struct _EFI_DEBUGPORT_UART16550_CONTROL_INTERFACE	*This,
    OUT UINT32                      	*Control
    );

#define EFI_DEBUGPORT_UART16550_CONTROL_INTERFACE_REVISION   0x00010000

typedef struct _EFI_DEBUGPORT_UART16550_CONTROL_INTERFACE {
    UINT32                          	Revision;
	EFI_UART16550_SET_ATTRIBUTES		SetAttributes;
	EFI_UART16550_SET_CONTROL_BITS		SetControl;
	EFI_UART16550_GET_CONTROL_BITS 		GetControl;
	DEBUGPORT_16550_CONFIG_DATA			*Mode;
} EFI_DEBUGPORT_UART16550_CONTROL_INTERFACE;
        

#define DEVICE_PATH_DEBUGPORT DEBUGPORT_IO_PROTOCOL
        
#endif /* _EFIDBG_H_ */
