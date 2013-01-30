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

//See RealMacs
/*
iMac13,1 01620006   DID=152 
MBA51    01660008   DID=166
MBA52    01660009   DID=166
MBP91    01660004	  DID=166
MBP92    01660003   DID=166
MBP101   01660002   DID=166
MBP102   01660001   DID=166 Intel HD Graphics 4000
MM62     0166000b   DID=166 
*/
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
