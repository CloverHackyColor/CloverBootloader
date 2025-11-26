PROGRAM = Clover-GenConfig

SRCROOT := $(abspath $(CURDIR)/..)
SYMROOT := $(abspath $(CURDIR)/../../sym)
OBJROOT := $(SYMROOT)/build/$(PROGRAM)
INSTALL_DIR_NAME=utils
UTILSDIR= $(SYMROOT)/$(INSTALL_DIR_NAME)

include ${SRCROOT}/Make.rules

XCODE_VERSION_GE_4 := $(shell expr `xcodebuild -version | sed -nE 's/^Xcode ([0-9]+).*/\1/p'` \>= 4)
XCODE_VERSION_GE_10 := $(shell expr `xcodebuild -version | sed -nE 's/^Xcode ([0-9]+).*/\1/p'` \>= 10)

XCODEBUILD_OPTIONS = OBJROOT=$(OBJROOT) CONFIGURATION_BUILD_DIR=$(OBJROOT) DEPLOYMENT_LOCATION=YES DSTROOT=$(SYMROOT) INSTALL_PATH='/$(INSTALL_DIR_NAME)'

ifeq "$(XCODE_VERSION_GE_10)" "1"
	XCODEBUILD_OPTIONS += ARCHS=x86_64 VALID_ARCHS=x86_64 ONLY_ACTIVE_ARCH=YES
endif

ifeq "$(XCODE_VERSION_GE_4)" "1"
	XCODEBUILD_OPTIONS += -scheme 'Clover-GenConfig'
	BUILD_ACTION=install
endif
XCODEBUILD_OPTIONS += -configuration 'Release' -project 'Clover-GenConfig.xcodeproj'


SRCS := $(wildcard *.c)
PROG := $(addprefix $(UTILSDIR)/, $(PROGRAM))

all: $(PROG)

$(PROG): $(SRCS)
	@echo "Building Clover-GenConfig..."
	@$/chmod 755 Clover-GenConfig.sh
	@$/chmod 755 BuildCGC
	@$/./BuildCGC -v -r -T -f ./Clover-GenConfig.sh
	@$/rm -rf ./CloverGenConfig/Clover-GenConfig.c
	@$/Sleep 1
	@$/cp -rp ./Clover-GenConfig.sh.x.c ./CloverGenConfig/Clover-GenConfig.c
	@$/rm -rf Clover-GenConfig.sh.x
	@$/rm -rf Clover-GenConfig.sh.x.c
	@echo "\t[XCODE] $(PROGRAM)"
	@/usr/bin/xcodebuild $(XCODEBUILD_OPTIONS) $(BUILD_ACTION) >/dev/null

install-local: $(PROG)
	@sudo install -d -g 0 -o 0 /usr/local/bin
	@sudo install -psv -g 0 -o 0 $(PROG) /usr/local/bin

clean-local:
	@/usr/bin/xcodebuild $(XCODEBUILD_OPTIONS) clean  >/dev/null
	@rm -rf $(OBJROOT) *~

