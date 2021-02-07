/*
 * DataHubExt.h
 *
 */

#ifndef PLATFORM_DATAHUBEXT_H_
#define PLATFORM_DATAHUBEXT_H_

EFI_STATUS
EFIAPI
DataHubInstall (
                IN EFI_HANDLE           ImageHandle,
                IN EFI_SYSTEM_TABLE     *SystemTable
                );

#endif /* PLATFORM_DATAHUBEXT_H_ */
