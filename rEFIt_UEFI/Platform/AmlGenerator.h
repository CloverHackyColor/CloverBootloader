/*
 *  aml_generator.h
 *  Chameleon
 *
 *  Created by Mozodojo on 20/07/10.
 *  Copyright 2010 mozo. All rights reserved.
 *
 */

#ifndef _AML_GENERATOR_H
#define _AML_GENERATOR_H

#include "Platform.h"

/*
static inline BOOLEAN aml_isvalidchar(char c)
{
	return IS_UPPER(c) || IS_DIGIT(c) || c == '_';
};
*/


struct aml_chunk
{
  UINT8              Type;
  UINT8              pad;
  UINT16             Length;
  UINT32             pad2;
  CHAR8              *Buffer;

  UINT16             Size;
  UINT16             pad3[3];

  struct aml_chunk*  Next;
  struct aml_chunk*  First;
  struct aml_chunk*  Last;
};
typedef struct aml_chunk AML_CHUNK;


#define  AML_CHUNK_NONE          0xff
#define  AML_CHUNK_ZERO          0x00
#define  AML_CHUNK_ONE           0x01
#define  AML_CHUNK_ALIAS         0x06
#define  AML_CHUNK_NAME          0x08
#define  AML_CHUNK_BYTE          0x0A
#define  AML_CHUNK_WORD          0x0B
#define  AML_CHUNK_DWORD         0x0C
#define  AML_CHUNK_STRING        0x0D
#define  AML_CHUNK_QWORD         0x0E
#define  AML_CHUNK_SCOPE         0x10
#define  AML_CHUNK_PACKAGE       0x12
#define  AML_CHUNK_METHOD        0x14
#define AML_CHUNK_RETURN         0xA4
#define AML_LOCAL0               0x60
#define AML_STORE_OP             0x70
//-----------------------------------
// defines added by pcj
#define  AML_CHUNK_BUFFER        0x11
#define  AML_CHUNK_STRING_BUFFER 0x15
#define  AML_CHUNK_OP            0x5B
#define  AML_CHUNK_REFOF         0x71
#define  AML_CHUNK_DEVICE        0x82
#define  AML_CHUNK_LOCAL0        0x60
#define  AML_CHUNK_LOCAL1        0x61
#define  AML_CHUNK_LOCAL2        0x62

#define  AML_CHUNK_ARG0          0x68
#define  AML_CHUNK_ARG1          0x69
#define  AML_CHUNK_ARG2          0x6A
#define  AML_CHUNK_ARG3          0x6B


BOOLEAN aml_add_to_parent(AML_CHUNK* parent, AML_CHUNK* node);
AML_CHUNK* aml_create_node(AML_CHUNK* parent);
VOID aml_destroy_node(AML_CHUNK* node);
AML_CHUNK* aml_add_buffer(AML_CHUNK* parent, /* CONST*/ UINT8* buffer, UINT32 size);
AML_CHUNK* aml_add_byte(AML_CHUNK* parent, UINT8 value);
AML_CHUNK* aml_add_word(AML_CHUNK* parent, UINT16 value);
AML_CHUNK* aml_add_dword(AML_CHUNK* parent, UINT32 value);
AML_CHUNK* aml_add_qword(AML_CHUNK* parent, UINT64 value);
AML_CHUNK* aml_add_scope(AML_CHUNK* parent, /* CONST*/ CHAR8* name);
AML_CHUNK* aml_add_name(AML_CHUNK* parent, CONST CHAR8* name);
AML_CHUNK* aml_add_method(AML_CHUNK* parent, CONST CHAR8* name, UINT8 args);
AML_CHUNK* aml_add_return_name(AML_CHUNK* parent, CONST CHAR8* name);
AML_CHUNK* aml_add_return_byte(AML_CHUNK* parent, UINT8 value);
AML_CHUNK* aml_add_package(AML_CHUNK* parent);
AML_CHUNK* aml_add_alias(AML_CHUNK* parent, /* CONST*/ CHAR8* name1, /* CONST*/ CHAR8* name2);
UINT32 aml_calculate_size(AML_CHUNK* node);
UINT32 aml_write_node(AML_CHUNK* node, CHAR8* buffer, UINT32 offset);
UINT32 aml_write_size(UINT32 size, CHAR8* buffer, UINT32 offset);

// add by pcj
AML_CHUNK* aml_add_string(AML_CHUNK* parent, CONST CHAR8* string);
AML_CHUNK* aml_add_byte_buffer(AML_CHUNK* parent, /* CONST*/ UINT8* data,UINT32 size);
AML_CHUNK* aml_add_string_buffer(AML_CHUNK* parent, CONST CHAR8* string);
AML_CHUNK* aml_add_device(AML_CHUNK* parent, CONST CHAR8* name);
AML_CHUNK* aml_add_local0(AML_CHUNK* parent);
AML_CHUNK* aml_add_store(AML_CHUNK* parent);
AML_CHUNK* aml_add_return(AML_CHUNK* parent);


UINT32 get_size(UINT8* Buffer, UINT32 adr);

#endif /* !_AML_GENERATOR_H */
