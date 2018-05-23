ifeq ($(shell uname),Linux)
NASM=/usr/bin/nasm
else
ifndef TOOLCHAIN_DIR
TOOLCHAIN_DIR=$(HOME)/src/opt/local
endif
NASM=$(TOOLCHAIN_DIR)/bin/nasm
endif
RM=/bin/rm

BINDIR=bin
PRODUCTS32=$(BINDIR)/start32H.com2 $(BINDIR)/efi32.com3
PRODUCTS64=$(BINDIR)/Start64H.com $(BINDIR)/Start64H2.com \
  $(BINDIR)/Start64H3.com $(BINDIR)/Start64H4.com \
  $(BINDIR)/Start64H5.com $(BINDIR)/Start64H6.com bin/efi64.com3

all: $(PRODUCTS32) $(PRODUCTS64)

clean:
	$(RM) -f $(PRODUCTS32) $(PRODUCTS64)

$(BINDIR)/start32H.com2: start.nasm
	$(NASM) -f bin -o $@ $<

$(BINDIR)/efi32.com3: efi32.nasm
	$(NASM) -f bin -o $@ $<

$(BINDIR)/Start64H.com: start.nasm
	$(NASM) -DX64 -DCHARACTER_TO_SHOW=0x36 -f bin -o $@ $<

$(BINDIR)/Start64H2.com: start.nasm
	$(NASM) -DX64 -DCHARACTER_TO_SHOW=0x42 -f bin -o $@ $<

$(BINDIR)/Start64H3.com: start.nasm
	$(NASM) -DX64 -DCHARACTER_TO_SHOW=0x35 -DUSE_LOW_EBDA -f bin -o $@ $<

$(BINDIR)/Start64H4.com: start.nasm
	$(NASM) -DX64 -DCHARACTER_TO_SHOW=0x4C -DUSE_LOW_EBDA -f bin -o $@ $<

$(BINDIR)/Start64H5.com: start.nasm
	$(NASM) -DX64 -DCHARACTER_TO_SHOW=0x54 -DGENPAGE -f bin -o $@ $<

$(BINDIR)/Start64H6.com: start.nasm
	$(NASM) -DX64 -DCHARACTER_TO_SHOW=0x58 -DGENPAGE -f bin -o $@ $<

$(BINDIR)/efi64.com3: efi64.nasm
	$(NASM) -f bin -o $@ $<
