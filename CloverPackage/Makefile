#	Makefile for kernel booter
SRCROOT = $(CURDIR)

image iso: utils
	@${SRCROOT}/makeiso

pkg installer:
	@${SRCROOT}/makepkg

updatepo:
	@${SRCROOT}/package/translate.sh --update-po

utils:
	@make -C "${SRCROOT}"/utils all

install:
	@make -C "${SRCROOT}"/utils install

clean:
	rm -rf sym obj dst

.PHONY: clean image iso pkg installer updatepo utils
