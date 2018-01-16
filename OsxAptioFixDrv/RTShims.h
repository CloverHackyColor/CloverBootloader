#ifndef RT_SHIMS_H_
#define RT_SHIMS_H_

extern UINTN gRTShimsDataStart;
extern UINTN gRTShimsDataEnd;

extern UINTN gGetVariable;
extern UINTN gGetNextVariableName;
extern UINTN gSetVariable;
extern UINTN gGetTime;
extern UINTN gSetTime;
extern UINTN gGetWakeupTime;
extern UINTN gSetWakeupTime;
extern UINTN gGetNextHighMonoCount;
extern UINTN gResetSystem;
extern UINTN gGetVariableBoot;

extern UINTN RTShimGetVariable;
extern UINTN RTShimGetNextVariableName;
extern UINTN RTShimSetVariable;
extern UINTN RTShimGetTime;
extern UINTN RTShimSetTime;
extern UINTN RTShimGetWakeupTime;
extern UINTN RTShimSetWakeupTime;
extern UINTN RTShimGetNextHighMonoCount;
extern UINTN RTShimResetSystem;

extern VOID *RTShims;

#endif // RT_SHIMS_H_
