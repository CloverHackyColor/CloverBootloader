include ../Make.defaults

CDIR=$(TOPDIR)/..

all:

clean:

install:
	mkdir -p $(INSTALLROOT)/include/efi
	mkdir -p $(INSTALLROOT)/include/efi/protocol
	mkdir -p $(INSTALLROOT)/include/efi/$(ARCH)
	$(INSTALL) -m 644 *.h $(INSTALLROOT)/include/efi
	$(INSTALL) -m 644 protocol/*.h $(INSTALLROOT)/include/efi/protocol
	$(INSTALL) -m 644 $(ARCH)/*.h $(INSTALLROOT)/include/efi/$(ARCH)
ifeq ($(ARCH),ia64)
	mkdir -p $(INSTALLROOT)/include/efi/protocol/ia64
	$(INSTALL) -m 644 protocol/ia64/*.h $(INSTALLROOT)/include/efi/protocol/ia64
endif

include ../Make.rules
