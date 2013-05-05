
CloverPrefpane:
	@echo "Building CloverPrefpane preference..."
	@/usr/bin/xcodebuild -project 'CloverPrefpane.xcodeproj' \
		-configuration 'Release'  CONFIGURATION_BUILD_DIR=$(CURDIR)/build

install:
	@$(CURDIR)/install.sh

clean:
	@rm -rf build *~

.PHONY: CloverPrefpane install clean
