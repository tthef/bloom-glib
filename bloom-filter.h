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
   gint       key_len;
   guint8     data[0];
};

typedef guint32 (*BloomHashFunc) (gconstpointer key, int len, guint32 seed);

void         bloom_filter_remove_all (BloomFilter *filter);
GType        bloom_filter_get_type   (void) G_GNUC_CONST;
BloomFilter *bloom_filter_new_full   (gsize         width,
                                      gint          key_len,
                                      guint         n_hash_funcs,
                                      BloomHashFunc first_hash_func,
                                      ...);
BloomFilter *bloom_filter_new_murmur  (gsize width,
                                       gint  key_len,
                                       guint n_hash_funcs);

BloomFilter *bloom_filter_ref        (BloomFilter *filter);
void         bloom_filter_unref      (BloomFilter *filter);

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
   BloomHashFunc hash_func;
   guint i;
   guint32 seed = 0;

   g_return_if_fail(filter);

   for (i = 0; i < filter->hash_funcs->len; i++) {
     hash_func = g_ptr_array_index(filter->hash_funcs, i);
     seed = hash_func(key, filter->key_len, seed);
     bloom_filter_set_bit(filter, seed % filter->width);
   }
}

G_INLINE_FUNC gboolean
bloom_filter_contains (BloomFilter   *filter,
                       gconstpointer  key)
{
   BloomHashFunc hash_func;
   guint i;
   guint32 seed = 0;

   g_return_val_if_fail(filter, FALSE);

   for (i = 0; i < filter->hash_funcs->len; i++) {
      hash_func = g_ptr_array_index(filter->hash_funcs, i);
      seed = hash_func(key, filter->key_len, seed);
      if (!bloom_filter_get_bit(filter, seed % filter->width)) {
         return FALSE;
      }
   }

   return TRUE;
}

G_END_DECLS

#endif /* BLOOM_FILTER_H */
