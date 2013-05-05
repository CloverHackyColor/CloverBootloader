#!/bin/bash

##
# adjust settings here
##
src_locale="en"                   # this is the language you design in
xib_file="CloverPrefpane.xib"     # the name of your XIB interface builder file
strings_file="CloverPrefpane.strings"  # the name of your strings file

set -u

##
# no customizations below this line
##
cd "$(dirname $0)"

declare -r SOURCE_DIR="src"

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

# Extract source locale strings (use this to check if you added new strings to your xibs)
echo -n "Updating "$src_locale" strings file... "
ibtool --generate-strings-file $strings_file.utf16 "$SOURCE_DIR/$src_locale.lproj/$xib_file"
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

[[ "$EXTRACT_ONLY" -eq 1 ]] && exit 0

# Generate localized interfaces
#for locale in $target_locales ; do
for locale_dir in "$SOURCE_DIR/"*.lproj ; do
    locale="${locale_dir%.lproj}"
    locale="${locale#$SOURCE_DIR/}"
    [[ "$locale" == $src_locale ]] && continue
    echo -n "Generating "$locale" interface... "
    if [ -d "$SOURCE_DIR/$locale.lproj" ] ; then
        if [ -f "$SOURCE_DIR/$locale.lproj/$strings_file" ] ; then
            ibtool --strings-file "$SOURCE_DIR/$locale.lproj/$strings_file" \
			 --write "$SOURCE_DIR/$locale.lproj/$xib_file" "$SOURCE_DIR/$src_locale.lproj/$xib_file"
            echo "done"
        else
            echo "strings file $SOURCE_DIR/$locale.lproj/$strings_file not found in locale dir $locale.lproj - skipping $locale locale"
        fi
    else
        echo "locale dir "$locale".lproj not found - skipping "$locale" locale"
    fi
done
##
# end of script
##
