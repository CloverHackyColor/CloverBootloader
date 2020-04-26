#include <Platform.h>
#include "../cpp_foundation/XStringArray.h"

int XStringArray_tests()
{

#ifdef JIEF_DEBUG
//	printf("XStringWArray_tests -> Enter\n");
#endif

	XStringWArray array1;

	if ( !array1.isEmpty() ) return 1;

	array1.Add(L"1"_XSW);
	if ( array1.isEmpty() ) return 2;
	if ( array1[0] != "1"_XS ) return 21;
	array1.Add(L"2"_XSW);
	if ( array1[1] != "2"_XS ) return 21;

	if ( !array1.contains(L"2"_XSW) ) return 5;

	// Test == and !=
	{
		XStringWArray array1bis;
		array1bis.Add(L"1"_XSW);
		array1bis.Add(L"2"_XSW);

		if ( !(array1 == array1bis) ) return 10;
		if ( array1 != array1bis ) return 11;
	}
	
	// Split
	{
		XStringArray array = Split<XStringArray>("   word1   word2    word3   ", " ");
		if ( array[0] != "word1"_XS ) return 31;
		if ( array[1] != "word2"_XS ) return 32;
		if ( array[2] != "word3"_XS ) return 33;
	}
	{
		XStringArray array = Split<XStringArray>("word1, word2, word3", ", ");
		if ( array[0] != "word1"_XS ) return 31;
		if ( array[1] != "word2"_XS ) return 32;
		if ( array[2] != "word3"_XS ) return 33;
	}
	{
		XStringArray array = Split<XStringArray>("   word1   word2    word3   "_XS, " "_XS);
		if ( array[0] != "word1"_XS ) return 31;
		if ( array[1] != "word2"_XS ) return 32;
		if ( array[2] != "word3"_XS ) return 33;
	}

	// Test concat and Split
	{
		XStringW c = array1.ConcatAll(L", "_XSW, L"^"_XSW, L"$"_XSW);
		if ( c != L"^1, 2$"_XSW ) return 1;

		// Split doesn't handle prefix and suffix yet.
		c = array1.ConcatAll(L", "_XSW);

		XStringWArray array1bis = Split<XStringWArray>(c);
		if ( array1 != array1bis ) return 20;
		XStringWArray array2bis = Split<XStringWArray>(c);
		if ( array1 != array2bis ) return 20;
		XStringArray array3bis = Split<XStringArray>(c);
		if ( array1 != array3bis ) return 20;
	}

	XStringWArray array2;
	array2.Add(L"2"_XSW);
	array2.Add(L"1"_XSW);

	if ( array2[0] != L"2"_XSW ) return 30;
	if ( array2[1] != L"1"_XSW ) return 31;


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
