#!/bin/bash

# Script for building GNU gettext
#
# Created by JrCs on 4/29/13.
#

# Gettext source version
# here we can change source versions of tools
#
export GETTEXT_VERSION=${GETTEXT_VERSION:-gettext-0.18.2.1}

# Change PREFIX if you want gettext installed on different place
#
export PREFIX=${PREFIX:-~/src/opt/local}

# ./configure arguments for Gettext
#
export GETTEXT_CONFIG="ac_cv_prog_AWK=/usr/bin/awk ac_cv_path_GREP=/usr/bin/grep ac_cv_path_SED=/usr/bin/sed --disable-csharp --disable-java --disable-native-java --disable-openmp --without-emacs --with-included-gettext --with-included-glib --with-included-libcroco --with-included-libunistring --with-included-libxml --without-git --without-cvs --prefix=$PREFIX"

# You can change DIR_MAIN if u wan't gettext source downloaded
# in different folder.
#
export RAMDISK_MNT_PT=/tmp/buildgettext-ramdisk
export DIR_MAIN=${DIR_MAIN:-~/src}
export DIR_TOOLS=${DIR_TOOLS:-$DIR_MAIN/tools}
export DIR_BUILD=${DIR_BUILD:-$RAMDISK_MNT_PT}
export DIR_DOWNLOADS=${DIR_DOWNLOADS:-$DIR_TOOLS/download}
export DIR_LOGS=${DIR_LOGS:-$DIR_TOOLS/logs}

## Paths for GCC (Xcode 4.1 fix) - works with Xcode 3.2 - Xcode 4.2
#
export CC="/usr/bin/gcc"
export CXX="/usr/bin/g++"
export CPP="/usr/bin/cpp"
export LD="/usr/bin/ld"

# Here we set MAKEFLAGS for GCC so it knows how many cores can use
# faster compile!
#
export MAKEFLAGS="-j `sysctl -n hw.ncpu`"

set -u

### Check Functions ###

fnCheckXcode ()
# Function: checking installation of Xcode Tools
{
[ ! -f /usr/bin/xcodebuild ] && \
echo "ERROR: Install Xcode Tools from Apple before using this script." && \
exit
}


### Main Function START ###

# Function: Creating directory structure for EDK

[ ! -d ${DIR_MAIN} ]       && mkdir ${DIR_MAIN}
[ ! -d ${DIR_TOOLS} ]      && mkdir ${DIR_TOOLS}
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
        [ -n "$dev_ramdisk" ] && newfs_hfs -v "Build Gettext RamDisk" "$dev_ramdisk"
        [ ! -d "$RAMDISK_MNT_PT" ] && mkdir "$RAMDISK_MNT_PT"
        mount -t hfs "$dev_ramdisk" "$RAMDISK_MNT_PT"

    fi
    # Automatically remove RAMDISK on exit
    trap 'echo; echo "- Ejecting RAM disk"; cd "$HOME"; umount "$RAMDISK_MNT_PT" && hdiutil detach "$dev_ramdisk"' EXIT
}


### Download ###

fnDownloadGettext ()
# Function: Download gettext source
{
    cd "$DIR_DOWNLOADS"
    if [[ ! -f ${GETTEXT_VERSION}.tar.gz ]]; then
        echo "Status: ${GETTEXT_VERSION} not found."
        curl --remote-name http://ftp.gnu.org/pub/gnu/gettext/${GETTEXT_VERSION}.tar.gz
    fi
}


### Extract ###

# Function to extract source tarballs
fnExtract ()
{
    exec 3>&1 1>&2 # Save stdout and redirect stdout to stderr

    local tarball="$1"
    local package=${tarball%%.tar*}
    tarball="${DIR_DOWNLOADS}/$tarball"
    local flagfile="${DIR_BUILD}/$package.extracted"

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

    if [[ ! -d "${DIR_BUILD}/$top_level_dir" || ! -f "$flagfile" ]]; then
        echo "-  ${package} extract..."
        rm -rf "${DIR_BUILD}/$top_level_dir" # Remove old directory if exists
        tar -C "$DIR_BUILD" -x "$tar_filter_option" -f "${tarball}" && touch "${DIR_BUILD}/$package.extracted"
    fi

    # Restore stdout for the result and close file desciptor 3
    exec 1>&3-
    echo "${DIR_BUILD}/$top_level_dir" # Return the full path where the tarball has been extracted
}


### Compile ###

fnCompileGettext ()
# Function: Compiling Gettext in PREFIX location
{
    # Mount RamDisk
    mountRamDisk

    export BUILD_GETTEXT_DIR=$DIR_BUILD/gettext

    # Extract the tarball
    local GETTEXT_DIR=$(fnExtract "${GETTEXT_VERSION}.tar.gz")

    # Gettext build
    rm -rf "$BUILD_GETTEXT_DIR"
    mkdir -p "$BUILD_GETTEXT_DIR" && cd "$BUILD_GETTEXT_DIR"
    echo "-  ${GETTEXT_VERSION} configure..."
    "${GETTEXT_DIR}"/configure $GETTEXT_CONFIG >$DIR_LOGS/gettext.config.log.txt 2>&1 || exit 1
    echo "-  ${GETTEXT_VERSION} make..."
    make >$DIR_LOGS/gettext.make.log.txt 2>&1 || exit 1
    echo "-  ${GETTEXT_VERSION} installing..."
    make install-strip >$DIR_LOGS/gettext.install.log.txt 2>&1 || exit 1
    rm -rf "$BUILD_GETTEXT_DIR"
    echo "-  ${GETTEXT_VERSION} installed in $PREFIX  -"
}


### fnFunctions ###

fnGettext ()
# Function: Gettext main script
{
    fnDownloadGettext
    fnCompileGettext
}


### Main ###

fnGettext
