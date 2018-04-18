
PROGRAM = fdisk440

SRCROOT := $(abspath $(CURDIR)/..)
SYMROOT := $(abspath $(CURDIR)/../../sym)
OBJROOT := $(SYMROOT)/build/$(PROGRAM)

INSTALL_DIR_NAME := utils
UTILSDIR := $(SYMROOT)/$(INSTALL_DIR_NAME)

DIRS_NEEDED := $(OBJROOT)

include ${SRCROOT}/Make.rules

CFLAGS := $(CFLAGS) -mmacosx-version-min=10.7
LDFLAGS := $(LDFALGS) -mmacosx-version-min=10.7
CXXFLAGS := $(CXXFLAGS) -mmacosx-version-min=10.7

SRCS := $(wildcard *.c)

#OBJS = $(SRCS:.c=.o32) $(SRCS:.c=.o64)
OBJS = $(SRCS:.c=.o64)
OBJS := $(addprefix $(OBJROOT)/, $(OBJS))

PROGRAM := $(addprefix $(UTILSDIR)/, $(PROGRAM))

all: $(PROGRAM)

$(PROGRAM): $(DIRS_NEEDED) $(OBJS)
	@mkdir -p $(UTILSDIR)
#	@echo "\t[LD32] $(@F)_32"
#	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -arch i386 -o $@_32 $(filter %.o32,$^)
	@echo "\t[LD64] $(@F)_64"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -arch x86_64 -o $@_64 $(filter %.o64,$^)
#	@echo "\t[LIPO] $@"
#	@lipo -create -arch i386 $@_32 -arch x86_64 $@_64 -output $@
	@lipo -create -arch x86_64 $@_64 -output $@
	@strip $@
#	@rm $@_32 $@_64
	@rm $@_64

install-local: $(PROGRAM)
	@sudo install -d -g 0 -o 0 /usr/local/bin /usr/local/man/man8
	@sudo install -psv -g 0 -o 0 $(PROGRAM) /usr/local/bin
	@sudo install -pv  -g 0 -o 0 fdisk440.8 /usr/local/man/man8

clean-local:
	@rm -f $(PROGRAM)
	@rm -rf $(OBJROOT) *~
