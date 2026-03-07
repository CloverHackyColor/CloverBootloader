
# CloverBootloader 
[![CI](https://github.com/CloverHackyColor/CloverBootloader/actions/workflows/main.yml/badge.svg)](https://github.com/CloverHackyColor/CloverBootloader/actions/workflows/main.yml) [![License](https://img.shields.io/badge/License-BSD%202--Clause-orange.svg)](https://github.com/CloverHackyColor/CloverBootloader/blob/master/LICENSE) 

Bootloader for macOS, Windows and Linux in UEFI and in legacy mode
### Read the DOC [Clover-Documentation](https://github.com/CloverHackyColor/Clover-Documentation) / [Clover-Documentation Site](https://cloverhackycolor.github.io/Clover-Documentation/)

# Features

- Boot macOS, Windows, and Linux in UEFI
-  or legacy mode on Mac or PC with UEFI or BIOS firmware
- Boot using UEFI firmware directly or CloverEFI UEFI firmware emulation
- Customizable GUI including themes, icons, fonts, background images, animations, and mouse pointers.
- Theme manager and theme repository at https://github.com/CloverHackyColor/CloverThemes
- Native screen resolution in GUI
- Press Page Up or Page Down to change GUI resolution
- Press **F1** for multilingual help, depending on language setting in configuration
- Press **F2** to save `preboot.log` from GUI
- Press **F3** to show hidden entries
- Press **F4** to save original (OEM) ACPI tables into `/EFI/CLOVER/ACPI/origin`
- Press **F5** to test DSDT patching
- Press **F6** to save graphics firmware into `/EFI/CLOVER/misc`
- Press **F7** to test HDA output
- Press **F9** to switch screen resolution
- Press **F10** to save screenshots from GUI
- Press **F11** to reset NVRAM
- Press **F12** to eject CD/DVD
- GUI refreshes after CD/DVD insertion
- Ability to boot previously selected boot entry after default timeout
- Boot entries menu scrolls if screen resolution is too low for menu
- Create custom boot entries for personalizing boot entries and add support for other operating systems
- Create Clover boot entry in NVRAM with tool from GUI
- Launch EFI command shell from GUI
- Startup sound checked by **F7**

### Developers:

*   Slice, with help of Kabyl, usr-sse2, jadran, Blackosx, dmazar, STLVNUB, pcj, apianti, JrCs, pene, FrodoKenny, skoczy, ycr.ru, Oscar09, xsmile, SoThOr, rehabman, Download-Fritz, nms42, Sherlocks, Zenit432, cecekpawon, stinga11, TheRacerMaster, solstice, Micky1979, Needy, joevt, ErmaC, vit9696, ath, savvas, syscl, goodwin\_c, clovy, jief\_machak, chris1111, vector\_sigma, LAbyOne, Florin9doi, YBronst, Hnanoto.
    
    ### Source code credits to:
    Intel, Apple, Oracle, Chameleon, rEFIt and Xom, nanosvg.
    
    ### Packages credits to :
    Chameleon team, crazybirdy, JrCs, chris1111.
    
    ### Clover is open source based on different projects :    
*   Chameleon, rEFIt, XNU, VirtualBox. [The main is EDK2 latest revision](https://github.com/CloverHackyColor/CloverBootloader)
*   Recent developments and changes in details at [Clover Change Explanations](https://www.insanelymac.com/forum/topic/304530-clover-change-explanations/)
*   Support forum discussion [AppleLife](https://applelife.ru/threads/clover.42089/) (Russian) [Insanelymac](https://www.insanelymac.com/forum/topic/284656-clover-general-discussion/) (English) [macos86.it](https://www.macos86.it/forum/46-clover/) (Italian)
---------------------------------------------------------
