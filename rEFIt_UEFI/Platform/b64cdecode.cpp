/*
cdecoder.c - c source to a base64 decoding algorithm implementation

This is part of the libb64 project, and has been placed in the public domain.
For details, see http://sourceforge.net/projects/libb64
*/

#include <Platform.h> // Only use angled for Platform, else, xcode project won't compile
#include "b64cdecode.h"

int base64_decode_value(char value_in)
{
	static const signed char decoding[] = {62,-1,-1,-1,63,52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-2,-1,-1,-1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51};
	int value_in_i = (unsigned char) value_in;
	value_in_i -= 43;
	if (value_in_i < 0 || (unsigned int) value_in_i >= sizeof decoding) return -1;
	return decoding[value_in_i];
}

void base64_init_decodestate(base64_decodestate* state_in)
{
	state_in->step = step_a;
	state_in->plainchar = 0;
}

long base64_decode_block(const char* code_in, const int length_in, char* plaintext_out, base64_decodestate* state_in)
{
	const char* codechar = code_in;
	char* plainchar = plaintext_out;
	int fragment;

	*plainchar = state_in->plainchar;

	switch (state_in->step)
	{
		while (1)
		{
	case step_a:
			do {
				if (codechar == code_in+length_in)
				{
					state_in->step = step_a;
					state_in->plainchar = *plainchar;
					return (long)(plainchar - plaintext_out); // we assume that plainchar - plaintext_out cannot be > MAX_LONG
				}
				fragment = base64_decode_value(*codechar++);
			} while (fragment < 0);
			*plainchar    = (char) ((fragment & 0x03f) << 2);
	case step_b:
			do {
				if (codechar == code_in+length_in)
				{
					state_in->step = step_b;
					state_in->plainchar = *plainchar;
					return (long)(plainchar - plaintext_out); // we assume that plainchar - plaintext_out cannot be > MAX_LONG
				}
				fragment = base64_decode_value(*codechar++);
			} while (fragment < 0);
			*plainchar++ |= (char) ((fragment & 0x030) >> 4);
			*plainchar    = (char) ((fragment & 0x00f) << 4);
	case step_c:
			do {
				if (codechar == code_in+length_in)
				{
					state_in->step = step_c;
					state_in->plainchar = *plainchar;
					return (long)(plainchar - plaintext_out); // we assume that plainchar - plaintext_out cannot be > MAX_LONG
				}
				fragment = base64_decode_value(*codechar++);
			} while (fragment < 0);
			*plainchar++ |= (char) ((fragment & 0x03c) >> 2);
			*plainchar    = (char) ((fragment & 0x003) << 6);
	case step_d:
			do {
				if (codechar == code_in+length_in)
				{
					state_in->step = step_d;
					state_in->plainchar = *plainchar;
					return (long)(plainchar - plaintext_out); // we assume that plainchar - plaintext_out cannot be > MAX_LONG
				}
				fragment = base64_decode_value(*codechar++);
			} while (fragment < 0);
			*plainchar++   |= (char) ((fragment & 0x03f));
		}
	}
	/* control should not reach here */
	return (long)(plainchar - plaintext_out); // we assume that plainchar - plaintext_out cannot be > MAX_LONG
}


/** UEFI interface to base54 decode.
 * Decodes EncodedData into a new allocated buffer and returns it. Caller is responsible to FreePool() it.
 * If DecodedSize != NULL, then size od decoded data is put there.
 * If return value is not NULL, DecodedSize IS > 0
 */
UINT8 *Base64DecodeClover(IN CONST CHAR8 *EncodedData, UINTN EncodedSize, OUT UINTN *DecodedSize)
{
	INTN				DecodedSizeInternal;
	UINT8				*DecodedData;
	base64_decodestate	state_in;

  if (EncodedData == NULL || EncodedSize == 0 ) {
		return NULL;
	}

	// to simplify, we'll allocate the same size, although smaller size is needed
	DecodedData = (__typeof__(DecodedData))AllocateZeroPool(EncodedSize);

	base64_init_decodestate(&state_in);
	DecodedSizeInternal = base64_decode_block(EncodedData, (const int)EncodedSize, (char*) DecodedData, &state_in);

	if ( DecodedSizeInternal == 0 ) {
    FreePool(DecodedData);
    DecodedData = NULL;
  }

	if (DecodedSize != NULL) {
    if ( DecodedSizeInternal < 0 ) panic("Base64DecodeClover : DecodedSizeInternal < 0");
		*DecodedSize = (UINTN)DecodedSizeInternal;
	}

	return DecodedData;
}


/** UEFI interface to base54 decode.
 * Decodes EncodedData into a new allocated buffer and returns it. Caller is responsible to FreePool() it.
 * If DecodedSize != NULL, then size od decoded data is put there.
 * If return value is not NULL, DecodedSize IS > 0
 */
UINT8 *Base64DecodeClover(IN CONST CHAR8 *EncodedData, OUT UINTN *DecodedSize)
{
	if (EncodedData == NULL) {
		return NULL;
	}
	return Base64DecodeClover(EncodedData, strlen(EncodedData), DecodedSize);
}
