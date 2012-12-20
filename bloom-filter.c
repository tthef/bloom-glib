/* bloom-filter.c
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

#include <limits.h>

#include "bloom-filter.h"

BloomFilter *
bloom_filter_new (gsize     width,
                  guint     n_hash_funcs,
                  GHashFunc first_hash_func,
                  ...)
{
   BloomFilter *filter;
   va_list args;
   guint i;

   g_return_val_if_fail(width, NULL);
   g_return_val_if_fail(n_hash_funcs, NULL);
   g_return_val_if_fail(first_hash_func, NULL);

   filter = g_malloc0(sizeof *filter + (width / CHAR_BIT) + 1);
   filter->ref_count = 1;
   filter->width = width;
   filter->hash_funcs = g_ptr_array_new();
   g_ptr_array_add(filter->hash_funcs, first_hash_func);

   va_start(args, first_hash_func);
   for (i = 1; i < n_hash_funcs; i++) {
      g_ptr_array_add(filter->hash_funcs, va_arg(args, GHashFunc));
   }
   va_end(args);

   return filter;
}

BloomFilter *
bloom_filter_ref (BloomFilter *filter)
{
   g_return_val_if_fail(filter, NULL);
   g_return_val_if_fail(filter->ref_count > 0, NULL);
   g_atomic_int_inc(&filter->ref_count);
   return filter;
}

void
bloom_filter_unref (BloomFilter *filter)
{
   g_return_if_fail(filter);
   g_return_if_fail(filter->ref_count > 0);

   if (g_atomic_int_dec_and_test(&filter->ref_count)) {
      g_ptr_array_unref(filter->hash_funcs);
      g_free(filter);
   }
}

GType
bloom_filter_get_type (void)
{
   static volatile GType type_id;

   if (g_once_init_enter(&type_id)) {
      GType gtype;
      gtype = g_boxed_type_register_static("BloomFilter",
                                           (GBoxedCopyFunc)bloom_filter_ref,
                                           (GBoxedFreeFunc)bloom_filter_unref);
      g_once_init_leave(&type_id, gtype);
   }

   return type_id;
}
