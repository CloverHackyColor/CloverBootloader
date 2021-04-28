#!/bin/bash
SCRIPT_ABS_FILENAME=`LC_ALL=en_US.ISO8859-1 perl -e 'use Cwd "abs_path";print abs_path(shift)' "${BASH_SOURCE[0]}"`
SCRIPT_DIR=`dirname "$SCRIPT_ABS_FILENAME"`

cd "$SCRIPT_DIR"

echo "  ../Include/Library/printf_lite-conf.h"
echo "  ../Include/Library/printf_lite.h"

find . -not -path "./PlatformPOSIX/*" -and -not -path "./PlatformIA32/*" -and \( -name "*.h" -or -name "*.cpp" -or -name "*.c" \) -print0 | sort -fz | while read -d $'\0' file
do
  echo -e "  "$(echo "$file" | sed 's|./||')
done
