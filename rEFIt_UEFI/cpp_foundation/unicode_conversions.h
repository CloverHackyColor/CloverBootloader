/*
 *
 * Created by jief the 24 Feb 2020.
 * Copyright (c) 2020 Jief
 * All rights reserved.
 *
 */

#ifndef __unicode_conversions_h__
#define __unicode_conversions_h__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <wchar.h>
#include <string.h>

#ifndef __cplusplus
#include <stdint.h>
//typedef uint16_t wchar_t;
typedef uint32_t char32_t;
typedef uint16_t char16_t;
#endif

//#ifndef __WCHAR_MAX__
//#define __WCHAR_MAX__ WCHAR_MAX
//#endif

#if __WCHAR_MAX__ <= 0xFFFFu
    #define wchar_cast char16_t
#else
    #define wchar_cast char32_t
#endif

#ifndef wchar_cast
#error wchar_cast
#endif


/*
 * len means nb utf32 char
 * size means nb of underlying native type (nb of char16_t, nb of char32_t, etc.
 */



/******   utility   *****/

/*
 * Size of an UTF32 char when represented in UTF8
 * Return value : size
 */
size_t utf8_size_of_utf32_char(char32_t c);

/*
 * Increment size and return a pointer to the next char
 * Return value : pointer to the end of string or at the error
 */
const char32_t* utf8_size_of_utf32_char_ptr(const char32_t *s, size_t* size);

/*
 * Store an utf32 char in dst, if there is enough room (dst_max_size is >= size of converted utf32 char)
 * If there is enough room, dst_max_size is decrement and dst is increment and returned
 * If there isn't enough room, dst_max_size is set to 0 and dst is returned
 */
char* get_utf8_from_char32(char* dst, size_t* dst_max_size, char32_t utf32_char);

/*
 * Store an utf32 char in dst, if there is enough room (dst_max_size is >= size of converted utf32 char)
 * If there is enough room, dst_max_size is decrement and dst is increment and returned
 * If there isn't enough room, dst_max_size is set to 0 and dst is returned
 */
char16_t* get_utf16_from_char32(char16_t* dst, size_t* dst_max_size, char32_t utf32_char);

/*
 * Store an utf32 char in dst, if there is enough room (dst_max_size is >= size of converted utf32 char)
 * If there is enough room, dst_max_size is decrement and dst is increment and returned
 * If there isn't enough room, dst_max_size is set to 0 and dst is returned
 */
char32_t* get_utf32_from_char32(char32_t* dst, size_t* dst_max_size, char32_t utf32_char);

/*
 * char32 will be set to 0 at the end of string or at error
 * Return value : pointer to the end of string or at the error
 */
const char* get_char32_from_utf8_string(const char* s, char32_t* char32);

/*
 * get nth char32 of an utf8 string
 * Return value : pointer to the end of string or at the error
 */
char32_t get_char32_from_utf8_string_at_pos(const char* s, size_t n);


/******   utf8 - utf32   *****/

/*
 * Size in bytes of an utf32 string if it were converted to utf8
 * Return value : size
 */
size_t utf8_size_of_utf32_string(const char32_t* s);

/*
 * Size in bytes of an utf32 string of at most len char if it were converted to utf8
 * Return value : size
 */
size_t utf8_size_of_utf32_string_len(const char32_t* s, size_t len);

/*
* Size in bytes of an utf8 string of at most len char if it were converted to utf32
* Return value : size
*/
size_t utf32_size_of_utf8_string(const char* s);

/*
* Size in bytes of an utf8 string of at most len char if it were converted to utf8
* Return value : size, <= len
*/
size_t utf32_size_of_utf8_string_len(const char* s, size_t len);

/*
 * Convert s to dst. Do not add null terminator.
 * Return the number of utf32 char written
 */
size_t utf32_stringnn_from_utf8_string(char32_t* dst, size_t dst_max_size, const char* s);
size_t utf32_string_from_utf8_string(char32_t* dst, size_t dst_max_size, const char* s);
size_t utf32_string_from_utf8_string_len(char32_t* dst, size_t dst_max_size, const char* s, size_t len);
size_t utf32_string_from_utf8_string_size(char32_t* dst, size_t dst_max_size, const char* s, size_t size);
/*
* Convert s to dst. Do not add null terminator.
* Return the number of utf8 char written
*/
size_t utf8_stringnn_from_utf32_string(char* dst, size_t dst_max_size, const char32_t *s);
size_t utf8_string_from_utf32_string(char* dst, size_t dst_max_size, const char32_t *s);
size_t utf8_string_from_utf32_string_len(char* dst, size_t dst_max_size, const char32_t *s, size_t len);
size_t utf8_string_from_utf32_string_size(char* dst, size_t dst_max_size, const char32_t *s, size_t size);

char32_t get_char32_from_utf8_string_at_pos(const char* s, size_t pos);


/******   utf8 - utf16   *****/

/*
 * Increment size
 * Return value : pointer to the end of string or at the error
 */
const char16_t* utf8_size_of_utf16_char_ptr(const char16_t *s, size_t* size);

size_t utf8_size_of_utf16_string(const char16_t* s);
size_t utf8_size_of_utf16_string_len(const char16_t* s, size_t len);

size_t utf16_size_of_utf8_string(const char* s);
size_t utf16_size_of_utf8_string_len(const char* s, size_t len);

size_t utf8_stringnn_from_utf16_string(char* dst, size_t dst_max_size, const char16_t *s);
size_t utf8_string_from_utf16_string(char* dst, size_t dst_max_size, const char16_t *s);
size_t utf8_string_from_utf16_string_len(char* dst, size_t dst_max_size, const char16_t *s, size_t len);
size_t utf8_string_from_utf16_string_size(char* dst, size_t dst_max_size, const char16_t *s, size_t size);
size_t utf16_stringnn_from_utf8_string(char16_t* dst, size_t dst_max_size, const char* s);
size_t utf16_string_from_utf8_string(char16_t* dst, size_t dst_max_size, const char* s);
size_t utf16_string_from_utf8_string_len(char16_t* dst, size_t dst_max_size, const char* s, size_t len);
size_t utf16_string_from_utf8_string_size(char16_t* dst, size_t dst_max_size, const char* s, size_t size);


/******   utf16 - utf32   *****/

size_t utf16_size_of_utf32_char(char32_t c);
void get_char16_from_char32(char32_t char32, char16_t* char16_1, char16_t* char16_2);
char32_t get_char32_from_char16(char16_t char16_1, char16_t char16_2);
const char16_t* get_char32_from_utf16_string(const char16_t* s, char32_t* char32);


size_t utf16_size_of_utf32_string(const char32_t *s);
size_t utf16_size_of_utf32_string_len(const char32_t *s, size_t len);
size_t utf32_size_of_utf16_string(const char16_t *s);
size_t utf32_size_of_utf16_string_len(const char16_t *s, size_t len);

size_t utf16_stringnn_from_utf32_string(char16_t* dst, size_t dst_max_size, const char32_t *s);
size_t utf16_string_from_utf32_string(char16_t* dst, size_t dst_max_size, const char32_t *s);
size_t utf16_string_from_utf32_string_len(char16_t* dst, size_t dst_max_size, const char32_t *s, size_t len);
size_t utf16_string_from_utf32_string_size(char16_t* dst, size_t dst_max_size, const char32_t *s, size_t size);
size_t utf32_stringnn_from_utf16_string(char32_t* dst, size_t dst_max_size, const char16_t *s);
size_t utf32_string_from_utf16_string(char32_t* dst, size_t dst_max_size, const char16_t *s);
size_t utf32_string_from_utf16_string_len(char32_t* dst, size_t dst_max_size, const char16_t *s, size_t len);
size_t utf32_string_from_utf16_string_size(char32_t* dst, size_t dst_max_size, const char16_t *s, size_t size);


/*
 * get nth char32 of an utf16 string
 * Return value : pointer to the end of string or at the error
 */
char32_t get_char32_from_utf16_string_at_pos(const char16_t* s, size_t pos);


/******   utf8 - wchar_t   *****/

size_t utf8_size_of_wchar_string(const wchar_t* s);
size_t utf8_size_of_wchar_string_len(const wchar_t* s, size_t len);
size_t wchar_size_of_utf8_string(const char* s);
size_t wchar_size_of_utf8_string_len(const char* s, size_t len);

size_t utf8_stringnn_from_wchar_string(char* dst, size_t dst_max_size, const wchar_t* s);
size_t utf8_string_from_wchar_string(char* dst, size_t dst_max_size, const wchar_t* s);
size_t utf8_string_from_wchar_string_len(char* dst, size_t dst_max_size, const wchar_t* s, size_t len);
size_t utf8_string_from_wchar_string_size(char* dst, size_t dst_max_size, const wchar_t* s, size_t size);
size_t wchar_stringnn_from_utf8_string(wchar_t* dst, size_t dst_max_size, const char* s);
size_t wchar_string_from_utf8_string(wchar_t* dst, size_t dst_max_size, const char* s);
size_t wchar_string_from_utf8_string_len(wchar_t* dst, size_t dst_max_size, const char* s, size_t len);
size_t wchar_string_from_utf8_string_size(wchar_t* dst, size_t dst_max_size, const char* s, size_t size);


/******   utf16 - wchar_t   *****/
size_t utf16_size_of_wchar_string(const wchar_t* s);
size_t utf16_size_of_wchar_string_len(const wchar_t* s, size_t len);
size_t wchar_size_of_utf16_string(const char16_t *s);
size_t wchar_size_of_utf16_string_len(const char16_t *s, size_t len);

size_t utf16_stringnn_from_wchar_string(char16_t* dst, size_t dst_max_size, const wchar_t* s);
size_t utf16_string_from_wchar_string(char16_t* dst, size_t dst_max_size, const wchar_t* s);
size_t utf16_string_from_wchar_string_len(char16_t* dst, size_t dst_max_size, const wchar_t* s, size_t len);
size_t utf16_string_from_wchar_string_size(char16_t* dst, size_t dst_max_size, const wchar_t* s, size_t size);
size_t wchar_stringnn_from_utf16_string(wchar_t* dst, size_t dst_max_size, const char16_t* s);
size_t wchar_string_from_utf16_string(wchar_t* dst, size_t dst_max_size, const char16_t* s);
size_t wchar_string_from_utf16_string_len(wchar_t* dst, size_t dst_max_size, const char16_t* s, size_t len);
size_t wchar_string_from_utf16_string_size(wchar_t* dst, size_t dst_max_size, const char16_t* s, size_t size);


/******   utf32 - wchar_t   *****/
size_t utf32_size_of_wchar_string(const wchar_t* s);
size_t utf32_size_of_wchar_string_len(const wchar_t* s, size_t len);
size_t wchar_size_of_utf32_string(const char32_t *s);
size_t wchar_size_of_utf32_string_len(const char32_t *s, size_t len);

size_t utf32_stringnn_from_wchar_string(char32_t* dst, size_t dst_max_size, const wchar_t* s);
size_t utf32_string_from_wchar_string(char32_t* dst, size_t dst_max_size, const wchar_t* s);
size_t utf32_string_from_wchar_string_len(char32_t* dst, size_t dst_max_size, const wchar_t* s, size_t len);
size_t utf32_string_from_wchar_string_size(char32_t* dst, size_t dst_max_size, const wchar_t* s, size_t size);
size_t wchar_stringnn_from_utf32_string(wchar_t* dst, size_t dst_max_size, const char32_t* s);
size_t wchar_string_from_utf32_string(wchar_t* dst, size_t dst_max_size, const char32_t* s);
size_t wchar_string_from_utf32_string_len(wchar_t* dst, size_t dst_max_size, const char32_t* s, size_t len);
size_t wchar_string_from_utf32_string_size(wchar_t* dst, size_t dst_max_size, const char32_t* s, size_t size);

char32_t get_char32_from_wchar_string_at_pos(const char16_t* s, size_t pos);


/******   no conversion   *****/

size_t utf8_size_of_utf8_string(const char* s);
size_t utf8_size_of_utf8_string_len(const char* s, size_t len);
size_t utf16_size_of_utf16_string(const char16_t* s);
size_t utf16_size_of_utf16_string_len(const char16_t* s, size_t len);
size_t utf32_size_of_utf32_string(const char32_t* s);
size_t utf32_size_of_utf32_string_len(const char32_t* s, size_t len);
size_t wchar_size_of_wchar_string(const wchar_t* s);
size_t wchar_size_of_wchar_string_len(const wchar_t* s, size_t len);

size_t utf8_stringnn_from_utf8_string(char* dst, size_t dst_max_size, const char *s);
size_t utf8_string_from_utf8_string(char* dst, size_t dst_max_size, const char *s);
size_t utf8_string_from_utf8_string_len(char* dst, size_t dst_max_size, const char *s, size_t len);
size_t utf8_string_from_utf8_string_size(char* dst, size_t dst_max_size, const char *s, size_t size);
size_t utf16_stringnn_from_utf16_string(char16_t* dst, size_t dst_max_size, const char16_t *s);
size_t utf16_string_from_utf16_string(char16_t* dst, size_t dst_max_size, const char16_t *s);
size_t utf16_string_from_utf16_string_len(char16_t* dst, size_t dst_max_size, const char16_t *s, size_t len);
size_t utf16_string_from_utf16_string_size(char16_t* dst, size_t dst_max_size, const char16_t *s, size_t size);
size_t utf32_stringnn_from_utf32_string(char32_t* dst, size_t dst_max_size, const char32_t *s);
size_t utf32_string_from_utf32_string(char32_t* dst, size_t dst_max_size, const char32_t *s);
size_t utf32_string_from_utf32_string_len(char32_t* dst, size_t dst_max_size, const char32_t *s, size_t len);
size_t utf32_string_from_utf32_string_size(char32_t* dst, size_t dst_max_size, const char32_t *s, size_t size);
size_t wchar_stringnn_from_wchar_string(wchar_t* dst, size_t dst_max_size, const wchar_t *s);
size_t wchar_string_from_wchar_string(wchar_t* dst, size_t dst_max_size, const wchar_t *s);
size_t wchar_string_from_wchar_string_len(wchar_t* dst, size_t dst_max_size, const wchar_t *s, size_t len);
size_t wchar_string_from_wchar_string_size(wchar_t* dst, size_t dst_max_size, const wchar_t *s, size_t size);

char32_t get_char32_from_utf32_string_at_pos(const char32_t* s, size_t pos);

/******   convenience   *****/

//inline size_t length_of_utf8_string(const char* s);
//inline size_t length_of_utf16_string(const char16_t* s);
//inline size_t length_of_utf32_string(const char32_t* s); // UTF32 length == size
//inline size_t length_of_wchar_string(const wchar_t* s);


#ifdef __cplusplus
}
#endif

/* ==================================  C++ Convienience  ============================================= */

#ifdef __cplusplus

inline char*     get_char32_from_string(char* s,     char32_t* char32) { return (char*)get_char32_from_utf8_string(s, char32); }
inline char16_t* get_char32_from_string(char16_t* s, char32_t* char32) { return (char16_t*)get_char32_from_utf16_string(s, char32); }
inline char32_t* get_char32_from_string(char32_t* s, char32_t* char32) { *char32 = *s; if ( !*s ) return s; return s+1; }
inline wchar_t*  get_char32_from_string(wchar_t* s,  char32_t* char32) { return (wchar_t*)get_char32_from_string((wchar_cast*)s, char32); }

inline const char* get_char32_from_string(const char* s, char32_t* char32) { return get_char32_from_utf8_string(s, char32); }
inline const char16_t* get_char32_from_string(const char16_t* s, char32_t* char32) { return get_char32_from_utf16_string(s, char32); }
inline const char32_t* get_char32_from_string(const char32_t* s, char32_t* char32) { *char32 = *s; if ( !*s ) return s; return s+1; }
inline const wchar_t* get_char32_from_string(const wchar_t* s, char32_t* char32) { return (wchar_t*)get_char32_from_string((wchar_cast*)s, char32); }

inline char32_t get_char32_from_utf_string_at_pos(const char* s, size_t pos) { return get_char32_from_utf8_string_at_pos(s, pos); }
inline char32_t get_char32_from_utf_string_at_pos(const char16_t* s, size_t pos) { return get_char32_from_utf16_string_at_pos(s, pos); }
inline char32_t get_char32_from_utf_string_at_pos(const char32_t* s, size_t pos) { return get_char32_from_utf32_string_at_pos(s, pos); }
inline char32_t get_char32_from_utf_string_at_pos(const wchar_t* s, size_t pos) { return get_char32_from_utf_string_at_pos((wchar_cast*)s, pos); }



inline char* store_utf_from_char32(char* dst, size_t* dst_max_size, char32_t utf32_char) { return get_utf8_from_char32(dst, dst_max_size, utf32_char); }
inline char16_t* store_utf_from_char32(char16_t* dst, size_t* dst_max_size, char32_t utf32_char) { return get_utf16_from_char32(dst, dst_max_size, utf32_char);  }
inline char32_t* store_utf_from_char32(char32_t* dst, size_t* dst_max_size, char32_t utf32_char) { return get_utf32_from_char32(dst, dst_max_size, utf32_char);  }
inline wchar_t* store_utf_from_char32(wchar_t* dst, size_t* dst_max_size, char32_t utf32_char) { return (wchar_t*)store_utf_from_char32((wchar_cast*)dst, dst_max_size, utf32_char); }


/* Returns amount of char32_t if the string were converted to UTF32 */
inline size_t length_of_utf_string(const char* s) { return utf32_size_of_utf8_string(s); };
inline size_t length_of_utf_string(const char16_t* s) { return utf32_size_of_utf16_string(s); };
inline size_t length_of_utf_string(const char32_t* s) { return utf32_size_of_utf32_string(s); };
inline size_t length_of_utf_string(const wchar_t* s) { return length_of_utf_string((wchar_cast*)s); };



/* Returns amount of utf chars. For example, an UTF16 surrogate pair will count for 2 */
inline size_t size_of_utf_string(const char* s) { return utf8_size_of_utf8_string(s); }
inline size_t size_of_utf_string(const char16_t* s) { return utf16_size_of_utf16_string(s); }
inline size_t size_of_utf_string(const char32_t* s) { return utf32_size_of_utf32_string(s); } // for UTF32 size and length are equal
inline size_t size_of_utf_string(const wchar_t* s) { return size_of_utf_string((wchar_cast*)s); }

/* Returns amount of utf chars. Type of utf chars are determined by the first parameter. */
inline size_t utf_size_of_utf_string(const char* dummy, const char* s) { (void)dummy; return utf8_size_of_utf8_string(s); }
inline size_t utf_size_of_utf_string(const char16_t* dummy, const char* s) { (void)dummy; return utf16_size_of_utf8_string(s); }
inline size_t utf_size_of_utf_string(const char32_t* dummy, const char* s) { (void)dummy; return utf32_size_of_utf8_string(s); }
inline size_t utf_size_of_utf_string(const wchar_t* dummy, const char* s) { (void)dummy; return utf_size_of_utf_string((wchar_cast*)dummy, s); }

inline size_t utf_size_of_utf_string(const char*, const char16_t* s) { return utf8_size_of_utf16_string(s); }
inline size_t utf_size_of_utf_string(const char16_t*, const char16_t* s) { return utf16_size_of_utf16_string(s); }
inline size_t utf_size_of_utf_string(const char32_t*, const char16_t* s) { return utf32_size_of_utf16_string(s); }
inline size_t utf_size_of_utf_string(const wchar_t* t, const char16_t* s) { return utf_size_of_utf_string((wchar_cast*)t, s); }

inline size_t utf_size_of_utf_string(const char*, const char32_t* s) { return utf8_size_of_utf32_string(s); }
inline size_t utf_size_of_utf_string(const char16_t*, const char32_t* s) { return utf16_size_of_utf32_string(s); }
inline size_t utf_size_of_utf_string(const char32_t*, const char32_t* s) { return utf32_size_of_utf32_string(s); }
inline size_t utf_size_of_utf_string(const wchar_t* t, const char32_t* s) { return utf_size_of_utf_string((wchar_cast*)t, s); }

inline size_t utf_size_of_utf_string(const char* t, const wchar_t* s) { return utf_size_of_utf_string(t, (wchar_cast*)s); }
inline size_t utf_size_of_utf_string(const char16_t* t, const wchar_t* s) { return utf_size_of_utf_string(t, (wchar_cast*)s); }
inline size_t utf_size_of_utf_string(const char32_t* t, const wchar_t* s) { return utf_size_of_utf_string(t, (wchar_cast*)s); }
inline size_t utf_size_of_utf_string(const wchar_t* t, const wchar_t* s) { return utf_size_of_utf_string(t, (wchar_cast*)s); }


/* Returns amount of utf chars needed to represent the first [len] utf32 chars. */
inline size_t size_of_utf_string_len(const char* s, size_t len) { return utf8_size_of_utf8_string_len(s, len); }
inline size_t size_of_utf_string_len(const char16_t* s, size_t len) { return utf16_size_of_utf16_string_len(s, len); }
inline size_t size_of_utf_string_len(const char32_t* s, size_t len) { return utf32_size_of_utf32_string_len(s, len); } // for UTF32 size and length are equal
inline size_t size_of_utf_string_len(const wchar_t* s, size_t len) { return size_of_utf_string_len((wchar_cast*)s, len); }

/* Returns amount of utf chars needed to represent the first [len] utf32 chars. Type of utf chars are determined by the first parameter. */
inline size_t utf_size_of_utf_string_len(const char*, const char* s, size_t len) { return utf8_size_of_utf8_string_len(s, len); }
inline size_t utf_size_of_utf_string_len(const char16_t*, const char* s, size_t len) { return utf16_size_of_utf8_string_len(s, len); }
inline size_t utf_size_of_utf_string_len(const char32_t*, const char* s, size_t len) { return utf32_size_of_utf8_string_len(s, len); }
inline size_t utf_size_of_utf_string_len(const wchar_t* t, const char* s, size_t len) { return utf_size_of_utf_string_len((wchar_cast*)t, s, len); }

inline size_t utf_size_of_utf_string_len(const char*, const char16_t* s, size_t len) { return utf8_size_of_utf16_string_len(s, len); }
inline size_t utf_size_of_utf_string_len(const char16_t*, const char16_t* s, size_t len) { return utf16_size_of_utf16_string_len(s, len); }
inline size_t utf_size_of_utf_string_len(const char32_t*, const char16_t* s, size_t len) { return utf32_size_of_utf16_string_len(s, len); }
inline size_t utf_size_of_utf_string_len(const wchar_t* t, const char16_t* s, size_t len) { return utf_size_of_utf_string_len((wchar_cast*)t, s, len); }

inline size_t utf_size_of_utf_string_len(const char*, const char32_t* s, size_t len) { return utf8_size_of_utf32_string_len(s, len); }
inline size_t utf_size_of_utf_string_len(const char16_t*, const char32_t* s, size_t len) { return utf16_size_of_utf32_string_len(s, len); }
inline size_t utf_size_of_utf_string_len(const char32_t*, const char32_t* s, size_t len) { return utf32_size_of_utf32_string_len(s, len); }
inline size_t utf_size_of_utf_string_len(const wchar_t* t, const char32_t* s, size_t len) { return utf_size_of_utf_string_len((wchar_cast*)t, s, len); }

inline size_t utf_size_of_utf_string_len(const char* t, const wchar_t* s, size_t len) { return utf_size_of_utf_string_len(t, (wchar_cast*)s, len); }
inline size_t utf_size_of_utf_string_len(const char16_t* t, const wchar_t* s, size_t len) { return utf_size_of_utf_string_len(t, (wchar_cast*)s, len); }
inline size_t utf_size_of_utf_string_len(const char32_t* t, const wchar_t* s, size_t len) { return utf_size_of_utf_string_len(t, (wchar_cast*)s, len); }
inline size_t utf_size_of_utf_string_len(const wchar_t* t, const wchar_t* s, size_t len) { return utf_size_of_utf_string_len((wchar_cast*)t, (wchar_cast*)s, len); }

//
//
///* Returns amount of utf chars needed to represent the first [size] native chars chars. */
//inline size_t size_of_utf_string_size(const char* s, size_t size) { return utf8_size_of_utf8_string_size(s, size); }
//inline size_t size_of_utf_string_size(const char16_t* s, size_t size) { return utf16_size_of_utf16_string_size(s, size); }
//inline size_t size_of_utf_string_size(const char32_t* s, size_t size) { return utf32_size_of_utf32_string_size(s, size); } // for UTF32 size and sizegth are equal
//inline size_t size_of_utf_string_size(const wchar_t* s, size_t size) { return size_of_utf_string_size((wchar_cast*)s, size); }
//
///* Returns amount of utf chars needed to represent the first [size] native chars. Type of utf chars are determined by the first parameter. */
//inline size_t utf_size_of_utf_string_size(const char*, const char* s, size_t size) { return utf8_size_of_utf8_string_size(s, size); }
//inline size_t utf_size_of_utf_string_size(const char16_t*, const char* s, size_t size) { return utf16_size_of_utf8_string_size(s, size); }
//inline size_t utf_size_of_utf_string_size(const char32_t*, const char* s, size_t size) { return utf32_size_of_utf8_string_size(s, size); }
//inline size_t utf_size_of_utf_string_size(const wchar_t* t, const char* s, size_t size) { return utf_size_of_utf_string_size((wchar_cast*)t, s, size); }
//
//inline size_t utf_size_of_utf_string_size(const char*, const char16_t* s, size_t size) { return utf8_size_of_utf16_string_size(s, size); }
//inline size_t utf_size_of_utf_string_size(const char16_t*, const char16_t* s, size_t size) { return utf16_size_of_utf16_string_size(s, size); }
//inline size_t utf_size_of_utf_string_size(const char32_t*, const char16_t* s, size_t size) { return utf32_size_of_utf16_string_size(s, size); }
//inline size_t utf_size_of_utf_string_size(const wchar_t* t, const char16_t* s, size_t size) { return utf_size_of_utf_string_size((wchar_cast*)t, s, size); }
//
//inline size_t utf_size_of_utf_string_size(const char*, const char32_t* s, size_t size) { return utf8_size_of_utf32_string_size(s, size); }
//inline size_t utf_size_of_utf_string_size(const char16_t*, const char32_t* s, size_t size) { return utf16_size_of_utf32_string_size(s, size); }
//inline size_t utf_size_of_utf_string_size(const char32_t*, const char32_t* s, size_t size) { return utf32_size_of_utf32_string_size(s, size); }
//inline size_t utf_size_of_utf_string_size(const wchar_t* t, const char32_t* s, size_t size) { return utf_size_of_utf_string_size((wchar_cast*)t, s, size); }
//
//inline size_t utf_size_of_utf_string_size(const char* t, const wchar_t* s, size_t size) { return utf_size_of_utf_string_size(t, (wchar_cast*)s, size); }
//inline size_t utf_size_of_utf_string_size(const char16_t* t, const wchar_t* s, size_t size) { return utf_size_of_utf_string_size(t, (wchar_cast*)s, size); }
//inline size_t utf_size_of_utf_string_size(const char32_t* t, const wchar_t* s, size_t size) { return utf_size_of_utf_string_size(t, (wchar_cast*)s, size); }
//inline size_t utf_size_of_utf_string_size(const wchar_t* t, const wchar_t* s, size_t size) { return utf_size_of_utf_string_size((wchar_cast*)t, (wchar_cast*)s, size); }
//
//



/*
 * Conversions
 */

inline size_t utf_string_from_utf_string(char* dst, size_t dst_max_size, const char* s) { return utf8_string_from_utf8_string(dst, dst_max_size, s); }
inline size_t utf_string_from_utf_string(char16_t* dst, size_t dst_max_size, const char* s) { return utf16_string_from_utf8_string(dst, dst_max_size, s); }
inline size_t utf_string_from_utf_string(char32_t* dst, size_t dst_max_size, const char* s) { return utf32_string_from_utf8_string(dst, dst_max_size, s); }
inline size_t utf_string_from_utf_string(wchar_t* dst, size_t dst_max_size, const char* s) { return utf_string_from_utf_string((wchar_cast*)dst, dst_max_size, s); }

inline size_t utf_string_from_utf_string(char* dst, size_t dst_max_size, const char16_t *s) { return utf8_string_from_utf16_string(dst, dst_max_size, s); }
inline size_t utf_string_from_utf_string(char16_t* dst, size_t dst_max_size, const char16_t *s) { return utf16_string_from_utf16_string(dst, dst_max_size, s); }
inline size_t utf_string_from_utf_string(char32_t* dst, size_t dst_max_size, const char16_t *s) { return utf32_string_from_utf16_string(dst, dst_max_size, s); }
inline size_t utf_string_from_utf_string(wchar_t* dst, size_t dst_max_size, const char16_t *s) { return utf_string_from_utf_string((wchar_cast*)dst, dst_max_size, s); }

inline size_t utf_string_from_utf_string(char* dst, size_t dst_max_size, const char32_t *s) { return utf8_string_from_utf32_string(dst, dst_max_size, s); }
inline size_t utf_string_from_utf_string(char16_t* dst, size_t dst_max_size, const char32_t *s) { return utf16_string_from_utf32_string(dst, dst_max_size, s); }
inline size_t utf_string_from_utf_string(char32_t* dst, size_t dst_max_size, const char32_t *s) { return utf32_string_from_utf32_string(dst, dst_max_size, s); }
inline size_t utf_string_from_utf_string(wchar_t* dst, size_t dst_max_size, const char32_t *s) { return utf_string_from_utf_string((wchar_cast*)dst, dst_max_size, s); }

inline size_t utf_string_from_utf_string(char* dst, size_t dst_max_size, const wchar_t *s) { return utf_string_from_utf_string(dst, dst_max_size, (wchar_cast*)s); }
inline size_t utf_string_from_utf_string(char16_t* dst, size_t dst_max_size, const wchar_t *s) { return utf_string_from_utf_string(dst, dst_max_size, (wchar_cast*)s); }
inline size_t utf_string_from_utf_string(char32_t* dst, size_t dst_max_size, const wchar_t *s) { return utf_string_from_utf_string(dst, dst_max_size, (wchar_cast*)s); }
inline size_t utf_string_from_utf_string(wchar_t* dst, size_t dst_max_size, const wchar_t *s) { return utf_string_from_utf_string(dst, dst_max_size, (wchar_cast*)s); }


// utf_stringnn... do not add NULL at the end after conversion
inline size_t utf_stringnn_from_utf_string(char* dst, size_t dst_max_size, const char* s) { return utf8_stringnn_from_utf8_string(dst, dst_max_size, s); }
inline size_t utf_stringnn_from_utf_string(char16_t* dst, size_t dst_max_size, const char* s) { return utf16_stringnn_from_utf8_string(dst, dst_max_size, s); }
inline size_t utf_stringnn_from_utf_string(char32_t* dst, size_t dst_max_size, const char* s) { return utf32_stringnn_from_utf8_string(dst, dst_max_size, s); }
inline size_t utf_stringnn_from_utf_string(wchar_t* dst, size_t dst_max_size, const char* s) { return utf_stringnn_from_utf_string((wchar_cast*)dst, dst_max_size, s); }

inline size_t utf_stringnn_from_utf_string(char* dst, size_t dst_max_size, const char16_t *s) { return utf8_stringnn_from_utf16_string(dst, dst_max_size, s); }
inline size_t utf_stringnn_from_utf_string(char16_t* dst, size_t dst_max_size, const char16_t *s) { return utf16_stringnn_from_utf16_string(dst, dst_max_size, s); }
inline size_t utf_stringnn_from_utf_string(char32_t* dst, size_t dst_max_size, const char16_t *s) { return utf32_stringnn_from_utf16_string(dst, dst_max_size, s); }
inline size_t utf_stringnn_from_utf_string(wchar_t* dst, size_t dst_max_size, const char16_t *s) { return utf_stringnn_from_utf_string((wchar_cast*)dst, dst_max_size, s); }

inline size_t utf_stringnn_from_utf_string(char* dst, size_t dst_max_size, const char32_t *s) { return utf8_stringnn_from_utf32_string(dst, dst_max_size, s); }
inline size_t utf_stringnn_from_utf_string(char16_t* dst, size_t dst_max_size, const char32_t *s) { return utf16_stringnn_from_utf32_string(dst, dst_max_size, s); }
inline size_t utf_stringnn_from_utf_string(char32_t* dst, size_t dst_max_size, const char32_t *s) { return utf32_stringnn_from_utf32_string(dst, dst_max_size, s); }
inline size_t utf_stringnn_from_utf_string(wchar_t* dst, size_t dst_max_size, const char32_t *s) { return utf_stringnn_from_utf_string((wchar_cast*)dst, dst_max_size, s); }

inline size_t utf_stringnn_from_utf_string(char* dst, size_t dst_max_size, const wchar_t *s) { return utf_stringnn_from_utf_string(dst, dst_max_size, (wchar_cast*)s); }
inline size_t utf_stringnn_from_utf_string(char16_t* dst, size_t dst_max_size, const wchar_t *s) { return utf_stringnn_from_utf_string(dst, dst_max_size, (wchar_cast*)s); }
inline size_t utf_stringnn_from_utf_string(char32_t* dst, size_t dst_max_size, const wchar_t *s) { return utf_stringnn_from_utf_string(dst, dst_max_size, (wchar_cast*)s); }
inline size_t utf_stringnn_from_utf_string(wchar_t* dst, size_t dst_max_size, const wchar_t *s) { return utf_stringnn_from_utf_string(dst, dst_max_size, (wchar_cast*)s); }






inline size_t utf_string_from_utf_string_len(char* dst, size_t dst_max_size, const char* s, size_t len) { return utf8_string_from_utf8_string_len(dst, dst_max_size, s, len); }
inline size_t utf_string_from_utf_string_len(char16_t* dst, size_t dst_max_size, const char* s, size_t len) { return utf16_string_from_utf8_string_len(dst, dst_max_size, s, len); }
inline size_t utf_string_from_utf_string_len(char32_t* dst, size_t dst_max_size, const char* s, size_t len) { return utf32_string_from_utf8_string_len(dst, dst_max_size, s, len); }
inline size_t utf_string_from_utf_string_len(wchar_t* dst, size_t dst_max_size, const char* s, size_t len) { return utf_string_from_utf_string_len((wchar_cast*)dst, dst_max_size, s, len); }

inline size_t utf_string_from_utf_string_len(char* dst, size_t dst_max_size, const char16_t * s, size_t len) { return utf8_string_from_utf16_string_len(dst, dst_max_size, s, len); }
inline size_t utf_string_from_utf_string_len(char16_t* dst, size_t dst_max_size, const char16_t * s, size_t len) { return utf16_string_from_utf16_string_len(dst, dst_max_size, s, len); }
inline size_t utf_string_from_utf_string_len(char32_t* dst, size_t dst_max_size, const char16_t * s, size_t len) { return utf32_string_from_utf16_string_len(dst, dst_max_size, s, len); }
inline size_t utf_string_from_utf_string_len(wchar_t* dst, size_t dst_max_size, const char16_t * s, size_t len) { return utf_string_from_utf_string_len((wchar_cast*)dst, dst_max_size, s, len); }

inline size_t utf_string_from_utf_string_len(char* dst, size_t dst_max_size, const char32_t * s, size_t len) { return utf8_string_from_utf32_string_len(dst, dst_max_size, s, len); }
inline size_t utf_string_from_utf_string_len(char16_t* dst, size_t dst_max_size, const char32_t * s, size_t len) { return utf16_string_from_utf32_string_len(dst, dst_max_size, s, len); }
inline size_t utf_string_from_utf_string_len(char32_t* dst, size_t dst_max_size, const char32_t * s, size_t len) { return utf32_string_from_utf32_string_len(dst, dst_max_size, s, len); }
inline size_t utf_string_from_utf_string_len(wchar_t* dst, size_t dst_max_size, const char32_t * s, size_t len) { return utf_string_from_utf_string_len((wchar_cast*)dst, dst_max_size, s, len); }

inline size_t utf_string_from_utf_string_len(char* dst, size_t dst_max_size, const wchar_t * s, size_t len) { return utf_string_from_utf_string_len(dst, dst_max_size, (wchar_cast*)s, len); }
inline size_t utf_string_from_utf_string_len(char16_t* dst, size_t dst_max_size, const wchar_t * s, size_t len) { return utf_string_from_utf_string_len(dst, dst_max_size, (wchar_cast*)s, len); }
inline size_t utf_string_from_utf_string_len(char32_t* dst, size_t dst_max_size, const wchar_t * s, size_t len) { return utf_string_from_utf_string_len(dst, dst_max_size, (wchar_cast*)s, len); }
inline size_t utf_string_from_utf_string_len(wchar_t* dst, size_t dst_max_size, const wchar_t * s, size_t len) { return utf_string_from_utf_string_len(dst, dst_max_size, (wchar_cast*)s, len); }



inline size_t utf_string_from_utf_string_size(char* dst, size_t dst_max_size, const char* s, size_t size) { return utf8_string_from_utf8_string_size(dst, dst_max_size, s, size); }
inline size_t utf_string_from_utf_string_size(char16_t* dst, size_t dst_max_size, const char* s, size_t size) { return utf16_string_from_utf8_string_size(dst, dst_max_size, s, size); }
inline size_t utf_string_from_utf_string_size(char32_t* dst, size_t dst_max_size, const char* s, size_t size) { return utf32_string_from_utf8_string_size(dst, dst_max_size, s, size); }
inline size_t utf_string_from_utf_string_size(wchar_t* dst, size_t dst_max_size, const char* s, size_t size) { return utf_string_from_utf_string_size((wchar_cast*)dst, dst_max_size, s, size); }

inline size_t utf_string_from_utf_string_size(char* dst, size_t dst_max_size, const char16_t * s, size_t size) { return utf8_string_from_utf16_string_size(dst, dst_max_size, s, size); }
inline size_t utf_string_from_utf_string_size(char16_t* dst, size_t dst_max_size, const char16_t * s, size_t size) { return utf16_string_from_utf16_string_size(dst, dst_max_size, s, size); }
inline size_t utf_string_from_utf_string_size(char32_t* dst, size_t dst_max_size, const char16_t * s, size_t size) { return utf32_string_from_utf16_string_size(dst, dst_max_size, s, size); }
inline size_t utf_string_from_utf_string_size(wchar_t* dst, size_t dst_max_size, const char16_t * s, size_t size) { return utf_string_from_utf_string_size((wchar_cast*)dst, dst_max_size, s, size); }

inline size_t utf_string_from_utf_string_size(char* dst, size_t dst_max_size, const char32_t * s, size_t size) { return utf8_string_from_utf32_string_size(dst, dst_max_size, s, size); }
inline size_t utf_string_from_utf_string_size(char16_t* dst, size_t dst_max_size, const char32_t * s, size_t size) { return utf16_string_from_utf32_string_size(dst, dst_max_size, s, size); }
inline size_t utf_string_from_utf_string_size(char32_t* dst, size_t dst_max_size, const char32_t * s, size_t size) { return utf32_string_from_utf32_string_size(dst, dst_max_size, s, size); }
inline size_t utf_string_from_utf_string_size(wchar_t* dst, size_t dst_max_size, const char32_t * s, size_t size) { return utf_string_from_utf_string_size((wchar_cast*)dst, dst_max_size, s, size); }

inline size_t utf_string_from_utf_string_size(char* dst, size_t dst_max_size, const wchar_t * s, size_t size) { return utf_string_from_utf_string_size(dst, dst_max_size, (wchar_cast*)s, size); }
inline size_t utf_string_from_utf_string_size(char16_t* dst, size_t dst_max_size, const wchar_t * s, size_t size) { return utf_string_from_utf_string_size(dst, dst_max_size, (wchar_cast*)s, size); }
inline size_t utf_string_from_utf_string_size(char32_t* dst, size_t dst_max_size, const wchar_t * s, size_t size) { return utf_string_from_utf_string_size(dst, dst_max_size, (wchar_cast*)s, size); }
inline size_t utf_string_from_utf_string_size(wchar_t* dst, size_t dst_max_size, const wchar_t * s, size_t size) { return utf_string_from_utf_string_size(dst, dst_max_size, (wchar_cast*)s, size); }



#endif //  __cplusplus













#undef wchar_cast

#endif /* __unicode_conversions_h__ */
