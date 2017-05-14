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
	@${SRCROOT}/makepkg 1

slimpkg2:
	@${SRCROOT}/makepkg 2

slimpkg3:
	@${SRCROOT}/makepkg 3

updatepo:
	@${SRCROOT}/package/translate.sh --update-po

utils:
	@make -C "${SRCROOT}"/utils all

install:
	@make -C "${SRCROOT}"/utils install

clean:
	rm -rf sym obj dst

.PHONY: clean image iso pkg installer updatepo utils
