/* list.h - header for grub list */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2009  Free Software Foundation, Inc.
 *
 *  GRUB is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GRUB is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GRUB.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GRUB_LIST_HEADER
#define GRUB_LIST_HEADER 1

#include <grub/symbol.h>
#include <grub/err.h>
#include <grub/compiler.h>

struct grub_list
{
  struct grub_list *next;
  struct grub_list **prev;
};
typedef struct grub_list *grub_list_t;

void EXPORT_FUNC(grub_list_push) (grub_list_t *head, grub_list_t item);
void EXPORT_FUNC(grub_list_remove) (grub_list_t item);

#define FOR_LIST_ELEMENTS(var, list) for ((var) = (list); (var); (var) = (var)->next)
#define FOR_LIST_ELEMENTS_SAFE(var, nxt, list) for ((var) = (list), (nxt) = ((var) ? (var)->next : 0); (var); (var) = (nxt), ((nxt) = (var) ? (var)->next : 0))

static inline void *
grub_bad_type_cast_real (int line, const char *file)
     ATTRIBUTE_ERROR ("bad type cast between incompatible grub types");

static inline void *
grub_bad_type_cast_real (int line, const char *file)
{
  grub_fatal ("error:%s:%u: bad type cast between incompatible grub types",
	      file, line);
}

#define grub_bad_type_cast() grub_bad_type_cast_real(__LINE__, GRUB_FILE)

#define GRUB_FIELD_MATCH(ptr, type, field) \
  ((char *) &(ptr)->field == (char *) &((type) (ptr))->field)

#define GRUB_AS_LIST(ptr) \
  (GRUB_FIELD_MATCH (ptr, grub_list_t, next) && GRUB_FIELD_MATCH (ptr, grub_list_t, prev) ? \
   (grub_list_t) ptr : (grub_list_t) grub_bad_type_cast ())

#define GRUB_AS_LIST_P(pptr) \
  (GRUB_FIELD_MATCH (*pptr, grub_list_t, next) && GRUB_FIELD_MATCH (*pptr, grub_list_t, prev) ? \
   (grub_list_t *) (void *) pptr : (grub_list_t *) grub_bad_type_cast ())

struct grub_named_list
{
  struct grub_named_list *next;
  struct grub_named_list **prev;
  char *name;
};
typedef struct grub_named_list *grub_named_list_t;

void * EXPORT_FUNC(grub_named_list_find) (grub_named_list_t head,
					  const char *name);

#define GRUB_AS_NAMED_LIST(ptr) \
  ((GRUB_FIELD_MATCH (ptr, grub_named_list_t, next) \
    && GRUB_FIELD_MATCH (ptr, grub_named_list_t, prev)	\
    && GRUB_FIELD_MATCH (ptr, grub_named_list_t, name))? \
   (grub_named_list_t) ptr : (grub_named_list_t) grub_bad_type_cast ())

#define GRUB_AS_NAMED_LIST_P(pptr) \
  ((GRUB_FIELD_MATCH (*pptr, grub_named_list_t, next) \
    && GRUB_FIELD_MATCH (*pptr, grub_named_list_t, prev)   \
    && GRUB_FIELD_MATCH (*pptr, grub_named_list_t, name))? \
   (grub_named_list_t *) (void *) pptr : (grub_named_list_t *) grub_bad_type_cast ())

#endif /* ! GRUB_LIST_HEADER */
