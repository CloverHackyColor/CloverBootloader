
CloverUpdater:
	@echo "Building CloverUpdater application..."
	@/usr/bin/xcodebuild -project 'src/CloverUpdater.xcodeproj' \
		-configuration 'Release' CONFIGURATION_BUILD_DIR=$(CURDIR)/build

install:
	@$(CURDIR)/install-updater.sh

clean:
	@rm -rf build *~

.PHONY: CloverUpdater install clean
