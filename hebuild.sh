#!/bin/bash

#  ebuild.sh ->ebuild.sh  //renamed to be unique file begining from E
#  Script for building CloverEFI source under OS X or Linux
#  Supported chainloads(compilers) are XCODE*, GCC*, UNIXGCC and CLANG
#
#
#  Created by Jadran Puharic on 1/6/12.
#  Modified by JrCs on 3/9/13.
#  Zenith432, STLVNUB, cecekpawon 2016
#  Micky1979 2016

# Go to the Clover root directory
cd "$(dirname $0)"

# Global variables
declare -r SELF="${0##*/}"
declare -r CLOVERROOT="$PWD"
declare -r SYSNAME="$(uname)"
declare -r DRIVERS_LEGACY="BIOS" # same in buildpkg.sh/makeiso
declare -r DRIVERS_UEFI="UEFI"   # same in buildpkg.sh/makeiso
declare -r DRIVERS_OFF="off"     # same in buildpkg.sh/makeiso

startBuildEpoch=$(date -u "+%s")

if [[ "$SYSNAME" == Linux ]]; then
  echo "Linux"
  declare -r NUMBER_OF_CPUS=$(nproc)
else
  declare -r NUMBER_OF_CPUS=$(sysctl -n hw.logicalcpu)
fi
declare -a EDK2_BUILD_OPTIONS=
print_option_help_wc=
have_fmt=
PLATFORMFILE=
MODULEFILE=
TARGETRULE=

SCRIPT_VERS="2026-03-01"

# Macro
M_NOGRUB=0
M_APPLEHFS=0

# Default values
export TOOLCHAIN=GCC152
export TARGETARCH=X64
export BUILDTARGET=RELEASE
export BUILDTHREADS=$(( NUMBER_OF_CPUS + 1 ))
export WORKSPACE=${WORKSPACE:-}
export CONF_PATH=${CONF_PATH:-}
#export NASM_PREFIX=
export PYTHON_COMMAND=python3

# if building through Xcode, then TOOLCHAIN_DIR is not defined
# checking if it is where CloverGrowerPro put it
#if [[ "$SYSNAME" == Linux ]]; then
#  export TOOLCHAIN=GCC152
#  TOOLCHAIN_DIR=${TOOLCHAIN_DIR:-/usr}
#else
#  if [[ "$TOOLCHAIN" == XCLANG ]]; then
#    TOOLCHAIN_DIR=/opt/local
#  elif [[ -d ~/src/opt/local ]]; then
#    TOOLCHAIN_DIR=~/src/opt/local
#  else
#    TOOLCHAIN_DIR=${TOOLCHAIN_DIR:-"$CLOVERROOT"/toolchain}
#  fi
#  export DIR_MAIN=${DIR_MAIN:-"$CLOVERROOT"/toolchain}
#fi
#export TOOLCHAIN_DIR
#echo "TOOLCHAIN_DIR: $TOOLCHAIN_DIR"

VBIOSPATCHCLOVEREFI=0
ONLYSATA0PATCH=0
USE_BIOS_BLOCKIO=0
USE_LOW_EBDA=1
CLANG=0
GENPAGE=0
LLVM=0

FORCEREBUILD=0
NOBOOTFILES=0
ENABLE_MODERN_CPU=1

declare -r GIT=`which git`
#declare -r GITDIR=`git status 2> /dev/null`        # unsafe as git repository may exist in parent directory
#declare -r VERSTXT="vers.txt"
if [[ -x "/usr/bin/sw_vers" ]]; then
  declare -r OSVER="$(sw_vers -productVersion | sed -e 's/\.0$//g')"
elif [[ -x "/usr/bin/lsb_release" ]]; then
  # Linux print the name+version in in two lines, sed serves to made it in one line!
  # ..otherwise Clover fail because Version.h will have a line with no null terminated char.
  declare -r OSVER="$(lsb_release -sir | sed -e ':a;N;$!ba;s/\n/ /g')"
fi

# Bash options
set -e # errexit
set -u # Blow on unbound variable

## FUNCTIONS ##

print_option_help () {
  if [[ x$print_option_help_wc = x ]]; then
      if wc -L  </dev/null > /dev/null 2>&1; then
          print_option_help_wc=-L
      elif wc -m  </dev/null > /dev/null 2>&1; then
          print_option_help_wc=-m
      else
          print_option_help_wc=-b
      fi
  fi
  if [[ x$have_fmt = x ]]; then
      if fmt -w 40  </dev/null > /dev/null 2>&1; then
          have_fmt=y;
      else
          have_fmt=n;
      fi
  fi
  local print_option_help_lead="  $1"
  local print_option_help_lspace="$(echo "$print_option_help_lead" | wc $print_option_help_wc)"
  local print_option_help_fill="$((26 - print_option_help_lspace))"
  printf "%s" "$print_option_help_lead"
  local print_option_help_nl=
  if [[ $print_option_help_fill -le 0 ]]; then
      print_option_help_nl=y
      echo
  else
      print_option_help_i=0;
      while [[ $print_option_help_i -lt $print_option_help_fill ]]; do
          printf " "
          print_option_help_i=$((print_option_help_i+1))
      done
      print_option_help_nl=n
  fi
  local print_option_help_split=
  if [[ x$have_fmt = xy ]]; then
      print_option_help_split="$(echo "$2" | fmt -w 50)"
  else
      print_option_help_split="$2"
  fi
  if [[ x$print_option_help_nl = xy ]]; then
      echo "$print_option_help_split" | awk '{ print "                          " $0; }'
  else
      echo "$print_option_help_split" | awk 'BEGIN   { n = 0 }
          { if (n == 1) print "                          " $0; else print $0; n = 1 ; }'
  fi
}

# Function to manage PATH
pathmunge () {
    if [[ ! $PATH =~ (^|:)$1(:|$) ]]; then
        if [[ "${2:-}" = "after" ]]; then
            export PATH=$PATH:$1
        else
            export PATH=$1:$PATH
        fi
    fi
}

packagesPathmunge () {
    if [[ -z "${PACKAGES_PATH:-}" ]]; then
        export PACKAGES_PATH="$WORKSPACE" # must be always the first
    fi
    if [[ ! $PACKAGES_PATH =~ (^|:)$1(:|$) ]]; then
        if [[ "${2:-}" = "after" ]]; then
            export PACKAGES_PATH=$PACKAGES_PATH:$1
        else
            export PACKAGES_PATH=$1:$PACKAGES_PATH
        fi
    fi
}

# Add edk2 build option
addEdk2BuildOption() {
    EDK2_BUILD_OPTIONS=("${EDK2_BUILD_OPTIONS[@]}" $@)
}

# Add edk2 build macro
addEdk2BuildMacro() {
  local macro="$1"
  [[ "$macro" == "NO_GRUB_DRIVERS" ]] && M_NOGRUB=1
  if [[ "$macro" == "USE_APPLE_HFSPLUS_DRIVER" && "$TARGETARCH" == "X64" ]]; then
    [[ ! -e "${CLOVERROOT}"/FileSystems/HFSPlus/X64/HFSPlus.efi ]] && return
    M_APPLEHFS=1
  fi
  addEdk2BuildOption "-D" "$macro"
}

# Check NASM

IsNumericOnly() {
  if [[ "${1}" =~ ^-?[0-9]+$ ]]; then
    return 0 # no, contains other or is empty
  else
    return 1 # yes is an integer (no matter for bash if there are zeroes at the beginning comparing it as integer)
  fi
}

isNASMGood() {
  # nasm should be greater or equal to 2.12.02 to be good building Clover.
  # There was a bad macho relocation in outmacho.c, fixed by Zenith432
  # and accepted by nasm devel during 2.12.rcxx (release candidate)
  # modern nasm is 2.15

  result=1
  local nasmver=$( "${1}" -v | grep 'NASM version' | awk '{print $3}' )

  case "$nasmver" in
  2.16.0[2-9]* | 2.16.[1-9]* | 2.1[3-9]* | 2.[2-9]* | [3-9]* | [1-9][1-9]*)
	result=0;;
  *)
	printf "\n\e[1;33mUnknown or unsupported NASM version found at:\n${1}\n\n\e[0m";;
  esac

  return $result
}

# Check Xcode toolchain
checkXcode () {
    XCODE_BUILD="/usr/bin/xcodebuild"
    local LOCALBIN="/usr/local/bin"

    if [[ ! -x "${XCODE_BUILD}" ]]; then
       echo "ERROR: Install Xcode Tools from Apple before using this script." >&2; exit 1
    fi

  if [[ -x "/opt/local/bin/mtoc.NEW_jief" ]]; then
    export MTOC_PREFIX="/opt/local/bin/"
  elif [[ -x "${LOCALBIN}/mtoc.NEW_jief" ]]; then
    export MTOC_PREFIX="${LOCALBIN}/"
  elif [[ -x "${TOOLCHAIN_DIR}/bin/mtoc.NEW_jief" ]]; then
    export MTOC_PREFIX="${TOOLCHAIN_DIR}/bin/"
  else
    export MTOC_PREFIX="${TOOLCHAIN_DIR}/bin/"
    ./buildmtoc.sh
  fi
  echo "MTOC_PREFIX: $MTOC_PREFIX"
}

# Print the usage.
usage() {
    echo "Script for building CloverEFI sources on Darwin OS X"
    echo "Version from ${SCRIPT_VERS}"
    printf "Usage: %s [OPTIONS] [all|fds|genc|genmake|clean|cleanpkg|cleanall|cleanlib|modules|libraries]\n" "$SELF"
    echo
    echo "Configuration:"
    print_option_help "-n THREADNUMBER" "Build the platform using multi-threaded compiler [default is number of CPUs + 1]"
    print_option_help "-h, --help"    "print this message and exit"
    print_option_help "-v, --version" "print the version information and exit"
    echo
    echo "Toolchain:"
    print_option_help "-clang"     "use Clang toolchain"
    print_option_help "-llvm"      "use LLVM toolchain"
#    print_option_help "-gcc49"     "use GCC 4.9 toolchain"
    print_option_help "-gcc53"     "use GCC 5.3 toolchain, including gcc-11"
    print_option_help "-gcc131"    "use GCC 13.1 toolchain, including gcc-14.2"
    print_option_help "-gcc151"    "use GCC 15.1 toolchain"
    print_option_help "-gcc152"    "use GCC 15.2 toolchain"
#    print_option_help "-unixgcc"   "use UNIXGCC toolchain, unsupported"
#    print_option_help "-xcode"     "use XCode 3.2 toolchain"
    print_option_help "-xcode5"     "use XCode 5 toolchain, "
    print_option_help "-xcode8"     "use XCode 8 toolchain  [Default]"
    print_option_help "-xcode14"     "use XCode 14 toolchain"
    print_option_help "-xcode16"     "use XCode 16-26 toolchain"
    print_option_help "-t TOOLCHAIN, --tagname=TOOLCHAIN" "force to use a specific toolchain"
    echo
    echo "Target:"
    print_option_help "-x64"       "build Clover in 64-bit [boot6] [Default]"
    print_option_help "-mc, --x64-mcp"   "build Clover in 64-bit [boot7] using BiosBlockIO (compatible with MCP chipset)"
    print_option_help "-p PLATFORMFILE, --platform=PLATFORMFILE" "Build the platform specified by the DSC filename argument"
    print_option_help "-m MODULEFILE, --module=MODULEFILE" "Build only the module specified by the INF filename argument"
    print_option_help "-b BUILDTARGET, --buildtarget=BUILDTARGET" "using the BUILDTARGET to build the platform"
    print_option_help "-clean"     "same as clean"
    print_option_help "-cleanall"  "same as cleanall"
    echo
    echo "Options:"
    print_option_help "-D MACRO, --define=MACRO" "Macro: \"Name[=Value]\"."
    print_option_help "--vbios-patch-cloverefi" "activate vbios patch in CloverEFI"
    print_option_help "--only-sata0" "activate only SATA0 patch"
    print_option_help "--std-ebda" "ebda offset dont shift to 0x88000"
    print_option_help "--genpage" "dynamically generate page table under ebda"
    print_option_help "--no-usb" "disable USB support"
    print_option_help "--no-lto" "disable Link Time Optimisation"
    print_option_help "--no-mcpu" "disable AutoModernCPUQuirks" 
 #   print_option_help "--edk2shell <MinimumShell|FullShell>" "copy edk2 Shell to EFI tools dir"
    echo
    echo "build options:"
    print_option_help "-fr, --force-rebuild" "force rebuild all targets"
    print_option_help "-mc, --x64-mcp" " generate boot7 file with drivers using legacy BIOS functions"
    print_option_help "-nb, --no-bootfiles" "don't generate boot files"
    echo
    echo "Report bugs to     https://github.com/CloverHackyColor/CloverBootloader/issues"
}

# Manage option argument
argument () {
  local opt=$1
  shift

  if [[ $# -eq 0 ]]; then
      printf "%s: option \`%s' requires an argument\n" "$0" "$opt"
      exit 1
  fi

  echo $1
}

# Check the command line arguments
checkCmdlineArguments() {
    while [[ $# -gt 0 ]]; do
        local option=$1
        shift
        case "$option" in
            -llvm  | --llvm)   TOOLCHAIN=LLVM ; CLANG=1 ;;
            -clang  | --clang)   TOOLCHAIN=XCLANG ; CLANG=1 ;;
            -xcode5  | --xcode5 )  TOOLCHAIN=XCODE5 ; CLANG=1 ;;
            -xcode8  | --xcode8 )  TOOLCHAIN=XCODE8 ; CLANG=1 ;;
            -xcode14 | --xcode14 )  TOOLCHAIN=XCODE14 ; CLANG=1 ;;
            -xcode15 | --xcode15 )  TOOLCHAIN=XCODE15 ; CLANG=1 ;;
            -xcode16 | --xcode16 )  TOOLCHAIN=XCODE16 ; CLANG=1 ;;
            -GCC49  | --GCC49)   TOOLCHAIN=GCC49   ;;
            -gcc49  | --gcc49)   TOOLCHAIN=GCC49   ;;
            -GCC53  | --GCC53)   TOOLCHAIN=GCC53   ;;
            -gcc53  | --gcc53)   TOOLCHAIN=GCC53   ;;
            -gcc131  | --gcc131)   TOOLCHAIN=GCC131   ;;
            -gcc151  | --gcc151)   TOOLCHAIN=GCC151   ;;
            -gcc152  | --gcc152)   TOOLCHAIN=GCC152   ;;
            -xcode  | --xcode )  TOOLCHAIN=XCODE32 ;;
            -x64 | --x64)
#                printf "\`%s' is deprecated because Clover is 64 bit only. This message will be removed soon\n" "$option" 1>&2
#                sleep 4
                ;;
            -mc | --x64-mcp)   USE_BIOS_BLOCKIO=1 ;;
            -clean)    TARGETRULE=clean ;;
            -cleanall) TARGETRULE=cleanall ;;
            -fr | --force-rebuild) FORCEREBUILD=1 ;;
            -nb | --no-bootfiles) NOBOOTFILES=1 ;;
#            -d | -debug | --debug)  BUILDTARGET=DEBUG ;;
#            -r | -release | --release) BUILDTARGET=RELEASE ;;
            -a) TARGETARCH=$(argument $option "$@")
#                printf "\`%s' is deprecated because Clover is 64 bit only. This message will be removed soon\n" "$option" 1>&2
#                sleep 4
                ;;
            --arch=*)
#                printf "\`%s' is deprecated because Clover is 64 bit only. This message will be removed soon\n" "$option" 1>&2
#                sleep 4
                ;;
            -p) PLATFORMFILE=$(argument $option "$@"); shift
                ;;
            --platform=*)
                PLATFORMFILE=$(echo "$option" | sed 's/--platform=//')
                ;;
            -m) MODULEFILE=$(argument $option "$@"); shift
                ;;
            --module=*)
                MODULEFILE=$(echo "$option" | sed 's/--module=//')
                ;;
            -b) BUILDTARGET=$(argument $option "$@"); shift
                ;;
            --buildtarget=*)
                BUILDTARGET=$(echo "$option" | sed 's/--buildtarget=//')
                ;;
            -t) TOOLCHAIN=$(argument $option "$@"); shift
                ;;
            --tagname=*)
                TOOLCHAIN=$(echo "$option" | sed 's/--tagname=//')
                ;;
            -D)
                addEdk2BuildMacro $(argument $option "$@"); shift
                ;;
            --define=*)
                addEdk2BuildMacro $(echo "$option" | sed 's/--define=//')
                ;;
            -n)
                BUILDTHREADS=$(argument $option "$@"); shift
                ;;
            --vbios-patch-cloverefi)
                VBIOSPATCHCLOVEREFI=1
                ;;
            --only-sata0)
                ONLYSATA0PATCH=1
                ;;
            --std-ebda)
                USE_LOW_EBDA=0
                ;;
            --genpage)
                GENPAGE=1
                ;;
            --no-mcpu)
                ENABLE_MODERN_CPU=0
                ;;   
            --no-usb)
                addEdk2BuildMacro DISABLE_USB_SUPPORT
                ;;
            --no-lto)
                addEdk2BuildMacro DISABLE_LTO
                ;;
            --ext-pre | --ext-co | --ext-build)
                printf "\`%s' is deprecated. This message will be removed soon\n" "$option" 1>&2
                sleep 4
                ;;
            --edk2shell) EDK2SHELL=$(argument $option "$@"); shift
                ;;
            -h | -\? | -help | --help)
                usage && exit 0
                ;;
            -v | --version)
                echo "$SELF vers from $SCRIPT_VERS" && exit 0
                ;;
            -*)
                printf "Unrecognized option \`%s'\n" "$option" 1>&2
                exit 1
                ;;
            *)
               TARGETRULE="$option"
               ;;
        esac
    done

    # Update variables
    PLATFORMFILE="${PLATFORMFILE:-Clover.dsc}"
    if [ ! -z "${MODULEFILE}" ]; then
        MODULEFILE=" -m $MODULEFILE"
    fi

    # Allow custom config path
    if [[ -f "${CONF_PATH}/target.txt" ]]; then
      addEdk2BuildOption "--conf=${CONF_PATH%/}"
    elif [[ -f "${CLOVERROOT}/Conf/target.txt" ]]; then
      addEdk2BuildOption "--conf=${CLOVERROOT}/Conf"
    fi
}

## Check tools for the toolchain
checkToolchain() {
    case "$TOOLCHAIN" in
        XCLANG|XCODE*) checkXcode ;;
                *) export MTOC_PREFIX="${TOOLCHAIN_DIR}/bin/" ;;
    esac

  if [[ "$SYSNAME" == Linux ]]; then
    export GCC152_BIN="$TOOLCHAIN_DIR/bin/"
    if [[ ! -x "${GCC152_BIN}gcc" ]]; then
        echo "No clover toolchain for Linux found !" >&2
        echo "Install on your system or define the TOOLCHAIN_DIR variable." >&2
        exit 1
    fi
  else
    export GCC152_BIN="$TOOLCHAIN_DIR/cross/bin/x86_64-clover-linux-gnu-"
    if [[ $TOOLCHAIN == GCC* ]] && [[ ! -x "${GCC152_BIN}gcc" ]]; then
      echo "No clover toolchain found !" >&2
      echo "Build it with the build_gcc15.sh script or define the TOOLCHAIN_DIR variable." >&2
      exit 1
    fi
  fi

  if [[ -x "/opt/local/bin/nasm" ]]; then
    export NASM_PREFIX="/opt/local/bin/"
  elif [[ -x "${TOOLCHAIN_DIR}/bin/nasm" ]]; then
    # using $TOOLCHAIN_DIR here should allow Clover source to be
    # inside any sub folder instead of only in ~/
    export NASM_PREFIX="${TOOLCHAIN_DIR}/bin/"
  else
    export NASM_PREFIX="${TOOLCHAIN_DIR}/bin/"
    ./buildnasm.sh
  fi

  if [[ $TOOLCHAIN == XCLANG ]]; then
    export XCLANG_PREFIX="/opt/local/bin/"
    export XCLANG_BIN="/opt/local/libexec/llvm-22/bin/"
  fi

  echo "NASM_PREFIX: $NASM_PREFIX"

  #NASM_VER=`nasm -v | awk '/version/ {print $3}'`
  NASM_VER=`${NASM_PREFIX}nasm -v | sed -nE 's/^.*version.([0-9\.]+).*$/\1/p'`

  echo "NASM_VER: $NASM_VER"
  if [[ "$SYSNAME" == Darwin ]]; then
    if ! isNASMGood "${NASM_PREFIX}nasm"; then echo "your nasm is not good to build Clover!" && exit 1; fi
  fi
}

# Main build script
MainBuildScript() {
 #   checkCmdlineArguments $@
 
    #
    # Setup workspace if it is not set
    #
    local EDK2DIR=$(cd "$CLOVERROOT" && echo "$PWD")
    if [[ -z "$WORKSPACE" ]]; then
        echo "Initializing workspace"
        if [[ ! -x "${EDK2DIR}"/edksetup.sh ]]; then
            echo "Error: Can't find edksetup.sh script !" >&2
            exit 1
        fi

        # This version is for the tools in the BaseTools project.
        # this assumes svn pulls have the same root dir
        #  export EDK_TOOLS_PATH=`pwd`/../BaseTools
        # This version is for the tools source in edk2
  #      cd "$EDK2DIR"
        export EDK_TOOLS_PATH="${PWD}"/BaseTools
        set +u
        source ./edksetup.sh BaseTools
        set -u
		if [ ! -z "${MTOC_PREFIX:-}" ]; then
        	echo "MTOC=$MTOC_PREFIX/mtoc.NEW_jief" > "$WORKSPACE"/Xcode/CloverX64/mtoc_path.txt
		fi
        cd "$CLOVERROOT"
    else
        echo "Building from: $WORKSPACE"
    fi

    checkToolchain
echo "Toolchain=$TOOLCHAIN at $TOOLCHAIN_DIR "


export BUILD_DIR="${WORKSPACE}/Build/Clover/${BUILDTARGET}_${TOOLCHAIN}"
export BUILD_DIR_ARCH="${BUILD_DIR}/$TARGETARCH"
echo "BUILD_DIR: $BUILD_DIR"
echo "BUILD_DIR_ARCH: $BUILD_DIR_ARCH"

 #   local repoRev=$(git describe --tags $(git rev-list --tags --max-count=1﻿))
	local repoRev=$(git describe --tags --abbrev=0)

    #
    # we are building the same rev as before?
    local SkipAutoGen=0
    #
    if [[ -f "$CLOVERROOT"/Version.h ]]; then
        local builtedRev=$(cat "$CLOVERROOT"/Version.h  \
                           | grep '#define FIRMWARE_REVISION L' | awk -v FS="(\"|\")" '{print $2}')
#    echo "old revision ${builtedRev}" >echo.txt
#    echo "new revision ${repoRev}" >>echo.txt

        if [ "${repoRev}" = "${builtedRev}" ]; then SkipAutoGen=1; fi
    fi


    export CLOVER_PKG_DIR="$CLOVERROOT"/CloverPackage/CloverV2
    echo "CLOVER_PKG_DIR: $CLOVER_PKG_DIR"

    # Cleaning part of the script if we have told to do it
    if [[ "$TARGETRULE" == cleanpkg ]]; then
#        if [[ "$SYSNAME" != Linux ]]; then
#            # Make some house cleaning
#           echo "Cleaning CloverUpdater files..."
#            make -C "$CLOVERROOT"/CloverPackage/CloverUpdater clean
#
#            echo "Cleaning CloverPrefpane files..."
#            make -C "$CLOVERROOT"/CloverPackage/CloverPrefpane clean
#        fi

        echo "Cleaning bootsector files..."
        local BOOTHFS="$CLOVERROOT"/BootHFS
        DESTDIR="$CLOVER_PKG_DIR"/BootSectors make -C $BOOTHFS clean

        echo
        # Use subshell to use shopt
        (
            echo "Cleaning packaging files..."
            shopt -s nullglob
            find  "$CLOVER_PKG_DIR"/Bootloaders/x64/ -mindepth 1 -not -path "**/.svn*" -delete
            if [[ -d "$CLOVER_PKG_DIR"/EFI/BOOT ]]; then
                find  "$CLOVER_PKG_DIR"/EFI/BOOT/ -name '*.efi' -mindepth 1 -not -path "**/.svn*" -delete
                rmdir "$CLOVER_PKG_DIR"/EFI/BOOT &>/dev/null
            fi
            local dir
            for dir in "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers*; do
                find  "$dir" -mindepth 1 -not -path "**/.svn*" -delete
                rmdir "$dir" &>/dev/null
            done
            find  "$CLOVER_PKG_DIR"/EFI/CLOVER/ -name '*.efi' -maxdepth 1 -not -path "**/.svn*" -delete
            for dir in "$CLOVER_PKG_DIR"/drivers/$DRIVERS_OFF/*; do
                find  "$dir" -mindepth 1 -not -path "**/.svn*" -delete
            done
        )
        echo  "Done!"
        exit $?

    elif [[ "$TARGETRULE" == clean || "$TARGETRULE" == cleanall ]]; then
        build --quiet -p $PLATFORMFILE  -b $BUILDTARGET \
         -t $TOOLCHAIN -n $BUILDTHREADS $TARGETRULE
        [[ "$TARGETRULE" == cleanall ]] && make -C $WORKSPACE/BaseTools clean
        exit $?
    fi

    # Create edk tools if necessary
    if  [[ ! -x "$EDK_TOOLS_PATH/Source/C/bin/GenFv" ]]; then
        echo "Building tools as they are not found"
      if [[ "$SYSNAME" == Linux ]]; then
        echo "Linux"   
        make -C "$WORKSPACE"/BaseTools "BUILD_CC=clang"
      else     
        make -C "$WORKSPACE"/BaseTools CC="gcc -Wno-deprecated-declarations"
      fi
    fi

    # Apply options
    [[ "$USE_BIOS_BLOCKIO" -ne 0 ]]    && addEdk2BuildMacro 'USE_BIOS_BLOCKIO'
    [[ "$VBIOSPATCHCLOVEREFI" -ne 0 ]] && addEdk2BuildMacro 'ENABLE_VBIOS_PATCH_CLOVEREFI'
    [[ "$ONLYSATA0PATCH" -ne 0 ]] && addEdk2BuildMacro 'ONLY_SATA_0'
    [[ "$USE_LOW_EBDA" -ne 0 ]] && addEdk2BuildMacro 'USE_LOW_EBDA'
    [[ -d "$WORKSPACE/MdeModulePkg/Universal/Variable/EmuRuntimeDxe" ]] && addEdk2BuildMacro 'HAVE_LEGACY_EMURUNTIMEDXE'
    [[ "$LLVM" -ne 0 ]] && addEdk2BuildMacro 'LLVM'
    [[ "$ENABLE_MODERN_CPU" -ne 0 ]] && addEdk2BuildMacro 'ENABLE_MODERN_CPU_QUIRKS'

    local cmd="${EDK2_BUILD_OPTIONS[@]}"

    if (( $SkipAutoGen == 1 )) && (( $FORCEREBUILD == 0 )); then
        cmd="build --skip-autogen $cmd"
    else
        cmd="build $cmd"
    fi

    cmd="$cmd -p $PLATFORMFILE $MODULEFILE -a $TARGETARCH -b $BUILDTARGET -DLESS_DEBUG"
    cmd="$cmd -t $TOOLCHAIN -n $BUILDTHREADS $TARGETRULE"

    echo
    echo "Running edk2 build for Clover$TARGETARCH using the command:"
    echo "$cmd"
    echo

    # Build Clover version
    if (( $SkipAutoGen == 0 )) || (( $FORCEREBUILD == 1 )); then

 #     local clover_revision=$(cat "${CLOVERROOT}/${VERSTXT}")     
 #     local clover_revision=$(git describe --tags $(git rev-list --tags --max-count=1﻿))
	    local clover_revision=$(git describe --tags --abbrev=0)
      local clover_build_date=$(date '+%Y-%m-%d %H:%M:%S')
      #echo "#define FIRMWARE_VERSION \"2.31\"" > "$CLOVERROOT"/Version.h

      echo "#define FIRMWARE_BUILDDATE \"${clover_build_date}\"" > "$CLOVERROOT"/Version.h
      echo "#define FIRMWARE_REVISION L\"${clover_revision}\""   >> "$CLOVERROOT"/Version.h

      local sha1="(github unknown)"
      if [[ -d "${CLOVERROOT}"/.git ]]; then
        sha1="($(git rev-parse --abbrev-ref HEAD), commit $(git rev-parse --short HEAD))"
      fi
      echo "#define REVISION_STR \"Clover revision: ${clover_revision} $sha1\"" >> "$CLOVERROOT"/Version.h
#      echo "#define REVISION_STR \"Clover revision: ${clover_revision}\"" >> "$CLOVERROOT"/Version.h

      rev_date=$(git show -s --format=%ci $(git rev-parse HEAD))
      echo "#define REVISION_DATE \"${rev_date}\"" >> "$CLOVERROOT"/Version.h
      COMMIT_HASH="$(git rev-parse HEAD)"
      echo "#define COMMIT_HASH \"$COMMIT_HASH\"" >> "$CLOVERROOT"/Version.h
      #build_id_date="$(date +%Y%m%d%H%M%S)"
      build_id_date="$(git show -s --format=%cd --date=format:%Y%m%d%H%M%S)"
      number_of_commit="$(git rev-list tags/$(git describe --tags --abbrev=0)..HEAD --count)"
      if [ $number_of_commit -gt 0 ]
      then
        build_id="$build_id_date"-"${COMMIT_HASH::7}"
      else
        build_id="$build_id_date"-"${COMMIT_HASH::7}"-"$clover_revision"
      fi
      if [[ -n $(git status -s) ]]
      then
        build_id="$build_id"-dirty
      fi
      echo "#define BUILD_ID \"$build_id\"" >> "$CLOVERROOT"/Version.h

      local clover_build_info="Args: "
      if [[ -n "$@" ]]; then
        clover_build_info="${clover_build_info} $@"
      fi

      clover_build_info="${clover_build_info} | $(echo $cmd | xargs | sed -e "s, -p ${PLATFORMFILE} , ,")"

      if [[ -n "${OSVER:-}" ]]; then
        clover_build_info="${clover_build_info} | OS: ${OSVER}"
      fi
      if [[ -n "${XCODE_VERSION:-}" ]]; then
        clover_build_info="${clover_build_info} | XCODE: ${XCODE_VERSION}"
      fi
      # removing force rebuild related flags, and ensure only one blank space is used as separator
      clover_build_info=$(echo ${clover_build_info} | sed -e 's/ -fr / /' \
                         | sed -e 's/ --force-rebuild / /' | sed -e 's/ --skip-autogen / /' \
                         | sed -e 's/build//' | sed -e 's/Args: | /Args: /' | sed -e 's/  / /')

      echo "#define BUILDINFOS_STR \"${clover_build_info}\"" >> "$CLOVERROOT"/Version.h

#      cp "$CLOVERROOT"/Version.h "$CLOVERROOT"/rEFIt_UEFI/
    fi

    eval "$cmd"
}

copyBin() {
  local cpSrc="$1"
  local cpDest="$2"
  local cpFile=$(basename "$2")
  local cpDestDIR=$(dirname "$cpDest")

  [[ ! -f  "$cpSrc" || ! -d  "$cpDestDIR" ]] && return
  [[ -d  "$cpDest" ]] && cpFile=$(basename "$cpSrc")

  echo "  -> $cpFile"
  cp -f "$cpSrc" "$cpDest" 2>/dev/null
}

setInitBootMsg(){
    local byte="35"
    case "${1}" in
    *boot2)
        byte="32"
    ;;
    *boot3)
        byte="33"
    ;;
    *boot4)
        byte="34"
    ;;
    *boot5)
        byte="35"
    ;;
    *boot6)
        byte="36"
    ;;
    *boot7)
        byte="37"
    ;;
    *boot7-MCP79)
        byte="4d"
    ;;
    *boot8)
        byte="38"
    ;;
    *boot9)
        byte="39"
    ;;
    *)
        return;
    ;;
    esac

    if [[ -f "${1}" ]]; then
        echo -e "Changing byte at 0xa9 of $(basename ${1}) to show \x${byte} as init message:"
        printf "\x${byte}" | dd conv=notrunc of="${1}" bs=1 seek=$((0xa9))
    fi
}

# Deploy Clover files for packaging
MainPostBuildScript() {
#  if [[ -z "$EDK_TOOLS_PATH" ]]; then
    export BASETOOLS_DIR="$WORKSPACE"/BaseTools/Source/C/bin
#  else
#    export BASETOOLS_DIR="$EDK_TOOLS_PATH"/Source/C/bin
#  fi
  export BOOTSECTOR_BIN_DIR="$CLOVERROOT"/CloverEFI/BootSector/bin
	if (( $NOBOOTFILES == 0 )); then
    echo Compressing DUETEFIMainFv.FV ... at "${BUILD_DIR}/FV/DUETEFIMAINFV${TARGETARCH}.Fv"
    "$BASETOOLS_DIR"/LzmaCompress -e -o "${BUILD_DIR}/FV/DUETEFIMAINFV${TARGETARCH}.z" "${BUILD_DIR}/FV/DUETEFIMAINFV${TARGETARCH}.Fv"

    echo Compressing DxeCore.efi ... "$BUILD_DIR_ARCH/DxeCore.efi"
    "$BASETOOLS_DIR"/LzmaCompress -e -o "${BUILD_DIR}/FV/DxeMain${TARGETARCH}.z" "$BUILD_DIR_ARCH/DxeCore.efi"

    echo Compressing DxeIpl.efi ...
    "$BASETOOLS_DIR"/LzmaCompress -e -o "${BUILD_DIR}/FV/DxeIpl${TARGETARCH}.z" "$BUILD_DIR_ARCH/DxeIpl.efi"

    echo "Generate Loader Image ..."
	fi

  cloverEFIFile=boot$((6 + USE_BIOS_BLOCKIO))
  if (( $NOBOOTFILES == 0 )); then
    "$BASETOOLS_DIR"/GenFw --rebase 0x10000 -o "$BUILD_DIR_ARCH/EfiLoader.efi" "$BUILD_DIR_ARCH/EfiLoader.efi"
    "$BASETOOLS_DIR"/EfiLdrImage -o "${BUILD_DIR}"/FV/Efildr64 \
    "$BUILD_DIR_ARCH"/EfiLoader.efi                \
    "${BUILD_DIR}"/FV/DxeIpl${TARGETARCH}.z        \
    "${BUILD_DIR}"/FV/DxeMain${TARGETARCH}.z       \
    "${BUILD_DIR}"/FV/DUETEFIMAINFV${TARGETARCH}.z
    if [[ "$GENPAGE" -eq 0 && "$USE_LOW_EBDA" -ne 0 ]]; then
      if [[ "$SYSNAME" == Linux ]]; then
        local -r EL_SIZE=$(stat -c "%s" "${BUILD_DIR}"/FV/Efildr64)
      else
        local -r EL_SIZE=$(stat -f "%z" "${BUILD_DIR}"/FV/Efildr64)
      fi
      if (( $((EL_SIZE)) > 417792 )); then
        echo 'warning: boot file bigger than low-ebda permits, switching to --std-ebda'
        USE_LOW_EBDA=0
      fi
    fi

    local -ar COM_NAMES=(H H2 H3 H4 H5 H6 H5 H6)           # Note: (H{,2,3,4,5,6,5,6}) works in Linux bash, but not Darwin bash
    startBlock=Start64${COM_NAMES[$((GENPAGE << 2 | USE_LOW_EBDA << 1 | USE_BIOS_BLOCKIO))]}.com
    if [[ "$GENPAGE" -ne 0 ]]; then
      cat $BOOTSECTOR_BIN_DIR/$startBlock $BOOTSECTOR_BIN_DIR/efi64.com3 "${BUILD_DIR}"/FV/Efildr64 > "${BUILD_DIR}"/FV/boot
    else
      cat $BOOTSECTOR_BIN_DIR/$startBlock $BOOTSECTOR_BIN_DIR/efi64.com3 "${BUILD_DIR}"/FV/Efildr64 > "${BUILD_DIR}"/FV/Efildr20Pure

      if [[ "$USE_LOW_EBDA" -ne 0 ]]; then
        "$BASETOOLS_DIR"/GenPage "${BUILD_DIR}"/FV/Efildr20Pure -b 0x88000 -f 0x68000 -o "${BUILD_DIR}"/FV/Efildr20
      else
        "$BASETOOLS_DIR"/GenPage "${BUILD_DIR}"/FV/Efildr20Pure -o "${BUILD_DIR}"/FV/Efildr20
      fi
      # Create CloverEFI file
      dd if="${BUILD_DIR}"/FV/Efildr20 of="${BUILD_DIR}"/FV/boot bs=512 skip=1
    fi

    rm -rf "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers 2> /dev/null

    # clean old drivers directories
    if [[ "$DRIVERS_LEGACY" != drivers64 ]]; then
      rm -rf "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers64
    fi

    if [[ "$DRIVERS_UEFI" != drivers64UEFI ]]; then
      rm -rf "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers64UEFI
    fi

    rm -rf "$CLOVER_PKG_DIR"/CloverV2/drivers-Off

    # Be sure that all needed directories exists
    mkdir -p "$CLOVER_PKG_DIR"/Bootloaders/x64
    mkdir -p "$CLOVER_PKG_DIR"/EFI/BOOT
    mkdir -p "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers/$DRIVERS_LEGACY
    mkdir -p "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers/$DRIVERS_UEFI
    # off drivers
    mkdir -p "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_LEGACY/FileVault2
    mkdir -p "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI/FileVault2
    mkdir -p "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI/MemoryFix
    mkdir -p "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_LEGACY/FileSystem
    mkdir -p "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI/FileSystem
    mkdir -p "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI/HID
    mkdir -p "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI/Other

    # Install CloverEFI file
    echo "Copy CloverEFI:"
    copyBin "${BUILD_DIR}"/FV/boot "$CLOVER_PKG_DIR"/Bootloaders/x64/$cloverEFIFile
    # For GENPAGE, the character "[TX]" is at offset 0x74 of Start64H[56].com, not offset 0xa9 - zenith432
    if [[ "$GENPAGE" -eq 0 ]]; then
      setInitBootMsg "$CLOVER_PKG_DIR"/Bootloaders/x64/$cloverEFIFile
    fi
    copyBin "$BUILD_DIR_ARCH"/CLOVERX64.efi "$CLOVER_PKG_DIR"/EFI/BOOT/BOOTX64.efi
    copyBin "$BUILD_DIR_ARCH"/CLOVERX64.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/CLOVERX64.efi

    # Mandatory drivers
    echo "Copy Mandatory drivers:"
    binArray=( XhciDxe EnglishDxe )
    for efi in "${binArray[@]}"
    do
      copyBin "$BUILD_DIR_ARCH"/$efi.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers/$DRIVERS_LEGACY/$efi.efi
    done

#    binArray=( AppleImageCodec AppleKeyAggregator )
#    for efi in "${binArray[@]}"
#    do
#      copyBin "$BUILD_DIR_ARCH"/$efi.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_LEGACY/FileVault2/$efi.efi
#    done

    binArray=( ApfsDriverLoader )
    for efi in "${binArray[@]}"
    do
      copyBin "$BUILD_DIR_ARCH"/$efi.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_LEGACY/FileSystem/$efi.efi
    done

    if [[ $M_APPLEHFS -eq 1 ]]; then
      copyBin "${CLOVERROOT}"/FileSystems/HFSPlus/X64/HFSPlus.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_LEGACY/FileSystem/HFSPlus.efi
    fi


    binArray=( FSInject AudioDxe )
    for efi in "${binArray[@]}"
    do
      copyBin "$BUILD_DIR_ARCH"/$efi.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers/$DRIVERS_UEFI/$efi.efi
    done

#    binArray=( AppleImageCodec AppleKeyAggregator )
#    for efi in "${binArray[@]}"
#    do
#      copyBin "$BUILD_DIR_ARCH"/$efi.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI/FileVault2/$efi.efi
#    done

    if [[ $M_NOGRUB -eq 0 ]]; then
      binArray=( GrubEXFAT GrubISO9660 GrubNTFS GrubUDF GrubZFS GrubUFS GrubUFS2 )
      for efi in "${binArray[@]}"
      do
        copyBin "$BUILD_DIR_ARCH"/$efi.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_LEGACY/FileSystem/$efi.efi
      done
    fi

    # drivers64UEFI
    binArray=( CsmVideoDxe EnglishDxe EmuVariableUefi NvmExpressDxe OsxFatBinaryDrv PartitionDxe )

    for efi in "${binArray[@]}"
    do
      copyBin "$BUILD_DIR_ARCH"/$efi.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI/Other/$efi.efi
    done

    binArray=( Ps2MouseDxe UsbKbDxe UsbMouseDxe AptioInputFix )

    for efi in "${binArray[@]}"
    do
      copyBin "$BUILD_DIR_ARCH"/$efi.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI/HID/$efi.efi
    done

    binArray=( ApfsDriverLoader Fat VBoxExt2 Ext4Dxe VBoxIso9600 VBoxHfs )

    for efi in "${binArray[@]}"
    do
      copyBin "$BUILD_DIR_ARCH"/$efi.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI/FileSystem/$efi.efi
    done

    if [[ $M_APPLEHFS -eq 1 ]]; then
      copyBin "${CLOVERROOT}"/FileSystems/HFSPlus/X64/HFSPlus.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI/FileSystem/HFSPlus.efi
    fi

    # drivers64UEFI/FileVault2
    binArray=( AppleKeyFeeder HashServiceFix )

    for efi in "${binArray[@]}"
    do
      copyBin "$BUILD_DIR_ARCH"/$efi.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI/FileVault2/$efi.efi
    done

    # drivers64UEFI/MemoryFix
    binArray=( OpenRuntime )

    for efi in "${binArray[@]}"
    do
      copyBin "$BUILD_DIR_ARCH"/$efi.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers/$DRIVERS_OFF/$DRIVERS_UEFI/MemoryFix/$efi.efi
    done
    
    # Applications
    echo "Copy Applications:"
    copyBin "$BUILD_DIR_ARCH"/bdmesg.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/tools/
    copyBin "$BUILD_DIR_ARCH"/ControlMsrE2.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/tools/

    if [[ "${EDK2SHELL:-}" == "MinimumShell" ]]; then
      copyBin "${WORKSPACE}"/ShellBinPkg/MinUefiShell/X64/Shell.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/tools/Shell64U.efi
    elif [[ "${EDK2SHELL:-}" == "FullShell" ]]; then
      copyBin "${WORKSPACE}"/ShellBinPkg/UefiShell/X64/Shell.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/tools/Shell64U.efi
    else
      copyBin "$BUILD_DIR_ARCH"/Shell.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/tools/Shell64U.efi
    fi
  fi

  echo "Done!"

  # Build and install Bootsectors
  echo
  echo "Generating BootSectors"
  local BOOTHFS="$CLOVERROOT"/BootHFS
  DESTDIR="$CLOVER_PKG_DIR"/BootSectors make -C $BOOTHFS

}

# BUILD START #

# Default locale
export LC_ALL=POSIX

startBuildEpoch=$(date -u "+%s")

checkCmdlineArguments $@

if [[ "$SYSNAME" == Linux ]]; then
  export TOOLCHAIN=GCC152
  TOOLCHAIN_DIR=${TOOLCHAIN_DIR:-/usr}
else
  if [[ "$TOOLCHAIN" == XCLANG ]]; then
    TOOLCHAIN_DIR=/opt/local
  elif [[ -d ~/src/opt/local ]]; then
    TOOLCHAIN_DIR=~/src/opt/local
  else
    TOOLCHAIN_DIR=${TOOLCHAIN_DIR:-"$CLOVERROOT"/toolchain}
  fi
  export DIR_MAIN=${DIR_MAIN:-"$CLOVERROOT"/toolchain}
fi
export TOOLCHAIN_DIR
echo "TOOLCHAIN_DIR: $TOOLCHAIN_DIR"

# Add toolchain bin directory to the PATH
if [[ "$SYSNAME" != Linux ]]; then
  pathmunge "$TOOLCHAIN_DIR/bin"
fi

MainBuildScript $@

export BUILD_DIR="${WORKSPACE}/Build/Clover/${BUILDTARGET}_${TOOLCHAIN}"
export BUILD_DIR_ARCH="${BUILD_DIR}/$TARGETARCH"

rm -rf ${WORKSPACE}/Build/*.efi
rm -rf ${WORKSPACE}/Build/*.zip

#extract build_id from efi instead of Version.h to be 100% sure that name correspond to actual content.
dstFileName=CloverX64-"$BUILDTARGET"_"$TOOLCHAIN"-"$(grep -aEo "CloverBuildIdGrepTag: [^[:cntrl:]]*" < "$BUILD_DIR_ARCH"/CLOVERX64.efi | sed "s/CloverBuildIdGrepTag: //")"

copyBin "$BUILD_DIR_ARCH"/CLOVERX64.efi ${WORKSPACE}/Build/"$dstFileName".efi
rm -f ${WORKSPACE}/Build/"$dstFileName".zip
zip ${WORKSPACE}/Build/"$dstFileName".zip ${WORKSPACE}/Build/"$dstFileName".efi

if [[ -z $MODULEFILE  ]] && (( $NOBOOTFILES == 0 )); then
    MainPostBuildScript
else
 copyBin "$BUILD_DIR_ARCH"/CLOVERX64.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/CLOVERX64.efi
 copyBin "$BUILD_DIR_ARCH"/CLOVERX64.efi "$CLOVER_PKG_DIR"/EFI/BOOT/BOOTX64.efi
fi

# Local Variables:      #
# mode: ksh             #
# tab-width: 4          #
# indent-tabs-mode: nil #
# End:                  #
#
# vi: set expandtab ts=4 sw=4 sts=4: #

echo "Done!"
stopBuildEpoch=$(date -u "+%s")
buildTime=$(expr $stopBuildEpoch - $startBuildEpoch)
if [[ $buildTime -ge 3600 ]]; then
	timeToBuild=$(printf "%dh%dm%ds" $((buildTime/3600)) $((buildTime/60%60)) $((buildTime%60)))
elif [[ $buildTime -gt 59 ]]; then
    timeToBuild=$(printf "%dm%ds" $((buildTime/60%60)) $((buildTime%60)))
else
    timeToBuild=$(printf "%ds" $((buildTime)))
fi

printf -- "\n* %s %s %s\n" "Clover build process took " "$timeToBuild" " to complete..."

