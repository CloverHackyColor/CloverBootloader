SRCROOT = $(abspath $(CURDIR)/..)
OBJROOT = $(abspath $(CURDIR)/../../sym/bdmesg)

include ${SRCROOT}/Make.rules

PROGRAMS := bdmesg

LDFLAGS := $(LDFLAGS) -framework IOKit -framework CoreFoundation -mmacosx-version-min=10.5

OBJS := $(addprefix $(OBJROOT)/, $(addsuffix .o32, $(PROGRAMS)) $(addsuffix .o64, $(PROGRAMS)))
PROG := $(addprefix $(OBJROOT)/, $(PROGRAMS))

DIRS_NEEDED = $(OBJROOT)

all: $(PROG)

$(PROG): $(DIRS_NEEDED) $(OBJS)
	@echo "\t[LD32] $(@F)_32"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -arch i386 -o $(OBJROOT)/$(@F)_32 $(OBJROOT)/$(@F).o32
	@echo "\t[LD64] $(@F)_64"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -arch x86_64 -o $(OBJROOT)/$(@F)_64 $(OBJROOT)/$(@F).o64
	@echo "\t[LIPO] $(@F)"
	@lipo -create -arch i386 $(OBJROOT)/$(@F)_32 -arch x86_64 $(OBJROOT)/$(@F)_64 -output $@
	@$(RM) $(OBJROOT)/$(@F)_32 $(OBJROOT)/$(@F)_64

install-local: $(PROG)
	@sudo install -d -g 0 -o 0 /usr/local/bin
	@sudo install -psv -g 0 -o 0 $(PROG) /usr/local/bin

clean-local:
	@$(RM) -f $(PROG) $(OBJS)
