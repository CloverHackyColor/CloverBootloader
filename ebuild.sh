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
if [[ "$SYSNAME" == Linux ]]; then
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

SCRIPT_VERS="2018-06-18"

# Macro
M_NOGRUB=0
M_APPLEHFS=0

# Default values
export TOOLCHAIN=XCODE8
export TARGETARCH=X64
export BUILDTARGET=RELEASE
export BUILDTHREADS=$(( NUMBER_OF_CPUS + 1 ))
export WORKSPACE=${WORKSPACE:-}
export CONF_PATH=${CONF_PATH:-}
export EXT_DOWNLOAD=0 # 0 = don't download, 1 = download precompiled, 2 = checkout & build, 3 = build
#export NASM_PREFIX=

# if building through Xcode, then TOOLCHAIN_DIR is not defined
# checking if it is where CloverGrowerPro put it
if [[ "$SYSNAME" == Linux ]]; then
  export TOOLCHAIN=GCC53
  TOOLCHAIN_DIR=${TOOLCHAIN_DIR:-/usr}
else
  TOOLCHAIN_DIR=${TOOLCHAIN_DIR:-"$CLOVERROOT"/../../toolchain}
fi
if [[ ! -d $TOOLCHAIN_DIR ]]; then
  TOOLCHAIN_DIR="${PWD}"/../../opt/local
fi
export TOOLCHAIN_DIR
echo "TOOLCHAIN_DIR: $TOOLCHAIN_DIR"

VBIOSPATCHCLOVEREFI=0
ONLYSATA0PATCH=0
USE_BIOS_BLOCKIO=0
USE_LOW_EBDA=1
CLANG=0
GENPAGE=0

FORCEREBUILD=0
NOBOOTFILES=0

declare -r GIT=`which git`
#declare -r GITDIR=`git status 2> /dev/null`        # unsafe as git repository may exist in parent directory
declare -r VERSTXT="vers.txt"
if [[ -x "/usr/bin/sw_vers" ]]; then
  declare -r OSVER="$(sw_vers -productVersion | sed -e 's/\.0$//g')"
elif [[ -x "/usr/bin/lsb_release" ]]; then
  # Linux print the name+version in in two lines, sed serves to made it in one line!
  # ..otherwise Clover fail because Version.h will have a line with no null terminated char.
  declare -r OSVER="$(lsb_release -sir | sed -e ':a;N;$!ba;s/\n/ /g')"
fi
PATCH_FILE=

# Bash options
set -e # errexit
set -u # Blow on unbound variable

## FUNCTIONS ##

function exitTrap() {
    if [[ -n "$PATCH_FILE" && -n "$WORKSPACE" ]]; then
        echo -n "Unpatching edk2..."
        ( cd "$WORKSPACE" && cat "$CLOVERROOT"/Patches_for_EDK2/$PATCH_FILE | eval "$PATCH_CMD -p0 -R" &>/dev/null )
        if [[ $? -eq 0 ]]; then
            echo " done"
        else
            echo " failed"
        fi
    fi
}

# Check if we need to patch the sources
checkPatch() {
  #if [[ -x /usr/bin/git ]]; then
  #    PATCH_CMD="/usr/bin/git apply --whitespace=nowarn"
  if [[ -n "${GIT}" ]]; then
      PATCH_CMD="${GIT} apply --whitespace=nowarn"
  else
      PATCH_CMD="/usr/bin/patch"
  fi

  checkToolchain

  if [[ "$SYSNAME" == Linux ]]; then
    export GCC53_BIN="$TOOLCHAIN_DIR/bin/"
    if [[ ! -x "${GCC53_BIN}gcc" ]]; then
        echo "No clover toolchain found !" >&2
        echo "Install on your system or define the TOOLCHAIN_DIR variable." >&2
        exit 1
    fi
  else
    if [[ -n "${XCODE_BUILD:-}" ]]; then
      #declare -r XCODE_MAJOR_VERSION="$(xcodebuild -version | sed -nE 's/^Xcode ([0-9]).*/\1/p')"
      XCODE_VERSION="$(echo `$XCODE_BUILD -version` | sed -nE 's/^Xcode ([0-9.]+).*/\1/p')"
      declare -r XCODE_MAJOR_VERSION="$(echo $XCODE_VERSION | cut -d. -f1)"

      case "$XCODE_MAJOR_VERSION" in
          5) PATCH_FILE=;;
      esac
    fi

    export GCC53_BIN="$TOOLCHAIN_DIR/cross/bin/x86_64-clover-linux-gnu-"
    if [[ $TOOLCHAIN == GCC* ]] && [[ ! -x "${GCC53_BIN}gcc" ]]; then
      echo "No clover toolchain found !" >&2
      echo "Build it with the build_gcc8.sh script or define the TOOLCHAIN_DIR variable." >&2
      exit 1
    fi
  fi

# Linux does not come with nasm installed!

#  if [[ ! -x "$TOOLCHAIN_DIR"/bin/nasm ]]; then
#      echo "No nasm binary found in toolchain directory !" >&2
#      if [[ "$SYSNAME" != Linux ]]; then
#        echo "Build it with the buildnasm.sh script." >&2
#      fi
#      exit 1
#  fi

  if [[ -f "/opt/local/bin/nasm" ]]; then
    export NASM_PREFIX="/opt/local/bin/"
  elif [[ -f "${TOOLCHAIN_DIR}/bin/nasm" ]]; then
    # using $TOOLCHAIN_DIR here should allow Clover source to be
    # inside any sub folder instead of only in ~/
    export NASM_PREFIX="${TOOLCHAIN_DIR}/bin/"
  else
    export NASM_PREFIX=""
  fi

  echo "NASM_PREFIX: $NASM_PREFIX"

  #NASM_VER=`nasm -v | awk '/version/ {print $3}'`
  NASM_VER=`${NASM_PREFIX}nasm -v | sed -nE 's/^.*version.([0-9\.]+).*$/\1/p'`

  echo "NASM_VER: $NASM_VER"
  if [[ "$SYSNAME" == Darwin ]]; then
    if ! isNASMGood "${NASM_PREFIX}nasm"; then echo "your nasm is not good to build Clover!" && exit 1; fi
  fi
}

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
needNASM() {
  local nasmPath=""
  local nasmArray=( $(which -a nasm) )
  local needInstall=1
  local good=""

  if [ ${#nasmArray[@]} -ge "1" ]; then

    for i in "${nasmArray[@]}"
    do
      echo "found nasm v$(${i} -v | grep 'NASM version' | awk '{print $3}') at $(dirname ${i})"
    done

    # we have a good nasm?
    for i in "${nasmArray[@]}"
    do
      if isNASMGood "${i}"; then
        good="${i}"
        break
      fi
    done

    if [[ -x "${good}" ]] ; then
      # only nasm at index 0 is used!
      if [[ "${good}" == "${nasmArray[0]}" ]]; then
        echo "${good} is ok.."
      else
        echo "this one is good:"
        echo "${good}"
      fi
    else
      # no nasm versions suitable for Clover
      echo "nasm found, but is not good to build Clover.."
      needInstall=0
    fi
  else
    needInstall=0
    echo "nasm not found.."
  fi
  return $needInstall
}

isNASMGood() {
  # nasm should be greater or equal to 2.12.02 to be good building Clover.
  # There was a bad macho relocation in outmacho.c, fixed by Zenith432
  # and accepted by nasm devel during 2.12.rcxx (release candidate)

  result=1
  local nasmver=$( "${1}" -v | grep 'NASM version' | awk '{print $3}' )

  case "$nasmver" in
  2.12.0[2-9]* | 2.12.[1-9]* | 2.1[3-9]* | 2.[2-9]* | [3-9]* | [1-9][1-9]*)
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
    local CLOVERBIN="${CLOVERROOT}/BuildTools/usr/local/bin"

    if [[ ! -x "${XCODE_BUILD}" ]]; then
       echo "ERROR: Install Xcode Tools from Apple before using this script." >&2; exit 1
    fi

  if [[ -f "/opt/local/bin/mtoc.NEW" ]]; then
    export MTOC_PREFIX="/opt/local/bin/"
  elif [[ -f "${LOCALBIN}/mtoc.NEW" ]]; then
    export MTOC_PREFIX="${LOCALBIN}/"
  elif [[ -f "${TOOLCHAIN_DIR}/bin/mtoc.NEW" ]]; then
    export MTOC_PREFIX="${TOOLCHAIN_DIR}/bin/"
  elif [[ -f "${CLOVERBIN}/mtoc.NEW" ]]; then
    # using $TOOLCHAIN_DIR here should allow Clover source to be
    # inside any sub folder instead of only in ~/
    export MTOC_PREFIX="${CLOVERBIN}/"
  else
    ./buildmtoc.sh
    export MTOC_PREFIX="${TOOLCHAIN_DIR}/bin/"
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
    print_option_help "-clang"     "use XCode Clang toolchain"
    print_option_help "-llvm"      "use LLVM toolchain"
    print_option_help "-gcc49"     "use GCC 4.9 toolchain"
    print_option_help "-gcc53"     "use GCC 5.3 toolchain"
    print_option_help "-unixgcc"   "use UNIXGCC toolchain"
    print_option_help "-xcode"     "use XCode 3.2 toolchain"
    print_option_help "-xcode5"     "use XCode 5-7 toolchain "
    print_option_help "-xcode8"     "use XCode 8 toolchain  [Default]"
    print_option_help "-t TOOLCHAIN, --tagname=TOOLCHAIN" "force to use a specific toolchain"
    echo
    echo "Target:"
    print_option_help "-ia32"      "build Clover in 32-bit [boot3]"
    print_option_help "-x64"       "build Clover in 64-bit [boot6] [Default]"
    print_option_help "-mc, --x64-mcp"   "build Clover in 64-bit [boot7] using BiosBlockIO (compatible with MCP chipset)"
    print_option_help "-a TARGETARCH, --arch=TARGETARCH" "overrides target.txt's TARGET_ARCH definition"
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
    print_option_help "--ext-pre" "enable external driver download"
    print_option_help "--ext-co" "checkout & build external drivers at ..src/EXT_PACKAGES"
    print_option_help "--ext-build" "build existing external drivers located at ..src/EXT_PACKAGES"
    print_option_help "--edk2shell <MinimumShell|FullShell>" "copy edk2 Shell to EFI tools dir"
    echo
    echo "build options:"
    print_option_help "-fr, --force-rebuild" "force rebuild all targets"
    print_option_help "-nb, --no-bootfiles" "don't generate boot files"
    echo
    echo "Report bugs to https://sourceforge.net/p/cloverefiboot/discussion/1726372/"
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
            -clang  | --clang)   TOOLCHAIN=XCLANG ; CLANG=1 ;;
            -llvm   | --llvm)    TOOLCHAIN=LLVM  ; CLANG=1 ;;
            -xcode5  | --xcode5 )  TOOLCHAIN=XCODE5 ; CLANG=1 ;;
            -xcode8  | --xcode8 )  TOOLCHAIN=XCODE8 ; CLANG=1 ;;
            -GCC49  | --GCC49)   TOOLCHAIN=GCC49   ;;
            -gcc49  | --gcc49)   TOOLCHAIN=GCC49   ;;
            -GCC53  | --GCC53)   TOOLCHAIN=GCC53   ;;
            -gcc53  | --gcc53)   TOOLCHAIN=GCC53   ;;
            -unixgcc | --gcc)    TOOLCHAIN=UNIXGCC ;;
            -xcode  | --xcode )  TOOLCHAIN=XCODE32 ;;
            -ia32 | --ia32)      TARGETARCH=IA32   ;;
            -x64 | --x64)        TARGETARCH=X64    ;;
            -mc | --x64-mcp)     TARGETARCH=X64 ; USE_BIOS_BLOCKIO=1 ;;
            -clean)    TARGETRULE=clean ;;
            -cleanall) TARGETRULE=cleanall ;;
            -fr | --force-rebuild) FORCEREBUILD=1 ;;
            -nb | --no-bootfiles) NOBOOTFILES=1 ;;
#            -d | -debug | --debug)  BUILDTARGET=DEBUG ;;
#            -r | -release | --release) BUILDTARGET=RELEASE ;;
            -a) TARGETARCH=$(argument $option "$@"); shift
                ;;
            --arch=*)
                TARGETARCH=$(echo "$option" | sed 's/--arch=//')
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
            --no-usb)
                addEdk2BuildMacro DISABLE_USB_SUPPORT
                ;;
            --no-lto)
                addEdk2BuildMacro DISABLE_LTO
                ;;
            --ext-pre)
                EXT_DOWNLOAD=1
                ;;
            --ext-co)
                EXT_DOWNLOAD=2
                ;;
            --ext-build)
                EXT_DOWNLOAD=3
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
    PLATFORMFILE="${PLATFORMFILE:-Clover/Clover.dsc}"
    if [ ! -z "${MODULEFILE}" ]; then
        MODULEFILE=" -m Clover/$MODULEFILE"
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
    esac
}

checkExtTools() {
    case "$EXT_DOWNLOAD" in
        1)
          if [[ ! -x $(which wget) ]] && [[ ! -x $(which curl) ]]; then
            echo "Missing wget or curl, will not download external drivers!"
            EXT_DOWNLOAD=0
          fi ;;
        2 | 3)
          if [[ ! -x $(which git) ]]; then
            echo "Missing git, can't clone external drivers!"
            EXT_DOWNLOAD=0
          fi ;;
    esac
}

downloadExtDriver() {
    # Downloads latest zip release from GitHub.
    # Usage: downloadExtDriver "org/project" "ProjectName" "ReleasePrefix" "ReleaseSuffix"
    # Release filename is made of "${ReleasePrefix}${ReleaseTag}${ReleaseSuffix}.zip"
    if [ -d tmp ]; then
        echo "tmp dir already exists, aborting!"
        return
    fi

    url="https://api.github.com/repos/${1}/releases/latest"
    if [ "$(which curl)" != "" ]; then
        vers=$(curl -Ls "${url}" | grep '"tag_name":' | sed -E 's/.*"([^"]+)".*/\1/')
    else
        vers=$(wget -qO - "${url}" | grep '"tag_name":' | sed -E 's/.*"([^"]+)".*/\1/')
    fi

    if [ "$vers" = "" ]; then
        echo "Skipping ${2} due to unknown version!"
        return
    fi

    echo "  -> Downloading ${2} version ${vers}..."

    url="https://github.com/${1}/releases/download/${vers}/${3}${vers}${4}.zip"
    ret=0
    if [ "$(which curl)" != "" ]; then
        curl -sOL "${url}" || ret=1
    else
        wget -qO "${url}" || ret=1
    fi

    if [[ "$ret" -ne 0 ]] || [ ! -f "${3}${vers}${4}.zip" ]; then
        echo "Failed to download ${2}!"
        return
    fi

    ret=0; mkdir tmp || ret=1
    if [[ "$ret" -eq 0 ]]; then
        ret=0; cd tmp || ret=1
        if [[ "$ret" -eq 0 ]]; then
            ret=0; unzip -q ../"${3}${vers}${4}.zip" || ret=1
            if [[ "$ret" -eq 0 ]]; then
                ret=0; cp Drivers/*.efi "$CLOVER_PKG_DIR"/drivers-Off/drivers64UEFI/ || ret=1
                if [[ "$ret" -ne 0 ]]; then
                    echo "Failed to copy ${2} drivers!"
                fi
            else
                echo "Failed to unzip to ${3}${vers}${4}.zip!"
            fi
            cd - &>/dev/null
        else
            echo "Failed to cd to tmp dir!"
        fi
    else
        echo "Failed to create tmp dir!"
    fi

    rm -rf "${3}${vers}${4}.zip" tmp
}

# Main build script
MainBuildScript() {
    checkCmdlineArguments $@
    #checkToolchain
    checkExtTools
    checkPatch

#    echo "NASM_PREFIX: ${NASM_PREFIX}"

    local repoRev="0000"
    if [[ -d .svn ]]; then
#        repoRev=$(svnversion -n | tr -d [:alpha:])
		repoRev=$(svn info | grep "Revision" | tr -cd [:digit:])
    elif [[ -d .git ]]; then
        repoRev=$(git svn find-rev git-svn | tr -cd [:digit:])
    fi

    echo -n "${repoRev}" > "${VERSTXT}"

    #
    # we are building the same rev as before?
    local SkipAutoGen=0
    #
    if [[ -f "$CLOVERROOT"/rEFIt_UEFI/Version.h ]]; then
        local builtedRev=$(cat "$CLOVERROOT"/rEFIt_UEFI/Version.h  \
                           | grep '#define FIRMWARE_REVISION L' | awk -v FS="(\"|\")" '{print $2}')
#    echo "old revision ${builtedRev}" >echo.txt
#    echo "new revision ${repoRev}" >>echo.txt

        if [ "${repoRev}" = "${builtedRev}" ]; then SkipAutoGen=1; fi
    fi

    #
    # Setup workspace if it is not set
    #
    local EDK2DIR=$(cd "$CLOVERROOT"/.. && echo "$PWD")
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
        cd "$EDK2DIR"
        export EDK_TOOLS_PATH="${PWD}"/BaseTools
        set +u
        source ./edksetup.sh BaseTools
        set -u
        cd "$CLOVERROOT"
    else
        echo "Building from: $WORKSPACE"
    fi

    # Trying to patch edk2
#    if [[ -n "$PATCH_FILE" ]]; then
#        echo -n "Patching edk2..."
#        ( cd "$WORKSPACE" && cat "$CLOVERROOT"/Patches_for_EDK2/$PATCH_FILE | eval "$PATCH_CMD -p0" &>/dev/null )
#        if [[ $? -eq 0 ]]; then
#            echo " done"
#        else
#            echo " failed"
#        fi
#    fi

    export CLOVER_PKG_DIR="$CLOVERROOT"/CloverPackage/CloverV2

    # Cleaning part of the script if we have told to do it
    if [[ "$TARGETRULE" == cleanpkg ]]; then
        if [[ "$SYSNAME" != Linux ]]; then
            # Make some house cleaning
            echo "Cleaning CloverUpdater files..."
            make -C "$CLOVERROOT"/CloverPackage/CloverUpdater clean

            echo "Cleaning CloverPrefpane files..."
            make -C "$CLOVERROOT"/CloverPackage/CloverPrefpane clean
        fi

        echo "Cleaning bootsector files..."
        local BOOTHFS="$CLOVERROOT"/BootHFS
        DESTDIR="$CLOVER_PKG_DIR"/BootSectors make -C $BOOTHFS clean

        echo
        # Use subshell to use shopt
        (
            echo "Cleaning packaging files..."
            shopt -s nullglob
            find  "$CLOVER_PKG_DIR"/Bootloaders/{ia32,x64}/ -mindepth 1 -not -path "**/.svn*" -delete
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
            for dir in "$CLOVER_PKG_DIR"/drivers-Off/drivers*; do
                find  "$dir" -mindepth 1 -not -path "**/.svn*" -delete
            done
        )
        echo  "Done!"
        exit $?

    elif [[ "$TARGETRULE" == clean || "$TARGETRULE" == cleanall ]]; then
        build --quiet -p $PLATFORMFILE -a $TARGETARCH -b $BUILDTARGET \
         -t $TOOLCHAIN -n $BUILDTHREADS $TARGETRULE
        [[ "$TARGETRULE" == cleanall ]] && make -C $WORKSPACE/BaseTools clean
        exit $?
    fi

    # Create edk tools if necessary
    if  [[ ! -x "$EDK_TOOLS_PATH/Source/C/bin/GenFv" ]]; then
        echo "Building tools as they are not found"
        make -C "$WORKSPACE"/BaseTools CC="gcc -Wno-deprecated-declarations"
    fi

    # Apply options
    [[ "$USE_BIOS_BLOCKIO" -ne 0 ]]    && addEdk2BuildMacro 'USE_BIOS_BLOCKIO'
    [[ "$VBIOSPATCHCLOVEREFI" -ne 0 ]] && addEdk2BuildMacro 'ENABLE_VBIOS_PATCH_CLOVEREFI'
    [[ "$ONLYSATA0PATCH" -ne 0 ]] && addEdk2BuildMacro 'ONLY_SATA_0'
    [[ "$USE_LOW_EBDA" -ne 0 ]] && addEdk2BuildMacro 'USE_LOW_EBDA'
    [[ -d "$WORKSPACE/MdeModulePkg/Universal/Variable/EmuRuntimeDxe" ]] && addEdk2BuildMacro 'HAVE_LEGACY_EMURUNTIMEDXE'
    [[ "$CLANG" -ne 0 ]] && addEdk2BuildMacro 'CLANG'

    local cmd="${EDK2_BUILD_OPTIONS[@]}"

    if (( $SkipAutoGen == 1 )) && (( $FORCEREBUILD == 0 )); then
        cmd="build --skip-autogen $cmd"
    else
        cmd="build $cmd"
    fi

    cmd="$cmd -p $PLATFORMFILE $MODULEFILE -a $TARGETARCH -b $BUILDTARGET"
    cmd="$cmd -t $TOOLCHAIN -n $BUILDTHREADS $TARGETRULE"

    echo
    echo "Running edk2 build for Clover$TARGETARCH using the command:"
    echo "$cmd"
    echo

    # Build Clover version
    if (( $SkipAutoGen == 0 )) || (( $FORCEREBUILD == 1 )); then
      local clover_revision=$(cat "${CLOVERROOT}/${VERSTXT}")
      local clover_build_date=$(date '+%Y-%m-%d %H:%M:%S')
      #echo "#define FIRMWARE_VERSION \"2.31\"" > "$CLOVERROOT"/Version.h
      echo "#define FIRMWARE_BUILDDATE \"${clover_build_date}\"" > "$CLOVERROOT"/Version.h
      echo "#define FIRMWARE_REVISION L\"${clover_revision}\""   >> "$CLOVERROOT"/Version.h
      echo "#define REVISION_STR \"Clover revision: ${clover_revision}\"" >> "$CLOVERROOT"/Version.h

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

      cp "$CLOVERROOT"/Version.h "$CLOVERROOT"/rEFIt_UEFI/
    fi

    eval "$cmd"

    # looks for external drivers to build
    local EXT_PACKAGES="$EDK2DIR"/../EXT_PACKAGES

    # add github links below to checkout packages
    local extDriversDependecies=( 'https://github.com/acidanthera/AptioFixPkg'
                                  'https://github.com/acidanthera/AppleSupportPkg'
                                  'https://github.com/acidanthera/OcSupportPkg'
                                  'https://github.com/acidanthera/EfiPkg')
    # add below drivers you want to build
    local externalDrivers=( AptioFixPkg AppleSupportPkg )

  # if [[ $TOOLCHAIN == XCODE* ]]; then # this can be also for XCODE8... waiting the mantainer to do a little fix
  # described here: https://www.insanelymac.com/forum/topic/306156-clover-problems-and-solutions/?do=findComment&comment=2638177
    if [[ $TOOLCHAIN == XCODE5 ]]; then
      extDriversDependecies+=( 'https://github.com/acidanthera/VirtualSMC' )
      externalDrivers+=( VirtualSmcPkg )
    fi

    if [[ "$EXT_DOWNLOAD" -eq 2 ]]; then
      local pkg=""
      for link in "${extDriversDependecies[@]}"
      do
        mkdir -p "$EXT_PACKAGES"
        pkg=$(basename $link)

        rm -rf "${EXT_PACKAGES}/${pkg}"

        local branch=master

        case $pkg in
        OcSupportPkg | EfiPkg)
          branch=master
        ;;
        esac

        cmd="git clone $link -b $branch ${EXT_PACKAGES}/${pkg}"
        eval "$cmd"
        if [[ $? -eq 0 ]]; then
          EXT_DOWNLOAD=3
        else
          echo "Error: $pkg cannot be cloned!"
          exit 1
        fi
        case $pkg in
        VirtualSMC)
          cp -R "${EXT_PACKAGES}/${pkg}"/VirtualSmcPkg "${EXT_PACKAGES}"/
          rm -rf "${EXT_PACKAGES}/${pkg}"
        ;;
        esac
      done
    fi

    if [[ "$EXT_DOWNLOAD" -eq 3 ]]; then
      for drv in "${externalDrivers[@]}"
      do
        if [[ -f "$EXT_PACKAGES"/"${drv}"/"${drv}".dsc ]]; then
          packagesPathmunge "$EXT_PACKAGES" after
          cmd="build"
#          if (( $SkipAutoGen == 1 )) && (( $FORCEREBUILD == 0 )); then
#            cmd+=" --skip-autogen"
#          fi # SkipAutoGen is not adviced here
          cmd+=" -a $TARGETARCH -b $BUILDTARGET -t $TOOLCHAIN -n $BUILDTHREADS"
          cmd+=" -p ${EXT_PACKAGES}/${drv}/${drv}.dsc"
          echo
          echo "Running edk2 build for ${drv}${TARGETARCH} using the command:"
          echo "$cmd"
          eval "$cmd"
          echo
        else
          echo "Error: can't find ${drv}/${drv}.dsc,"
          echo "retry with --ext-co (require internet connection)"
          exit 1
        fi
      done
    fi
}

copyBin() {
  local cpSrc="$1"
  local cpDest="$2"
  local cpFile=$(basename "$2")
  #local cpArch=32
  local cpDestDIR=$(dirname "$cpDest")

  [[ ! -f  "$cpSrc" || ! -d  "$cpDestDIR" ]] && return
  [[ -d  "$cpDest" ]] && cpFile=$(basename "$cpSrc")
  #[[ "$cpFile" == *"-64"* ]] && cpArch=64

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
    if [[ -z "$EDK_TOOLS_PATH" ]]; then
      export BASETOOLS_DIR="$WORKSPACE"/BaseTools/Source/C/bin
    else
      export BASETOOLS_DIR="$EDK_TOOLS_PATH"/Source/C/bin
    fi
    export BOOTSECTOR_BIN_DIR="$CLOVERROOT"/CloverEFI/BootSector/bin
    export APTIO_BUILD_DIR_ARCH="${WORKSPACE}/Build/AptioFixPkg/${BUILDTARGET}_${TOOLCHAIN}/$TARGETARCH"
    export APFS_BUILD_DIR_ARCH="${WORKSPACE}/Build/AppleSupportPkg/${BUILDTARGET}_${TOOLCHAIN}/$TARGETARCH"
    export VIRTUALSMC_BUILD_DIR_ARCH="${WORKSPACE}/Build/VirtualSmcPkg/${BUILDTARGET}_${TOOLCHAIN}/$TARGETARCH"
	if (( $NOBOOTFILES == 0 )); then
    echo Compressing DUETEFIMainFv.FV ...
    "$BASETOOLS_DIR"/LzmaCompress -e -o "${BUILD_DIR}/FV/DUETEFIMAINFV${TARGETARCH}.z" "${BUILD_DIR}/FV/DUETEFIMAINFV${TARGETARCH}.Fv"

    echo Compressing DxeCore.efi ...
    "$BASETOOLS_DIR"/LzmaCompress -e -o "${BUILD_DIR}/FV/DxeMain${TARGETARCH}.z" "$BUILD_DIR_ARCH/DxeCore.efi"

    echo Compressing DxeIpl.efi ...
    "$BASETOOLS_DIR"/LzmaCompress -e -o "${BUILD_DIR}/FV/DxeIpl${TARGETARCH}.z" "$BUILD_DIR_ARCH/DxeIpl.efi"

    echo "Generate Loader Image ..."
	fi
    if [[ "${TARGETARCH}" = IA32 ]]; then
      cloverEFIFile=boot3
      if (( $NOBOOTFILES == 0 )); then
        "$BASETOOLS_DIR"/GenFw --rebase 0x10000 -o "$BUILD_DIR_ARCH/EfiLoader.efi" "$BUILD_DIR_ARCH/EfiLoader.efi"
        "$BASETOOLS_DIR"/EfiLdrImage -o "${BUILD_DIR}"/FV/Efildr32 \
        "${BUILD_DIR}"/${TARGETARCH}/EfiLoader.efi                \
        "${BUILD_DIR}"/FV/DxeIpl${TARGETARCH}.z                   \
        "${BUILD_DIR}"/FV/DxeMain${TARGETARCH}.z                  \
        "${BUILD_DIR}"/FV/DUETEFIMAINFV${TARGETARCH}.z
        cat $BOOTSECTOR_BIN_DIR/start32H.com2 $BOOTSECTOR_BIN_DIR/efi32.com3 \
        "${BUILD_DIR}"/FV/Efildr32 > "${BUILD_DIR}"/FV/boot
      fi
      rm -rf "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers3* 2> /dev/null
      rm -rf "$CLOVER_PKG_DIR"/drivers-Off/drivers3* 2> /dev/null

      mkdir -p "$CLOVER_PKG_DIR"/Bootloaders/ia32
      mkdir -p "$CLOVER_PKG_DIR"/EFI/BOOT
      mkdir -p "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers32
      mkdir -p "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers32UEFI
      mkdir -p "$CLOVER_PKG_DIR"/drivers-Off/drivers32
      mkdir -p "$CLOVER_PKG_DIR"/drivers-Off/drivers32UEFI

      # CloverEFI
      copyBin "${BUILD_DIR}"/FV/boot "$CLOVER_PKG_DIR"/Bootloaders/ia32/$cloverEFIFile
      # The following line is bad because the character '3' is at offset 0xc5 of start32H.com2, not offset 0xa9 - zenith432
      #setInitBootMsg "$CLOVER_PKG_DIR"/Bootloaders/ia32/$cloverEFIFile
      copyBin "$BUILD_DIR_ARCH"/CLOVER.efi "$CLOVER_PKG_DIR"/EFI/BOOT/BOOTIA32.efi
      copyBin "$BUILD_DIR_ARCH"/CLOVER.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/CLOVERIA32.efi

      # Mandatory drivers
      echo "Copy Mandatory drivers:"
      copyBin "$BUILD_DIR_ARCH"/FSInject.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers32/FSInject-32.efi

      binArray=( FSInject OsxFatBinaryDrv VBoxHfs )
      for efi in "${binArray[@]}"
      do
        copyBin "$BUILD_DIR_ARCH"/$efi.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers32UEFI/$efi-32.efi
      done

      # Optional drivers
      echo "Copy Optional drivers:"
      binArray=( VBoxIso9600 VBoxExt2 VBoxExt4 )
      for efi in "${binArray[@]}"
      do
        copyBin "$BUILD_DIR_ARCH"/$efi.efi "$CLOVER_PKG_DIR"/drivers-Off/drivers32/$efi-32.efi
      done

      if [[ $M_NOGRUB -eq 0 ]] && (( $NOBOOTFILES == 0 )); then
        binArray=( GrubEXFAT GrubISO9660 GrubNTFS GrubUDF )
        for efi in "${binArray[@]}"
        do
          copyBin "$BUILD_DIR_ARCH"/$efi.efi "$CLOVER_PKG_DIR"/drivers-Off/drivers32/$efi-32.efi
        done
      fi

      binArray=( Ps2KeyboardDxe Ps2MouseAbsolutePointerDxe Ps2MouseDxe UsbMouseDxe XhciDxe )
      for efi in "${binArray[@]}"
      do
        copyBin "$BUILD_DIR_ARCH"/$efi.efi "$CLOVER_PKG_DIR"/drivers-Off/drivers32/$efi-32.efi
      done

      # Applications
      echo "Copy Applications:"
      copyBin "$BUILD_DIR_ARCH"/bdmesg.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/tools/bdmesg-32.efi

      if [[ "${EDK2SHELL:-}" == "MinimumShell" ]]; then
        copyBin "${WORKSPACE}"/ShellBinPkg/MinUefiShell/Ia32/Shell.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/tools/Shell32.efi
      elif [[ "${EDK2SHELL:-}" == "FullShell" ]]; then
        copyBin "${WORKSPACE}"/ShellBinPkg/UefiShell/Ia32/Shell.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/tools/Shell32.efi
      fi
    fi

    if [[ "$TARGETARCH" = X64 ]]; then
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
      fi

      rm -rf "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers6* 2> /dev/null
      rm -rf "$CLOVER_PKG_DIR"/drivers-Off/drivers6* 2> /dev/null

      # Be sure that all needed directories exists
      mkdir -p "$CLOVER_PKG_DIR"/Bootloaders/x64
      mkdir -p "$CLOVER_PKG_DIR"/EFI/BOOT
      mkdir -p "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers64
      mkdir -p "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers64UEFI
      mkdir -p "$CLOVER_PKG_DIR"/drivers-Off/drivers64/FileVault2
      mkdir -p "$CLOVER_PKG_DIR"/drivers-Off/drivers64UEFI/FileVault2

      # Install CloverEFI file
      echo "Copy CloverEFI:"
      copyBin "${BUILD_DIR}"/FV/boot "$CLOVER_PKG_DIR"/Bootloaders/x64/$cloverEFIFile
      # For GENPAGE, the character "[TX]" is at offset 0x74 of Start64H[56].com, not offset 0xa9 - zenith432
      if [[ "$GENPAGE" -eq 0 ]]; then
        setInitBootMsg "$CLOVER_PKG_DIR"/Bootloaders/x64/$cloverEFIFile
      fi
      copyBin "$BUILD_DIR_ARCH"/CLOVER.efi "$CLOVER_PKG_DIR"/EFI/BOOT/BOOTX64.efi
      copyBin "$BUILD_DIR_ARCH"/CLOVER.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/CLOVERX64.efi

      # Mandatory drivers
      echo "Copy Mandatory drivers:"
# copyBin "$BUILD_DIR_ARCH"/FSInject.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers64/FSInject-64.efi
      binArray=( FSInject XhciDxe SMCHelper AudioDxe )
      for efi in "${binArray[@]}"
      do
        copyBin "$BUILD_DIR_ARCH"/$efi.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers64/$efi-64.efi
      done

      binArray=( AppleImageCodec AppleKeyAggregator AppleUITheme FirmwareVolume )
      for efi in "${binArray[@]}"
      do
        copyBin "$BUILD_DIR_ARCH"/$efi.efi "$CLOVER_PKG_DIR"/drivers-Off/drivers64/FileVault2/$efi-64.efi
      done

      binArray=( FSInject DataHubDxe SMCHelper AudioDxe )
      for efi in "${binArray[@]}"
      do
        copyBin "$BUILD_DIR_ARCH"/$efi.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers64UEFI/$efi-64.efi
      done

      binArray=( AppleImageCodec AppleUITheme AppleKeyAggregator FirmwareVolume )
      for efi in "${binArray[@]}"
      do
        copyBin "$BUILD_DIR_ARCH"/$efi.efi "$CLOVER_PKG_DIR"/drivers-Off/drivers64UEFI/FileVault2/$efi-64.efi
      done

      # Optional VirtualSMC
      copyBin "$VIRTUALSMC_BUILD_DIR_ARCH"/VirtualSMC.efi "$CLOVER_PKG_DIR"/drivers-Off/drivers64/VirtualSMC-64.efi
      copyBin "$VIRTUALSMC_BUILD_DIR_ARCH"/VirtualSMC.efi "$CLOVER_PKG_DIR"/drivers-Off/drivers64UEFI/VirtualSMC-64.efi

      if [[ $M_APPLEHFS -eq 0 ]]; then
        copyBin "$BUILD_DIR_ARCH"/VBoxHfs.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers64UEFI/VBoxHfs-64.efi
      else
        copyBin "${CLOVERROOT}"/FileSystems/HFSPlus/X64/HFSPlus.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/drivers64UEFI/HFSPlus.efi
      fi

      # Optional drivers
#echo "Copy Optional drivers:"
      # drivers64
      # Ps2KeyboardDxe Ps2MouseAbsolutePointerDxe
#binArray=( )
#for efi in "${binArray[@]}"
#do
#copyBin "$BUILD_DIR_ARCH"/$efi.efi "$CLOVER_PKG_DIR"/drivers-Off/drivers64/$efi-64.efi
#done

      if [[ $M_NOGRUB -eq 0 ]]; then
        binArray=( GrubEXFAT GrubISO9660 GrubNTFS GrubUDF )
        for efi in "${binArray[@]}"
        do
          copyBin "$BUILD_DIR_ARCH"/$efi.efi "$CLOVER_PKG_DIR"/drivers-Off/drivers64/$efi-64.efi
        done
      fi

      case "$EXT_DOWNLOAD" in
        1)
          downloadExtDriver "acidanthera/AptioFixPkg" AptioFix "AptioFix-" "-RELEASE"
          downloadExtDriver "acidanthera/AppleSupportPkg" AppleSupport "AppleSupport-v" "-RELEASE"
        ;;
        0 | 2 | 3)
          copyBin "$APTIO_BUILD_DIR_ARCH"/AptioMemoryFix.efi "$CLOVER_PKG_DIR"/drivers-Off/drivers64UEFI/AptioMemoryFix-64.efi
          copyBin "$APTIO_BUILD_DIR_ARCH"/AptioInputFix.efi "$CLOVER_PKG_DIR"/drivers-Off/drivers64UEFI/FileVault2/AptioInputFix-64.efi

          copyBin "$APFS_BUILD_DIR_ARCH"/AppleUISupport.efi "$CLOVER_PKG_DIR"/drivers-Off/drivers64UEFI/FileVault2/AppleUISupport-64.efi
          binArray=( ApfsDriverLoader AppleImageLoader )
          for efi in "${binArray[@]}"
          do
            copyBin "$APFS_BUILD_DIR_ARCH"/$efi.efi "$CLOVER_PKG_DIR"/drivers-Off/drivers64UEFI/$efi-64.efi
            copyBin "$APFS_BUILD_DIR_ARCH"/$efi.efi "$CLOVER_PKG_DIR"/drivers-Off/drivers64/$efi-64.efi
          done
        ;;
      esac
      # drivers64UEFI
      binArray=( CsmVideoDxe EnglishDxe EmuVariableUefi Fat NvmExpressDxe OsxAptioFix3Drv OsxAptioFixDrv OsxFatBinaryDrv OsxLowMemFixDrv PartitionDxe Ps2MouseDxe UsbKbDxe UsbMouseDxe VBoxExt2 VBoxExt4 VBoxIso9600)

      for efi in "${binArray[@]}"
      do
        copyBin "$BUILD_DIR_ARCH"/$efi.efi "$CLOVER_PKG_DIR"/drivers-Off/drivers64UEFI/$efi-64.efi
      done

      # drivers64UEFI/FileVault2
      binArray=( AppleKeyFeeder HashServiceFix )

      for efi in "${binArray[@]}"
      do
        copyBin "$BUILD_DIR_ARCH"/$efi.efi "$CLOVER_PKG_DIR"/drivers-Off/drivers64UEFI/FileVault2/$efi-64.efi
      done

      # Applications
      echo "Copy Applications:"
      copyBin "$BUILD_DIR_ARCH"/bdmesg.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/tools/

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
    echo "Done!"
}

# BUILD START #
#trap 'exitTrap' EXIT

# Default locale
export LC_ALL=POSIX


# Add toolchain bin directory to the PATH
if [[ "$SYSNAME" != Linux ]]; then
  pathmunge "$TOOLCHAIN_DIR/bin"
fi

MainBuildScript $@
export BUILD_DIR="${WORKSPACE}/Build/Clover/${BUILDTARGET}_${TOOLCHAIN}"
export BUILD_DIR_ARCH="${BUILD_DIR}/$TARGETARCH"

if [[ -z $MODULEFILE  ]] && (( $NOBOOTFILES == 0 )); then
    MainPostBuildScript
else
 copyBin "$BUILD_DIR_ARCH"/CLOVER.efi "$CLOVER_PKG_DIR"/EFI/CLOVER/CLOVERX64.efi
 copyBin "$BUILD_DIR_ARCH"/CLOVER.efi "$CLOVER_PKG_DIR"/EFI/BOOT/BOOTX64.efi
fi

# Local Variables:      #
# mode: ksh             #
# tab-width: 4          #
# indent-tabs-mode: nil #
# End:                  #
#
# vi: set expandtab ts=4 sw=4 sts=4: #
