/*++

  Copyright (c)  2007 - 2010 Intel Corporation. All rights reserved
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:

  CpuSupport.c
    
Abstract:

  Cpu related instruction for Video driver

Revision History:

--*/

//#include "Tiano.h"
//#include "CpuIA32.h"
#include <Library/BaseLib.h>


VOID
FlushDataCache (
  IN EFI_PHYSICAL_ADDRESS          Start,
  IN UINT64                        Length
  )
{
	AsmWbinvd ();
 // EfiWbinvd ();
}
