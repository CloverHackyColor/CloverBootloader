# CloverBootloader
Bootloader for macOS, Windows and Linux in UEFI and in legacy mode
# Features

- Boot macOS, Windows, and Linux in UEFI or legacy mode on Mac or PC with UEFI or BIOS firmware
- Boot using UEFI firmware directly or CloverEFI UEFI firmware emulation
- Customizable GUI including themes, icons, fonts, background images, animations, and mouse pointers.
- Theme manager and theme repository at http://sourceforge.net/p/cloverefiboot/themes/
- Native screen resolution in GUI
- Press Page Up or Page Down to change GUI resolution
- Press **F1** for multilingual help, depending on language setting in configuration
- Press **F2** to save `preboot.log` from GUI
- Press **F4** to save original (OEM) ACPI tables into `/EFI/CLOVER/ACPI/origin`
- Press **F5** to test DSDT patching
- Press **F6** to save graphics firmware into `/EFI/CLOVER/misc`
- Press **F10** to save screenshots from GUI
- Press **F11** to reset NVRAM
- Press **F12** to eject CD/DVD
- GUI refreshes after CD/DVD insertion
- Ability to boot previously selected boot entry after default timeout
- Boot entries menu scrolls if screen resolution is too low for menu
- Create custom boot entries for personalizing boot entries and add support for other operating systems
- Create Clover boot entry in NVRAM with tool from GUI
- Launch EFI command shell from GUI
- Startup sound controlled by **F8** and checked by **F7**
