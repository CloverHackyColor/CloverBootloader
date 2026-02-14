#!/bin/bash

./ebuild.sh -gcc152 -fr -mc --no-usb -D NO_GRUB_DRIVERS_EMBEDDED
./ebuild.sh -gcc152 -fr -D NO_GRUB_DRIVERS_EMBEDDED
cd CloverPackage
./makepkg >log.txt
./makeiso
./makeV2
#make iso
cd ..
echo "done!"

