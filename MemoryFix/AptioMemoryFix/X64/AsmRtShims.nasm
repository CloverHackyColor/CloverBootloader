;------------------------------------------------------------------------------
;
; Functions to wrap RuntimeServices code that needs to be written to
;
; by Download-Fritz & vit9696
; refactored by Zenith432
;------------------------------------------------------------------------------

BITS     64
DEFAULT  REL

; Constructs a shim with write protection patch avoiding
; a conflict with XNU W^X mapping.
; The argument marks a number of args the func takes (1~5).
%macro        ConstructShim 1
%if %1 > 5
    %error "At Most 5 Args Supported."
%endif
    cmp        qword [ASM_PFX(gRequiresWriteUnprotect)], 0
    jz         .SKIP_WRITE_UNPROTECT
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
.SKIP_WRITE_UNPROTECT:
    jmp        rax
%endmacro

; Redirects Boot prefixed variables from gBootVariableGuid
; to gRedirectVariableGuid.
; Variable name is assumed to be in %rcx.
; Guid is assumed to be in %rdx.
; Temporary registers: %rax.
%macro        PerformBootVariableRedirect 0
    ; Check if we have variable redirection enabled.
    mov        rax, qword [ASM_PFX(gBootVariableRedirect)]
    test       rax, rax
    jz         .SKIP_BOOT_VARIABLE_REDIRECT
    ; Compare whether GUID matches gBootVariableGuid
    mov        rax, qword [rdx]
    cmp        qword [ASM_PFX(gBootVariableGuid)], rax
    jnz        .SKIP_BOOT_VARIABLE_REDIRECT
    mov        rax, qword [rdx+8]
    cmp        qword [ASM_PFX(gBootVariableGuid)+8], rax
    jnz        .SKIP_BOOT_VARIABLE_REDIRECT
    ; Compare whether variable prefix matches Boot
    mov        ax, word [rcx]
    cmp        ax, 'B'
    jnz        .SKIP_BOOT_VARIABLE_REDIRECT
    mov        ax, word [rcx+2]
    cmp        ax, 'o'
    jnz        .SKIP_BOOT_VARIABLE_REDIRECT
    mov        ax, word [rcx+4]
    cmp        ax, 'o'
    jnz        .SKIP_BOOT_VARIABLE_REDIRECT
    mov        ax, word [rcx+6]
    cmp        ax, 't'
    jnz        .SKIP_BOOT_VARIABLE_REDIRECT
    ; This is a Boot prefixed variable from gBootVariableGuid.
    ; Redirect it to gRedirectVariableGuid.
    lea        rdx, [ASM_PFX(gRedirectVariableGuid)]
.SKIP_BOOT_VARIABLE_REDIRECT:
%endmacro

SECTION .text

ALIGN          8         ; to align the dqs

global ASM_PFX(gRtShimsDataStart)
ASM_PFX(gRtShimsDataStart):

global ASM_PFX(RtShimsReturnInvalidParameter)
ASM_PFX(RtShimsReturnInvalidParameter):
    mov        rax, 0x8000000000000002
    ret

global ASM_PFX(RtShimsReturnSecurityViolation)
ASM_PFX(RtShimsReturnSecurityViolation):
    mov        rax, 0x800000000000001A
    ret

global ASM_PFX(RtShimSetVariable)
ASM_PFX(RtShimSetVariable):
    ; For performance and simplicity do initial validation ourselves.
    test       rcx, rcx
    jz         ASM_PFX(RtShimsReturnInvalidParameter)     ; VariableName is NULL
    test       rdx, rdx
    jz         ASM_PFX(RtShimsReturnInvalidParameter)     ; VendorGuid is NULL
.INITIAL_VALIDATION_OVER:
    PerformBootVariableRedirect
    ; Once boot.efi virtualizes the pointers we should protect read-only
    ; variables from writing.
    mov        rax, qword [ASM_PFX(gGetVariableOverride)]
    test       rax, rax
    jnz        .SKIP_ACCESS_CHECK
    ; We have a virtualized pointer, so we also need to protect write-only
    ; variables from reading. Compare VendorGuid against gReadOnlyVariableGuid
    ; and return EFI_SECURITY_VIOLATION on equals.
    mov        rax, qword [rdx]
    cmp        qword [ASM_PFX(gReadOnlyVariableGuid)], rax
    jnz        .SKIP_ACCESS_CHECK
    mov        rax, qword [rdx+8]
    cmp        qword [ASM_PFX(gReadOnlyVariableGuid)+8], rax
    jz         ASM_PFX(RtShimsReturnSecurityViolation)
.SKIP_ACCESS_CHECK:
    mov        rax, qword [ASM_PFX(gSetVariable)]
    jmp        FiveArgsShim

global ASM_PFX(RtShimGetVariable)
ASM_PFX(RtShimGetVariable):
    ; For performance and simplicity do initial validation ourselves.
    test       rcx, rcx
    jz         ASM_PFX(RtShimsReturnInvalidParameter)     ; VariableName is NULL
    test       rdx, rdx
    jz         ASM_PFX(RtShimsReturnInvalidParameter)     ; VendorGuid is NULL
    test       r9, r9
    jz         ASM_PFX(RtShimsReturnInvalidParameter)     ; DataSize is NULL
    cmp        qword [rsp+0x28], 0
    jnz        .INITIAL_VALIDATION_OVER                   ; Data is not NULL
    mov        rax, qword [r9]
    test       rax, rax
    jnz        ASM_PFX(RtShimsReturnInvalidParameter)     ; Data is NULL and *DataSize is not 0
.INITIAL_VALIDATION_OVER:
    PerformBootVariableRedirect
    ; Once boot.efi virtualizes the pointers we should protect write-only
    ; variables from reading. Prior to that a custom wrapper is used.
    mov        rax, qword [ASM_PFX(gGetVariableOverride)]
    test       rax, rax
    jnz        .SKIP_ACCESS_CHECK_WRAPPER
    ; We have a virtualized pointer, so we also need to protect write-only
    ; variables from reading. Compare VendorGuid against gWriteOnlyVariableGuid
    ; and return EFI_SECURITY_VIOLATION on equals.
    mov        rax, qword [rdx]
    cmp        qword [ASM_PFX(gWriteOnlyVariableGuid)], rax
    jnz        .SKIP_ACCESS_CHECK_INTERNAL
    mov        rax, qword [rdx+8]
    cmp        qword [ASM_PFX(gWriteOnlyVariableGuid)+8], rax
    jz         ASM_PFX(RtShimsReturnSecurityViolation)
.SKIP_ACCESS_CHECK_INTERNAL:
    mov        rax, qword [ASM_PFX(gGetVariable)]
.SKIP_ACCESS_CHECK_WRAPPER:
    ;jmp       short FiveArgsShim
    ; fall through to FiveArgsShim

FiveArgsShim:
    ConstructShim 5

global ASM_PFX(RtShimGetNextVariableName)
ASM_PFX(RtShimGetNextVariableName):
    ; TODO: I am not sure whether we need GetNextVariableName support
    ; for boot variable routing... Probably good enough without it.
    mov        rax, qword [ASM_PFX(gGetNextVariableName)]
    jmp        short FourArgsShim

global ASM_PFX(RtShimGetTime)
ASM_PFX(RtShimGetTime):
    ; On old AMI firmwares (like the one found in GA-Z87X-UD4H) there is a chance
    ; of getting 2047 (EFI_UNSPECIFIED_TIMEZONE) from GetTime. This is valid,
    ; yet is disliked by some software including but not limited to UEFI Shell.
    ; See the patch: https://lists.01.org/pipermail/edk2-devel/2018-May/024534.html
    ; As a workaround we make sure this does not happen at all.
    push       rsi
    push       rbx
    push       rcx                    ; Save the original EFI_TIME pointer
    sub        rsp, 0x20
    pushfq
    cli
    pop        rsi
    mov        rbx, cr0
    mov        rax, rbx
    and        rax, 0xfffffffffffeffff
    mov        cr0, rax
    mov        rax, qword [ASM_PFX(gGetTime)]
    call       rax
    add        rsp, 0x20
    test       ebx, 0x10000
    je         .SKIP_RESTORE_WP
    mov        cr0, rbx
.SKIP_RESTORE_WP:
    pop        rbx                    ; load saved EFI_TIME pointer
    test       rax, rax               ; check for EFI_ERROR
    js         .SKIP_CORRECT_TIMEZONE
    cmp        word [rbx + 12], 2047  ; offsetof(EFI_TIME, TimeZone)
    jnz        .SKIP_CORRECT_TIMEZONE
    mov        word [rbx + 12], 0     ; default to UTC
.SKIP_CORRECT_TIMEZONE:
    pop        rbx
    test       si, 0x200
    pop        rsi
    je         .SKIP_RESTORE_INTR
    sti
.SKIP_RESTORE_INTR:
    ret

global ASM_PFX(RtShimSetTime)
ASM_PFX(RtShimSetTime):
    mov        rax, qword [ASM_PFX(gSetTime)]
    jmp        short FourArgsShim

global ASM_PFX(RtShimGetWakeupTime)
ASM_PFX(RtShimGetWakeupTime):
    mov        rax, qword [ASM_PFX(gGetWakeupTime)]
    jmp        short FourArgsShim

global ASM_PFX(RtShimSetWakeupTime)
ASM_PFX(RtShimSetWakeupTime):
    mov        rax, qword [ASM_PFX(gSetWakeupTime)]
    jmp        short FourArgsShim

global ASM_PFX(RtShimGetNextHighMonoCount)
ASM_PFX(RtShimGetNextHighMonoCount):
    mov        rax, qword [ASM_PFX(gGetNextHighMonoCount)]
    jmp        short FourArgsShim

global ASM_PFX(RtShimResetSystem)
ASM_PFX(RtShimResetSystem):
    mov        rax, qword [ASM_PFX(gResetSystem)]   ; Note - doesn't return!
    ;jmp       short FourArgsShim
    ; fall through to FourArgsShim

FourArgsShim:
    ConstructShim 4

ALIGN          8

global ASM_PFX(gRequiresWriteUnprotect)
ASM_PFX(gRequiresWriteUnprotect): dq  0

global ASM_PFX(gBootVariableRedirect)
ASM_PFX(gBootVariableRedirect):   dq  0

global ASM_PFX(gGetNextVariableName)
ASM_PFX(gGetNextVariableName):    dq  0

global ASM_PFX(gGetVariable)
ASM_PFX(gGetVariable):            dq  0

global ASM_PFX(gSetVariable)
ASM_PFX(gSetVariable):            dq  0

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

global ASM_PFX(gGetVariableOverride)
ASM_PFX(gGetVariableOverride):    dq  0

global ASM_PFX(gBootVariableGuid)
ASM_PFX(gBootVariableGuid):       times 2 dq 0

global ASM_PFX(gRedirectVariableGuid)
ASM_PFX(gRedirectVariableGuid):   times 2 dq 0

global ASM_PFX(gReadOnlyVariableGuid)
ASM_PFX(gReadOnlyVariableGuid):   times 2 dq 0

global ASM_PFX(gWriteOnlyVariableGuid)
ASM_PFX(gWriteOnlyVariableGuid):  times 2 dq 0

global ASM_PFX(gRtShimsDataEnd)
ASM_PFX(gRtShimsDataEnd):
