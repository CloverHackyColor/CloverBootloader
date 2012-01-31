# 
# BootSectors for Clover EFI project
#
# Duet EDKII HDD Sectors FAT32 : Mbr.bin / Gpt.bin 
#			       : bd32.bin / bd32_64.bin
#
# Bootloader sectors "Efildr20" - 32bit : start32.com - efi32.com3
#                               - 64bit : St32_64.com - efi64.com2
#
# Clover EDKII HDD Sectors : boot0  / boot0md
#		    	   : boot1h / boot1f32
# 		   	   
# Bootloader sectors "boot" - 32bit : start32H.com2 - efi32.com3
#                           - 64bit : St32_64.com   - efi64.com2
#
#
# Source Used: - Chameleon Bootloader Project 
#		  boot0 / boot0md - boot1h / boot1f32
#
#	       - BootDuet - Miguel Lopes Santos Ramos
#		  bd32.bin / bd32_64.bin
#
#              - BootSectors - DuetPkg EDKII
#		  start32.com - efi32.com2  
#		  St32_64.com - efi64.com2
#		  start32.com - efi32.com3
#		  St32_64.com - efi64.com2
#
# Chameleon ; forge.voodooprojects.org/p/chameleon/
# BootDuet ;  https://github.com/migle/BootDuet
# DuetPkg ;   http://tianocore.sourceforge.net/
# 
# GCC crosstool is used for building on OS X
# 

ASM=/opt/local/ia32/gcc
DLINK=/opt/local/ia32/ld
BINDIR=../bin/
NASM=nasm

CFLAGS_BOOTDUET = -nostdlib -Wl,--oformat=binary -Wl,-Ttext=0x7c00
all : boot0 boot0md boot1f32 boot1h start32H.com2 efi32.com3 bd32.bin bd32_64.bin start32.com St32_64.com efi64.com2


# Clover EDKII HDD Sectors

boot0 : boot0.s
	$(NASM) boot0.s -o $(BINDIR)$@

boot0md : boot0md.s
	$(NASM) boot0md.s -o $(BINDIR)$@

boot1f32 : boot1f32.s
	$(NASM) boot1f32.s -o $(BINDIR)$@

boot1h : boot1.s
	$(NASM) boot1.s -o $(BINDIR)$@


# Bootloader sectors "boot" 32bit

start32H.com2 : start32H.S
	$(ASM) -c -o start32H.o start32H.S
	$(DLINK) --oformat binary -o start32H.com start32H.o -Ttext 0 -Map start32.map
	dd if=start32H.com of=$(BINDIR)$@ bs=512 skip=1
	rm -r start32.map start32H.o start32H.com

efi32.com3 : efi32.S
	$(ASM) -c -o efi32.o efi32.S
	$(DLINK) --oformat binary -o efi32.com efi32.o -Ttext 0 -Map efi32.map
	rm -r efi32.map efi32.o
	dd if=efi32.com of=$(BINDIR)$@ bs=512 skip=264
	rm -r efi32.com

# Bootloader sectors "boot" 64bit
# For now only Efildr20 works, but I have some idea...


# Duet EDKII HDD Sectors

bd32.bin : BootDuet.S
	$(ASM) -DFAT=32 -DWITH_VALIDATION $(CFLAGS_BOOTDUET) -o ../bin/$@ $< -Wa,-a=$(@:.bin=.lst) -Wl,-Map=$(@:.bin=.map)
	rm -r bd32.lst bd32.map

bd32_64.bin : BootDuet.S
	$(ASM) -DFAT=32 -DWITH_LBA_64BIT $(CFLAGS_BOOTDUET) -o ../bin/$@ $< -Wa,-a=$(@:.bin=.lst) -Wl,-Map=$(@:.bin=.map)
	rm -r bd32_64.lst bd32_64.map


# Bootloader sectors "Efildr20" 32bit

start32.com : start32.S
	$(ASM) -c -o start32.o start32.S
	$(DLINK) --oformat binary -o $(BINDIR)$@ start32.o -Ttext 0 -Map start32.map


# Bootloader sectors "Efildr20" 64bit

St32_64.com : st32_64.S
	$(ASM) -c -o st32_64.o st32_64.S
	$(DLINK) --oformat binary -o $(BINDIR)$@ st32_64.o -Ttext 0 -Map st32_64.map
	rm -r st32_64.map st32_64.o

efi64.com2 : efi64.S
	$(ASM) -c -o efi64.o efi64.S
	$(DLINK) --oformat binary -o efi64.com efi64.o -Ttext 0 -Map efi64.map
	dd if=efi64.com of=$(BINDIR)$@ bs=512 skip=264
	rm -r efi64.o efi64.map efi64.com
clean:
	rm $(BINDIR)boot0 $(BINDIR)boot0md