Slice

The Clover EFI boot project is just Duet but with follow corrections:

Note! Efildr20 is restricted in size to 483kb (or 452kb?) so I have to erase unnecessary codes from the project to be able to add new features, new drivers and just update existing drivers.
I am not a member of EDK2 team and can't influence on the sources so I copy some sources into my project and correct them. Hope some day Tiano will look into my corrections and apply them.

1. My computer has no Serial Port so I need no DEBUG to Serial. To reduce a space I delete all DEBUG codes from BdsDxe, BdsLib, BdsPlatform, DxeIpl, DxeCore, EfiLdr.
2. To start from ReadOnly volume I have to exclude any FS writing operations, mostly related to Efivar.bin.
3. Some quirky BIOSes needs to be patched in Smbios and Acpi tables that performed by SmbiosGenDxe and AcpiPlatformDxe. But also I need to correct SmbiosProtocol and AcpiProtocol because EDK2 prevented these tables to be patched.
4. Modules PciBusNoEnumerationDxe, RuntimeDxe, SataControllerDxe, VideoBios, VgaClass are corrected because of non-compilability of originals.
5. Oracle's project VirtualBox contains some necessary patches to IdeController, IdeBus, PeCoffLib, ConsoleControlProtocol, UefiCpuDxe and PartitionDxe. It also contains more FileSystem's drivers, not only FAT32. It's a pity VBoxHFS is not perfect and NTFS driver is absent.
6. I also added features that will be accounted as improvement so they are not interesting for EDK2. For example more resolutions in BiosVideo. And more.
7. Some not used modules in the project will remain here as a code examples.

Second step I can launch rEFIt.efi and boot into real OS that impossible with original Duet. But I still have problems:

1. Notebook Dell Inspiron 1525. Intel Core2Duo T8300 2400MHz, Intel X3100 video.
~~ANY Efildr20 just reboot immediately.~~
Resolved. Explanation [here](http://www.projectosx.com/forum/index.php?showtopic=2008&view=findpost&p=16107)
2. I can boot into OS but with 1024Mb RAM installed the OS crashes with DMA operations. 
Looks like the EFI EDK2 has wrong MemoryMap.
~~WIKI said that it might be 24bytes while start32 uses only 20bytes.~~
~~Resolved by patching OS kernel.~~
Fully resolved by correcting memory map.
3. I can go to S3 state (sleep) but never return back to system. The module S3Resume supposes to return to Pei. NO, NO, NO!!! I want to return to system!
Resolved! S3Save and S3Resume assume UEFI method and I just set BIOS method for resume. S3Save and S3Resume no more used.

All major problems are resolved and now the project will be polished.
