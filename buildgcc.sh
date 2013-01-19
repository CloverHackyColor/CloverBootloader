#!/bin/bash

# Script for GCC chainload in OS X made for EDKII
# 
# Primary use is for creating better crosscompile support than 
# mingw-gcc-build.py that is found in BaseTools/gcc/
# 
# With this we can use Native GCC chainload for EDKII  
# development
#
# Xcode Tools are required
# Script tested on "Xcode 3.2" - Snow Leopard  
#                  "Xcode 4.1" - Lion
#                  "Xcode 4.5" - Mountain Lion
#
#  
# Created by Jadran Puharic on 1/25/12.
# Enhanced by JrCs on 01/19/2013.
#

# GCC chainload source version 
# here we can change source versions of tools
#
export BINUTILS_VERSION=binutils-2.23.1
export GCC_VERSION=${GCC_VERSION:-4.7.2}
export GMP_VERSION=gmp-5.0.5
export MPFR_VERSION=mpfr-3.1.1
export MPC_VERSION=mpc-0.9

# Change PREFIX if you want gcc and binutils 
# installed on different place
#
export PREFIX=${PREFIX:-~/src/opt/local}

# Change target mode of crosscompiler for
# IA32 and X64 - (we know that this one works best)
# 
export TARGET_IA32="i686-linux-gnu"
export TARGET_X64="x86_64-linux-gnu"

# ./configure arguments for GCC
# 
export GCC_CONFIG="--prefix=$PREFIX --with-sysroot=$PREFIX --disable-werror --with-gmp=$PREFIX --oldincludedir=$PREFIX/include --with-gnu-as --with-gnu-ld --with-newlib --verbose --disable-libssp --disable-nls --enable-languages=c,c++"

# ./configure arguments for Binutils
#
export BINUTILS_CONFIG="--prefix=$PREFIX  --with-sysroot=$PREFIX --disable-werror --with-gmp=$PREFIX --with-mpfr=$PREFIX --with-mpc=$PREFIX"

# You can change DIR_MAIN if u wan't gcc source downloaded 
# in different folder. 
#
export RAMDISK_MNT_PT=/tmp/buildgcc-ramdisk
export DIR_MAIN=${DIR_MAIN:-~/src}
export DIR_TOOLS=${DIR_TOOLS:-$DIR_MAIN/tools}
export DIR_GCC=${DIR_GCC:-$DIR_TOOLS/gcc}
export DIR_BUILD=${DIR_BUILD:-$RAMDISK_MNT_PT}
export DIR_DOWNLOADS=${DIR_DOWNLOADS:-$DIR_GCC/download}
export DIR_LOGS=${DIR_LOGS:-$DIR_GCC/logs}

## Paths for GCC (Xcode 4.1 fix) - works with Xcode 3.2 - Xcode 4.2
#
export CC="/usr/bin/gcc"
export CXX="/usr/bin/g++"
export CPP="/usr/bin/cpp"
export LD="/usr/bin/ld"

#export CC="/opt/local/bin/gcc-apple-4.2"
#export CXX="/opt/local/bin/g++-apple-4.2"
#export CPP="/opt/local/bin/cpp-apple-4.2"
#export LD="/opt/local/bin/ld"

# Here we set MAKEFLAGS for GCC so it knows how many cores can use
# faster compile!
#
export MAKEFLAGS="-j `sysctl -n hw.ncpu`"


### Check Functions ###

fnCheckXcode ()
# Function: checking installation of Xcode Tools
{
[ ! -f /usr/bin/xcodebuild ] && \
echo "ERROR: Install Xcode Tools from Apple before using this script." && \
exit
}

fnHelp ()
# Function: Help
{
echo 
echo " Script for building GCC chainload on Darwin OS X"
echo
echo "   Usage: ./buildgcc.sh [ARCH] [TOOL]"
echo 
echo "   [ARCH]     [TOOL]"
echo "   -ia32      -binutils"
echo "   -x64       -gcc"
echo "              -all"
echo
echo " Example: ./buildgcc.sh -ia32 -all"
echo
}


### Main Function START ### 

# Function: Creating directory structure for EDK

[ ! -d ${DIR_MAIN} ]       && mkdir ${DIR_MAIN}
[ ! -d ${DIR_TOOLS} ]      && mkdir ${DIR_TOOLS}
[ ! -d ${DIR_GCC} ]        && mkdir ${DIR_GCC}
[ ! -d ${DIR_DOWNLOADS} ]  && mkdir ${DIR_DOWNLOADS}
[ ! -d ${DIR_LOGS} ]       && mkdir ${DIR_LOGS}
[ ! -d ${PREFIX}/include ] && mkdir -p ${PREFIX}/include
echo


# RAMdisk
function mountRamDisk() {
    dev_ramdisk=$(mount | grep "$RAMDISK_MNT_PT" | awk '{print $1}')
    if [ -z "$dev_ramdisk" ];then
        echo "- Creating new RAM disk"
        dev_ramdisk=`hdiutil attach -nomount ram://614400 | awk '{print $1}'`
        echo
        [ -n "$dev_ramdisk" ] && newfs_hfs -v "BuildGCC RamDisk" "$dev_ramdisk"
        [ ! -d "$RAMDISK_MNT_PT" ] && mkdir "$RAMDISK_MNT_PT"
        mount -t hfs "$dev_ramdisk" "$RAMDISK_MNT_PT"

    fi
    # Automatically remove RAMDISK on exit
    trap 'echo; echo "- Ejecting RAM disk"; cd "$HOME"; umount "$RAMDISK_MNT_PT" && hdiutil detach "$dev_ramdisk"' EXIT
}

# Download #

fnDownloadBinutils ()
# Function: Download Binutils source
{
    cd $DIR_DOWNLOADS
    [ ! -f ${DIR_DOWNLOADS}/${BINUTILS_VERSION}.tar.bz2 ] && echo "Status: ${BINUTILS_VERSION} not found." && curl --remote-name http://mirror.aarnet.edu.au/pub/gnu/binutils/${BINUTILS_VERSION}.tar.bz2
}

fnDownloadGCC ()
# Function: Download GCC source
{
    cd $DIR_DOWNLOADS
#    [ ! -f ${DIR_DOWNLOADS}/gcc-core-${GCC_VERSION}.tar.bz2 ] && echo "Status: gcc-core not found." && curl --remote-name ftp://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-core-${GCC_VERSION}.tar.bz2
#    [ ! -f ${DIR_DOWNLOADS}/gcc-g++-${GCC_VERSION}.tar.bz2 ] && echo "Status: gcc-g++ not found." && curl --remote-name ftp://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-g++-${GCC_VERSION}.tar.bz2
    [ ! -f ${DIR_DOWNLOADS}/gcc-${GCC_VERSION}.tar.bz2 ] && echo "Status: gcc-${GCC_VERSION} not found." && curl --remote-name http://mirrors.kernel.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.bz2

}

fnDownloadSource ()
{
    cd $DIR_DOWNLOADS
    [ ! -f ${DIR_DOWNLOADS}/${GMP_VERSION}.tar.bz2 ] && echo "Status: ${GMP_VERSION} not found." && curl --remote-name http://mirror.aarnet.edu.au/pub/gnu/gmp//${GMP_VERSION}.tar.bz2
    [ ! -f ${DIR_DOWNLOADS}/${MPFR_VERSION}.tar.bz2 ] && echo "Status: ${MPFR_VERSION} not found." && curl --remote-name http://mirror.aarnet.edu.au/pub/gnu/mpfr/${MPFR_VERSION}.tar.bz2
    [ ! -f ${DIR_DOWNLOADS}/${MPC_VERSION}.tar.gz ] && echo "Status: ${MPC_VERSION} not found." && curl --remote-name http://www.multiprecision.org/mpc/download/${MPC_VERSION}.tar.gz
    fnDownloadBinutils
    fnDownloadGCC
}


### Extract ###

# Function to extract source tarballs
fnExtract ()
{
    exec 3>&1 1>&2 # Save stdout and redirect stdout to stderr

    local tarball="$1"
    local package=${tarball%%.tar*}
    tarball="${DIR_DOWNLOADS}/$tarball"
    local flagfile="${DIR_GCC}/$package.extracted"

    local filetype=$(file -L --brief "$tarball" | tr '[A-Z]' '[a-z]')
    local tar_filter_option=""

    case ${filetype} in # convert to lowercase
        gzip\ *)  tar_filter_option='--gzip' ;;
        bzip2\ *) tar_filter_option='--bzip2';;
        lzip\ *)  tar_filter_option='--lzip' ;;
        lzop\ *)  tar_filter_option='--lzop' ;;
        lzma\ *)  tar_filter_option='--lzma' ;;
        xz\ *)    tar_filter_option='--xz'   ;;
        *tar\ archive*) tar_filter_option='';;
        *) echo "Unrecognized file format of '$tarball'"
           exit 1
           ;;
    esac

    # Get the root directory from the tarball
    local first_line=$(dd if="$tarball" bs=1024 count=256 2>/dev/null | \
        tar -t $tar_filter_option -f - 2>/dev/null | head -1)
    local top_level_dir=${first_line#./}  # remove leading ./
    top_level_dir=${top_level_dir%%/*}    # keep only the top level directory

    [ -z "$top_level_dir" ] && echo "Error can't extract top level dir from $tarball" && exit 1

    if [[ ! -d "${DIR_GCC}/$top_level_dir" || ! -f "$flagfile" ]]; then
        echo "-  ${package} extract..."
        rm -rf "${DIR_GCC}/$top_level_dir" # Remove old directory if exists
        tar -C "$DIR_GCC" -x "$tar_filter_option" -f "${tarball}" && touch "${DIR_GCC}/$package.extracted"
        echo "-  ${package} extracted"

    fi

    # Restore stdout for the result and close file desciptor 3
    exec 1>&3-
    echo "${DIR_GCC}/$top_level_dir" # Return the full path where the tarball has been extracted
}

### Compile ###

fnCompileLibs ()
# Function: Compiling GMP/MPFR/MPC in PREFIX location
{
    # Mount RamDisk
    mountRamDisk

    # Compile GMP
    local GMP_DIR=$(fnExtract "${GMP_VERSION}.tar.bz2")

    rm -rf "${DIR_BUILD}/$ARCH-gmp"
    mkdir -p "${DIR_BUILD}/$ARCH-gmp" && cd "${DIR_BUILD}/$ARCH-gmp"
    echo "-  ${GMP_VERSION} configure..."
    "${GMP_DIR}"/configure --prefix=$PREFIX > $DIR_LOGS/gmp.$ARCH.config.log.txt 2> /dev/null
    echo "-  ${GMP_VERSION} make..."
    make 1> /dev/null 2> $DIR_LOGS/gmp.$ARCH.make.log.txt
    make install 1> $DIR_LOGS/gmp.$ARCH.install.log.txt 2> /dev/null
    rm -rf "${DIR_BUILD}/$ARCH-gmp"
    echo "-  ${GMP_VERSION} installed in $PREFIX  -"

    # Compile MPFR
    local MPFR_DIR=$(fnExtract "${MPFR_VERSION}.tar.bz2")

    rm -rf "${DIR_BUILD}/$ARCH-mpfr"
    mkdir -p "${DIR_BUILD}/$ARCH-mpfr" && cd "${DIR_BUILD}/$ARCH-mpfr"
    echo "-  ${MPFR_VERSION} configure..."
    "${MPFR_DIR}"/configure --prefix=$PREFIX --with-gmp=$PREFIX > $DIR_LOGS/mpfr.$ARCH.config.log.txt 2> /dev/null
    echo "-  ${MPFR_VERSION} make..."
    make 1> /dev/null 2> $DIR_LOGS/mpfr.$ARCH.make.log.txt
    make install 1> $DIR_LOGS/mpfr.$ARCH.install.log.txt 2> /dev/null
    rm -rf "${DIR_BUILD}/$ARCH-mpfr"
    echo "-  ${MPFR_VERSION} installed in $PREFIX  -"

    # Compile MPC
    local MPC_DIR=$(fnExtract "${MPC_VERSION}.tar.gz")

    rm -rf "${DIR_BUILD}/$ARCH-mpc"
    mkdir -p "${DIR_BUILD}/$ARCH-mpc" && cd "${DIR_BUILD}/$ARCH-mpc"
    echo "-  ${MPC_VERSION} configure..."
    "${MPC_DIR}"/configure --prefix=$PREFIX --with-gmp=$PREFIX --with-mpfr=$PREFIX  > $DIR_LOGS/mpc.$ARCH.config.log.txt 2> /dev/null
    echo "-  ${MPC_VERSION} make..."
    make 1> /dev/null 2> $DIR_LOGS/mpc.$ARCH.make.log.txt
    make install 1> $DIR_LOGS/mpc.$ARCH.install.log.txt 2> /dev/null
    rm -rf "${DIR_BUILD}/$ARCH-mpc"
    echo "-  ${MPC_VERSION} installed in $PREFIX  -"
}

fnCompileBinutils ()
# Function: Compiling Binutils in PREFIX location
{
    # Mount RamDisk
    mountRamDisk

    export BUILD_BINUTILS_DIR=$DIR_BUILD/$ARCH-binutils

    # Extract the tarball
    local BINUTILS_DIR=$(fnExtract "${BINUTILS_VERSION}.tar.bz2")
    
    # Check GMP/MPFR/MPC
    [ ! -f $PREFIX/include/gmp.h ]  && echo "Error: ${GMP_VERSION} not installed, check logs"  && exit
    [ ! -f $PREFIX/include/mpfr.h ] && echo "Error: ${MPFR_VERSION} not installed, check logs" && exit
    [ ! -f $PREFIX/include/mpc.h ]  && echo "Error: ${MPC_VERSION} not installed, check logs"  && exit

    # Binutils build
    rm -rf "${DIR_BUILD}/$ARCH-binutils"
    mkdir -p "${DIR_BUILD}/$ARCH-binutils" && cd "${DIR_BUILD}/$ARCH-binutils"
    echo "-  ${BINUTILS_VERSION} configure..."
    "${BINUTILS_DIR}"/configure --target=$TARGET $BINUTILS_CONFIG > $DIR_LOGS/binutils.$ARCH.config.log.txt 2> /dev/null
    echo "-  ${BINUTILS_VERSION} make..."
    make all 1> /dev/null 2> $DIR_LOGS/binutils.$ARCH.make.log.txt
    make install 1> $DIR_LOGS/binutils.$ARCH.install.log.txt 2> /dev/null
    rm -rf "${DIR_BUILD}/$ARCH-binutils"
    [ ! -f $PREFIX/bin/$TARGET-ld ] && echo "Error: binutils-${BINUTILS_VERSION} not installed, check logs" && exit
    echo "-  ${BINUTILS_VERSION} installed in $PREFIX  -"
}

fnCompileGCC ()
# Function: Compiling GCC in PREFIX location
{
    # Mount RamDisk
    mountRamDisk

    export PATH=$PATH:$PREFIX/bin

    # Extract the tarball
    local GCC_DIR=$(fnExtract "gcc-${GCC_VERSION}.tar.bz2")

    rm -rf "${DIR_BUILD}/$ARCH-gcc"
    mkdir -p "${DIR_BUILD}/$ARCH-gcc" && cd "${DIR_BUILD}/$ARCH-gcc"

    echo "-  gcc-${GCC_VERSION} configure..."
    "${GCC_DIR}"/configure --target=$TARGET $GCC_CONFIG > $DIR_LOGS/gcc.$ARCH.config.log.txt 2> /dev/null

    echo "-  gcc-${GCC_VERSION} make..."
    make all-gcc 1> /dev/null 2> $DIR_LOGS/gcc.$ARCH.make.log.txt
    make install-gcc 1> $DIR_LOGS/gcc.$ARCH.install.log.txt 2> /dev/null

    rm -rf "${DIR_BUILD}/$ARCH-gcc"

    [ ! -x $PREFIX/bin/$TARGET-gcc ] && echo "Error: gcc-${GCC_VERSION} not installed, check logs" && exit
    echo "-  gcc-${GCC_VERSION} installed in $PREFIX  -"  
    echo
}

fnMakeSymLinks ()
# Function: SymLinks in PREFIX location
{
    [ ! -d ${PREFIX}/$ARCH ] && mkdir ${PREFIX}/$ARCH
    cd $PREFIX/$ARCH
    ln -s $PREFIX/bin/$TARGET-gcc $PREFIX/$ARCH/gcc 2> /dev/null 
    ln -s $PREFIX/bin/$TARGET-ld $PREFIX/$ARCH/ld 2> /dev/null 
    ln -s $PREFIX/bin/$TARGET-objcopy $PREFIX/$ARCH/objcopy 2> /dev/null 
    ln -s $PREFIX/bin/$TARGET-ar $PREFIX/$ARCH/ar 2> /dev/null 
    echo "Finished: symlinks are in: "$PREFIX/$ARCH
}

### ARGUMENTS fnFunctions ### 

fnBinutils ()
# Function: Binutils main script
{
    fnDownloadBinutils
    fnCompileBinutils
}

fnGCC ()
# Functions: GCC main script
{	
    fnDownloadGCC
    fnCompileGCC
}

fnALL ()
# Functions: Build all source
{
    fnDownloadSource
    fnCompileLibs
    fnBinutils
    fnGCC
    fnMakeSymLinks
}

fnArchIA32 ()
# Function: setting arch type ia32
{
    export TARGET="$TARGET_IA32"
    echo "-  Building GCC chainload for $TARGET_IA32  -"
    export ARCH="ia32"
    export ABI_VER="32"
}

fnArchX64 ()
# Function: setting arch type x64
{
    export TARGET="$TARGET_X64"
    echo "-  Building GCC chainload for $TARGET_X64  -"
    export ARCH="x64"
    export ABI_VER="64"
}

# 1. Argument ARCH
case "$1" in
'')
fnHelp && exit
;;
'-help')
fnHelp && exit
;;
'-ia32')
fnArchIA32
;;
'-x64')
fnArchX64
;;
*)
echo $"Error!"
echo $"Usage: {ia32|x64|help}"
exit 1
esac

# 2. Argument Case
case "$2" in
'')
echo "Example: ./buildgcc.sh -ia32 -all" && exit
;;
'-binutils')
fnBinutils
;;
'-gcc')
fnGCC
;;
'-all')
fnALL
;;
*)
echo $"Error!"
echo $"Usage: {binutils|gcc|all}"
exit 1
esac
