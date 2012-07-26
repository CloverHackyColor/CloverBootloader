#!/bin/sh

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
#
#  
# Created by Jadran Puharic on 1/25/12.
# 
# 

# GCC chainload source version 
# here we can change source versions of tools
#
export BINUTILS_VERSION=binutils-2.22
export GCC_VERSION=4.6.3
export GMP_VERSION=gmp-5.0.5
export MPFR_VERSION=mpfr-3.1.0
export MPC_VERSION=mpc-0.9

# Change PREFIX if you want gcc and binutils 
# installed on different place
#
export PREFIX=~/src/opt/local

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
export DIR_MAIN=~/src
export DIR_TOOLS=$DIR_MAIN/tools/
export DIR_GCC=$DIR_MAIN/tools/gcc 
export DIR_DOWNLOADS=$DIR_GCC/download
export DIR_LOGS=$DIR_GCC/logs

## Paths for GCC (Xcode 4.1 fix) - works with Xcode 3.2 - Xcode 4.2
#
export CC="/usr/bin/gcc-4.2"
export CXX="/usr/bin/g++-4.2"
export CPP="/usr/bin/cpp-4.2"
export LD="/usr/bin/ld" 

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

[ ! -d ${DIR_MAIN} ] && mkdir ${DIR_MAIN}
[ ! -d ${DIR_TOOLS} ] && mkdir ${DIR_TOOLS}
[ ! -d ${DIR_GCC} ] && mkdir ${DIR_GCC}
[ ! -d ${DIR_DOWNLOADS} ] && mkdir ${DIR_DOWNLOADS}
[ ! -d ${DIR_LOGS} ] && mkdir ${DIR_LOGS}
[ ! -d ${PREFIX}/include ] && mkdir ${PREFIX}/include
echo


# Download #

fnDownloadBinutils ()
# Function: Download Binutils source
{
    cd $DIR_DOWNLOADS
    [ ! -f ${DIR_DOWNLOADS}/${BINUTILS_VERSION}.tar.bz2 ] && echo "Status: binutils not found." && curl --remote-name ftp://ftp.gnu.org/gnu/binutils/${BINUTILS_VERSION}.tar.bz2
}

fnDownloadGCC ()
# Function: Download GCC source
{
    cd $DIR_DOWNLOADS
    [ ! -f ${DIR_DOWNLOADS}/gcc-core-${GCC_VERSION}.tar.bz2 ] && echo "Status: gcc-core not found." && curl --remote-name ftp://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-core-${GCC_VERSION}.tar.bz2
    [ ! -f ${DIR_DOWNLOADS}/gcc-g++-${GCC_VERSION}.tar.bz2 ] && echo "Status: gcc-g++ not found." && curl --remote-name ftp://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-g++-${GCC_VERSION}.tar.bz2
}

fnDownloadSource ()
{
    cd $DIR_DOWNLOADS
    [ ! -f ${DIR_DOWNLOADS}/${GMP_VERSION}.tar.bz2 ] && echo "Status: gmp not found." && curl --remote-name ftp://ftp.gnu.org/gnu/gmp/${GMP_VERSION}.tar.bz2
    [ ! -f ${DIR_DOWNLOADS}/${MPFR_VERSION}.tar.bz2 ] && echo "Status: mpfr not found." && curl --remote-name ftp://ftp.gnu.org/gnu/mpfr/${MPFR_VERSION}.tar.bz2
    [ ! -f ${DIR_DOWNLOADS}/${MPC_VERSION}.tar.gz ] && echo "Status: mpc not found." && curl --remote-name http://www.multiprecision.org/mpc/download/${MPC_VERSION}.tar.gz
    fnDownloadBinutils
    fnDownloadGCC
}


### Compile ###

fnCompileLibs ()
# Function: Compiling GMP/MPFR/MPC in PREFIX location
{
# Compile GMP
    cd $DIR_DOWNLOADS
    echo
    [ ! -f $DIR_DOWNLOADS/$GMP_VERSION.tar.bz2.extracted ] && echo "-  $GMP_VERSION extract..." && tar -xf $GMP_VERSION.tar.bz2 > $GMP_VERSION.tar.bz2.extracted
    echo "-  $GMP_VERSION extracted"
    [ ! -d ${DIR_GCC}/$ARCH-gmp ] && mkdir ${DIR_GCC}/$ARCH-gmp 
    [ -d ${DIR_GCC}/$ARCH-gmp ] && cd ${DIR_GCC}/$ARCH-gmp && rm -rf * 
    echo "-  $GMP_VERSION configure..."
    ../download/$GMP_VERSION/configure --prefix=$PREFIX > $DIR_LOGS/gmp.$ARCH.config.log.txt 2> /dev/null
    echo "-  $GMP_VERSION make..."
    make 1> /dev/null 2> $DIR_LOGS/gmp.$ARCH.make.log.txt
    make install 1> $DIR_LOGS/gmp.$ARCH.install.log.txt 2> /dev/null
    echo "-  $GMP_VERSION installed in $PREFIX  -"

# Compile MPFR
    cd $DIR_DOWNLOADS
    echo
    [ ! -f $DIR_DOWNLOADS/$MPFR_VERSION.tar.bz2.extracted ] && echo "-  $MPFR_VERSION extract..." && tar -xf $MPFR_VERSION.tar.bz2 > $MPFR_VERSION.tar.bz2.extracted
    echo "-  $MPFR_VERSION extracted"
    [ ! -d ${DIR_GCC}/$ARCH-mpfr ] && mkdir ${DIR_GCC}/$ARCH-mpfr 
    [ -d ${DIR_GCC}/$ARCH-mpfr ] && cd ${DIR_GCC}/$ARCH-mpfr && rm -rf * 
    echo "-  $MPFR_VERSION configure..."
    ../download/$MPFR_VERSION/configure --prefix=$PREFIX --with-gmp=$PREFIX > $DIR_LOGS/mpfr.$ARCH.config.log.txt 2> /dev/null
    echo "-  $MPFR_VERSION make..."
    make 1> /dev/null 2> $DIR_LOGS/mpfr.$ARCH.make.log.txt
    make install 1> $DIR_LOGS/mpfr.$ARCH.install.log.txt 2> /dev/null
    echo "-  $MPFR_VERSION installed in $PREFIX  -"

# Compile MPC
    cd $DIR_DOWNLOADS
    echo
    [ ! -f $DIR_DOWNLOADS/$MPC_VERSION.tar.gz.extracted ] && echo "-  $MPC_VERSION extract..." && tar -xf $MPC_VERSION.tar.gz > $MPC_VERSION.tar.gz.extracted
    echo "-  $MPC_VERSION extracted"
    [ ! -d ${DIR_GCC}/$ARCH-mpc ] && mkdir ${DIR_GCC}/$ARCH-mpc 
    [ -d ${DIR_GCC}/$ARCH-mpc ] && cd ${DIR_GCC}/$ARCH-mpc && rm -rf * 
    echo "-  $MPC_VERSION configure..."
    ../download/$MPC_VERSION/configure --prefix=$PREFIX --with-gmp=$PREFIX --with-mpfr=$PREFIX  > $DIR_LOGS/mpc.$ARCH.config.log.txt 2> /dev/null
    echo "-  $MPC_VERSION make..."
    make 1> /dev/null 2> $DIR_LOGS/mpc.$ARCH.make.log.txt
    make install 1> $DIR_LOGS/mpc.$ARCH.install.log.txt 2> /dev/null
    echo "-  $MPC_VERSION installed in $PREFIX  -"
}

fnCompileBinutils ()
# Function: Compiling Binutils in PREFIX location
{
    export BUILD_BINUTILS_DIR=$DIR_GCC/$ARCH-binutils
    cd $DIR_DOWNLOADS
    echo
    [ ! -f $DIR_DOWNLOADS/$BINUTILS_VERSION.tar.bz2.extracted ] && echo "-  $BINUTILS_VERSION extract" && tar -xf $BINUTILS_VERSION.tar.bz2 > $BINUTILS_VERSION.tar.bz2.extracted
    echo "-  $BINUTILS_VERSION extracted"
    
    # Check GMP/MPFR/MPC
    [ ! -f $PREFIX/include/gmp.h ] && echo "Error: $GMP_VERSION not installed, check logs" && exit
    [ ! -f $PREFIX/include/mpfr.h ] && echo "Error: $MPFR_VERSION not installed, check logs" && exit
    [ ! -f $PREFIX/include/mpc.h ] && echo "Error: $MPC_VERSION not installed, check logs" && exit

    # Binutils build
    [ ! -d ${DIR_GCC}/$ARCH-binutils ] && mkdir ${DIR_GCC}/$ARCH-binutils 
    [ -d ${DIR_GCC}/$ARCH-binutils ] && cd ${DIR_GCC}/$ARCH-binutils && rm -rf * 
    echo "-  $BINUTILS_VERSION configure..."
    ../download/$BINUTILS_VERSION/configure --target=$TARGET $BINUTILS_CONFIG 1> $DIR_LOGS/binutils.$ARCH.config.log.txt 2> /dev/null
    echo "-  $BINUTILS_VERSION make..."
    make all 1> /dev/null 2> $DIR_LOGS/binutils.$ARCH.make.log.txt
    make install 1> $DIR_LOGS/binutils.$ARCH.install.log.txt 2> /dev/null
    [ ! -f $PREFIX/bin/$TARGET-ld ] && echo "Error: $BINUTILS_VERSION not installed, check logs" && exit
    echo "-  $BINUTILS_VERSION installed in $PREFIX  -"
}

fnCompileGCC ()
# Function: Compiling GCC in PREFIX location
{
    export PATH=$PATH:$PREFIX/bin
    cd $DIR_DOWNLOADS
    echo
    [ ! -f $DIR_DOWNLOADS/gcc-core-${GCC_VERSION}.tar.bz2.extracted ] && echo "-  gcc-core-$GCC_VERSION extract..." && tar -xf gcc-core-${GCC_VERSION}.tar.bz2 > gcc-core-${GCC_VERSION}.tar.bz2.extracted
    [ ! -f $DIR_DOWNLOADS/gcc-g++-${GCC_VERSION}.tar.bz2.extracted ] && echo "-  gcc-g++-$GCC_VERSION extract..." && tar -xf gcc-g++-${GCC_VERSION}.tar.bz2 > gcc-g++-${GCC_VERSION}.tar.bz2.extracted 
    echo "-  gcc-$GCC_VERSION extracted"
    [ ! -d ${DIR_GCC}/$ARCH-gcc ] && mkdir ${DIR_GCC}/$ARCH-gcc 
    [ -d ${DIR_GCC}/$ARCH-gcc ] && cd ${DIR_GCC}/$ARCH-gcc && rm -rf * 
    echo "-  gcc-$GCC_VERSION configure..."
    ../download/gcc-$GCC_VERSION/configure --target=$TARGET $GCC_CONFIG > $DIR_LOGS/gcc.$ARCH.config.log.txt 2> /dev/null
    echo "-  gcc-$GCC_VERSION make..."
    make all-gcc 1> /dev/null 2> $DIR_LOGS/gcc.$ARCH.make.log.txt
    make install-gcc 1> $DIR_LOGS/gcc.$ARCH.install.log.txt 2> /dev/null
    [ ! -f $PREFIX/bin/$TARGET-gcc ] && echo "Error: gcc-$GCC_VERSION not installed, check logs" && exit
    echo "-  gcc-$GCC_VERSION installed in $PREFIX  -"  
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
    export TARGET=$TARGET_IA32
    echo "-  Building GCC chainload for $TARGET_IA32  -"
    export ARCH="ia32"
    export ABI_VER="32"
}

fnArchX64 ()
# Function: setting arch type x64
{
    export TARGET=$TARGET_X64
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