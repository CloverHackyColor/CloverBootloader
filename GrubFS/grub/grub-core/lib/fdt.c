/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2013  Free Software Foundation, Inc.
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

#include <grub/fdt.h>
#include <grub/misc.h>
#include <grub/mm.h>

#define FDT_SUPPORTED_VERSION	17

#define FDT_BEGIN_NODE	0x00000001
#define FDT_END_NODE	0x00000002
#define FDT_PROP	0x00000003
#define FDT_NOP		0x00000004
#define FDT_END		0x00000009

#define struct_end(fdt)	\
	((grub_addr_t) fdt + grub_fdt_get_off_dt_struct(fdt)	\
	 + grub_fdt_get_size_dt_struct(fdt))

/* Size needed by a node entry: 2 tokens (FDT_BEGIN_NODE and FDT_END_NODE), plus
   the NULL-terminated string containing the name, plus padding if needed. */
#define node_entry_size(node_name)	\
	(2 * sizeof(grub_uint32_t)	\
	+ ALIGN_UP (grub_strlen (name) + 1, sizeof(grub_uint32_t)))

/* Size needed by a property entry: 1 token (FDT_PROPERTY), plus len and nameoff
   fields, plus the property value, plus padding if needed. */
#define prop_entry_size(prop_len)	\
	(3 * sizeof(grub_uint32_t) + ALIGN_UP(prop_len, sizeof(grub_uint32_t)))

#define SKIP_NODE_NAME(name, token, end)	\
  name = (char *) ((token) + 1);	\
  while (name < (char *) end)	\
  {	\
    if (!*name++)	\
      break;	\
  }	\
  token = (grub_uint32_t *) ALIGN_UP((grub_addr_t) (name), sizeof(*token))


static grub_uint32_t *get_next_node (const void *fdt, char *node_name)
{
  grub_uint32_t *end = (void *) struct_end (fdt);
  grub_uint32_t *token;

  if (node_name >= (char *) end)
    return NULL;
  while (*node_name++)
  {
    if (node_name >= (char *) end)
  	  return NULL;
  }
  token = (grub_uint32_t *) ALIGN_UP ((grub_addr_t) node_name, 4);
  while (token < end)
  {
    switch (grub_be_to_cpu32(*token))
    {
      case FDT_BEGIN_NODE:
        token = get_next_node (fdt, (char *) (token + 1));
        if (!token)
          return NULL;
        break;
      case FDT_END_NODE:
        token++;
        if (token >= end)
          return NULL;
        return token;
      case FDT_PROP:
        /* Skip property token and following data (len, nameoff and property
           value). */
        token += prop_entry_size(grub_be_to_cpu32(*(token + 1)))
                 / sizeof(*token);
        break;
      case FDT_NOP:
        token++;
        break;
      default:
        return NULL;
    }
  }
  return NULL;
}

static int get_mem_rsvmap_size (const void *fdt)
{
  int size = 0;
  grub_uint64_t *ptr = (void *) ((grub_addr_t) fdt
                                 + grub_fdt_get_off_mem_rsvmap (fdt));

  do
  {
    size += 2 * sizeof(*ptr);
    if (!*ptr && !*(ptr + 1))
      return size;
    ptr += 2;
  } while ((grub_addr_t) ptr <= (grub_addr_t) fdt + grub_fdt_get_totalsize (fdt)
                                  - 2 * sizeof(grub_uint64_t));
  return -1;
}

static grub_uint32_t get_free_space (void *fdt)
{
  int mem_rsvmap_size = get_mem_rsvmap_size (fdt);

  if (mem_rsvmap_size < 0)
    /* invalid memory reservation block */
    return 0;
  return (grub_fdt_get_totalsize (fdt) - sizeof(grub_fdt_header_t)
          - mem_rsvmap_size - grub_fdt_get_size_dt_strings (fdt)
          - grub_fdt_get_size_dt_struct (fdt));
}

static int add_subnode (void *fdt, int parentoffset, const char *name)
{
  grub_uint32_t *token = (void *) ((grub_addr_t) fdt
                                   + grub_fdt_get_off_dt_struct(fdt)
                                   + parentoffset);
  grub_uint32_t *end = (void *) struct_end (fdt);
  unsigned int entry_size = node_entry_size (name);
  unsigned int struct_size = grub_fdt_get_size_dt_struct(fdt);
  char *node_name;

  SKIP_NODE_NAME(node_name, token, end);

  /* Insert the new subnode just after the properties of the parent node (if
     any).*/
  while (1)
  {
    if (token >= end)
      return -1;
    switch (grub_be_to_cpu32(*token))
    {
      case FDT_PROP:
        /* Skip len, nameoff and property value. */
        token += prop_entry_size(grub_be_to_cpu32(*(token + 1)))
                 / sizeof(*token);
        break;
      case FDT_BEGIN_NODE:
      case FDT_END_NODE:
        goto insert;
      case FDT_NOP:
        token++;
        break;
      default:
        /* invalid token */
        return -1;
    }
  }
insert:
  grub_memmove (token + entry_size / sizeof(*token), token,
                (grub_addr_t) end - (grub_addr_t) token);
  *token = grub_cpu_to_be32(FDT_BEGIN_NODE);
  token[entry_size / sizeof(*token) - 2] = 0;	/* padding bytes */
  grub_strcpy((char *) (token + 1), name);
  token[entry_size / sizeof(*token) - 1] = grub_cpu_to_be32(FDT_END_NODE);
  grub_fdt_set_size_dt_struct (fdt, struct_size + entry_size);
  return ((grub_addr_t) token - (grub_addr_t) fdt
          - grub_fdt_get_off_dt_struct(fdt));
}

/* Rearrange FDT blocks in the canonical order: first the memory reservation
   block (just after the FDT header), then the structure block and finally the
   strings block. No free space is left between the first and the second block,
   while the space between the second and the third block is given by the
   clearance argument. */
static int rearrange_blocks (void *fdt, unsigned int clearance)
{
  grub_uint32_t off_mem_rsvmap = ALIGN_UP(sizeof(grub_fdt_header_t), 8);
  grub_uint32_t off_dt_struct = off_mem_rsvmap + get_mem_rsvmap_size (fdt);
  grub_uint32_t off_dt_strings = off_dt_struct
                                 + grub_fdt_get_size_dt_struct (fdt)
                                 + clearance;
  grub_uint8_t *fdt_ptr = fdt;
  grub_uint8_t *tmp_fdt;

  if ((grub_fdt_get_off_mem_rsvmap (fdt) == off_mem_rsvmap)
      && (grub_fdt_get_off_dt_struct (fdt) == off_dt_struct))
    {
      /* No need to allocate memory for a temporary FDT, just move the strings
         block if needed. */
      if (grub_fdt_get_off_dt_strings (fdt) != off_dt_strings)
        {
          grub_memmove(fdt_ptr + off_dt_strings,
                       fdt_ptr + grub_fdt_get_off_dt_strings (fdt),
                       grub_fdt_get_size_dt_strings (fdt));
          grub_fdt_set_off_dt_strings (fdt, off_dt_strings);
        }
      return 0;
    }
  tmp_fdt = grub_malloc (grub_fdt_get_totalsize (fdt));
  if (!tmp_fdt)
    return -1;
  grub_memcpy (tmp_fdt + off_mem_rsvmap,
               fdt_ptr + grub_fdt_get_off_mem_rsvmap (fdt),
               get_mem_rsvmap_size (fdt));
  grub_fdt_set_off_mem_rsvmap (fdt, off_mem_rsvmap);
  grub_memcpy (tmp_fdt + off_dt_struct,
               fdt_ptr + grub_fdt_get_off_dt_struct (fdt),
               grub_fdt_get_size_dt_struct (fdt));
  grub_fdt_set_off_dt_struct (fdt, off_dt_struct);
  grub_memcpy (tmp_fdt + off_dt_strings,
               fdt_ptr + grub_fdt_get_off_dt_strings (fdt),
               grub_fdt_get_size_dt_strings (fdt));
  grub_fdt_set_off_dt_strings (fdt, off_dt_strings);

  /* Copy reordered blocks back to fdt. */
  grub_memcpy (fdt_ptr + off_mem_rsvmap, tmp_fdt + off_mem_rsvmap,
               grub_fdt_get_totalsize (fdt) - off_mem_rsvmap);

  grub_free(tmp_fdt);
  return 0;
}

static grub_uint32_t *find_prop (void *fdt, unsigned int nodeoffset,
				 const char *name)
{
  grub_uint32_t *prop = (void *) ((grub_addr_t) fdt
                                 + grub_fdt_get_off_dt_struct (fdt)
                                 + nodeoffset);
  grub_uint32_t *end = (void *) struct_end(fdt);
  grub_uint32_t nameoff;
  char *node_name;

  SKIP_NODE_NAME(node_name, prop, end);
  while (prop < end - 2)
  {
    if (grub_be_to_cpu32(*prop) == FDT_PROP)
      {
        nameoff = grub_be_to_cpu32(*(prop + 2));
        if ((nameoff + grub_strlen (name) < grub_fdt_get_size_dt_strings (fdt))
            && !grub_strcmp (name, (char *) fdt +
                             grub_fdt_get_off_dt_strings (fdt) + nameoff))
        {
          if (prop + prop_entry_size(grub_be_to_cpu32(*(prop + 1)))
              / sizeof (*prop) >= end)
            return NULL;
          return prop;
        }
        prop += prop_entry_size(grub_be_to_cpu32(*(prop + 1))) / sizeof (*prop);
      }
    else if (grub_be_to_cpu32(*prop) == FDT_NOP)
      prop++;
    else
      return NULL;
  }
  return NULL;
}

/* Check the FDT header for consistency and adjust the totalsize field to match
   the size allocated for the FDT; if this function is called before the other
   functions in this file and returns success, the other functions are
   guaranteed not to access memory locations outside the allocated memory. */
int grub_fdt_check_header_nosize (void *fdt)
{
  if (((grub_addr_t) fdt & 0x7) || (grub_fdt_get_magic (fdt) != FDT_MAGIC)
      || (grub_fdt_get_version (fdt) < FDT_SUPPORTED_VERSION)
      || (grub_fdt_get_last_comp_version (fdt) > FDT_SUPPORTED_VERSION)
      || (grub_fdt_get_off_dt_struct (fdt) & 0x00000003)
      || (grub_fdt_get_size_dt_struct (fdt) & 0x00000003)
      || (grub_fdt_get_off_dt_struct (fdt) + grub_fdt_get_size_dt_struct (fdt)
          > grub_fdt_get_totalsize (fdt))
      || (grub_fdt_get_off_dt_strings (fdt) + grub_fdt_get_size_dt_strings (fdt)
          > grub_fdt_get_totalsize (fdt))
      || (grub_fdt_get_off_mem_rsvmap (fdt) & 0x00000007)
      || (grub_fdt_get_off_mem_rsvmap (fdt)
          > grub_fdt_get_totalsize (fdt) - 2 * sizeof(grub_uint64_t)))
    return -1;
  return 0;
}

int grub_fdt_check_header (void *fdt, unsigned int size)
{
  if (size < sizeof (grub_fdt_header_t)
      || (grub_fdt_get_totalsize (fdt) > size)
      || grub_fdt_check_header_nosize (fdt) == -1)
    return -1;
  return 0;
}

/* Find a direct sub-node of a given parent node. */
int grub_fdt_find_subnode (const void *fdt, unsigned int parentoffset,
			   const char *name)
{
  grub_uint32_t *token, *end;
  char *node_name;

  if (parentoffset & 0x3)
    return -1;
  token = (void *) ((grub_addr_t) fdt + grub_fdt_get_off_dt_struct(fdt)
                    + parentoffset);
  end = (void *) struct_end (fdt);
  if ((token >= end) || (grub_be_to_cpu32(*token) != FDT_BEGIN_NODE))
    return -1;
  SKIP_NODE_NAME(node_name, token, end);
  while (token < end)
  {
    switch (grub_be_to_cpu32(*token))
    {
      case FDT_BEGIN_NODE:
        node_name = (char *) (token + 1);
        if (node_name + grub_strlen (name) >= (char *) end)
          return -1;
        if (!grub_strcmp (node_name, name))
          return (int) ((grub_addr_t) token - (grub_addr_t) fdt
                        - grub_fdt_get_off_dt_struct (fdt));
        token = get_next_node (fdt, node_name);
        if (!token)
          return -1;
        break;
      case FDT_PROP:
        /* Skip property token and following data (len, nameoff and property
           value). */
        if (token >= end - 1)
          return -1;
        token += prop_entry_size(grub_be_to_cpu32(*(token + 1)))
                 / sizeof(*token);
        break;
      case FDT_NOP:
        token++;
        break;
      default:
        return -1;
    }
  }
  return -1;
}

int grub_fdt_add_subnode (void *fdt, unsigned int parentoffset,
			  const char *name)
{
  unsigned int entry_size = node_entry_size(name);

  if ((parentoffset & 0x3) || (get_free_space (fdt) < entry_size))
    return -1;

  /* The new node entry will increase the size of the structure block: rearrange
     blocks such that there is sufficient free space between the structure and
     the strings block, then add the new node entry. */
  if (rearrange_blocks (fdt, entry_size) < 0)
    return -1;
  return add_subnode (fdt, parentoffset, name);
}

int grub_fdt_set_prop (void *fdt, unsigned int nodeoffset, const char *name,
		       const void *val, grub_uint32_t len)
{
  grub_uint32_t *prop;
  int prop_name_present = 0;
  grub_uint32_t nameoff = 0;

  if ((nodeoffset >= grub_fdt_get_size_dt_struct (fdt)) || (nodeoffset & 0x3)
      || (grub_be_to_cpu32(*(grub_uint32_t *) ((grub_addr_t) fdt
                           + grub_fdt_get_off_dt_struct (fdt) + nodeoffset))
          != FDT_BEGIN_NODE))
    return -1;
  prop = find_prop (fdt, nodeoffset, name);
  if (prop)
    {
	  grub_uint32_t prop_len = ALIGN_UP(grub_be_to_cpu32 (*(prop + 1)),
                                        sizeof(grub_uint32_t));
	  grub_uint32_t i;

      prop_name_present = 1;
	  for (i = 0; i < prop_len / sizeof(grub_uint32_t); i++)
        *(prop + 3 + i) = grub_cpu_to_be32 (FDT_NOP);
      if (len > ALIGN_UP(prop_len, sizeof(grub_uint32_t)))
        {
          /* Length of new property value is greater than the space allocated
             for the current value: a new entry needs to be created, so save the
             nameoff field of the current entry and replace the current entry
             with NOP tokens. */
          nameoff = grub_be_to_cpu32 (*(prop + 2));
    	  *prop = *(prop + 1) = *(prop + 2) = grub_cpu_to_be32 (FDT_NOP);
          prop = NULL;
        }
    }
  if (!prop || !prop_name_present) {
    unsigned int needed_space = 0;

    if (!prop)
      needed_space = prop_entry_size(len);
    if (!prop_name_present)
      needed_space += grub_strlen (name) + 1;
    if (needed_space > get_free_space (fdt))
      return -1;
    if (rearrange_blocks (fdt, !prop ? prop_entry_size(len) : 0) < 0)
      return -1;
  }
  if (!prop_name_present) {
    /* Append the property name at the end of the strings block. */
    nameoff = grub_fdt_get_size_dt_strings (fdt);
    grub_strcpy ((char *) fdt + grub_fdt_get_off_dt_strings (fdt) + nameoff,
                 name);
    grub_fdt_set_size_dt_strings (fdt, grub_fdt_get_size_dt_strings (fdt)
                                  + grub_strlen (name) + 1);
  }
  if (!prop) {
    char *node_name = (char *) ((grub_addr_t) fdt
                                + grub_fdt_get_off_dt_struct (fdt) + nodeoffset
                                + sizeof(grub_uint32_t));

    prop = (void *) (node_name + ALIGN_UP(grub_strlen(node_name) + 1, 4));
    grub_memmove (prop + prop_entry_size(len) / sizeof(*prop), prop,
                  struct_end(fdt) - (grub_addr_t) prop);
    grub_fdt_set_size_dt_struct (fdt, grub_fdt_get_size_dt_struct (fdt)
                                 + prop_entry_size(len));
    *prop = grub_cpu_to_be32 (FDT_PROP);
    *(prop + 2) = grub_cpu_to_be32 (nameoff);
  }
  *(prop + 1) = grub_cpu_to_be32 (len);

  /* Insert padding bytes at the end of the value; if they are not needed, they
     will be overwritten by the following memcpy. */
  *(prop + prop_entry_size(len) / sizeof(grub_uint32_t) - 1) = 0;

  grub_memcpy (prop + 3, val, len);
  return 0;
}

int
grub_fdt_create_empty_tree (void *fdt, unsigned int size)
{
  struct grub_fdt_empty_tree *et;

  if (size < GRUB_FDT_EMPTY_TREE_SZ)
    return -1;

  grub_memset (fdt, 0, size);
  et = fdt;

  et->empty_node.tree_end = grub_cpu_to_be32_compile_time (FDT_END);
  et->empty_node.node_end = grub_cpu_to_be32_compile_time (FDT_END_NODE);
  et->empty_node.node_start = grub_cpu_to_be32_compile_time (FDT_BEGIN_NODE);
  ((struct grub_fdt_empty_tree *) fdt)->header.off_mem_rsvmap =
    grub_cpu_to_be32 (ALIGN_UP (sizeof (grub_fdt_header_t), 8));

  grub_fdt_set_off_dt_strings (fdt, sizeof (*et));
  grub_fdt_set_off_dt_struct (fdt,
			      sizeof (et->header) + sizeof (et->empty_rsvmap));
  grub_fdt_set_version (fdt, FDT_SUPPORTED_VERSION);
  grub_fdt_set_last_comp_version (fdt, FDT_SUPPORTED_VERSION);
  grub_fdt_set_size_dt_struct (fdt, sizeof (et->empty_node));
  grub_fdt_set_totalsize (fdt, size);
  grub_fdt_set_magic (fdt, FDT_MAGIC);

  return 0;
}
