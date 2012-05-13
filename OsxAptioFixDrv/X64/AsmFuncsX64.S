#------------------------------------------------------------------------------
#
# Some assembler helper functions plus boot.efi kernel jump callback
#
# by dmazar
#
#------------------------------------------------------------------------------

# C callback method called on jump to kernel after boot.efi finishes 
.extern	KernelEntryPatchJumpBack

# saved 64bit state
ASM_GLOBAL ASM_PFX(SavedCR3)
ASM_GLOBAL ASM_PFX(SavedGDTR)
ASM_GLOBAL ASM_PFX(SavedIDTR)
ASM_GLOBAL ASM_PFX(AsmKernelEntry)

	.data
# variables accessed from both 32 and 64 bit code
# need to have this exactly in this order
DataBase:

# 64 bit state
SavedGDTROff =		. - DataBase
ASM_PFX(SavedGDTR):		.word 0
						.quad 0

SavedIDTROff =		. - DataBase
ASM_PFX(SavedIDTR):		.word 0
						.quad 0

SavedCR3Off =			. - DataBase
ASM_PFX(SavedCR3):		.quad 0

SavedCSOff =			. - DataBase
SavedCS:				.word 0

SavedDSOff =			. - DataBase
SavedDS:				.word 0

# 32 bit state
SavedGDTR32Off =		. - DataBase
SavedGDTR32:			.word 0
						.quad 0		# 32 bit is W, L, but not sure about 32/64 bit ldgt/sdgt

SavedIDTR32Off =		. - DataBase
SavedIDTR32:			.word 0
						.quad 0

SavedCS32Off =			. - DataBase
SavedCS32:				.word 0

SavedDS32Off =			. - DataBase
SavedDS32:				.word 0

SavedESP32Off =			. - DataBase
SavedESP32:				.long 0

# kernel entry
AsmKernelEntryOff =			. - DataBase
ASM_PFX(AsmKernelEntry):	.long 0

	.align 2

# GDT not used since we are reusing UEFI state
# but left here in case will be needed.
#
# GDR record
GDTROff = 		. - DataBase
GDTR:			.word GDT_END - GDT_BASE - 1   # GDT limit
GDTR_BASE:		.quad 0                        # GDT base - needs to be set in code

	.align 8

# GDT table
GDT_BASE:
# null descriptor
NULL_SEL =		. - GDT_BASE			# 0x00
	.word 0			# limit 15:0
	.word 0			# base 15:0
	.byte 0			# base 23:16
	.byte 0			# type
	.byte 0			# limit 19:16, flags
	.byte 0			# base 31:24

# 64 bit code segment descriptor
CODE64_SEL =	. - GDT_BASE			# 0x08
	.word 0xFFFF		# limit 0xFFFFF
	.word 0				# base 0
	.byte 0
	.byte 0x9A			# P=1 | DPL=00 | S=1 (User) # Type=A=1010: Code/Data=1 | C:Conforming=0 | R:Readable=1 | A:Accessed=0
	.byte 0xAF			# Flags=A=1010: G:Granularity=1 (4K) | D:Default Operand Size=0 (in long mode) | L:Long=1 (64 bit) | AVL=0
	.byte 0

# 32 bit and 64 bit data segment descriptor (in 64 bit almost all is ignored, so can be reused)
DATA_SEL =		. - GDT_BASE			# 0x10
	.word 0xFFFF		# limit 0xFFFFF
	.word 0				# base 0
	.byte 0
	.byte 0x92			# P=1 | DPL=00 | S=1 (User) # Type=2=0010: Code/Data=0 | E:Expand-Down=0 | W:Writable=1 | A:Accessed=0
	.byte 0xCF			# Flags=C=1100: G:Granularity=1 (4K) | D/B=1 D not used when E=0, for stack B=1 means 32 bit stack | L:Long=0 not used | AVL=0
	.byte 0

# 32 bit code segment descriptor
CODE32_SEL =	. - GDT_BASE			# 0x18
	.word 0xFFFF		# limit 0xFFFFF
	.word 0				# base 0
	.byte 0
	.byte 0x9A			# P=1 | DPL=00 | S=1 (User) # Type=A=1010: Code/Data=1 | C:Conforming=0 | R:Readable=1 | A:Accessed=0
	.byte 0xCF			# Flags=C=1100: G:Granularity=1 (4K) | D:Default Operand Size=0 (in long mode) | L:Long=0 (32 bit) | AVL=0
	.byte 0

GDT_END:


	.text
	.code64

#------------------------------------------------------------------------------
# UINT64
# EFIAPI
# MyAsmReadSp (
#   VOID
#   );
#------------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(MyAsmReadSp)
ASM_PFX(MyAsmReadSp):
    movq	%rsp, %rax 
	add		$8, %rax			# return SP as caller see it
    ret

#------------------------------------------------------------------------------
# VOID
# EFIAPI
# MyAsmPrepareJumpFromKernel (
#   );
#------------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(MyAsmPrepareJumpFromKernel)
ASM_PFX(MyAsmPrepareJumpFromKernel):
	# save 64 bit state
	sgdt	ASM_PFX(SavedGDTR)
	sidt	ASM_PFX(SavedIDTR)
	movq	%cr3, %rax
	movq	%rax, ASM_PFX(SavedCR3)
	movw	%cs, SavedCS
	movw	%ds, SavedDS

	# pass DataBase to 32 bit code
	lea		DataBase, %rax
	movl	%eax, DataBaseAdr

	ret

#------------------------------------------------------------------------------
# sample code that is used for patching kernel entry
# this compiles in 64 bit, but gives correct opcode for 32 bit
# (kernel starts in 32 bit)
#------------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(MyAsmEntryPatchCodeSample32)
ASM_PFX(MyAsmEntryPatchCodeSample32):

	.code32

	movl	$0x11223344, %ecx					# -> B9 44 33 22 11
	#jmp		ecx								# jmp ecx -> FF E1
	call	*%ecx								# call ecx -> FF D1

#------------------------------------------------------------------------------
# MyAsmJumpFromKernel32
# 
# Callback from boot.efi - this is where we jump when boot.efi jumps to kernel.
# State is prepared for kernel: 32 bit, no paging, pointer to bootArgs in eax.
#
# Port from MS code - maybe some things could be better written for GCC.
#------------------------------------------------------------------------------


ASM_GLOBAL ASM_PFX(MyAsmJumpFromKernel32)
ASM_PFX(MyAsmJumpFromKernel32):

	.code32

	#hlt	# uncomment to stop here for test

	# save bootArgs pointer to edi
	mov		%eax, %edi

	# load ebx with DataBase - we'll access our saved data with it
	.byte	0xBB				# mov ebx, OFFSET DataBase
DataBaseAdr:	.long 0
	
	# let's find out kernel entry point - we'll need it to jump back.
	# we are called with
	#   mov ecx, 0x11223344
	#   call ecx
	# and that left return addr on stack. those instructions
	# are 7 bytes long, and if we take address from stack and
	# substitute 7 from it, we will get kernel entry point.
	pop		%ecx
	sub		$7, %ecx
	# and save it
	movl	%ecx, AsmKernelEntryOff(%ebx)
	
	# lets save 32 bit state to be able to recover it later
	sgdt	SavedGDTR32Off(%ebx)
	sidt	SavedIDTR32Off(%ebx)
	movw	%cs, SavedCS32Off(%ebx)
	movw	%ds, SavedDS32Off(%ebx)
	movl	%esp, SavedESP32Off(%ebx)
	
	#
	# move to 64 bit mode ...
	#
	
	# load saved UEFI GDT, IDT
	# will become active after code segment is changed in long jump
	lgdt	SavedGDTROff(%ebx)
	lidt	SavedIDTROff(%ebx)
	
	# enable the 64-bit page-translation-table entries by setting CR4.PAE=1
	movl	%cr4, %eax
	bts		$5, %eax
	movl	%eax, %cr4
	
	# set the long-mode page tables - reuse saved UEFI tables
	movl	SavedCR3Off(%ebx), %eax
	movl	%eax, %cr3

	# enable long mode (set EFER.LME=1).
	movl 	$0xc0000080, %ecx		# EFER MSR number.
	rdmsr							# Read EFER.
	bts		$8, %eax				# Set LME=1.
	wrmsr							# Write EFER.

	# enable paging to activate long mode (set CR0.PG=1)
	movl	%cr0, %eax				# Read CR0.
	bts		$31, %eax				# Set PG=1.
	movl	%eax, %cr0				# Write CR0.

	# jump to the 64-bit code segment
	movw	SavedCSOff(%ebx), %ax
	push 	%eax
	call	_RETF32

	#
	# aloha!
	# - if there is any luck, we are in 64 bit mode now
	#
	.code64
	
	#hlt	# uncomment to stop here for test

	# set segmens
	movw	SavedDSOff(%rbx), %ax
	movw	%ax, %ds
	# set up stack ...
	# not sure if needed, but lets set ss to ds
	movw	%ax, %ss
	# lets align the stack
	movq	%rsp, %rax
	andq	$0xfffffffffffffff8, %rax
	movq	%rax, %rsp
	
	# call our C code with bootArgs as first arg (in rcx)
	movq	%rdi, %rcx
	push	%rcx
	# KernelEntryPatchJumpBack should be EFIAPI
	# and rbx should not be changed by EFIAPI calling convention
	call	KernelEntryPatchJumpBack
	#hlt	# uncomment to stop here for test
	# return value in rax is bootArgs pointer
	mov		%rax, %rdi

	#
	# time to go back to 32 bit
	#

	# load saved 32 bit gdtr
	lgdt	SavedGDTR32Off(%rbx)
	# push saved cs and rip (with call) to stack and do retf
	movw	SavedCS32Off(%rbx), %ax
	push 	%rax
	call	_RETF64

	#
	# ok, 32 bit opcode again from here
	#
	.code32

	# disable paging (set CR0.PG=0)
	movl	%cr0, %eax				# Read CR0.
	btr		$31, %eax				# Set PG=0.
	movl	%eax, %cr0				# Write CR0.

	# disable long mode (set EFER.LME=0).
	movl 	$0xc0000080, %ecx		# EFER MSR number.
	rdmsr							# Read EFER.
	btr		$8, %eax				# Set LME=0.
	wrmsr							# Write EFER.
	jmp		toNext
	toNext:	
	#
	# we are in 32 bit protected mode, no paging
	#

	# now reload saved 32 bit state data
	lidt	SavedIDTR32Off(%ebx)
	movw	SavedDS32Off(%ebx), %ax
	movw	%ax, %ss
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movl	SavedESP32Off(%ebx), %esp

	#
	# set boot args pointer to eax and jump to kernel
	#
	movl	%edi, %eax
	movl	AsmKernelEntryOff(%ebx), %ebx
	#hlt	# uncomment to stop here for test
	jmp		*%ebx			# jmp DWORD PTR ebx in 32 bit


_RETF64:
	.byte	0x48
_RETF32:
	retf

