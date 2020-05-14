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
# Script tested on "Xcode 3.2.2" - Snow Leopard
#                  "Xcode 4.2"   - Snow Leopard
#                  "Xcode 4.6"   - Lion
#                  "Xcode 4.6"   - Mountain Lion
#                  "Xcode 5.0"   - Mountain Lion
#                  "Xcode 5.0.1" - Mavericks
#                  "Xcode 6.1"   - Yosemite
#
#
# Created by Jadran Puharic on 1/25/12.
# Enhanced by JrCs on 01/19/2013.
#

set -u # exit with error if unbound variables

# GCC toolchain source version
# here we can change source versions of tools
#
export BINUTILS_VERSION=${BINUTILS_VERSION:-binutils-2.34}
export GCC_VERSION=${GCC_VERSION:-10.1.0}

# Version of libraries are from ./contrib/download_prerequisites in gcc source directory
export GMP_VERSION=${GMP_VERSION:-gmp-6.2.0}
export MPFR_VERSION=${MPFR_VERSION:-mpfr-4.0.2}
export MPC_VERSION=${MPC_VERSION:-mpc-1.1.0}
export ISL_VERSION=${ISL_VERSION:-isl-0.22.1}

# Change PREFIX if you want gcc and binutils
# installed on different place
#
export PREFIX=${PREFIX:-~/src/opt/local}

export GCC_MAJOR_VERSION=$(echo $GCC_VERSION | awk -F. '{ print $1$2}')

# You can change DIR_MAIN if u wan't gcc source downloaded
# in different folder.
#
export RAMDISK_MNT_PT=/tmp/buildgcc-ramdisk
export DIR_MAIN=${DIR_MAIN:-~/src}
export DIR_TOOLS=${DIR_TOOLS:-$DIR_MAIN/tools}
export DIR_GCC=${DIR_GCC:-$DIR_TOOLS/gcc}
export DIR_BUILD=${DIR_BUILD:-$RAMDISK_MNT_PT}
export DIR_DOWNLOADS=${DIR_DOWNLOADS:-$DIR_TOOLS/download}
export DIR_LOGS=${DIR_LOGS:-$DIR_TOOLS/logs}

# Set MAKE and LD to prevent problem during compilation when Xcode path has spaces
#
export MAKE=make
export LD=ld

# Here we set MAKEFLAGS for GCC so it knows how many cores we can use
# faster compile !
export MAKEFLAGS="-j $(( $(sysctl -n hw.ncpu) * 2 ))"

# Default locale
export LC_ALL=POSIX

# Use standard PATH
export PATH="/usr/bin:/bin:/usr/sbin:/sbin"

### Check Functions ###

# Function: checking installation of Xcode Tools
CheckXCode () {
    local OSXVER="`/usr/bin/sw_vers -productVersion | cut -d '.' -f1,2`"
    local OSXARCH="`/usr/bin/uname -m`"
    local OSXREV="`/usr/bin/uname -r`"
    export BUILDVER="$OSXVER"
    export BUILDARCH="$OSXARCH"
    export BUILDREV="$OSXREV"
    echo "  Running on Mac OS X ${OSXVER}, with ${OSXARCH} architecture."
    if [[ ! -x /usr/bin/xcrun ]]; then
        echo "ERROR: Install Xcode Tools from Apple before using this script." >&2
        exit 1
    else
        export SDK="`/usr/bin/xcrun --show-sdk-path 2>/dev/null`"
        [ -z "${SDK}" ] && export SDK="/"
        if [ ! -d "${SDK}/usr/include" ]; then
            echo "ERROR: Cannot find Xcode SDK." >&2
            echo "Please run Xcode and select an available \"Command Line Tools\" from Xcode->Preferences->Locations." >&2
            exit 1
        else
            echo "  Using Xcode SDK: ${SDK}"
        fi
    fi
}

### Main Function START ###

# Function: Creating directory structure for EDK

[ ! -d ${DIR_MAIN} ]       && mkdir ${DIR_MAIN}
[ ! -d ${DIR_TOOLS} ]      && mkdir ${DIR_TOOLS}
[ ! -d ${DIR_GCC} ]        && mkdir ${DIR_GCC}
[ ! -d ${DIR_DOWNLOADS} ]  && mkdir ${DIR_DOWNLOADS}
[ ! -d ${DIR_LOGS} ]       && mkdir ${DIR_LOGS}
[ ! -d ${PREFIX}/include ] && mkdir -p ${PREFIX}/include

# Function: to manage PATH
pathmunge () {
    if [[ ! $PATH =~ (^|:)$1(:|$) ]]; then
        if [[ "${2:-}" = "after" ]]; then
            export PATH=$PATH:$1
        else
            export PATH=$1:$PATH
        fi
    fi
}

# RAMdisk
function mountRamDisk() {
    dev_ramdisk=$(mount | grep "$RAMDISK_MNT_PT" | awk '{print $1}')
    if [ -z "$dev_ramdisk" ];then
        echo "- Creating new RAM disk"
        dev_ramdisk=`hdiutil attach -nomount ram://3145728 | awk '{print $1}'`
        [ -n "$dev_ramdisk" ] && newfs_hfs -v "BuildGCC RamDisk" "$dev_ramdisk"
        [ ! -d "$RAMDISK_MNT_PT" ] && mkdir "$RAMDISK_MNT_PT"
        mount -t hfs "$dev_ramdisk" "$RAMDISK_MNT_PT"
        touch "$RAMDISK_MNT_PT/.metadata_never_index"
        echo
    fi
    # Automatically remove RAMDISK on exit
    trap 'echo; echo "- Ejecting RAM disk"; cd "$HOME"; umount "$RAMDISK_MNT_PT" && hdiutil detach "$dev_ramdisk"' EXIT
}

# Download #
DownloadSource () {
    cd $DIR_DOWNLOADS
    if [[ ! -f ${DIR_DOWNLOADS}/${GMP_VERSION}.tar.xz ]]; then
        echo "Status: ${GMP_VERSION} not found."
        curl -f -o download.tmp --remote-name https://ftp.gnu.org/gnu/gmp/${GMP_VERSION}.tar.xz || exit 1
        mv download.tmp ${GMP_VERSION}.tar.xz
    fi

    if [[ ! -f ${DIR_DOWNLOADS}/${MPFR_VERSION}.tar.xz ]]; then
        echo "Status: ${MPFR_VERSION} not found."
        curl -f -o download.tmp --remote-name https://ftp.gnu.org/gnu/mpfr/${MPFR_VERSION}.tar.xz || exit 1
        mv download.tmp ${MPFR_VERSION}.tar.xz
    fi

    if [[ ! -f ${DIR_DOWNLOADS}/${MPC_VERSION}.tar.gz ]]; then
        echo "Status: ${MPC_VERSION} not found."
        curl -f -o download.tmp --remote-name https://ftp.gnu.org/gnu/mpc/${MPC_VERSION}.tar.gz || exit 1
        mv download.tmp ${MPC_VERSION}.tar.gz
    fi

    if [[ ! -f ${DIR_DOWNLOADS}/${ISL_VERSION}.tar.xz ]]; then
        echo "Status: ${ISL_VERSION} not found."
        curl -o download.tmp --remote-name http://isl.gforge.inria.fr/${ISL_VERSION}.tar.xz || exit 1
        mv download.tmp ${ISL_VERSION}.tar.xz
    fi

    if [[ ! -f ${DIR_DOWNLOADS}/${BINUTILS_VERSION}.tar.xz ]]; then
        echo "Status: ${BINUTILS_VERSION} not found."
        curl -f -o download.tmp --remote-name https://ftp.gnu.org/gnu/binutils/${BINUTILS_VERSION}.tar.xz || exit 1
        mv download.tmp ${BINUTILS_VERSION}.tar.xz
    fi

    if [[ ! -f ${DIR_DOWNLOADS}/gcc-${GCC_VERSION}.tar.xz ]]; then
        echo "Status: gcc-${GCC_VERSION} not found."
        curl -f -o download.tmp --remote-name https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.xz || exit 1
        mv download.tmp gcc-${GCC_VERSION}.tar.xz
    fi
}


### Extract ###

# Function to extract source tarballs
ExtractTarball ()
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

    if [[ ! -d "${DIR_GCC}/$top_level_dir" ]]; then
        echo "- ${package} extracting..."
        rm -rf "${DIR_GCC}/$top_level_dir" # Remove old directory if exists
        rm -rf "$DIR_GCC/tmp"   # Remove old partial extraction
        mkdir -p "$DIR_GCC/tmp" # Create temporary directory
        tar -C "$DIR_GCC/tmp" -x "$tar_filter_option" -f "${tarball}"
        mv "$DIR_GCC/tmp/$top_level_dir" "$DIR_GCC/$top_level_dir"
        rm -rf "$DIR_GCC/tmp"
        echo "- ${package} extracted"
    fi

    # Restore stdout for the result and close file descriptor 3
    exec 1>&3-
    echo "${DIR_GCC}/$top_level_dir" # Return the full path where the tarball has been extracted
}

### Compile ###

# Function: Compiling libraries needed by GCC
CompileLibs () {
    if [[ ! -f $PREFIX/include/gmp.h ]]; then
        # Mount RamDisk
        mountRamDisk

        # Compile GMP
        local GMP_DIR=$(ExtractTarball "${GMP_VERSION}.tar.xz")

        rm -rf "${DIR_BUILD}/$ARCH-gmp"
        mkdir -p "${DIR_BUILD}/$ARCH-gmp" && cd "${DIR_BUILD}/$ARCH-gmp"

        # building gmp on Catalina currently requires no-stack-check flag - NOT NEEDED anymore with latest gmp
        #if [[ "${BUILDVER}" == "10.15" ]]; then
        #  echo "- ${GMP_VERSION} applying Catalina flag..."
        #  export CPPFLAGS="-fno-stack-check"
        #fi

        echo "- ${GMP_VERSION} configure..."
        "${GMP_DIR}"/configure --prefix=$PREFIX > $DIR_LOGS/gmp.$ARCH.configure.log.txt 2>&1
        echo "- ${GMP_VERSION} make..."
        make 1> /dev/null 2> $DIR_LOGS/gmp.$ARCH.make.log.txt
        make install 1> $DIR_LOGS/gmp.$ARCH.install.log.txt 2> /dev/null
        rm -rf "${DIR_BUILD}/$ARCH-gmp" "$GMP_DIR"

        echo "- ${GMP_VERSION} installed in $PREFIX"

        # unset flag which was set for Catalina
        #if [[ "${BUILDVER}" == "10.15" ]]; then
        #  unset CPPFLAGS
        #fi
    fi

    if [[ ! -f $PREFIX/include/mpfr.h ]]; then
        # Mount RamDisk
        mountRamDisk

        # Compile MPFR
        local MPFR_DIR=$(ExtractTarball "${MPFR_VERSION}.tar.xz")

        rm -rf "${DIR_BUILD}/$ARCH-mpfr"
        mkdir -p "${DIR_BUILD}/$ARCH-mpfr" && cd "${DIR_BUILD}/$ARCH-mpfr"
        curl -L https://www.mpfr.org/${MPFR_VERSION}/allpatches | patch -N -Z -p1 --directory="${MPFR_DIR}"
        echo "- ${MPFR_VERSION} configure..."
        "${MPFR_DIR}"/configure --prefix=$PREFIX --with-gmp=$PREFIX > $DIR_LOGS/mpfr.$ARCH.configure.log.txt 2>&1
        echo "- ${MPFR_VERSION} make..."
        make 1> /dev/null 2> $DIR_LOGS/mpfr.$ARCH.make.log.txt
        make install 1> $DIR_LOGS/mpfr.$ARCH.install.log.txt 2> /dev/null
        rm -rf "${DIR_BUILD}/$ARCH-mpfr" "$MPFR_DIR"
        echo "- ${MPFR_VERSION} installed in $PREFIX"
    fi

    if [[ ! -f $PREFIX/include/mpc.h ]]; then
        # Mount RamDisk
        mountRamDisk

        # Compile MPC
        local MPC_DIR=$(ExtractTarball "${MPC_VERSION}.tar.gz")

        rm -rf "${DIR_BUILD}/$ARCH-mpc"
        mkdir -p "${DIR_BUILD}/$ARCH-mpc" && cd "${DIR_BUILD}/$ARCH-mpc"
        echo "- ${MPC_VERSION} configure..."
        "${MPC_DIR}"/configure --prefix=$PREFIX --with-gmp=$PREFIX --with-mpfr=$PREFIX > $DIR_LOGS/mpc.$ARCH.configure.log.txt 2>&1
        echo "- ${MPC_VERSION} make..."
        make 1> /dev/null 2> $DIR_LOGS/mpc.$ARCH.make.log.txt
        make install 1> $DIR_LOGS/mpc.$ARCH.install.log.txt 2> /dev/null
        rm -rf "${DIR_BUILD}/$ARCH-mpc" "$MPC_DIR"
        echo "- ${MPC_VERSION} installed in $PREFIX"
    fi

    if [[ ! -f $PREFIX/lib/libisl.a ]]; then
        # Mount RamDisk
        mountRamDisk

        # Compile ISL
        local ISL_DIR=$(ExtractTarball "${ISL_VERSION}.tar.xz")

        rm -rf "${DIR_BUILD}/$ARCH-isl"
        mkdir -p "${DIR_BUILD}/$ARCH-isl" && cd "${DIR_BUILD}/$ARCH-isl"
        echo "- ${ISL_VERSION} configure..."
        "${ISL_DIR}"/configure --prefix=$PREFIX --with-gmp-prefix=$PREFIX --with-gcc-arch=$ARCH > $DIR_LOGS/isl.$ARCH.configure.log.txt 2>&1
        echo "- ${ISL_VERSION} make..."
        make 1> /dev/null 2> $DIR_LOGS/isl.$ARCH.make.log.txt
        make install 1> $DIR_LOGS/isl.$ARCH.install.log.txt 2> /dev/null
        rm -rf "${DIR_BUILD}/$ARCH-isl" "$ISL_DIR"
        echo "- ${ISL_VERSION} installed in $PREFIX"
    fi
}

# Function: Compiling Binutils
CompileBinutils () {

    if [[ -x "$PREFIX/cross/bin/$TARGET-ld" ]]; then
        return
    fi

    # Check GMP/MPFR/MPC
    [[ ! -f $PREFIX/include/gmp.h ]]  && echo "Error: ${GMP_VERSION} not installed, check logs"  && exit
    [[ ! -f $PREFIX/include/mpfr.h ]] && echo "Error: ${MPFR_VERSION} not installed, check logs" && exit
    [[ ! -f $PREFIX/include/mpc.h ]]  && echo "Error: ${MPC_VERSION} not installed, check logs"  && exit

#   [[ ! -x $PREFIX/bin/gcc ]] && echo "Error native ${GCC_VERSION} not installed !" && exit

    # Mount RamDisk
    mountRamDisk

    # Using our native GCC compiler
    # pathmunge $PREFIX/bin

    export BUILD_BINUTILS_DIR=${DIR_BUILD}/$ARCH-binutils

    # Extract the tarball
    local BINUTILS_DIR=$(ExtractTarball "${BINUTILS_VERSION}.tar.xz")

    # Binutils build
    rm -rf "$BUILD_BINUTILS_DIR"
    mkdir -p "$BUILD_BINUTILS_DIR" && cd "$BUILD_BINUTILS_DIR"
    echo "- ${BINUTILS_VERSION} configure..."
    local cmd="LDFLAGS=\"-dead_strip\" ${BINUTILS_DIR}/configure --host=${BUILDARCH}-apple-darwin${BUILDREV} --enable-plugins --build=${BUILDARCH}-apple-darwin${BUILDREV} --target=$TARGET --prefix=$PREFIX/cross --with-included-gettext --disable-werror --with-gmp=$PREFIX --with-mpfr=$PREFIX --with-mpc=$PREFIX --with-isl=$PREFIX --disable-isl-version-check --disable-nls --enable-deterministic-archives --with-mmap --with-system-zlib"
    local logfile="$DIR_LOGS/binutils.$ARCH.configure.log.txt"
    echo "$cmd" > "$logfile"
    eval "$cmd" >> "$logfile" 2>&1
    echo "- ${BINUTILS_VERSION} make..."
    cmd="make"
    logfile="$DIR_LOGS/binutils.$ARCH.make.log.txt"
    echo "$cmd" > "$logfile"
    eval "$cmd" >/dev/null 2>> "$logfile"
    if [[ $? -ne 0 ]]; then
        echo "Error compiling ${BINUTILS_VERSION} ! Check the log $logfile"
        exit 1
    fi
    cmd="make install-strip"
    logfile="$DIR_LOGS/binutils.$ARCH.install.log.txt"
    echo "$cmd" > "$logfile"
    eval "$cmd" >> "$logfile" 2>&1
    if [[ $? -ne 0 ]]; then
        echo "Error installing ${BINUTILS_VERSION} ! Check the log $logfile"
        exit 1
    fi
    rm -rf "$BUILD_BINUTILS_DIR" "$BINUTILS_DIR"

    [ ! -f $PREFIX/cross/bin/$TARGET-ld ] && echo "Error: ${BINUTILS_VERSION} not installed, check logs in $DIR_LOGS" && exit 1
    echo "- ${BINUTILS_VERSION} installed in $PREFIX/cross"
}


GCC_native () {
    if [[ ! -x "$PREFIX"/bin/gcc ]]; then
        # Mount RamDisk
        mountRamDisk

        local GCC_DIR=$(ExtractTarball "gcc-${GCC_VERSION}.tar.xz") || exit 1

        local BUILD_DIR="${DIR_BUILD}/$ARCH-gcc-native"
        rm -rf "$BUILD_DIR"
        mkdir -p "$BUILD_DIR" && cd "$BUILD_DIR"

        # Create the sdk directory and link it to the native sdk directory
        # We need to create individual links, as creating a link to just $SDK would result in the real path being followed and resolved
        local TOOLCHAIN_SDK_DIR="$PREFIX/sdk"
        rm -rf "$TOOLCHAIN_SDK_DIR"
        mkdir -p "$TOOLCHAIN_SDK_DIR"
        ln -sf "$SDK/usr" "$TOOLCHAIN_SDK_DIR/usr"
        ln -sf "$SDK/System" "$TOOLCHAIN_SDK_DIR/System"

        export CXXFLAGS="-O2 -I$PREFIX/include -I$PREFIX/sdk/include"
        export CFLAGS="-O2 -I$PREFIX/include -I$PREFIX/sdk/include"
        export LDFLAGS="-L$PREFIX/lib -L$PREFIX/sdk/lib -dead_strip -Wl,-dead_strip_dylibs"


        local cmd="${GCC_DIR}/configure --prefix='$PREFIX' --with-sysroot='$TOOLCHAIN_SDK_DIR' --enable-languages=c,c++ --libdir='$PREFIX/lib/gcc$GCC_MAJOR_VERSION' --includedir='$PREFIX/include/gcc$GCC_MAJOR_VERSION' --datarootdir='$PREFIX/share/gcc$GCC_MAJOR_VERSION' --with-included-gettext --with-system-zlib --disable-nls --enable-plugin --with-gxx-include-dir='$PREFIX/include/gcc$GCC_MAJOR_VERSION/c++/' --with-gmp='$PREFIX' --with-mpfr='$PREFIX' --with-mpc='$PREFIX' --with-isl='$PREFIX' --disable-bootstrap  --disable-isl-version-check"
        local logfile="$DIR_LOGS/gcc-native.$ARCH.configure.log.txt"
        echo "$cmd" > "$logfile"
        echo "- gcc-${GCC_VERSION} (native) configure..."
        eval "$cmd" >> "$logfile" 2>&1
        if [[ $? -ne 0 ]]; then
            echo "Error configuring GCC-${GCC_VERSION} ! Check the log $logfile" && exit 1
        fi

        cmd="make BOOT_CFLAGS='-O2 -I$PREFIX/include -I$PREFIX/sdk/include'"
        local logfile="$DIR_LOGS/gcc-native.$ARCH.make.log.txt"
        echo "$cmd" > "$logfile"
        echo "- gcc-${GCC_VERSION} (native) make..."
        eval "$cmd" >> "$logfile" 2>&1
        if [[ $? -ne 0 ]]; then
            echo "Error compiling GCC-${GCC_VERSION} ! Check the log $logfile" && exit 1
        fi

        cmd="make install-strip"
        local logfile="$DIR_LOGS/gcc-native.$ARCH.install.log.txt"
        echo "$cmd" > "$logfile"
        echo "- gcc-${GCC_VERSION} (native) installing..."
        eval "$cmd" >> "$logfile" 2>&1
        if [[ $? -ne 0 ]]; then
            echo "Error installing GCC-${GCC_VERSION} ! Check the log $logfile" && exit 1
        fi

        unset CFLAGS
        unset CXXFLAGS
        unset LDFLAGS

        # Create a new and clean sdk directory
        rm -rf "$TOOLCHAIN_SDK_DIR"
        mkdir -p "$TOOLCHAIN_SDK_DIR/usr/lib"
        # Copy header and library files needed to compile Basetools
        echo "- Copying headers and library files..."
        rsync -aH --copy-unsafe-links "$SDK/usr/include" "$TOOLCHAIN_SDK_DIR/usr/"
        rsync -aH --copy-unsafe-links "$SDK/usr/lib"/libSystem* "$SDK/usr/lib"/crt* "$TOOLCHAIN_SDK_DIR/usr/lib/"

        echo "- gcc-${GCC_VERSION} installed in $PREFIX"
        rm -rf "$BUILD_DIR"

        # Create a link for cc
        ln -f "$PREFIX"/bin/gcc "$PREFIX"/bin/cc
    fi
}

# Function: Compiling cross GCC compiler
CompileCrossGCC () {

    [[ -x $PREFIX/cross/bin/$TARGET-gcc ]] && return 0

    # Check GMP/MPFR/MPC
    [[ ! -f $PREFIX/include/gmp.h ]]  && echo "Error: ${GMP_VERSION} not installed, check logs"  && exit
    [[ ! -f $PREFIX/include/mpfr.h ]] && echo "Error: ${MPFR_VERSION} not installed, check logs" && exit
    [[ ! -f $PREFIX/include/mpc.h ]]  && echo "Error: ${MPC_VERSION} not installed, check logs"  && exit

    # Mount RamDisk
    mountRamDisk

    # Using our native GCC compiler
    # pathmunge $PREFIX/bin

    # Extract the tarball
    local GCC_DIR=$(ExtractTarball "gcc-${GCC_VERSION}.tar.xz")

    if [[ "${BUILDVER}" == "10.15" ]]; then
      echo "- gcc-${GCC_VERSION} applying Catalina path..."
      #patch no longer needed with gcc 9.3.0, CPATH still needed
      #curl -L https://raw.githubusercontent.com/Homebrew/formula-patches/b8b8e65e/gcc/9.2.0-catalina.patch | patch -N -Z -p1 --directory="${GCC_DIR}"
      export CPATH=$SDK/usr/include
    fi

    local BUILD_DIR="${DIR_BUILD}/$ARCH-gcc-cross"
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR" && cd "$BUILD_DIR"

    export CFLAGS="-Os -I$PREFIX/include -I$PREFIX/sdk/include"
    export CPPFLAGS="-Os -I$PREFIX/include -I$PREFIX/sdk/include"
    export CXXFLAGS="-Os -I$PREFIX/include -I$PREFIX/sdk/include"
    export CXXCPPFLAGS="-Os -I$PREFIX/include -I$PREFIX/sdk/include"
    export LDFLAGS="-Os -L$PREFIX/lib -L$PREFIX/sdk/lib -dead_strip -Wl,-dead_strip_dylibs"
    export BOOT_CFLAGS="-Os -I$PREFIX/include -I$PREFIX/sdk/include"
    export BOOT_CPPFLAGS="-Os -I$PREFIX/include -I$PREFIX/sdk/include"

    echo "- gcc-${GCC_VERSION} configure..."
    "${GCC_DIR}"/configure --host=${BUILDARCH}-apple-darwin${BUILDREV} --build=${BUILDARCH}-apple-darwin${BUILDREV} --target=$TARGET --prefix="$PREFIX/cross" --with-gmp="$PREFIX" --with-mpfr="$PREFIX" --with-mpc="$PREFIX" --with-isl="$PREFIX" --with-system-zlib --with-gnu-as --with-gnu-ld --with-newlib --disable-libssp --disable-nls --disable-werror --enable-languages=c,c++ --enable-plugin --disable-isl-version-check > "$DIR_LOGS"/gcc.$ARCH.configure.log.txt 2>&1

    echo "- gcc-${GCC_VERSION} make..."
    make all-gcc 1> /dev/null 2> $DIR_LOGS/gcc.$ARCH.make.log.txt
    make install-strip-gcc 1> $DIR_LOGS/gcc.$ARCH.install.log.txt 2> /dev/null

    unset CFLAGS
    unset CPPFLAGS
    unset CXXFLAGS
    unset CXXCPPFLAGS
    unset LDFLAGS
    unset BOOT_CFLAGS
    unset BOOT_CPPFLAGS

    # unset path which was set for Catalina
    if [[ "${BUILDVER}" == "10.15" ]]; then
      unset CPATH
    fi

    [[ ! -x $PREFIX/cross/bin/$TARGET-gcc ]] && \
     echo "Error: gcc-${GCC_VERSION} not installed, check logs in $DIR_LOGS" && exit 1

    rm -rf "${BUILD_DIR}" "$GCC_DIR"

    echo "- gcc-${GCC_VERSION} installed in $PREFIX/cross"
    echo
}

export TARGET="x86_64-clover-linux-gnu"
export ARCH="x64"
export ABI_VER="64"
echo "- Building GCC $GCC_VERSION toolchain for $ARCH"
echo "- to $PREFIX/cross/bin/${TARGET}"
CheckXCode      || exit 1

DownloadSource  || exit 1

startBuildEpoch=$(date -u "+%s")

CompileLibs     || exit 1
#GCC_native     || exit 1
CompileBinutils || exit 1
CompileCrossGCC || exit 1

# Remove GCC source directory
[[ -d "$DIR_GCC" ]] && rm -rf "$DIR_GCC"

stopBuildEpoch=$(date -u "+%s")
buildTime=$(expr $stopBuildEpoch - $startBuildEpoch)
if [[ $buildTime -gt 59 ]]; then
    timeToBuild=$(printf "%dm%ds" $((buildTime/60%60)) $((buildTime%60)))
else
    timeToBuild=$(printf "%ds" $((buildTime)))
fi

printf -- "\n* %s %s %s\n" "GCC toolchain Build process took" "$timeToBuild" "to complete..."
