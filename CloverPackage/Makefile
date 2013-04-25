#	Makefile for kernel booter
SRCROOT = $(CURDIR)

image iso:
	@${SRCROOT}/makeiso

pkg installer:
	@${SRCROOT}/makepkg

updatepo:
	@${SRCROOT}/package/translate.sh --update-po

clean:
	rm -rf sym obj dst

.PHONY: clean image iso pkg installer updatepo
