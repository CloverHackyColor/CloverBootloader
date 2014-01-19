/** @file

  BootServices overrides module.

  By dmazar, 26/09/2012             

**/

#ifndef __DMP_BOOT_SERVICES_H__
#define __DMP_BOOT_SERVICES_H__


/** Flag is TRUE before ExitBootServices() is called. FALSE in runtime. */
extern BOOLEAN InBootServices;

#if CAPTURE_CONSOLE_OUTPUT >= 1
extern BOOLEAN InConsolePrint;
#endif

/** Installs our boot services overrides. */
EFI_STATUS EFIAPI
OvrBootServices(EFI_BOOT_SERVICES	*BS);


#endif // __DMP_BOOT_SERVICES_H__
