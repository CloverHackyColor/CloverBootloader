/* gui.h - GUI components header file. */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2008,2009  Free Software Foundation, Inc.
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

#include <grub/types.h>
#include <grub/err.h>
#include <grub/video.h>
#include <grub/bitmap.h>
#include <grub/gfxmenu_view.h>
#include <grub/mm.h>

#ifndef GRUB_GUI_H
#define GRUB_GUI_H 1

/* The component ID identifying GUI components to be updated as the timeout
   status changes.  */
#define GRUB_GFXMENU_TIMEOUT_COMPONENT_ID "__timeout__"

typedef struct grub_gui_component *grub_gui_component_t;
typedef struct grub_gui_container *grub_gui_container_t;
typedef struct grub_gui_list *grub_gui_list_t;

typedef void (*grub_gui_component_callback) (grub_gui_component_t component,
                                             void *userdata);

/* Component interface.  */

struct grub_gui_component_ops
{
  void (*destroy) (void *self);
  const char * (*get_id) (void *self);
  int (*is_instance) (void *self, const char *type);
  void (*paint) (void *self, const grub_video_rect_t *bounds);
  void (*set_parent) (void *self, grub_gui_container_t parent);
  grub_gui_container_t (*get_parent) (void *self);
  void (*set_bounds) (void *self, const grub_video_rect_t *bounds);
  void (*get_bounds) (void *self, grub_video_rect_t *bounds);
  void (*get_minimal_size) (void *self, unsigned *width, unsigned *height);
  grub_err_t (*set_property) (void *self, const char *name, const char *value);
  void (*repaint) (void *self, int second_pass);
};

struct grub_gui_container_ops
{
  void (*add) (void *self, grub_gui_component_t comp);
  void (*remove) (void *self, grub_gui_component_t comp);
  void (*iterate_children) (void *self,
                            grub_gui_component_callback cb, void *userdata);
};

struct grub_gui_list_ops
{
  void (*set_view_info) (void *self,
                         grub_gfxmenu_view_t view);
  void (*refresh_list) (void *self,
                        grub_gfxmenu_view_t view);
};

struct grub_gui_progress_ops
{
  void (*set_state) (void *self, int visible, int start, int current, int end);
};

typedef void (*grub_gfxmenu_set_state_t) (void *self, int visible, int start,
					  int current, int end);

struct grub_gfxmenu_timeout_notify
{
  struct grub_gfxmenu_timeout_notify *next;
  grub_gfxmenu_set_state_t set_state;
  grub_gui_component_t self;
};

extern struct grub_gfxmenu_timeout_notify *grub_gfxmenu_timeout_notifications;

static inline grub_err_t
grub_gfxmenu_timeout_register (grub_gui_component_t self,
			       grub_gfxmenu_set_state_t set_state)
{
  struct grub_gfxmenu_timeout_notify *ne = grub_malloc (sizeof (*ne));
  if (!ne)
    return grub_errno;
  ne->set_state = set_state;
  ne->self = self;
  ne->next = grub_gfxmenu_timeout_notifications;
  grub_gfxmenu_timeout_notifications = ne;
  return GRUB_ERR_NONE;
}

static inline void
grub_gfxmenu_timeout_unregister (grub_gui_component_t self)
{
  struct grub_gfxmenu_timeout_notify **p, *q;

  for (p = &grub_gfxmenu_timeout_notifications, q = *p;
       q; p = &(q->next), q = q->next)
    if (q->self == self)
      {
	*p = q->next;
	grub_free (q);
	break;
      }
}

typedef signed grub_fixed_signed_t;
#define GRUB_FIXED_1 0x10000

/* Special care is taken to round to nearest integer and not just truncate.  */
static inline signed
grub_divide_round (signed a, signed b)
{
  int neg = 0;
  signed ret;
  if (b < 0)
    {
      b = -b;
      neg = !neg;
    }
  if (a < 0)
    {
      a = -a;
      neg = !neg;
    }
  ret = (unsigned) (a + b / 2) / (unsigned) b;
  return neg ? -ret : ret;
}

static inline signed
grub_fixed_sfs_divide (signed a, grub_fixed_signed_t b)
{
  return grub_divide_round (a * GRUB_FIXED_1, b);
}

static inline grub_fixed_signed_t
grub_fixed_fsf_divide (grub_fixed_signed_t a, signed b)
{
  return grub_divide_round (a, b);
}

static inline signed
grub_fixed_sfs_multiply (signed a, grub_fixed_signed_t b)
{
  return (a * b) / GRUB_FIXED_1;
}

static inline signed
grub_fixed_to_signed (grub_fixed_signed_t in)
{
  return in / GRUB_FIXED_1;
}

static inline grub_fixed_signed_t
grub_signed_to_fixed (signed in)
{
  return in * GRUB_FIXED_1;
}

struct grub_gui_component
{
  struct grub_gui_component_ops *ops;
  signed x;
  grub_fixed_signed_t xfrac;
  signed y;
  grub_fixed_signed_t yfrac;
  signed w;
  grub_fixed_signed_t wfrac;
  signed h;
  grub_fixed_signed_t hfrac;
};

struct grub_gui_progress
{
  struct grub_gui_component component;
  struct grub_gui_progress_ops *ops;
};

struct grub_gui_container
{
  struct grub_gui_component component;
  struct grub_gui_container_ops *ops;
};

struct grub_gui_list
{
  struct grub_gui_component component;
  struct grub_gui_list_ops *ops;
};


/* Interfaces to concrete component classes.  */

grub_gui_container_t grub_gui_canvas_new (void);
grub_gui_container_t grub_gui_vbox_new (void);
grub_gui_container_t grub_gui_hbox_new (void);
grub_gui_component_t grub_gui_label_new (void);
grub_gui_component_t grub_gui_image_new (void);
grub_gui_component_t grub_gui_progress_bar_new (void);
grub_gui_component_t grub_gui_list_new (void);
grub_gui_component_t grub_gui_circular_progress_new (void);

/* Manipulation functions.  */

/* Visit all components with the specified ID.  */
void grub_gui_find_by_id (grub_gui_component_t root,
                          const char *id,
                          grub_gui_component_callback cb,
                          void *userdata);

/* Visit all components.  */
void grub_gui_iterate_recursively (grub_gui_component_t root,
                                   grub_gui_component_callback cb,
                                   void *userdata);

/* Helper functions.  */

static __inline void
grub_gui_save_viewport (grub_video_rect_t *r)
{
  grub_video_get_viewport ((unsigned *) &r->x,
                           (unsigned *) &r->y,
                           (unsigned *) &r->width,
                           (unsigned *) &r->height);
}

static __inline void
grub_gui_restore_viewport (const grub_video_rect_t *r)
{
  grub_video_set_viewport (r->x, r->y, r->width, r->height);
}

/* Set a new viewport relative the the current one, saving the current
   viewport in OLD so it can be later restored.  */
static __inline void
grub_gui_set_viewport (const grub_video_rect_t *r, grub_video_rect_t *old)
{
  grub_gui_save_viewport (old);
  grub_video_set_viewport (old->x + r->x,
                           old->y + r->y,
                           r->width,
                           r->height);
}

static inline int
grub_video_have_common_points (const grub_video_rect_t *a,
			       const grub_video_rect_t *b)
{
  if (!((a->x <= b->x && b->x <= a->x + a->width)
	|| (b->x <= a->x && a->x <= b->x + b->width)))
    return 0;
  if (!((a->y <= b->y && b->y <= a->y + a->height)
	|| (b->y <= a->y && a->y <= b->y + b->height)))
    return 0;
  return 1;
}

static inline int
grub_video_bounds_inside_region (const grub_video_rect_t *b,
                                 const grub_video_rect_t *r)
{
  if (r->x > b->x || r->x + r->width < b->x + b->width)
    return 0;
  if (r->y > b->y || r->y + r->height < b->y + b->height)
    return 0;
  return 1;
}

#endif /* ! GRUB_GUI_H */
