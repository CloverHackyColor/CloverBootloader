#!/bin/bash

cd "$(dirname "$0")" || exit 1
export DUETTOOL=Clover_Duet.command
cp -Rp $DUETTOOL ./CloverPackage/CloverV2
./CloverPackage/CloverV2/$DUETTOOL
Sleep 1
rm -rf ./CloverPackage/CloverV2/$DUETTOOL
