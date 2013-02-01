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
; Partition Boot Loader: boot1h
;
; This program is designed to reside in sector 0+1 of an HFS+ partition.
; It expects that the MBR has left the drive number in DL
; and a pointer to the partition entry in SI.
; 
; This version requires a BIOS with EBIOS (LBA) support.
;
; This code is written for the NASM assembler.
;   nasm boot1.s -o boot1h

;
; This version of boot1h tries to find a stage2 boot file in the root folder.
;
; NOTE: this is an experimental version with multiple extent support.
;
; Written by Tamás Kosárszky on 2008-04-14
;

;
; Set to 1 to enable obscure debug messages.
;
DEBUG				EQU		0

;
; Set to 1 to enable unused code.
;
UNUSED				EQU		0

;
; Set to 1 to enable verbose mode.
;
VERBOSE				EQU		1

;
; Various constants.
;
NULL		   		EQU		0
CR					EQU		0x0D
LF					EQU		0x0A

mallocStart			EQU		0x1000								; start address of local workspace area
maxSectorCount		EQU		64									; maximum sector count for readSectors
maxNodeSize			EQU		16384

kSectorBytes		EQU		512									; sector size in bytes
kBootSignature		EQU		0xAA55								; boot sector signature

kBoot1StackAddress	EQU		0xFFF0								; boot1 stack pointer
kBoot1LoadAddr		EQU		0x7C00								; boot1 load address
kBoot1RelocAddr		EQU		0xE000								; boot1 relocated address
kBoot1Sector1Addr	EQU		kBoot1RelocAddr + kSectorBytes		; boot1 load address for sector 1
kHFSPlusBuffer		EQU		kBoot1Sector1Addr + kSectorBytes	; HFS+ Volume Header address

kBoot2Sectors		EQU		(480 * 1024 - 512) / kSectorBytes	; max size of 'boot' file in sectors = 448 but I want 472
kBoot2Segment		EQU		0x2000								; boot2 load segment
kBoot2Address		EQU		kSectorBytes						; boot2 load address

;
; Format of fdisk partition entry.
;
; The symbol 'part_size' is automatically defined as an `EQU'
; giving the size of the structure.
;
			struc part
.bootid		resb 1		; bootable or not 
.head		resb 1		; starting head, sector, cylinder
.sect		resb 1		;
.cyl		resb 1		;
.type		resb 1		; partition type
.endhead	resb 1		; ending head, sector, cylinder
.endsect	resb 1		;
.endcyl		resb 1		;
.lba		resd 1		; starting lba
.sectors	resd 1		; size in sectors
			endstruc

;-------------------------------------------------------------------------
; HFS+ related structures and constants
;
kHFSPlusSignature		EQU		'H+'		; HFS+ volume signature
kHFSPlusCaseSignature	EQU		'HX'		; HFS+ volume case-sensitive signature
kHFSPlusCaseSigX		EQU		'X'			; upper byte of HFS+ volume case-sensitive signature
kHFSPlusExtentDensity	EQU		8			; 8 extent descriptors / extent record

;
; HFSUniStr255
;
					struc	HFSUniStr255
.length				resw	1
.unicode			resw	255
					endstruc

;
; HFSPlusExtentDescriptor
;
					struc	HFSPlusExtentDescriptor
.startBlock			resd	1
.blockCount			resd	1
					endstruc

;
; HFSPlusForkData
;
					struc	HFSPlusForkData
.logicalSize		resq	1
.clumpSize			resd	1
.totalBlocks		resd	1
.extents			resb	kHFSPlusExtentDensity * HFSPlusExtentDescriptor_size
					endstruc

;
; HFSPlusVolumeHeader
;
					struc	HFSPlusVolumeHeader
.signature			resw	1
.version			resw	1
.attributes			resd	1
.lastMountedVersion resd	1
.journalInfoBlock	resd	1
.createDate			resd	1
.modifyDate			resd	1
.backupDate			resd	1
.checkedDate		resd	1
.fileCount			resd	1
.folderCount		resd	1
.blockSize			resd	1
.totalBlocks		resd	1
.freeBlocks			resd	1
.nextAllocation		resd	1
.rsrcClumpSize		resd	1
.dataClumpSize		resd	1
.nextCatalogID		resd	1
.writeCount			resd	1
.encodingsBitmap	resq	1
.finderInfo			resd	8
.allocationFile		resb	HFSPlusForkData_size
.extentsFile		resb	HFSPlusForkData_size
.catalogFile		resb	HFSPlusForkData_size
.attributesFile		resb	HFSPlusForkData_size
.startupFile		resb	HFSPlusForkData_size
					endstruc

;
; B-tree related structures and constants
;

kBTIndexNode		EQU		0
kBTMaxRecordLength	EQU		264					; sizeof(kHFSPlusFileThreadRecord)
kHFSRootParentID	EQU		1					; Parent ID of the root folder
kHFSRootFolderID	EQU		2					; Folder ID of the root folder
kHFSExtentsFileID	EQU		3					; File ID of the extents overflow file
kHFSCatalogFileID	EQU		4					; File ID of the catalog file
kHFSPlusFileRecord	EQU		0x200
kForkTypeData		EQU		0
kForkTypeResource	EQU		0xFF

;
; BTNodeDescriptor
;
					struc	BTNodeDescriptor
.fLink				resd	1
.bLink				resd	1
.kind				resb	1
.height				resb	1
.numRecords			resw	1
.reserved			resw	1
					endstruc

;
; BTHeaderRec
;
					struc	BTHeaderRec
.treeDepth			resw	1
.rootNode			resd	1
.leafRecords		resd	1
.firstLeafNode		resd	1
.lastLeafNode		resd	1
.nodeSize			resw	1
.maxKeyLength		resw	1
.totalNodes			resd	1
.freeNodes			resd	1
.reserved1			resw	1
.clumpSize			resd	1
.btreeType			resb	1
.keyCompareType		resb	1
.attributes			resd	1
.reserved3			resd	16
					endstruc

;
; BTIndexRec
;
					struc	BTIndexRec
.childID			resd	1
					endstruc

;
; HFSPlusCatalogKey
;
					struc	HFSPlusCatalogKey
;
; won't use the keyLength field for easier addressing data inside this structure
;
;.keyLength			resw	1

.parentID			resd	1
.nodeName			resb	HFSUniStr255_size
					endstruc

;
; HFSPlusExtentKey
;
					struc	HFSPlusExtentKey
;
; won't use the keyLength field for easier addressing data inside this structure
;
;.keyLength			resw	1

.forkType			resb	1
.pad				resb	1
.fileID				resd	1
.startBlock			resd	1
					endstruc

;
; HFSPlusBSDInfo
;
					struc	HFSPlusBSDInfo
.ownerID			resd	1
.groupID			resd	1
.adminFlags			resb	1
.ownerFlags			resb	1
.fileMode			resw	1
.special			resd	1
					endstruc
					
;
; FileInfo
;
					struc	FileInfo
.fileType			resd	1
.fileCreator		resd	1
.finderFlags		resw	1
.location			resw	2
.reservedField		resw	1
					endstruc

;
; ExtendedFileInfo
;
					struc	ExtendedFileInfo
.reserved1			resw	4
.extFinderFlags		resw	1
.reserved2			resw	1
.putAwayFolderID	resd	1
					endstruc

;
; HFSPlusCatalogFile
;
					struc	HFSPlusCatalogFile
.recordType			resw	1
.flags				resw	1
.reserved1			resd	1
.fileID				resd	1
.createDate			resd	1
.contentModDate		resd	1
.attributeModDate	resd	1
.accessDate			resd	1
.backupDate			resd	1
.permissions		resb	HFSPlusBSDInfo_size
.userInfo			resb	FileInfo_size
.finderInfo			resb	ExtendedFileInfo_size
.textEncoding		resd	1
.reserved2			resd	1
.dataFork			resb	HFSPlusForkData_size
.resourceFork		resb	HFSPlusForkData_size
					endstruc

;
; Macros.
;
%macro jmpabs 1
	push	WORD %1
	ret
%endmacro

%macro DebugCharMacro 1
	pushad
	mov		al, %1
	call	print_char
	call	getc
	popad
%endmacro

%macro PrintCharMacro 1
	pushad
	mov		al, %1
	call	print_char
	popad
%endmacro

%macro PutCharMacro 1
	call	print_char
%endmacro

%macro PrintHexMacro 1
	call	print_hex
%endmacro

%macro PrintString 1
	mov		si, %1
	call	print_string
%endmacro
        
%macro LogString 1
	mov		di, %1
	call	log_string
%endmacro

%if DEBUG
  %define DebugChar(x) DebugCharMacro x
  %define PrintChar(x) PrintCharMacro x
  %define PutChar(x) PutCharMacro
  %define PrintHex(x) PrintHexMacro x
%else
  %define DebugChar(x)
  %define PrintChar(x)
  %define PutChar(x)
  %define PrintHex(x)
%endif
	
;--------------------------------------------------------------------------
; Start of text segment.

    SEGMENT .text

	ORG		kBoot1RelocAddr

;--------------------------------------------------------------------------
; Boot code is loaded at 0:7C00h.
;
start:
    ;
    ; Set up the stack to grow down from kBoot1StackSegment:kBoot1StackAddress.
    ; Interrupts should be off while the stack is being manipulated.
    ;
    cli                             ; interrupts off
    xor		ax, ax                  ; zero ax
    mov		ss, ax                  ; ss <- 0
    mov     sp, kBoot1StackAddress  ; sp <- top of stack
    sti                             ; reenable interrupts

    mov     ds, ax                  ; ds <- 0
    mov     es, ax                  ; es <- 0

    ;
    ; Relocate boot1 code.
    ;
    push	si
    mov		si, kBoot1LoadAddr		; si <- source
    mov		di, kBoot1RelocAddr		; di <- destination
    cld								; auto-increment SI and/or DI registers
    mov		cx, kSectorBytes		; copy 256 words
    rep		movsb					; repeat string move (word) operation
    pop		si
    
    ;
    ; Code relocated, jump to startReloc in relocated location.
    ;
	; FIXME: Is there any way to instruct NASM to compile a near jump
	; using absolute address instead of relative displacement?
	;
	jmpabs	startReloc

;--------------------------------------------------------------------------
; Start execution from the relocated location.
;
startReloc:

    ;
    ; Initializing global variables.
    ;
    mov     eax, [si + part.lba]
    mov     [gPartLBA], eax					; save the current partition LBA offset
    mov     [gBIOSDriveNumber], dl			; save BIOS drive number
	mov		WORD [gMallocPtr], mallocStart	; set free space pointer

    ;
    ; Loading upper 512 bytes of boot1h and HFS+ Volume Header.
    ;
	xor		ecx, ecx						; sector 1 of current partition
	inc		ecx
    mov     al, 2							; read 2 sectors: sector 1 of boot1h + HFS+ Volume Header
    mov     edx, kBoot1Sector1Addr
    call    readLBA

    ;
    ; Initializing more global variables.
    ;
	mov		eax, [kHFSPlusBuffer + HFSPlusVolumeHeader.blockSize]
	bswap	eax								; convert to little-endian
	shr		eax, 9							; convert to sector unit
	mov		[gBlockSize], eax				; save blockSize as little-endian sector unit!

	;
	; Looking for HFSPlus ('H+') or HFSPlus case-sensitive ('HX') signature.
	;
	mov		ax, [kHFSPlusBuffer + HFSPlusVolumeHeader.signature]
	cmp		ax, kHFSPlusCaseSignature
	je		findRootBoot
    cmp     ax, kHFSPlusSignature
    jne     error

;--------------------------------------------------------------------------
; Find stage2 boot file in a HFS+ Volume's root folder.
;
findRootBoot:
	mov		al, kHFSCatalogFileID
	lea		si, [searchCatalogKey]
	lea		di, [kHFSPlusBuffer + HFSPlusVolumeHeader.catalogFile + HFSPlusForkData.extents]
	call	lookUpBTree
	jne		error

	lea		si, [bp + BTree.recordDataPtr]
	mov		si, [si]
	cmp		WORD [si], kHFSPlusFileRecord
	jne		error

;  EAX = Catalog File ID
;  	BX = read size in sectors
;  ECX = file offset in sectors
;  EDX = address of read buffer
;   DI = address of HFSPlusForkData

	;
	; Use the second big-endian double-word as the file length in HFSPlusForkData.logicalSize
	;
	mov		ebx, [si + HFSPlusCatalogFile.dataFork + HFSPlusForkData.logicalSize + 4]
	bswap	ebx									; convert file size to little-endian
	add		ebx, kSectorBytes - 1				; adjust size before unit conversion
	shr		ebx, 9								; convert file size to sector unit
	cmp		bx, kBoot2Sectors					; check if bigger than max stage2 size
	ja		error
	mov		eax, [si + HFSPlusCatalogFile.fileID]
	bswap	eax									; convert fileID to little-endian
	xor		ecx, ecx
	mov		edx, (kBoot2Segment << 4) + kBoot2Address
	lea		di, [si + HFSPlusCatalogFile.dataFork + HFSPlusForkData.extents]
	call	readExtent

%if VERBOSE
	LogString(bootfile_msg)
%endif

boot2:

%if DEBUG
	DebugChar ('!')
%endif

%if UNUSED
	;
	; Waiting for a key press.
	;

    mov     ah, 0
    int		0x16
%endif

    mov     dl, [gBIOSDriveNumber]			; load BIOS drive number
    jmp     kBoot2Segment:kBoot2Address

error:

%if VERBOSE
    LogString(error_str)
%endif
	
hang:
    hlt
    jmp     hang

;--------------------------------------------------------------------------
; readSectors - Reads more than 127 sectors using LBA addressing.
;
; Arguments:
;   AX = number of 512-byte sectors to read (valid from 1-1280).
;   EDX = pointer to where the sectors should be stored.
;   ECX = sector offset in partition 
;
; Returns:
;   CF = 0  success
;        1 error
;
readSectors:
	pushad
	mov		bx, ax

.loop:
	xor		eax, eax						; EAX = 0
	mov		al, bl							; assume we reached the last block.
	cmp		bx, maxSectorCount				; check if we really reached the last block
	jb		.readBlock						; yes, BX < MaxSectorCount
	mov		al, maxSectorCount				; no, read MaxSectorCount

.readBlock:
	call	readLBA
	sub		bx, ax							; decrease remaning sectors with the read amount
	jz		.exit							; exit if no more sectors left to be loaded
	add		ecx, eax						; adjust LBA sector offset
	shl		ax, 9							; convert sectors to bytes
	add		edx, eax						; adjust target memory location
	jmp		.loop							; read remaining sectors

.exit:
	popad
	ret

;--------------------------------------------------------------------------
; readLBA - Read sectors from a partition using LBA addressing.
;
; Arguments:
;   AL = number of 512-byte sectors to read (valid from 1-127).
;   EDX = pointer to where the sectors should be stored.
;   ECX = sector offset in partition 
;   [bios_drive_number] = drive number (0x80 + unit number)
;
; Returns:
;   CF = 0  success
;        1 error
;
readLBA:
    pushad                          		; save all registers
    push    es								; save ES
    mov     bp, sp                 			; save current SP

    ;
    ; Convert EDX to segment:offset model and set ES:BX
    ;
    ; Some BIOSes do not like offset to be negative while reading
    ; from hard drives. This usually leads to "boot1: error" when trying
    ; to boot from hard drive, while booting normally from USB flash.
    ; The routines, responsible for this are apparently different.
    ; Thus we split linear address slightly differently for these
    ; capricious BIOSes to make sure offset is always positive.
    ;

	mov		bx, dx							; save offset to BX
	and		bh, 0x0f						; keep low 12 bits
	shr		edx, 4							; adjust linear address to segment base
	xor		dl, dl							; mask low 8 bits
	mov		es, dx							; save segment to ES

    ;
    ; Create the Disk Address Packet structure for the
    ; INT13/F42 (Extended Read Sectors) on the stack.
    ;

    ; push    DWORD 0              			; offset 12, upper 32-bit LBA
    push    ds                      		; For sake of saving memory,
    push    ds                      		; push DS register, which is 0.

    add     ecx, [gPartLBA]         		; offset 8, lower 32-bit LBA
    push    ecx

    push    es                      		; offset 6, memory segment

    push    bx                      		; offset 4, memory offset

    xor     ah, ah             				; offset 3, must be 0
    push    ax                      		; offset 2, number of sectors

    push    WORD 16                 		; offset 0-1, packet size

    ;
    ; INT13 Func 42 - Extended Read Sectors
    ;
    ; Arguments:
    ;   AH    = 0x42
    ;   [bios_drive_number] = drive number (0x80 + unit number)
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
	mov     dl, [gBIOSDriveNumber]			; load BIOS drive number
	mov     si, sp
	mov     ah, 0x42
	int     0x13

	jc		error

    ;
    ; Issue a disk reset on error.
    ; Should this be changed to Func 0xD to skip the diskette controller
    ; reset?
    ;
;	xor     ax, ax                  		; Func 0
;	int     0x13                    		; INT 13
;	stc                             		; set carry to indicate error

.exit:
    mov     sp, bp                  		; restore SP
    pop     es								; restore ES
    popad
    ret

%if VERBOSE

;--------------------------------------------------------------------------
; Write a string with 'boot1: ' prefix to the console.
;
; Arguments:
;   ES:DI   pointer to a NULL terminated string.
;
; Clobber list:
;   DI
;
log_string:
    pushad

    push	di
    mov		si, log_title_str
    call	print_string

    pop		si
    call	print_string

    popad
    
    ret

;-------------------------------------------------------------------------
; Write a string to the console.
;
; Arguments:
;   DS:SI   pointer to a NULL terminated string.
;
; Clobber list:
;   AX, BX, SI
;
print_string:
    mov     bx, 1                   		; BH=0, BL=1 (blue)

.loop:
    lodsb                           		; load a byte from DS:SI into AL
    cmp     al, 0               			; Is it a NULL?
    je      .exit                   		; yes, all done
    mov     ah, 0xE                 		; INT10 Func 0xE
    int     0x10                    		; display byte in tty mode
    jmp     .loop

.exit:
    ret

%endif ; VERBOSE

%if DEBUG

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
    call    print_nibble            		; display upper nibble
    pop     ax
    call    print_nibble            		; display lower nibble
    ror     eax, 8
    loop    .loop

%if UNUSED
	mov     al, 10							; carriage return
	call    print_char
	mov     al, 13
	call    print_char
%endif ; UNUSED

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

;--------------------------------------------------------------------------
; getc - wait for a key press
;
getc:
    pushad
    mov     ah, 0
    int		0x16
    popad
    ret

;--------------------------------------------------------------------------
; Write a ASCII character to the console.
;
; Arguments:
;   AL = ASCII character.
;
print_char:
    pushad
    mov     bx, 1                   		; BH=0, BL=1 (blue)
    mov     ah, 0x0e                		; bios INT 10, Function 0xE
    int     0x10                    		; display byte in tty mode
    popad
    ret

%endif ; DEBUG

%if UNUSED

;--------------------------------------------------------------------------
; Convert null terminated string to HFSUniStr255
;
; Arguments:
;   DS:DX   pointer to a NULL terminated string.
;   ES:DI   pointer to result.
;
ConvertStrToUni:
    pushad									; save registers
    push	di								; save DI for unicode string length pointer
    mov		si, dx							; use SI as source string pointer
    xor		ax, ax							; AX = unicode character
    mov		cl, al							; CL = string length

.loop:
    stosw									; store unicode character (length 0 at first run)
    lodsb									; load next character to AL
    inc		cl								; increment string length count
    cmp		al, NULL						; check for string terminator
    jne		.loop
    
    pop		di								; restore unicode string length pointer
    dec		cl								; ignoring terminator from length count
    mov		[di], cl						; save string length
    popad									; restore registers
    ret

%endif ; UNUSED

;--------------------------------------------------------------------------
; Convert big-endian HFSUniStr255 to little-endian
;
; Arguments:
;   DS:SI = pointer to big-endian HFSUniStr255
;	ES:DI = pointer to result buffer
;
ConvertHFSUniStr255ToLE:
	pushad
	lodsw
	xchg	ah, al
	stosw
	cmp		al, 0
	je		.exit
	mov		cx, ax

.loop:
	lodsw
	xchg	ah, al							; convert AX to little-endian

	;
	; When working with a case-sensitive HFS+ (HX) filesystem, we shouldn't change the case.
	;
	cmp		BYTE [kHFSPlusBuffer + HFSPlusVolumeHeader.signature + 1], kHFSPlusCaseSigX
	je		.keepcase

	or		ax, ax
	jne		.convertToLE
	dec		ax								; NULL must be the strongest char

.convertToLE:
	cmp		ah, 0
	ja		.keepcase
	cmp		al, 'A'
	jb		.keepcase
	cmp		al, 'Z'
	ja		.keepcase
	add		al, 32							; convert to lower-case

.keepcase:
	stosw
	loop	.loop

.exit:
	popad
	ret

;--------------------------------------------------------------------------
; compare HFSPlusExtentKey structures
;
; Arguments:
;   DS:SI = search key
;   ES:DI = trial key
;
; Returns:
;	[BTree.searchResult] = result
;	FLAGS = relation between search and trial keys
;
compareHFSPlusExtentKeys:
	pushad
	
	mov		dl, 0							; DL = result of comparison, DH = bestGuess
	mov		eax, [si + HFSPlusExtentKey.fileID]
	cmp		eax, [di + HFSPlusExtentKey.fileID]
	jne		.checkFlags
	
	cmp		BYTE [si + HFSPlusExtentKey.forkType], kForkTypeData
	jne		.checkFlags
	
	mov		eax, [si + HFSPlusExtentKey.startBlock]
	cmp		eax, [di + HFSPlusExtentKey.startBlock]
	je		compareHFSPlusCatalogKeys.exit

.checkFlags:
	ja		compareHFSPlusCatalogKeys.searchKeyGreater		; search key > trial key
	jb		compareHFSPlusCatalogKeys.trialKeyGreater		; search key < trial key

;--------------------------------------------------------------------------
; Compare HFSPlusCatalogKey structures
;
; Arguments:
;   DS:SI = search key
;   ES:DI = trial key
;
; Returns:
;	[BTree.searchResult] = result
;	FLAGS = relation between search and trial keys
;
compareHFSPlusCatalogKeys:
	pushad
	xor		dx, dx					; DL = result of comparison, DH = bestGuess
	xchg	si, di
	lodsd
	mov		ecx, eax				; ECX = trial parentID
	xchg	si, di
	lodsd							; EAX = search parentID
	cmp		eax, ecx
	ja		.searchKeyGreater		; search parentID > trial parentID
	jb		.trialKeyGreater		; search parentID < trial parentID

.compareNodeName:					; search parentID = trial parentID
	xchg	si, di
    lodsw
    mov		cx, ax					; CX = trial nodeName.length
	xchg	si, di
    lodsw							; AX = search nodeName.length
	cmp		cl, 0					; trial nodeName.length = 0?
	je		.searchKeyGreater

    cmp		ax, cx
	je		.strCompare
    ja		.searchStrLonger

.trialStrLonger:
    dec		dh
    mov		cx, ax
    jmp		.strCompare

.searchStrLonger:
    inc		dh

.strCompare:
    repe	cmpsw
    ja		.searchKeyGreater
    jb		.trialKeyGreater
    mov		dl, dh
    jmp		.exit

.trialKeyGreater:
    dec		dl
    jmp		.exit
    
.searchKeyGreater:
    inc		dl
	
.exit:
	mov		[bp + BTree.searchResult], dl
    cmp		dl, 0							; set flags to check relation between keys

	popad
	ret

;--------------------------------------------------------------------------
; Allocate memory
;
; Arguments:
;   CX = size of requested memory
;
; Returns:
;   BP = start address of allocated memory
;
; Clobber list:
;   CX
;
malloc:
	push	ax								; save AX
	push	di								; save DI
	mov		di, [gMallocPtr]				; start address of free space
	push	di								; save free space start address
	inc		di								;
	inc		di								; keep the first word untouched
	dec		cx								; for the last memory block pointer.
	dec		cx								;
	mov		al, NULL						; fill with zero
	rep		stosb							; repeat fill
	mov		[gMallocPtr], di				; adjust free space pointer
	pop		bp								; BP = start address of allocated memory
	mov		[di], bp						; set start address of allocated memory at next
											; allocation block's free space address.
	pop		di								; restore DI
	pop		ax								; restore AX
	ret

%if UNUSED

;--------------------------------------------------------------------------
; Free allocated memory
;
; Returns:
;   BP = start address of previously allocated memory
;
free:
	lea		bp, [gMallocPtr]
	mov		bp, [bp]
	mov		[gMallocPtr], bp
	ret

%endif ; UNUSED

;--------------------------------------------------------------------------
; Static data.
;

%if VERBOSE
bootfile_msg        db		'/boot', CR, LF, NULL
%endif

;--------------------------------------------------------------------------
; Pad the rest of the 512 byte sized sector with zeroes. The last
; two bytes is the mandatory boot sector signature.
;
; If the booter code becomes too large, then nasm will complain
; that the 'times' argument is negative.

pad_table_and_sig:
	times			510-($-$$) db 0
	dw				kBootSignature

;
; Sector 1 code area
;

;--------------------------------------------------------------------------
; lookUpBTree - initializes a new BTree instance and
;               look up for HFSPlus Catalog File or Extent Overflow keys
;
; Arguments:
;   AL = kHFSPlusFileID (Catalog or Extents Overflow)
;	SI = address of searchKey
;   DI = address of HFSPlusForkData.extents
;
; Returns:
;   BP = address of BTree instance
;  ECX = rootNode's logical offset in sectors
;
lookUpBTree:
	mov		cx, BTree_size							; allocate memory with BTree_size
	call	malloc									; BP = start address of allocated memory.
	mov		[bp + BTree.fileID], al					; save fileFileID
	mov		edx, [di]								; first extent of current file
	call	blockToSector							; ECX = converted to sector unit
	mov		al, 1									; 1 sector is enough for
	xor		edx, edx								; reading current file's header.
	lea		dx, [bp + BTree.BTHeaderBuffer]			; load into BTreeHeaderBuffer
	call	readLBA									; read
	mov		ax, [bp + BTree.BTHeaderBuffer + BTNodeDescriptor_size + BTHeaderRec.nodeSize]
	xchg	ah, al									; convert to little-endian
	mov		[bp + BTree.nodeSize], ax				; save nodeSize

	;
	; Always start the lookup process with the root node.
	;
	mov		edx, [bp + BTree.BTHeaderBuffer + BTNodeDescriptor_size + BTHeaderRec.rootNode]

.readNode:
	;
	; Converting nodeID to sector unit
	;
	mov		ax, [bp + BTree.nodeSize]
	shr		ax, 9									; convert nodeSize to sectors
	mov		bx, ax									; BX = read sector count
	cwde
	bswap	edx										; convert node ID to little-endian
	mul		edx										; multiply with nodeSize converted to sector unit
	mov		ecx, eax								; ECX = file offset in BTree

	mov		eax, [bp + BTree.fileID]
	lea		edx, [bp + BTree.nodeBuffer]
	call	readExtent

	;
	; AX = lowerBound = 0
	;
	xor		ax, ax

	;
	; BX = upperBound = numRecords - 1
	;
	mov		bx, [bp + BTree.nodeBuffer + BTNodeDescriptor.numRecords]
	xchg	bh, bl
	dec		bx
	
.bsearch:
	cmp		ax, bx
	ja		.checkResult							; jump if lowerBound > upperBound
	
	mov		cx, ax
	add		cx, bx
	shr		cx, 1									; test index = (lowerBound + upperBound / 2)

	call	getBTreeRecord

%if UNUSED
	pushad
	jl		.csearchLessThanTrial
	jg		.csearchGreaterThanTrial
	PrintChar('=')
	jmp		.csearchCont
.csearchGreaterThanTrial:
	PrintChar('>')
	jmp		.csearchCont
.csearchLessThanTrial:
	PrintChar('<')
.csearchCont:
	popad
%endif ; UNUSED
	
.adjustBounds:
	je		.checkResult
	jl		.searchLessThanTrial
	jg		.searchGreaterThanTrial
	jmp		.bsearch	

.searchLessThanTrial:
	mov		bx, cx
	dec		bx										; upperBound = index - 1
	jmp		.bsearch

.searchGreaterThanTrial:
	mov		ax, cx
	inc		ax										; lowerBound = index + 1
	jmp		.bsearch
	
.checkResult:
	cmp		BYTE [bp + BTree.searchResult], 0
	jge		.foundKey

	mov		cx, bx
	call	getBTreeRecord

.foundKey:
	cmp		BYTE [bp + BTree.nodeBuffer + BTNodeDescriptor.kind], kBTIndexNode
	jne		.exit

	lea		bx, [bp + BTree.recordDataPtr]
	mov		bx, [bx]
	mov		edx, [bx]
	jmp		.readNode
	
.exit:
	cmp		BYTE [bp + BTree.searchResult], 0
	ret

;--------------------------------------------------------------------------
; getBTreeRecord - read and compare BTree record
;
; Arguments:
;   CX = record index
;   SI = address of search key
;
; Returns:
;   [BTree.searchResult] = result of key compare
;   [BTree.recordDataPtr] = address of record data
;
getBTreeRecord:
	pushad
	push	si									; save SI
	lea		di, [bp + BTree.nodeBuffer]			; DI = start of nodeBuffer
	push	di									; use later
	mov		ax, [bp + BTree.nodeSize]			; get nodeSize
	add		di, ax								; DI = beyond nodeBuffer
	inc		cx									; increment index
	shl		cx, 1								; * 2
	sub		di, cx								; DI = pointer to record
	mov		ax, [di]							; offset to record
	xchg	ah, al								; convert to little-endian
	pop		di									; start of nodeBuffer
	add		di, ax								; DI = address of record key
	mov		si, di								; save to SI
	mov		ax, [di]							; keyLength
	xchg	ah, al								; convert to little-endian
	inc		ax									; suppress keySize (2 bytes)
	inc		ax									;
	add		di, ax								; DI = address of record data
	mov		[bp + BTree.recordDataPtr], di		; save address of record data
	lea		di, [bp + BTree.trialKey]
	push	di									; save address of trialKey
	lodsw										; suppress keySize (2 bytes)
	;
	; Don't need to compare as DWORD since all reserved CNIDs fits to a single byte
	;
	cmp		BYTE [bp + BTree.fileID], kHFSCatalogFileID
	je		.prepareTrialCatalogKey

.prepareTrialExtentKey:
	mov		bx, compareHFSPlusExtentKeys
	movsw										; copy forkType + pad
	mov		cx, 2								; copy fileID + startBlock

.extentLoop:
	lodsd
	bswap	eax									; convert to little-endian
	stosd
	loop	.extentLoop
	jmp		.exit

.prepareTrialCatalogKey:
	mov		bx, compareHFSPlusCatalogKeys
	lodsd
	bswap	eax									; convert ParentID to little-endian
	stosd
	call	ConvertHFSUniStr255ToLE				; convert nodeName to little-endian

.exit:
	pop		di									; restore address of trialKey

%if UNUSED	
;
; Print catalog trial key
;
	pushad
	mov		si, di
	lodsd
	PrintChar('k')
	PrintHex()
	lodsw
	cmp		ax, 0
	je		.printExit
	mov		cx, ax
.printLoop:
	lodsw
	call	print_char
	loop	.printLoop  
.printExit:
	popad
;
;
;
%endif ; UNUSED
	
%if UNUSED	
;
; Print extent trial key
;
	pushad
	PrintChar('k')
	mov		si, di
	xor		eax, eax
	lodsw
	PrintHex()
	lodsd
	PrintHex()
	lodsd
	PrintHex()
	popad
;
;
;
%endif ; UNUSED

	pop		si									; restore SI
	call	bx									; call key compare proc
	popad
	ret 

;--------------------------------------------------------------------------
; readExtent - read extents from a HFS+ file (multiple extent support)
;
; Arguments:
;  EAX = Catalog File ID
;  	BX = read size in sectors
;  ECX = file offset in sectors
;  EDX = address of read buffer
;   DI = address of HFSPlusForkData.extents
;
readExtent:
	pushad
	;
	; Save Catalog File ID as part of a search HFSPlusExtentKey
	; for a possible Extents Overflow lookup.
	;
	mov		[bp + BTree.searchExtentKey + HFSPlusExtentKey.fileID], eax
	mov		[bp + BTree.readBufferPtr], edx
	mov		ax, bx
	cwde
	mov		[bp + BTree.readSize], eax
	mov		ebx, ecx							; EBX = file offset
	xor		eax, eax
	mov		[bp + BTree.currentExtentOffs], eax

.beginExtentBlock:
	mov		BYTE [bp + BTree.extentCount], 0

.extentSearch:
	cmp		BYTE [bp + BTree.extentCount], kHFSPlusExtentDensity
	jb		.continue

.getNextExtentBlock:
	push	ebx
	mov		eax, [bp + BTree.currentExtentOffs]

	;
	; Converting sector unit to HFS+ allocation block unit.
	;
	xor		edx, edx
	div		DWORD [gBlockSize]				; divide with blockSize

	;
	; Preparing searchExtentKey's startBlock field.
	;
	mov		[bp + BTree.searchExtentKey + HFSPlusExtentKey.startBlock], eax

	mov		al, kHFSExtentsFileID
	lea		si, [bp + BTree.searchExtentKey]
	lea		di, [kHFSPlusBuffer + HFSPlusVolumeHeader.extentsFile + HFSPlusForkData.extents]
	call	lookUpBTree
	jnz		NEAR .exit

	;
	; BP points to the new workspace allocated by lookUpBTree.
	;
	lea		di, [bp + BTree.recordDataPtr]
	mov		di, [di]

	;
	; Switch back to the previous workspace.
	;
	lea		bp, [gMallocPtr]
	mov		bp, [bp]
	mov		[gMallocPtr], bp

	pop		ebx
	jmp		.beginExtentBlock
	
.continue:
	mov		edx, [di + HFSPlusExtentDescriptor.blockCount]
	call	blockToSector								; ECX = converted current extent's blockCount to sectors
	mov		eax, [bp + BTree.currentExtentOffs]			; EAX = current extent's start offset (sector)
	mov		edx, eax
	add		edx, ecx									; EDX = next extent's start offset (sector)
	cmp		ebx, edx
	mov		[bp + BTree.currentExtentOffs], edx			; set currentExtentOffs as the next extent's start offset
	jae		.nextExtent									; jump to next extent if file offset > next extent's start offset

.foundExtent:
	mov		edx, ebx
	sub		edx, eax									; EDX = relative offset within current extent
	mov		eax, edx									; will be used below to determine read size
	mov		esi, [bp + BTree.readSize]					; ESI = remaining sectors to be read
	add		edx, esi
	cmp		edx, ecx									; test if relative offset + readSize fits to this extent
	jbe		.read										; read all remaining sectors from this extent

.splitRead:
	sub		ecx, eax									; read amount of sectors beginning at relative offset
	mov		esi, ecx									; of current extent up to the end of current extent

.read:
	mov		edx, [di + HFSPlusExtentDescriptor.startBlock]
	call	blockToSector								; ECX = converted to sectors
	add		ecx, eax									; file offset converted to sectors
	
	push	si
	mov		ax, si
	mov		edx, [bp + BTree.readBufferPtr]
	call	readSectors
	pop		si
	
	add		ebx, esi
	mov		ax, si
	cwde
	shl		ax, 9										; convert SI (read sector count) to byte unit
	add		[bp + BTree.readBufferPtr], eax
	sub		[bp + BTree.readSize], esi
	
	jz		.exit

.nextExtent:
	add		di, kHFSPlusExtentDensity
	inc		BYTE [bp + BTree.extentCount]
	jmp		.extentSearch

.exit:
	popad
	ret

;--------------------------------------------------------------------------
; Convert big-endian HFSPlus allocation block to sector unit
;
; Arguments:
;   EDX = allocation block
;
; Returns:
;   ECX = allocation block converted to sector unit
;
; Clobber list:
;   EDX
;
blockToSector:
	push	eax
	mov		eax, [gBlockSize]
	bswap	edx								; convert allocation block to little-endian
	mul		edx				 				; multiply with block number
	mov		ecx, eax						; result in EAX
	pop		eax
	ret

%if UNUSED

;--------------------------------------------------------------------------
; Convert sector unit to HFSPlus allocation block unit
;
; Arguments:
;   EDX = sector
;
; Returns:
;   ECX = converted to allocation block unit
;
; Clobber list:
;   EDX
;
sectorToBlock:
	push	eax
	mov		eax, edx
	xor		edx, edx
	div		DWORD [gBlockSize]				; divide with blockSize
	mov		ecx, eax						; result in EAX
	pop		eax
	ret

%endif ; UNUSED

%if UNUSED

;--------------------------------------------------------------------------
; Convert big-endian BTree node ID to sector unit
;
; Arguments:
;   EDX = node ID
;
; Returns:
;   ECX = node ID converted to sector unit
;
; Clobber list:
;   EDX
;
nodeToSector:
	push	eax
	mov		ax, [bp + BTree.nodeSize]
	shr		ax, 9							; convert nodeSize to sectors
	cwde
	bswap	edx								; convert node ID to little-endian
	mul		edx								; multiply with node ID
	mov		ecx, eax						; result in EAX
	pop		eax
	ret

%endif ; UNUSED

;--------------------------------------------------------------------------
; Static data.
;

%if VERBOSE
log_title_str		db		'boot1: ', NULL
error_str			db		'error', NULL
%endif

searchCatalogKey	dd		kHFSRootFolderID
					dw		searchCatKeyNameLen
searchCatKeyName	dw		'b', 'o', 'o', 't'			; must be lower case
searchCatKeyNameLen	EQU		($ - searchCatKeyName) / 2

;--------------------------------------------------------------------------
; Pad the rest of the 512 byte sized sector with zeroes. The last
; two bytes is the mandatory boot sector signature.
;
pad_sector_1:
	times			1022-($-$$) db 0
	dw				kBootSignature

;
; Local BTree variables
;
					struc			BTree
.mallocLink			resw			1						; pointer to previously allocated memory block
.fileID				resd			1						; will use as BYTE
.nodeSize			resd			1						; will use as WORD
.searchExtentKey	resb			HFSPlusExtentKey_size
.searchResult		resb			1
.trialKey			resb			kBTMaxRecordLength
.recordDataPtr		resw			1
.readBufferPtr		resd			1
.currentExtentOffs	resd			1
.readSize			resd			1
.extentCount		resb			1
	ALIGNB			2
.BTHeaderBuffer		resb			kSectorBytes
.nodeBuffer			resb			maxNodeSize
					endstruc

;
; Global variables
;

	ABSOLUTE		kHFSPlusBuffer + HFSPlusVolumeHeader_size

gPartLBA			resd	1
gBIOSDriveNumber	resw	1
gBlockSize			resd	1
gMallocPtr			resw	1

; END
