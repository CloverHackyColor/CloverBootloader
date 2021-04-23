
echo "  ../Include/Library/printf_lite-conf.h"
echo "  ../Include/Library/printf_lite.h"

find . -not -path "./PlatformPOSIX/*" -and -not -path "./PlatformIA32/*" -and \( -name "*.h" -or -name "*.cpp" -or -name "*.c" \) -print0 | sort -fz | while read -d $'\0' file
do
  echo -e "  "$(echo "$file" | sed 's|./||')
done
