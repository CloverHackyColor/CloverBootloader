; Copyright (c) 1999-2003 Apple Computer, Inc. All rights reserved.
;
; @APPLE_LICENSE_HEADER_START@
; 
; Portions Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights
; Reserved.  This file contains Original Code and/or Modifications of
; Original Code as defined in and that are subject to the Apple Public
; Source License Version 2.0 (the "License").  You may not use this file
; except in compliance with the License.  Please obtain a copy of the
; License at http://www.apple.com/publicsource and read it before using
; this file.
; 
; The Original Code and all software distributed under the License are
; distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
; EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
; INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
; License for the specific language governing rights and limitations
; under the License.
; 
; @APPLE_LICENSE_HEADER_END@
;
; Boot Loader: boot0
;
; A small boot sector program written in x86 assembly whose only
; responsibility is to locate the active partition, load the
; partition booter into memory, and jump to the booter's entry point.
; It leaves the boot drive in DL and a pointer to the partition entry in SI.
; 
; This boot loader must be placed in the Master Boot Record.
;
; In order to coexist with a fdisk partition table (64 bytes), and
; leave room for a two byte signature (0xAA55) in the end, boot0 is
; restricted to 446 bytes (512 - 64 - 2). If boot0 did not have to
; live in the MBR, then we would have 510 bytes to work with.
;
; boot0 is always loaded by the BIOS or another booter to 0:7C00h.
;
; This code is written for the NASM assembler.
;   nasm boot0.s -o boot0

;
; This version of boot0 implements hybrid GUID/MBR partition scheme support
;
; Written by Tam‡s Kos‡rszky on 2008-03-10
;
; Turbo added EFI System Partition boot support
;
; Added KillerJK's switchPass2 modifications
;
; dmazar: 10/7/2011 added scanning of all BIOS accessible drives:
;   - iterates over all drives and searches for HSF bootable partition (with boot1h)
;     and loads from it
;   - if not found, itarates over drives again and searches for active partition and
;     loads from it
;
; dmazar: 19/7/2011
; Searching for bootable partition works in 3 passes now:
;
; - Pass1:
;     - for the boot drive only:
;         - searches MBR partition table for an active HSF+ bootable partition and boots it
;         - if not found and disk is actually GPT, then searches for the first HFS+ bootable
;           partition (or EFI with boot1f32) in the GPT array and boots it
;         - if still not found, then continues with Pass2
;
; - Pass2:
;     - iterates over all drives and for each drive:
;         - searches MBR partition table for the first HSF+ bootable partition and boots it
;         - if not found and disk is actually GPT, then searches for the first HFS+ bootable
;           partition (or EFI with boot1f32) in the GPT array and boots it
;         - if still not found, then continues with the next drive
;     - if all drives are searched and nothing found, then continues with Pass3

; - Pass3:
;     - iterates over all drives and for each drive:
;         - searches MBR partition table for the first active bootable partition and boots it
;         - if not found and disk is actually GPT, then searches for the first HFS+ bootable
;           partition (or EFI with boot1f32) in the GPT and boots it
;         - if still not found, then continues with the next drive
;     - if all drives are searched and nothing found, finishes with "boot0: error"
;
; Bootable partition above means a partition with the boot sector signature (0xAA55)
; at the end of the partition boot sector.
; Booting partition means loading partition boot sector and passing control to partition
; boot loader (for example boot1h).
; Drives are searched in the order defined in the BIOS. Drive which is selected as the boot drive
; is searched first.
;
; If compiled with DEBUG=1 gives debug output:
;     P - starting new pass
;     D - starting disk scanning: MBR and then GPT
;     p - checking MBR partition entry
;     t - testing MBR partition
;     l - MBR or GPT partition satisfies conditions - loading partition boot sector
;     G - found GPT
;     + - stage 1 booter loaded, press a key to continue
;     E - error
;

;
; Set to 1 to enable obscure debug messages.
;
;DEBUG				EQU  CONFIG_BOOT0_DEBUG
DEBUG				EQU  0
NOT_USED			EQU  0				; exclude print_hex - no space for it 

;
; Verbose - write boot0 messages
; No space for verbose and debug in the same time
;
;VERBOSE				EQU  CONFIG_BOOT0_VERBOSE
%if DEBUG
VERBOSE				EQU  0
%else
VERBOSE				EQU  1
%endif

;
; Various constants.
;
kBoot0Segment		EQU  0x0000
kBoot0Stack			EQU  0xFFF0			; boot0 stack pointer
kBoot0LoadAddr		EQU  0x7C00			; boot0 load address
kBoot0RelocAddr		EQU  0xE000			; boot0 relocated address

kMBRBuffer			EQU  0x1000			; MBR buffer address
kLBA1Buffer			EQU  0x1200			; LBA1 - GPT Partition Table Header buffer address
kGPTABuffer			EQU  0x1400			; GUID Partition Entry Array buffer address

kPartTableOffset	EQU  0x1be
kMBRPartTable		EQU  kMBRBuffer + kPartTableOffset

kSectorBytes		EQU  512			; sector size in bytes
kBootSignature		EQU  0xAA55			; boot sector signature
kHFSPSignature		EQU  'H+'			; HFS+ volume signature
kHFSPCaseSignature	EQU  'HX'			; HFS+ volume case-sensitive signature
kFAT32BootCodeOffset EQU  0x5a			; offset of boot code in FAT32 boot sector
kBoot1FAT32Magic	EQU  'BO'			; Magic string to detect our boot1f32 code 


kGPTSignatureLow	EQU  'EFI '			; GUID Partition Table Header Signature
kGPTSignatureHigh	EQU  'PART'
kGUIDLastDwordOffs	EQU  12				; last 4 byte offset of a GUID

kPartCount			EQU  4				; number of paritions per table
kPartTypeHFS		EQU  0xaf			; HFS+ Filesystem type
kPartTypePMBR		EQU  0xee			; On all GUID Partition Table disks a Protective MBR (PMBR)
										; in LBA 0 (that is, the first block) precedes the
										; GUID Partition Table Header to maintain compatibility
										; with existing tools that do not understand GPT partition structures.
							  			; The Protective MBR has the same format as a legacy MBR 
					  					; and contains one partition entry with an OSType set to 0xEE
										; reserving the entire space used on the disk by the GPT partitions,
										; including all headers.

kPartActive	        EQU  0x80			; active flag enabled
kPartInactive	    EQU  0x00			; active flag disabled
kHFSGUID	        EQU  0x48465300		; first 4 bytes of Apple HFS Partition Type GUID.
kAppleGUID			EQU  0xACEC4365		; last 4 bytes of Apple type GUIDs. 
kEFISystemGUID		EQU  0x3BC93EC9		; last 4 bytes of EFI System Partition Type GUID:
										; C12A7328-F81F-11D2-BA4B-00A0C93EC93B

%ifdef FLOPPY
kDriveNumber		EQU  0x00
%else
kDriveNumber		EQU  0x80
%endif

kPass1				EQU  3				; Pass1 
kPass2				EQU  2				; Pass2 
kPass3				EQU  1				; Pass3 

;
; Format of fdisk partition entry.
;
; The symbol 'part_size' is automatically defined as an `EQU'
; giving the size of the structure.
;
           struc part
.bootid    resb 1      ; bootable or not 
.head      resb 1      ; starting head, sector, cylinder
.sect      resb 1      ;
.cyl       resb 1      ;
.type      resb 1      ; partition type
.endhead   resb 1      ; ending head, sector, cylinder
.endsect   resb 1      ;
.endcyl    resb 1      ;
.lba       resd 1      ; starting lba
.sectors   resd 1      ; size in sectors
           endstruc

;
; Format of GPT Partition Table Header
;
							struc	gpth
.Signature 					resb	8 
.Revision  					resb	4
.HeaderSize					resb	4
.HeaderCRC32				resb	4
.Reserved					resb	4
.MyLBA						resb	8
.AlternateLBA				resb	8
.FirstUsableLBA				resb	8
.LastUsableLBA				resb	8
.DiskGUID					resb	16
.PartitionEntryLBA			resb	8
.NumberOfPartitionEntries	resb	4
.SizeOfPartitionEntry		resb	4
.PartitionEntryArrayCRC32	resb	4
							endstruc

;				
; Format of GUID Partition Entry Array
;
						  	struc	gpta
.PartitionTypeGUID			resb	16
.UniquePartitionGUID		resb	16
.StartingLBA				resb	8
.EndingLBA					resb	8
.Attributes					resb	8
.PartitionName				resb	72
							endstruc

;
; Macros.
;
%macro DebugCharMacro 1
    mov   al, %1
    call  print_char
%endmacro

%macro DebugPauseMacro 0
    call  getc
%endmacro

%macro LogStringMacro 1
    mov   di, %1
    call  log_string
%endmacro

%if DEBUG
%define DebugChar(x)  DebugCharMacro x
%define DebugPause  DebugPauseMacro
%else
%define DebugChar(x)
%define DebugPause
%endif

%if VERBOSE
%define LogString(x)  LogStringMacro x
%else
%define LogString(x)
%endif

;--------------------------------------------------------------------------
; Start of text segment.

    SEGMENT .text

    ORG     kBoot0RelocAddr
    
;--------------------------------------------------------------------------
; Boot code is loaded at 0:7C00h.
;
start:
    ;
    ; Set up the stack to grow down from kBoot0Segment:kBoot0Stack.
    ; Interrupts should be off while the stack is being manipulated.
    ;
    cli                             ; interrupts off
    xor     ax, ax                  ; zero ax
    mov     ss, ax                  ; ss <- 0
    mov     sp, kBoot0Stack         ; sp <- top of stack
    sti                             ; reenable interrupts

    mov     es, ax                  ; es <- 0
    mov     ds, ax                  ; ds <- 0

    ;
    ; Relocate boot0 code.
    ;
    mov     si, kBoot0LoadAddr      ; si <- source
    mov     di, kBoot0RelocAddr     ; di <- destination
    cld                             ; auto-increment SI and/or DI registers
    mov     cx, kSectorBytes/2      ; copy 256 words
    repnz   movsw                   ; repeat string move (word) operation

    ;
    ; Code relocated, jump to start_reloc in relocated location.
    ;
    jmp     kBoot0Segment:start_reloc

;--------------------------------------------------------------------------
; Start execution from the relocated location.
;
start_reloc:

	;
	; BH is pass counter
	; Pass1 BH=3, Pass2 BH=2, Pass3 BH=1
	;
    mov		bh, kPass1				; BH = 3. Pass1

pass_loop:

	DebugChar('P')					; starting new pass
	push	dx						; save dl (boot drive) for next pass
	
	
.scan_drive:

    ;
    ; Since this code may not always reside in the MBR, always start by
    ; loading the MBR to kMBRBuffer and LBA1 to kGPTBuffer.
    ;

	push	bx						; save BH (scan pass counter)
    xor     eax, eax
    mov     [my_lba], eax			; store LBA sector 0 for read_lba function
    mov     al, 2					; load two sectors: MBR and LBA1
    mov     bx, kMBRBuffer			; MBR load address
    call    load
	pop		bx						; restore BH
    jc      .next_pass				; MBR load error - normally because we scanned all drives

	DebugChar('D')					; starting disk scanning
	
    ;
    ; Look for the booter partition in the MBR partition table,
    ; which is at offset kMBRPartTable.
    ;
    mov     si, kMBRPartTable		; pointer to partition table
    call    find_boot				; will not return on success
	
	; if returns - booter partition is not found
	
	; skip scanning of all drives in Pass1
	cmp		bh, kPass1
	je		.next_pass
	
	; try next drive
	; if next drive does not exists - will break on the MBR load error above
	inc		dl
	jmp		short .scan_drive
	

.next_pass:
	; all drives scanned - move to next pass
	pop		dx						; restore orig boot drive
	dec		bh						; decrement scan pass counter
	jnz		pass_loop				; if not zero - exec next pass
	
	; we ran all passes - nothing found - error
	
error:
    DebugChar('E')
    DebugPause
    LogString(boot_error_str)

hang:
    hlt
    jmp     short hang


;--------------------------------------------------------------------------
; Find the active (boot) partition and load the booter from the partition.
;
; Arguments:
;   DL = drive number (0x80 + unit number)
;   SI = pointer to fdisk partition table.
;   BH = pass counter
;
; Clobber list:
;   EAX, BX, EBP
;
find_boot:

    ;
    ; Check for boot block signature 0xAA55 following the 4 partition
    ; entries.
    ;
    cmp     WORD [si + part_size * kPartCount], kBootSignature
    jne	    .exit        	  			; boot signature not found.

    xor	    bl, bl						; BL will be set to 1 later in case of
										; Protective MBR has been found

.start_scan:							
    mov     cx, kPartCount          	; number of partition entries per table

.loop:

	DebugChar('p')									; checking partition entry
    mov	    eax, [si + part.lba]					; save starting LBA of current 
    mov	    [my_lba], eax							; MBR partition entry for read_lba function
    cmp     BYTE [si + part.type], 0				; unused partition?
    je      .continue  								; skip to next entry
    cmp	    BYTE [si + part.type], kPartTypePMBR	; check for Protective MBR 
    jne	    .testPass

    mov     BYTE [si + part.bootid], kPartInactive	; found Protective MBR
        											; clear active flag to make sure this protective
													; partition won't be used as a bootable partition.
    mov	    bl, 1									; Assume we can deal with GPT but try to scan
					    							; later if not found any other bootable partitions.

	;
	; The following code between .testPass and .tryToBoot performs checking for 3 passes: 
	; Pass1 (BH=3) if (partition is HFS+ and active) then { DH=1; call loadBootSector} 
	; Pass2 (BH=2) if (partition is HFS+) then { DH=1; call loadBootSector}
	; Pass3 (BH=1) if (partition is active) then { DH=0; call loadBootSector}
	;
	; BH is Pass counter
	; DH is argument to loadBootSector
	;  = 0 - skip HFS+ partition signature check
	;  = 1 - check for HFS+ partition signature
	; 
	; Code may be harder to read because I tried to optimized it for minimum size.
	;
													
.testPass:
	DebugChar('t')									; testing partition
    xor		dh, dh               					; DH=0 This will be used in Pass3 (partition is active, not HFS+).
    												
    cmp	    bh, kPass3								; If this is Pass3 (BH=1)
    je	    .checkActive							; check for active flag only.

.checkHFS:
													; We are in Pass1 (BH=3) or Pass2 (BH=2).
  	inc		dh	                					; DH=1
    cmp	    BYTE [si + part.type], kPartTypeHFS		; Check for a HFS+ partition.
    jne     .continue

    cmp	    bh, kPass2								; It's HFS+. That's enough checking for Pass2,
    je	    .tryToBoot								; so try to boot (with DH=1)
    												; Pass1 needs active flag check also ...

.checkActive:
													; We are in Pass1 or Pass3
    cmp     BYTE [si + part.bootid], kPartActive	; Check if partition is Active
    jne     .continue


    ;
    ; Found boot partition, read boot sector to memory.
    ;

.tryToBoot:

    call    loadBootSector
    jne     .continue
    jmp	    SHORT initBootLoader

.continue:
    add     si, BYTE part_size     			; advance SI to next partition entry
    loop    .loop                 		 	; loop through all partition entries

    ;
    ; Scanned all partitions but not found any with active flag enabled
    ; Anyway if we found a protective MBR before we still have a chance 
    ; for a possible GPT Header at LBA 1
    ;    
    dec	    bl
    jnz     .exit							; didn't find Protective MBR before
    call    checkGPT

.exit:
    ret										; Giving up.


    ;
    ; Jump to partition booter. The drive number is already in register DL.
    ; SI is pointing to the modified partition entry.
    ;
initBootLoader:    

    DebugChar('+')
    DebugPause

    LogString(done_str)

    jmp     kBoot0LoadAddr

    
    ; 
    ; Found Protective MBR Partition Type: 0xEE
    ; Check for 'EFI PART' string at the beginning
    ; of LBA1 for possible GPT Table Header
    ;
checkGPT:
    push    bx

    mov	    di, kLBA1Buffer						; address of GUID Partition Table Header
    cmp	    DWORD [di], kGPTSignatureLow		; looking for 'EFI '
    jne	    .exit								; not found. Giving up.
    cmp	    DWORD [di + 4], kGPTSignatureHigh   ; looking for 'PART'
    jne	    .exit								; not found. Giving up indeed.

    DebugChar('G')								; found GPT
    mov	    si, di
    
    ;
    ; Loading GUID Partition Table Array
    ;
    mov     eax, [si + gpth.PartitionEntryLBA]          ; starting LBA of GPT Array
    mov     [my_lba], eax								; save starting LBA for read_lba function
    mov     cx, [si + gpth.NumberOfPartitionEntries]	; number of GUID Partition Array entries
    mov     bx, [si + gpth.SizeOfPartitionEntry]		; size of GUID Partition Array entry
    push    bx											; push size of GUID Partition entry

    ;
    ; Calculating number of sectors we need to read for loading a GPT Array
    ;
;    push    dx							; preserve DX (DL = BIOS drive unit number)
;    mov	    ax, cx					; AX * BX = number of entries * size of one entry
;    mul     bx							; AX = total byte size of GPT Array
;    pop	    dx						; restore DX
;    shr     ax, 9						; convert to sectors

    ;
    ; ... or:
    ; Current GPT Arrays uses 128 partition entries each 128 bytes long
    ; 128 entries * 128 bytes long GPT Array entries / 512 bytes per sector = 32 sectors
    ;
	mov		al, 32					; maximum sector size of GPT Array (hardcoded method)

    mov	    bx, kGPTABuffer
    push    bx						; push address of GPT Array
    call    load					; read GPT Array
    pop	    si						; SI = address of GPT Array
    pop	    bx						; BX = size of GUID Partition Array entry
    ;jc       error
	jc       .exit					; dmazar's change to continue disk scanning if encountering invalid LBA.

    ;
    ; Walk through GUID Partition Table Array
    ; and load boot record from first available HFS+ partition.
    ;
    ; If it has boot signature (0xAA55) then jump to it
    ; otherwise skip to next partition.
    ;

    LogString(gpt_str)

.gpt_loop:

    mov     eax, [si + gpta.PartitionTypeGUID + kGUIDLastDwordOffs]
	
	cmp		eax, kAppleGUID			; check current GUID Partition for Apple's GUID type
	je		.gpt_ok

	;
	; Turbo - also try EFI System Partition
	;

	cmp		eax, kEFISystemGUID		; check current GUID Partition for EFI System Partition GUID type
	jne		.gpt_continue

.gpt_ok:
    ;
    ; Found HFS Partition
    ;

    mov	    eax, [si + gpta.StartingLBA]			; load boot sector from StartingLBA
    mov	    [my_lba], eax		
	mov		dh, 1									; Argument for loadBootSector to check HFS+ partition signature.
    call    loadBootSector
    jne	    .gpt_continue							; no boot loader signature

    mov	    si, kMBRPartTable						; fake the current GUID Partition
    mov	    [si + part.lba], eax					; as MBR style partition for boot1h
    mov     BYTE [si + part.type], kPartTypeHFS		; with HFS+ filesystem type (0xAF)
    jmp	    SHORT initBootLoader    
    
.gpt_continue:

    add	    si, bx									; advance SI to next partition entry
    loop    .gpt_loop								; loop through all partition entries	

.exit:
    pop     bx
    ret												; no more GUID partitions. Giving up.


;--------------------------------------------------------------------------
; loadBootSector - Load boot sector
;
; Arguments:
;   DL = drive number (0x80 + unit number)
;   DH = 0 skip HFS+ partition signature checking
;        1 enable HFS+ partition signature checking
;   [my_lba] = starting LBA.
;
; Returns:
;   ZF = 0 if boot sector hasn't kBootSignature
;        1 if boot sector has kBootSignature
;
loadBootSector:
    pusha
    
    DebugChar('l')									; loading partition boot sector

    mov     al, 3
    mov     bx, kBoot0LoadAddr
    call    load
    ;jc      error
    or   dl, dl ; to set flag Z=0	; dmazar's change to continue disk scanning if encountering invalid LBA.
    jc      .exit					; dmazar's change to continue disk scanning if encountering invalid LBA.

	or		dh, dh
	jz		.checkBootSignature
	
.checkHFSSignature:

    ;LogString(test_str)			; dmazar: removed to get space

	;
	; Looking for HFSPlus ('H+') or HFSPlus case-sensitive ('HX') signature.
	;
	mov		ax, [kBoot0LoadAddr + 2 * kSectorBytes]
    cmp     ax, kHFSPSignature		; 'H+'
	je		.checkBootSignature
	cmp		ax, kHFSPCaseSignature	; 'HX'
    je		.checkBootSignature
	
	;
	; Looking for boot1f32 magic string.
	;
	mov		ax, [kBoot0LoadAddr + kFAT32BootCodeOffset]
	cmp		ax, kBoot1FAT32Magic
    jne     .exit

.checkBootSignature:
    ;
    ; Check for boot block signature 0xAA55
    ;
    mov	    di, bx
    cmp     WORD [di + kSectorBytes - 2], kBootSignature

.exit:

    popa

    ret


;--------------------------------------------------------------------------
; load - Load one or more sectors from a partition.
;
; Arguments:
;   AL = number of 512-byte sectors to read.
;   ES:BX = pointer to where the sectors should be stored.
;   DL = drive number (0x80 + unit number)
;   [my_lba] = starting LBA.
;
; Returns:
;   CF = 0 success
;        1 error
;
load:
    push    cx

.ebios:
    mov     cx, 5                   ; load retry count
.ebios_loop:
    call    read_lba                ; use INT13/F42
    jnc     .exit
    loop    .ebios_loop

.exit:
    pop     cx
    ret

;--------------------------------------------------------------------------
; read_lba - Read sectors from a partition using LBA addressing.
;
; Arguments:
;   AL = number of 512-byte sectors to read (valid from 1-127).
;   ES:BX = pointer to where the sectors should be stored.
;   DL = drive number (0x80 + unit number)
;   [my_lba] = starting LBA.
;
; Returns:
;   CF = 0 success
;        1 error
;
read_lba:
    pushad                          ; save all registers
    mov     bp, sp                  ; save current SP

    ;
    ; Create the Disk Address Packet structure for the
    ; INT13/F42 (Extended Read Sectors) on the stack.
    ;

;   push    DWORD 0                 ; offset 12, upper 32-bit LBA
    push    ds                      ; For sake of saving memory,
    push    ds                      ; push DS register, which is 0.
    mov     ecx, [my_lba]           ; offset 8, lower 32-bit LBA
    push    ecx
    push    es                      ; offset 6, memory segment
    push    bx                      ; offset 4, memory offset
    xor     ah, ah                  ; offset 3, must be 0
    push    ax                      ; offset 2, number of sectors

    ; It pushes 2 bytes with a smaller opcode than if WORD was used
    push    BYTE 16                 ; offset 0-1, packet size

    ;
    ; INT13 Func 42 - Extended Read Sectors
    ;
    ; Arguments:
    ;   AH    = 0x42
    ;   DL    = drive number (80h + drive unit)
    ;   DS:SI = pointer to Disk Address Packet
    ;
    ; Returns:
    ;   AH    = return status (sucess is 0)
    ;   carry = 0 success
    ;           1 error
    ;
    ; Packet offset 2 indicates the number of sectors read
    ; successfully.
    ;
    mov     si, sp
    mov     ah, 0x42
    int     0x13

    jnc     .exit

    ;
    ; Issue a disk reset on error.
    ; Should this be changed to Func 0xD to skip the diskette controller
    ; reset?
    ;
    xor     ax, ax                  ; Func 0
    int     0x13                    ; INT 13
    stc                             ; set carry to indicate error

.exit:
    mov     sp, bp                  ; restore SP
    popad
    ret


%if VERBOSE

;--------------------------------------------------------------------------
; Write a string with 'boot0: ' prefix to the console.
;
; Arguments:
;   ES:DI   pointer to a NULL terminated string.
;
; Clobber list:
;   DI
;
log_string:
    pusha
        
    push	di
    mov		si, log_title_str
    call	print_string

    pop		si
    call	print_string

    popa
    
    ret

    
;--------------------------------------------------------------------------
; Write a string to the console.
;
; Arguments:
;   DS:SI   pointer to a NULL terminated string.
;
; Clobber list:
;   AX, BX, SI
;
print_string:
    mov     bx, 1                   ; BH=0, BL=1 (blue)
    cld                             ; increment SI after each lodsb call
.loop:
    lodsb                           ; load a byte from DS:SI into AL
    cmp     al, 0                   ; Is it a NULL?
    je      .exit                   ; yes, all done
    mov     ah, 0xE                 ; INT10 Func 0xE
    int     0x10                    ; display byte in tty mode
    jmp     short .loop
.exit:
    ret

%endif ;VERBOSE


%if DEBUG

;--------------------------------------------------------------------------
; Write a ASCII character to the console.
;
; Arguments:
;   AL = ASCII character.
;
print_char:
    pusha
    mov     bx, 1                   ; BH=0, BL=1 (blue)
    mov     ah, 0x0e                ; bios INT 10, Function 0xE
    int     0x10                    ; display byte in tty mode
    popa
    ret

getc:
    pusha
    mov    ah, 0
    int    0x16
    popa
    ret
%endif ;DEBUG
	
%if NOT_USED

;--------------------------------------------------------------------------
; Write the 4-byte value to the console in hex.
;
; Arguments:
;   EAX = Value to be displayed in hex.
;
print_hex:
    pushad
    mov     cx, WORD 4
    bswap   eax
.loop:
    push    ax
    ror     al, 4
    call    print_nibble            ; display upper nibble
    pop     ax
    call    print_nibble            ; display lower nibble
    ror     eax, 8
    loop    .loop

    mov     al, 10                  ; carriage return
    call    print_char
    mov     al, 13
    call    print_char

    popad
    ret
	
print_nibble:
    and     al, 0x0f
    add     al, '0'
    cmp     al, '9'
    jna     .print_ascii
    add     al, 'A' - '9' - 1
.print_ascii:
    call    print_char
    ret

%endif ; NOT_USED



%if VERBOSE

;--------------------------------------------------------------------------
; NULL terminated strings.
;
log_title_str		db  10, 13, 'boot0:', 0
boot_error_str   	db  'error', 0

gpt_str			db  'GPT', 0
;test_str		db  'test', 0
done_str		db  'done', 0

%endif

;--------------------------------------------------------------------------
; Pad the rest of the 512 byte sized booter with zeroes. The last
; two bytes is the mandatory boot sector signature.
;
; If the booter code becomes too large, then nasm will complain
; that the 'times' argument is negative.

;
; According to EFI specification, maximum boot code size is 440 bytes 
;

pad_boot:
    times 440-($-$$) db 0

pad_table_and_sig:
    times 510-($-$$) db 0
    dw    kBootSignature
	
	
	ABSOLUTE 0xE400

;
; In memory variables.
;
my_lba			resd	1	; Starting LBA for read_lba function

; END
