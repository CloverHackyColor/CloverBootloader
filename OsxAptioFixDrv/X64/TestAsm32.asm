;------------------------------------------------------------------------------
;
;  This is used just to produce 32 bit opcode and compare it with 64 bit.
;
;	in Win SDK prompt:
; 	SetEnv /x86
;	ml /c /Fl TestAsm32.asm
;------------------------------------------------------------------------------
	
XDTR	STRUCT
			Limit	DW	?
			Base	DQ	?
XDTR	ENDS

EXTERN	gKernelEntry: dword

	.model flat
	.data
	
; variables accessed from both 32 and 64 bit code
; need to have this exactly in this order
DataBase	LABEL PTR


; 64 bit state
SavedGDTROff	EQU $-DataBase
SavedGDTR		XDTR	<?>

SavedIDTROff	EQU $-DataBase
SavedIDTR		XDTR	<?>

SavedCR3Off		EQU $-DataBase
SavedCR3		DQ		?

SavedCSOff		EQU $-DataBase
SavedCS			DW		?

SavedDSOff		EQU $-DataBase
SavedDS			DW		?

; 32 bit state
SavedGDTR32Off	EQU $-DataBase
SavedGDTR32		XDTR	<?>

SavedIDTR32Off	EQU	$-DataBase
SavedIDTR32		XDTR	<?>

SavedCS32Off	EQU $-DataBase
SavedCS32		DW			?

SavedDS32Off	EQU $-DataBase
SavedDS32		DW			?

SavedESP32Off	EQU $-DataBase
SavedESP32		DD			?

; kernel entry - 32 bit
AsmKernelEntryOff				EQU $-DataBase
AsmKernelEntry					DD			?


;
; for copying kernel image from reloc block to proper mem place
;

; kernel image start in reloc block (source) - 32 bit
AsmKernelImageStartRelocOff		EQU $-DataBase
AsmKernelImageStartReloc		DD			?

; kernel image start (destination) - 32 bit
AsmKernelImageStartOff			EQU $-DataBase
AsmKernelImageStart				DD			?

; kernel image size - 32 bit
AsmKernelImageSizeOff			EQU $-DataBase
AsmKernelImageSize				DD			?

; address of relocated MyAsmCopyAndJumpToKernel32 - 32 bit
MyAsmCopyAndJumpToKernel32AddrOff		EQU $-DataBase
MyAsmCopyAndJumpToKernel32Addr		DD			?

	.code

MyAsmEntryPatchCodeSample32   PROC
	mov		ecx, 011223344h
	;push	ecx
	;ret
	jmp		ecx
MyAsmEntryPatchCodeSample32   ENDP


;------------------------------------------------------------------------------
; MyAsmJumpFromKernel32
; 
;------------------------------------------------------------------------------
MyAsmJumpFromKernel32   PROC
	
	;hlt	; uncomment to stop here for test
	; save bootArgs pointer to edi
	mov		edi, eax
	
	; load ebx with DataBase - we'll access our saved data with it
	db		0BBh				; mov ebx, OFFSET DataBase
DataBaseAdr	dd	0

	; let's find out kernel entry point - we'll need it to jump back.
	; we are called with
	;   mov ecx, 0x11223344
	;   call ecx
	; and that left return addr on stack. those instructions
	; are 7 bytes long, and if we take address from stack and
	; substitute 7 from it, we will get kernel entry point.
	pop		ecx							; 32 bit: pop ecx
	sub		ecx, 7
	; and save it
	mov		DWORD PTR [ebx + AsmKernelEntryOff], ecx

	; lets save 32 bit state to be able to recover it later
	; rbx is ebx in 32 bit
	sgdt	FWORD PTR [ebx + SavedGDTR32Off]
	sidt	FWORD PTR [ebx + SavedIDTR32Off]
	mov		WORD  PTR [ebx + SavedCS32Off], cs
	mov		WORD  PTR [ebx + SavedDS32Off], ds
	mov		DWORD PTR [ebx + SavedESP32Off], esp
	
	;
	; move to 64 bit mode ...
	;
	
	; load saved UEFI GDT, IDT
	; will become active after code segment is changed in long jump
	; rbx is ebx in 32 bit
	lgdt	FWORD PTR [ebx + SavedGDTROff]
	lidt	FWORD PTR [ebx + SavedIDTROff]

	; enable the 64-bit page-translation-table entries by setting CR4.PAE=1
	mov		eax, cr4
	bts		eax, 5
	mov		cr4, eax
	
	; set the long-mode page tables - reuse saved UEFI tables
	mov		eax, DWORD PTR [ebx +SavedCR3Off]
	mov		cr3, eax
	
	; enable long mode (set EFER.LME=1).
	mov 	ecx, 0c0000080h			; EFER MSR number.
	rdmsr							; Read EFER.
	bts		eax, 8					; Set LME=1.
	wrmsr							; Write EFER.

	; enable paging to activate long mode (set CR0.PG=1)
	mov		eax, cr0				; Read CR0.
	bts		eax, 31					; Set PG=1.
	mov		cr0, eax				; Write CR0.
	
	; jump to the 64-bit code segment
	mov		ax, WORD  PTR [ebx + SavedCSOff]
	push 	eax
	call	_RETF32
	
	;
	; aloha!
	; if there is any luck, we are in 64 bit mode now
	;
	;hlt	; uncomment to stop here for test
	
	; set segmens
	;mov		ax, WORD  PTR [ebx + SavedDSOff]
	;mov		ds, ax
	; set up stack ...
	; not sure if needed, but lets set ss to ds
	;mov		ss, ax
	; lets align the stack
	;mov		rax, rsp
	;and		rax, 0xfffffffffffffff0
	;mov		rsp, rax
	
	; call our C code with bootArgs as first arg (in rcx)
	;mov		rcx, rdi
	;push	rcx
	; KernelEntryPatchJumpBack should be EFIAPI
	; and rbx should not be changed by EFIAPI calling convention
	;call	KernelEntryPatchJumpBack
	;hlt	; uncomment to stop here for test
	; return value in rax is bootArgs pointer
	;mov		rdi, rax
	
	;
	; time to go back to 32 bit
	;
	
	; load saved 32 bit gdtr
	;lgdt	FWORD PTR [ebx + SavedGDTR32Off]
	; push saved cs and rip (with call) to stack and do retf
	;mov		ax, WORD  PTR [ebx + SavedCS32Off]
	;push 	rax
	;call	_RETF64

	;
	; ok, 32 bit opcode again from here
	;

	; disable paging (set CR0.PG=0)
	mov		eax, cr0				; Read CR0.
	btr		eax, 31					; Set PG=0.
	mov		cr0, eax				; Write CR0.
	
	; disable long mode (set EFER.LME=0).
	mov 	ecx, 0c0000080h			; EFER MSR number.
	rdmsr							; Read EFER.
	btr		eax, 8					; Set LME=0.
	wrmsr							; Write EFER.
	jmp		toNext
toNext:	
	;
	; we are in 32 bit protected mode, no paging
	;

	; now reload saved 32 bit state data
	lidt	FWORD PTR [ebx + SavedIDTR32Off]
	mov		ax, WORD  PTR [ebx + SavedDS32Off]
	mov		ss, ax
	mov		ds, ax
	mov		es, ax
	mov		fs, ax
	mov		gs, ax
	mov		esp, DWORD PTR [ebx + SavedESP32Off]
	
	;
	; prepare vars for copying kernel to proper mem
	; and jump to kernel: set registers as needed
	; by MyAsmCopyAndJumpToKernel32
	;
	
	; boot args back from edi
	mov		eax, edi
	; kernel entry point
	mov		edx, DWORD PTR [ebx + AsmKernelEntryOff]
	
	; source, destination and size for kernel copy
	mov		esi, DWORD PTR [ebx + AsmKernelImageStartRelocOff]
	mov		edi, DWORD PTR [ebx + AsmKernelImageStartOff]
	mov		ecx, DWORD PTR [ebx + AsmKernelImageSizeOff]
	
	; address of relocated MyAsmCopyAndJumpToKernel32
	mov		ebx, DWORD PTR [ebx + MyAsmCopyAndJumpToKernel32AddrOff]
	; note: ebx not valid as a pointer to DataBase any more
	
	;
	; jump to MyAsmCopyAndJumpToKernel32
	;
	jmp		DWORD PTR ebx			; jmp DWORD PTR ebx in 32 bit
	

_RETF64:
	DB	048h
_RETF32:
	retf

; the following is not used - it's here just for a reference
	; jump to the 64-bit code segment
	; as xnu kernel does it
	; example:
	; 07f8437a4: 68 28 00 00 cb = push 0cb000008h - last word on stack is segment 0x0008, 07f8437a8h contains CB opcode which is retf
	; 07f8437a9: e8 fa ff ff ff = call 07f8437a8h - push EIP on stack and continue with 07f8437a8h which is retf
	; retf does far return to next instr after 'call $-1'
	
	;push	((0cbh SHL 24) OR CODE64_SEL)
	;call	$-1

	
MyAsmJumpFromKernel32   ENDP


;------------------------------------------------------------------------------
; MyAsmCopyAndJumpToKernel32
; 
; This is the last part of the code - it will copy kernel image from reloc
; block to proper mem place and jump to kernel.
; It's 32 bit code and runs after switching back to 32 bit.
; This code will be relocated (copied) to higher mem by PrepareJumpFromKernel().
;
; Expects:
; EAX = address of boot args (proper address, not from reloc block)
; EDX = kernel entry point
; ESI = start of kernel image in reloc block (source)
; EDI = proper start of kernel image (destination)
; ECX = kernel image size in bytes
;------------------------------------------------------------------------------
		align 08h
MyAsmCopyAndJumpToKernel32   PROC
	
	;
	; we will move double words (4 bytes)
	; so ajust ECX to number of double words.
	; just in case ECX is not multiple of 4 - inc by 1
	;
	shr		ecx, 2
	inc		ecx
	
	;
	; copy kernel image from reloc block to proper mem place.
	; all params should be already set:
	; ECX = number of double words
	; DS:ESI = source
	; ES:EDI = destination
	;
	cld								; direction is up
	rep movsd
	
	;
	; and finally jump to kernel:
	; EAX already contains bootArgs pointer,
	; and EDX contains kernel entry point
	;
	;hlt
	jmp		DWORD PTR edx			; jmp DWORD PTR edx in 32 bit
MyAsmCopyAndJumpToKernel32   ENDP
MyAsmCopyAndJumpToKernel32End:


END
