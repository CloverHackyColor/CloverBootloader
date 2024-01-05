
#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "../cpp_foundation/XObjArray.h"
#include "../cpp_foundation/XArray.h"

class TestObjInt
{
  public:
    UINTN m_v;
    bool* m_destructor_called;

  TestObjInt(UINTN v, bool* destructor_called) : m_v(v), m_destructor_called(destructor_called)
  {
  	*m_destructor_called=false;
  };
  TestObjInt(const TestObjInt&) = delete;
  TestObjInt& operator=(const TestObjInt&) = delete;

  ~TestObjInt()
  {
  	*m_destructor_called = true;
  };
};

int XObjArray_tests()
{
	bool m_destructor_called11 = false; // vs2017 warning
	bool m_destructor_called12 = false; // vs2017 warning
	bool m_destructor_called13 = false; // vs2017 warning
	bool m_destructor_called14 = false; // vs2017 warning

	TestObjInt* obj14 = new TestObjInt(14, &m_destructor_called14);
	{
		TestObjInt obj11(11, &m_destructor_called11);

		{
			XObjArray<TestObjInt> array1;
			array1.AddReference(&obj11, false); // Do not free this object when the array is freed, This object declared on the stack, so it'll be free by C++
			array1.AddReference(new TestObjInt(12, &m_destructor_called12), true); // Free this object. C++ doesn't keep any reference of new allocated object. So it won't be freed by C++
			array1.AddReference(new TestObjInt(13, &m_destructor_called13), true); // Free this object. C++ doesn't keep any reference of new allocated object. So it won't be freed by C++
			array1.AddReference(obj14, false); // We keep a reference an obj14. Let's not ask the array to free it.

			if ( array1[0].m_v != 11 ) return 1;
			if ( array1[1].m_v != 12 ) return 2;
			if ( array1[2].m_v != 13 ) return 3;
			if ( array1[3].m_v != 14 ) return 4;

			array1.RemoveAtIndex(1);

			if ( array1[1].m_v != 13 ) return 5;
		}

		// Here the array and objects we added saying true as second parameter of AddReference.
		if ( !m_destructor_called12 ) return 6;
		if ( !m_destructor_called13 ) return 7;
		// obj11 and obj14 should not be destroyed yet.
		if ( m_destructor_called11 ) return 8;
		if ( m_destructor_called14 ) return 9;
	}
	// obj11 must be destroyed by C++.
	if ( !m_destructor_called11 ) return 10;

	delete(obj14);
	if ( !m_destructor_called14 ) return 11;

  {
    XObjArray<XString8> testCtor(true, new XString8("s1"_XS8), new XString8("s2"_XS8), new XString8("s3"_XS8), NULL);
    if ( testCtor[0] != "s1"_XS8 ) return 20;
    if ( testCtor[1] != "s2"_XS8 ) return 21;
    if ( testCtor[2] != "s3"_XS8 ) return 22;
  }
	return 0;
}
