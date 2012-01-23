#ifndef __LIBSAIO_GMA_H
#define __LIBSAIO_GMA_H
#include "device_inject.h"

BOOLEAN setup_gma_devprop(pci_dt_t *gma_dev);

struct gma_gpu_t {
	UINT32 device;
	CHAR8 *name;
};




#endif /* !__LIBSAIO_GMA_H */
