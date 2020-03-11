#include "../cpp_foundation/XStringW.h"

class XStringWTest : public XStringW
{
public:
  XStringWTest(const wchar_t *S) : XStringW()
  {
    StrCpy(S);
  }
};

extern XStringWTest global_str3;
extern XStringWTest global_str4;
