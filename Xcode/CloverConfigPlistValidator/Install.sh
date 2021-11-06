#!/bin/bash
#
#  Install.sh
#  alternate build for buldpkg
#  Created on: May 12, 2021
#  Author: LAbyOne
#
cd "$(dirname $([ -L $0 ] && readlink $0 || echo $0))"
REVISION=$(git describe --tags $(git rev-list --tags --max-count=1﻿))

declare -r ROOT="$PWD"
declare -r CLOVERROOT=$(dirname $(dirname $ROOT))
declare -r BUILD_PATH="${CLOVERROOT}"/Xcode/CloverConfigPlistValidator/build
declare -r CCPV_PATH="${BUILD_PATH}"/ccpv
declare -r INSTALL_DIR="$CLOVERROOT/CloverPackage/CloverConfigPlistValidator"
declare -r SYMROOT="${CLOVERROOT}/CloverPackage/sym"

# Clear old sym directory and Create a new one
if [[ -d "$SYMROOT" && $(stat -f '%u' "$SYMROOT") -eq 0 ]]; then
    sudo rm -rf "$SYMROOT"
fi
rm -rf "$SYMROOT"
mkdir "$SYMROOT"

# make CloverConfigPlistValidator build
buildccpv() {
echo "- Building ccpv and archive"
cd "${CLOVERROOT}"/Xcode/CloverConfigPlistValidator
/usr/bin/xcodebuild -project 'CloverConfigPlistValidator.xcodeproj' \
                    -configuration 'Release' \
					CONFIGURATION_BUILD_DIR=${BUILD_PATH} \
                    EPLOYMENT_LOCATION=NO \
                    ARCHS=x86_64 VALID_ARCHS=x86_64 ONLY_ACTIVE_ARCH=YES >/dev/null

# Clear old ccpv directory and Create a new one
rm -Rf "$INSTALL_DIR"
mkdir -p "$INSTALL_DIR"

# Install files
cp "$BUILD_PATH"/CloverConfigPlistValidator "$INSTALL_DIR"/ccpv

# make archive
mv "$BUILD_PATH"/CloverConfigPlistValidator "$BUILD_PATH"/CloverConfigPlistValidator_${REVISION}
cd "$BUILD_PATH"
zip -qr CloverConfigPlistValidator_${REVISION}.zip CloverConfigPlistValidator_${REVISION}
cp "$BUILD_PATH"/CloverConfigPlistValidator_${REVISION}.zip "${SYMROOT}"
# clean up
rm -Rf "$BUILD_PATH"
}

buildccpv
