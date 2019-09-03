/*
 * BootDuet - Replacement boot program for DUET.
 * Copyright 2011 Miguel Lopes Santos Ramos <mail@miguel.ramos.name>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * See:
 *	- README for a description of this project
 *	- INSTALL for installation instructions
 */

/*
 * The Master Boot Record (MBR) will have loaded this program into physical
 * address 0x7c00 and jumped into 0000:7c00 just as the BIOS would.
 * The environment that EFILDR expects us to have prepared is as follows:
 *
 *	0x015000 - 0x019000	EFIVAR.BIN loaded here, if the file exists
 *	0x019000 - 0x019004	The serial number of the FAT boot volume
 *	0x019004 - 0x019005	One byte indicating the following conditions:
 *				- 0 - EFIVAR.BIN was loaded
 *				- 1 - EFIVAR.BIN does not exist
 *				- 2 - EFIVAR.BIN exists but is not exactly 16k
 *	0x020000 - 0x0a0000	EFILDR loaded here
 *
 * Given those constraints, we define our memory map as follows. Note that we
 * work mostly in the lower 64k of physical memory with segment registers set
 * to zero.
 *
 *	0x006000 - 0x007000	Room for several FAT sectors (4k) (FAT12)
 *	0x007000 - 0x007c00	We have 3k here for stack and local variables
 *	0x007c00 - 0x007e00	Where BootDuet is loaded (512b)
 *	0x007e00 - 0x008000	Room for one FAT sector (FAT32 and FAT16)
 *	0x008000 - 0x010000	Room for root directory entries (32k at most)
 *	0x010000 - 0x015000	Unused space (20k)
 *	0x015000 - 0x019000	Room for EFIVAR.BIN (16k at most)
 *	0x019000 - 0x019005	Parameters for EFILDR (5 bytes)
 *	0x019005 - 0x020000	Unused space (almost 4k)
 *	0x020000 - 0x0a0000	Room for EFILDR (512k at most)
 */

.equ	BaseAddress, 0x7c00	# Offset address for our own code and data
#if FAT == 32 || FAT == 16
.equ	FatOffset, 0x7e00	# Offset address for the FAT buffer
#else
.equ	FatOffset, 0x6000	# Offset address for the FAT buffer
#endif
.equ	RootOffset, 0x8000	# Offset address for the root dir buffer

/*
 * Constants everyone knows about.
 */

.equ	DirEntrySize, 32	# Size of directory entry for all types of FAT
.equ	DirEntryShift, 5	# Shift factor for directory entry

#if defined(WITH_LBA_64BIT)
.equ	SizeOfLBA, 8		# Bytes in a 64-bit LBA variable
#else
.equ	SizeOfLBA, 4		# Bytes in a 32-bit LBA variable
#endif
#if FAT == 32
.equ	SizeOfCluster, 4	# Bytes in a cluster pointer variable
#elif FAT == 16 || FAT == 12
.equ	SizeOfCluster, 2	# Bytes in a cluster pointer variable
#endif

/*
 * This is a 16-bit code segment and we're in real mode.
 */
		.code16

/*
 * This jump is canonical, don't change the target.
 */

		.global	_start
_start:		jmp	main
		nop

/*
 * BIOS Parameter Block (BPB)
 *
 * The BPB is the same for all types of FAT filesystems.
 * The following labels are placeholders that make writing the code that
 * accesses this data easier.
 * The real data is what is filled in by the format program.
 * The intention is that you only overwrite the part of your volume boot
 * sector that contains the code and leave this part unchanged.
 */

		.space	8, 0x20	# OEM ID waste (OS or format program)

		.word	512	# Bytes per sector (will anything other than 512 work?)
		.byte	0	# Sectors per cluster
		.word	0	# Reserved sectors before first FAT including boot
		.byte	0	# Number of FATs (usually 1 or 2)
		.word	0	# Root directory entries (only FAT12 or FAT16)
		.word	0	# Number of sectors or 0 (not used)
		.byte	0	# Media descriptor (irrelevant)
		.word	0	# Sectors per FAT (for FAT12 or FAT16)
		.word	0	# Sectors per track (irrelevant for LBA)
		.word	0	# Number of heads (irrelevant for LBA)
		.long	0	# Hidden sectors before partition
		.long	0	# Number of sectors (if wSectors = 0)

/*
 * The next symbols allow us to reference exactly that same data in the BPB,
 * but relatively to our main stack frame.
 * Addressing relative to the frame pointer saves us a byte on each memory
 * reference, however, GNU as makes it hard for us to do it.
 */
		.equ	main_wBps,		0x0b
		.equ	main_bSpc,		0x0d
		.equ	main_wReserved,		0x0e
		.equ	main_bFats,		0x10
		.equ	main_wRootEntries,	0x11
		.equ	main_wSectsPerFat,	0x16
		.equ	main_lHidden,		0x1c

#if FAT == 32

/*
 * Extended BIOS Parameter Block (EBPB) for FAT32.
 */

		.long	0	# Sectors per FAT (for FAT32)
		.word	0	# FAT flags (irrelevant)
		.word	0	# FAT32 version (only known is 0)
		.long	0	# First cluster of the root directory
		.word	0	# Sector number of FSINFO (irrelevant)
		.word	0	# Backup boot sector number or 0 for no backup
		.space	12, 0	# another 12 bytes gone to waste
bDrive:		.byte	0	# BIOS drive number (we use what comes in DL)
		.byte	0	# another byte gone to waste
		.byte	0	# EBPB signature (should be 0x29)
		.long	0	# Volume ID (serial number)
		.space	11, 0x20# Volume label (8.3) (irrelevant)
		.space	8, 0x20	# another 8 bytes gone to waste ("FAT32   ")

/*
 * As before, symbols for memory references relative to the main stack frame.
 */
		.equ	main_lSectsPerFat,	0x24
		.equ	main_lRootCluster,	0x2c
		.equ	main_bDrive,		0x40
		.equ	main_bSignature,	0x42
		.equ	main_lVolId,		0x43
		.equ	main_FsType,		0x52

#elif FAT == 16 || FAT == 12

/*
 * Extended BIOS Parameter Block (EBPB) for recent versions of FAT12 and FAT16.
 */

bDrive:		.byte	0	# BIOS drive number (we use what comes in DL)
		.byte	0	# another byte gone to waste
		.byte	0	# EBPB signature (should be 0x29)
		.long	0	# Volume ID (serial number)
		.space	11, 0x20# Volume label (8.3) (irrelevant)
		.space	8, 0x20	# File system type ("FAT12   " or "FAT16   ")

/*
 * As before, symbols for memory references relative to the main stack frame.
 */
 		.equ	main_bDrive,		0x24
		.equ	main_bSignature,	0x26
		.equ	main_lVolId,		0x27
		.equ	main_FsType,		0x36

#endif

/*
 * And we finally start our main function.
 * We have 420 bytes left for the boot code, those wasted bytes would have
 * come in handy.
 * We'll probably have DL set with the BIOS boot disk number. If this is an MBR
 * disk, we may even have SI pointing to the right partition entry, if it is GPT
 * maybe a boot loader faked a partition entry. Anyway, we don't trust that.
 */

main:
	/*
	 * Initialize our memory model with all segment registers at 0000.
	 */
	 	cli				# no stack, no interrupts
		pushw	%cs
		popw	%ss			# SS = CS = 0000
		movw	$BaseAddress,%sp	# stack goes down from 7c00
		sti				# got stack, got interrupts

		cld				# we work upward

		pushw	%cs
		popw	%ds			# DS = CS = 0000

		movw	%sp,%bp			# yes, we use a frame pointer

#if !defined(WITH_HARDCODED_DRIVE)
		movb	%dl,main_bDrive(%bp)	# store drive number, which the
						# MBR should have passed us on
						# DL, to the EBPB.
#endif

#if defined(WITH_VALIDATION) && !defined(DEBUG)

/*
 * fsck - boot sector validation. This validation is only to help troubleshoot.
 * On tighter versions, such as FAT12 with LBA 64 bit, we skip this, there's no
 * space.
 */
fsck:
	/*
	 * - check that file system signature is 0x29
	 * - check that hidden sectors isn't zero
	 */
	 	cmpb	$0x29,main_bSignature(%bp)
		jne	fsck.1
		cmpl	$0,main_lHidden(%bp)
		jne	fsck.2

	/*
	 * Something went wrong with the installation, tell the user and halt.
	 */
fsck.1:
	 	movw	$Invalid,%si		# SI = error message
		movb	$InvalidLen,%cl		# CL = length
		call	print
fsck.h:		jmp	fsck.h			# Hot halt
fsck.2:

	/*
	 * Fall through to fsinit.
	 */

#endif

/*
 * init - initializes variables on the main stack frame.
 *
 * returns:
 *	sFat, sRoot, wRootSects and sData variables are set,
 *	wRootEnd, bEfiLdr, bEfiVar and cFatCache are set to harmless defaults.
 *
 * registers trashed: EAX, EBX, ECX, EDX
 */
init:
	/*
	 * The following assembler symbol is a neat trick that helps us not
	 * getting lost when computing frame offsets of local variables.
	 */
		main_frame = 0

	/*
	 * Compute the LBA of the first sector of the first FAT.
	 */
		main_frame = main_frame - SizeOfLBA
		main_sFat = main_frame		# LBA at which FAT starts

#if !defined(WITH_LBA_64BIT)
		movzwl	main_wReserved(%bp),%eax

		addl	main_lHidden(%bp),%eax	# EAX = hidden + reserved
		pushl	%eax			# store to sFat
#else
		movzwl	main_wReserved(%bp),%eax
		xorl	%edx,%edx

		addl	main_lHidden(%bp),%eax
		adcl	lHiddenHigh,%edx	# EDX:EAX = hidden + reserved
		pushl	%edx			# store to sFat
		pushl	%eax
#endif

	/*
	 * Compute the LBA of the first sector after all copies of FAT. This
	 * will be, on FAT12 and FAT16, the first sector of the root directory
	 * and on FAT32 the first sector of the first cluster (the data region).
	 */
#if FAT == 32
		main_frame = main_frame - SizeOfLBA
		main_sData = main_frame		# LBA at which data starts

		movl	main_lSectsPerFat(%bp),%ecx
#elif FAT == 16 || FAT == 12
		main_frame = main_frame - SizeOfLBA
		main_sRoot = main_frame		# LBA at which root starts

		movzwl	main_wSectsPerFat(%bp),%ecx
#endif
		movzbl	main_bFats(%bp),%eax	# EAX = number of FATs
		mull	%ecx			# EDX:EAX = total FAT sectors

#if !defined(WITH_LBA_64BIT)
		addl	main_sFat(%bp),%eax	# EAX = sFat + total FAT
		pushl	%eax			# store to sData/sRoot
#else
		addl	main_sFat(%bp),%eax
		adcl	main_sFat+4(%bp),%edx	# EDX:EAX = sFat + total FAT
		pushl	%edx			# store to sData/sRoot
		pushl	%eax
#endif

#if FAT == 12 || FAT == 16
	/*
	 * Compute the LBA of the first sector of the root directory, which
	 * comes immediately after all copies of the FAT.
	 */
	 	main_frame = main_frame - 2
		main_wRootSects = main_frame	# number of sectors in root
		main_frame = main_frame - SizeOfLBA
		main_sData = main_frame		# LBA at which data starts

		movzwl	main_wRootEntries(%bp),%eax

		bsrw	main_wBps(%bp),%cx
		subw	$DirEntryShift,%cx
		shrw	%cl,%ax

		pushw	%ax			# store to wRootSects

#if !defined(WITH_LBA_64BIT)
		addl	main_sRoot(%bp),%eax	# EAX = sRoot + wRootSects
		pushl	%eax			# store to sData
#else
		xorl	%edx,%edx
		addl	main_sRoot(%bp),%eax
		adcl	main_sRoot+4(%bp),%edx	# EDX:EAX = sRoot + wRootSects
		pushl	%edx			# store to sData
		pushl	%eax
#endif
#endif

	/*
	 * Other variables that must be initialized.
	 * cEfiVar must come lower in memory than cEfiLdr. Together, they form
	 * a sorted array of two elements.
	 */
	 	main_frame = main_frame - 2
	 	main_wRootEnd = main_frame	# ptr to end off root buffer
		main_frame = main_frame - 1
		main_bEfiLdr = main_frame	# EFILDR present?
		main_frame = main_frame - 1
		main_bEfiVar = main_frame	# EFIVAR.BIN present?
	 	main_frame = main_frame - SizeOfCluster
		main_cFatCache = main_frame	# loaded/cached FAT sector

		pushw	%cs			# zero wRootEnd
		pushw	$1			# bEfiLdr = 0, bEfiVar = 1
#if FAT == 32
		pushw	%cs			# set cFatCache to funny value
		pushw	$0xffff
#elif FAT == 16 || FAT == 12
		pushw	$0xffff			# set cFatCache to funny value
#endif

	/*
	 * Fall through to readroot.
	 */
	
/*
 * readroot - reads the root directory into the predetermined buffer.
 *
 * returns:
 *	wRootEnd variable is set
 *
 * registers trashed: EAX, EBX, ECX, EDX, SI
 */
readroot:
	/*
	 * We read the root directory to a predifined buffer and hope that it
	 * won't cross the boundary of the first 64k.
	 */
		pushw	%cs
		popw	%es			# ES = CS = 0000
		movw	$RootOffset,%di		# ES:DI = root buffer

#if FAT == 32

	/*
	 * On FAT32, root is a normal file with the start cluster registered on
	 * the EBPB.
	 */
		movl	main_lRootCluster(%bp),%eax
		call	fread			# read the root directory

		movw	%di,main_wRootEnd(%bp)	# store DI to wRootEnd

#elif FAT == 16 || FAT == 12

	/*
	 * On FAT16 and FAT12, we must compute the number of sectors taken by
	 * the root directory and read them from the disk.
	 */
	 	movw	main_wRootSects(%bp),%cx

		movl	main_sRoot(%bp),%eax	# EDX:EAX = start LBA of root
#if !defined(WITH_LBA_64BIT)
		xorl	%edx,%edx
#else
		movl	main_sRoot+4(%bp),%edx	# EDX:EAX = start LBA of root
#endif

	/*
	 * Read all sectors on the root directory.
	 */
		call	read

	/*
	 * After reading, advance DI by the number of bytes read and return.
	 * We don't advance ES. Even if we did, the next routine would have
	 * a problem if the root directory was superimposed by the files
	 * that it reads.
	 */
	 	movw	main_wBps(%bp),%ax
		mulw	%cx			# DX:AX = bytes read
		addw	%ax,%di			# advance DI by AX bytes

		movw	%di,main_wRootEnd(%bp)	# store DI to wRootEnd

#endif

	/*
	 * Fall through to scanroot.
	 */

/*
 * scanroot - scans the root directory for EFILDR and EFIVAR.BIN, if any of
 *	these files is found, it is read and bEfiLdr or bEfiVar variables are
 *	set appropriately
 *
 * returns:
 *	bEfiLdr and bEfiVar variables are set
 *
 * registers trashed: EAX, EBX, ECX, EDX, SI, DI, ES
 */
scanroot:
		movw	$RootOffset,%si		# DS:SI = root buffer

scanroot.1:
	/*
	 * Check if this directory entry is EFILDR
	 */
	 	pushw	%cs
		popw	%es			# ES = CS
	 	movw	$EfiLdr,%di		# ES:DI = &EfiLdr
		call	fncmp
		jne	scanroot.2		# jump if this isn't EFILDR

		movb	$1,main_bEfiLdr(%bp)	# bEfiLdr = 1, we have it
		pushw	$0x2000			# read EFILDR to 2000:0000
		jmp	scanroot.r

scanroot.2:
	/*
	 * Check if this directory entry is EFIVAR.BIN and, if so, check its
	 * size to see if it is 16k and then set the bHaveVar variable
	 * accordingly.
	 * If the size is right, then proceed to reading it.
	 */
	 	movw	$EfiVar,%di		# ES:DI = &EfiVar (assume ES=CS)
		call	fncmp
		jne	scanroot.n		# jump if this isn't EFIVAR.BIN

		movb	$2,main_bEfiVar(%bp)	# bEfiVar = 2, maybe size is bad
#if FAT == 12
		cmpl	$0,0x1c(%si)		# is zero size?
		je	scanroot.n		# it is, skip it
#else
		cmpl	$0x4000,0x1c(%si)	# compare size to 16k
		jne	scanroot.n		# size is not exactly 16k...
#endif

		movb	$0,main_bEfiVar(%bp)	# bEfiVar = 0, we have it
		pushw	$0x1500			# read EFIVAR.BIN to 1500:0000

scanroot.r:
	/*
	 * Found interesting file, print its name, so the user knows something
	 * if we halt for some reason, and read it to ES:0000, where ES is the
	 * word on top of the stack.
	 */
	 	movb	$FilenameLen,%cl	# CL = number of chars
		call	print			# print the file name

		popw	%es			# restore ES from the stack
	 	xorw	%di,%di			# ES:DI = xxxx:0000
#if FAT == 32
		pushw	0x14(%si)		# push high 16 bits of cluster
		pushw	0x1a(%si)		# push low 16 bits of cluster
		popl	%eax			# EAX = 32-bit cluster number
#elif FAT == 16 || FAT == 12
		movw	0x1a(%si),%ax		# AX = 16-bit cluster number
#endif
		call	fread			# read file EAX into ES:DI

scanroot.n:
	/*
	 * Proceed to the next directory entry.
	 */
	 	addw	$DirEntrySize,%si	# advance SI
		cmpw	main_wRootEnd(%bp),%si	# have we reached the end?
		jb	scanroot.1		# if not, loop

	/*
	 * Fall through to setup.
	 */

/*
 * setup - sets up environment to what EFILDR expects to find
 */
setup:
	/*
	 * Check that EFILDR was found and that we have something to jump into.
	 */
	 	movw	main_bEfiVar(%bp),%ax	# AL = bEfiVar, AH = bEfiLdr
		or	%ah,%ah			# is bEfiLdr set?
		jnz	setup.1

	/*
	 * Print error message and halt.
	 */
	 	movw	$Missing,%si		# SI = Missing
		movb	$MissingLen,%cl		# CL = length
		call	print
setup.h:	jmp	setup.h			# Hot halt

setup.1:
#if !defined(DEBUG)

	/*
	 * Copy the volume id (serial number) of the FAT file system to the
	 * physical address 19000 and the value of bEfiVar to 19004.
	 */
	 	pushw	$0x1900
		popw	%es			# ES = 1900

		movb	%al,%es:(4)		# store AL to 1900:0004
		movl	main_lVolId(%bp),%eax	# EAX = volume serial number
		movl	%eax,%es:(0)		# store EAX to 1900:0000

	/*
	 * Jump into EFILDR at 2000:0200, the second sector of start.com.
	 */
		ljmp	$0x2000,$0x0200

#endif

#if defined(DEBUG)

/*
 * printn - print a 32-bit number on the screen in decimal and halt
 *
 * parameters:
 *	EAX	number to print
 */
printn:
		leaw	main_FsType(%bp),%si	# Overwrite volume label and id
		xorl	%ebx,%ebx
		movb	$10,%bl			# EBX = decimal base
		xorw	%cx,%cx			# CX will count digits
printn.1:
		xorl	%edx,%edx		# satisfy the DIV instruction
		divl	%ebx			# EAX = EAX / 10, EDX = EAX % 10

		decw	%si			# --SI
		incw	%cx			# ++CX
		addb	$'0',%dl		# DL = decimal digit to print
		movb	%dl,(%si)		# *SI == DL

		testl	%eax,%eax		# did we reach zero?
		jnz	printn.1

		call	print
printn.h:	jmp	printn.h		# halt

#endif

/*
 * print - print a message on the screen
 *
 * parameters:
 *	DS:SI	pointer to the message being written
 *	CL	number of characters to write
 *
 * registers trashed: AL, CX, DI, ES
 */
print:
		pushw	%si
		pushw	$0xb800
		popw	%es
		xorb	%ch,%ch			# high 16 bits of counter = 0
		xorw	%di,%di			# ES:DI = b800:0000
		movb	$0x07,%al		# AL = 0x07 (white, non-blink)
print.1:
		movsb				# move one byte of text
		stosb				# store one byte of attributes
		loop	print.1
		popw	%si
		ret

/*
 * fncmp - compares two 8.3 style filenames
 *
 * parameters:
 *	DS:SI	pointer to one filename
 *	ES:DI	pointer the other filename
 *
 * returns:
 *	zero flag set if filenames are equal
 *
 * registers trashed: CX, DI
 */
fncmp:		
		pushw	%si
		movw	$FilenameLen,%cx	# compare all 11 characters
	repe	cmpsb
		popw	%si			# pop won't affect flags
		ret

/*
 * fread - reads a file given its first cluster
 *
 * parameters:
 *	EAX	first cluster to read (AX on FAT12/FAT16)
 *	ES:DI	destination buffer
 *
 * returns:
 *	ES:DI	pointer past the used portion of the destination buffer
 *
 * registers trashed: EAX, EBX, ECX, EDX
 */
fread:
	/*
	 * First we establish a stack frame here. We will assume, for
	 * size optimization, that the stack frame above is the main stack
	 * frame. That means this function won't work if it's not called in that
	 * context.
	 * The margin between frames contains IP and BP only (4 bytes).
	 */
	 	pushw	%bp
		movw	%sp,%bp

		fread_frame = 0

		fread_wBps = main_wBps - main_frame + 4
		fread_bSpc = main_bSpc - main_frame + 4

		fread_sFat = main_sFat - main_frame + 4
		fread_sData = main_sData - main_frame + 4

		fread_cFatCache = main_cFatCache - main_frame + 4

fread.1:
	/*
	 * Check if the cluster is something crazy, such as the last cluster in
	 * file or a bad sector or anything strange.
	 */
#if FAT == 32
		cmpl	$2,%eax
		jb	fread.2
		cmpl	$0x0ffffff0,%eax
		jb	fread.3
#elif FAT == 16
		cmpw	$2,%ax
		jb	fread.2
		cmpw	$0xfff0,%ax
		jb	fread.3
#elif FAT == 12
		cmpw	$2,%ax
		jb	fread.2
		cmpw	$0x0ff0,%ax
		jb	fread.3
#endif

fread.2:
	/*
	 * If the cluster number is crazy, then we return and that's it.
	 */
	 	leave
	 	ret
fread.3:
	/*
	 * Clean the cluster number, on FAT12 and FAT16, we may have trash
	 * on the upper 16 bits of EAX.
	 * Then save the cluster number on a local variable.
	 */
	 	fread_frame = fread_frame - 4	# 4 bytes even for FAT12/16
		fread_cluster = fread_frame	# new local var cluster

#if FAT == 16 || FAT == 12
		movzwl	%ax,%eax		# zero extend AX
#endif
		pushl	%eax			# store to var cluster

	/*
	 * Compute the start LBA for this cluster, first the offset from the
	 * data region, then the LBA.
	 * ECX is kept with the number of sectors per cluster.
	 */
#if FAT == 32
		subl	$2,%eax
#elif FAT == 16 || FAT == 12
		decw	%ax
		decw	%ax
#endif
		movzbl	fread_bSpc(%bp),%ecx	# ECX = sectors per cluster
		mull	%ecx			# EDX:EAX = (cluster - 2) * ECX

		addl	fread_sData(%bp),%eax	# EDX:EAX = start LBA
#if defined(WITH_LBA_64BIT)
		adcl	fread_sData+4(%bp),%edx
#endif

	/*
	 * Read the cluster, the number of sectors to read is bSpc (in CX).
	 */
		call	read

	/*
	 * After reading, advance DI by the number of bytes read and on overflow
	 * advance ES too.
	 */
	 	movw	fread_wBps(%bp),%ax
		mulw	%cx			# AX = bytes per cluster (bSpc * wBps)
		addw	%ax,%di			# advance DI by AX bytes
		jnc	fread.4			# check for carry

		pushw	%es			# advance ES by 64k
		addw	$0x1000,fread_frame-2(%bp)
		popw	%es

fread.4:
	/*
	 * Now we must locate the next cluster in the file, and for that, first
	 * locate the FAT sector that we must read by obtaining the quotient and
	 * remainder of the division of the cluster number by the number of
	 * cluster pointers per FAT sector.
	 */
		popl	%eax			# restore from fread_cluster
	 	fread_frame = fread_frame + 4	# fread_cluster goes away

	 	fread_frame = fread_frame - 2
		fread_index = fread_frame	# new local var index

#if FAT == 32
		movzwl	fread_wBps(%bp),%ebx	# EBX = bytes per sector
		shrw	$2,%bx			# EBX = ptrs per sector
		xorl	%edx,%edx
		divl	%ebx			# EAX = FAT sector, EDX = ptr

		pushw	%dx			# save cluster pointer index
#elif FAT == 16
		movw	fread_wBps(%bp),%bx	# BX = bytes per sector
		shrw	$1,%bx			# BX = ptrs per sector
		xorw	%dx,%dx
		divw	%bx			# AX = FAT sector, DX = ptr

		pushw	%dx			# save cluster pointer index
#elif FAT == 12
		movw	fread_wBps(%bp),%bx	# BX = bytes per sector
		shlw	$1,%bx			# BX = ptrs per trio
		xorw	%dx,%dx
		divw	%bx			# AX = FAT trio, DX = ptr

		pushw	%dx			# save cluster pointer index

		movw	$3,%bx
		mulw	%bx			# AX = first sector of trio
#endif

	/*
	 * Check if this FAT sector was already loaded and, if so, skip to the
	 * next section.
	 */
#if FAT == 32
	 	cmpl	fread_cFatCache(%bp),%eax
#elif FAT == 16 || FAT == 12
	 	cmpw	fread_cFatCache(%bp),%ax
#endif
		je	fread.5

	/*
	 * We have to load the relevant sector of FAT. We keep the sector that
	 * we will load in cFatCache (if the read fails we will abort anyway).
	 */
#if FAT == 32
	 	movl	%eax,fread_cFatCache(%bp)
#elif FAT == 16 || FAT == 12
	 	movw	%ax,fread_cFatCache(%bp)
#endif

		xorl	%edx,%edx
#if !defined(WITH_LBA_64BIT)
		addl	fread_sFat(%bp),%eax	# EDX:EAX = FAT LBA
#else
		addl	fread_sFat(%bp),%eax
		adcl	fread_sFat+4(%bp),%edx	# EDX:EAX = FAT LBA
#endif

	/*
	 * Now read that sector into the buffer for FAT sectors (0000:7e00).
	 */
		pushw	%es
		pushw	%di

		pushw	%cs
		popw	%es
		movw	$FatOffset,%di		# ES:DI = 0000:xxxx
#if FAT == 12
		movw	$3,%cx			# on FAT12, read 3 sectors
#else
		movw	$1,%cx
#endif
		call	read			# read the FAT sector

		popw	%di
		popw	%es

fread.5:
	/*
	 * Finally we read the cluster pointer in this FAT sector and repeat the
	 * whole thing.
	 */
		popw	%bx			# restore fread_index to BX
	 	fread_frame = fread_frame + 2	# fread_index goes away

#if FAT == 32
		shlw	$2,%bx			# BX = index * 4
	 	movl	FatOffset(%bx),%eax	# EAX = next cluster in file
		jmp	fread.1
#elif FAT == 16
		shlw	$1,%bx			# BX = index * 2
		movw	FatOffset(%bx),%ax	# AX = next cluster in file
		jmp	fread.1
#elif FAT == 12
		xorw	%cx,%cx			# CX = default shift factor 0
		movw	%bx,%ax			# AX = index too
		testw	$1,%ax			# is it even or odd?
		jz	fread.6			# if even, CL (shift factor) = 0
		movb	$4,%cl			# if odd, CL (shift factor) = 4
fread.6:
		shrw	$1,%ax
		addw	%ax,%bx			# BX = 3 * (index / 2) + odd?

		movw	FatOffset(%bx),%ax	# low or high 12 of AX is ptr
		shrw	%cl,%ax			# shift right by CL
		andw	$0x0fff,%ax		# trim high bits
		jmp	fread.1
#endif

/*
 * read - reads sectors by 64-bit Logical Block Address (LBA)
 *
 * parameters:
 *	EDX:EAX	LBA of first sector to read
 * 	CX	number of sectors to read
 *	ES:DI	destination buffer
 *
 * registers trashed: AX, DL
 */
read:
		pushw	%si

	/*
	 * Fill in Disk Address Packet (DAP) structure. This is filled on the
	 * stack from top to bottom.
	 */
	 	pushl	%edx
		pushl	%eax
		pushw	%es
		pushw	%di
		pushw	%cx
		pushw	$16

	/*
	 * Call BIOS int 13h AH=42h: Extended Read Sectors From Drive.
	 */
	 	movb	$0x42,%ah
		movb	bDrive,%dl
		movw	%sp,%si			# SI = SP (base of DAP)
		int	$0x13

	/*
	 * Return on success or halt on error (an indicative message would have
	 * already been written on the screen).
	 */
read.h:		jc	read.h			# Halt if carry
		addw	$16,%sp
		popw	%si
		ret

/*
 * String constants for the whole program and their length.
 */

#if defined(WITH_VALIDATION) && !defined(DEBUG)
		.equ	InvalidLen, 3
Invalid:	.ascii	"Bad"
#endif

		.equ	MissingLen, 19		# "Missing " + 8.3 filename
Missing:	.ascii	"Missing "

		.equ	FilenameLen, 11		# 8.3 filename (11)
#if FAT == 32
EfiLdr:		.ascii	"EFILDR20   "
#elif FAT == 16
EfiLdr:		.ascii	"EFILDR16   "
#elif FAT == 12
EfiLdr:		.ascii	"EFILDR     "
#endif
EfiVar:		.ascii	"EFIVAR  BIN"

/*
 * Since the BPB only has room for a 32-bit number of hidden sectors, if we need
 * 64-bit LBA, we need room for storing the high-order 32 bits of hidden
 * sectors.
 */

#if defined(WITH_LBA_64BIT)
		.org	0x01fa
lHiddenHigh:	.long	0
#endif

/*
 * The boot sector signature at the end of the sector.
 */
 		.org	0x01fe
wSignature:	.word	0xaa55
