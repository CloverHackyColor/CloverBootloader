#!/bin/bash

##
# adjust settings here
##
application="Clover Preference Panel" # this is the name of the application
string_file="Localizable.strings"     # the name of your Localizable string file

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
	tmp_dir="/tmp/$application"
    mkdir -p "$tmp_dir"
    # Extract source locale strings (use this to check if you added new strings to your xibs)
    echo -n "Updating strings file for ${application}... "
    genstrings -s GetLocalizedString -o "$tmp_dir" $SOURCE_DIR/*.m
    iconv -f utf-16 -t utf-8 "$tmp_dir/$string_file" | grep -vE '^\/\*.*\*\/$' | \
     grep -vE '^$' >"$tmp_dir/$string_file.new"
    cmp --silent "$string_file" "$tmp_dir/$string_file.new"
    if [[ $? -ne 0 ]]; then
        mv -f "$tmp_dir/$string_file.new" "$string_file"
    fi
    rm -rf "$tmp_dir"
    echo "done"
else
    echo "XCode version too old. Don't extracting locale strings from source files"
fi

[[ "$EXTRACT_ONLY" -eq 1 ]] && exit 0

# Generate localized strings
for locale in "$PO_DIR"/*.po ; do
     locale="${locale%.po}"
     locale="${locale#$PO_DIR/}"
     echo -n "Check "$locale" strings... "
     [[ ! -d "$SOURCE_DIR/$locale.lproj" ]] && mkdir -p "$SOURCE_DIR/$locale.lproj"
     if [ ! -f "$SOURCE_DIR/$locale.lproj/$string_file" ] ; then
         echo -n "strings file $SOURCE_DIR/$locale.lproj/$string_file not found in locale dir '$locale.lproj' - using template file... "
         cp -f "$string_file" "$SOURCE_DIR/$locale.lproj/$string_file"
     fi
     echo "done"
done

exit 0
##
# end of script
##
