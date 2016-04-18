ifeq ($(shell uname),Linux)
AS=/usr/bin/as
LD=/usr/bin/ld
else
ifndef TOOLCHAIN_DIR
TOOLCHAIN_DIR=$(HOME)/src/opt/local
endif
AS=$(TOOLCHAIN_DIR)/cross/bin/x86_64-clover-linux-gnu-as
LD=$(TOOLCHAIN_DIR)/cross/bin/x86_64-clover-linux-gnu-ld
endif
RM=/bin/rm
PRODUCTS=bin/Start64H5.com bin/Start64H6.com

all: $(PRODUCTS)

clean:
	$(RM) -f $(PRODUCTS)

%.com: %.obj
	$(LD) --oformat=binary -Ttext=0x200 -o $@ $<
	$(RM) -f $<

bin/Start64H5.obj: st32_64H.S
	$(AS) -o $@ $<

bin/Start64H6.obj: st32_64H.S
	$(AS) --defsym CHARACTER_TO_SHOW=0x58 -o $@ $<
