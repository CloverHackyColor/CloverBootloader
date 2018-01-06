BITS     64
DEFAULT  REL

SECTION .text

global ASM_PFX(gRTShimsDataStart)
ASM_PFX(gRTShimsDataStart):


global ASM_PFX(RTShimSetVariable)
ASM_PFX(RTShimSetVariable):
    push       rsi
    push       rbx
    sub        rsp, 0x38
    pushfq
    pop        rsi
    cli
    mov        rbx, cr0
    mov        rax, rbx
    and        rax, 0xfffffffffffeffff
    mov        cr0, rax
    mov        rax, qword [rsp+0x70]
    mov        qword [rsp+0x20], rax
    mov        rax, qword [ASM_PFX(gSetVariable)]
    call       rax
    test       ebx, 0x10000
    je         SET_SKIP_RESTORE_WP
    mov        cr0, rbx
SET_SKIP_RESTORE_WP:
    test       esi, 0x200
    je         SET_SKIP_RESTORE_INTR
    sti
SET_SKIP_RESTORE_INTR:
    add        rsp, 0x38
    pop        rbx
    pop        rsi
    ret

global ASM_PFX(RTShimGetVariable)
ASM_PFX(RTShimGetVariable):
    push       rsi
    push       rbx
    sub        rsp, 0x38
    pushfq
    pop        rsi
    cli
    mov        rbx, cr0
    mov        rax, rbx
    and        rax, 0xfffffffffffeffff
    mov        cr0, rax
    mov        rax, qword [rsp+0x70]
    mov        qword [rsp+0x20], rax
    mov        rax, qword [ASM_PFX(gGetVariable)]
    call       rax
    test       ebx, 0x10000
    je         GET_SKIP_RESTORE_WP
    mov        cr0, rbx
GET_SKIP_RESTORE_WP:
    test       esi, 0x200
    je         GET_SKIP_RESTORE_INTR
    sti
GET_SKIP_RESTORE_INTR:
    add        rsp, 0x38
    pop        rbx
    pop        rsi
    ret

global ASM_PFX(RTShimGetNextVariableName)
ASM_PFX(RTShimGetNextVariableName):
    nop
    push       rsi
    push       rbx
    sub        rsp, 0x28
    pushfq
    pop        rsi
    cli
    mov        rbx, cr0
    mov        rax, rbx
    and        rax, 0xfffffffffffeffff
    mov        cr0, rax
    mov        rax, qword [ASM_PFX(gGetNextVariableName)]
    call       rax
    test       ebx, 0x10000
    je         NEXT_SKIP_RESTORE_WP
    mov        cr0, rbx
NEXT_SKIP_RESTORE_WP:
    test       esi, 0x200
    je         NEXT_SKIP_RESTORE_INTR
    sti
NEXT_SKIP_RESTORE_INTR:
    add        rsp, 0x28
    pop        rbx
    pop        rsi
    ret


global ASM_PFX(gGetNextVariableName)
ASM_PFX(gGetNextVariableName):  dq  0

global ASM_PFX(gGetVariable)
ASM_PFX(gGetVariable):          dq  0

global ASM_PFX(gSetVariable)
ASM_PFX(gSetVariable):          dq  0


global ASM_PFX(gRTShimsDataEnd)
ASM_PFX(gRTShimsDataEnd):
