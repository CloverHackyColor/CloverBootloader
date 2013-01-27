#ifndef __LIBSAIO_GMA_H
#define __LIBSAIO_GMA_H
#include "device_inject.h"

BOOLEAN setup_gma_devprop(pci_dt_t *gma_dev);

struct gma_gpu_t {
	UINT32 device;
	CHAR8 *name;
};

/*
 //information from Henry http://blog.stuffedcow.net/2012/07/intel-hd4000-qeci-acceleration/
 // reference to ElNono_
AAPL,ig-platform-id	Memory (MB)	Pipes	Ports	Comment
01660000	96	3	4
01660001	96	3	4
01660002	64	3	1	No DVI
01660003	64	2	2
01660004	32	3	1	No DVI
01620005	32	2	3
01620006	0	  0	0	No display
01620007	0	  0	0	No display
01660008	64	3	3
01660009	64	3	3
0166000a	32	2	3
0166000b	32	2	3
*/


#endif /* !__LIBSAIO_GMA_H */
