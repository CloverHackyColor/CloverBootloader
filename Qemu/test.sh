#!/bin/bash

#cp -v /Users/sergey/src/edk2/Clover/CloverPackage/CloverV2/EFI/CLOVER/drivers64UEFI/VBoxHfs-64.efi /Volumes/TEFI/EFI/CLOVER/drivers64UEFI
#sudo umount /Volumes/QEFI
diskutil umount /dev/disk2s1
#qemu -L ~/Desktop/QEMU-Clover/QEMU -m 2048 -cpu core2duo -bios OVMF.fd  -machine q35 -usb -device usb-mouse,bus=usb-bus.0,port=2 -device usb-kbd,bus=usb-bus.0,port=1 -device ahci,id=ahc -device ide-drive,bus=ahc.0,drive=hdc -drive id=hdc,file=/Users/sergey/Desktop/QEMU-Clover/QEMU-test2.img -device ide-drive,bus=ahc.1,drive=hdb -drive id=hdb,if=none,file=/Users/sergey/Desktop/QEMU-Clover/freedos.img -net nic,macaddr=00:12:32:43:55:16 -net user,name=lan -serial stdio

#qemu -L ~/Desktop/QEMU-Clover/QEMU -m 2048 -vga std -cpu core2duo -bios OVMF.fd  -machine q35 -usb -device usb-mouse,bus=usb-bus.0,port=2 -device usb-kbd,bus=usb-bus.0,port=1 -hda /Users/sergey/Desktop/QEMU-Clover/QEMU-test2.img -hdc /Users/sergey/Desktop/QEMU-Clover/freedos.img -net nic,macaddr=00:12:32:43:55:16 -net user,name=lan
#-serial stdio

qemu -L . -m 2048 -cpu core2duo -bios bios.bin-1.13.0  -machine q35 -device ahci,id=ahc -drive format=raw,file=QEMU-test3.img -usb -device usb-mouse,bus=usb-bus.0,port=2 -device usb-kbd,bus=usb-bus.0,port=1
#-serial stdio

#qemu -L ~/Desktop/QEMU-Clover/QEMU -m 2048 -vga std -cpu core2duo -bios bios.bin  -machine q35 -usb -device usb-mouse,bus=usb-bus.0,port=2 -device usb-kbd,bus=usb-bus.0,port=1 -hda /Users/sergey/Desktop/QEMU-Clover/QEMU-test2.img -hdc /Users/sergey/Desktop/QEMU-Clover/freedos.img -net nic,macaddr=00:12:32:43:55:16 -net user,name=lan
#-serial stdio

#sudo mkdir /Volumes/QEFI
#sudo mount -t msdos /dev/disk1s1 /Volumes/QEFI
diskutil mount /dev/disk2s1
