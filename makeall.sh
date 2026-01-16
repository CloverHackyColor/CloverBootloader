#!/bin/bash

./ebuild.sh -gcc151 -fr -n 5 -mc --no-usb -D NO_GRUB_DRIVERS_EMBEDDED
./ebuild.sh -gcc151 -fr -n 5 -D NO_GRUB_DRIVERS_EMBEDDED
cd CloverPackage
./makepkg >log.txt
./makeiso
./makeV2
#make iso
cd ..
echo "done!"

