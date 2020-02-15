#!/bin/bash

#./ebuild.sh -fr --no-lto --ia32  -D NO_GRUB_DRIVERS_EMBEDDED
./ebuild.sh -fr -mc --no-usb -D NO_GRUB_DRIVERS_EMBEDDED
./ebuild.sh -fr -D NO_GRUB_DRIVERS_EMBEDDED
cd CloverPackage
./makepkg
./makeiso
#make iso
cd ..
echo "done!"

