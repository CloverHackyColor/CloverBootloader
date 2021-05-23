#
#  Makefile
#  alternate build for buldpkg
#  Created on: May 12, 2021
#  Author: LAbyOne
#
 
CloverConfigPlistValidator:
	@echo [XCODE] $(PROGRAMS)
	@$(CURDIR)/Install.sh
	

clean:
	@/usr/bin/xcodebuild -project 'CloverConfigPlistValidator.xcodeproj' clean  >/dev/null
	@rm -rf build *~
	@echo [CLEAN] $(PROGRAMS)

.PHONY: CloverConfigPlistValidator clean
