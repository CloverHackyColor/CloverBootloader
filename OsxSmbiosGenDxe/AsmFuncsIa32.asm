;/** @file
;  Low level Ia32 do_cpuid and rdmsr64 (DOES NOT WORK!).
;
;**/

;.586
.model flat,C

data SEGMENT

data ENDS

text SEGMENT

;------------------------------------------------------------------------------
; void EFIAPI
; do_cpuid (
;   uint32_t selector,
;   uint32_t data[4]
;   )
;
; Abstract: Execute cpuid and stores results in data: data[0]=eax, data[1]=ebx, data[2]=ecx, data[3]=ecx
;
do_cpuid       PROC    PUBLIC

;
; params:
;   selector - on stack
;   data - pointer, on stack
;
; cpuid uses nonvolatile ebx so it must be saved and restore
;
                push 	ebp						; save non-volatile ebp
                mov 	ebp, esp				; ebp = esp - we'll read input params from stack
                push 	ebx						; save non-volatile regs we use
                mov 	eax, [ebp + 8]			; eax = 1st param: selector
                cpuid
                mov 	ebp, [ebp + 12]			; ebp = 2nd param: data 
                mov     [ebp], eax				; data[0] = eax
                mov     [ebp+4], ebx			; data[1] = ebx ...
                mov     [ebp+8], ecx
                mov     [ebp+12], edx
                pop     ebx
                pop     ebp
                ret
do_cpuid       ENDP

;------------------------------------------------------------------------------
; uint64_t EFIAPI
; rdmsr64 (
;   uint32_t msr
;   )
;
; Abstract: Loads the contents of a 64-bit model specific register (MSR) specified in the msr and returns it.
; DOES NOT WORK!
;
rdmsr64       PROC    PUBLIC

;
; params:
;   msr - on stack
; returns: value of the register (we pass it back in edx:eax in this 32bit version)
;
;
                mov 	ecx, [esp + 4]			; ecx = 1st param: msr
                rdmsr
				; return in edx:eax - high dword in edx, low dword in eax - as we need it for return value
                ret
rdmsr64       ENDP

text ENDS

END
