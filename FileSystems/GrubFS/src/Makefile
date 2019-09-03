# meta-Makefile for the filesystem drivers
#

INSTALL_DIR     = /boot/efi/EFI/refind/drivers_x64
FILESYSTEMS     = affs bfs btrfs exfat ext2 hfs hfsplus iso9660 jfs nilfs2 ntfs reiserfs sfs udf ufs2 xfs zfs
HOSTARCH        = $(shell uname -m | sed s,i[3456789]86,ia32,)

ifeq ($(HOSTARCH),ia64)
  DRIVERARCH    = ia64
endif
ifeq ($(HOSTARCH),ia32)
  DRIVERARCH    = ia32
endif
ifeq ($(HOSTARCH),x86_64)
  DRIVERARCH    = x64
endif

all:	$(FILESYSTEMS)

affs:
	@rm -f grub_driver.o
	+make DRIVERNAME=$@ FSDIR=fs -f Make.gnuefi

bfs:
	@rm -f grub_driver.o
	+make DRIVERNAME=$@ FSDIR=fs -f Make.gnuefi

btrfs:
	@rm -f grub_driver.o
	+make DRIVERNAME=$@ FSDIR=fs EXTRAMODULE=gzio EXTRAMODULEDIR=io -f Make.gnuefi

ext2:
	@rm -f grub_driver.o
	+make DRIVERNAME=$@ FSDIR=fs -f Make.gnuefi

exfat:
	@rm -f grub_driver.o
	+make DRIVERNAME=$@ FSDIR=fs -f Make.gnuefi

hfs:
	@rm -f grub_driver.o
	+make DRIVERNAME=$@ FSDIR=fs -f Make.gnuefi

hfsplus:
	@rm -f grub_driver.o
	+make DRIVERNAME=$@ FSDIR=fs EXTRAMODULE=$@comp EXTRAMODULEDIR=fs EXTRAMODULE2=gzio EXTRAMODULE2DIR=io -f Make.gnuefi

iso9660:
	@rm -f grub_driver.o
	+make DRIVERNAME=$@ FSDIR=fs -f Make.gnuefi

jfs:
	@rm -f grub_driver.o
	+make DRIVERNAME=$@ FSDIR=fs -f Make.gnuefi

nilfs2:
	@rm -f grub_driver.o
	+make DRIVERNAME=$@ FSDIR=fs -f Make.gnuefi

ntfs:
	@rm -f grub_driver.o
	+make DRIVERNAME=$@ FSDIR=fs EXTRAMODULE=$@comp EXTRAMODULEDIR=fs -f Make.gnuefi

reiserfs:
	@rm -f grub_driver.o
	+make DRIVERNAME=$@ FSDIR=fs -f Make.gnuefi

sfs:
	@rm -f grub_driver.o
	+make DRIVERNAME=$@ FSDIR=fs -f Make.gnuefi

udf:
	@rm -f grub_driver.o
	+make DRIVERNAME=$@ FSDIR=fs -f Make.gnuefi

ufs2:
	@rm -f grub_driver.o
	+make DRIVERNAME=$@ FSDIR=fs -f Make.gnuefi

xfs:
	@rm -f grub_driver.o
	+make DRIVERNAME=$@ FSDIR=fs -f Make.gnuefi

zfs:
	@rm -f grub_driver.o
	+make DRIVERNAME=$@ FSDIR=fs/zfs EXTRAMODULE=gzio EXTRAMODULEDIR=io EXTRAOBJS="zfs_fletcher.o zfs_lz4.o zfs_lzjb.o zfs_sha256.o" -f Make.gnuefi

install: all
	@mkdir -p $(INSTALL_DIR)
	@cp -v $(FILESYSTEMS:%=%_$(DRIVERARCH).efi) $(INSTALL_DIR)

clean:
	+make -f Make.gnuefi clean
