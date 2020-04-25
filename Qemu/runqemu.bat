Powershell Dismount-DiskImage -ImagePath "C:\Users\Sergey\Desktop\QEMU\test1.vhdx"
"C:\Program Files\qemu\qemu-system-x86_64.exe" -L . -m 2048 -cpu Penryn -bios OVMF.fd -vga cirrus -device ahci,id=ahc -drive format=vhdx,id=hda,file=test1.vhdx -drive format=vhdx,id=hdb,file=freedos.vhdx -usb -device usb-mouse,bus=usb-bus.0,port=2 -device usb-kbd,bus=usb-bus.0,port=1

Powershell Mount-DiskImage -ImagePath "C:\Users\Sergey\Desktop\QEMU\test1.vhdx"