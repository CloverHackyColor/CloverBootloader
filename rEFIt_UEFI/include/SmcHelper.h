//#ifndef SMC_HELPER_H
//#define SMC_HELPER_H


#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/MemLogLib.h>

#include <Protocol/AppleSMC.h>


EFI_STATUS SMCHelperInstall(EFI_HANDLE* Handle);

//#endif
