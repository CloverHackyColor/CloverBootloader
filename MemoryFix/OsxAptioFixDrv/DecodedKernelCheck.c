/**

  funcs for checking decoded kernel in memory
  
  dmazar

**/

#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>

#include "DecodedKernelCheck.h"
#include "AsmFuncs.h"
#include "BootArgs.h"

// monitoring AlocatePages
extern UINT32 gKernelEntry;

typedef struct {
	char			*segname;
	unsigned long	vmaddr;
	long			vmsize;
	long			filesize;
	unsigned long	adler32;
	unsigned char	ssample[8];
	unsigned char	esample[8];
} mySegData_t;
	
	 
//from Chameleon:
// i386:
mySegData_t mySegData[] = { // segment, vmaddr, vmsize, filesize, adler32
        {"__TEXT", 0x200000, 0x568000, 0x568000, 0x9cea7b0b,
            {0xce, 0xfa, 0xed, 0xfe, 0x07, 0x00, 0x00, 0x00},
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
        },
        {"__DATA", 0x800000, 0x83000, 0x41000, 0x28829ec4,
            {0x28, 0x64, 0x65, 0x66, 0x61, 0x75, 0x6c, 0x74},
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
        },
        {"__INITGDT", 0x106000, 0x1000, 0x1000, 0xfad20d6d,
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
        },
        {"__INITPT", 0x100000, 0x6000, 0x6000, 0xa58b95ca,
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0x83, 0x00, 0xe0, 0xff, 0x00, 0x00, 0x00, 0x00}
        },
        {"__DESC", 0x883000, 0x3000, 0x3000, 0x26fe0f95,
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
        },
        {"__VECTORS", 0x886000, 0x1000, 0x1000, 0xc6ae0a5c,
            {0x14, 0x3c, 0x2c, 0x00, 0x29, 0x3c, 0x2c, 0x00},
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
        },
        {"__HIB", 0x108000, 0x7000, 0x7000, 0x99a0f193,
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
        },
        {"__SLEEP", 0x107000, 0x1000, 0x1000, 0x10000001,
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
        },
        {"__KLD", 0x887000, 0x13000, 0x13000, 0xf6d84329,
            {0x55, 0x89, 0xe5, 0x56, 0x83, 0xec, 0x04, 0xb8},
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
        },
        {"__PRELINK_TEXT", 0x932000, 0x1564000, 0x1564000, 0x874e72cf,
            {0xce, 0xfa, 0xed, 0xfe, 0x07, 0x00, 0x00, 0x00},
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
        },
        {"__PRELINK_INFO", 0x1e96000, 0x158000, 0x1575b8, 0x483406a5,
            {0x3c, 0x64, 0x69, 0x63, 0x74, 0x3e, 0x3c, 0x6b},
            {0x3c, 0x2f, 0x64, 0x69, 0x63, 0x74, 0x3e, 0x00}
        },
        {"__LINKEDIT", 0x89a000, 0x97428, 0x97428, 0x7802b8c4,
            {0x04, 0x00, 0x00, 0x00, 0x0f, 0x08, 0x00, 0x00},
            {0x72, 0x65, 0x61, 0x6c, 0x6c, 0x6f, 0x63, 0x00}
        },
};
int mySegDataNum = 12;
//unsigned long rentry = 0x2c3db0;
unsigned long rentryx64 = 0x2b8000;
unsigned long rentry = 0x2b8000;
unsigned char rentrySample[8] = {0x66, 0x8c, 0xdb, 0x8e, 0xc3, 0x89, 0xc5, 0x89};
// i386
// end from Chameleon

/*
 * Adler32 from Chameleon
*/

unsigned long OSSwapHostToBigInt32(unsigned long int32) {
	return int32;
}

#define BASE 65521L /* largest prime smaller than 65536 */
#define NMAX 5000
// NMAX (was 5521) the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1

#define DO1(buf,i)  {s1 += buf[i]; s2 += s1;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);

unsigned long Adler32(unsigned char *buf, long len)
{
	unsigned long s1 = 1; // adler & 0xffff;
	unsigned long s2 = 0; // (adler >> 16) & 0xffff;
	unsigned long result;
	int k;
	
	while (len > 0) {
		k = len < NMAX ? len : NMAX;
		len -= k;
		while (k >= 16) {
			DO16(buf);
			buf += 16;
			k -= 16;
		}
		if (k != 0) do {
			s1 += *buf++;
			s2 += s1;
		} while (--k);
		s1 %= BASE;
		s2 %= BASE;
	}
	result = (s2 << 16) | s1;
	// result is in big endian
	return OSSwapHostToBigInt32(result);
}




void PrintSample(unsigned char *sample, int size) {
	int i;
	for (i = 0; i < size; i++) {
		Print(L" %02x", *sample);
		sample++;
	}
}


EFI_STATUS
EFIAPI
CheckDecodedSegment (
	mySegData_t				*sData
	)
{
	EFI_STATUS				Status;
	//UINTN					i;
	unsigned char			*p;
	unsigned long			adler32;
	long					size;
	
	Status = EFI_SUCCESS;
	
	p = (UINT8 *)(UINTN) (sData->vmaddr + gRelocBase);
	size = sData->vmsize > sData->filesize ? sData->filesize : sData->vmsize;
	adler32 = Adler32(p, size);
	
	if (adler32 != sData->adler32) {
		Status = EFI_CRC_ERROR;
	}
	
	Print(L"Segment: %a, vmaddr=%lx, vmsize=%lx, filesize=%lx, adler32=%lx, our adler=%lx\n",
		sData->segname, sData->vmaddr, sData->vmsize, sData->filesize, sData->adler32, adler32);
	//Print(L"cham ssample:"); PrintSample(sData->ssample, 8); Print(L"\n");
	//Print(L"uefi ssample:"); PrintSample(p, 8); Print(L"\n");
	
	return Status;
}


EFI_STATUS
EFIAPI
CheckDecodedKernel (
		VOID
	)
{
	EFI_STATUS				Status;
	EFI_STATUS				segStatus;
	int						i;
	unsigned char			*p;
	
	Status = EFI_SUCCESS;
	
	for (i = 0; i < mySegDataNum; i++) {
		segStatus = CheckDecodedSegment(&mySegData[i]);
		if (EFI_ERROR(segStatus)) {
			Status = segStatus;
		}
	}
	
	Print(L"rentry %lx:\ncham sample:", rentry); PrintSample(rentrySample, 8); Print(L"\n");
	p = (UINT8 *)(UINTN) (rentry + gRelocBase);
	Print(L"uefi sample:"); PrintSample(p, 8); Print(L"\n");
	
	Print(L"CheckDecodedKernel Status=%r\n");
	return Status;
}



VOID
EFIAPI
DumpStack (
		UINT64				RSP
	)
{
	unsigned char			*p;
	
	Print(L"DumpStack RSP=%lx\n", RSP);
	
	p = (UINT8 *)(UINTN) RSP;
	PrintSample(p, 64);
	Print(L"\n");
	
	return;
}
