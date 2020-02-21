
	// STRUCT TEMPLATE remove_reference
template<class _Ty>
	struct remove_ref
	{	// remove reference
	using type = _Ty;
	};

template<class _Ty>
	struct remove_ref<_Ty&>
	{	// remove reference
	using type = _Ty;
	};
//
//template<class _Ty>
//	struct remove_ref<_Ty&&>
//	{	// remove rvalue reference
//	using type = _Ty;
//	};
