#!/bin/bash
#
# Copyright (c) 2008 - 2009, Apple Inc. All rights reserved.<BR>
# Copyright (c) 2010, Intel Corporation. All rights reserved.<BR>
#
# This program and the accompanying materials
# are licensed and made available under the terms and conditions of the BSD License
# which accompanies this distribution.  The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php
#
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#

set -e
shopt -s nocasematch


#
# Setup workspace if it is not set
#
if [ -z "$WORKSPACE" ]
then
  echo Initializing workspace
  if [ ! -e `pwd`/edksetup.sh ]
  then
    cd ..
  fi
# This version is for the tools in the BaseTools project.
# this assumes svn pulls have the same root dir
#  export EDK_TOOLS_PATH=`pwd`/../BaseTools
# This version is for the tools source in edk2
  export EDK_TOOLS_PATH=`pwd`/BaseTools
  echo $EDK_TOOLS_PATH
  source edksetup.sh BaseTools
else
  echo Building from: $WORKSPACE
fi

#
# Pick a default tool type for a given OS
#
TARGET_TOOLS=MYTOOLS
NETWORK_SUPPORT=
case `uname` in
  CYGWIN*) echo Cygwin not fully supported yet. ;;
  Darwin*)
      Major=$(uname -r | cut -f 1 -d '.')
      if [[ $Major == 9 ]]
      then
        echo UnixPkg requires Snow Leopard or later OS
        exit 1
      else
        TARGET_TOOLS=XCODE32
      fi
#      NETWORK_SUPPORT="-D NETWORK_SUPPORT"
      ;;
  Linux*) TARGET_TOOLS=ELFGCC ;;

esac

BUILD_ROOT_ARCH=$WORKSPACE/Build/Shell/DEBUG_"$TARGET_TOOLS"/IA32


#
# Build the edk2 UnixPkg
#
echo $PATH
echo `which build`
#build -p $WORKSPACE/UnixPkg/UnixPkg.dsc         -a IA32 -t $TARGET_TOOLS $NETWORK_SUPPORT -n 3 $1 $2 $3 $4 $5 $6 $7 $8
build -p $WORKSPACE/CloverEFI/UefiCpuPkg/UefiCpuPkg.dsc -a IA32 -t $TARGET_TOOLS -n 3 $*
exit $?

