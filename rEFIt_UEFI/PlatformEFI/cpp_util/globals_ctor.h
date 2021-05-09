

#ifdef __cplusplus
extern "C" {
#endif

#include <Uefi/UefiBaseType.h>


#ifdef __cplusplus
}
#endif

/*
 * NOTE : self.getSelfLoadedImage() must be initialized to call this.
 */
extern void construct_globals_objects(EFI_HANDLE ImageHandle);
