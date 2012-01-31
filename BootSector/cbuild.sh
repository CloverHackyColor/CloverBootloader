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
# Chameleon ; http://forge.voodooprojects.org/p/chameleon/
# BootDuet ;  https://github.com/migle/BootDuet
# DuetPkg ;   http://tianocore.sourceforge.net/
# 
# GCC crosstool is used for building on OS X
# 

cd src
make