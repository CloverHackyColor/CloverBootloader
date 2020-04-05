#include <Platform.h>
#include "../cpp_foundation/XString.h"
#include "../cpp_foundation/XStringArray.h"
#include "../cpp_foundation/XStringW.h"
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
  // with XStringW it will be replaced by if (LoadOptions.ExistIn(LoadOption))
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
  Offset = (Placement - LoadOptions);
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








static XString AddLoadOption(IN CONST XString& LoadOptions, IN CONST XString& LoadOption)
{
  // If either option strings are null nothing to do
  if (LoadOptions.isEmpty())
  {
    // return LoadOption as nothing to add
    return LoadOption;
  }
  // If there is no option or it is already present duplicate original
  else {
	  if ( LoadOptions.ExistIn(LoadOption) ) return LoadOptions;
	  // Otherwise add option
	  return SPrintf("%s %s", LoadOptions.c_str(), LoadOption.c_str()); //LoadOptions + LoadOption
  }
}

static XString RemoveLoadOption(IN const XString& LoadOptions, IN const XString& LoadOption)
{
//  CONST CHAR16 *Placement;
//  CHAR16 *NewLoadOptions;
//  UINTN   Length, Offset, OptionLength;

  //DBG("LoadOptions: '%ls', remove LoadOption: '%ls'\n", LoadOptions, LoadOption);
  // If there are no options then nothing to do
  if (LoadOptions.isEmpty()) return ""_XS;
  // If there is no option to remove then duplicate original
  if (LoadOption.isEmpty()) return LoadOptions;
  // If not present duplicate original
  xsize Offset = LoadOptions.IdxOf(LoadOption);
  if ( Offset == MAX_XSIZE ) return LoadOptions;

  // Get placement of option in original options
//  Offset = (Placement - LoadOptions);
  xsize Length = LoadOptions.length();
  xsize OptionLength = LoadOption.length();

  // If this is just part of some larger option (contains non-space at the beginning or end)
  if ((Offset > 0 && LoadOptions[Offset - 1] != ' ') ||
      ((Offset + OptionLength) < Length && LoadOptions[Offset + OptionLength] != ' ')) {
    return LoadOptions;
  }

  // Consume preceeding spaces
  while (Offset > 0 && LoadOptions[Offset - 1] == ' ') {
    OptionLength++;
    Offset--;
  }

  // Consume following spaces
  while (LoadOptions[Offset + OptionLength] == ' ') {
   OptionLength++;
  }

  // If it's the whole string return NULL
  if (OptionLength == Length) return ""_XS;

  XString NewLoadOptions;
  if (Offset == 0) {
    // Simple case - we just need substring after OptionLength position
    NewLoadOptions = LoadOptions.SubString(OptionLength, MAX_XSIZE);
  } else {
    // Copy preceeding substring
	NewLoadOptions = LoadOptions.SubString(0, Offset);
//    CopyMem(NewLoadOptions, LoadOptions, Offset * sizeof(CHAR16));
    if ((Offset + OptionLength) < Length) {
      // Copy following substring, but include one space also
      OptionLength--;
	  NewLoadOptions += LoadOptions.SubString(Offset + OptionLength, MAX_XSIZE);
//      CopyMem(NewLoadOptions + Offset, LoadOptions + Offset + OptionLength, (Length - OptionLength - Offset) * sizeof(CHAR16));
    }
  }
  return NewLoadOptions;
}











int BootOptions_tests()
{

#ifdef JIEF_DEBUG
//	DebugLog(2, "XStringW_tests -> Enter\n");
#endif

	{
		CHAR16* LoadOptions = NULL;
		
		LoadOptions = Old1_AddLoadOption(LoadOptions, L"opt1");
		LoadOptions = Old1_AddLoadOption(LoadOptions, L"opt2");
		LoadOptions = Old1_AddLoadOption(LoadOptions, L"opt3");
		
		if ( XString().takeValueFrom(LoadOptions) != "opt1 opt2 opt3"_XS ) return 1;
		
		CHAR16* LoadOptions1 = Old1_RemoveLoadOption(LoadOptions, L"opt1");
		if ( XString().takeValueFrom(LoadOptions1) != "opt2 opt3"_XS ) return 1;
		CHAR16* LoadOptions2 = Old1_RemoveLoadOption(LoadOptions, L"opt2");
		if ( XString().takeValueFrom(LoadOptions2) != "opt1 opt3"_XS ) return 1;
		CHAR16* LoadOptions3 = Old1_RemoveLoadOption(LoadOptions, L"opt3");
		if ( XString().takeValueFrom(LoadOptions3) != "opt1 opt2"_XS ) return 1;
	}
	{
		XString LoadOptions;
		
		LoadOptions = AddLoadOption(LoadOptions, "opt1"_XS);
		LoadOptions = AddLoadOption(LoadOptions, "opt2"_XS);
		LoadOptions = AddLoadOption(LoadOptions, "opt3"_XS);
		
		if ( LoadOptions != "opt1 opt2 opt3"_XS ) return 1;
		
		XString LoadOptions1 = RemoveLoadOption(LoadOptions, "opt1"_XS);
		if ( LoadOptions1 != "opt2 opt3"_XS ) return 1;
		XString LoadOptions2 = RemoveLoadOption(LoadOptions, "opt2"_XS);
		if ( LoadOptions2 != "opt1 opt3"_XS ) return 1;
		XString LoadOptions3 = RemoveLoadOption(LoadOptions, "opt3"_XS);
		if ( LoadOptions3 != "opt1 opt2"_XS ) return 1;
	}
	{
		XStringArray LoadOptions;
		
		LoadOptions.AddID("opt1"_XS);
		LoadOptions.AddID("opt2"_XS);
		LoadOptions.AddID("opt3"_XS);
		
		if ( LoadOptions.ConcatAll(" "_XS) != "opt1 opt2 opt3"_XS ) return 1;
		
		XStringArray LoadOptions1 = LoadOptions;
		LoadOptions1.Remove("opt1"_XS);
		if ( LoadOptions1.ConcatAll(" "_XS) != "opt2 opt3"_XS ) return 1;
		XStringArray LoadOptions2 = LoadOptions;
		LoadOptions2.Remove("opt2"_XS);
		if ( LoadOptions2.ConcatAll(" "_XS) != "opt1 opt3"_XS ) return 1;
		XStringArray LoadOptions3 = LoadOptions;
		LoadOptions3.Remove("opt3"_XS);
		if ( LoadOptions3.ConcatAll(" "_XS) != "opt1 opt2"_XS ) return 1;
	}

	return 0;
}

