#!/bin/bash

# Prevent the script from doing bad things
set -u  # Abort with unset variables

packagename="Clover"

# Go to the script directory to create the package
cd "$(dirname $0)"

declare -r PKGROOT="$PWD"
declare -r SRCROOT="$PWD"/../../..
declare -r SYMROOT=../sym
declare -r PKG_RESOURCES_DIR="${SYMROOT}"/Resources

# ====== LANGUAGE SETUP ======
export LANG='en_US.UTF-8'
export LC_COLLATE='C'
export LC_CTYPE='C'

# ====== REVISION/VERSION ======
declare -r CLOVER_REVISION=$(git describe --tags --abbrev=0)

# ==== CHECK ENVIRONEMENT ====

GETTEXT_PREFIX=${GETTEXT_PREFIX:-"${SRCROOT}"/opt/local}

echo "${GETTEXT_PREFIX}"

# Check that the gettext utilities exists
if [[ ! -x "$GETTEXT_PREFIX/bin/msgmerge" ]]; then
    msgmerge_bin="$(type -P msgmerge)"
    if [[ -x "$msgmerge_bin" ]]; then
        export GETTEXT_PREFIX="${msgmerge_bin%/bin/msgmerge}"
    else
        build_gettext_script="$(cd "$PKGROOT"/../..; PWD -P)/buildgettext.sh"
        echo "GNU gettext utilities is mandatory to build Clover package." >&2
        echo "Use the $build_gettext_script script to build them." >&2
        exit 1
    fi
fi

export PATH="${GETTEXT_PREFIX}/bin:${PATH}"

# ========== OPTIONS ===========
UPDATE_PO=0

while [[ $# -gt 0 ]]; do
    option=$1
    shift
    case "$option" in
        --update-po) UPDATE_PO=1 ;;
        -*)
            printf "Unrecognized option \`%s'\n" "$option" 1>&2
            exit 1
            ;;
    esac
done

TEMPLATES_DIR="Resources/templates"
CLOVER_UPDATER_DIR="../CloverUpdater"
CLOVER_PREFPANE_DIR="../CloverPrefpane"
PODIR="po"

# Update CloverUpdater.strings
"$CLOVER_UPDATER_DIR"/translate_xib.sh --extract-only

# Update CloverPrefpane.strings
"$CLOVER_PREFPANE_DIR"/translate_xib.sh --extract-only
# Update Localizable.strings
"$CLOVER_PREFPANE_DIR"/translate_source.sh --extract-only

# Check if pot and po files need to be updated
IFS=$'\n' # '

last_resources_update=0
for file in "$TEMPLATES_DIR"/*.html "$TEMPLATES_DIR"/Localizable.strings \
 "$CLOVER_UPDATER_DIR"/CloverUpdater.strings \
 "$CLOVER_PREFPANE_DIR"/CloverPrefpane.strings "$CLOVER_PREFPANE_DIR"/Localizable.strings; do
    timestamp=$(stat -f %m "$file")
    [[ $timestamp -gt $last_resources_update ]] && last_resources_update=$timestamp
done
last_pot_update=$(stat -f %m "$PODIR"/clover.pot)
[[ $last_pot_update -lt $last_resources_update ]] && UPDATE_PO=1

if [[ "$UPDATE_PO" -ne 1 ]]; then
    # Copying po and pot files outside the repository
    po_tmpdir=$(mktemp -d -t po)
    ditto --noextattr --noqtn "${PKGROOT}"/po/  "$po_tmpdir"/
    # Automatically remove temporary directory at exit
    trap 'echo; rm -rf "$po_tmpdir"' EXIT
    PODIR="$po_tmpdir"
fi

PERLLIB=bin/po4a/lib                                                   \
 bin/po4a/po4a                                                         \
 --package-name 'Clover'                                               \
 --package-version "r${CLOVER_REVISION}"             \
 --msgmerge-opt '--lang=$lang --previous --width=79'                   \
 --variable PODIR="$PODIR"                                             \
 --variable TEMPLATES_DIR="$TEMPLATES_DIR"                             \
 --variable CLOVER_UPDATER_DIR="$CLOVER_UPDATER_DIR"                   \
 --variable CLOVER_PREFPANE_DIR="$CLOVER_PREFPANE_DIR"                 \
 --variable OUTPUT_DIR="${PKG_RESOURCES_DIR}/${packagename}/Resources" \
 po4a-clover.cfg


# Local Variables:      #
# mode: ksh             #
# tab-width: 4          #
# indent-tabs-mode: nil #
# End:                  #
#
# vi: set expandtab ts=4 sw=4 sts=4: #
