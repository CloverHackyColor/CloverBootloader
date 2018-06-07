//********************************************************************
//	created:	28:8:2012   18:10
//	filename: 	AppleDeviceControl.h
//	author:		tiamo
//	purpose:	apple device control
//********************************************************************

#ifndef _APPLE_DEVICE_CONTROL_H_
#define _APPLE_DEVICE_CONTROL_H_

#define APPLE_DEVICE_CONTROL_PROTOCOL_GUID	{0x8ece08d8, 0xa6d4, 0x430b, {0xa7, 0xb0, 0x2d, 0xf3, 0x18, 0xe7, 0x88, 0x4a}}

EFI_FORWARD_DECLARATION(APPLE_DEVICE_CONTROL_PROTOCOL);

typedef EFI_STATUS (EFIAPI* CONNECT_DISPLAY)();
/* but in other word it will be
 typedef EFI_STATUS (EFIAPI* RESTORE_CONFIG)(APPLE_GRAPH_CONFIG_PROTOCOL* This, UINT32 Param1, UINT32 Param2, VOID* Param3, VOID* Param4, VOID* Param5);
*/

typedef EFI_STATUS (EFIAPI* CONNECT_ALL)();

typedef struct _APPLE_DEVICE_CONTROL_PROTOCOL
{
	UINT64																Signature;   //0
	CONNECT_DISPLAY												ConnectDisplay; //8
	UINT64																Unknown2;  //0x10
	CONNECT_ALL														ConnectAll; //0x18
}APPLE_DEVICE_CONTROL_PROTOCOL;

extern EFI_GUID gAppleDeviceControlProtocolGuid;

#endif
