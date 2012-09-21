/*
 *  BlockRT.c
 *  
 *  dmazar, 21/09/2012
 *  
 *  Module for blocking runtime service GetVariable() when called from kernel.
 *  Used to enable sign in to iCloud on some boards.
 *  Until we find proper solution.
 *
 */


#include "Platform.h"


/** Chameleon's code for function that returns EFI_UNSUPPORTED
 *
 * 32 bit:
 * movl $0x80000003,%eax; ret
 *
 UINT8		UNSUPPORTEDRET_INSTRUCTIONS_32[] = {0xb8, 0x03, 0x00, 0x00, 0x80, 0xc3};
 *
 * 64 bit should be different:
 * movq $0x8000000000000003, %rax; ret
 UINT8		UNSUPPORTEDRET_INSTRUCTIONS_64[] = {0x48, 0xb8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc3};
 *
 */

/** Pointer to EfiRuntimeServicesCode area where UNSUPPORTEDRET is copied */
VOID		*RTUnsupportedRetFunc = NULL;



/** Template function that returns EFI_UNSUPPORTED.
 *  Will be copied to EfiRuntimeServicesCode
 *  area and installed as gRT->GetVariable().
 */
EFI_STATUS
EFIAPI
GetVariableTemplate(
	IN  CHAR16				*VariableName,
	IN  EFI_GUID			*VendorGuid,
	OUT UINT32				*Attributes,
	OPTIONAL IN OUT UINTN	*DataSize,
	OUT VOID				*Data
)
{
	//
	// Note: we can not do much here since this code
	// will be moved somwhere else in memory, by us first
	// and then also by boot.efi (RT defragmentation).
	// We can not use any Clover variables, constants ...
	// Plus we are loosing ability to call original
	// GetVariable() since it will be also moved during
	// RT defragmentation.
	//
	return EFI_UNSUPPORTED;
}

/** The size of GetVariableTemplate code.
 *  Well, we do not know that size, but we can be
 *  sure that it is smaller then 4K, and  the minimum
 *  RuntimeCode page that we can allocate is 4K anyway.
 */
#define		GET_VARIABLE_TEMPLATE_SIZE		4096

//
// Functions
//

/** Sets up BlockRT service: allocates EfiRuntimeServicesCode
 *  and installs function with UNSUPPORTEDRET_INSTRUCTIONS there.
 *  Since we are doing memory allocation, this must be called
 *  before ExitBootServices().
 */
EFI_STATUS BlockRTSetup()
{
	EFI_STATUS				Status;
	EFI_PHYSICAL_ADDRESS	BufferAddr = 0;

	
	Status = gBS->AllocatePages(
								AllocateAnyPages,
								EfiRuntimeServicesCode,
								//EFI_SIZE_TO_PAGES(sizeof(UNSUPPORTEDRET_INSTRUCTIONS)),
								EFI_SIZE_TO_PAGES(GET_VARIABLE_TEMPLATE_SIZE),
								&BufferAddr
								);
	if (Status == EFI_SUCCESS) {
		RTUnsupportedRetFunc = (VOID*)(UINTN)BufferAddr;
		//CopyMem(RTUnsupportedRetFunc, (VOID*)UNSUPPORTEDRET_INSTRUCTIONS, sizeof(UNSUPPORTEDRET_INSTRUCTIONS));
		CopyMem(RTUnsupportedRetFunc, (VOID*)(UINTN)GetVariableTemplate, GET_VARIABLE_TEMPLATE_SIZE);
		/* debug
		Print(L"RTUnsupportedRetFunc prepared at %p.\nSize = %d\n",
			  RTUnsupportedRetFunc,
			  //sizeof(UNSUPPORTEDRET_INSTRUCTIONS)
			  GET_VARIABLE_TEMPLATE_SIZE
			  );
		gBS->Stall(5000000);
		*/
	}
	
	return Status;
}

/** Installs our UNSUPPORTEDRET function as gRT->GetVariable().
 *  Shuld be called as late as possible (OnExitBootServices),
 *  because we want to block  GetVariable() calls for kernel,
 *  and not for boot services.
 */
VOID BlockRTInstall()
{
	
	if (RTUnsupportedRetFunc == NULL) {
		return;
	}
	
	gRT->GetVariable = (EFI_GET_VARIABLE)(UINTN)RTUnsupportedRetFunc;
	gRT->Hdr.CRC32 = 0;
	gBS->CalculateCrc32(gRT, gRT->Hdr.HeaderSize, &gRT->Hdr.CRC32);
	
	/* debug
	{
		EFI_STATUS				Status;
		UINT64					TestVar;
		UINTN					DataSize = sizeof(TestVar);
		
		Print(L"Checking gRT->GetVariable ...\n");
		Status = gRT->GetVariable(L"TestVar", &gEfiAppleBootGuid, NULL, &DataSize, &TestVar);
		Print(L"Status = %r\n", Status);
		gBS->Stall(10000000);
	}
	*/
}