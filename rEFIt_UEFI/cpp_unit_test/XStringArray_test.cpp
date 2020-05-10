#include <Platform.h>
#include "../cpp_foundation/XStringArray.h"

int XStringArray_tests()
{

#ifdef JIEF_DEBUG
//	printf("XStringWArray_tests -> Enter\n");
#endif

    {
        XStringWArray array1;

        if ( !array1.isEmpty() ) return 1;

        array1.Add(L"word1"_XSW);
        if ( array1.isEmpty() ) return 2;
        if ( array1[0] != "word1"_XS8 ) return 21;
        array1.Add(L"other2"_XSW);
        if ( array1[1] != "other2"_XS8 ) return 21;

        if ( !array1.contains(L"other2"_XSW) ) return 5;
        if ( !array1.containsIC(L"oTHer2"_XSW) ) return 6;
    }
    
	// Test == and !=
	{
        
        XStringWArray array1;
        array1.Add(L"word1"_XSW);
        array1.Add(L"other2"_XSW);

		XStringWArray array1bis;
		array1bis.Add(L"word1"_XSW);
		array1bis.Add(L"other2"_XSW);

		if ( !(array1 == array1bis) ) return 10;
		if ( array1 != array1bis ) return 11;
	}
	
	// Split
	{
		XStringArray array = Split<XStringArray>("   word1   word2    word3   ", " ");
		if ( array[0] != "word1"_XS8 ) return 31;
		if ( array[1] != "word2"_XS8 ) return 32;
		if ( array[2] != "word3"_XS8 ) return 33;
	}
	{
		XStringArray array = Split<XStringArray>("word1, word2, word3", ", ");
		if ( array[0] != "word1"_XS8 ) return 41;
		if ( array[1] != "word2"_XS8 ) return 42;
		if ( array[2] != "word3"_XS8 ) return 43;
	}
	{
		XStringArray array = Split<XStringArray>("   word1   word2    word3   "_XS8, " "_XS8);
		if ( array[0] != "word1"_XS8 ) return 51;
		if ( array[1] != "word2"_XS8 ) return 52;
		if ( array[2] != "word3"_XS8 ) return 53;
	}
    {
        XStringArray array = Split<XStringArray>("   word1   word2    word3   "_XS8, " "_XS8);
        XString8 xs = array.ConcatAll(' ', '^', '$');
        if ( xs  != "^word1 word2 word3$"_XS8 ) return 31;
    }
    
    // Test concat and Split
    {
        XStringWArray array;
        array.Add(L"word1"_XSW);
        array.Add("other2"_XS8);
        array.Add("3333");
        array.Add(L"4th_item");
        {
            XStringW c = array.ConcatAll(L", "_XSW, L"^"_XSW, L"$"_XSW);
            if ( c != L"^word1, other2, 3333, 4th_item$"_XSW ) return 1;
        }
        {
            XStringW c = array.ConcatAll(L", ", L"^", L"$");
            if ( c != L"^word1, other2, 3333, 4th_item$"_XSW ) return 1;
        }

        // Split doesn't handle prefix and suffix yet.
        XStringW c = array.ConcatAll(L", ");

        XStringWArray arraybis = Split<XStringWArray>(c);
        if ( array != arraybis ) return 20;
        XStringArray array3bis = Split<XStringArray>(c);
        if ( array != array3bis ) return 20;
    }
    // Test Split char[64]
    {
        char buf[64];
        strcpy(buf, "word1 other2 3333 4th_item");
        XStringArray array = Split<XStringArray>(buf, " ");

        if ( array[0] != "word1"_XS8 ) return 31;
        if ( array[1] != "other2"_XS8 ) return 32;
        if ( array[2] != "3333"_XS8 ) return 33;
        if ( array[3] != "4th_item"_XS8 ) return 34;
    }
    // Test concat and Split @Pene
    {
        XStringArray array;
        array.Add(L"word1");
        array.Add(L"other2");
        array.Add(L"3333");
        array.Add(L"4th_item");
        
        XStringArray LoadOptions2;
        
        LoadOptions2 = Split<XStringArray>(array.ConcatAll(" "_XS8).wc_str(), " ");
        if ( LoadOptions2 != array ) return 22;
        
        LoadOptions2 = Split<XStringArray>(array.ConcatAll(" "_XS8), " ");
        if ( LoadOptions2 != array ) return 22;
        
        LoadOptions2 = Split<XStringArray>(array.ConcatAll(" "_XS8), " "_XS8);
        if ( LoadOptions2 != array ) return 22;
        
        LoadOptions2 = Split<XStringArray>(array.ConcatAll(" "), " ");
        if ( LoadOptions2 != array ) return 22;
        
        LoadOptions2 = array;
        if ( LoadOptions2 != array ) return 22;
    }
    //
    {
        XStringWArray array;
        array.Add(L"word1"_XSW);
        array.Add(L"other2"_XSW);
        array.Add(L"3333"_XSW);
        array.Add(L"4th_item"_XSW);

        XStringWArray array2;
        array2.Add(L"word1"_XSW);
        array2.Add(L"3333"_XSW);
        array2.Add(L"other2"_XSW);
        array2.Add(L"4th_item"_XSW);

        if ( array == array2 ) return 40; // Array != because order is different
        if ( !array.Same(array2) ) return 41; // Arrays are the same

    }
    {
        XStringWArray array1;
        array1.Add(L"word1"_XSW);
        array1.Add(L"other2"_XSW);

        array1.AddNoNull(L"3333"_XSW);
        if ( array1.size() != 3 ) return 50;
        array1.AddNoNull(L""_XSW);
        if ( array1.size() != 3 ) return 51;
        array1.AddEvenNull(XStringW());
        if ( array1.size() != 4 ) return 52;
        array1.AddID(L"other2"_XSW);
        if ( array1.size() != 4 ) return 53;
    }
    {
        XStringArray array;
        array.Add(L"word1");
        array.Add(L"other2");
        array.Add(L"3333");
        array.Add(L"4th_item");
		
		array.remove("WOrd1"_XS8);
        if ( !array.contains("word1"_XS8) ) return 22;
		array.remove("word1"_XS8);
        if ( array.contains("word1"_XS8) ) return 22;
		array.removeIC("oTHEr2"_XS8);
        if ( array.contains("other2"_XS8) ) return 22;
		array.removeIC("4th_ITEM"_XS8);
        if ( array.contains("4th_item"_XS8) ) return 22;
		XString8 c = array.ConcatAll();
//		printf("c=%s\n", c.c_str());
	}
    {
        XStringArray array;
        array.Add(L"splash");
        array.Add(L"quiet");

		array.remove("splash"_XS8);
        if ( array.contains("splash﻿"_XS8) ) return 22;
		array.removeIC("quiet"_XS8);
        if ( array.contains("quiet"_XS8) ) return 22;
        if ( array.size() != 0 ) return 22;
		XString8 c = array.ConcatAll();
//		printf("c=%s\n", c.c_str());
	}



  return 0;
}
