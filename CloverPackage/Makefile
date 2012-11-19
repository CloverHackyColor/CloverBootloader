#	Makefile for kernel booter
SRCROOT = $(CURDIR)
SYMROOT = $(SRCROOT)/sym
CLOVERROOT = $(SRCROOT)/CloverV2
PKG_BUILD_DIR = $(SYMROOT)/package
IMGROOT = $(SYMROOT)/CloverCD
IMGSKELROOT = $(SYMROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot
CDBOOT32 = ${IMGROOT}/usr/standalone/i386/cdboot32
EFIROOT = $(SRCROOT)/CloverV2/EFI

REVISION = `cat $(SRCROOT)/revision`
PRODUCT = Clover-$(REVISION)
CDLABEL = ${PRODUCT}-X64
CDLABEL32 = ${PRODUCT}-IA32
ISOIMAGE = ${SYMROOT}/${CDLABEL}.iso
ISOIMAGE32 = ${SYMROOT}/${CDLABEL32}.iso

THEME = metal
EXCLUDE = --exclude=.svn --exclude=.DS_Store --exclude=sym --exclude=obj \
		--exclude=package --exclude=archive --exclude=User_Guide_src --exclude=*.sh

$(SRCROOT)/revision:
	@svnversion -n | tr -d [:alpha:] > $(SRCROOT)/revision

#all: $(SYMROOT) $(OBJROOT) $(CONFIG_HEADERS) $(HEADER_VERSION) $(SRCROOT)/revision
#	@$(MAKE) all

cdboot:
	@echo "making cdboot"
	@cp -f $(CLOVERROOT)/BootSectors/cdboot $(SYMROOT)/i386/
	@dd if=$(CLOVERROOT)/Bootloaders/x64/boot of=$(SYMROOT)/i386/cdboot conv=sync bs=2k seek=1 &> /dev/null

	@echo Update cdboot with boot file size info
	@stat -f%z $(CLOVERROOT)/Bootloaders/x64/boot \
		| perl -ane "print pack('V',@F[0]);" \
		| dd of=$(SYMROOT)/i386/cdboot bs=1 count=4 seek=2044 conv=notrunc &> /dev/null

cdboot32:
	@echo "making cdboot-32"
	@cp -f $(CLOVERROOT)/BootSectors/cdboot $(SYMROOT)/i386/cdboot32
	@dd if=$(CLOVERROOT)/Bootloaders/ia32/boot of=$(SYMROOT)/i386/cdboot32 conv=sync bs=2k seek=1 &> /dev/null

	@echo Update cdboot with boot file size info
	@stat -f%z $(CLOVERROOT)/Bootloaders/ia32/boot \
		| perl -ane "print pack('V',@F[0]);" \
		| dd of=$(SYMROOT)/i386/cdboot32 bs=1 count=4 seek=2044 conv=notrunc &> /dev/null

image: 
	@echo "================= Distrib ================="
	@echo "\t[RM] ${IMGROOT}"
	@rm -rf ${IMGROOT}	
	@echo "\t[MKDIR] ${IMGROOT}/usr/standalone/i386"			  	  
	@mkdir -p ${IMGROOT}/usr/standalone/i386
	@mkdir -p ${IMGROOT}/usr/standalone/i386/x64
	@mkdir -p ${IMGROOT}/usr/standalone/i386/ia32
	@echo "\t[MKDIR] ${IMGROOT}/EFI"
	@mkdir -p ${IMGROOT}/EFI				
	@echo "\t[MKDIR] ${IMGROOT}/EFI/BOOT/themes/"
	@mkdir -p ${IMGROOT}/EFI/BOOT/themes/				
	@echo "\t[MKDIR] ${IMGROOT}/usr/bin"
	@mkdir -p ${IMGROOT}/usr/bin
	@if [ -e "$(IMGSKELROOT)" ]; then				\
		@echo "\t[CP] ${IMGROOTSKEL} ${IMGROOT}"		\
		@cp -R -f "${IMGSKELROOT}"/* "${IMGROOT}";		\
	fi;	
	@echo "\t[CP] cdboot"						  
	@cp -f ${SYMROOT}/i386/cdboot ${CDBOOT}
	@cp -f ${SYMROOT}/i386/cdboot32 ${CDBOOT32}
	@echo "\t[CP] metal"
	@cp -r -f $(CLOVERROOT)/themespkg/metal ${IMGROOT}/EFI/BOOT/themes/
	@echo "\t[CP] CLOVERX64"
	@cp -f $(CLOVERROOT)/EFI/BOOT/CLOVERX64.efi ${IMGROOT}/EFI/BOOT/
	@echo "\t[CP] CLOVERIA32"
	@cp -f $(CLOVERROOT)/EFI/BOOT/CLOVERIA32.efi ${IMGROOT}/EFI/BOOT/
#	@echo "CP BOOTX64"
	@cp -f $(CLOVERROOT)/EFI/BOOT/BOOTX64.efi ${IMGROOT}/EFI/BOOT/
#	@echo "CP font"
#	@cp -r -f $(CLOVERROOT)/EFI/BOOT/font ${IMGROOT}/EFI/BOOT/
#	@echo "CP refit.conf"
	@cp -f $(CLOVERROOT)/EFI/BOOT/refit.conf ${IMGROOT}/EFI/BOOT/
#	@echo "CP config.plist"
	@cp -f $(CLOVERROOT)/EFI/config-org.plist ${IMGROOT}/EFI/config.plist
	@echo "\t[CP] drivers64"
	@cp -r -f $(CLOVERROOT)/EFI/drivers64 ${IMGROOT}/EFI/
	@cp -r -f $(CLOVERROOT)/EFI/drivers64UEFI ${IMGROOT}/EFI/
	@cp -r -f $(CLOVERROOT)/EFI/tools ${IMGROOT}/EFI/
#	@echo "ADD drivers64"
	@cp -f $(CLOVERROOT)/drivers-Off/drivers64/UsbMouseDxe-64.efi ${IMGROOT}/EFI/drivers64/
	@cp -f $(CLOVERROOT)/drivers-Off/drivers64/PS2MouseDxe-64.efi ${IMGROOT}/EFI/drivers64/
	@echo "\t[CP] drivers32"
	@cp -r -f $(CLOVERROOT)/EFI/drivers32 ${IMGROOT}/EFI/
#	@echo "ADD drivers32"
	@cp -f $(CLOVERROOT)/drivers-Off/drivers32/UsbMouseDxe-32.efi ${IMGROOT}/EFI/drivers32/
	@cp -f $(CLOVERROOT)/drivers-Off/drivers32/PS2MouseDxe-32.efi ${IMGROOT}/EFI/drivers32/

	@echo "\t[CP] boot"
	@cp -f $(CLOVERROOT)/Bootloaders/x64/boot ${IMGROOT}/usr/standalone/i386/x64/
#	@cp -f $(CLOVERROOT)/Bootloaders/x64/boot6 ${IMGROOT}/usr/standalone/i386/x64/
	@cp -f $(CLOVERROOT)/Bootloaders/ia32/boot ${IMGROOT}/usr/standalone/i386/ia32/
#	@cp -f $(CLOVERROOT)/Bootloaders/ia32/boot3 ${IMGROOT}/usr/standalone/i386/ia32/
	@cp -f ${SYMROOT}/i386/boot0 ${IMGROOT}/usr/standalone/i386
	@cp -f ${SYMROOT}/i386/boot0hfs ${IMGROOT}/usr/standalone/i386
	@cp -f ${SYMROOT}/i386/boot0md ${IMGROOT}/usr/standalone/i386
	@cp -f ${SYMROOT}/i386/boot1h2 ${IMGROOT}/usr/standalone/i386
	@cp -f ${SYMROOT}/i386/boot1f32alt ${IMGROOT}/usr/standalone/i386
#ifdef CONFIG_FDISK440
	@cp -f ${SYMROOT}/i386/fdisk440 ${IMGROOT}/usr/bin
#endif
	@rm -rf ${IMGROOT}/.s*
	@rm -rf ${IMGROOT}/*/.s*
	@rm -rf ${IMGROOT}/*/*/.s*
	@rm -rf ${IMGROOT}/*/*/*/.s*
	@rm -rf ${IMGROOT}/*/*/*/*/.s*
	@echo "\t[HDIUTIL] ${ISOIMAGE}"
	@mkdir -p ${SYMROOT}/CloverISO
	@hdiutil makehybrid -iso -joliet -hfs -hfs-volume-name \
		${CDLABEL} -eltorito-boot ${CDBOOT} -no-emul-boot -ov -o   \
		"${ISOIMAGE}" ${IMGROOT} -quiet 
	@echo "\t[LZMA] ${ISOIMAGE}"
#	@lzma e ${ISOIMAGE} ${ISOIMAGE}.lzma
	@mv -f ${ISOIMAGE} ${SYMROOT}/CloverISO/
	@echo "\t[HDIUTIL] ${ISOIMAGE32}"
	@hdiutil makehybrid -iso -joliet -hfs -hfs-volume-name \
		${CDLABEL32} -eltorito-boot ${CDBOOT32} -no-emul-boot -ov -o   \
		"${ISOIMAGE32}" ${IMGROOT} -quiet 
	@echo "\t[LZMA] ${ISOIMAGE32}"
#	@lzma e ${ISOIMAGE32} ${ISOIMAGE32}.lzma
	@mv -f ${ISOIMAGE32} ${SYMROOT}/CloverISO/
		  	  
#	@echo "\t[GZ] ${DISTFILE}.tgz"
#	@rm -f ${DISTFILE}.tar.gz
#	@cd ${SYMROOT} && tar -cf ${DISTFILE}.tar ${DISTROOT}
#	@gzip --best ${DISTFILE}.tar
#	@mv ${DISTFILE}.tar.gz ${DISTFILE}.tgz

pkg installer:
	@echo " "
	@echo "===================== Building Package ================="
	@${SRCROOT}/package/buildpkg.sh "$(SRCROOT)" "$(SYMROOT)" "$(PKG_BUILD_DIR)"

clean:
	rm -rf sym obj dst

.PHONY: clean
.PHONY: cdboot
.PHONY: image
.PHONY: pkg
.PHONY: installer
