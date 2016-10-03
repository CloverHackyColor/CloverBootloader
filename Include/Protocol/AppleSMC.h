//********************************************************************
//	created:	29:9:2012   13:49
//	filename: 	AppleSMC.h
//	author:		tiamo
//	purpose:	apple smc
//********************************************************************

#ifndef _APPLE_SMC_H_
#define _APPLE_SMC_H_

#define APPLE_SMC_PROTOCOL_GUID	{0x17407e5a, 0xaf6c, 0x4ee8, {0x98, 0xa8, 0x00, 0x21, 0x04, 0x53, 0xcd, 0xd9}}

//EFI_FORWARD_DECLARATION(APPLE_SMC_PROTOCOL);
//typedef struct _##x x
typedef struct _APPLE_SMC_PROTOCOL APPLE_SMC_PROTOCOL;

typedef EFI_STATUS (EFIAPI* APPLE_SMC_READ_DATA)(IN APPLE_SMC_PROTOCOL* This, IN UINT32 DataId, IN UINT32 DataLength, OUT VOID* DataBuffer);

typedef EFI_STATUS (EFIAPI* APPLE_SMC_WRITE_DATA)(IN APPLE_SMC_PROTOCOL* This, IN UINT32 DataId, IN UINT32 DataLength, IN VOID* DataBuffer);

struct _APPLE_SMC_PROTOCOL
{
	UINT64																	Signature;
	APPLE_SMC_READ_DATA											ReadData;
  APPLE_SMC_WRITE_DATA										WriteData;
};

extern EFI_GUID gAppleSMCProtocolGuid;

#endif
