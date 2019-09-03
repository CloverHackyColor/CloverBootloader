#!/bin/sh
./gnu-gcc -c -o st32_64.o st32_64.S -Os -arch=x64 -save-temps -g -fno-strict-aliasing -Wall -Werror -fno-stack-protector
./gnu-ld --oformat binary -o Start64H.com st32_64.o -Ttext 0 -Map start.map

./gnu-gcc -c -o efi64.o efi64.S -Os -arch=i386 -save-temps -combine -mms-bitfields -fshort-wchar -fno-strict-aliasing -Wall -Werror -Wno-missing-braces -c -fno-stack-protector
./gnu-ld --oformat binary -o efi64.com efi64.o -Ttext 0 -Map efi64.map
dd if=efi64.com of=efi64.com3 bs=512 skip=264

cat Start64H.com efi64.com3 Efildr64 >Efildr20Pure
../../BaseTools/Source/C/bin/GenPage Efildr20Pure -o Efildr20
dd if=Efildr20 of=BOOT bs=512 skip=1
