
XCODE_VERSION_GE_11 := $(shell expr `xcodebuild -version | sed -nE 's/^Xcode ([0-9]+).*/\1/p'` \>= 11)

XCODEBUILD_OPTIONS=-project 'Clover.xcodeproj' CONFIGURATION_BUILD_DIR=$(CURDIR)/build DEPLOYMENT_LOCATION=NO
ifeq "$(XCODE_VERSION_GE_11)" "1"
	XCODEBUILD_OPTIONS += ARCHS=x86_64 VALID_ARCHS=x86_64 ONLY_ACTIVE_ARCH=YES
	XCODEBUILD_OPTIONS += -configuration 'Release'
	XCODEBUILD_OPTIONS += -scheme 'Clover'
else
	$(error Xcode 11 or greater and swift 5 or greater are requested)
endif

CloverApp:
	@echo "Building Clover.app..."

	@/usr/bin/xcodebuild $(XCODEBUILD_OPTIONS) $(BUILD_ACTION)  >/dev/null
	@echo [XCODE] $(PROGRAMS)

clean:
	@/usr/bin/xcodebuild $(XCODEBUILD_OPTIONS) clean  >/dev/null
	@rm -rf build *~
	@echo [CLEAN] $(PROGRAMS)

.PHONY: CloverApp clean
