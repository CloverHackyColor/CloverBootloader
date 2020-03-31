#include <Platform.h>
#include "../cpp_foundation/XStringWArray.h"
#include "../cpp_foundation/XStringW.h"

int XStringWArray_tests()
{

#ifdef JIEF_DEBUG
//	DebugLog(2, "XStringWArray_tests -> Enter\n");
#endif

	XStringWArray array1;
	
	if ( !array1.IsNull() ) return 1;
	
	array1.Add(L"1"_XSW);
	if ( array1.IsNull() ) return 2;
	array1.Add(L"2"_XSW);

	if ( array1[0] != L"1" ) return 3;
	if ( array1[1] != L"2" ) return 4;
	
	if ( !array1.Contains(L"2"_XSW) ) return 5;

	// Test == and !=
	{
		XStringWArray array1bis;
		array1bis.Add(L"1"_XSW);
		array1bis.Add(L"2"_XSW);
		
		if ( !(array1 == array1bis) ) return 10;
		if ( array1 != array1bis ) return 11;
	}
	
	// Test concat and Split
	{
		XStringW c = array1.ConcatAll(L", "_XSW, L"^"_XSW, L"$"_XSW);
		if ( c != L"^1, 2$" ) return 1;

		// Split doesn't handle prefix and suffix yet.
		c = array1.ConcatAll(L", "_XSW);

		XStringWArray array1bis = Split(c);
		if ( array1 != array1bis ) return 20;
	}

	XStringWArray array2;
	array2.Add(L"2"_XSW);
	array2.Add(L"1"_XSW);

	if ( array2[0] != L"2" ) return 30;
	if ( array2[1] != L"1" ) return 31;


	if ( array1 == array2 ) return 40; // Array != because order is different
	if ( !array1.Same(array2) ) return 41; // Arrays are the same
	
	array1.AddNoNull(L"3"_XSW);
	if ( array1.size() != 3 ) return 50;
	array1.AddNoNull(L""_XSW);
	if ( array1.size() != 3 ) return 51;
	array1.AddEvenNull(XStringW());
	if ( array1.size() != 4 ) return 52;
	array1.AddID(L"2"_XSW);
	if ( array1.size() != 4 ) return 53;


  return 0;
}
