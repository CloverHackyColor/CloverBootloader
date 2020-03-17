//*************************************************************************************************
//*************************************************************************************************
//
//                                          STRING
//
//*************************************************************************************************
//*************************************************************************************************

#if !defined(__XUINTN_H__)
#define __XUINTN_H__

#include "XToolsCommon.h"

class XUINTN
{
public:
	UINTN m_value;

public:
	XUINTN() : m_value(0) {};
	XUINTN(UINT64 v) : m_value(v) {};
	XUINTN(unsigned int v) : m_value(v) {};
	XUINTN(INT64 v) { if ( v < 0 ) panic("XUINTN(INT64 v) -> value is < 0"); m_value=(UINT64)v; };
	XUINTN(int v) { if ( v < 0 ) panic("XUINTN(int v) -> value is < 0"); m_value=(UINT64)v; };

	bool operator != (UINT8 u) const { return m_value != u; };
	bool operator != (UINT16 u) const { return m_value != u; };
	bool operator != (UINT32 u) const { return m_value != u; };
	bool operator != (UINT64 u) const { return m_value != u; };

	bool operator != (INT8 i) const { if ( i < 0 ) return false; return m_value != (UINT8)i; };
	bool operator != (INT16 i) const { if ( i < 0 ) return false; return m_value != (UINT16)i; };
	bool operator != (INT32 i) const { if ( i < 0 ) return false; return m_value != (UINT32)i; };
	bool operator != (INT64 i) const { if ( i < 0 ) return false; return m_value != (UINT64)i; };

	bool operator == (UINT8 u) const { return m_value == u; };
	bool operator == (UINT16 u) const { return m_value == u; };
	bool operator == (UINT32 u) const { return m_value == u; };
	bool operator == (UINT64 u) const { return m_value == u; };

	bool operator == (INT8 i) const { if ( i < 0 ) return false; return m_value == (UINT8)i; };
	bool operator == (INT16 i) const { if ( i < 0 ) return false; return m_value == (UINT16)i; };
	bool operator == (INT32 i) const { if ( i < 0 ) return false; return m_value == (UINT32)i; };
	bool operator == (INT64 i) const { if ( i < 0 ) return false; return m_value == (UINT64)i; };

/* seems not needed to define all type for operator + and -, contrary to the other operators. */
//	UINTN operator + (int i) const { if ( (i>=0 && value + (unsigned int)i < value) || (i<0 && value + (unsigned int)i > value)) panic("UINTN operator + (int i) const -> overflow"); return (UINTN)(value + (unsigned int)i); };
//	UINTN operator + (UINT8 u) const { if ( value + u < value ) panic("UINTN operator + (UINT8 u) -> overflow"); return value + u; };
//	UINTN operator + (UINT16 u) const { if ( value + u < value ) panic("UINTN operator + (UINT16 u) -> overflow"); return value + u; };
//	UINTN operator + (UINT32 u) const { if ( value + u < value ) panic("UINTN operator + (UINT32 u) -> overflow"); return value + u; };
	UINTN operator + (UINT64 u) const { if ( m_value + u < m_value ) panic("UINTN operator + (UINT64 u) -> overflow"); return m_value + u; };

	UINTN operator - (UINT64 u) const { if ( m_value - u > m_value ) panic("UINTN operator - (UINT64 u) -> overflow"); return m_value - u; };

	UINTN operator > (UINT64 u) const { return m_value > u; };
	UINTN operator >= (UINT64 u) const { return m_value >= u; };
	UINTN operator < (UINT64 u) const { return m_value < u; };
	UINTN operator <= (UINT64 u) const { return m_value <= u; };

	
	explicit operator UINT8() const { if ( m_value > MAX_UINT8 ) panic("operator UINT8() const -> value too big to be casted as UINT8"); return (UINT8)m_value; };
	explicit operator UINT16() const { if ( m_value > MAX_UINT16 ) panic("operator UINT16() const -> value too big to be casted as UINT16"); return (UINT16)m_value; };
	explicit operator UINT32() const { if ( m_value > MAX_UINT32 ) panic("operator UINT32() const -> value too big to be casted as UINT32"); return (UINT32)m_value; };
	explicit operator UINT64() const { return m_value; };
	
	explicit operator INT8() const { if ( m_value > MAX_INT8 ) panic("operator INT8() const -> value too big to be casted as INT8"); return (INT8)m_value; };
	explicit operator INT16() const { if ( m_value > MAX_INT16 ) panic("operator INT16() const -> value too big to be casted as INT16"); return (INT16)m_value; };
	explicit operator INT32() const { if ( m_value > MAX_INT32 ) panic("operator INT32() const -> value too big to be casted as INT32"); return (INT32)m_value; };
	explicit operator INT64() const { if ( m_value > MAX_INT64 ) panic("operator INT64() const -> value too big to be casted as INT64"); return (INT64)m_value; };

};

#endif
