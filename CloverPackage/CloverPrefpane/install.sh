#!/bin/bash

# Prevent the script from doing bad things
set -u  # Abort with unset variables

# Go to the script directory
cd "$(dirname $0)"

declare -r CLOVER_PREFPANE_DIR="$PWD"

# Build application if necessary
make CloverPrefpane || exit $?

# Install files
open "$CLOVER_PREFPANE_DIR"/build/Clover.prefpane

echo "CloverPrefpane install successfully !"

# Local Variables:      #
# mode: ksh             #
# tab-width: 4          #
# indent-tabs-mode: nil #
# End:                  #
#
# vi: set expandtab ts=4 sw=4 sts=4: #
