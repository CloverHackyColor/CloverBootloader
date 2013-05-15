
XCODE_VERSION_GE_4 := $(shell expr `xcodebuild -version | sed -nE 's/^Xcode ([0-9]).*/\1/p'` \>= 4)

XCODEBUILD_OPTIONS=-project 'CloverPrefpane.xcodeproj' CONFIGURATION_BUILD_DIR=$(CURDIR)/build DEPLOYMENT_LOCATION=NO

ifeq "$(XCODE_VERSION_GE_4)" "1"
	XCODEBUILD_OPTIONS += -scheme 'Clover'
	BUILD_ACTION=archive
else
	XCODEBUILD_OPTIONS += -configuration 'Release'
endif

CloverPrefpane:
	@echo "Building CloverPrefpane preference..."
	@/usr/bin/xcodebuild $(XCODEBUILD_OPTIONS) $(BUILD_ACTION)

install:
	@$(CURDIR)/install.sh

clean:
	@/usr/bin/xcodebuild $(XCODEBUILD_OPTIONS) clean
	@rm -rf build *~

.PHONY: CloverPrefpane install clean
