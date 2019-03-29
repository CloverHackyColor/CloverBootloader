#!/bin/bash

# the script to update only needed folders from edk2
# to not keep unnecessary Arm*, Beagle, NT32, Unix...

cd ../MdePkg/
svn up
cd ../MdeModulePkg/
svn up
cd ../CryptoPkg/
svn up
cd ../IntelFrameworkModulePkg/
svn up
cd ../IntelFrameworkPkg/
svn up
cd ../OvmfPkg/
svn up
cd ../OptionRomPkg/
svn up
cd ../PcAtChipsetPkg/
svn up
cd ../ShellPkg/
svn up
cd ../UefiCpuPkg/
svn up
cd ../BaseTools/Conf
svn up
cd ../Source/
svn up
cd ../Scripts/
svn up
cd ../../Clover/
svn up

echo "now you can do: cp -R Patches_for_EDK2/* ../"