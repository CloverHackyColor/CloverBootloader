SRCROOT = $(abspath $(CURDIR)/..)
OBJROOT = $(abspath $(CURDIR)/../../sym/utils)

DIR = boot1-install

include ${SRCROOT}/Make.rules

XCODE_VERSION_GE_4 := $(shell expr `xcodebuild -version | sed -nE 's/^Xcode ([0-9]).*/\1/p'` \>= 4)

XCODEBUILD_OPTIONS = OBJROOT=$(OBJROOT)  DEPLOYMENT_LOCATION=NO CONFIGURATION_BUILD_DIR=$(OBJROOT)

ifeq "$(XCODE_VERSION_GE_4)" "1"
	XCODEBUILD_OPTIONS += -scheme 'boot1-install'
	BUILD_ACTION=archive
else
	XCODEBUILD_OPTIONS += -configuration 'Release'
endif

DIRS_NEEDED := $(OBJROOT)
PROGRAMS := boot1-install
PROG := $(addprefix $(OBJROOT)/, $(PROGRAMS))

all: $(PROG)

$(PROG):
	@/usr/bin/xcodebuild $(XCODEBUILD_OPTIONS) $(BUILD_ACTION) >/dev/null
	@rm -rf $(OBJROOT)/*.build
	@rm -rf $(OBJROOT)/*.dSYM

	@echo [XCODE] $(PROGRAMS)

install-local: $(PROG)
	@sudo install -d -g 0 -o 0 /usr/local/bin /usr/local/man/man8
	@sudo install -psv -g 0 -o 0 $(PROGRAM) /usr/local/bin
	@sudo install -pv  -g 0 -o 0 boot1-install.8 /usr/local/man/man8

clean-local:
	@/usr/bin/xcodebuild $(XCODEBUILD_OPTIONS) clean  >/dev/null
	@rm -rf build *~
	@echo [CLEAN-LOCAL] $(PROGRAMS)

clean:
	@/usr/bin/xcodebuild $(XCODEBUILD_OPTIONS) clean  >/dev/null
	@rm -rf build *~
	@echo [CLEAN] $(PROGRAMS)

