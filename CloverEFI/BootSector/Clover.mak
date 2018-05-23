ifeq ($(shell uname),Linux)
CC=/usr/bin/gcc
LD=/usr/bin/ld
else
ifndef TOOLCHAIN_DIR
TOOLCHAIN_DIR=$(HOME)/src/opt/local
endif
CC=$(TOOLCHAIN_DIR)/cross/bin/x86_64-clover-linux-gnu-gcc
LD=$(TOOLCHAIN_DIR)/cross/bin/x86_64-clover-linux-gnu-ld
endif
RM=/bin/rm
DD=/bin/dd
CHMOD=/bin/chmod

BINDIR=bin
PRODUCTS32=$(BINDIR)/start32H.com2 $(BINDIR)/efi32.com3
PRODUCTS64=$(BINDIR)/Start64H.com $(BINDIR)/Start64H2.com \
  $(BINDIR)/Start64H3.com $(BINDIR)/Start64H4.com \
  $(BINDIR)/Start64H5.com $(BINDIR)/Start64H6.com bin/efi64.com3

all: $(PRODUCTS32) $(PRODUCTS64)

clean:
	$(RM) -f $(PRODUCTS32) $(PRODUCTS64)

$(BINDIR)/start32H.com2: start32H.S
	$(CC) -c -o $(basename $@).o $<
	$(LD) --oformat binary -Ttext=0 -o $(basename $@).tmp $(basename $@).o
	$(DD) if=$(basename $@).tmp of=$@ bs=512 skip=1
	$(RM) -f $(basename $@).o $(basename $@).tmp

$(BINDIR)/efi32.com3: efi32.S
	$(CC) -c -o $(basename $@).o $<
	$(LD) --oformat binary -Ttext=0 -o $(basename $@).tmp $(basename $@).o
	$(DD) if=$(basename $@).tmp of=$@ bs=4096 skip=33
	$(RM) -f $(basename $@).o $(basename $@).tmp

$(BINDIR)/Start64H.com: st32_64.S
	$(CC) -c -Wa,--defsym,CHARACTER_TO_SHOW=0x36 -o $(basename $@).o $<
	$(LD) --oformat=binary -Ttext=0 -o $@ $(basename $@).o
	$(CHMOD) -x $@
	$(RM) -f $(basename $@).o

$(BINDIR)/Start64H2.com: st32_64.S
	$(CC) -c -Wa,--defsym,CHARACTER_TO_SHOW=0x42 -o $(basename $@).o $<
	$(LD) --oformat=binary -Ttext=0 -o $@ $(basename $@).o
	$(CHMOD) -x $@
	$(RM) -f $(basename $@).o

$(BINDIR)/Start64H3.com: st32_64.S
	$(CC) -c -Wa,--defsym,CHARACTER_TO_SHOW=0x35,--defsym,USE_LOW_EBDA=1 -o $(basename $@).o $<
	$(LD) --oformat=binary -Ttext=0 -o $@ $(basename $@).o
	$(CHMOD) -x $@
	$(RM) -f $(basename $@).o

$(BINDIR)/Start64H4.com: st32_64.S
	$(CC) -c -Wa,--defsym,CHARACTER_TO_SHOW=0x4C,--defsym,USE_LOW_EBDA=1 -o $(basename $@).o $<
	$(LD) --oformat=binary -Ttext=0 -o $@ $(basename $@).o
	$(CHMOD) -x $@
	$(RM) -f $(basename $@).o

$(BINDIR)/Start64H5.com: st32_64H.S
	$(CC) -c -o $(basename $@).o $<
	$(LD) --oformat=binary -Ttext=0x200 -o $@ $(basename $@).o
	$(CHMOD) -x $@
	$(RM) -f $(basename $@).o

$(BINDIR)/Start64H6.com: st32_64H.S
	$(CC) -c -Wa,--defsym,CHARACTER_TO_SHOW=0x58 -o $(basename $@).o $<
	$(LD) --oformat=binary -Ttext=0x200 -o $@ $(basename $@).o
	$(CHMOD) -x $@
	$(RM) -f $(basename $@).o

$(BINDIR)/efi64.com3: efi64.S
	$(CC) -c -Wa,--32 -o $(basename $@).o $<
	$(LD) -m elf_i386 --oformat binary -Ttext=0 -o $(basename $@).tmp $(basename $@).o
	$(DD) if=$(basename $@).tmp of=$@ bs=4096 skip=33
	$(RM) -f $(basename $@).o $(basename $@).tmp
