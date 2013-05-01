
CloverUpdater:
	/usr/bin/xcodebuild -project 'CloverUpdater.xcode/CloverUpdater.xcodeproj' \
		-configuration 'Release' CONFIGURATION_BUILD_DIR=$(CURDIR)/build

install:
	@$(CURDIR)/install-updater.sh

clean:
	@rm -rf build

.PHONY: CloverUpdater install clean
