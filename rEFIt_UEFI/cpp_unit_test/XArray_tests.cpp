#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "../cpp_foundation/XArray.h"


int XArray_tests()
{
	XArray<UINTN> array1;
	array1.Add(12);
	array1.Add(34);
	array1.Add(56);

	if ( array1[0] != 12 ) return 1;
	if ( array1[1] != 34 ) return 2;
	if ( array1[2] != 56 ) return 3;

	array1.RemoveAtIndex(1);

	if ( array1[1] != 56 ) return 4;

	return 0;
}
