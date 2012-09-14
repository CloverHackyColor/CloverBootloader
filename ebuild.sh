#!/bin/sh

#  cbuild.sh
#  Script for building CloverEFI source under OS X
#  Supported chainloads(compilers) are XCODE32, UNIXGCC and CLANG
#  
#
#  Created by Jadran Puharic on 1/6/12.
#  


## FUNCTIONS ##

fnXcode ()
# Function: Xcode chainload
{
[ ! -f /usr/bin/xcodebuild ] && \
echo "ERROR: Install Xcode Tools from Apple before using this script." && exit
echo "CHAINLOAD: XCODE"
export TARGET_TOOLS=XCODE32
}

fnXcode4 ()
# Function: Xcode chainload
{
[ ! -f /usr/bin/xcodebuild ] && \
echo "ERROR: Install Xcode Tools from Apple before using this script." && exit
echo "CHAINLOAD: XCODE"
export TARGET_TOOLS=XCODE41
}

fnGCC46 ()
# Function: Xcode chainload
{
[ ! -f /usr/bin/xcodebuild ] && \
echo "ERROR: Install Xcode Tools from Apple before using this script." && exit
echo "CHAINLOAD: GCC46"
export TARGET_TOOLS=GCC46
}

fnClang ()
# Function: Clang chainload
{
echo "CHAINLOAD: XCODE CLANG"
export TARGET_TOOLS=XCLANG
}

fnLClang ()
# Function: Clang chainload
{
echo "CHAINLOAD: LLVM CLANG"
export TARGET_TOOLS=LCLANG
}

fnUnixgcc ()
# Function: Unixgcc chainload
{
echo "CHAINLOAD: UNIXGCC"
export TARGET_TOOLS=UNIXGCC
}

fnArchIA32 ()
# Function: IA32 Arch function
{
echo "ARCH: IA32"
export PROCESSOR=IA32
export Processor=Ia32
}

fnArchX64 ()
# Function: X64 Arch function
{
echo "ARCH: X64"
export PROCESSOR=X64
export Processor=X64
}

fnArch64MCP ()
# Function: X64 Arch function for MCP
{
echo "ARCH: X64 MCP"
export PROCESSOR=X64
export Processor=64MCP
}

fnDebug ()
# Function: Debug version of compiled source
{
echo "TARGET: DEBUG"
export TARGET=DEBUG
export VTARGET=DEBUG
}

fnRelease ()
# Function: Release version of compiled source
{
echo "TARGET: RELEASE"
export TARGET=RELEASE
export VTARGET=RELEASE
}

fnHelp ()
# Function: Help
{
echo ""
echo "Script for building CloverEFI source on Darwin OS X"
echo
echo "Usage: ./cbuild.sh [COMPILER] [ARCH] [TYPE]"
echo 
echo "Script arguments:"
echo "[COMPILER]   [ARCH]     [TYPE]"
echo "-xcode       -ia32      -debug"
echo "-clang       -x64       -release"
echo "-unixgcc"
echo "-gcc46"
echo "-llvm"
echo
echo "Example: ./cbuild.sh -xcode -ia32 -release"
echo "Example: ./cbuild.sh -gcc46 -x64 -release"
echo "Example: ./cbuild.sh -gcc46 -ia32 -release"
echo "example: ./cbuild.sh -32"
echo "example: ./cbuild.sh -64"
echo "example: ./cbuild.sh -34"
echo
echo "If you want to clean a build:"
echo "Example: ./cbuild.sh -xcode -ia32 -release -clean"
echo "Example: ./cbuild.sh -xcode -ia32 -release -cleanall"
echo
}

fnHelpArgument ()
# Function: Help with arguments
{
echo "ERROR!"
echo "Example: ./cbuild.sh -xcode -ia32 -release"
echo "Example: ./cbuild.sh -gcc46 -x64 -release"
}

## MAIN ARGUMENT PART##

# 1. Argument Case
    case "$1" in
        '')
         fnHelp && exit
        ;;
        '-help')
         fnHelp && exit
        ;;
        '-xcode')
         fnXcode
        ;;
        '-xcode4')
         fnXcode4
        ;;
        '-clang')
         fnClang
        ;;
        '-llvm')
        fnLClang
        ;;
        '-unixgcc')
         fnUnixgcc
        ;;
        '-gcc46')
         fnGCC46
        ;;
        '-32')
         fnXcode
         fnArchIA32
        ;;
        '-34')
         fnGCC46
         fnArchIA32
        ;;
        '-64')
         fnGCC46
         fnArchX64
        ;;
        '-mc')
         fnGCC46
         fnArch64MCP
        ;;
        *)
         echo $"ERROR!"
         echo $"COMPILER: {-xcode|-xcode4|-clang|-unixgcc|-gcc46}"
		echo $"or default {-32|-64|-34|-MC}"
        exit 1		
    esac

# 2. Argument Case
    case "$2" in
#        '')
#        fnHelpArgument && exit
#        ;;
        '-ia32')
         fnArchIA32
        ;;
        '-x64')
         fnArchX64
        ;;
        *)
#         echo $"ERROR!"
#         echo $"ARCH: {-ia32|-x64}"
#        exit 1
		echo $"using default for compiler"
		;;
    esac

# 3. Argument Case
    case "$3" in
#        '')
#         fnHelpArgument && exit
#        ;;
        '-debug')
         fnDebug
        ;;
        
        '-release')
         fnRelease
        ;;
        *)
#         echo $"ERROR!"
#         echo $"TYPE: {-debug|-release}"
#        exit 1
		 echo $"default -release"
		 fnRelease
		 ;;
    esac

# 4. Argument Case
    case "$4" in
        '-clean')
        export ARG=clean
        ;;

        '-cleanall')
        export ARG=cleanall
        ;;
    esac


## MAIN BUILD/POSTBUILD SCRIPT PART##

fnMainBuildScript ()
# Function MAIN DUET BUILD SCRIPT
{
set -e
shopt -s nocasematch
svnversion -n | tr -d [:alpha:] >vers.txt
#
# Setup workspace if it is not set
#
if [ -z "$WORKSPACE" ]
then
echo Initializing workspace
if [ ! -e `pwd`/edksetup.sh ]
then
cd ..
fi
# This version is for the tools in the BaseTools project.
# this assumes svn pulls have the same root dir
#  export EDK_TOOLS_PATH=`pwd`/../BaseTools
# This version is for the tools source in edk2
export EDK_TOOLS_PATH=`pwd`/BaseTools
echo $EDK_TOOLS_PATH
source edksetup.sh BaseTools
else
echo Building from: $WORKSPACE
fi


BUILD_ROOT_ARCH=$WORKSPACE/Build/Clover$Processor/"$VTARGET"_"$TARGET_TOOLS"/$PROCESSOR

if  [[ ! -f `which build` || ! -f `which GenFv` ]];
then
# build the tools if they don't yet exist. Bin scheme
echo Building tools as they are not in the path
make -C $WORKSPACE/BaseTools
elif [[ ( -f `which build` ||  -f `which GenFv` )  && ! -d  $EDK_TOOLS_PATH/Source/C/bin ]];
then
# build the tools if they don't yet exist. BinWrapper scheme
echo Building tools no $EDK_TOOLS_PATH/Source/C/bin directory
make -C $WORKSPACE/BaseTools
else
echo using prebuilt tools
fi

# Cleaning part of the script if we have $4 argument: clean/cleanall
if [[ $ARG == cleanall ]]; then
make -C $WORKSPACE/BaseTools clean
build -p $WORKSPACE/Clover/Clover$Processor.dsc -a $PROCESSOR -b $VTARGET -t $TARGET_TOOLS -n 3 clean
exit $?
fi

if [[ $ARG == clean ]]; then
build -p $WORKSPACE/Clover/Clover$Processor.dsc -a $PROCESSOR -b $VTARGET -t $TARGET_TOOLS -n 3 clean
exit $?
fi

# Build the CloverPkg
echo Running edk2 build for Clover$Processor
#rm $WORKSPACE/Clover/Version.h
echo "#define FIRMWARE_VERSION \"2.31\"" > $WORKSPACE/Clover/Version.h
echo "#define FIRMWARE_BUILDDATE \"`date \"+%Y-%m-%d %H:%M:%S\"`\"" >> $WORKSPACE/Clover/Version.h
#echo "#define FIRMWARE_REVISION L\"`svnversion -n | tr -d [:alpha:]`\"" >> $WORKSPACE/Clover/Version.h
echo "#define FIRMWARE_REVISION \"`cat Clover/vers.txt`\"" >> $WORKSPACE/Clover/Version.h
echo "#define FIRMWARE_REVISION_STR \"Clover revision: `cat Clover/vers.txt`\"" >> $WORKSPACE/Clover/Version.h

build -p $WORKSPACE/Clover/Clover$Processor.dsc -a $PROCESSOR -b $VTARGET -t $TARGET_TOOLS -n 3 $*

}


fnMainPostBuildScript ()
# Function MAIN DUET POSTBUILD SCRIPT
{

if [ -z "$EDK_TOOLS_PATH" ]
then
export BASETOOLS_DIR=$WORKSPACE/BaseTools/Source/C/bin
else
export BASETOOLS_DIR=$EDK_TOOLS_PATH/Source/C/bin
fi
export BOOTSECTOR_BIN_DIR=$WORKSPACE/Clover/BootSector/bin
export BUILD_DIR=$WORKSPACE/Build/Clover/"$VTARGET"_"$TARGET_TOOLS"

#[ ! -f $BUILD_DIR/FV/DUETEFIMAINFV.z ] && \
#echo "ERROR: Build not finished exiting PostBuild Part..." && exit

#
# Boot sector module could only be built under IA32 tool chain - sure?
#

echo Compressing DUETEFIMainFv.FV ...
$BASETOOLS_DIR/LzmaCompress -e -o $BUILD_DIR/FV/DUETEFIMAINFV.z $BUILD_DIR/FV/DUETEFIMAINFV.Fv

echo Compressing DxeMain.efi ...
$BASETOOLS_DIR/LzmaCompress -e -o $BUILD_DIR/FV/DxeMain.z $BUILD_DIR/$PROCESSOR/DxeCore.efi

echo Compressing DxeIpl.efi ...
$BASETOOLS_DIR/LzmaCompress -e -o $BUILD_DIR/FV/DxeIpl.z $BUILD_DIR/$PROCESSOR/DxeIpl.efi	

echo Generate Loader Image ...

if [ $PROCESSOR = IA32 ]
then
$BASETOOLS_DIR/GenFw --rebase 0x10000 -o $BUILD_DIR/$PROCESSOR/EfiLoader.efi $BUILD_DIR/$PROCESSOR/EfiLoader.efi
$BASETOOLS_DIR/EfiLdrImage -o $BUILD_DIR/FV/Efildr32 $BUILD_DIR/$PROCESSOR/EfiLoader.efi $BUILD_DIR/FV/DxeIpl.z $BUILD_DIR/FV/DxeMain.z $BUILD_DIR/FV/DUETEFIMAINFV.z

cat $BOOTSECTOR_BIN_DIR/start32.com $BOOTSECTOR_BIN_DIR/efi32.com3 $BUILD_DIR/FV/Efildr32 > $BUILD_DIR/FV/Efildr20	
cat $BOOTSECTOR_BIN_DIR/start32H.com2 $BOOTSECTOR_BIN_DIR/efi32.com3 $BUILD_DIR/FV/Efildr32 > $BUILD_DIR/FV/boot
cp -v $BUILD_DIR/FV/boot $WORKSPACE/Clover/CloverPackage/CloverV2/Bootloaders/ia32/
cp -v $BUILD_DIR/IA32/FSInject.efi $WORKSPACE/Clover/CloverPackage/CloverV2/EFI/drivers32/FSInject-32.efi
#cp -v $BUILD_DIR/IA32/VBoxIso9600.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers32/VBoxIso9600-32.efi
cp -v $BUILD_DIR/IA32/VBoxExt2.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers32/VBoxExt2-32.efi
cp -v $BUILD_DIR/IA32/Ps2KeyboardDxe.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers32/Ps2KeyboardDxe-32.efi
cp -v $BUILD_DIR/IA32/Ps2MouseAbsolutePointerDxe.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers32/Ps2MouseAbsolutePointerDxe-32.efi
cp -v $BUILD_DIR/IA32/Ps2MouseDxe.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers32/Ps2MouseDxe-32.efi
cp -v $BUILD_DIR/IA32/UsbMouseDxe.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers32/UsbMouseDxe-32.efi
cp -v $BUILD_DIR/IA32/XhciDxe.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers32/XhciDxe-32.efi
cp -v $BUILD_DIR/IA32/OsxFatBinaryDrv.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers32UEFI/OsxFatBinaryDrv-32.efi

echo Done!
fi

if [ $Processor = X64 ]
then
$BASETOOLS_DIR/GenFw --rebase 0x10000 -o $BUILD_DIR/$PROCESSOR/EfiLoader.efi $BUILD_DIR/$PROCESSOR/EfiLoader.efi
$BASETOOLS_DIR/EfiLdrImage -o $BUILD_DIR/FV/Efildr64 $BUILD_DIR/$PROCESSOR/EfiLoader.efi $BUILD_DIR/FV/DxeIpl.z $BUILD_DIR/FV/DxeMain.z $BUILD_DIR/FV/DUETEFIMAINFV.z
#cat $BOOTSECTOR_BIN_DIR/Start64.com $BOOTSECTOR_BIN_DIR/efi64.com2 $BUILD_DIR/FV/Efildr64 > $BUILD_DIR/FV/EfildrPure
#$BASETOOLS_DIR/GenPage $BUILD_DIR/FV/EfildrPure -o $BUILD_DIR/FV/Efildr
#cat $BOOTSECTOR_BIN_DIR/St16_64.com $BOOTSECTOR_BIN_DIR/efi64.com2 $BUILD_DIR/FV/Efildr64 > $BUILD_DIR/FV/Efildr16Pure
#$BASETOOLS_DIR/GenPage $BUILD_DIR/FV/Efildr16Pure -o $BUILD_DIR/FV/Efildr16
cat $BOOTSECTOR_BIN_DIR/Start64H.com $BOOTSECTOR_BIN_DIR/efi64.com3 $BUILD_DIR/FV/Efildr64 > $BUILD_DIR/FV/Efildr20Pure
$BASETOOLS_DIR/GenPage $BUILD_DIR/FV/Efildr20Pure -o $BUILD_DIR/FV/Efildr20
#cat $BOOTSECTOR_BIN_DIR/Start64.com2 $BOOTSECTOR_BIN_DIR/efi64.com2 $BUILD_DIR/FV/Efildr64 > $BUILD_DIR/FV/bootPure
#$BASETOOLS_DIR/GenPage $BUILD_DIR/FV/bootPure -o $BUILD_DIR/FV/boot
dd if=$BUILD_DIR/FV/Efildr20 of=$BUILD_DIR/FV/boot bs=512 skip=1
cp -v $BUILD_DIR/FV/boot $WORKSPACE/Clover/CloverPackage/CloverV2/Bootloaders/x64/
cp -v $BUILD_DIR/X64/FSInject.efi $WORKSPACE/Clover/CloverPackage/CloverV2/EFI/drivers64/FSInject-64.efi
cp -v $BUILD_DIR/X64/FSInject.efi $WORKSPACE/Clover/CloverPackage/CloverV2/EFI/drivers64UEFI/FSInject-64.efi
#cp -v $BUILD_DIR/X64/VBoxIso9600.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers64/VBoxIso9600-64.efi
cp -v $BUILD_DIR/X64/VBoxExt2.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers64/VBoxExt2-64.efi
cp -v $BUILD_DIR/X64/PartitionDxe.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers64UEFI/PartitionDxe-64.efi
cp -v $BUILD_DIR/X64/DataHubDxe.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers64UEFI/DataHubDxe-64.efi

#cp -v $BUILD_DIR/X64/Ps2KeyboardDxe.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers64/Ps2KeyboardDxe-64.efi
#cp -v $BUILD_DIR/X64/Ps2MouseAbsolutePointerDxe.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers64/Ps2MouseAbsolutePointerDxe-64.efi
cp -v $BUILD_DIR/X64/Ps2MouseDxe.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers64/Ps2MouseDxe-64.efi
cp -v $BUILD_DIR/X64/UsbMouseDxe.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers64/UsbMouseDxe-64.efi
cp -v $BUILD_DIR/X64/XhciDxe.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers64/XhciDxe-64.efi
cp -v $BUILD_DIR/X64/OsxFatBinaryDrv.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers64UEFI/OsxFatBinaryDrv-64.efi
cp -v $BUILD_DIR/X64/OsxAptioFixDrv.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers64UEFI/OsxAptioFixDrv-64.efi
cp -v $BUILD_DIR/X64/OsxLowMemFixDrv.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers64UEFI/OsxLowMemFixDrv-64.efi
echo Done!
fi

if [ $Processor = 64MCP ]
then
$BASETOOLS_DIR/GenFw --rebase 0x10000 -o $BUILD_DIR/$PROCESSOR/EfiLoader.efi $BUILD_DIR/$PROCESSOR/EfiLoader.efi
$BASETOOLS_DIR/EfiLdrImage -o $BUILD_DIR/FV/Efildr64 $BUILD_DIR/$PROCESSOR/EfiLoader.efi $BUILD_DIR/FV/DxeIpl.z $BUILD_DIR/FV/DxeMain.z $BUILD_DIR/FV/DUETEFIMAINFV.z
cat $BOOTSECTOR_BIN_DIR/Start64H.com $BOOTSECTOR_BIN_DIR/efi64.com3 $BUILD_DIR/FV/Efildr64 > $BUILD_DIR/FV/Efildr20Pure
$BASETOOLS_DIR/GenPage $BUILD_DIR/FV/Efildr20Pure -o $BUILD_DIR/FV/Efildr20
dd if=$BUILD_DIR/FV/Efildr20 of=$BUILD_DIR/FV/boot7 bs=512 skip=1
cp -v $BUILD_DIR/FV/boot7 $WORKSPACE/Clover/CloverPackage/CloverV2/Bootloaders/X64/
cp -v $BUILD_DIR/X64/FSInject.efi $WORKSPACE/Clover/CloverPackage/CloverV2/EFI/drivers64/FSInject-64.efi
#cp -v $BUILD_DIR/X64/VBoxIso9600.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers64/VBoxIso9600-64.efi
cp -v $BUILD_DIR/X64/VBoxExt2.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers64/VBoxExt2-64.efi
cp -v $BUILD_DIR/X64/PartitionDxe.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers64/PartitionDxe-64.efi

#cp -v $BUILD_DIR/X64/Ps2KeyboardDxe.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers64/Ps2KeyboardDxe-64.efi
#cp -v $BUILD_DIR/X64/Ps2MouseAbsolutePointerDxe.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers64/Ps2MouseAbsolutePointerDxe-64.efi
cp -v $BUILD_DIR/X64/Ps2MouseDxe.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers64/Ps2MouseDxe-64.efi
cp -v $BUILD_DIR/X64/UsbMouseDxe.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers64/UsbMouseDxe-64.efi
cp -v $BUILD_DIR/X64/XhciDxe.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers64/XhciDxe-64.efi
cp -v $BUILD_DIR/X64/OsxFatBinaryDrv.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers64UEFI/OsxFatBinaryDrv-64.efi
cp -v $BUILD_DIR/X64/OsxAptioFixDrv.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers64UEFI/OsxAptioFixDrv-64.efi
cp -v $BUILD_DIR/X64/OsxLowMemFixDrv.efi $WORKSPACE/Clover/CloverPackage/CloverV2/drivers-Off/drivers64UEFI/OsxLowMemFixDrv-64.efi
echo Done!
fi

} 

# BUILD START #
fnMainBuildScript
fnMainPostBuildScript

