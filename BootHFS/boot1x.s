;
; Copyright (c) 2014-2015 Zenith432 All rights reserved.
;
; Partition Boot Loader: boot1x
; This version of boot1x tries to find a stage2 boot file in the root folder.
;
; Credits:
;   Portions based on boot1f32.
;   Thanks to Robert Shullich for
;     "Reverse Engineering the Microsoft exFAT File System" dated Dec 1, 2009.
;   T13 Commitee document EDD-4 for information about BIOS int 0x13.
;
; This program is designed to reside in blocks 0 - 1 of an exFAT partition.
; It expects that the MBR has left the drive number in DL.
; 
; This version requires a BIOS with EBIOS (LBA) support.
;
; This code is written for the NASM assembler.
;   nasm -f bin -o boot1x boot1x.s
;
; Written by zenith432 during November 2014.
; Modified by zenith432 January 2015
;   Added Feature: Support digit keypress with two second delay (-DSELECTION_FEATURE)
;
	bits 16

%define VERBOSE 1
%define USESIDL 1
%define USEBP 1
kMaxBlockCount equ 127			; Max block count supported by Int 0x13, function 0x42, old school
kBootBlockBytes equ 512			; Bytes in a exFAT Boot block
kBootSignature equ 0xaa55		; Boot block signature
kBoot1StackAddress equ 0xfff0		; Address of top-of-stack 0:0xfff0
kBoot1LoadAddr equ 0x7c00		; Address of loaded boot block 0:0x7c00
kBoot2Segment equ 0x2000		; Address for boot2 0x2000:0x200
kBoot2Address equ 512
kFATBuf equ 0x6c00			; Address for FAT block buffer 0:0x6c00 (4K space)
kRootDirBuf equ 0x5c00			; Address for Root Directory block buffer 0:0x5c00 (4K space)
kMaxCluster equ 0xfffffff7		; exFAT max cluster value + 1 (for FAT32 it's 0x0ffffff8)
kMaxContigClusters equ 1024		; Max contiguous clusters returned by getRange
kBootNameHash equ 0xdc36		; exFAT name hash for 'BOOT' (in UTF16LE)
kBoot2MaxBytes equ (512 * 1024 - 512)	; must fit between 0x20200 and 0xa0000

	struc PartitionEntry	; MBR partition entry (truncated)
	times 8 resb 1
.lba:	resd 1			; starting lba
	endstruc

	struc BootParams	; BOOT file parameters
.cluster:	resd 1		; 1st cluster of BOOT
.size:		resd 1		; size of BOOT in bytes
		resw 1
.flag:		resb 1
	endstruc

	struc DirIterator	; exFAT Directory Iterator
.entries_end:	resb 1		; beyond last 32-byte entry (possible values 16, 32, 64, 128)
.cluster:	resd 1		; current cluster
.lba_high:	resd 1		; upper 32 bits of lba
.lba_end:	resd 1		; beyond last block (lower 32-bits)
.lba:		resd 1		; current block
.entry:		resb 1		; current 32-byte entry
	endstruc

	struc FATCache		; Manages cache state for FAT blocks
.shift:	resb 1			; right shift for converting cluster # to FAT block address
.mask:	resw 1			; bit mask for finding cluster # in FAT block
.lba:	resd 1			; lba # cached in FAT block buffer (note that FAT block address is limited to 32 bits)
	endstruc

%ifdef USEBP
%define BPR bp - gPartitionOffset +
%else
%define BPR
%endif

	section .text
	org kBoot1LoadAddr
	jmp start
	times (3 - $ + $$) nop
gOEMName: times 8 db 0		; 'EXFAT   '

;
; Scratch Area
; Used for data structures
;
	times (64 - BootParams_size - DirIterator_size - FATCache_size - $ + $$) db 0
gsParams:   times BootParams_size db 0
gsIterator: times DirIterator_size db 0
gsFATCache: times FATCache_size db 0

;
; exFAT BPB
;
gPartitionOffset: dd 0, 0
gVolumeLength: dd 0, 0
gFATOffset: dd 0
gFATLength: dd 0
gClusterHeapOffset: dd 0
gClusterCount: dd 0
gRootDirectory1stCluster: dd 0
gVolumeSerialNubmer: dd 0
gFileSystemRevision: dw 0	; 0x100
gVolumeFlags: dw 0
gBytesPerBlock: db 0		; range 9 - 12 (power of 2)
gBlocksPerCluster: db 0		; gBytesPerBlock + gBlocksPerCluster <= 25 (power of 2)
gNumberOfFATs: db 0		; should be 1
gDriveSelect: db 0		; probably 0x80
gPercentInUse: db 0
	times 7 db 0
start:
	cli
	xor eax, eax
	mov ss, ax
	mov sp, kBoot1StackAddress
	sti
	mov ds, ax
	mov es, ax

	;
	; Initializing global variables.
	;
%ifdef USEBP
	mov bp, gPartitionOffset
%endif
%ifdef USESIDL
	;
	; Shouldn't be necessary to use DS:SI because
	; 1) Existing gPartitionOffset must be correct in
	;    order for filesystem to work well when mounted.
	; 2) LBA may be 64 bits if booted from GPT.
	; 3) Not all MBR boot records pass DS:SI
	;    pointing to MBR partition entry.
	;
%if 0
	mov ecx, [si + PartitionEntry.lba]
	mov [BPR gPartitionOffset + 4], eax
	mov [BPR gPartitionOffset], ecx
%endif
	;
	; However, by convention BIOS passes boot
	;   drive number in dl, so use that instead
	;   of existing gDriveSelect
	;
	mov [BPR gDriveSelect], dl
%endif

	;
	; Initialize FAT Cache
	;
	dec eax
	mov dword [BPR gsFATCache + FATCache.lba], eax	; alternatively store gFATLength here
	mov cl, [BPR gBytesPerBlock]
	sub cl, 2	; range 7 - 10
	mov [BPR gsFATCache + FATCache.shift], cl
	neg ax
	shl ax, cl
	dec ax
	mov [BPR gsFATCache + FATCache.mask], ax

	;
	; Initialize Iterator
	;
	mov al, 1
	sub cl, 3	; range 4 - 7
	shl al, cl
	mov [BPR gsIterator + DirIterator.entries_end], al
	mov [BPR gsIterator + DirIterator.entry], al
	xor eax, eax
	mov ecx, [BPR gRootDirectory1stCluster]
	mov [BPR gsIterator + DirIterator.lba_end], eax
	mov [BPR gsIterator + DirIterator.lba], eax
	mov [BPR gsIterator + DirIterator.cluster], ecx

%ifdef VERBOSE
	mov di, init_str
	call log_string
%endif

%ifdef SELECTION_FEATURE
	call setBootFile
%endif
	;
	; Search root directory for BOOT
	;
.loop:
	call nextDirEntry
	jc error
	cld
	lodsb
.revert:
	test al, al	; end of root directory?
	jz error
	cmp al, 0x85	; file/subdir entry?
	jnz .loop
	lodsb
	cmp al, 2	; 2ndary count should be 2
	jb .loop
	add si, 2	; skip checksum
	lodsb
	test al, 0x10	; file attributes - check not a directory
	jnz .loop
	call nextDirEntry
	jc error
	cld
	lodsb
	cmp al, 0xc0	; stream extension entry?
	jnz .revert
	lodsb
	mov dl, al	; General 2ndary flag
	inc si
	lodsb
.name_length_point:
	cmp al, 4	; name length
	jnz .loop
	lodsw		; name hash
.name_hash_point:
	cmp ax, kBootNameHash
	jnz .loop
	add si, 2
	mov eax, [si + 4] ; high 32 bits of valid data length
	test eax, eax
	jz .more
	and dl, 0xfe	; if size too big, mark as no allocation
.more:
	lodsd		; valid data length
	mov [BPR gsParams + BootParams.size], eax
	add si, 8
	lodsd		; first cluster
	mov [BPR gsParams + BootParams.cluster], eax
	mov [BPR gsParams + BootParams.flag], dl
	call nextDirEntry
	jc error
	cld
	lodsb
	cmp al, 0xc1
	jnz .revert
	inc si		; skip flags
	lodsd			; unicode chars 1 - 2
	or eax, 0x200020	; tolower
	cmp eax, 0x6f0062	; 'bo' in UTF16LE
	jnz .loop
	lodsd			; unicode chars 3 - 4
	or eax, 0x200020	; tolower
	cmp eax, 0x74006f	; 'ot' in UTF16LE
	jnz .loop
	;
	; done - found boot file!
	;
	mov dl, [BPR gsParams + BootParams.flag]
	test dl, 1	; no allocation or length too big?
	jz error
	mov ebx, [BPR gsParams + BootParams.size]
	cmp ebx, kBoot2MaxBytes + 1
	jnb error
	call BytesToBlocks	; convert size to blocks
	; boot2 file size in blocks is in bx
load_boot2:		; anchor for localizing next labels
	xor esi, esi	; no blocks after 1st range
	test dl, 2	; FAT Chain?
	cmovnz edx, [BPR gsParams + BootParams.cluster]	; if not
	jnz .oneshot	; load contiguous file
	;
	; load via FAT
	;
	mov si, bx	; total blocks to si
.loop:
	mov eax, [BPR gsParams + BootParams.cluster]
	mov edx, eax
	call getRange
	test ebx, ebx
	jnz .nonempty
	test si, si
	jnz error
	jmp boot2
.nonempty:
	cmp ebx, esi
	cmovnb bx, si
	sub si, bx
	mov [BPR gsParams + BootParams.cluster], eax
.oneshot:
	call ClusterToLBA
	mov ax, bx
	mov ecx, edx
	mov edx, (kBoot2Segment << 4) | kBoot2Address
	call readBlocks
	; TODO: error
	test si, si
	jnz .loop
	; fall through to boot2
boot2:
	mov dl, [BPR gDriveSelect]	; load BIOS drive number
	jmp kBoot2Segment:kBoot2Address

error:
%ifdef VERBOSE
	mov di, error_str
	call log_string
%endif

hang:
	hlt
	jmp hang

;--------------------------------------------------------------------------
; ClusterToLBA - Converts cluster number to 64-bit LBA
;
; Arguments:
;    EDX = cluster number
;
; Returns
;    EDI:EDX = corresponding block address
;
; Assumes input cluster number is valid
;
ClusterToLBA:
	push cx
	xor edi, edi
	sub edx, 2
	mov cl, [BPR gBlocksPerCluster]
	shld edi, edx, cl
	shl edx, cl
	add edx, [BPR gClusterHeapOffset]
	adc edi, 0
	pop cx
	ret

;--------------------------------------------------------------------------
; BytesToBlocks - Converts byte size to blocks (rounding up to next block)
;
; Arguments:
;    EBX = size in bytes
;
; Returns:
;    EBX = size in blocks (rounded up)
;
; Clobbers eax, cl
;
BytesToBlocks:
	xor eax, eax
	inc ax
	mov cl, [BPR gBytesPerBlock]
	shl ax, cl
	dec ax
	add ebx, eax
	shr ebx, cl
	ret

	times (kBootBlockBytes - 2 - $ + $$) nop
	dw kBootSignature
block1_end:

;--------------------------------------------------------------------------
; nextDirEntry - Locates the next 32-byte entry in Root Directory,
;    loading block if necessary.
;
; Returns:
;    CF set if end of Root Directory
;    CF clear, and DS:SI points to next entry if exists
;
; Clobbers eax, ebx, ecx, edx, edi
;
nextDirEntry:
	movzx ax, [BPR gsIterator + DirIterator.entry]
	cmp al, [BPR gsIterator + DirIterator.entries_end]
	jb .addressentry
	mov ecx, [BPR gsIterator + DirIterator.lba]
	mov edi, [BPR gsIterator + DirIterator.lba_high]
	cmp ecx, [BPR gsIterator + DirIterator.lba_end]
	jnz .readblock
	mov eax, [BPR gsIterator + DirIterator.cluster]
	mov edx, eax
	call getRange
	test ebx, ebx
	jnz .nonempty
	stc
	ret
.nonempty:
	mov [BPR gsIterator + DirIterator.cluster], eax
	call ClusterToLBA
	mov ecx, edx
	add edx, ebx
	mov [BPR gsIterator + DirIterator.lba_high], edi
	mov [BPR gsIterator + DirIterator.lba_end], edx
.readblock:
	mov al, 1
%if 0
	mov edx, kRootDirBuf
%else
	xor edx, edx
	mov dh, kRootDirBuf >> 8
%endif
	call readLBA
	; TODO error
	inc ecx
	jnz .skip
	inc edi
	mov [BPR gsIterator + DirIterator.lba_high], edi
.skip:
	mov [BPR gsIterator + DirIterator.lba], ecx
	xor ax, ax
.addressentry:
	mov si, ax
	inc al
	mov [BPR gsIterator + DirIterator.entry], al
	shl si, 5
	add si, kRootDirBuf
	clc
	ret

;--------------------------------------------------------------------------
; getRange - Calculates contiguous range of clusters from FAT
;
; Arguments:
;    EAX = start cluster
;
; Returns:
;    EAX = next cluster after range
;    EBX = number of contiguous blocks in range
;
; Range calculated is at most kMaxContigClusters clusters long
;
getRange:
	push ecx
	push edx
	push edi
	push si
	xor edi, edi
%if 0
	mov edx, kFATBuf
%else
	mov edx, edi
	mov dh, kFATBuf >> 8
%endif
	mov ebx, edi
.loop:
	cmp eax, 2
	jb .finishup
	cmp eax, -9 ;kMaxCluster
	jnb .finishup
	cmp bx, kMaxContigClusters
	jnb .finishup
	inc bx
	mov si, ax
	and si, [BPR gsFATCache + FATCache.mask]
	shl si, 2
	mov ecx, eax
	inc ecx
	push ecx
	mov cl, [BPR gsFATCache + FATCache.shift]
	shr eax, cl
	cmp eax, [BPR gsFATCache + FATCache.lba]
	jz .iscached
	mov ecx, [BPR gFATOffset]
	add ecx, eax
	mov [BPR gsFATCache + FATCache.lba], eax
	mov al, 1
	call readLBA
	; TODO: error?
.iscached:
	pop ecx
	mov eax, [kFATBuf + si]
	cmp eax, ecx
	jz .loop

.finishup:
	mov cl, [BPR gBlocksPerCluster]
	shl ebx, cl
	pop si
	pop edi
	pop edx
	pop ecx
	ret

%ifdef SELECTION_FEATURE
;--------------------------------------------------------------------------
; setBootFile - Waits two seconds for a keypress.
;   If keypress is digit '0' - '9' alters boot file from /boot to /boot<digit>
;   If keypress anything else or no keypress - uses /boot.
;
; Arguments:
;    None
;
; Returns:
;    None
;
; Clobbers ax, cx, dx
;
setBootFile:
	mov cx, 2000	; loop counter = max 2000 miliseconds in total
.loop:
	mov ah, 1	; int 0x16, Func 0x01 - get keyboard status/preview key
	int 0x16
	jnz .keypress	; got keypress
	; wait for 1 ms: int 0x15, Func 0x86 (wait for cx:dx microseconds)
	push cx		; save loop counter
	xor cx, cx
	mov dx, 1000
	mov ah, 0x86
	int 0x15
	pop cx		; restore loop counter
	loop .loop
.done:
	ret
.keypress:
	xor ah, ah	; read the char from buffer to spend it
	int 0x16
	; have a key - ASCII is in al
	cmp al, '0'
	jb .done
	cmp al, '9' + 1
	jae .done
	;
	; Alter code so name length tested is 5 instead of 4
	; Compute new hash value with digit and alter code
	;   to check for modified hash value.
	; Note: code continues to compare 4 characters 'boot'.
	;   For any other ascii character in 5th position,
	;   the hash value does not collide.  There are
	;   non-ascii unicode characters in 5th position that
	;   collide with hash-value, but ignore those for simplicity.
	;
	xor ah, ah
	inc byte [start.name_length_point + 1]
	add ax, (kBootNameHash >> 1) | ((kBootNameHash & 1) << 15)
	ror ax, 1
	mov [start.name_hash_point + 1], ax
	ret
%endif

;--------------------------------------------------------------------------
; readBlocks - Reads more than kMaxBlockCount blocks using LBA addressing.
;
; Arguments:
;   AX = number of blocks to read (valid from 1-1280).
;   EDX = pointer to where the blocks should be stored.
;   EDI:ECX = block offset in partition (64 bits)
;
; Returns:
;   CF = 0 success
;        1 error
;
readBlocks:
	pushad
	mov bx, ax

.loop:
	xor eax, eax
	mov al, kMaxBlockCount
	cmp bx, ax
	cmovb ax, bx
	call readLBA
	; TODO: error?
	sub bx, ax
	jz .exit
	add ecx, eax
	adc edi, 0
	push cx
	mov cl, [BPR gBytesPerBlock]
	shl eax, cl
	pop cx
	add edx, eax
	jmp .loop

.exit:
	popad
	ret

;--------------------------------------------------------------------------
; readLBA - Read blocks from a partition using LBA addressing.
;
; Arguments:
;   AL = number of blocks to read (valid from 1-kMaxBlockCount).
;   EDX = pointer to where the blocks should be stored.
;   EDI:ECX = block offset in partition (64 bits)
;   [gDriveSelect] = drive number (0x80 + unit number)
;   [gPartitionOffset] = partition location on drive
;
; Returns:
;   CF = 0 success
;        1 error
;	Presently, jumps to error on BIOS-reported failure
;
readLBA:
	pushad                          		; save all registers
	push es						; save ES
	mov bp, sp                 	 		; save current SP

	;
	; Adjust to 16 bit segment:offset address
	;   to allow for reading up to 64K
	;
	mov bl, dl
	and bx, 0xf
	shr edx, 4
	mov es, dx

	;
	; Create the Disk Address Packet structure for the
	; INT13/F42 (Extended Read Sectors) on the stack.
	;

	add ecx, [gPartitionOffset]
	adc edi, [gPartitionOffset + 4]
	push edi
	push ecx
	push es
	push bx
	xor ah, ah
	push ax
	push word 16

	;
	; INT13 Func 42 - Extended Read Sectors
	;
	; Arguments:
	;   AH    = 0x42
	;   DL    = drive number (0x80 + unit number)
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
	mov dl, [gDriveSelect]		; load BIOS drive number
	mov si, sp
	mov ah, 0x42
	int 0x13

	jc error

	;
	; Issue a disk reset on error.
	; Should this be changed to Func 0xD to skip the diskette controller
	; reset?
	;
;	xor     ax, ax                  		; Func 0
;	int     0x13                    		; INT 13
;	stc                             		; set carry to indicate error

;.exit
 	mov sp, bp
 	pop es
	popad
	ret

%ifdef VERBOSE

;--------------------------------------------------------------------------
; Write a string with log_title_str prefix to the console.
;
; Arguments:
;   DS:DI   pointer to a NULL terminated string.
;
log_string:
	pushad
	push di
	mov si, log_title_str
	call print_string
	pop si
	call print_string
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
	mov bx, 1	; BH=0, BL=1 (blue)

.loop:
	lodsb		; load a byte from DS:SI into AL
	test al, al	; Is it a NULL?
	jz .exit	; yes, all done
	mov ah, 0xE	; INT10 Func 0xE
	int 0x10 	; display byte in tty mode
	jmp .loop

.exit:
	ret

%endif ; VERBOSE

;--------------------------------------------------------------------------
; Static data.
;

%ifdef VERBOSE
log_title_str:	db 13, 10, 'boot1x: ', 0
init_str:	db 'init', 0
error_str:	db 'error', 0
%endif

	times (kBootBlockBytes - 4 - $ + block1_end) db 0
	dw 0, kBootSignature
