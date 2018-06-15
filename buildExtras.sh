#!/bin/bash

# Script for building NASM
#
# Original Created by JrCs on 8/30/14.
# Enhanced by STLVNB on 13/04/2016. 
#


# Change PREFIX if you want nasm installed on different place
#
TOOLCHAIN_DIR=${TOOLCHAIN_DIR:-~/src/opt/local}
export PREFIX=${PREFIX:-$TOOLCHAIN_DIR}

if [[ "$(uname)" == Darwin ]] && [[ ! -x "$TOOLCHAIN_DIR"/cross/bin/x86_64-clover-linux-gnu-gcc ]]; then
    echo "No clover toolchain found !" >&2
    if [ -z $CLEAN_BUILD ]; then
   	  echo "Press b to BUILD it OR else define the TOOLCHAIN_DIR variable." >&2
   	  read -n 1 result
   	  [ "$result" != "b" ] && exit 1
  	else
      echo -n b
  	fi
   	echo "uilding it"
 	./build_gcc8.sh
   	echo "Continuing..."
fi

#
# Nasm source version
# here we can change source versions of tools
# Always use current Version when building, will auto compile newest STABLE version.
#
nasmcheck=$(curl -Is https://downuptime.net/nasm.us.html | grep HTTP | cut -d ' ' -f2)
if [ "$nasmcheck" == 200 ]; then
	nasmVersInfo=$(curl -s -f https://www.nasm.us | grep "/releasebuilds/")
	if [ "${nasmVersInfo:158:1}" == "." ]; then
		verLen=7
	else
		verLen=4
	fi
	export NASM_VERSION="${nasmVersInfo:154:$verLen}"
else
	export NASM_VERSION=2.13.03
fi

#
# iasl source version
# here we can change source versions of tools
# Always use current Version when building, will auto compile newest STABLE version.
#
acpicacheck=$(curl -Is https://downuptime.net/acpica.us.html | grep HTTP | cut -d ' ' -f2)
if [ $acpicacheck == 200 ]; then
	acpicaVersInfo=$(curl -s -f https://acpica.org/downloads/ | grep 'The current release of ACPICA is version <strong>')
	export acpicaVers="${acpicaVersInfo:191:8}" 
else
	export acpicaVers=20160108
fi



# ./configure arguments for Nasm
#
export NASM_CONFIG="--prefix=$PREFIX"

# You can change DIR_MAIN if u wan't nasm source downloaded
# in different folder.
#
export RAMDISK_MNT_PT=/tmp/buildnasm-ramdisk
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

# Function: checking installation of Xcode Tools
fnCheckXcode () {
    [ ! -f /usr/bin/xcodebuild ] && \
        echo "ERROR: Install Xcode Tools from Apple before using this script." && \
        exit
}
#
# colorize output
# 
function echoc(){
    local exp=$1;
    local color=$2;
    local newline="$3";
    if ! [[ $color =~ '^[0-9]$' ]] ; then
       case $(echo $color | tr '[:upper:]' '[:lower:]') in
        black) color=0 ;;
        red) color=1 ;;
        green) color=2 ;;
        yellow) color=3 ;;
        blue) color=4 ;;
        magenta) color=5 ;;
        cyan) color=6 ;;
        white|*) color=7 ;; # white or invalid color
       esac
    fi
    tput setaf $color;
    tput bold;
    echo $newline "$exp";
    tput sgr0;
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



# Add XCode bin directory for the command line tools to the PATH
pathmunge "$(xcode-select --print-path)"/usr/bin

# Add toolchain bin directory to the PATH
pathmunge "$TOOLCHAIN_DIR"/bin

cd ${DIR_DOWNLOADS}
iaslLocalVers=
if [[ -f ${PREFIX}/bin/iasl ]]; then
	iaslLocalInfo=$(${PREFIX}/bin/iasl -v)
	iaslLocalVers=${iaslLocalInfo:68:8}
fi
nasmLocalVers=
if [[ -f ${PREFIX}/bin/nasm ]]; then
	nasmLocalVers=$(${PREFIX}/bin/nasm -v)
	nasmLocalVers=${nasmLocalVers:13:$verLen}
fi

if [ "$iaslLocalVers" != "$acpicaVers" ]; then
	echoc "Detected updated SVN iasl " red -n; echoc ":-$acpicaVers-:" green
	iaslUpdate=Yes
else
	echoc "Detected local iasl " red -n; echoc ":-$iaslLocalVers-:" green 
	iaslUpdate=No
fi
export TARBALL_ACPICA=acpica-unix-$acpicaVers

if [ "$nasmLocalVers" != "$NASM_VERSION" ]; then
	echoc "Detected updated SVN nasm " red -n; echoc ":-$NASM_VERSION-:" green
	nasmUpdate=Yes
else
	echoc "Detected local nasm " red -n; echoc ":-$nasmLocalVers-:" green
	nasmUpdate=No
fi
if [[ ! -f ${DIR_DOWNLOADS}/${TARBALL_ACPICA}.tar.gz ]]; then
  echoc "Downloading https://acpica.org/sites/acpica/files/${TARBALL_ACPICA}.tar.gz" green
  echo
  curl -f -o download.tmp --remote-name https://acpica.org/sites/acpica/files/${TARBALL_ACPICA}.tar.gz || exit 1
  mv download.tmp ${TARBALL_ACPICA}.tar.gz
  echo
fi
if [[ "$iaslUpdate" == "Yes" ]]; then
  if [ -f ${PREFIX}/bin/iasl ]; then
  	rm -rf ${PREFIX}/bin/iasl
  fi
fi 
if [ ! -f ${PREFIX}/bin/iasl ]; then 
  echoc "Building ACPICA $acpicaVers" green
  tar -zxf ${TARBALL_ACPICA}.tar.gz
  cd ${TARBALL_ACPICA}
  make iasl CC=gcc 1> /dev/null 2> $DIR_LOGS/${TARBALL_ACPICA}.make.log.txt
  make install 1> $DIR_LOGS/${TARBALL_ACPICA}.install.log.txt 2> /dev/null
  rm -Rf ${DIR_DOWNLOADS}/${TARBALL_ACPICA}
  echo
fi
cd ${DIR_DOWNLOADS}
tarball="nasm-${NASM_VERSION}.tar.xz"
if [[ ! -f "$tarball" ]]; then
	echoc "Status: $tarball not found." red
    curl -f -o download.tmp --remote-name https://www.nasm.us/pub/nasm/releasebuilds/${NASM_VERSION}/$tarball || exit 1
    mv download.tmp $tarball
fi


if [[ "$nasmUpdate" == "Yes" ]]; then
  echoc "Building nasm V${NASM_VERSION}" green
  tar -zxf $tarball
  cd nasm-${NASM_VERSION}
  ./configure --prefix=${PREFIX} 1> /dev/null 2> $DIR_LOGS/$tarball.config.log.txt
  make CC=clang 1> /dev/null 2> $DIR_LOGS/$tarball.make.log.txt
  cp nasm ${PREFIX}/bin/
  rm -Rf ${DIR_DOWNLOADS}/nasm-${NASM_VERSION}
  echo
fi
