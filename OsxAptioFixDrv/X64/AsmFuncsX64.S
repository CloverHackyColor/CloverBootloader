#------------------------------------------------------------------------------
#
# Some assembler helper functions plus boot.efi kernel jump callback
#
# by dmazar
#
#------------------------------------------------------------------------------

# C callback method called on jump to kernel after boot.efi finishes 
#.extern	KernelEntryPatchJumpBack

# saved 64bit state
ASM_GLOBAL ASM_PFX(SavedCR3)
ASM_GLOBAL ASM_PFX(SavedGDTR)
ASM_GLOBAL ASM_PFX(SavedIDTR)

# addresses of relocated MyAsmCopyAndJumpToKernel code - filled by PrepareJumpFromKernel()
ASM_GLOBAL ASM_PFX(MyAsmCopyAndJumpToKernel32Addr)
ASM_GLOBAL ASM_PFX(MyAsmCopyAndJumpToKernel64Addr)

# kernel entry address - filled by KernelEntryPatchJump()
ASM_GLOBAL ASM_PFX(AsmKernelEntry)

# params for kernel image relocation - filled by KernelEntryPatchJumpBack()
ASM_GLOBAL ASM_PFX(AsmKernelImageStartReloc)
ASM_GLOBAL ASM_PFX(AsmKernelImageStart)
ASM_GLOBAL ASM_PFX(AsmKernelImageSize)

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

	.p2align 3

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

	.p2align 3

# address of relocated MyAsmCopyAndJumpToKernel32 - 64 bit
MyAsmCopyAndJumpToKernel32AddrOff =			. - DataBase
ASM_PFX(MyAsmCopyAndJumpToKernel32Addr):	.quad 0

# address of relocated MyAsmCopyAndJumpToKernel64 - 64 bit
MyAsmCopyAndJumpToKernel64AddrOff =			. - DataBase
ASM_PFX(MyAsmCopyAndJumpToKernel64Addr):	.quad 0

# kernel entry - 64 bit
AsmKernelEntryOff =			. - DataBase
ASM_PFX(AsmKernelEntry):					.quad 0

#
# for copying kernel image from reloc block to proper mem place
#

# kernel image start in reloc block (source) - 64 bit
AsmKernelImageStartRelocOff =				. - DataBase
ASM_PFX(AsmKernelImageStartReloc):			.quad 0

# kernel image start (destination) - 64 bit
AsmKernelImageStartOff =					. - DataBase
ASM_PFX(AsmKernelImageStart):				.quad 0

# kernel image size - 64 bit
AsmKernelImageSizeOff =						. - DataBase
ASM_PFX(AsmKernelImageSize):				.quad 0


	.p2align 3

# GDT not used since we are reusing UEFI state
# but left here in case will be needed.
#
# GDR record
GDTROff = 		. - DataBase
GDTR:			.word L_GDT_LEN                # GDT limit
GDTR_BASE:		.quad 0                        # GDT base - needs to be set in code

	.p2align 3

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
L_GDT_LEN = . - GDT_BASE - 1


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
	sgdt	ASM_PFX(SavedGDTR)(%rip)
	sidt	ASM_PFX(SavedIDTR)(%rip)
	movq	%cr3, %rax
	movq	%rax, ASM_PFX(SavedCR3)(%rip)
	mov	%cs, SavedCS(%rip)
	mov	%ds, SavedDS(%rip)

	# pass DataBase to 32 bit code
	lea		DataBase(%rip), %rax
	movl	%eax, DataBaseAdr(%rip)

	# prepare MyAsmEntryPatchCode:
	# patch MyAsmEntryPatchCode with address of MyAsmJumpFromKernel
	lea		ASM_PFX(MyAsmJumpFromKernel)(%rip), %rax
	movl	%eax, MyAsmEntryPatchCodeJumpFromKernelPlaceholder(%rip)

	ret

#------------------------------------------------------------------------------
# Code that is used for patching kernel entry to jump back
# to our code (to MyAsmJumpFromKernel):
# - load ecx (rcx) with address to MyAsmJumpFromKernel
# - jump to MyAsmJumpFromKernel
# The same generated opcode must run properly in both 32 and 64 bit.
# 64 bit:
# - we must set rcx to 0 (upper 4 bytes) before loading ecx with address (lower 4 bytes of rcx)
# - this requires xor %rcx, %rcx
# - and that opcode contains 0x48 in front of 32 bit xor %ecx, %ecx
# 32 bit:
# - 0x48 opcode is dec %eax in 32 bit
# - and then we must inc %eax later if 32 bit is detected in MyAsmJumpFromKernel
#
# This code is patched with address of MyAsmJumpFromKernel
# (into MyAsmEntryPatchCodeJumpFromKernelPlaceholder)
# and then copied to kernel entry address by KernelEntryPatchJump()
#------------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(MyAsmEntryPatchCode)
ASM_PFX(MyAsmEntryPatchCode):

	.code32
	dec		%eax								# -> 48
	xor		%ecx, %ecx							# -> 31 C9
	.byte	0xb9								# movl	$0x11223344, %ecx -> B9 44 33 22 11
MyAsmEntryPatchCodeJumpFromKernelPlaceholder:
	.long	0x11223344
	call	*%ecx								# -> FF D1
#	jmp		*%ecx									# -> FF E1

#	.code64
#	xor		%rcx, %rcx							# -> 48 31 C9
#	movl	$0x11223344, %ecx					# -> B9 44 33 22 11
#	call	*%rcx								# -> FF D1
#	#jmp	*%rcx								# -> FF E1
ASM_GLOBAL ASM_PFX(MyAsmEntryPatchCodeEnd)
ASM_PFX(MyAsmEntryPatchCodeEnd):


#------------------------------------------------------------------------------
# MyAsmJumpFromKernel
# 
# Callback from boot.efi - this is where we jump when boot.efi jumps to kernel.
#
# - test if we are in 32 bit or in 64 bit
# - if 64 bit, then jump to MyAsmJumpFromKernel64
# - else just continue with MyAsmJumpFromKernel32
#------------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(MyAsmJumpFromKernel)
ASM_PFX(MyAsmJumpFromKernel):

	# writing in 32 bit, but code must run in 64 bit also
	.code32
	push	%eax					# save bootArgs pointer to stack
	movl 	$0xc0000080, %ecx		# EFER MSR number.
	rdmsr							# Read EFER.
	bt		$8, %eax				# Check if LME==1 -> CF=1.
	pop		%eax
	jc		MyAsmJumpFromKernel64	# LME==1 -> jump to 64 bit code
	# otherwise, continue with MyAsmJumpFromKernel32
	# but first add 1 to it since it was decremented in 32 bit
	# in MyAsmEntryPatchCode
	inc		%eax

	# test the above code in 64 bit - above 32 bit code gives opcode
	# that is equivalent to following in 64 bit
#.code64
#	push	%rax					# save bootArgs pointer to stack
#	movl 	$0xc0000080, %ecx		# EFER MSR number.
#	rdmsr							# Read EFER.
#	bt		$8, %eax				# Check if LME==1 -> CF=1.
#	pop		%rax
#	jnc		MyAsmJumpFromKernel64	# LME==1 -> jump to 64 bit code


#------------------------------------------------------------------------------
# MyAsmJumpFromKernel32
# 
# Callback from boot.efi in 32 bit mode.
# State is prepared for kernel: 32 bit, no paging, pointer to bootArgs in eax.
#------------------------------------------------------------------------------
MyAsmJumpFromKernel32:

	.code32

	# save bootArgs pointer to edi
	mov		%eax, %edi

	# load ebx with DataBase - we'll access our saved data with it
	.byte	0xBB				# mov ebx, OFFSET DataBase
DataBaseAdr:	.long 0
	
	# let's find out kernel entry point - we'll need it to jump back.
	# we are called with
	#   dec		%eax
	#   xor		%ecx, %ecx
	#   mov ecx, 0x11223344
	#   call ecx
	# and that left return addr on stack. those instructions
	# are 10 bytes long, and if we take address from stack and
	# substitute 10 from it, we will get kernel entry point.
	pop		%ecx
	sub		$10, %ecx
	# and save it
	movl	%ecx, AsmKernelEntryOff(%ebx)
	
	# lets save 32 bit state to be able to recover it later
	sgdt	SavedGDTR32Off(%ebx)
	sidt	SavedIDTR32Off(%ebx)
	mov	%cs, SavedCS32Off(%ebx)
	mov	%ds, SavedDS32Off(%ebx)
	movl	%esp, SavedESP32Off(%ebx)
	
	#
	# move to 64 bit mode ...
	#
	
	# FIXME: all this with interrupts enabled? no-no

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

	# set segments
	movw	SavedDSOff(%rbx), %ax
	movl	%eax, %ds
	# set up stack ...
	# not sure if needed, but lets set ss to ds
	movl	%eax, %ss  # disables interrupts for 1 instruction to load rsp
	# lets align the stack
#	movq	%rsp, %rax
#	andq	$0xfffffffffffffff0, %rax
#	movq	%rax, %rsp
	andq	$0xfffffffffffffff0, %rsp
	
	# call our C code
	# (calling conv.: always reserve place for 4 args on stack)
	# KernelEntryPatchJumpBack (rcx = rax = bootArgs, rdx = 0 = 32 bit kernel jump)
	movq	%rdi, %rcx
	xor		%rdx, %rdx
	push	%rdx
	push	%rdx
	push	%rdx
	push	%rcx

# TEST 64 bit jump
#	movq	%rdi, %rax
#	movq	ASM_PFX(AsmKernelEntry)(%rip), %rdx
#	jmp		*%rdx
# TEST end

	# KernelEntryPatchJumpBack should be EFIAPI
	# and rbx should not be changed by EFIAPI calling convention
	call	ASM_PFX(KernelEntryPatchJumpBack)
	#hlt	# uncomment to stop here for test
	# return value in rax is bootArgs pointer
	mov		%rax, %rdi

	#
	# time to go back to 32 bit
	#

	# FIXME: all this with interrupts enabled? no-no

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
	movl	%eax, %ds
	movl	%eax, %es
	movl	%eax, %fs
	movl	%eax, %gs
	movl	%eax, %ss  # disables interrupts for 1 instruction to load esp
	movl	SavedESP32Off(%ebx), %esp

	#
	# prepare vars for copying kernel to proper mem
	# and jump to kernel: set registers as needed
	# by MyAsmCopyAndJumpToKernel32
	#
	
	# boot args back from edi
	movl	%edi, %eax
	# kernel entry point
	movl	AsmKernelEntryOff(%ebx), %edx
	
	# source, destination and size for kernel copy
	movl	AsmKernelImageStartRelocOff(%ebx), %esi
	movl	AsmKernelImageStartOff(%ebx), %edi
	movl	AsmKernelImageSizeOff(%ebx), %ecx
	
	# address of relocated MyAsmCopyAndJumpToKernel32
	movl	MyAsmCopyAndJumpToKernel32AddrOff(%ebx), %ebx
	# note: ebx not valid as a pointer to DataBase any more
	
	#
	# jump to MyAsmCopyAndJumpToKernel32
	#
	jmp		*%ebx


_RETF64:
	.byte	0x48
_RETF32:
	lret


#------------------------------------------------------------------------------
# MyAsmJumpFromKernel64
#
# Callback from boot.efi in 64 bit mode.
# State is prepared for kernel: 64 bit, pointer to bootArgs in rax.
#------------------------------------------------------------------------------
MyAsmJumpFromKernel64:

.code64
	# let's find out kernel entry point - we'll need it to jump back.
	pop		%rcx
	sub		$10, %rcx
	# and save it
	movq	%rcx, ASM_PFX(AsmKernelEntry)(%rip)

	# call our C code
	# (calling conv.: always reserve place for 4 args on stack)
	# KernelEntryPatchJumpBack (rcx = rax = bootArgs, rdx = 1 = 64 bit kernel jump)
	movq	%rax, %rcx
	xor		%rdx, %rdx
	inc		%edx
	push	%rdx
	push	%rdx
	push	%rdx
	push	%rcx
	# KernelEntryPatchJumpBack should be EFIAPI
	call	ASM_PFX(KernelEntryPatchJumpBack)
	#hlt	# uncomment to stop here for test
	# return value in rax is bootArgs pointer

	#
	# prepare vars for copying kernel to proper mem
	# and jump to kernel: set registers as needed
	# by MyAsmCopyAndJumpToKernel64
	#

	# kernel entry point
	movq	ASM_PFX(AsmKernelEntry)(%rip), %rdx

	# source, destination and size for kernel copy
	movq	ASM_PFX(AsmKernelImageStartReloc)(%rip), %rsi
	movq	ASM_PFX(AsmKernelImageStart)(%rip), %rdi
	movq	ASM_PFX(AsmKernelImageSize)(%rip), %rcx
	
	# address of relocated MyAsmCopyAndJumpToKernel64
	movq	ASM_PFX(MyAsmCopyAndJumpToKernel64Addr)(%rip), %rbx

	#
	# jump to MyAsmCopyAndJumpToKernel64
	#
	jmp		*%rbx
	ret

	.p2align 3
#------------------------------------------------------------------------------
# MyAsmCopyAndJumpToKernel 
# 
# This is the last part of the code - it will copy kernel image from reloc
# block to proper mem place and jump to kernel.
# There are separate versions for 32 and 64 bit.
# This code will be relocated (copied) to higher mem by PrepareJumpFromKernel().
#------------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(MyAsmCopyAndJumpToKernel)
ASM_PFX(MyAsmCopyAndJumpToKernel):

#------------------------------------------------------------------------------
# MyAsmCopyAndJumpToKernel32
#
# Expects:
# EAX = address of boot args (proper address, not from reloc block)
# EDX = kernel entry point
# ESI = start of kernel image in reloc block (source)
# EDI = proper start of kernel image (destination)
# ECX = kernel image size in bytes
#------------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(MyAsmCopyAndJumpToKernel32)
ASM_PFX(MyAsmCopyAndJumpToKernel32):

	.code32
	
	#
	# we will move double words (4 bytes)
	# so ajust ECX to number of double words.
	# just in case ECX is not multiple of 4 - inc by 1
	#
	shrl	$2, %ecx
	incl	%ecx
	
	#
	# copy kernel image from reloc block to proper mem place.
	# all params should be already set:
	# ECX = number of double words
	# DS:ESI = source
	# ES:EDI = destination
	#
	cld								# direction is up
	rep movsl
	
	#
	# and finally jump to kernel:
	# EAX already contains bootArgs pointer,
	# and EDX contains kernel entry point
	#
	jmp		*%edx


#------------------------------------------------------------------------------
# MyAsmCopyAndJumpToKernel64
#
# Expects:
# RAX = address of boot args (proper address, not from reloc block)
# RDX = kernel entry point
# RSI = start of kernel image in reloc block (source)
# RDI = proper start of kernel image (destination)
# RCX = kernel image size in bytes
#------------------------------------------------------------------------------
	.p2align 3
ASM_GLOBAL ASM_PFX(MyAsmCopyAndJumpToKernel64)
ASM_PFX(MyAsmCopyAndJumpToKernel64):
	.code64

	#
	# we will move quad words (8 bytes)
	# so ajust RCX to number of double words.
	# just in case RCX is not multiple of 8 - inc by 1
	#
	shr	$3, %rcx
	inc	%rcx

	#
	# copy kernel image from reloc block to proper mem place.
	# all params should be already set:
	# RCX = number of double words
	# RSI = source
	# RDI = destination
	#
	cld								# direction is up
	rep movsq

	#
	# and finally jump to kernel:
	# RAX already contains bootArgs pointer,
	# and RDX contains kernel entry point
	#
	# hlt
	jmp		*%rdx

ASM_GLOBAL ASM_PFX(MyAsmCopyAndJumpToKernelEnd)
ASM_PFX(MyAsmCopyAndJumpToKernelEnd):

