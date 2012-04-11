#	Makefile for kernel booter
SRCROOT = $(CURDIR)
SYMROOT = $(SRCROOT)/sym
PKG_BUILD_DIR = $(SYMROOT)/package

pkg installer: 
	@echo "================= Building Package ================="
	@${SRCROOT}/package/buildpkg.sh "$(SRCROOT)" "$(SYMROOT)" "$(PKG_BUILD_DIR)"

.PHONY: pkg
.PHONY: installer
