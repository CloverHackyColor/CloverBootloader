;------------------------------------------------------------------------------
;
;  This is used just to produce 32 bit opcode and compare it with 64 bit.
;
;	in Win SDK prompt:
; 	SetEnv /x86
;	ml /c /Fl AsmFuncs32.asm
;------------------------------------------------------------------------------
	
XDTR	STRUCT
			Limit	DW	?
			Base	DQ	?
XDTR	ENDS

EXTERN	gKernelEntry: dword

	.model flat
	.data
	
DataBase	LABEL PTR

SavedCR3Off		EQU $-DataBase
SavedCR3		DQ		?

SavedGDTR32Off	EQU $-DataBase
SavedGDTR32		XDTR	<?>

SavedIDTR32Off	EQU	$-DataBase
SavedIDTR32		XDTR	<?>

SavedCR332Off	EQU	$-DataBase
SavedCR332		DD	?

SavedCS32Off	EQU $-DataBase
SavedCS32		DW			?

SavedSS32Off	EQU $-DataBase
SavedSS32		DD			?

SavedESP32Off	EQU $-DataBase
SavedESP32		DD			?

	.code

MyAsmEntryPatchCodeSample32   PROC
	mov		ecx, 011223344h
	;push	ecx
	;ret
	jmp		ecx
MyAsmEntryPatchCodeSample32   ENDP

MyAsmEntryPatchCode2Sample32   PROC
	mov		ecx, 011223344h
	;push	ecx
	;ret
	call	ecx
MyAsmEntryPatchCode2Sample32   ENDP


;------------------------------------------------------------------------------
; MyAsmJump32
;------------------------------------------------------------------------------
MyAsmJumpFromKernel32   PROC
	pop		ecx
	sub		ecx, 7
	
	; lets save 32 bit state to be able to recover it later
	push	ss
	push	ds
	push	es
	push	fs
	push	gs
	sgdt	FWORD PTR [ebx + SavedGDTR32Off]
	sidt	FWORD PTR [ebx + SavedIDTR32Off]
	mov		WORD PTR [ebx + SavedCS32Off], cs
	mov		DWORD PTR [ebx + SavedESP32Off], esp
	; push eax - two dummy pushes to leave some gap between old and new stack
	; since we are messing with stack later
	push	eax
	push	eax

	; save bootArgs pointer to edi
	mov		edi, eax
	
	mov		ebx, 011223344h

	;sgdt	SavedGDTR_32
	;sidt	SavedIDTR_32
	;mov		eax, cr3
	;mov		DWORD PTR SavedCR3_32, eax
	;mov		[SavedCR3_32], eax

	; move to 64 bit mode ...
	
	; load saved UEFI GDT
	; will become active after code segment is changed in long jump
	lgdt	FWORD PTR [ebx]

	; prepare our long jump
	;lea		eax, in64bit
	;mov		DWORD PTR in64bitOff, eax
	;mov		ax, WORD PTR SavedCS
	;mov		WORD PTR in64bitSeg, ax
	
	; Enable the 64-bit page-translation-table entries by setting CR4.PAE=1
	mov		eax, cr4
	bts		eax, 5
	mov		cr4, eax
	
	; set the long-mode page tables - reuse saved UEFI tables
	mov		eax, DWORD PTR [ebx + 20]
	mov		cr3, eax
	
	; enable long mode (set EFER.LME=1).
	mov 	ecx, 0c0000080h			; EFER MSR number.
	rdmsr							; Read EFER.
	bts		eax, 8					; Set LME=1.
	wrmsr							; Write EFER.

	; enable paging to activate long mode (set CR0.PG=1)
	mov		eax, cr0				; Read CR0.
	bts		eax, 31					; Set PE=1.
	mov		cr0, eax				; Write CR0.
	
	; jump to the 64-bit code segment
	xor		eax, eax
	mov		ax, WORD PTR [ebx + 28]
	or		eax, (0cbh SHL 24)

	mov		eax, 0cbh SHL 24
	or		ax, WORD PTR [ebx + 28]
	push	eax
	call	$-1
	
	DB		66h, 0eah                   ; jmp far cs:in64bit
in64bitOff	DD	?
in64bitSeg	DW	?

in64bit:
	mov		eax, 08h
	mov		ds, eax
	hlt


	; load saved 32 bit gdtr
	lgdt	FWORD PTR [ebx + SavedGDTR32Off]
	; push saved cs and rip (with call) to stack and do retf
	mov		ax, WORD  PTR [ebx + SavedCS32Off]
	push 	eax
	call	_RETF64

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
	
	;
	; we are in 32 bit protected mode, no paging
	;
	
	; now reload saved 32 bit state data
	lidt	FWORD PTR [ebx + SavedIDTR32Off]
	mov		ax, WORD  PTR [ebx + SavedSS32Off]
	mov		ss, ax
	mov		ds, ax
	mov		es, ax
	mov		fs, ax
	mov		gs, ax
	mov		esp, DWORD PTR [ebx + SavedESP32Off]

	;
	; set boot args pointer to eax and jump to kernel
	;
	mov		eax, edi
	;mov		ebx, 02b8000h
	mov		ebx, DWORD PTR [gKernelEntry]
	jmp		DWORD PTR ebx			; jmp DWORD PTR ebx in 32 bit
	
	hlt


_RETF64:
	DB	048h
_RETF32:
	retf

	;
	; code from lion 64bit start
	;
	mov		eax, edx
	mov		edi, eax
	mov		esp, 00010c000h
	mov		eax, 0002ba040h
	mov		ebx, 0002b800ch
	jmp		DWORD PTR ebx



	;sub		eax, QWORD PTR gRellocBase				; rax contains pointer to boot args
	sub		eax, 010000000h
	
	;hlt	; test 1
	
	;hlt	; test 2
	;ret
	jmp		ecx

	add		ecx, 9
	mov		bx, ds
	mov		es, ebx
	mov		ebp, eax		;/* Move kernbootstruct to ebp */
	mov		ebx, eax		;/* get pointer to kernbootstruct */
	
	pop		gs
	pop		fs
	pop		es
	pop		ds
	pop		ss
MyAsmJumpFromKernel32   ENDP

END
