#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile

#if 0
#define DBG(...) DebugLog(2, __VA_ARGS__)
#else
#define DBG(...)
#endif



#if defined(__clang__)
  typedef void (destructor_func_t)();
#elif defined(__GNUC__)
  typedef void (destructor_func_t)(void*);
#elif defined(_MSC_VER)
typedef void (__cdecl destructor_func_t)(void);
// Next was needed at some point. 2020-04-08 today it's breaking compilation in VS Studio (but not with edkII build system)
//extern "C" int _fltused = 0; // it should be a single underscore since the double one is the mangled name
#else
# error compiler not supported
#endif

typedef destructor_func_t* destructor_func_ptr_t;

struct atexit_func_entry_t
{
	destructor_func_ptr_t destructor_func;
#if !defined(__clang__) && defined(__GNUC__)
	void *obj_ptr;
	void *dso_handle;
#endif
};



static atexit_func_entry_t*atexit_func_entry_array = NULL;
static UINTN atexit_func_entry_count = 0;
static UINTN atexit_func_entry_size = 0;


#if defined (__clang__)

extern "C" int atexit(destructor_func_ptr_t destructor_func)
{
DBG("atexit(%p) sizeof=%d\n", destructor_func, sizeof(atexit_func_entry_t));

#elif defined(__GNUC__)

extern "C" int __cxa_atexit(destructor_func_ptr_t destructor_func, void *objptr, void *dso)
{
DBG("atexit(%p, %p, %p, %d)\n", destructor_func, objptr, dso, sizeof(atexit_func_entry_t));

#elif defined(_MSC_VER)

extern "C" int atexit(void(__cdecl *destructor_func)(void))
{
	DBG("atexit(%p, %p, %p, %d)\n", destructor_func, objptr, dso, sizeof(atexit_func_entry_t));

#endif
	if ( !atexit_func_entry_array )
	{
DBG("atexit : allocate\n");
		atexit_func_entry_array = (atexit_func_entry_t*)AllocatePool(16*sizeof(atexit_func_entry_t));
		if ( !atexit_func_entry_array ) {
			DBG("atexit : allocate returned NULL\n");
			CpuDeadLoop();
		}
		atexit_func_entry_count = 0;
		atexit_func_entry_size = 16;
	}

	if (atexit_func_entry_count >= atexit_func_entry_size)
	{
DBG("atexit : reallocate atexit_func_entry_array=%d\n", (UINTN)atexit_func_entry_array);
		atexit_func_entry_array = (atexit_func_entry_t *)ReallocatePool(atexit_func_entry_size*sizeof(atexit_func_entry_t), (atexit_func_entry_size+16)*sizeof(atexit_func_entry_t), atexit_func_entry_array);
		if ( !atexit_func_entry_array ) {
			DBG("atexit : reallocate returned NULL\n");
			CpuDeadLoop();
		}
		atexit_func_entry_size += 16;
	}
	atexit_func_entry_array[atexit_func_entry_count].destructor_func = destructor_func;
#if !defined(__clang__) && defined(__GNUC__)
	atexit_func_entry_array[atexit_func_entry_count].obj_ptr = objptr;
	atexit_func_entry_array[atexit_func_entry_count].dso_handle = dso;
#endif
	atexit_func_entry_count++;
DBG("atexit : exit (atexit_func_entry_count=%d)\n", atexit_func_entry_count);
	return 0; /*I would prefer if functions returned 1 on success, but the ABI says...*/
}

void destruct_globals_objects(void *f)
{
	UINTN i = atexit_func_entry_count;

	while (i--)
	{
#   if defined(__clang__)

			DBG("destruct_globals_objects: idx=%d %p\n", i, atexit_func_entry_array[i].destructor_func);
			(*atexit_func_entry_array[i].destructor_func)();
			atexit_func_entry_array[i].destructor_func = NULL;
			atexit_func_entry_count--;

#   elif defined(__GNUC__)

			DBG("destruct_globals_objects: idx=%d %d, %p, %p\n", i, atexit_func_entry_array[i].destructor_func, atexit_func_entry_array[i].obj_ptr, atexit_func_entry_array[i].dso_handle);
			(*atexit_func_entry_array[i].destructor_func)(atexit_func_entry_array[i].obj_ptr);
			atexit_func_entry_array[i].destructor_func = NULL;
			atexit_func_entry_count--;

#   endif
	}
}
