;------------------------------------------------------------------------------
;
; Some assembler helper functions plus boot.efi kernel jump callback
;
; by dmazar
;
;------------------------------------------------------------------------------

XDTR	STRUCT
			Limit	DW	?
			Base	DQ	?
XDTR	ENDS

; C callback method called on jump to kernel after boot.efi finishes 
EXTERN	KernelEntryPatchJumpBack:PROC

; saved 64bit state
PUBLIC SavedCR3
PUBLIC SavedGDTR
PUBLIC SavedIDTR
PUBLIC AsmKernelEntry

	.data
; variables accessed from both 32 and 64 bit code
; need to have this exactly in this order
DataBase:

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

; kernel entry
AsmKernelEntryOff	EQU $-DataBase
AsmKernelEntry		DD			?

        align 02h

; GDT not used since we are reusing UEFI state
; but left here in case will be needed.
;
; GDR record
GDTROff			EQU $-DataBase
GDTR			dw GDT_END - GDT_BASE - 1   ; GDT limit
GDTR_BASE		dq 0                        ; GDT base - needs to be set in code

        align 08h
; GDT
GDT_BASE:
; null descriptor
NULL_SEL		equ $-GDT_BASE			; 0x00
		dw 0			; limit 15:0
		dw 0			; base 15:0
		db 0			; base 23:16
		db 0			; type
		db 0			; limit 19:16, flags
		db 0			; base 31:24

; 64 bit code segment descriptor
CODE64_SEL		equ $-GDT_BASE			; 0x08
		dw 0FFFFh		; limit 0xFFFFF
		dw 0			; base 0
		db 0
		db 09Ah			; P=1 | DPL=00 | S=1 (User) # Type=A=1010: Code/Data=1 | C:Conforming=0 | R:Readable=1 | A:Accessed=0
		db 0AFh			; Flags=A=1010: G:Granularity=1 (4K) | D:Default Operand Size=0 (in long mode) | L:Long=1 (64 bit) | AVL=0
		db 0

; 32 bit and 64 bit data segment descriptor (in 64 bit almost all is ignored, so can be reused)
DATA_SEL		equ $-GDT_BASE			; 0x10
		dw 0FFFFh		; limit 0xFFFFF
		dw 0            ; base 0
		db 0
		db 092h         ; P=1 | DPL=00 | S=1 (User) # Type=2=0010: Code/Data=0 | E:Expand-Down=0 | W:Writable=1 | A:Accessed=0
		db 0CFh         ; Flags=C=1100: G:Granularity=1 (4K) | D/B=1 D not used when E=0, for stack B=1 means 32 bit stack | L:Long=0 not used | AVL=0
		db 0

; 32 bit code segment descriptor
CODE32_SEL		equ $-GDT_BASE			; 0x18
		dw 0FFFFh		; limit 0xFFFFF
		dw 0			; base 0
		db 0
		db 09Ah			; P=1 | DPL=00 | S=1 (User) # Type=A=1010: Code/Data=1 | C:Conforming=0 | R:Readable=1 | A:Accessed=0
		db 0CFh			; Flags=C=1100: G:Granularity=1 (4K) | D:Default Operand Size=0 (in long mode) | L:Long=0 (32 bit) | AVL=0
		db 0

GDT_END:
	.code

;------------------------------------------------------------------------------
; UINT64
; EFIAPI
; MyAsmReadSp (
;   VOID
;   );
;------------------------------------------------------------------------------
MyAsmReadSp   PROC
    mov     rax, rsp
	add		rax, 8			; return SP as caller see it
    ret
MyAsmReadSp   ENDP


;------------------------------------------------------------------------------
; VOID
; EFIAPI
; MyAsmPrepareJumpFromKernel (
;   );
;------------------------------------------------------------------------------
MyAsmPrepareJumpFromKernel   PROC
	; save 64 bit state
	sgdt	SavedGDTR
	sidt	SavedIDTR
	mov		rax, cr3
	mov		QWORD PTR SavedCR3, rax
	mov		WORD PTR SavedCS, cs
	mov		WORD PTR SavedDS, ds
	
	; pass DataBase to 32 bit code
	lea		rax, DataBase
	mov		DWORD PTR DataBaseAdr, eax;
	
	ret
MyAsmPrepareJumpFromKernel   ENDP

;------------------------------------------------------------------------------
; sample code that is used for patching kernel entry
; this compiles in 64 bit, but gives correct opcode for 32 bit
; (kernel starts in 32 bit)
;------------------------------------------------------------------------------
MyAsmEntryPatchCodeSample32   PROC
	mov		ecx, 0x11223344					; -> B9 44 33 22 11
	;jmp		rcx								; jmp ecx -> FF E1
	call	rcx								; call ecx -> FF D1
MyAsmEntryPatchCodeSample32   ENDP


;------------------------------------------------------------------------------
; MyAsmJumpFromKernel32
; 
; Callback from boot.efi - this is where we jump when boot.efi jumps to kernel.
; State is prepared for kernel: 32 bit, no paging, pointer to bootArgs in eax.
;
; MS 64 bit compiler generates only 64 bit opcode, but this function needs
; combined 32 and 64 bit code. Code can be written only with 64 bit instructions,
; but generated opcode must be valid 32 bit. This is a big issue.
; Well, I guess I know now how the guys in Intel are feeling when
; they have to work with MS tools on a similar code.
;
; Another problem is that it's not possible to access saved variables
; from 32 bit code (64 bit code has relative addresses, but 32 bit does not
; and depends on fixes during load and that is not happening sice
; generated code is marked 64 bit, or something similar).
; To overcome this, starting address of ou DataBase is passed in runtime - stored
; to DataBaseAdr below as an argument to mov. 
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
	pop		rcx							; 32 bit: pop ecx
	sub		ecx, 7
	; and save it
	mov		DWORD PTR [rbx + AsmKernelEntryOff], ecx

	; lets save 32 bit state to be able to recover it later
	; rbx is ebx in 32 bit
	sgdt	FWORD PTR [rbx + SavedGDTR32Off]
	sidt	FWORD PTR [rbx + SavedIDTR32Off]
	mov		WORD  PTR [rbx + SavedCS32Off], cs
	mov		WORD  PTR [rbx + SavedDS32Off], ds
	mov		DWORD PTR [rbx + SavedESP32Off], esp
	
	;
	; move to 64 bit mode ...
	;
	
	; load saved UEFI GDT, IDT
	; will become active after code segment is changed in long jump
	; rbx is ebx in 32 bit
	lgdt	FWORD PTR [rbx + SavedGDTROff]
	lidt	FWORD PTR [rbx + SavedIDTROff]

	; enable the 64-bit page-translation-table entries by setting CR4.PAE=1
	mov		rax, cr4
	bts		eax, 5
	mov		cr4, rax
	
	; set the long-mode page tables - reuse saved UEFI tables
	mov		eax, DWORD PTR [rbx +SavedCR3Off]
	mov		cr3, rax
	
	; enable long mode (set EFER.LME=1).
	mov 	ecx, 0c0000080h			; EFER MSR number.
	rdmsr							; Read EFER.
	bts		eax, 8					; Set LME=1.
	wrmsr							; Write EFER.

	; enable paging to activate long mode (set CR0.PG=1)
	mov		rax, cr0				; Read CR0.
	bts		eax, 31					; Set PG=1.
	mov		cr0, rax				; Write CR0.
	
	; jump to the 64-bit code segment
	mov		ax, WORD  PTR [rbx + SavedCSOff]
	push 	rax
	call	_RETF32
	
	;
	; aloha!
	; if there is any luck, we are in 64 bit mode now
	;
	;hlt	; uncomment to stop here for test
	
	; set segmens
	mov		ax, WORD  PTR [rbx + SavedDSOff]
	mov		ds, ax
	; set up stack ...
	; not sure if needed, but lets set ss to ds
	mov		ss, ax
	; lets align the stack
	mov		rax, rsp
	and		rax, 0xfffffffffffffff8
	mov		rsp, rax
	
	; call our C code with bootArgs as first arg (in rcx)
	mov		rcx, rdi
	push	rcx
	; KernelEntryPatchJumpBack should be EFIAPI
	; and rbx should not be changed by EFIAPI calling convention
	call	KernelEntryPatchJumpBack
	;hlt	; uncomment to stop here for test
	; return value in rax is bootArgs pointer
	mov		rdi, rax
	
	;
	; time to go back to 32 bit
	;
	
	; load saved 32 bit gdtr
	lgdt	FWORD PTR [rbx + SavedGDTR32Off]
	; push saved cs and rip (with call) to stack and do retf
	mov		ax, WORD  PTR [rbx + SavedCS32Off]
	push 	rax
	call	_RETF64

	;
	; ok, 32 bit opcode again from here
	;

	; disable paging (set CR0.PG=0)
	mov		rax, cr0				; Read CR0.
	btr		eax, 31					; Set PG=0.
	mov		cr0, rax				; Write CR0.
	
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
	lidt	FWORD PTR [rbx + SavedIDTR32Off]
	mov		ax, WORD  PTR [rbx + SavedDS32Off]
	mov		ss, ax
	mov		ds, ax
	mov		es, ax
	mov		fs, ax
	mov		gs, ax
	mov		esp, DWORD PTR [rbx + SavedESP32Off]
	
	;
	; set boot args pointer to eax and jump to kernel
	;
	mov		eax, edi
	mov		ebx, DWORD PTR [rbx + AsmKernelEntryOff]
	;hlt	; uncomment to stop here for test
	jmp		QWORD PTR rbx			; jmp DWORD PTR ebx in 32 bit
	

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
	push	((0cbh SHL 24) OR CODE64_SEL)
	call	$-1

	
MyAsmJumpFromKernel32   ENDP

END
