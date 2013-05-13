XCODEBUILD_OPTIONS=-project 'CloverPrefpane.xcodeproj' CONFIGURATION_BUILD_DIR=$(CURDIR)/build 
# -scheme 'Clover' DEPLOYMENT_LOCATION=NO

CloverPrefpane:
	@echo "Building CloverPrefpane preference..."
	/usr/bin/xcodebuild $(XCODEBUILD_OPTIONS) #archive

install:
	@$(CURDIR)/install.sh

clean:
	@/usr/bin/xcodebuild $(XCODEBUILD_OPTIONS) clean
	@rm -rf build *~

.PHONY: CloverPrefpane install clean
