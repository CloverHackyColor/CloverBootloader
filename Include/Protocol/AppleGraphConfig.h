//********************************************************************
//	created:	28:8:2012   18:15
//	filename: 	AppleGraphConfig.h
//	author:		tiamo
//	purpose:	graph config
//********************************************************************

#ifndef _APPLE_GRAPH_CONFIG_H_
#define _APPLE_GRAPH_CONFIG_H_

//03622D6D-362A-4E47-9710-C238B23755C1
#define APPLE_GRAPH_CONFIG_PROTOCOL_GUID   {0x03622D6D, 0x362A, 0x4E47, {0x97, 0x10, 0xC2, 0x38, 0xB2, 0x37, 0x55, 0xC1}}

typedef struct _APPLE_GRAPH_CONFIG_PROTOCOL APPLE_GRAPH_CONFIG_PROTOCOL;

typedef EFI_STATUS (EFIAPI* RESTORE_CONFIG)(APPLE_GRAPH_CONFIG_PROTOCOL* This, UINT32 Param1, UINT32 Param2, VOID* Param3, VOID* Param4, VOID* Param5);

struct _APPLE_GRAPH_CONFIG_PROTOCOL
{
	UINT64	         Unknown0;
	RESTORE_CONFIG	 RestoreConfig;
};

extern EFI_GUID gAppleGraphConfigProtocolGuid;

#endif
