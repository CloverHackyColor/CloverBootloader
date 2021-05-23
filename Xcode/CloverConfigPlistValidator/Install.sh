#!/bin/bash
#
#  Install.sh
#  alternate build for buldpkg
#  Created on: May 12, 2021
#  Author: LAbyOne
#
cd "$(dirname $([ -L $0 ] && readlink $0 || echo $0))"

declare -r ROOT="$PWD"
declare -r CLOVERROOT=$(dirname $(dirname $ROOT))
declare -r BUILD_PATH="${CLOVERROOT}"/Xcode/CloverConfigPlistValidator/build
declare -r CCPV_PATH="${BUILD_PATH}"/ccpv
declare -r INSTALL_DIR="$CLOVERROOT/CloverPackage/CloverConfigPlistValidator"

buildccpv() {
echo "- Building ccpv..."
cd "${CLOVERROOT}"/Xcode/CloverConfigPlistValidator
/usr/bin/xcodebuild -project 'CloverConfigPlistValidator.xcodeproj' \
                    -configuration 'Release' \
					CONFIGURATION_BUILD_DIR=${BUILD_PATH} \
                    EPLOYMENT_LOCATION=NO \
                    ARCHS=x86_64 VALID_ARCHS=x86_64 ONLY_ACTIVE_ARCH=YES >/dev/null

# remove old and recreate directory
rm -Rf "$INSTALL_DIR"
mkdir -p "$INSTALL_DIR"

# Install files
mv "$BUILD_PATH"/CloverConfigPlistValidator "$INSTALL_DIR"/ccpv

# clean up
rm -Rf "$BUILD_PATH"
}


buildccpv