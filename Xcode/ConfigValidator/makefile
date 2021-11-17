
XCODE_VERSION_GE_4 := $(shell expr `xcodebuild -version | sed -nE 's/^Xcode ([0-9]+).*/\1/p'` \>= 4)
XCODE_VERSION_GE_10 := $(shell expr `xcodebuild -version | sed -nE 's/^Xcode ([0-9]+).*/\1/p'` \>= 10)

XCODEBUILD_OPTIONS=-project 'ConfigValidator.xcodeproj' CONFIGURATION_BUILD_DIR=$(CURDIR)/build DEPLOYMENT_LOCATION=NO

ifeq "$(XCODE_VERSION_GE_10)" "1"
	XCODEBUILD_OPTIONS += ARCHS=x86_64 VALID_ARCHS=x86_64 ONLY_ACTIVE_ARCH=YES
endif

ifeq "$(XCODE_VERSION_GE_4)" "1"
	XCODEBUILD_OPTIONS += -scheme 'ConfigValidator'
#	BUILD_ACTION=running
else
	XCODEBUILD_OPTIONS += -configuration 'Release'
endif

ConfigValidator:
	@echo "Building ConfigValidator..."
	@echo "ConfiValidator uses latest release of CloverConfigPlistValidator"
	@/usr/bin/xcodebuild $(XCODEBUILD_OPTIONS) $(BUILD_ACTION)  >/dev/null
	@echo [XCODE] $(PROGRAMS)

.PHONY: ConfigValidator clean
