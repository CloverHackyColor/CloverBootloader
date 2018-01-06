#ifndef RT_SHIMS_H_
#define RT_SHIMS_H_

extern UINTN gRTShimsDataStart;
extern UINTN gRTShimsDataEnd;

extern UINTN gGetVariable;
extern UINTN gGetNextVariableName;
extern UINTN gSetVariable;

extern UINTN RTShimGetVariable;
extern UINTN RTShimGetNextVariableName;
extern UINTN RTShimSetVariable;

extern VOID *RTShims;

#endif // RT_SHIMS_H_
