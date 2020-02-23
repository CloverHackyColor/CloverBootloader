
#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile

CONST CHAR16 *
EFIAPI
ConstStrStr (
  IN      CONST CHAR16              *String,
  IN      CONST CHAR16              *SearchString
  );
