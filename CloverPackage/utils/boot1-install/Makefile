SRCROOT = $(abspath $(CURDIR)/..)
OBJROOT = $(abspath $(CURDIR)/../../sym/boot1-install)

DIR = boot1-install

include ${SRCROOT}/Make.rules


LDFLAGS := $(LDFALGS) -mmacosx-version-min=10.5 \
-framework CoreFoundation \
-framework DiskArbitration \
-Wl,-no_source_version \
-Wl,-no_function_starts \
-Wl,-no_data_in_code_info \
-Wl,-no_version_load_command \
-Wl,-no_uuid \
-Wl,-no_dependent_dr_info



OBJS =  boot1-install.o32 boot1-install.o64

OBJS := $(addprefix $(OBJROOT)/, $(OBJS))

PROGRAM = boot1-install
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

install-local: $(PROGRAM)
	@sudo install -d -g 0 -o 0 /usr/local/bin /usr/local/man/man8
	@sudo install -psv -g 0 -o 0 $(PROGRAM) /usr/local/bin
	@sudo install -pv  -g 0 -o 0 boot1-install.8 /usr/local/man/man8

clean-local:
	@$(RM) -f $(PROGRAM) $(OBJS)
