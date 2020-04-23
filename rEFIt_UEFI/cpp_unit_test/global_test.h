#include "../cpp_foundation/XString.h"

class XStringWTest : public XStringW
{
public:
  XStringWTest(const wchar_t *S) : XStringW()
  {
    strcpy(S);
  }
};

class XStringTest : public XString
{
public:
  XStringTest(const char *S) : XString()
  {
    strcpy(S);
  }
};

extern XStringTest global_str1;
extern XStringTest global_str2;

extern XStringWTest global_str3;
extern XStringWTest global_str4;
