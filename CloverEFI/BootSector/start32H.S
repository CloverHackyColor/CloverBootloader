#------------------------------------------------------------------------------
#*
#*   Copyright (c) 2006 - 2007, Intel Corporation. All rights reserved.<BR>
#*   This program and the accompanying materials                          
#*   are licensed and made available under the terms and conditions of the BSD License         
#*   which accompanies this distribution.  The full text of the license may be found at        
#*   http://opensource.org/licenses/bsd-license.php                                            
#*                                                                                             
#*   THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
#*   WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
#*   
#*    start32.asm
#*  
#*   Abstract:
#*
#------------------------------------------------------------------------------

#define data32  .byte 0x66
#define addr32  .byte 0x67


        #.MODEL small
        .stack: 
        .486p: 
        .code16 

.equ                        FAT_DIRECTORY_ENTRY_SIZE, 0x020
.equ                        FAT_DIRECTORY_ENTRY_SHIFT, 5
.equ                        BLOCK_SIZE, 0x0200
.equ                        BLOCK_MASK, 0x01ff
.equ                        BLOCK_SHIFT, 9

        .org 0x0

.global _start
_start:

Ia32Jump: 
  jmp   BootSectorEntryPoint  # JMP inst    - 3 bytes
  nop

OemId:               .ascii  "INTEL   "    # OemId                           - 8 bytes
SectorSize:          .word  0              # Sector Size                     - 2 bytes
SectorsPerCluster:   .byte  0              # Sector Per Cluster              - 1 byte
ReservedSectors:     .word  0              # Reserved Sectors                - 2 bytes
NoFats:              .byte  0              # Number of FATs                  - 1 byte
RootEntries:         .word  0              # Root Entries                    - 2 bytes
Sectors:             .word  0              # Number of Sectors               - 2 bytes
Media:               .byte  0              # Media                           - 1 byte
SectorsPerFat16:     .word  0              # Sectors Per FAT for FAT12/FAT16 - 2 byte
SectorsPerTrack:     .word  0              # Sectors Per Track               - 2 bytes
Heads:               .word  0              # Heads                           - 2 bytes
HiddenSectors:       .long  0              # Hidden Sectors                  - 4 bytes
LargeSectors:        .long  0              # Large Sectors                   - 4 bytes

#******************************************************************************
#
#The structure for FAT32 starting at offset 36 of the boot sector. (At this point, 
#the BPB/boot sector for FAT12 and FAT16 differs from the BPB/boot sector for FAT32.)
#
#******************************************************************************

SectorsPerFat32:     .long  0              # Sectors Per FAT for FAT32       - 4 bytes
ExtFlags:            .word  0              # Mirror Flag                     - 2 bytes
FSVersion:           .word  0              # File System Version             - 2 bytes
RootCluster:         .long  0              # 1st Cluster Number of Root Dir  - 4 bytes
FSInfo:              .word  0              # Sector Number of FSINFO         - 2 bytes
BkBootSector:        .word  0              # Sector Number of Bk BootSector  - 2 bytes
Reserved:            .fill 12,1,0          # Reserved Field                  - 12 bytes
PhysicalDrive:       .byte  0              # Physical Drive Number           - 1 byte
Reserved1:           .byte  0              # Reserved Field                  - 1 byte
Signature:           .byte  0              # Extended Boot Signature         - 1 byte
VolId:               .ascii  "    "        # Volume Serial Number            - 4 bytes
FatLabel:            .ascii  "Clover     " # Volume Label                    - 11 bytes
FileSystemType:      .ascii  "HFSPlus "    # File System Type                - 8 bytes

BootSectorEntryPoint: 
        #ASSUME ds:@code
        #ASSUME ss:@code
      # ds = 1000, es = 2000 + x (size of first cluster >> 4)
      # cx = Start Cluster of EfiLdr
      # dx = Start Cluster of Efivar.bin

# Re use the BPB data stored in Boot Sector
        movw    $0x7c00, %bp
JumpFarInstruction: 
        .byte   0xea
JumpOffset: 
        .word   0x200
JumpSegment: 
        .word   0x2000

/*
PrintStringAndHalt: 
        movw $0xb800, %ax
        movw %ax, %es
        movw $160, %di
        rep
        movsw
Halt: 
        jmp   Halt

ErrorString: 
        .byte 'S', 0x0c, 'E', 0x0c, 'r', 0x0c, 'r', 0x0c, 'o', 0x0c, 'r', 0x0c, '!', 0x0c

 */
         .org     0x01fa  # Will cause build break
LBAOffsetForBootSector: 
        .long   0x0

         .org    0x01fe   # Will cause build break
        .word   0xaa55

#******************************************************************************
#******************************************************************************
#******************************************************************************

.equ                 DELAY_PORT, 0x0ed           # Port to use for 1uS delay
.equ                 KBD_CONTROL_PORT, 0x060     # 8042 control port     
.equ                 KBD_STATUS_PORT, 0x064      # 8042 status port      
.equ                 WRITE_DATA_PORT_CMD, 0x0d1  # 8042 command to write the data port
.equ                 ENABLE_A20_CMD, 0x0df       # 8042 command to enable A20

 .org     0x200  # Will cause build break
 .code16
        jmp start		
#Em64String: 
#        .byte 'E', 0x0c, 'm', 0x0c, '6', 0x0c, '4', 0x0c, 'T', 0x0c, ' ', 0x0c, 'U', 0x0c, 'n', 0x0c, 's', 0x0c, 'u', 0x0c, 'p', 0x0c, 'p', 0x0c, 'o', 0x0c, 'r', 0x0c, 't', 0x0c, 'e', 0x0c, 'd', 0x0c, '!', 0x0c
Label:            .ascii  "Clover      " # Bootloader Label  
start:  
        movw %cs, %ax
        movw %ax, %ds
        movw %ax, %es
        movw %ax, %ss
        movw $MyStack, %sp

#        mov ax,0b800h
#        mov es,ax
#        mov byte ptr es:[160],'a'
#        mov ax,cs
#        mov es,ax
#		movw $0xb800, %ax
#		movw %ax, %es
#		movw $0x61, byte ptr %es:[160]
#		movw %cs, %ax
#		movw %ax, %es
//advanced algo
/*
; use the INT 0x15, eax= 0xE820 BIOS function to get a memory map
; inputs: es:di -> destination buffer for 24 byte entries
; outputs: bp = entry count, trashes all registers except esi
do_e820:
	xor ebx, ebx		; ebx must be 0 to start
	xor bp, bp		; keep an entry count in bp
	mov edx, 0x0534D4150	; Place "SMAP" into edx
	mov eax, 0xe820
	mov [es:di + 20], dword 1	; force a valid ACPI 3.X entry
	mov ecx, 24		; ask for 24 bytes
	int 0x15
	jc short .failed	; carry set on first call means "unsupported function"
	mov edx, 0x0534D4150	; Some BIOSes apparently trash this register?
	cmp eax, edx		; on success, eax must have been reset to "SMAP"
	jne short .failed
	test ebx, ebx		; ebx = 0 implies list is only 1 entry long (worthless)
	je short .failed
	jmp short .jmpin
.e820lp:
	mov eax, 0xe820		; eax, ecx get trashed on every int 0x15 call
	mov [es:di + 20], dword 1	; force a valid ACPI 3.X entry
	mov ecx, 24		; ask for 24 bytes again
	int 0x15
	jc short .e820f		; carry set means "end of list already reached"
	mov edx, 0x0534D4150	; repair potentially trashed register
.jmpin:
	jcxz .skipent		; skip any 0 length entries
	cmp cl, 20		; got a 24 byte ACPI 3.X response?
	jbe short .notext
	test byte [es:di + 20], 1	; if so: is the "ignore this data" bit clear?
	je short .skipent
.notext:
	mov ecx, [es:di + 8]	; get lower dword of memory region length
	or ecx, [es:di + 12]	; "or" it with upper dword to test for zero
	jz .skipent		; if length qword is 0, skip entry
	inc bp			; got a good entry: ++count, move to next storage spot
	add di, 24
.skipent:
	test ebx, ebx		; if ebx resets to 0, list is complete
	jne short .e820lp
.e820f:
	mov [mmap_ent], bp	; store the entry count
	clc			; there is "jc" on end of list to this point, so the carry must be cleared
	ret
.failed:
	stc			; "function unsupported" error exit
	ret
*/	
/*	xorl %ebx, %ebx
	xorl %ebp, %ebp
	movl $0x534d4150, %edx  # 0x534d4150 = 'SMAP'
	movl $0xe820, %eax
	leal MemoryMap, %edi
	movl $1, %es:20(%di)
	movl $24, %ecx
	int  $0x15
	jc  MemMapDone
	movl $0x534d4150, %edx
	cmpl %edx, %eax
	jne MemMapDone
	testl %ebx, %ebx
	je  MemMapDone
	jmp JumpIn
MemMapLoop:
	movl $0xe820, %eax	
	movl $1, %es:20(%di)
	int  $0x15	
	jc  MemMapDone
	movl $0x534d4150, %edx
JumpIn:
	jcxz SkipEntry
	cmpb $20, %cl
	jbe NoText
#	movl 20(%edi), %eax
	testb $1, 20(%edi)
	je SkipEntry
NoText:
	movl %es:8(%di), %ecx
	orl %es:12(%di), %ecx
	jz  SkipEntry
	incl %ebp
	addl $24, %edi
SkipEntry:
	test %ebx, %ebx
	jne  MemMapLoop
MemMapDone:
	movl %ebp, MemoryMapSize
	clc		
	*/

        movl $0, %ebx
        leal MemoryMap, %edi
MemMapLoop: 
        movl $0xe820, %eax
        movl $20, %ecx			#WIKI said $24
        movl $0x534d4150, %edx  # 0x534d4150 = 'SMAP' 
        int  $0x15
        jc  MemMapDone
        addl $20, %edi
        cmpl $0, %ebx
        je  MemMapDone
        jmp MemMapLoop
MemMapDone: 
        leal MemoryMap, %eax
        subl %eax, %edi                     # Get the address of the memory map
        movl %edi, MemoryMapSize            # Save the size of the memory map

        xorl    %ebx, %ebx
        movw    %cs, %bx                    # BX=segment
        shll    $4, %ebx                    # BX="linear" address of segment base
        leal    GDT_BASE(%ebx), %eax        # EAX=PHYSICAL address of gdt
        movl    %eax, gdtr + 2            # Put address of gdt into the gdtr
        leal    IDT_BASE(%ebx), %eax        # EAX=PHYSICAL address of idt
        movl    %eax, idtr + 2            # Put address of idt into the idtr
        leal    MemoryMapSize(%ebx), %edx   # Physical base address of the memory map

        addl $0x1000, %ebx                  # Source of EFI32 = $0x21000
        movl %ebx, JUMP+2
        addl $0x1000, %ebx
        movl %ebx, %esi                     # Source of EFILDR32 = $0x22000

#        mov ax,0b800h
#        mov es,ax
#        mov byte ptr es:[162],'b'
#        mov ax,cs
#        mov es,ax
#		movw $0xb800, %ax
#		movw %ax, %es
#		movw $0x62, byte ptr %es:[162]
#		movw %cs, %ax
#		movw %ax, %es


#
# Enable A20 Gate 
#
		
        movw $0x2401, %ax                   # Enable A20 Gate
        int $0x15
        jnc A20GateEnabled                  # Jump if it succeeded

#
# If INT 15 Function 2401 is not supported, then attempt to Enable A20 manually.
#
#New algo from WIKI
/*		cli
		call    a20wait
		movb	$0xAD, %al
		outb    %al, $KBD_STATUS_PORT
		
		call    a20wait
		movb	$0xD0, %al
		outb    %al, $KBD_STATUS_PORT
		
		call    a20wait2
		inb     $KBD_CONTROL_PORT, %al
		pushl	%eax
		
		call    a20wait
		movb	$0xD1, %al
		outb    %al, $KBD_STATUS_PORT
		
		call    a20wait
		popl	%eax
		orb		$2, %al
		outb    %al, $KBD_CONTROL_PORT
		
		call    a20wait
		movb	$0xAE, %al
		outb    %al, $KBD_STATUS_PORT
		
		call    a20wait
*/
#UEFI/DUET
        call    Empty8042InputBuffer        # Empty the Input Buffer on the 8042 controller
        jnz     Timeout8042                 # Jump if the 8042 timed out
        outw    %ax, $DELAY_PORT            # Delay 1 uS
        movb    $WRITE_DATA_PORT_CMD, %al   # 8042 cmd to write output port
        outb    %al, $KBD_STATUS_PORT       # Send command to the 8042
        call    Empty8042InputBuffer        # Empty the Input Buffer on the 8042 controller
        jnz     Timeout8042                 # Jump if the 8042 timed out
        movb    $ENABLE_A20_CMD, %al        # gate address bit 20 on
        outb    %al, $KBD_CONTROL_PORT      # Send command to thre 8042
        call    Empty8042InputBuffer        # Empty the Input Buffer on the 8042 controller
        movw    $25, %cx                    # Delay 25 uS for the command to complete on the 8042
Delay25uS: 
        outw    %ax, $DELAY_PORT            # Delay 1 uS
        loopl   Delay25uS
Timeout8042: 
		
#WIKI -fast A20gate
#		inb     $0x92, %al
#		orb		$2, %al
#		outb    %al, $0x92

A20GateEnabled: 
#//Slice - switch to graphics mode (1), or 80x25 mono text mode (2)
		movw	$0x0002, %ax 
		int		$0x10
		#put char 6
		movl	$0x000F, %ebx
		movl	$0x0E33, %eax
		movl	$0x0010, %ecx
		int		$0x10
           movw    $0x0008, %bx                # Flat data descriptor
#PAUSE1:		
#		jmp PAUSE1

#
# DISABLE INTERRUPTS - Entering Protected Mode
#

        cli

        .byte   0x66
        lgdt    gdtr
#PAUSE2:		
#		jmp PAUSE2
#		addr32
        data32
        lidt    idtr
#PAUSE3:		
#		jmp PAUSE3

        movl    %cr0, %eax
        orb     $1, %al
#		data32
#		or      $1, %eax
        movl    %eax, %cr0
				
JUMP: 
# jmp far 0010:00020000
        data32
        .byte 0xea
        .long 0x00020000
        .word 0x0010

Empty8042InputBuffer: 
        movw $0, %cx
Empty8042Loop: 
        outw    %ax, $DELAY_PORT            # Delay 1us
        inb     $KBD_STATUS_PORT, %al       # Read the 8042 Status Port
        andb    $0x2, %al                   # Check the Input Buffer Full Flag
        loopnz  Empty8042Loop               # Loop until the input buffer is empty or a timout of 65536 uS
        ret
/*		
a20wait:
        inb     $KBD_STATUS_PORT, %al
        testb    $2, %al
        jnz     a20wait
        ret
 
 
a20wait2:
        inb     $KBD_STATUS_PORT, %al
        testb   $1, %al
        jz      a20wait2
        ret
*/
/*
PrintStringAndHalt: 
        leaw ErrorString, %si
        movw $7, %cx

        movw $0xb800, %ax
        movw %ax, %es
        movw $160, %di
        rep
        movsw
Halt: 
        jmp   Halt

ErrorString: 
        .byte 'S', 0x0c, 'E', 0x0c, 'r', 0x0c, 'r', 0x0c, 'o', 0x0c, 'r', 0x0c, '!', 0x0c
*/		

##############################################################################
# data
##############################################################################

        .p2align 1

gdtr:    .word GDT_END - GDT_BASE - 1   
        .long 0                     # (GDT base gets set above)
##############################################################################
#   global descriptor table (GDT)
##############################################################################

        .p2align 1

GDT_BASE: 
# null descriptor
.equ                NULL_SEL, .-GDT_BASE
        .word 0         # limit 15:0
        .word 0         # base 15:0
        .byte 0         # base 23:16
        .byte 0         # type
        .byte 0         # limit 19:16, flags
        .byte 0         # base 31:24

# linear data segment descriptor
.equ            LINEAR_SEL, .-GDT_BASE
        .word 0xFFFF    # limit 0xFFFFF
        .word 0         # base 0
        .byte 0
        .byte 0x92      # present, ring 0, data, expand-up, writable
        .byte 0xCF      # page-granular, 32-bit
        .byte 0

# linear code segment descriptor
.equ            LINEAR_CODE_SEL, .-GDT_BASE
        .word 0xFFFF    # limit 0xFFFFF
        .word 0         # base 0
        .byte 0
        .byte 0x9A      # present, ring 0, data, expand-up, writable
        .byte 0xCF      # page-granular, 32-bit
        .byte 0

# system data segment descriptor
.equ            SYS_DATA_SEL, .-GDT_BASE
        .word 0xFFFF    # limit 0xFFFFF
        .word 0         # base 0
        .byte 0
        .byte 0x92      # present, ring 0, data, expand-up, writable
        .byte 0xCF      # page-granular, 32-bit
        .byte 0

# system code segment descriptor
.equ            SYS_CODE_SEL, .-GDT_BASE
        .word 0xFFFF    # limit 0xFFFFF
        .word 0         # base 0
        .byte 0
        .byte 0x9A      # present, ring 0, data, expand-up, writable
        .byte 0xCF      # page-granular, 32-bit
        .byte 0

# spare segment descriptor
.equ        SPARE3_SEL, .-GDT_BASE
        .word 0         # limit 0xFFFFF
        .word 0         # base 0
        .byte 0
        .byte 0         # present, ring 0, data, expand-up, writable
        .byte 0         # page-granular, 32-bit
        .byte 0

# spare segment descriptor
.equ        SPARE4_SEL, .-GDT_BASE
        .word 0         # limit 0xFFFFF
        .word 0         # base 0
        .byte 0
        .byte 0         # present, ring 0, data, expand-up, writable
        .byte 0         # page-granular, 32-bit
        .byte 0

# spare segment descriptor
.equ        SPARE5_SEL, .-GDT_BASE
        .word 0         # limit 0xFFFFF
        .word 0         # base 0
        .byte 0
        .byte 0         # present, ring 0, data, expand-up, writable
        .byte 0         # page-granular, 32-bit
        .byte 0

GDT_END: 

        .p2align 1



idtr:            .word IDT_END - IDT_BASE - 1  
        .long 0                     # (IDT base gets set above)
##############################################################################
#   interrupt descriptor table (IDT)
#
#   Note: The hardware IRQ's specified in this table are the normal PC/AT IRQ
#       mappings.  This implementation only uses the system timer and all other
#       IRQs will remain masked.  The descriptors for vectors 33+ are provided
#       for convenience.
##############################################################################

#idt_tag db "IDT",0     
        .p2align 1

IDT_BASE: 
# divide by zero (INT 0)
.equ                DIV_ZERO_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# debug exception (INT 1)
.equ                DEBUG_EXCEPT_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# NMI (INT 2)
.equ                NMI_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# soft breakpoint (INT 3)
.equ                BREAKPOINT_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# overflow (INT 4)
.equ                OVERFLOW_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# bounds check (INT 5)
.equ                BOUNDS_CHECK_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# invalid opcode (INT 6)
.equ                INVALID_OPCODE_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# device not available (INT 7)
.equ                DEV_NOT_AVAIL_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# double fault (INT 8)
.equ                DOUBLE_FAULT_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# Coprocessor segment overrun - reserved (INT 9)
.equ                RSVD_INTR_SEL1, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# invalid TSS (INT 0ah)
.equ                INVALID_TSS_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# segment not present (INT 0bh)
.equ                SEG_NOT_PRESENT_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# stack fault (INT 0ch)
.equ                STACK_FAULT_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# general protection (INT 0dh)
.equ                GP_FAULT_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# page fault (INT 0eh)
.equ                PAGE_FAULT_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# Intel reserved - do not use (INT 0fh)
.equ                RSVD_INTR_SEL2, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# floating point error (INT 10h)
.equ                FLT_POINT_ERR_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# alignment check (INT 11h)
.equ                ALIGNMENT_CHECK_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# machine check (INT 12h)
.equ                MACHINE_CHECK_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# SIMD floating-point exception (INT 13h)
.equ                SIMD_EXCEPTION_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# 84 unspecified descriptors, First 12 of them are reserved, the rest are avail
        .fill 84 * 8, 1, 0

# IRQ 0 (System timer) - (INT 68h)
.equ                IRQ0_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# IRQ 1 (8042 Keyboard controller) - (INT 69h)
.equ                IRQ1_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# Reserved - IRQ 2 redirect (IRQ 2) - DO NOT USE!!! - (INT 6ah)
.equ                IRQ2_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# IRQ 3 (COM 2) - (INT 6bh)
.equ                IRQ3_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# IRQ 4 (COM 1) - (INT 6ch)
.equ                IRQ4_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# IRQ 5 (LPT 2) - (INT 6dh)
.equ                IRQ5_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# IRQ 6 (Floppy controller) - (INT 6eh)
.equ                IRQ6_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# IRQ 7 (LPT 1) - (INT 6fh)
.equ                IRQ7_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# IRQ 8 (RTC Alarm) - (INT 70h)
.equ                IRQ8_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# IRQ 9 - (INT 71h)
.equ                IRQ9_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# IRQ 10 - (INT 72h)
.equ                 IRQ10_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# IRQ 11 - (INT 73h)
.equ                 IRQ11_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# IRQ 12 (PS/2 mouse) - (INT 74h)
.equ                 IRQ12_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# IRQ 13 (Floating point error) - (INT 75h)
.equ                 IRQ13_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# IRQ 14 (Secondary IDE) - (INT 76h)
.equ                 IRQ14_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

# IRQ 15 (Primary IDE) - (INT 77h)
.equ                 IRQ15_SEL, .-IDT_BASE
        .word 0            # offset 15:0
        .word SYS_CODE_SEL # selector 15:0
        .byte 0            # 0 for interrupt gate
        .byte 0x0e | 0x80  # type = 386 interrupt gate, present
        .word 0            # offset 31:16

        .fill 8, 1, 0

IDT_END: 

        .p2align 1

MemoryMapSize:  .long 0
MemoryMap:  .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0

        .long 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        .long 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

        .org 0x0fe0
MyStack:    
        # below is the pieces of the IVT that is used to redirect INT 68h - 6fh
        #    back to INT 08h - 0fh  when in real mode...  It is 'org'ed to a
        #    known low address (20f00) so it can be set up by PlMapIrqToVect in
        #    8259.c

        int $8
        iret

        int $9
        iret

        int $10
        iret

        int $11
        iret

        int $12
        iret

        int $13
        iret

        int $14
        iret

        int $15
        iret


        .org 0x0ffe
BlockSignature: 
        .word 0xaa55


