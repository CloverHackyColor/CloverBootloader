//
//  utf8Conversion.hpp
//
//  Created by jief the 24 Feb 2020.
//

#ifndef utf816Conversion_hpp
#define utf816Conversion_hpp


UINTN StrLenInWChar(const char *src, UINTN src_len);
UINTN utf8ToWChar(wchar_t* dst, UINTN dst_max_len,  const char *s, UINTN src_len);

#endif /* utf816Conversion_hpp */
