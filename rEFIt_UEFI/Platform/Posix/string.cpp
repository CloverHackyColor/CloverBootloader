
#include <posix.h>


// Function to implement strncat() function in C
char* strncat(char* destination, const char* source, size_t num)
{
  // make ptr point to the end of destination string
  char* ptr = destination + strlen(destination);

  // Appends characters of source to the destination string
  while (*source != '\0' && num--)
    *ptr++ = *source++;

  // null terminate destination string
  *ptr = '\0';

  // destination string is returned by standard strncat()
  return destination;
}


#ifndef strcmp
/*
 * Taken from glibc. Seems not optimized at all...
 */
/* Compare S1 and S2, returning less than, equal to or
   greater than zero if S1 is lexicographically less than,
   equal to or greater than S2.  */
int strcmp (const char *p1, const char *p2)
{
  const unsigned char *s1 = (const unsigned char *) p1;
  const unsigned char *s2 = (const unsigned char *) p2;
  unsigned char c1, c2;

  do
    {
      c1 = (unsigned char) *s1++;
      c2 = (unsigned char) *s2++;
      if (c1 == '\0')
        return c1 - c2;
    }
  while (c1 == c2);

  return c1 - c2;
}
#endif


#ifndef strncmp
// faster implementation than AsciiStrnCmp. AsciiStrnCmp calls AsciiStrSize 4 times.
//int strncmp( const char * s1, const char * s2, size_t n )
//{
//    while ( n && *s1 && ( *s1 == *s2 ) )
//    {
//        ++s1;
//        ++s2;
//        --n;
//    }
//    if ( n == 0 )
//    {
//        return 0;
//    }
//    else
//    {
//        return ( *(unsigned char *)s1 - *(unsigned char *)s2 );
//    }
//}
/* Compare no more than N characters of S1 and S2,
   returning less than, equal to or greater than zero
   if S1 is lexicographically less than, equal to or
   greater than S2.  */
int
strncmp(const char *s1, const char *s2, size_t n)
{
  unsigned char c1 = '\0';
  unsigned char c2 = '\0';

  if (n >= 4)
    {
      size_t n4 = n >> 2;
      do
        {
          c1 = (unsigned char) *s1++;
          c2 = (unsigned char) *s2++;
          if (c1 == '\0' || c1 != c2)
            return c1 - c2;
          c1 = (unsigned char) *s1++;
          c2 = (unsigned char) *s2++;
          if (c1 == '\0' || c1 != c2)
            return c1 - c2;
          c1 = (unsigned char) *s1++;
          c2 = (unsigned char) *s2++;
          if (c1 == '\0' || c1 != c2)
            return c1 - c2;
          c1 = (unsigned char) *s1++;
          c2 = (unsigned char) *s2++;
          if (c1 == '\0' || c1 != c2)
            return c1 - c2;
        } while (--n4 > 0);
      n &= 3;
    }

  while (n > 0)
    {
      c1 = (unsigned char) *s1++;
      c2 = (unsigned char) *s2++;
      if (c1 == '\0' || c1 != c2)
        return c1 - c2;
      n--;
    }

  return c1 - c2;
}




//
//// Got his here : http://mgronhol.github.io/fast-strcmp/
//int strncmp_(const char *ptr0, const char *ptr1, size_t len)
//{
//	size_t fast = len / sizeof(size_t) + 1;
//	size_t offset = (fast - 1) * sizeof(size_t);
//	size_t current_block = 0;
//
//	if (len < sizeof(size_t)) {
//		fast = 0;
//	}
//
//	size_t *lptr0 = (size_t*) ptr0;
//	size_t *lptr1 = (size_t*) ptr1;
//
//	while (current_block < fast) {
//		if ((lptr0[current_block] ^ lptr1[current_block])) {
//			size_t pos;
//			for (pos = current_block * sizeof(size_t); pos < len; ++pos) {
//				if ((ptr0[pos] ^ ptr1[pos]) || (ptr0[pos] == 0) || (ptr1[pos] == 0)) {
//					return (int) ((unsigned char) ptr0[pos] - (unsigned char) ptr1[pos]);
//				}
//			}
//		}
//
//		++current_block;
//	}
//
//	while (len > offset) {
//		if ((ptr0[offset] ^ ptr1[offset])) {
//			return (int) ((unsigned char) ptr0[offset] - (unsigned char) ptr1[offset]);
//		}
//		++offset;
//	}
//
//	return 0;
//}


#endif
