#!/bin/bash

#./ebuild.sh -gcc53 -fr --no-lto --ia32  -D NO_GRUB_DRIVERS_EMBEDDED
./ebuild.sh -gcc53 -fr -n 5 -mc --no-usb -D NO_GRUB_DRIVERS_EMBEDDED
./ebuild.sh -gcc53 -fr -n 5 -D NO_GRUB_DRIVERS_EMBEDDED
cd CloverPackage
#./makepkg
./makeiso
#make iso
cd ..
echo "done!"

