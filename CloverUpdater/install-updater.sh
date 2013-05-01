#!/bin/bash

# Prevent the script from doing bad things
set -u  # Abort with unset variables

# Go to the script directory
cd "$(dirname $0)"

declare -r CLOVER_UPDATER_DIR="$PWD"
declare -r CLOVER_APP_SUPPORT="/Library/Application Support/Clover"

sudo rm -f /Library/LaunchAgents/com.projectosx.Clover.Updater.plist

# Build application if necessary
make CloverUpdater || exit $?

# Install files
sudo cp "$CLOVER_UPDATER_DIR"/CloverUpdaterUtility.plist /Library/LaunchAgents/com.projectosx.Clover.Updater.plist
sudo mkdir -p "$CLOVER_APP_SUPPORT"
sudo cp -p "$CLOVER_UPDATER_DIR"/CloverUpdaterUtility "$CLOVER_APP_SUPPORT"/
sudo chmod +rx "$CLOVER_UPDATER_DIR"/CloverUpdaterUtility
sudo cp -pr "$CLOVER_UPDATER_DIR"/build/CloverUpdater.app "$CLOVER_APP_SUPPORT"/

echo "CloverUpdater install successfully !"

# Local Variables:      #
# mode: ksh             #
# tab-width: 4          #
# indent-tabs-mode: nil #
# End:                  #
#
# vi: set expandtab ts=4 sw=4 sts=4: #
