/**

  Allows to choose a random KASLR slide offset,
  when some offsets cannot be used.

  by Download-Fritz & vit9696

**/

#ifndef APTIOFIX_CUSTOM_SLIDE_H
#define APTIOFIX_CUSTOM_SLIDE_H

#include "BootArgs.h"

//
// Base kernel address.
//
#define BASE_KERNEL_ADDR       ((UINTN)0x100000)

//
// Slide offset per slide entry
//
#define SLIDE_GRANULARITY      ((UINTN)0x200000)

//
// Total possible number of KASLR slide offsets. 
//
#define TOTAL_SLIDE_NUM        256

/**
 * Ensures that the original csr-active-config is passed to the kernel,
 * and removes customised slide value for security reasons.
 * @return VOID
 */
VOID
RestoreCustomSlideOverrides (
  AMF_BOOT_ARGUMENTS  *BA
  );

/**
 * Patches boot.efi to support random and passed slide values in safe mode. 
 * @param ImageBase
 * @param ImageSize
 * @return VOID
 */
VOID
UnlockSlideSupportForSafeMode (
  UINT8  *ImageBase,
  UINTN  ImageSize
  );

/**
 * Custom gRT->GetVariable override used to return customised values
 * for boot-args and csr-active-config variables.
 * @return EFI_STATUS
 */
EFI_STATUS
EFIAPI
GetVariableCustomSlide (
  IN     CHAR16    *VariableName,
  IN     EFI_GUID  *VendorGuid,
  OUT    UINT32    *Attributes OPTIONAL,
  IN OUT UINTN     *DataSize,
  OUT    VOID      *Data
  );

/**
 * Checks whether the area overlaps with a possible kernel image area.
 * Returns TRUE if the given mem area overlaps, otherwise returns FALSE.
 * @return TRUE or FALSE
 */
BOOLEAN
OverlapsWithSlide (
  EFI_PHYSICAL_ADDRESS   Address,
  UINTN                  Size
  );

#endif // APTIOFIX_CUSTOM_SLIDE_H