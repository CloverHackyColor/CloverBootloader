#ifndef __LIBSAIO_GMA_H
#define __LIBSAIO_GMA_H
#include "device_inject.h"

BOOLEAN setup_gma_devprop(LOADER_ENTRY *Entry, pci_dt_t *gma_dev);

struct gma_gpu_t {
	UINT32 device;
	CONST CHAR8 *name;
};

/*
Chameleon
uint32_t ram = (((getVBEVideoRam() + 512) / 1024) + 512) / 1024;
uint32_t ig_platform_id;
switch (ram)
{
  case 96:
    ig_platform_id = 0x01660000; // 96mb
    break;    
  case 64:
    ig_platform_id = 0x01660009; // 64mb
    break;
  case 32:
    ig_platform_id = 0x01620005; // 32mb
    break;
  default:
*/    
    
#endif /* !__LIBSAIO_GMA_H */
