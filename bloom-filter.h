/* bloom-filter.h
 *
 * Copyright (C) 2012 Christian Hergert <christian@hergert.me>
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BLOOM_FILTER_H
#define BLOOM_FILTER_H

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _BloomFilter BloomFilter;

struct _BloomFilter
{
   /*< private >*/
   gint       ref_count;
   GPtrArray *hash_funcs;
   gsize      width;
   guint8     data[0];
};

GType        bloom_filter_get_type (void) G_GNUC_CONST;
BloomFilter *bloom_filter_new      (gsize        width,
                                    guint        n_hash_funcs,
                                    GHashFunc    first_hash_func,
                                    ...);
BloomFilter *bloom_filter_ref      (BloomFilter *filter);
void         bloom_filter_unref    (BloomFilter *filter);

G_INLINE_FUNC gboolean
bloom_filter_get_bit (BloomFilter *filter,
                      gsize        bit)
{
   g_return_val_if_fail(filter != NULL, FALSE);
   g_return_val_if_fail(bit < filter->width, FALSE);
   return !!(filter->data[bit >> 3] & (1 << (bit & 0x7)));
}

G_INLINE_FUNC void
bloom_filter_set_bit (BloomFilter *filter,
                      gsize        bit)
{
   g_return_if_fail(filter != NULL);
   g_return_if_fail(bit < filter->width);
   filter->data[bit >> 3] |= (1 << (bit & 0x7));
}

G_INLINE_FUNC void
bloom_filter_insert (BloomFilter   *filter,
                     gconstpointer  key)
{
   GHashFunc hash_func;
   guint i;

   g_return_if_fail(filter);

   for (i = 0; i < filter->hash_funcs->len; i++) {
      hash_func = g_ptr_array_index(filter->hash_funcs, i);
      bloom_filter_set_bit(filter, hash_func(key) % filter->width);
   }
}

G_INLINE_FUNC gboolean
bloom_filter_contains (BloomFilter   *filter,
                       gconstpointer  key)
{
   GHashFunc hash_func;
   guint i;

   g_return_val_if_fail(filter, FALSE);

   for (i = 0; i < filter->hash_funcs->len; i++) {
      hash_func = g_ptr_array_index(filter->hash_funcs, i);
      if (!bloom_filter_get_bit(filter, hash_func(key) % filter->width)) {
         return FALSE;
      }
   }

   return TRUE;
}

G_END_DECLS

#endif /* BLOOM_FILTER_H */
