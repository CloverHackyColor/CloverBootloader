#!/bin/bash

# Prevent the script from doing bad things
set -u  # Abort with unset variables

# Go to the script directory
cd "$(dirname $0)"

declare -r CLOVER_UPDATER_DIR="$PWD"
declare -r CLOVER_APP_SUPPORT="$HOME"/Library/Application Support/Clover

# Install files
sudo mkdir -p "$CLOVER_APP_SUPPORT"
sudo cp -p "$CLOVER_UPDATER_DIR"/CloverUpdaterUtility "$CLOVER_APP_SUPPORT"/
sudo chmod +rx "$CLOVER_UPDATER_DIR"/CloverUpdaterUtility

# Build application if necessary
make CloverUpdater || exit $?
sudo cp -pr "$CLOVER_UPDATER_DIR"/build/CloverUpdater.app "$CLOVER_APP_SUPPORT"/

rm -f "$HOME"/Library/LaunchAgents/com.projectosx.Clover.Updater.plist
cp "$CLOVER_UPDATER_DIR"/CloverUpdaterUtility.plist "$HOME"/Library/LaunchAgents/com.projectosx.Clover.Updater.plist

echo "CloverUpdater install successfully !"

# Local Variables:      #
# mode: ksh             #
# tab-width: 4          #
# indent-tabs-mode: nil #
# End:                  #
#
# vi: set expandtab ts=4 sw=4 sts=4: #
