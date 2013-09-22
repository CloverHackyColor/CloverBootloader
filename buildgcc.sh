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
#                  "Xcode 4.6" - Mountain Lion
#                  "Xcode 5.0" - Mountain Lion
#
#  
# Created by Jadran Puharic on 1/25/12.
# Enhanced by JrCs on 01/19/2013.
#

set -u # exit with error if unbound variables

# Use standard PATH
export PATH=/usr/bin:/bin:/usr/sbin:/sbin

# GCC chainload source version 
# here we can change source versions of tools
#
export BINUTILS_VERSION=${BINUTILS_VERSION:-binutils-2.23.2}
export GCC_VERSION=${GCC_VERSION:-4.8.1}
export GMP_VERSION=${GMP_VERSION:-gmp-5.1.2}
export MPFR_VERSION=${MPFR_VERSION:-mpfr-3.1.2}
export MPC_VERSION=${MPC_VERSION:-mpc-1.0.1}
export ISL_VERSION=${ISL_VERSION:-isl-0.11.1}
export CLOOG_VERSION=${CLOOG_VERSION:-cloog-0.18.0}

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
export GCC_CONFIG="--prefix=$PREFIX --with-sysroot=$PREFIX --disable-werror --with-gmp=$PREFIX --oldincludedir=$PREFIX/include --with-gnu-as --with-gnu-ld --with-cloog=$PREFIX --with-isl=$PREFIX --disable-isl-version-check --with-newlib --verbose --disable-libssp --disable-nls --enable-languages=c,c++"

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

# Here we set MAKEFLAGS for GCC so it knows how many cores can use
# faster compile!
#
export MAKEFLAGS="-j $(sysctl -n hw.ncpu)"

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
echo " Example: ./buildgcc.sh -x64 -all"
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
        dev_ramdisk=`hdiutil attach -nomount ram://1228800 | awk '{print $1}'`
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
    if [[ ! -f ${DIR_DOWNLOADS}/${BINUTILS_VERSION}.tar.bz2 ]]; then
        echo "Status: ${BINUTILS_VERSION} not found."
        curl -f -o download.tmp --remote-name http://mirror.aarnet.edu.au/pub/gnu/binutils/${BINUTILS_VERSION}.tar.bz2 || exit 1
        mv download.tmp ${BINUTILS_VERSION}.tar.bz2
    fi
}

fnDownloadGCC ()
# Function: Download GCC source
{
    cd $DIR_DOWNLOADS
    if [[ ! -f ${DIR_DOWNLOADS}/gcc-${GCC_VERSION}.tar.bz2 ]]; then
        echo "Status: gcc-${GCC_VERSION} not found."
        curl -f -o download.tmp --remote-name http://mirrors.kernel.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.bz2  || exit 1
        mv download.tmp gcc-${GCC_VERSION}.tar.bz2
    fi
}

fnDownloadSource ()
{
    cd $DIR_DOWNLOADS
    if [[ ! -f ${DIR_DOWNLOADS}/${GMP_VERSION}.tar.bz2 ]]; then
        echo "Status: ${GMP_VERSION} not found."
        curl -f -o download.tmp --remote-name http://mirror.aarnet.edu.au/pub/gnu/gmp/${GMP_VERSION}.tar.bz2 || exit 1
        mv download.tmp ${GMP_VERSION}.tar.bz2
    fi

    if [[ ! -f ${DIR_DOWNLOADS}/${MPFR_VERSION}.tar.bz2 ]]; then
        echo "Status: ${MPFR_VERSION} not found."
        curl -f -o download.tmp --remote-name http://mirror.aarnet.edu.au/pub/gnu/mpfr/${MPFR_VERSION}.tar.bz2 || exit 1
        mv download.tmp ${MPFR_VERSION}.tar.bz2
    fi

    if [[ ! -f ${DIR_DOWNLOADS}/${MPC_VERSION}.tar.gz ]]; then
        echo "Status: ${MPC_VERSION} not found."
        curl -f -o download.tmp --remote-name http://www.multiprecision.org/mpc/download/${MPC_VERSION}.tar.gz || exit 1
        mv download.tmp ${MPC_VERSION}.tar.gz
    fi

    if [[ ! -f ${DIR_DOWNLOADS}/${ISL_VERSION}.tar.bz2 ]]; then
        echo "Status: ${ISL_VERSION} not found."
        curl -o download.tmp --remote-name ftp://gcc.gnu.org/pub/gcc/infrastructure/${ISL_VERSION}.tar.bz2 || exit 1
        mv download.tmp ${ISL_VERSION}.tar.bz2
    fi

    if [[ ! -f ${DIR_DOWNLOADS}/${CLOOG_VERSION}.tar.gz ]]; then
        echo "Status: ${CLOOG_VERSION} not found."
        curl -o download.tmp --remote-name ftp://gcc.gnu.org/pub/gcc/infrastructure/${CLOOG_VERSION}.tar.gz || exit 1
        mv download.tmp ${CLOOG_VERSION}.tar.gz
    fi

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

    echo "-  ${package} extract..."
    rm -rf "${DIR_GCC}/$top_level_dir" # Remove old directory if exists
    tar -C "$DIR_GCC" -x "$tar_filter_option" -f "${tarball}" && touch "${DIR_GCC}/$package.extracted"
    echo "-  ${package} extracted"

    # Restore stdout for the result and close file descriptor 3
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
    "${GMP_DIR}"/configure --prefix=$PREFIX > $DIR_LOGS/gmp.$ARCH.configure.log.txt 2> /dev/null
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
    "${MPFR_DIR}"/configure --prefix=$PREFIX --with-gmp=$PREFIX > $DIR_LOGS/mpfr.$ARCH.configure.log.txt 2> /dev/null
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
    "${MPC_DIR}"/configure --prefix=$PREFIX --with-gmp=$PREFIX --with-mpfr=$PREFIX  > $DIR_LOGS/mpc.$ARCH.configure.log.txt 2> /dev/null
    echo "-  ${MPC_VERSION} make..."
    make 1> /dev/null 2> $DIR_LOGS/mpc.$ARCH.make.log.txt
    make install 1> $DIR_LOGS/mpc.$ARCH.install.log.txt 2> /dev/null
    rm -rf "${DIR_BUILD}/$ARCH-mpc"
    echo "-  ${MPC_VERSION} installed in $PREFIX  -"
    
    # Compile ISL
    local ISL_DIR=$(fnExtract "${ISL_VERSION}.tar.bz2")

    rm -rf "${DIR_BUILD}/$ARCH-isl"
    mkdir -p "${DIR_BUILD}/$ARCH-isl" && cd "${DIR_BUILD}/$ARCH-isl"
    echo "-  ${ISL_VERSION} configure..."
    "${ISL_DIR}"/configure --prefix=$PREFIX --with-gmp-prefix=$PREFIX --with-gcc-arch=$ARCH > $DIR_LOGS/isl.$ARCH.configure.log.txt 2> /dev/null
    echo "-  ${ISL_VERSION} make..."
    make 1> /dev/null 2> $DIR_LOGS/isl.$ARCH.make.log.txt
    make install 1> $DIR_LOGS/isl.$ARCH.install.log.txt 2> /dev/null
    rm -rf "${DIR_BUILD}/$ARCH-isl"
    echo "-  ${ISL_VERSION} installed in $PREFIX  -"

    # Compile CLOOG
    local CLOOG_DIR=$(fnExtract "${CLOOG_VERSION}.tar.gz")

    rm -rf "${DIR_BUILD}/$ARCH-cloog"
    mkdir -p "${DIR_BUILD}/$ARCH-cloog" && cd "${DIR_BUILD}/$ARCH-cloog"
    echo "-  ${CLOOG_VERSION} configure..."
    "${CLOOG_DIR}"/configure --prefix=$PREFIX --with-gmp-prefix=$PREFIX --with-isl-prefix=$PREFIX --with-gcc-arch=$ARCH  > $DIR_LOGS/cloog.$ARCH.configure.log.txt 2> /dev/null
    echo "-  ${CLOOG_VERSION} make..."
    make 1> /dev/null 2> $DIR_LOGS/cloog.$ARCH.make.log.txt
    make install 1> $DIR_LOGS/cloog.$ARCH.install.log.txt 2> /dev/null
    rm -rf "${DIR_BUILD}/$ARCH-cloog"
    echo "-  ${CLOOG_VERSION} installed in $PREFIX  -"
        
}

fnCompileBinutils ()
# Function: Compiling Binutils in PREFIX location
{
    # Check GMP/MPFR/MPC
    [ ! -f $PREFIX/include/gmp.h ]  && echo "Error: ${GMP_VERSION} not installed, check logs"  && exit
    [ ! -f $PREFIX/include/mpfr.h ] && echo "Error: ${MPFR_VERSION} not installed, check logs" && exit
    [ ! -f $PREFIX/include/mpc.h ]  && echo "Error: ${MPC_VERSION} not installed, check logs"  && exit

    # Mount RamDisk
    mountRamDisk

    export BUILD_BINUTILS_DIR=${DIR_BUILD}/$ARCH-binutils

    # Extract the tarball
    local BINUTILS_DIR=$(fnExtract "${BINUTILS_VERSION}.tar.bz2")
    # echo "-  ${BINUTILS_VERSION} patch..."
    # cp "${DIR_MAIN}"/edk2/Clover/Patches_to_compilers/"${BINUTILS_VERSION}"/binutils/readelf-new.c "${BINUTILS_DIR}"/binutils/readelf.c || exit 1
    # cp "${DIR_MAIN}"/edk2/Clover/Patches_to_compilers/"${BINUTILS_VERSION}"/opcodes/i386-dis-new.c "${BINUTILS_DIR}"/opcodes/i386-dis.c || exit 1
    
    # Binutils build
    rm -rf "$BUILD_BINUTILS_DIR"
    mkdir -p "$BUILD_BINUTILS_DIR" && cd "$BUILD_BINUTILS_DIR"
    echo "-  ${BINUTILS_VERSION} configure..."
    local cmd="${BINUTILS_DIR}/configure --target=$TARGET $BINUTILS_CONFIG"
    local logfile="$DIR_LOGS/binutils.$ARCH.configure.log.txt"
    echo "$cmd" > "$logfile"
    eval "$cmd" >> "$logfile" 2>&1
    echo "-  ${BINUTILS_VERSION} make..."
    cmd="make all"
    logfile="$DIR_LOGS/binutils.$ARCH.make.log.txt"
    echo "$cmd" > "$logfile"
    eval "$cmd" >/dev/null 2>> "$logfile"
    if [[ $? -ne 0 ]]; then
        echo "Error compiling ${BINUTILS_VERSION} ! Check the log $logfile"
        exit 1
    fi
    cmd="make install"
    logfile="$DIR_LOGS/binutils.$ARCH.install.log.txt"
    echo "$cmd" > "$logfile"
    eval "$cmd" >> "$logfile" 2>&1
    if [[ $? -ne 0 ]]; then
        echo "Error installing ${BINUTILS_VERSION} ! Check the log $logfile"
        exit 1
    fi

    rm -rf "$BUILD_BINUTILS_DIR"
    [ ! -f $PREFIX/bin/$TARGET-ld ] && echo "Error: ${BINUTILS_VERSION} not installed, check logs in $DIR_LOGS" && exit 1
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
    "${GCC_DIR}"/configure --target=$TARGET $GCC_CONFIG > $DIR_LOGS/gcc.$ARCH.configure.log.txt 2> /dev/null

    echo "-  gcc-${GCC_VERSION} make..."
    make all-gcc 1> /dev/null 2> $DIR_LOGS/gcc.$ARCH.make.log.txt
    make install-gcc 1> $DIR_LOGS/gcc.$ARCH.install.log.txt 2> /dev/null

    rm -rf "${DIR_BUILD}/$ARCH-gcc"

    [ ! -x $PREFIX/bin/$TARGET-gcc ] && echo "Error: gcc-${GCC_VERSION} not installed, check logs in $DIR_LOGS" && exit 1
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
    fnDownloadSource || exit 1
    fnCompileLibs    || exit 1
    fnBinutils       || exit 1
    fnGCC            || exit 1
    fnMakeSymLinks   || exit 1
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
case "${1:-}" in
    '')      fnHelp && exit ;;
    '-help') fnHelp && exit ;;
    '-ia32') fnArchIA32 ;;
    '-x64')  fnArchX64  ;;
    *)
	   echo $"Error!"
	   echo $"Usage: {ia32|x64|help}"
	   exit 1
	   ;;
esac

# 2. Argument Case
case "$2" in
    '')
        echo "Example: ./buildgcc.sh -ia32 -all" && exit ;;
    '-binutils')
        fnBinutils ;;
    '-gcc')
        fnGCC ;;
    '-all')
        fnALL ;;
    *)
       echo $"Error!"
       echo $"Usage: {binutils|gcc|all}"
       exit 1
esac
