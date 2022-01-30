#!/bin/sh
./shc -v -r -T -f ./ConfigValidator.sh
Sleep 1
rm -rf ./ConfigValidator/ConfigValidator.c
cp -rp ./ConfigValidator.sh.x.c ./ConfigValidator/ConfigValidator.c
rm -rf ./ConfigValidator.sh.x
rm -rf ./ConfigValidator.sh.x.c
