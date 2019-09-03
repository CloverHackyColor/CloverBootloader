#	Makefile for kernel booter
SRCROOT = $(CURDIR)
# Reset default charset for GNUsed problems...
LC_ALL=C
export LC_ALL

image iso: utils
	@${SRCROOT}/makeiso

pkg installer:
	@${SRCROOT}/makepkg

slimpkg1:
	@${SRCROOT}/makepkg --nothemes

slimpkg2:
	@${SRCROOT}/makepkg --nothemes --noprefpane

slimpkg3:
	@${SRCROOT}/makepkg --nothemes --noprefpane --norc --nolegacy

updatepo:
	@${SRCROOT}/package/translate.sh --update-po

utils:
	@make -C "${SRCROOT}"/utils all

install:
	@make -C "${SRCROOT}"/utils install

clean:
	rm -rf sym obj dst CloverPrefpane/build CloverUpdater/build

.PHONY: clean image iso pkg installer updatepo utils
