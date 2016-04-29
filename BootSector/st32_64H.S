#------------------------------------------------------------------------------
#*
#*   Copyright (c) 2006 - 2016, Intel Corporation. All rights reserved.<BR>
#*   Copyright (c) 2016, Clover Inc. All rights reserved.<BR>
#*   This program and the accompanying materials
#*   are licensed and made available under the terms and conditions of the BSD License
#*   which accompanies this distribution.  The full text of the license may be found at
#*   http://opensource.org/licenses/bsd-license.php
#*
#*   THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#*   WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#*
#*    st32_64H.S
#*
#*   Abstract:
#*
#------------------------------------------------------------------------------
# Build with
#   as -o st32_64H.o st32_64H.S
#   ld --oformat=binary -Ttext=0x200 -o st32_64H.com st32_64H.o
#
# To change character displayed use --defsym CHARACTER_TO_SHOW=<int value> parameter to as
#

        .code16

/*
.equ    DELAY_PORT, 0x0ed # Port to use for 1uS delay
.equ    KBD_CONTROL_PORT, 0x060 # 8042 control port
.equ    KBD_STATUS_PORT, 0x064 # 8042 status port
.equ    WRITE_DATA_PORT_CMD, 0x0d1 # 8042 command to write the data port
.equ    ENABLE_A20_CMD, 0x0df # 8042 command to enable A20
*/
.equ    FAST_ENABLE_A20_PORT, 0x92
.equ    FAST_ENABLE_A20_MASK, 2
.equ    IA32_EFER, 0xC0000080
.ifndef CHARACTER_TO_SHOW
.equ    CHARACTER_TO_SHOW, 'T'
.endif

.globl _start
_start:
        jmp 1f

        .ascii "CLOVERX64   "

1:
        movw %cs,%ax
        movw %ax,%ds
        movw %ax,%es
        movw %ax,%ss
        movw $MyStack, %sp

#
# Retrieve Bios Memory Map
#
        xorl %ebx,%ebx
        leal MemoryMap,%edi
MemMapLoop:
        movl $0xe820,%eax
        movl $20,%ecx
        movl $0x534d4150, %edx  # SMAP
        int  $0x15
        jc   MemMapDone
        addw $20,%di
        test %ebx,%ebx
        jne  MemMapLoop
MemMapDone:
        subw $MemoryMap,%di                 # Get the address of the memory map
        movl %edi, MemoryMapSize            # Save the size of the memory map

#
# Rebase Self
#
        xorl    %ebx,%ebx
        movw    %cs,%bx                     # BX=segment
        shll    $4,%ebx                     # BX="linear" address of segment base
        addl    %ebx, (gdtr + 2)            # Rebase address of GDT
        addl    %ebx, (idtr + 2)            # Rebase address of IDT
        addl    %ebx, JumpToLongMode        # Rebase ljmp Real Mode -> Long Mode

#
# Enable A20 Gate
#

        movw $0x2401,%ax                    # Enable A20 Gate
        int  $0x15
        jnc  A20GateEnabled                 # Jump if it suceeded

#
# If INT 15 Function 2401 is not supported, then attempt to Enable A20 manually.
#
/*
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
        movw    $25,%cx                     # Delay 25 uS for the command to complete on the 8042
Delay25uS: 
        outw    %ax, $DELAY_PORT            # Delay 1 uS
        loop    Delay25uS
Timeout8042: 
*/

# WIKI - fast A20gate
        inb     $FAST_ENABLE_A20_PORT, %al
        orb     $FAST_ENABLE_A20_MASK, %al
        outb    %al, $FAST_ENABLE_A20_PORT

A20GateEnabled: 

#
# Create Page Table
#
        call    CreatePageTable

#
# DISABLE INTERRUPTS - Entering Protected Mode
#  253668.pdf page 401
#
        movw    $0x000F, %bx
        movw    $(0x0E00 | (CHARACTER_TO_SHOW & 255)), %ax
        int     $0x10

        cli

    #
    # Ready Address of Page Table in EDX
    #
    movzwl PageTableSegment, %edx
    shll $4, %edx

    #
    # load GDT
    #
    lgdtl gdtr

    #
    # Enable the 64-bit page-translation-table entries by
    # setting CR4.PAE=1 (this is _required_ before activating
    # long mode). Paging is not enabled until after long mode
    # is enabled.
    #
    movl %cr4, %eax
    orb  $0x20, %al
    movl %eax, %cr4

    #
    # This is the Trapolean Page Tables that are guarenteed
    #  under 4GB.
    #
    # Address Map:
    #    10000 ~    12000 - efildr (loaded)
    #    20000 ~    21000 - start64.com
    #    21000 ~    22000 - efi64.com
    #    22000 ~    90000 - efildr
    #    90000 ~    96000 - 4G pagetable (will be reload later)
    #
    movl %edx, %cr3

    #
    # Enable long mode (set EFER.LME=1).
    #
    movl $IA32_EFER, %ecx
    rdmsr
    orw  $0x100, %ax
    wrmsr                 # Write EFER.

    #
    # Enable protected mode and paging to activate long mode (set CR0.PE=1, CR0.PG=1)
    #
    movl  %cr0, %eax        # Read CR0.
    orl   $0x80000001, %eax # Set PE, PG
    movl  %eax, %cr0        # Write CR0.

.equ JumpToLongMode, . + 2
    ljmpl $0x38, $InLongMode      # 0x38 is SYS_CODE64_SEL

InLongMode:

    .code64

    movw    $SYS_DATA_SEL,%ax
    movw    %ax,%ds
    movw    %ax,%es
    movw    %ax,%ss
    leaq    MyStack(%rip), %rsp     # Reload RSP

    #
    # load IDT
    #
    lidtq   idtr(%rip)

    jmp     BlockSignature + 2

    .code16

/*
Empty8042InputBuffer:
        xorw    %cx,%cx
Empty8042Loop:
        outw    %ax, $DELAY_PORT            # Delay 1us
        inb     $KBD_STATUS_PORT, %al       # Read the 8042 Status Port
        andb    $0x2,%al                    # Check the Input Buffer Full Flag
        loopnz  Empty8042Loop               # Loop until the input buffer is empty or a timout of 65536 uS
        ret
*/

#
# Find place for page table and create it
#

.equ EFILDR_BASE, 0x2000            # Offset to start of EFILDR block
.equ EFILDR_FILE_LENGTH, 8          # Dword in EFILDR_HEADER holding size of block
.equ EBDA_SEG, 0x40                 # Segment:Offset for finding the EBDA
.equ EBDA_OFFSET, 0xE

CreatePageTable:
    movl (EFILDR_BASE + EFILDR_FILE_LENGTH), %edx       # Size of EFILDR block -> EDX
    addl $(EFILDR_BASE + 15), %edx                      # Add base
    shrl $4, %edx                                       # And round up to multiple of 16
    movw %ds, %ax
    addw %ax, %dx                                       # Add in linear base
    addw $255, %dx
    xorb %dl, %dl                                       # And round up to page size
# DX holds 16-bit segment of page table

    movw %ds, %cx                   # Save DS
    movw $EBDA_SEG, %ax
    addb $6, %dh                    # Need 6 pages for table
    movw %ax, %ds
    movw EBDA_OFFSET, %ax           # EBDA 16-bit segment now in AX
    movw %cx, %ds                   # Restore DS
    cmpw %dx, %ax                   # Does page table fit under EBDA?
    jae 1f                          # Yes, continue
    jmp PageTableError              # No, abort
1:
    subb $6, %dh                    # Restore DX to start segment of page table
    movw %dx, PageTableSegment      # Stash it for client
    pushw %es
    pushw %di                       # Save ES:DI used to build page table

    movw %dx, %es
    xorw %di, %di                   # ES:DI points to start of page table
    incb %dh                        # Bump DX to next page

#
# Set up page table root page (only 1 entry)
#
    xorl %eax, %eax
    movw %dx, %ax
    incb %dh                        # Bump DX to next page
    shll $4, %eax
    orb $3, %al
    stosl
    xorl %eax, %eax
    movw $2046, %cx
    rep stosw                       # Wipe rest of 1st page

#
# Set up page table 2nd page (depth 1 - 4 entries)
#
    movw $4, %cx
2:
    movw %dx, %ax
    incb %dh                        # Bump DX to next page
    shll $4, %eax
    orb $3, %al
    stosl
    xorl %eax, %eax
    stosl
    loop 2b
    movw $2032, %cx                 # Wipe rest of 2nd page
    rep stosw

#
# Set up pages 3 - 6 (depth 2 - 2048 entries)
#
    xorl %edx, %edx                 # Start at base of memory
    movb $0x83, %dl                 # Flags at leaf nodes mark large pages (2MB each)
    movw $2048, %cx
3:
    movl %edx, %eax
    addl $0x200000, %edx            # Bump EDX to next large page
    stosl
    xorl %eax, %eax
    stosl
    loop 3b

#
# Done - restore ES:DI and return
#
    popw %di
    popw %es
    ret

#
# Get here if not enough space between boot file
#   and bottom of the EBDA - print error and halt
#
PageTableError:
    addw $2, %sp        # Clear return address of CreatePageTable
    movw $15, %bx
    movw $PageErrorMsg, %si
1:
    lodsb
    testb %al, %al
    jz 2f
    movb $14, %ah
    int $16
    jmp 1b
2:
    hlt
    jmp 2b

##############################################################################
# data
##############################################################################

        .p2align 1

PageTableSegment:   .word 0
PageErrorMsg:       .asciz "Unable to Allocate Memory for Page Table"

        .p2align 1

gdtr:   .word GDT_END - GDT_BASE - 1    # GDT limit
        .long GDT_BASE                  # (GDT base gets adjusted above)
##############################################################################
#   global descriptor table (GDT)
##############################################################################

        .p2align 1

GDT_BASE: 
# null descriptor
.equ            NULL_SEL, .-GDT_BASE        # Selector [0x0]
        .word 0         # limit 15:0
        .word 0         # base 15:0
        .byte 0         # base 23:16
        .byte 0         # type
        .byte 0         # limit 19:16, flags
        .byte 0         # base 31:24

# linear data segment descriptor
.equ            LINEAR_DATA_SEL, .-GDT_BASE # Selector [0x8]
        .word 0xFFFF    # limit 0xFFFFF
        .word 0         # base 0
        .byte 0
        .byte 0x92      # present, ring 0, data, expand-up, writable
        .byte 0xCF      # page-granular, 32-bit
        .byte 0

# linear code segment descriptor
.equ            LINEAR_CODE_SEL, .-GDT_BASE # Selector [0x10]
        .word 0xFFFF    # limit 0xFFFFF
        .word 0         # base 0
        .byte 0
        .byte 0x9A      # present, ring 0, code, non-conforming, readable
        .byte 0xCF      # page-granular, 32-bit
        .byte 0

# system data segment descriptor
.equ            SYS_DATA_SEL, .-GDT_BASE    # Selector [0x18]
        .word 0xFFFF    # limit 0xFFFFF
        .word 0         # base 0
        .byte 0
        .byte 0x92      # present, ring 0, data, expand-up, writable
        .byte 0xCF      # page-granular, 32-bit
        .byte 0

# system code segment descriptor
.equ            SYS_CODE_SEL, .-GDT_BASE    # Selector [0x20]
        .word 0xFFFF    # limit 0xFFFFF
        .word 0         # base 0
        .byte 0
        .byte 0x9A      # present, ring 0, code, non-conforming, readable
        .byte 0xCF      # page-granular, 32-bit
        .byte 0

# spare segment descriptor
.equ            SPARE3_SEL, .-GDT_BASE      # Selector [0x28]
        .word 0         # limit 0
        .word 0         # base 0
        .byte 0         #
        .byte 0         # non-present, ring 0, system, reserved
        .byte 0         #
        .byte 0

#
# system data segment descriptor
#
.equ            SYS_DATA64_SEL, .-GDT_BASE  # Selector [0x30]
        .word 0xFFFF    # limit 0xFFFFF
        .word 0         # base 0
        .byte 0
        .byte 0x92      # present, ring 0, data, expand-up, writable
        .byte 0xCF      # page-granular, 32-bit
        .byte 0

#
# system code segment descriptor
#
.equ            SYS_CODE64_SEL, .-GDT_BASE  # Selector [0x38]
        .word 0xFFFF    # limit 0xFFFFF
        .word 0         # base 0
        .byte 0
        .byte 0x9A      # present, ring 0, code, non-conforming, readable
        .byte 0xAF      # page-granular, 64-bit
        .byte 0

# spare segment descriptor
.equ            SPARE4_SEL, .-GDT_BASE      # Selector [0x40]
        .word 0         # limit 0
        .word 0         # base 0
        .byte 0
        .byte 0         # non-present, ring 0, system, reserved
        .byte 0         #
        .byte 0

GDT_END:

        .p2align 1

idtr:   .word IDT_END - IDT_BASE - 1    # IDT limit
        .quad IDT_BASE                  # (IDT base gets adjusted above)

##############################################################################
#   interrupt descriptor table (IDT)
#
#   Note: The hardware IRQ's specified in this table are the normal PC/AT IRQ
#       mappings.  This implementation only uses the system timer and all other
#       IRQs will remain masked.  The descriptors for vectors 33+ are provided
#       for convenience.
##############################################################################

        .p2align 1

IDT_BASE: 
# divide by zero (INT 0)
.equ                DIV_ZERO_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# debug exception (INT 1)
.equ                DEBUG_EXCEPT_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# NMI (INT 2)
.equ                NMI_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# soft breakpoint (INT 3)
.equ                BREAKPOINT_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# overflow (INT 4)
.equ                OVERFLOW_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# bounds check (INT 5)
.equ                BOUNDS_CHECK_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# invalid opcode (INT 6)
.equ                INVALID_OPCODE_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# device not available (INT 7)
.equ                DEV_NOT_AVAIL_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# double fault (INT 8)
.equ                DOUBLE_FAULT_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# Coprocessor segment overrun - reserved (INT 9)
.equ                RSVD_INTR_SEL1, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# invalid TSS (INT 0ah)
.equ                INVALID_TSS_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# segment not present (INT 0bh)
.equ                SEG_NOT_PRESENT_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# stack fault (INT 0ch)
.equ                STACK_FAULT_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# general protection (INT 0dh)
.equ                GP_FAULT_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# page fault (INT 0eh)
.equ                PAGE_FAULT_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# Intel reserved - do not use (INT 0fh)
.equ                RSVD_INTR_SEL2, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# floating point error (INT 10h)
.equ                FLT_POINT_ERR_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# alignment check (INT 11h)
.equ                ALIGNMENT_CHECK_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# machine check (INT 12h)
.equ                MACHINE_CHECK_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# SIMD floating-point exception (INT 13h)
.equ                SIMD_EXCEPTION_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# 84 unspecified descriptors, First 12 of them are reserved, the rest are avail
        .fill 84 * 16, 1, 0   # db (84 * 16) dup(0)

# IRQ 0 (System timer) - (INT 68h)
.equ                IRQ0_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# IRQ 1 (8042 Keyboard controller) - (INT 69h)
.equ                IRQ1_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# Reserved - IRQ 2 redirect (IRQ 2) - DO NOT USE!!! - (INT 6ah)
.equ                IRQ2_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# IRQ 3 (COM 2) - (INT 6bh)
.equ                IRQ3_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# IRQ 4 (COM 1) - (INT 6ch)
.equ                IRQ4_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# IRQ 5 (LPT 2) - (INT 6dh)
.equ                IRQ5_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# IRQ 6 (Floppy controller) - (INT 6eh)
.equ                IRQ6_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# IRQ 7 (LPT 1) - (INT 6fh)
.equ                IRQ7_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# IRQ 8 (RTC Alarm) - (INT 70h)
.equ                IRQ8_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# IRQ 9 - (INT 71h)
.equ                IRQ9_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# IRQ 10 - (INT 72h)
.equ                 IRQ10_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# IRQ 11 - (INT 73h)
.equ                 IRQ11_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# IRQ 12 (PS/2 mouse) - (INT 74h)
.equ                 IRQ12_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# IRQ 13 (Floating point error) - (INT 75h)
.equ                 IRQ13_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# IRQ 14 (Secondary IDE) - (INT 76h)
.equ                 IRQ14_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

# IRQ 15 (Primary IDE) - (INT 77h)
.equ                 IRQ15_SEL, .-IDT_BASE
        .word 0               # offset 15:0
        .word SYS_CODE64_SEL  # selector 15:0
        .byte 0               # 0 for interrupt gate
        .byte 0x0e | 0x80     # type = 386 interrupt gate, present
        .word 0               # offset 31:16
        .long 0               # offset 63:32
        .long 0               # 0 for reserved

        .fill 16, 1, 0

IDT_END:

        .p2align 1

MemoryMapSize:  .long 0
MemoryMap:  .fill 267, 4, 0

        .org 0x0de0

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

        .org 0x0dfe
BlockSignature:
        .word 0xaa55
