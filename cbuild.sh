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
echo "Xcode chainload"
export TARGET_TOOLS=XCODE32
}

fnClang ()
# Function: Clang chainload
{
echo "Clang chainload"
export TARGET_TOOLS=XCLANG
}

fnUnixgcc ()
# Function: Unixgcc chainload
{
echo "Unixgcc chainload"
export TARGET_TOOLS=UNIXGCC
}

fnArchIA32 ()
# Function: IA32 Arch function
{
echo "IA32 arch"
export PROCESSOR=IA32
export Processor=Ia32
}

fnArchX64 ()
# Function: X64 Arch function
{
echo "X64 arch"
export PROCESSOR=X64
export Processor=X64
}

fnDebug ()
# Function: Debug version of compiled source
{
echo "Debug TARGET"
export TARGET=DEBUG
export VTARGET=DEBUG
}

fnRelease ()
# Function: Release version of compiled source
{
echo "Release TARGET"
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
echo
echo "Example: ./cbuild.sh -xcode -ia32 -release"
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
echo "Example: ./cbuild.sh -unixgcc -x64 -release"
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

        '-clang')
         fnClang
        ;;

        '-unixgcc')
         fnUnixgcc
        ;;

    esac

# 2. Argument Case
    case "$2" in

        '')
        fnHelpArgument && exit
        ;;
        '-ia32')
         fnArchIA32
        ;;
        '-x64')
         fnArchX64
        ;;

    esac
# 3. Argument Case
    case "$3" in
        '')
         fnHelpArgument && exit
        ;;
        '-debug')
         fnDebug
        ;;
        
        '-release')
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


BUILD_ROOT_ARCH=$WORKSPACE/Build/Clover$PROCESSOR/"$VTARGET"_"$TARGET_TOOLS"/$PROCESSOR

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

# Build the edk2 DuetPkg
echo Running edk2 build for Clover$Processor
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
export BUILD_DIR=$WORKSPACE/Build/Clover$PROCESSOR/"$VTARGET"_"$TARGET_TOOLS"

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
echo Done!
fi

if [ $PROCESSOR = X64 ]
then
$BASETOOLS_DIR/GenFw --rebase 0x10000 -o $BUILD_DIR/$PROCESSOR/EfiLoader.efi $BUILD_DIR/$PROCESSOR/EfiLoader.efi
$BASETOOLS_DIR/EfiLdrImage -o $BUILD_DIR/FV/Efildr64 $BUILD_DIR/$PROCESSOR/EfiLoader.efi $BUILD_DIR/FV/DxeIpl.z $BUILD_DIR/FV/DxeMain.z $BUILD_DIR/FV/DUETEFIMAINFV.z
cat $BOOTSECTOR_BIN_DIR/Start64.com $BOOTSECTOR_BIN_DIR/efi64.com2 $BUILD_DIR/FV/Efildr64 > $BUILD_DIR/FV/EfildrPure
$BASETOOLS_DIR/GenPage $BUILD_DIR/FV/EfildrPure -o $BUILD_DIR/FV/Efildr
cat $BOOTSECTOR_BIN_DIR/St16_64.com $BOOTSECTOR_BIN_DIR/efi64.com2 $BUILD_DIR/FV/Efildr64 > $BUILD_DIR/FV/Efildr16Pure
$BASETOOLS_DIR/GenPage $BUILD_DIR/FV/Efildr16Pure -o $BUILD_DIR/FV/Efildr16
cat $BOOTSECTOR_BIN_DIR/start64H.com $BOOTSECTOR_BIN_DIR/efi64.com3 $BUILD_DIR/FV/Efildr64 > $BUILD_DIR/FV/Efildr20Pure
$BASETOOLS_DIR/GenPage $BUILD_DIR/FV/Efildr20Pure -o $BUILD_DIR/FV/Efildr20
cat $BOOTSECTOR_BIN_DIR/start64H.com2 $BOOTSECTOR_BIN_DIR/efi64.com3 $BUILD_DIR/FV/Efildr64 > $BUILD_DIR/FV/bootPure
$BASETOOLS_DIR/GenPage $BUILD_DIR/FV/bootPure -o $BUILD_DIR/FV/boot

echo Done!
fi

}

# BUILD START #
fnMainBuildScript
fnMainPostBuildScript

