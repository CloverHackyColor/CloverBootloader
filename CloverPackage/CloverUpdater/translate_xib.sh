#!/bin/bash

##
# adjust settings here
##
application="CloverUpdater"       # this is the name of the application
src_locale="en"                   # this is the language you design in
xib_file="MainMenu.xib"           # the name of your XIB interface builder file
strings_file="CloverUpdater.strings"  # the name of your strings file

set -u

##
# no customizations below this line
##
cd "$(dirname $0)"

declare -r SOURCE_DIR="src"
declare -r PO_DIR="../package/po"
declare -r XCODE_MAJOR_VERSION=$(xcodebuild -version | grep ^Xcode | awk '{print $2}' | sed -e 's/\..*//')

# ========== OPTIONS ===========
EXTRACT_ONLY=0

while [[ $# -gt 0 ]]; do
    option=$1
    shift
    case "$option" in
        --extract-only) EXTRACT_ONLY=1 ;;
        -*)
            printf "Unrecognized option \`%s'\n" "$option" 1>&2
            exit 1
            ;;
    esac
done

# Only extract source locale strings if XCode version > 4
if [[ "$XCODE_MAJOR_VERSION" -ge 4 ]]; then
    # Extract source locale strings (use this to check if you added new strings to your xibs)
    echo -n "Updating '$src_locale' strings file for ${application}... "
    ibtool --generate-strings-file $strings_file.utf16 "$SOURCE_DIR/$src_locale.lproj/$xib_file"
    if [[ $? -eq 0 ]]; then
        iconv -f utf-16 -t utf-8 $strings_file.utf16 | grep -vE '^\/\*.*\*\/$' | \
            grep -vE '^$' >$strings_file.new
        rm -f $strings_file.utf16
        cmp --silent $strings_file $strings_file.new
        if [[ $? -eq 0 ]]; then
        # No change
            rm -f $strings_file.new
        else
            mv -f $strings_file.new $strings_file
        fi
        echo "done"
    else
        echo "Generation failed. Not extracting locale strings from source XIB file"
    fi
else
    echo "XCode version too old. Not extracting locale strings from source XIB file"
fi

[[ "$EXTRACT_ONLY" -eq 1 ]] && exit 0

# Generate localized interfaces
#for locale in $target_locales ; do
for locale in "$PO_DIR"/*.po ; do
    locale="${locale%.po}"
    locale="${locale#$PO_DIR/}"
    [[ "$locale" == $src_locale ]] && continue
    echo -n "Generating "$locale" interface... "
    [[ ! -d "$SOURCE_DIR/$locale.lproj" ]] && mkdir -p "$SOURCE_DIR/$locale.lproj"
        if [ -f "$SOURCE_DIR/$locale.lproj/$strings_file" ] ; then
            ibtool --strings-file "$SOURCE_DIR/$locale.lproj/$strings_file" \
             --write "$SOURCE_DIR/$locale.lproj/$xib_file" "$SOURCE_DIR/$src_locale.lproj/$xib_file"
    else
        echo -n "strings file $SOURCE_DIR/$locale.lproj/$strings_file not found in locale dir '$locale.lproj' - using '$src_locale' locale... "
        cp -f "$SOURCE_DIR/$src_locale.lproj/$xib_file" "$SOURCE_DIR/$locale.lproj/$xib_file"
    fi
    echo "done"
done
##
# end of script
##
