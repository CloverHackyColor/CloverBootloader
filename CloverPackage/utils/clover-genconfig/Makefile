SRCROOT = $(abspath $(CURDIR)/..)
OBJROOT = $(abspath $(CURDIR)/../../sym/clover-genconfig)

include ${SRCROOT}/Make.rules

XCODE_VERSION_GE_4 := $(shell expr `xcodebuild -version | sed -nE 's/^Xcode ([0-9]).*/\1/p'` \>= 4)

XCODEBUILD_OPTIONS=-project 'clover-genconfig.xcodeproj' OBJROOT=$(OBJROOT) CONFIGURATION_BUILD_DIR=$(OBJROOT) DEPLOYMENT_LOCATION=NO

ifeq "$(XCODE_VERSION_GE_4)" "1"
	XCODEBUILD_OPTIONS += -scheme 'clover-genconfig'
	BUILD_ACTION=archive
else
	XCODEBUILD_OPTIONS += -configuration 'Release'
endif

DIRS_NEEDED := $(OBJROOT)
PROGRAMS := clover-genconfig
PROG := $(addprefix $(OBJROOT)/, $(PROGRAMS))

all: $(PROG)

$(PROG):
	@/usr/bin/xcodebuild $(XCODEBUILD_OPTIONS) $(BUILD_ACTION) >/dev/null
	@echo [XCODE] $(PROGRAMS)

install-local: $(PROG)
	@sudo install -d -g 0 -o 0 /usr/local/bin
	@sudo install -psv -g 0 -o 0 $(PROG) /usr/local/bin

clean-local:
	@/usr/bin/xcodebuild $(XCODEBUILD_OPTIONS) clean  >/dev/null
	@rm -rf build *~
	@echo [CLEAN-LOCAL] $(PROGRAMS)

clean:
	@/usr/bin/xcodebuild $(XCODEBUILD_OPTIONS) clean  >/dev/null
	@rm -rf build *~
	@echo [CLEAN] $(PROGRAMS)
