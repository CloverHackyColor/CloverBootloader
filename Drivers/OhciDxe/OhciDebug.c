/** @file
  This file provides the information dump support for OHCI when in debug mode.

Copyright(c) 2013 Intel Corporation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in
the documentation and/or other materials provided with the
distribution.
* Neither the name of Intel Corporation nor the names of its
contributors may be used to endorse or promote products derived
from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**/


#include "Ohci.h"


/*++

  Print the data of ED and the TDs attached to the ED

  @param  Uhc                   Pointer to OHCI private data 
  @param  Ed                    Pointer to a ED to free
  @param  Td                    Pointer to the Td head
  
  @retval EFI_SUCCESS           ED 

**/
EFI_STATUS
OhciDumpEdTdInfo (
  IN USB_OHCI_HC_DEV      *Uhc,
  IN ED_DESCRIPTOR        *Ed,
  IN TD_DESCRIPTOR        *Td,
  BOOLEAN                 Stage
  )
{
  UINT32                  Index;

  if (Stage) {
    DEBUG ((EFI_D_INFO, "\n Before executing command\n"));
  }else{
    DEBUG ((EFI_D_INFO, "\n after executing command\n"));
  }
  if (Ed != NULL) {
    DEBUG ((EFI_D_INFO, "\nED Address:%x, ED buffer:\n", (UINT32)(UINTN)Ed));
    DEBUG ((EFI_D_INFO, "DWord0  :TD Tail :TD Head :Next ED\n"));
    for (Index = 0; Index < sizeof (ED_DESCRIPTOR)/4; Index ++) {
      DEBUG ((EFI_D_INFO, "%8x ", *((UINT32*)(Ed) + Index)  ));
    }
    DEBUG ((EFI_D_INFO, "\nNext TD buffer:\n", (UINT32)(UINTN)Td));
  }
  while (Td != NULL) {
    if (Td->Word0.DirPID == TD_SETUP_PID) {
      DEBUG ((EFI_D_INFO, "\nSetup PID "));
    }else if (Td->Word0.DirPID == TD_OUT_PID) {
      DEBUG ((EFI_D_INFO, "\nOut PID "));
    }else if (Td->Word0.DirPID == TD_IN_PID) {
      DEBUG ((EFI_D_INFO, "\nIn PID "));
    }else if (Td->Word0.DirPID == TD_NODATA_PID) {
      DEBUG ((EFI_D_INFO, "\nNo data PID "));
    }
    DEBUG ((EFI_D_INFO, "TD Address:%x, TD buffer:\n", (UINT32)(UINTN)Td));
    DEBUG ((EFI_D_INFO, "DWord0  :CuBuffer:Next TD :Buff End:Next TD :DataBuff:ActLength\n"));
    for (Index = 0; Index < sizeof (TD_DESCRIPTOR)/4; Index ++) {
      DEBUG ((EFI_D_INFO, "%8x ", *((UINT32*)(Td) + Index)  ));
    }
    DEBUG ((EFI_D_INFO, "\nCurrent TD Data buffer(size%d)\n", (UINT32)Td->ActualSendLength));
    for (Index = 0; Index < Td->ActualSendLength; Index ++) {
      DEBUG ((EFI_D_INFO, "%2x ", *(UINT8 *)(UINTN)(Td->DataBuffer+Index) ));
    }
  Td = (TD_DESCRIPTOR *)(UINTN)(Td->NextTDPointer);
  }
  DEBUG ((EFI_D_INFO, "\n TD buffer End\n", (UINT32)(UINTN)Td));

  return EFI_SUCCESS;
}






