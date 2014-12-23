SRCROOT = $(abspath $(CURDIR)/..)
OBJROOT = $(abspath $(CURDIR)/../../sym/utils)

DIR = fdisk440

include ${SRCROOT}/Make.rules


LDFLAGS := $(LDFALGS) -mmacosx-version-min=10.5


OBJS =  cmd.o32 disk.o32 fdisk.o32 getrawpartition.o32 mbr.o32 misc.o32 opendev.o32 part.o32 user.o32 auto.o32 \
		cmd.o64 disk.o64 fdisk.o64 getrawpartition.o64 mbr.o64 misc.o64 opendev.o64 part.o64 user.o64 auto.o64

OBJS := $(addprefix $(OBJROOT)/, $(OBJS))

PROGRAM = fdisk440
PROGRAM:= $(addprefix $(OBJROOT)/, $(PROGRAM))

DIRS_NEEDED = $(OBJROOT)

all: $(PROGRAM)

$(PROGRAM): $(DIRS_NEEDED) $(OBJS)
	@echo "\t[LD32] $(@F)_32"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -arch i386 -o $@_32 $(filter %.o32,$^)
	@echo "\t[LD64] $(@F)_64"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -arch x86_64 -o $@_64 $(filter %.o64,$^)
	@echo "\t[LIPO] $@"
	@lipo -create -arch i386 $@_32 -arch x86_64 $@_64 -output $@
	@strip $@
	@rm $@_32 $@_64
	@rm -f $(OBJROOT)/*.o32*
	@rm -f $(OBJROOT)/*.o64*


install-local: $(PROGRAM)
	@sudo install -d -g 0 -o 0 /usr/local/bin /usr/local/man/man8
	@sudo install -psv -g 0 -o 0 $(PROGRAM) /usr/local/bin
	@sudo install -pv  -g 0 -o 0 fdisk440.8 /usr/local/man/man8

clean-local:
	@$(RM) -f $(PROGRAM) $(OBJS)
