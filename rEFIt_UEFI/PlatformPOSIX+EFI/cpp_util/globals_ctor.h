
/*
 * NOTE : Nothing needed when compiling on posix platform
 */

// Because we are building a "normal" app and not an embedded binary,
// globals objects will be constructed by the C++ startup initialization library
// NOTE : it means that it's slightly different than when we compiled for EFI
void construct_globals_objects(EFI_HANDLE ImageHandle) {};
