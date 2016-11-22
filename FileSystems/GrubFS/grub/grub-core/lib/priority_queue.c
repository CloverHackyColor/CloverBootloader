/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2011 Free Software Foundation, Inc.
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

#include <grub/priority_queue.h>
#include <grub/mm.h>
#include <grub/dl.h>

GRUB_MOD_LICENSE ("GPLv3+");

struct grub_priority_queue
{
  grub_size_t elsize;
  grub_size_t allocated;
  grub_size_t used;
  grub_comparator_t cmp;
  void *els;
};

static inline void *
element (struct grub_priority_queue *pq, grub_size_t k)
{
  return ((grub_uint8_t *) pq->els) + k * pq->elsize;
}

static inline void
swap (struct grub_priority_queue *pq, grub_size_t m, grub_size_t n)
{
  grub_uint8_t *p1, *p2;
  grub_size_t l;
  p1 = (grub_uint8_t *) element (pq, m);
  p2 = (grub_uint8_t *) element (pq, n);
  for (l = pq->elsize; l; l--, p1++, p2++)
    {
      grub_uint8_t t;
      t = *p1;
      *p1 = *p2;
      *p2 = t;
    }
}

static inline grub_size_t
parent (grub_size_t v)
{
  return (v - 1) / 2;
}

static inline grub_size_t
left_child (grub_size_t v)
{
  return 2 * v + 1;
}

static inline grub_size_t
right_child (grub_size_t v)
{
  return 2 * v + 2;
}

void *
grub_priority_queue_top (grub_priority_queue_t pq)
{
  if (!pq->used)
    return 0;
  return element (pq, 0);
}

void
grub_priority_queue_destroy (grub_priority_queue_t pq)
{
  grub_free (pq->els);
  grub_free (pq);
}

grub_priority_queue_t
grub_priority_queue_new (grub_size_t elsize,
			 grub_comparator_t cmp)
{
  struct grub_priority_queue *ret;
  void *els;
  els = grub_malloc (elsize * 8);
  if (!els)
    return 0;
  ret = (struct grub_priority_queue *) grub_malloc (sizeof (*ret));
  if (!ret)
    {
      grub_free (els);
      return 0;
    }
  ret->elsize = elsize;
  ret->allocated = 8;
  ret->used = 0;
  ret->cmp = cmp;
  ret->els = els;
  return ret;
}

/* Heap property: pq->cmp (element (pq, p), element (pq, parent (p))) <= 0. */
grub_err_t
grub_priority_queue_push (grub_priority_queue_t pq, const void *el)
{
  grub_size_t p;
  if (pq->used == pq->allocated)
    {
      void *els;
      els = grub_realloc (pq->els, pq->elsize * 2 * pq->allocated);
      if (!els)
	return grub_errno;
      pq->allocated *= 2;
      pq->els = els;
    }
  pq->used++;
  grub_memcpy (element (pq, pq->used - 1), el, pq->elsize);
  for (p = pq->used - 1; p; p = parent (p))
    {
      if (pq->cmp (element (pq, p), element (pq, parent (p))) <= 0)
	break;
      swap (pq, p, parent (p));
    }

  return GRUB_ERR_NONE;
}

void
grub_priority_queue_pop (grub_priority_queue_t pq)
{
  grub_size_t p;
  
  swap (pq, 0, pq->used - 1);
  pq->used--;
  for (p = 0; left_child (p) < pq->used; )
    {
      grub_size_t c;
      if (pq->cmp (element (pq, left_child (p)), element (pq, p)) <= 0
	  && (right_child (p) >= pq->used
	      || pq->cmp (element (pq, right_child (p)), element (pq, p)) <= 0))
	break;
      if (right_child (p) >= pq->used
	  || pq->cmp (element (pq, left_child (p)),
		      element (pq, right_child (p))) > 0)
	c = left_child (p);
      else
	c = right_child (p);
      swap (pq, p, c);
      p = c;
    }
}


