

Clover:
	@echo "Building Clover..."
	@./ebuild.sh -gcc131 -D LESS_DEBUG -D NO_GRUB_DRIVERS_EMBEDDED >/dev/null
	@echo [BUILD] "Clover " $Version.h

clean:
	@./ebuild.sh -gcc131 -clean >/dev/null
	@rm -rf build *~
	@echo [CLEAN] 

.PHONY: Clover clean
