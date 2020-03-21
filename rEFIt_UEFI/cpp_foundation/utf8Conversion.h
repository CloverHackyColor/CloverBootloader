//
//  utf8Conversion.hpp
//
//  Created by jief the 24 Feb 2020.
//

#ifndef utf816Conversion_hpp
#define utf816Conversion_hpp

#include <posix.h>

UINTN StrLenInWChar(const char *src);
UINTN utf8ToWChar(wchar_t* dst, UINTN dst_max_len,  const char *s);

#endif /* utf816Conversion_hpp */
