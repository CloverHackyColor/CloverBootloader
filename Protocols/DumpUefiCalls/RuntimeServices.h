/** @file

  RuntimeServices overrides module.

  By dmazar, 26/09/2012             

**/

#ifndef __DMP_RUNTIME_SERVICES_H__
#define __DMP_RUNTIME_SERVICES_H__


/** Original runtime services. */
extern EFI_RUNTIME_SERVICES gOrgRS;


EFI_STATUS EFIAPI
OvrRuntimeServices(EFI_RUNTIME_SERVICES	*RS);

EFI_STATUS EFIAPI
RestoreRuntimeServices(EFI_RUNTIME_SERVICES	*RS);


#endif // __DMP_RUNTIME_SERVICES_H__
