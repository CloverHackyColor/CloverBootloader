;/** @file
;  Low level x64 do_cpuid and rdmsr64.
;
;**/

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
;   selector - first param is in rcx
;   data - pointer, second param is in rdx
;
; cpuid uses non-volatile rbx so it must be saved and restore
;
                push    rbx
                mov     eax, ecx				; eax = ecx, rcx is our first parameter selector
                cpuid
                mov     [rdx], eax				; rdx = data
                mov     [rdx+4], ebx
                mov     [rdx+8], ecx
                mov     [rdx+12], edx
                pop     rbx
                ret
do_cpuid       ENDP

;------------------------------------------------------------------------------
; uint64_t EFIAPI
; rdmsr64 (
;   uint32_t msr
;   )
;
; Abstract: Loads the contents of a 64-bit model specific register (MSR) specified in the msr and returns it.
;
rdmsr64       PROC    PUBLIC

;
; params:
;   msr - register (we get it in rcx)
; returns: value of the register (returned in in rax)
;
				xor rax, rax						; rax = 0
                ; rcx is our first parameter selector - already in place for rdmsr
                rdmsr
				; return in edx:eax - we need it all in rax
				shl     rdx, 32						; rdmsr high dword (edx) in rdx
				or      rax, rdx					; join with low dword (eax) in rax - this is our return value
                ret
rdmsr64       ENDP

text ENDS

END
