//********************************************************************
//	created:	29:9:2012   13:49
//	filename: 	AppleSMC.h
//	author:		tiamo
//	purpose:	apple smc
//********************************************************************

#ifndef _APPLE_SMC_H_
#define _APPLE_SMC_H_

#define APPLE_SMC_PROTOCOL_GUID	{0x17407e5a, 0xaf6c, 0x4ee8, {0x98, 0xa8, 0x00, 0x21, 0x04, 0x53, 0xcd, 0xd9}}

EFI_FORWARD_DECLARATION(APPLE_SMC_PROTOCOL);

typedef EFI_STATUS (EFIAPI* APPLE_SMC_READ_DATA)(IN _APPLE_SMC_PROTOCOL* This, IN UINT32 DataId, IN UINT32 DataLength, OUT VOID* DataBuffer);

typedef struct _APPLE_SMC_PROTOCOL
{
	UINT64																	Signature;
	APPLE_SMC_READ_DATA											ReadData;
} APPLE_SMC_PROTOCOL;

extern EFI_GUID gAppleSMCProtocolGuid;

#endif
