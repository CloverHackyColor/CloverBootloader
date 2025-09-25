#!/bin/bash

./ebuild.sh -gcc151 -fr -n 5 -mc --no-usb -D NO_GRUB_DRIVERS_EMBEDDED
./ebuild.sh -gcc151 -fr -n 5 -D NO_GRUB_DRIVERS_EMBEDDED
cd CloverPackage
./makepkg
./makeiso
./makeV2
cd ..
echo "done!"

