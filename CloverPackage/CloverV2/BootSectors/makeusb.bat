@set EFI_BOOT_DISK=%1
@echo on

:CreateUsb_FAT32
@echo Format %EFI_BOOT_DISK% ...
@echo.> FormatCommandInput.txt
@format /FS:FAT32 /q %EFI_BOOT_DISK% < FormatCommandInput.txt > NUL
@del FormatCommandInput.txt
@echo Create boot sector ...
@Genbootsector.exe -i %EFI_BOOT_DISK% -o UsbBs32.bin
@Bootsectimage.exe -g UsbBs32.bin boot1f32.bin -f
@Genbootsector.exe -o %EFI_BOOT_DISK% -i boot1f32.bin
@Genbootsector.exe -m -o %EFI_BOOT_DISK% -i boot0.bin
@del usbbs32.bin
@echo Done.
@echo RePlug device before using it!
