#include <Platform.h>
#include "../cpp_foundation/XString.h"
#include "../cpp_foundation/XStringArray.h"
#include "global_test.h"



CHAR16 *Old1_AddLoadOption(IN CONST CHAR16 *LoadOptions, IN CONST CHAR16 *LoadOption)
{
  // If either option strings are null nothing to do
  if (LoadOptions == NULL)
  {
    if (LoadOption == NULL) return NULL;
    // Duplicate original options as nothing to add
    return EfiStrDuplicate(LoadOption);
  }
  // If there is no option or it is already present duplicate original
  // with XStringW it will be replaced by if (LoadOptions.contains(LoadOption))
  else if ((LoadOption == NULL) || StrStr(LoadOptions, LoadOption))
    return EfiStrDuplicate(LoadOptions);
  // Otherwise add option
//  return PoolPrint(L"%s %s", LoadOptions, LoadOption); //LoadOptions + LoadOption
  return SWPrintf("%ls %ls", LoadOptions, LoadOption).forgetDataWithoutFreeing(); //LoadOptions + LoadOption
}

CHAR16 *Old1_RemoveLoadOption(IN CONST CHAR16 *LoadOptions, IN CONST CHAR16 *LoadOption)
{
  CONST CHAR16 *Placement;
  CHAR16 *NewLoadOptions;
  UINTN   Length, Offset, OptionLength;

  //DBG("LoadOptions: '%ls', remove LoadOption: '%ls'\n", LoadOptions, LoadOption);
  // If there are no options then nothing to do
  if (LoadOptions == NULL) return NULL;
  // If there is no option to remove then duplicate original
  if (LoadOption == NULL) return EfiStrDuplicate(LoadOptions);
  // If not present duplicate original
  Placement = StrStr(LoadOptions, LoadOption);
  if (Placement == NULL) return EfiStrDuplicate(LoadOptions);

  // Get placement of option in original options
  Offset = UINTN(Placement - LoadOptions);
  Length = StrLen(LoadOptions);
  OptionLength = StrLen(LoadOption);

  // If this is just part of some larger option (contains non-space at the beginning or end)
  if ((Offset > 0 && LoadOptions[Offset - 1] != L' ') ||
      ((Offset + OptionLength) < Length && LoadOptions[Offset + OptionLength] != L' ')) {
    return EfiStrDuplicate(LoadOptions);
  }

  // Consume preceeding spaces
  while (Offset > 0 && LoadOptions[Offset - 1] == L' ') {
    OptionLength++;
    Offset--;
  }

  // Consume following spaces
  while (LoadOptions[Offset + OptionLength] == L' ') {
   OptionLength++;
  }

  // If it's the whole string return NULL
  if (OptionLength == Length) return NULL;

  if (Offset == 0) {
    // Simple case - we just need substring after OptionLength position
    NewLoadOptions = EfiStrDuplicate(LoadOptions + OptionLength);
  } else {
    // The rest of LoadOptions is Length - OptionLength, but we may need additional space and ending 0
    NewLoadOptions = (__typeof__(NewLoadOptions))AllocateZeroPool((Length - OptionLength + 2) * sizeof(CHAR16));
    // Copy preceeding substring
    CopyMem(NewLoadOptions, (void*)LoadOptions, Offset * sizeof(CHAR16));
    if ((Offset + OptionLength) < Length) {
      // Copy following substring, but include one space also
      OptionLength--;
      CopyMem(NewLoadOptions + Offset, (void*)(LoadOptions + Offset + OptionLength), (Length - OptionLength - Offset) * sizeof(CHAR16));
    }
  }
  return NewLoadOptions;
}






//
//static XString AddLoadOption(IN CONST XString& LoadOptions, IN CONST XString& LoadOption)
//{
//  // LoadOptions assumed out
//  // If either option strings are null nothing to do
//  if (LoadOptions.isEmpty()) //initially empty so return new option even if empty
//  {
//    // return LoadOption
//    return LoadOption;
//  }
//  // If there is no option or it is already present duplicate original
//  else {
//	  if ( LoadOptions.contains(LoadOption) ) return LoadOptions; //good
//	  // Otherwise add option
////	  return SPrintf("%s %s", LoadOptions.c_str(), LoadOption.c_str()); //LoadOptions + LoadOption
//    return LoadOptions + " "_XS8 + LoadOption; //why not?
//  }
//}
//
//static XString RemoveLoadOption(IN const XString& LoadOptions, IN const XString& LoadOption)
//{
////  CONST CHAR16 *Placement;
////  CHAR16 *NewLoadOptions;
////  UINTN   Length, Offset, OptionLength;
//
//  //DBG("LoadOptions: '%ls', remove LoadOption: '%ls'\n", LoadOptions, LoadOption);
//  // If there are no options then nothing to do
//  if (LoadOptions.isEmpty()) return ""_XS8;
//  // If there is no option to remove then duplicate original
//  if (LoadOption.isEmpty()) return LoadOptions;
//  // If not present duplicate original
//  xsize Offset = LoadOptions.indexOf(LoadOption);
//  if ( Offset == MAX_XSIZE ) return LoadOptions;
//
//  // Get placement of option in original options
////  Offset = (Placement - LoadOptions);
//  xsize Length = LoadOptions.length();
//  xsize OptionLength = LoadOption.length();
//
//  // If this is just part of some larger option (contains non-space at the beginning or end)
//  if ((Offset > 0 && LoadOptions[Offset - 1] != ' ') ||
//      ((Offset + OptionLength) < Length && LoadOptions[Offset + OptionLength] != ' ')) {
//    return LoadOptions;
//  }
//
//  // Consume preceeding spaces
//  while (Offset > 0 && LoadOptions[Offset - 1] == ' ') {
//    OptionLength++;
//    Offset--;
//  }
//
//  // Consume following spaces
//  while (LoadOptions[Offset + OptionLength] == ' ') {
//   OptionLength++;
//  }
//
//  // If it's the whole string return NULL
//  if (OptionLength == Length) return ""_XS8;
//
//  XString NewLoadOptions;
//  if (Offset == 0) {
//    // Simple case - we just need substring after OptionLength position
//    NewLoadOptions = LoadOptions.subString(OptionLength, MAX_XSIZE);
//  } else {
//    // Copy preceeding substring
//	NewLoadOptions = LoadOptions.subString(0, Offset);
////    CopyMem(NewLoadOptions, LoadOptions, Offset * sizeof(CHAR16));
//    if ((Offset + OptionLength) < Length) {
//      // Copy following substring, but include one space also
//      OptionLength--;
//	  NewLoadOptions += LoadOptions.subString(Offset + OptionLength, MAX_XSIZE);
////      CopyMem(NewLoadOptions + Offset, LoadOptions + Offset + OptionLength, (Length - OptionLength - Offset) * sizeof(CHAR16));
//    }
//  }
//  return NewLoadOptions;
//}
//
//






int BootOptions_tests()
{

#ifdef JIEF_DEBUG
//	printf("XStringW_tests -> Enter\n");
#endif

	{
		CHAR16* LoadOptions = NULL;
		
		LoadOptions = Old1_AddLoadOption(LoadOptions, L"opt1");
		LoadOptions = Old1_AddLoadOption(LoadOptions, L"opt2");
		LoadOptions = Old1_AddLoadOption(LoadOptions, L"opt3");
		
		if ( XString8().takeValueFrom(LoadOptions) != "opt1 opt2 opt3"_XS8 ) return 1;
		
		CHAR16* LoadOptions1 = Old1_RemoveLoadOption(LoadOptions, L"opt1");
		if ( XString8().takeValueFrom(LoadOptions1) != "opt2 opt3"_XS8 ) return 2;
		CHAR16* LoadOptions2 = Old1_RemoveLoadOption(LoadOptions, L"opt2");
		if ( XString8().takeValueFrom(LoadOptions2) != "opt1 opt3"_XS8 ) return 3;
		CHAR16* LoadOptions3 = Old1_RemoveLoadOption(LoadOptions, L"opt3");
		if ( XString8().takeValueFrom(LoadOptions3) != "opt1 opt2"_XS8 ) return 4;
	}
//	{
//		XString LoadOptions;
//		
//		LoadOptions = AddLoadOption(LoadOptions, "opt1"_XS8);
//		LoadOptions = AddLoadOption(LoadOptions, "opt2"_XS8);
//		LoadOptions = AddLoadOption(LoadOptions, "opt3"_XS8);
//		
//		if ( LoadOptions != "opt1 opt2 opt3"_XS8 ) return 10;
//		
//		XString LoadOptions1 = RemoveLoadOption(LoadOptions, "opt1"_XS8);
//		if ( LoadOptions1 != "opt2 opt3"_XS8 ) return 11;
//		XString LoadOptions2 = RemoveLoadOption(LoadOptions, "opt2"_XS8);
//		if ( LoadOptions2 != "opt1 opt3"_XS8 ) return 12;
//		XString LoadOptions3 = RemoveLoadOption(LoadOptions, "opt3"_XS8);
//		if ( LoadOptions3 != "opt1 opt2"_XS8 ) return 13;
//	}
	{
		XStringArray LoadOptions;
		
		LoadOptions.AddID("opt1"_XS8);
		LoadOptions.AddID("opt2"_XS8);
		LoadOptions.AddID("opt3"_XS8);
		
		if ( LoadOptions.ConcatAll(" "_XS8) != "opt1 opt2 opt3"_XS8 ) return 30;
		
		XStringArray LoadOptions1 = LoadOptions;
		LoadOptions1.remove("opt1"_XS8);
		if ( LoadOptions1.ConcatAll(" "_XS8) != "opt2 opt3"_XS8 ) return 31;
		XStringArray LoadOptions2 = LoadOptions;
		LoadOptions2.remove("opt2"_XS8);
		if ( LoadOptions2.ConcatAll(" "_XS8) != "opt1 opt3"_XS8 ) return 32;
		XStringArray LoadOptions3 = LoadOptions;
		LoadOptions3.remove("opt3"_XS8);
		if ( LoadOptions3.ConcatAll(" "_XS8) != "opt1 opt2"_XS8 ) return 33;
	}

	return 0;
}

