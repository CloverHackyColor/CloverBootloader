/*
 *
 */

#ifndef __remove_ref_h__
#define __remove_ref_h__



	// STRUCT TEMPLATE remove_reference
template<class _Ty>
struct _typeofam_remove_ref
{	// remove reference
	using type = _Ty;
};

template<class _Ty>
struct _typeofam_remove_ref<_Ty&>
{	// remove reference
	using type = _Ty;
};
//
//template<class _Ty>
//	struct remove_ref<_Ty&&>
//	{	// remove rvalue reference
//	using type = _Ty;
//	};



#endif
