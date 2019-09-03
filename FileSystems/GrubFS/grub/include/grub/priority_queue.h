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

#ifndef GRUB_PRIORITY_QUEUE_HEADER
#define GRUB_PRIORITY_QUEUE_HEADER 1

#include <grub/misc.h>
#include <grub/err.h>

#ifdef __cplusplus
extern "C" {
#endif

struct grub_priority_queue;
typedef struct grub_priority_queue *grub_priority_queue_t;
typedef int (*grub_comparator_t) (const void *a, const void *b);

grub_priority_queue_t grub_priority_queue_new (grub_size_t elsize,
					       grub_comparator_t cmp);
void grub_priority_queue_destroy (grub_priority_queue_t pq);
void *grub_priority_queue_top (grub_priority_queue_t pq);
void grub_priority_queue_pop (grub_priority_queue_t pq);
grub_err_t grub_priority_queue_push (grub_priority_queue_t pq, const void *el);

#ifdef __cplusplus
}
#endif

#endif
