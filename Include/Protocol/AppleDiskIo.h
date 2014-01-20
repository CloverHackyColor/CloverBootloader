//********************************************************************
//	created:	15:9:2012   23:53
//	filename: 	AppleDiskIo.h
//	author:		tiamo
//	purpose:	apple disk io protocol
//********************************************************************

#ifndef _APPLE_DISK_IO_H_
#define _APPLE_DISK_IO_H_

#define APPLE_DISK_IO_PROTOCOL_GUID											{0x5b27263b, 0x9083, 0x415e, 0x88, 0x9e, 0x64, 0x32, 0xca, 0xa9, 0xb8, 0x13}

EFI_FORWARD_DECLARATION(APPLE_DISK_IO_PROTOCOL);

typedef EFI_STATUS (EFIAPI* APPLE_DISK_READ)(IN APPLE_DISK_IO_PROTOCOL* This, IN UINT32 MediaId, IN UINT64 LBA, IN UINTN BufferSize, OUT VOID* Buffer);
typedef EFI_STATUS (EFIAPI* APPLE_DISK_FLUSH)(IN APPLE_DISK_IO_PROTOCOL* This);

typedef struct _APPLE_DISK_IO_PROTOCOL
{
	APPLE_DISK_READ															ReadDisk;
	APPLE_DISK_FLUSH														Flush;
}APPLE_DISK_IO_PROTOCOL;

extern EFI_GUID gAppleDiskIoProtocolGuid;

#endif
