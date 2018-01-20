BITS     64
DEFAULT  REL

%macro        ConstructShim 1
%if %1 > 5
    %error "At Most 5 Args Supported."
%endif
    push       rsi
    push       rbx
    sub        rsp, 0x28
    pushfq
    cli
    pop        rsi
    push       rax
    mov        rbx, cr0
    mov        rax, rbx
    and        rax, 0xfffffffffffeffff
    mov        cr0, rax
%if %1 > 4
    mov        rax, qword [rsp+0x68]
    mov        qword [rsp+0x28], rax
%endif
    pop        rax
    call       rax
    add        rsp, 0x28
    test       ebx, 0x10000
    je         .SKIP_RESTORE_WP
    mov        cr0, rbx
.SKIP_RESTORE_WP:
    pop        rbx
    test       si, 0x200
    pop        rsi
    je         .SKIP_RESTORE_INTR
    sti
.SKIP_RESTORE_INTR:
    ret
%endmacro

SECTION .text

ALIGN          8         ; to align the dqs

global ASM_PFX(gRTShimsDataStart)
ASM_PFX(gRTShimsDataStart):

global ASM_PFX(RTShimSetVariable)
ASM_PFX(RTShimSetVariable):
    mov        rax, qword [ASM_PFX(gSetVariable)]
    jmp        short FiveArgsShim

global ASM_PFX(RTShimGetVariable)
ASM_PFX(RTShimGetVariable):
    ; Until boot.efi virtualizes the pointers we use a custom wrapper.
    mov        rax, qword [ASM_PFX(gGetVariableBoot)]
    test       rax, rax
    jnz        .USE_CURRENT_FUNC
    mov        rax, qword [ASM_PFX(gGetVariable)]
.USE_CURRENT_FUNC:
    ;jmp       short FiveArgsShim
    ; fall through to FiveArgsShim

FiveArgsShim:
    ConstructShim 5

global ASM_PFX(RTShimGetNextVariableName)
ASM_PFX(RTShimGetNextVariableName):
    mov        rax, qword [ASM_PFX(gGetNextVariableName)]
    jmp        short FourArgsShim

global ASM_PFX(RTShimGetTime)
ASM_PFX(RTShimGetTime):
    mov        rax, qword [ASM_PFX(gGetTime)]
    jmp        short FourArgsShim

global ASM_PFX(RTShimSetTime)
ASM_PFX(RTShimSetTime):
    mov        rax, qword [ASM_PFX(gSetTime)]
    jmp        short FourArgsShim

global ASM_PFX(RTShimGetWakeupTime)
ASM_PFX(RTShimGetWakeupTime):
    mov        rax, qword [ASM_PFX(gGetWakeupTime)]
    jmp        short FourArgsShim

global ASM_PFX(RTShimSetWakeupTime)
ASM_PFX(RTShimSetWakeupTime):
    mov        rax, qword [ASM_PFX(gSetWakeupTime)]
    jmp        short FourArgsShim

global ASM_PFX(RTShimGetNextHighMonoCount)
ASM_PFX(RTShimGetNextHighMonoCount):
    mov        rax, qword [ASM_PFX(gGetNextHighMonoCount)]
    jmp        short FourArgsShim

global ASM_PFX(RTShimResetSystem)
ASM_PFX(RTShimResetSystem):
    mov        rax, qword [ASM_PFX(gResetSystem)]   ; Note - doesn't return!
    ;jmp       short FourArgsShim
    ; fall through to FourArgsShim

FourArgsShim:
    ConstructShim 4

ALIGN          8

global ASM_PFX(gGetNextVariableName)
ASM_PFX(gGetNextVariableName):  dq  0

global ASM_PFX(gGetVariable)
ASM_PFX(gGetVariable):          dq  0

global ASM_PFX(gSetVariable)
ASM_PFX(gSetVariable):          dq  0

global ASM_PFX(gGetTime)
ASM_PFX(gGetTime):                dq  0

global ASM_PFX(gSetTime)
ASM_PFX(gSetTime):                dq  0

global ASM_PFX(gGetWakeupTime)
ASM_PFX(gGetWakeupTime):          dq  0

global ASM_PFX(gSetWakeupTime)
ASM_PFX(gSetWakeupTime):          dq  0

global ASM_PFX(gGetNextHighMonoCount)
ASM_PFX(gGetNextHighMonoCount):   dq  0

global ASM_PFX(gResetSystem)
ASM_PFX(gResetSystem):            dq  0

global ASM_PFX(gGetVariableBoot)
ASM_PFX(gGetVariableBoot):        dq  0

global ASM_PFX(gRTShimsDataEnd)
ASM_PFX(gRTShimsDataEnd):
