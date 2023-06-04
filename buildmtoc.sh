#!/bin/bash

# Script for building MTOC
#

if [ -z "$WORKSPACE" ]
then
  echo WORKSPACE must be defined to Clover root path
  export WORKSPACE=`pwd`
fi

# Ctools source version
# here we can change source versions of tools
#
OSXVER="`/usr/bin/sw_vers -productVersion | cut -d '.' -f1,2`"
if [[ ${OSXVER} < 10.15 ]]; then
	export CCTOOLS_VERSION=${CCTOOLS_VERSION:-cctools-949.0.1}
else
	export CCTOOLS_VERSION=${CCTOOLS_VERSION:-cctools-986}
fi
# Change PREFIX if you want mtoc installed on different place
#
TOOLCHAIN_DIR=${TOOLCHAIN_DIR:-~/src/opt/local}
export PREFIX=${PREFIX:-$TOOLCHAIN_DIR}

# You can change DIR_MAIN if u want cctools source downloaded
# in different folder.
#
export RAMDISK_MNT_PT=/tmp/buildmtoc-ramdisk
export DIR_MAIN=${DIR_MAIN:-~/src}
export DIR_TOOLS=${DIR_TOOLS:-$DIR_MAIN/tools}
export DIR_BUILD=${DIR_BUILD:-$RAMDISK_MNT_PT}
export DIR_DOWNLOADS=${DIR_DOWNLOADS:-$DIR_TOOLS/download}
export DIR_LOGS=${DIR_LOGS:-$DIR_TOOLS/logs}

# Here we set MAKEFLAGS for GMAKE so it knows how many cores can use
# faster compile!
#
export MAKEFLAGS="-j `sysctl -n hw.ncpu`"

set -u

### Check Functions ###

# Function: checking installation of Xcode Tools
fnCheckXcode () {
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
[ ! -d ${PREFIX}/bin ]     && mkdir -p ${PREFIX}/bin
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
        [ -n "$dev_ramdisk" ] && newfs_hfs -v "Build Mtoc RamDisk" "$dev_ramdisk"
        [ ! -d "$RAMDISK_MNT_PT" ] && mkdir "$RAMDISK_MNT_PT"
        mount -t hfs "$dev_ramdisk" "$RAMDISK_MNT_PT"
        touch "$RAMDISK_MNT_PT/.metadata_never_index"
    fi
    # Automatically remove RAMDISK on exit
    trap 'echo; echo "- Ejecting RAM disk"; cd "$HOME"; umount "$RAMDISK_MNT_PT" && hdiutil detach "$dev_ramdisk"' EXIT
}


### Download ###

fnDownloadCctools ()
# Function: Download cctools source
{
    cd "$DIR_DOWNLOADS"
    local tarball="${CCTOOLS_VERSION}.tar.gz"
    if [[ ! -f ${DIR_DOWNLOADS}/${CCTOOLS_VERSION}.tar.gz ]]; then
        echo "Status: ${CCTOOLS_VERSION} not found."
        curl -k -f -o download.tmp --remote-name https://codeload.github.com/apple-oss-distributions/cctools/tar.gz/refs/tags/${CCTOOLS_VERSION} || exit 1
        mv download.tmp $tarball
    fi
}
#https://github.com/apple-oss-distributions/cctools/archive/refs/tags/cctools-973.0.1.tar.gz

### Extract ###

# Function to extract source tarballs
fnExtract ()
{
    exec 3>&1 1>&2 # Save stdout and redirect stdout to stderr
		
		# OS version check
		local OSXVER="`/usr/bin/sw_vers -productVersion | cut -d '.' -f1,2`"

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
        
        #jief copy the modified version that keeps __mod_init_func section
        #cp "$WORKSPACE"/BaseTools/Source/C/mtoc/mtoc-v921_jief.c "$DIR_BUILD/$top_level_dir"/efitools/mtoc.c
				if [[ ${OSXVER} < 10.15 ]]; then
					cp "$WORKSPACE"/Patches/Mtoc/mtoc-v973_jief.c "$DIR_BUILD/$top_level_dir"/efitools/mtoc.c
				else
					cp "$WORKSPACE"/Patches/Mtoc/mtoc-v986_jief.c "$DIR_BUILD/$top_level_dir"/efitools/mtoc.c
					cp "$WORKSPACE"/Patches/Mtoc/libstuff.xcconfig "$DIR_BUILD/$top_level_dir"/xcode/
				fi
    fi

    # Restore stdout for the result and close file desciptor 3
    exec 1>&3-
    echo "${DIR_BUILD}/$top_level_dir" # Return the full path where the tarball has been extracted
}

### Compile ###

fnCompileMtoc ()
# Function: Compiling Mtoc in PREFIX location
{
    # Mount RamDisk
    mountRamDisk
		
		# OS version check
		local OSXVER="`/usr/bin/sw_vers -productVersion | cut -d '.' -f1,2`"
		
    # Extract the tarball
    local CCTOOLS_DIR=$(fnExtract "${CCTOOLS_VERSION}.tar.gz")

    # Set SDKROOT for Makefiles
    export SDKROOT="$(xcrun -n --show-sdk-path)"

    # Mtoc build
    local cmd logfile
    cd "$CCTOOLS_DIR"
    # Removal of _structs.h needed as a workaround for a bug in cctools-900 or above
    /bin/rm -f ./include/mach/i386/_structs.h
    echo "-  cctools-${CCTOOLS_VERSION} make mtoc..."
		if [[ ${OSXVER} < 10.15 ]]; then
			cmd="(make LTO= EFITOOLS=efitools -C libstuff) && (make -C efitools)"
		else
			cmd="(xcodebuild -scheme mtoc -configuration Release ONLY_ACTIVE_ARCH=YES CONFIGURATION_BUILD_DIR=$PREFIX/bin)"
		fi
    logfile="$DIR_LOGS/mtoc.make.log.txt"
    echo "$cmd" > "$logfile"
    eval "$cmd" >> "$logfile" 2>&1
    if [[ $? -ne 0 ]]; then
        echo "Error compiling cctools-${CCTOOLS_VERSION} mtoc ! Check the log $logfile"
        exit 1
    fi
    echo "-  cctools-${CCTOOLS_VERSION} installing mtoc..."
		if [[ ${OSXVER} < 10.15 ]]; then
			cmd="install -c -s -m 555 efitools/mtoc.NEW ${PREFIX}/bin/mtoc.NEW_jief"
		else
			cmd="mv ${PREFIX}/bin/mtoc ${PREFIX}/bin/mtoc.NEW_jief"
		fi
    logfile="$DIR_LOGS/mtoc.install.log.txt"
    echo "$cmd" > "$logfile"
    eval "$cmd" >> "$logfile" 2>&1
    if [[ $? -ne 0 ]]; then
        echo "Error installing cctools-${CCTOOLS_VERSION} mtoc ! Check the log $logfile"
        exit 1
    fi
    rm -rf "$CCTOOLS_DIR"
    echo "-  cctools-${CCTOOLS_VERSION} mtoc installed in $PREFIX"
}

# remove unused additional files
fnCleanUp()
{
	if [[ ${OSXVER} > 10.15 ]]; then
		rm -r $PREFIX/bin/*.dSYM $PREFIX/bin/*.a
	fi
}

### fnFunctions ###

fnMtoc ()
# Function: Mtoc main script
{
    fnDownloadCctools
    fnCompileMtoc
		fnCleanUp
}


### Main ###

# Add XCode bin directory for the command line tools to the PATH
#pathmunge "$(xcode-select --print-path)"/usr/bin

fnMtoc
