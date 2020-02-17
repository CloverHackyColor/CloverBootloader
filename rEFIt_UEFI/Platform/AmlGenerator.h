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
