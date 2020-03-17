
CloverApp:
	@echo [XCODE] $(PROGRAMS)
	@package/build.sh
	

clean:
	@/usr/bin/xcodebuild -project 'Clover.xcodeproj' -scheme 'Clover' clean  >/dev/null
	@rm -rf build *~
	@echo [CLEAN] $(PROGRAMS)

.PHONY: CloverApp clean
