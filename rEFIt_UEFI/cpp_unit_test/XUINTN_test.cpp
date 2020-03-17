#include <Platform.h>
#include "../cpp_foundation/XUINTN.h"
#include "../cpp_foundation/XStringWP.h"

//#include <stdio.h>

#ifdef _MSC_VER
#pragma warning(disable : 4310)
#pragma warning(disable : 4127)
#endif

#define CHECK_CTOR_FAIL(value, code) \
{ \
	stop_at_panic = false; \
	i_have_panicked = false; \
	XUINTN xu(value); \
	if ( !i_have_panicked ) return code; \
	stop_at_panic = true; \
	i_have_panicked = false; \
}

#define CHECK_CTOR_8_16_32_64_int_FAIL(code) \
{ \
    CHECK_CTOR_FAIL((INT8)-10, code); \
    CHECK_CTOR_FAIL((INT16)-10, code+1); \
    CHECK_CTOR_FAIL((INT32)-10, code+2); \
    CHECK_CTOR_FAIL((INT64)-10, code+3); \
    CHECK_CTOR_FAIL((int)-10, code+4); \
}

// return code..code+3
#define CHECK_EQUAL_EQUAL_OPERATOR_OK_(xu, type, value, code) \
{ \
    type u = (value); \
	if ( !(xu == u) ) return code; \
	if ( !(xu.operator ==(u)) ) return code+1; \
	if ( xu != u ) return code+2; \
	if ( xu.operator !=(u) ) return code+3; \
}

// return code..code+33
#define CHECK_EQUAL_EQUAL_OPERATOR_8_16_32_64(type_root, value, code) \
{ \
    XUINTN xu(145); \
     \
    if ( (value) < MAX_##type_root##8 ) { xu = (type_root##8)(value); CHECK_EQUAL_EQUAL_OPERATOR_OK_(xu, type_root##8, (type_root##8)(value), code); } \
	 \
    if ( (value) < MAX_##type_root##16 ) { xu = (type_root##16)(value); CHECK_EQUAL_EQUAL_OPERATOR_OK_(xu, type_root##16, (type_root##16)(value), code+10); } \
	 \
    if ( (value) < MAX_##type_root##32 ) { xu = (type_root##32)(value); CHECK_EQUAL_EQUAL_OPERATOR_OK_(xu, type_root##32, (type_root##32)(value), code+20); } \
	 \
    if ( (value) < MAX_##type_root##64 ) { xu = (type_root##64)(value); CHECK_EQUAL_EQUAL_OPERATOR_OK_(xu, type_root##64, (type_root##64)(value), code+30); } \
}

// return code..code+7
#define CHECK_EQUAL_EQUAL_OPERATOR_FAIL(xu, type, value, code) \
{ \
    type u = (value); \
\
	stop_at_panic = false; \
	i_have_panicked = false; \
	if ( xu == u ) ; \
	if ( !i_have_panicked ) return code+1; \
	i_have_panicked = false; \
	if ( xu.operator ==(u) ) ; \
	if ( !i_have_panicked ) return code+3; \
	i_have_panicked = false; \
	if ( xu != u ) ; \
	if ( !i_have_panicked ) return code+5; \
	i_have_panicked = false; \
	if ( xu.operator !=(u) ) ; \
	if ( !i_have_panicked ) return code+7; \
	stop_at_panic = true; \
	i_have_panicked = false; \
}

// return code..code+33
#define CHECK_PLUS_27_OPERATOR(type_root, type, value, code) \
{ \
	XUINTN xu(27); \
	XUINTN xu2(27); \
	xu2 = xu + (type)(value)-1; \
	if ( xu2.m_value != (UINTN)(value) + 27-1 ) return code; \
}

// return code..code+333
#define CHECK_ALL_PLUS_OPERATOR(type_root, code) \
{ \
	CHECK_PLUS_27_OPERATOR(type_root, type_root##8, MAX_##type_root##8-27, code) \
	CHECK_PLUS_27_OPERATOR(type_root, type_root##16, MAX_##type_root##16-27, code+100) \
	CHECK_PLUS_27_OPERATOR(type_root, type_root##32, MAX_##type_root##32-27, code+200) \
	CHECK_PLUS_27_OPERATOR(type_root, type_root##64, MAX_##type_root##64-27, code+300) \
}

// return code..code+33
#define CHECK_MINUS_OPERATOR(type_root, type, value, code) \
{ \
	XUINTN xu(MAX_##type); \
	XUINTN xu2(0); \
	xu2 = xu - (type)(value); \
	if ( xu2.m_value != (UINTN)MAX_##type - (value) ) return code; \
}

// return code..code+333
#define CHECK_ALL_MINUS_OPERATOR(type_root, code) \
{ \
	CHECK_MINUS_OPERATOR(type_root, type_root##8, 45, code) \
	CHECK_MINUS_OPERATOR(type_root, type_root##16, 45, code+100) \
	CHECK_MINUS_OPERATOR(type_root, type_root##32, 45, code+200) \
	CHECK_MINUS_OPERATOR(type_root, type_root##64, 45, code+300) \
	CHECK_MINUS_OPERATOR(type_root, type_root##8, MAX_##type_root##8-1, code) \
	CHECK_MINUS_OPERATOR(type_root, type_root##16, MAX_##type_root##16-1, code+100) \
	CHECK_MINUS_OPERATOR(type_root, type_root##32, MAX_##type_root##32-1, code+200) \
	CHECK_MINUS_OPERATOR(type_root, type_root##64, MAX_##type_root##64-1, code+300) \
}

// return code..code+33
#define CHECK_SUPERIOR_OPERATOR(value1, type, value, code) \
{ \
	XUINTN xu(value1); \
	if ( !(xu > (type)(value)) ) return code; \
	if ( xu <= (type)(value) ) return code+1; \
	xu = value; \
	if ( !(xu < (type)(value1)) ) return code+2; \
	if ( xu >= (type)(value1) ) return code+3; \
}

#define CHECK_ALL_SUPERIOR_OPERATOR(type_root, code) \
{ \
	CHECK_SUPERIOR_OPERATOR(MAX_##type_root##8, type_root##8, MAX_##type_root##8-1, code) \
	CHECK_SUPERIOR_OPERATOR(MAX_##type_root##16, type_root##16, MAX_##type_root##16-1, code+10) \
	CHECK_SUPERIOR_OPERATOR(MAX_##type_root##32, type_root##32, MAX_##type_root##32-1, code+20) \
	CHECK_SUPERIOR_OPERATOR(MAX_##type_root##64, type_root##64, MAX_##type_root##64-1, code+30) \
	CHECK_SUPERIOR_OPERATOR(MAX_UINT32, unsigned int, MAX_UINT32-1, code+40) \
}

// return code..code+1
#define CHECK_CAST_TO_TYPE_OK(type, value, code) \
{ \
	stop_at_panic = false; \
	i_have_panicked = false; \
	XUINTN xu(value); \
	type u64; \
	u64 = (type)xu; \
	if ( i_have_panicked  || u64 != (value) ) return code; \
	u64 = xu.operator type(); \
	if ( i_have_panicked  || u64 != (value) ) return code+1; \
	stop_at_panic = true; \
	i_have_panicked = false; \
}

// return code..code+5
#define CHECK_CAST_TO_TYPE_8_16_32_64_OK(type_root, code) \
{ \
	CHECK_CAST_TO_TYPE_OK(type_root##8, (UINTN)MAX_##type_root##8, code); \
	CHECK_CAST_TO_TYPE_OK(type_root##16, (UINTN)MAX_##type_root##16, code+2); \
	CHECK_CAST_TO_TYPE_OK(type_root##32, (UINTN)MAX_##type_root##32, code+4); \
	CHECK_CAST_TO_TYPE_OK(type_root##64, (UINTN)MAX_##type_root##64, code+4); \
}

// return code..code+1
#define CHECK_CAST_TO_TYPE_PANIC(type, value, code) \
{ \
	stop_at_panic = false; \
	i_have_panicked = false; \
	XUINTN xu(value); \
	type u8; \
\
	u8 = (type)xu; \
	if ( !i_have_panicked ) return code; \
\
	/* same test as before. be sure that the overloaded cast is really called. */ \
	i_have_panicked = false; \
	u8 = xu.operator type(); \
	if ( !i_have_panicked ) return code+1; \
\
	stop_at_panic = true; \
	i_have_panicked = false; \
}

// return code..code+5
#define CHECK_CAST_TO_TYPE_8_16_32_PANIC(type_root, code) \
{ \
	CHECK_CAST_TO_TYPE_PANIC(type_root##8, (UINTN)MAX_##type_root##8+1, code); \
	CHECK_CAST_TO_TYPE_PANIC(type_root##16, (UINTN)MAX_##type_root##16+1, code+2); \
	CHECK_CAST_TO_TYPE_PANIC(type_root##32, (UINTN)MAX_##type_root##32+1, code+4); \
}

int XUINTN_tests()
{

	CHECK_CTOR_8_16_32_64_int_FAIL(200);


	CHECK_EQUAL_EQUAL_OPERATOR_8_16_32_64(UINT, MAX_UINT8, 300);
	CHECK_EQUAL_EQUAL_OPERATOR_8_16_32_64(UINT, MAX_UINT16, 400);
	CHECK_EQUAL_EQUAL_OPERATOR_8_16_32_64(UINT, MAX_UINT32, 500);
	CHECK_EQUAL_EQUAL_OPERATOR_8_16_32_64(UINT, MAX_UINT64, 600);
	{
		unsigned int ui = MAX_UINT32;
		XUINTN xu = ui;
		CHECK_EQUAL_EQUAL_OPERATOR_OK_(xu, unsigned int, (unsigned int)(MAX_UINT32), 680);
	}
	
	CHECK_EQUAL_EQUAL_OPERATOR_8_16_32_64(INT, MAX_INT8, 700);
	CHECK_EQUAL_EQUAL_OPERATOR_8_16_32_64(INT, MAX_INT16, 800);
	CHECK_EQUAL_EQUAL_OPERATOR_8_16_32_64(INT, MAX_INT32, 900);
	CHECK_EQUAL_EQUAL_OPERATOR_8_16_32_64(INT, MAX_INT64, 1000);
	{
		int i = MAX_INT32;
		XUINTN xu = i;
		CHECK_EQUAL_EQUAL_OPERATOR_OK_(xu, unsigned int, (unsigned int)(MAX_INT32), 690);
	}


	CHECK_ALL_PLUS_OPERATOR(UINT, 2000);
	CHECK_ALL_PLUS_OPERATOR(INT, 3000);
	CHECK_ALL_MINUS_OPERATOR(UINT, 2000);
	CHECK_ALL_MINUS_OPERATOR(INT, 3000);

	// Check + operation that fail
	{
        stop_at_panic = false;
        i_have_panicked = false;

		XUINTN xu1 = (UINT64)MAX_UINT64 - 1;
		XUINTN xu2 = xu1 + 2;
		if ( !i_have_panicked ) return 100;
		(void)xu2;

        stop_at_panic = true;
        i_have_panicked = false;
	}
	
	CHECK_ALL_SUPERIOR_OPERATOR(UINT, 5000);

	CHECK_CAST_TO_TYPE_8_16_32_64_OK(UINT, 210);
	if ( sizeof(unsigned int) == 4 ) {
		CHECK_CAST_TO_TYPE_OK(unsigned int, (UINTN)MAX_UINT32, 220);
	}else{
		panic("Please define unsigned int test in XUINTN_test.cpp");
	}
	CHECK_CAST_TO_TYPE_8_16_32_PANIC(UINT, 220);
	if ( sizeof(unsigned int) == 4 ) {
		CHECK_CAST_TO_TYPE_PANIC(unsigned int, (UINTN)MAX_UINT32+1, 230);
	}else{
		panic("Please define unsigned int test in XUINTN_test.cpp");
	}
	
	
	CHECK_CAST_TO_TYPE_8_16_32_64_OK(INT, 240);
	if ( sizeof(unsigned int) == 4 ) {
		CHECK_CAST_TO_TYPE_OK(unsigned int, (UINTN)MAX_UINT32, 250);
	}else{
		panic("Please define unsigned int test in XUINTN_test.cpp");
	}
	CHECK_CAST_TO_TYPE_8_16_32_PANIC(INT, 260);
	if ( sizeof(unsigned int) == 4 ) {
		CHECK_CAST_TO_TYPE_PANIC(int, (UINTN)MAX_INT32+1, 270);
	}else{
		panic("Please define unsigned int test in XUINTN_test.cpp");
	}


	return 0;
}

