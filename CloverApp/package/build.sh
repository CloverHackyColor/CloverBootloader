#!/bin/bash

cd "$(dirname $([ -L $0 ] && readlink $0 || echo $0))"

declare -r ROOT="$PWD"
declare -r CLOVERROOT=$(dirname $(dirname $ROOT))
declare -r SYM_PATH="${CLOVERROOT}"/CloverPackage/sym
declare -r BUILD_PATH="${CLOVERROOT}"/CloverApp/build
declare -r APP_PATH="${BUILD_PATH}"/Clover.app

xcodeVer() {
    /usr/bin/xcodebuild -version 2> /dev/null | head -n 1 | awk '{ print $2 }'
}

xcodeMajorVer() {
    xcodeVer | awk -F '.' '{ print $1 }'
}

xcodeCheck() {
local xmv=$(xcodeMajorVer)
if [[ $xmv -lt 11 ]]; then
  echo "Clover.app require Xcode 11 or newer, nothing done."
  exit 0
fi
echo "using Xcode $(xcodeVer)"
}

buildapp() {
echo "- Building Clover.app..."
cd "${CLOVERROOT}"/CloverApp
/usr/bin/xcodebuild -project 'Clover.xcodeproj' \
                    CONFIGURATION_BUILD_DIR=${BUILD_PATH} \
                    EPLOYMENT_LOCATION=NO \
                    -scheme 'Clover' >/dev/null
}

buildpkg() {
echo "- Building Clover.app package installer.."
cd "${ROOT}"
local INFO="${APP_PATH}"/Contents/Info.plist
local APPVERSION=$(defaults read "${INFO}" CFBundleShortVersionString)
local PKGNAME="Clover.app_v${APPVERSION}.pkg"

if [[ -d "${CLOVERROOT}"/.git ]];then
  PKGNAME="Clover.app_v${APPVERSION}_r$( git -C "${CLOVERROOT}" describe --tags --abbrev=0 ).pkg"
fi

rm -f "${SYM_PATH}"/Clover.app*.pkg
rm -f "${SYM_PATH}"/Clover.app*.zip
rm -rf "${BUILD_PATH}"/package
mkdir -p "${BUILD_PATH}"/package/temp/Applications
cp -R "${APP_PATH}" "${BUILD_PATH}"/package/temp/Applications/

pkgbuild --root "${BUILD_PATH}"/package/temp \
  --version "${APPVERSION}" \
  --scripts "${ROOT}"/Scripts \
  --component-plist package.plist \
  --install-location / \
  "${BUILD_PATH}"/package/CloverApp.pkg

awk -v n=5 -v s="  <title>Clover.app v${APPVERSION}</title>" 'NR == n {print s} {print}' distribution.xml | \
    sed -e "s/VERSION/${APPVERSION}/g" > "${BUILD_PATH}"/package/Distribution

cd "${BUILD_PATH}"/package
productbuild --distribution ./Distribution \
              --resources "${ROOT}"/Resources \
              --package-path ./CloverApp.pkg \
              "${SYM_PATH}/${PKGNAME}"

(cd "${SYM_PATH}" ; zip ${PKGNAME}.zip ${PKGNAME} )
 
rm -rf "${BUILD_PATH}"/package
rm -f "${SYM_PATH}"/Clover.app*.pkg
open "${SYM_PATH}"
}

xcodeCheck
buildapp
buildpkg
