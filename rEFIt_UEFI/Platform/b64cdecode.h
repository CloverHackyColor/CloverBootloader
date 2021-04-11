/*

cdecode.h - c header for a base64 decoding algorithm



This is part of the libb64 project, and has been placed in the public domain.

For details, see http://sourceforge.net/projects/libb64

*/



#ifndef BASE64_CDECODE_H

#define BASE64_CDECODE_H





UINT8 *Base64DecodeClover(IN CONST CHAR8 *EncodedData, size_t EncodedSize, OUT UINTN *DecodedSize);
UINT8 *Base64DecodeClover(IN CONST CHAR8 *EncodedData, OUT  UINTN *DecodedSize);


#endif /* BASE64_CDECODE_H */

