#!/bin/sh

#  cbuild.sh
#  Script for building CloverEFI source under OS X
#  Supported chainloads(compilers) are XCODE32, UNIXGCC and CLANG
#  
#
#  Created by Jadran Puharic on 1/6/12.
#  
svnversion -n | tr -d [:alpha:] >vers.txt
## FUNCTIONS ##

if [ "$mygccVers" == "" ]; then
	mygccVers=46
fi	

fnXcode ()
# Function: Xcode chainload
{
[ ! -f /usr/bin/gcc ] && \
echo "ERROR: Install Xcode Tools from Apple before using this script." && exit
echo "CHAINLOAD: XCODE"
export TARGET_TOOLS=XCODE32
}

fnXcode4 ()
# Function: Xcode chainload
{
[ ! -f /usr/bin/gcc ] && \
echo "ERROR: Install Xcode Tools from Apple before using this script." && exit
echo "CHAINLOAD: XCODE"
export TARGET_TOOLS=XCODE41
}

fnGCC()
# Function: Xcode chainload
{
echo "CHAINLOAD: GCC$mygccVers"
export TARGET_TOOLS="GCC$mygccVers"
}

fnClang ()
# Function: Clang chainload
{
echo "CHAINLOAD: CLANG"
export TARGET_TOOLS=XCLANG
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
echo "-gcc"
echo
echo "Example: ./cbuild.sh -xcode -ia32 -release"
echo "Example: ./cbuild.sh -gcc -x64 -release"
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
echo "Example: ./cbuild.sh -gcc -x64 -release"
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
        '-unixgcc')
         fnUnixgcc
        ;;
        '-gcc')
         fnGCC
        ;;
        *)
         echo $"ERROR!"
         echo $"COMPILER: {-xcode|-xcode4|-clang|-unixgcc|-gcc}"
        exit 1
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
        *)
         echo $"ERROR!"
         echo $"ARCH: {-ia32|-x64}"
        exit 1
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
        *)
         echo $"ERROR!"
         echo $"TYPE: {-debug|-release}"
        exit 1
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
cd ../..
#echo "current folder `pwd`"
#exit
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


BUILD_ROOT_ARCH=$WORKSPACE/Build/rEFIt/$TARGET_TOOLS_$VTARGET/$PROCESSOR
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

echo Running edk2 build for rEFIt_UEFI$Processor
rm -f $WORKSPACE/Clover/rEFIt_UEFI/Version.h
echo "#define FIRMWARE_VERSION \"2.31\"" > $WORKSPACE/Clover/rEFIt_UEFI/Version.h
echo "#define FIRMWARE_BUILDDATE \"`date \"+%Y-%m-%d %H:%M:%S\"`\"" >> $WORKSPACE/Clover/rEFIt_UEFI/Version.h
#echo "#define FIRMWARE_REVISION \"`svnversion -n | tr -d [:alpha:]`\"" >> $WORKSPACE/Clover/rEFIt_UEFI/Version.h
echo "#define FIRMWARE_REVISION \"`cat Clover/rEFIt_UEFI/vers.txt`\"" >> $WORKSPACE/Clover/rEFIt_UEFI/Version.h
#rm -f $WORKSPACE/Clover/rEFIt_UEFI/vers.txt
if [ $PROCESSOR = IA32 ]; then
# Cleaning part of the script if we have $4 argument: clean/cleanall
	if [[ $ARG == cleanall ]]; then
		make -C $WORKSPACE/BaseTools clean
		build -p $WORKSPACE/Clover/rEFIt_UEFI/rEFIt.dsc -a $PROCESSOR -b $VTARGET -t $TARGET_TOOLS -n 3 clean
		exit $?
	fi

	if [[ $ARG == clean ]]; then
		build -p $WORKSPACE/Clover/rEFIt_UEFI/rEFIt.dsc -a $PROCESSOR -b $VTARGET -t $TARGET_TOOLS -n 3 clean
		exit $?
	fi
	build -p $WORKSPACE/Clover/rEFIt_UEFI/rEFIt.dsc -a $PROCESSOR -b $VTARGET -t $TARGET_TOOLS -n 3 $* 
fi
if [ $PROCESSOR = X64 ]; then
# Cleaning part of the script if we have $4 argument: clean/cleanall
	if [[ $ARG == cleanall ]]; then
		make -C $WORKSPACE/BaseTools clean
		build -p $WORKSPACE/Clover/rEFIt_UEFI/rEFIt64.dsc -a $PROCESSOR -b $VTARGET -t $TARGET_TOOLS -n 3 clean
		exit $?
	fi

	if [[ $ARG == clean ]]; then
		build -p $WORKSPACE/Clover/rEFIt_UEFI/rEFIt64.dsc -a $PROCESSOR -b $VTARGET -t $TARGET_TOOLS -n 3 clean
		exit $?
	fi
	build -p $WORKSPACE/Clover/rEFIt_UEFI/rEFIt64.dsc -a $PROCESSOR -b $VTARGET -t $TARGET_TOOLS -n 3 $* 
fi
}

# BUILD START #
echo "$1 $2 $3 $4"
fnMainBuildScript 
cp -r "${WORKDIR}"/src/edk2/Build/rEFIt/RELEASE_$TARGET_TOOLS/${PROCESSOR}/CLOVER$PROCESSOR.efi $WORKSPACE/Clover/CloverPackage/CloverV2/EFI/BOOT
