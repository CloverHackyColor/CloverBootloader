#!/bin/bash

# Script for building GNU gettext
#
# Created by JrCs on 4/29/13.
#

set -u

# Gettext source version
# here we can change source versions of tools
#
export GETTEXT_VERSION=${GETTEXT_VERSION:-gettext-latest}

# Change PREFIX if you want gettext installed on different place
#
TOOLCHAIN_DIR=${TOOLCHAIN_DIR:-~/src/opt/local}
export PREFIX=${PREFIX:-$TOOLCHAIN_DIR}

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

# Here we set MAKEFLAGS for GCC so it knows how many cores can use
# faster compile!
#
export MAKEFLAGS="-j `sysctl -n hw.logicalcpu`"

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
        dev_ramdisk=`hdiutil attach -nomount ram://614400 | awk '{print $1}'`
        echo
        [ -n "$dev_ramdisk" ] && newfs_hfs -v "Build Gettext RamDisk" "$dev_ramdisk"
        [ ! -d "$RAMDISK_MNT_PT" ] && mkdir "$RAMDISK_MNT_PT"
        mount -t hfs "$dev_ramdisk" "$RAMDISK_MNT_PT"
        touch "$RAMDISK_MNT_PT/.metadata_never_index"
    fi
    # Automatically remove RAMDISK on exit
    trap 'echo; echo "- Ejecting RAM disk"; cd "$HOME"; umount "$RAMDISK_MNT_PT" && hdiutil detach "$dev_ramdisk"' EXIT
}


### Download ###

fnDownloadGettext ()
# Function: Download gettext source
{
    cd "$DIR_DOWNLOADS"
    local tarball="${GETTEXT_VERSION}.tar.xz"
    if [[ ! -f "$tarball" ]]; then
        echo "Status: $tarball not found."
        curl -f -o download.tmp --remote-name ftp://ftp.gnu.org/pub/gnu/gettext/$tarball || exit 1
        mv download.tmp $tarball
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
    local GETTEXT_DIR=$(fnExtract "${GETTEXT_VERSION}.tar.xz")

    # Gettext build
    local cmd logfile
    rm -rf "$BUILD_GETTEXT_DIR"
    mkdir -p "$BUILD_GETTEXT_DIR" && cd "$BUILD_GETTEXT_DIR"
    echo "-  ${GETTEXT_VERSION} configure..."
    cmd="'${GETTEXT_DIR}/configure' $GETTEXT_CONFIG"
    logfile="$DIR_LOGS/gettext.configure.log.txt"
    echo "$cmd" > "$logfile"
    eval "$cmd" >> "$logfile" 2>&1
    if [[ $? -ne 0 ]]; then
        echo "Error configuring ${GETTEXT_VERSION}! Check the log $logfile"
        exit 1
    fi
    echo "-  ${GETTEXT_VERSION} make..."
    cmd="make"
    logfile="$DIR_LOGS/gettext.make.log.txt"
    echo "$cmd" > "$logfile"
    eval "$cmd" >> "$logfile" 2>&1
    if [[ $? -ne 0 ]]; then
        echo "Error compiling ${GETTEXT_VERSION} ! Check the log $logfile"
        exit 1
    fi
    echo "-  ${GETTEXT_VERSION} installing..."
    cmd="make install-strip"
    logfile="$DIR_LOGS/gettext.install.log.txt"
    echo "$cmd" > "$logfile"
    eval "$cmd" >> "$logfile" 2>&1
    if [[ $? -ne 0 ]]; then
        echo "Error installing ${GETTEXT_VERSION} ! Check the log $logfile"
        exit 1
    fi
    rm -rf "$BUILD_GETTEXT_DIR"
    echo "-  ${GETTEXT_VERSION} installed in $PREFIX"
}


### fnFunctions ###

fnGettext ()
# Function: Gettext main script
{
    fnDownloadGettext
    fnCompileGettext
}


### Main ###

# Add XCode bin directory for the command line tools to the PATH
pathmunge "$(xcode-select --print-path)"/usr/bin

fnGettext
