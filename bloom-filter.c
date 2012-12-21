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
#include <string.h>

#include "bloom-filter.h"

/*
 * MurmurHash3, written by Austin Appleby, and placed in the public
 * domain.
 *
 * http://code.google.com/p/smhasher/source/browse/trunk/MurmurHash3.cpp
 *
 * Extracted MurmurHash3_x86_32 (without the MSVC specifics), changed types to
 * glib types, gave it a return value, assume 0-terminated if len == -1
 */

static inline guint32
rotl32 (guint32 x, gint8 r)
{
  return (x << r) | (x >> (32 - r));
}

#define ROTL32(x,y) rotl32(x,y)

/*
 * Block read - if your platform needs to do endian-swapping or can only
 * handle aligned reads, do the conversion here
 *
 * NB: original uses __attribute__((always_inline)), but this generates a
 *     warning with gcc, and since -Werror is on ...
 */
static inline guint32
getblock (const guint32 * p, int i)
{
  return p[i];
}

/*
 * Finalization mix - force all bits of a hash block to avalanche
 *
 * NB: original uses __attribute__((always_inline)), but this generates a
 *     warning with gcc, and since -Werror is on ...
 */
static inline guint32
fmix (guint32 h)
{
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;

  return h;
}

static guint32
murmur_hash3_x86_32 (gconstpointer key, int len, guint32 seed)
{
  const guint8 *data = (const guint8 *)key;
  const guint8 *tail;
  const guint32 c1 = 0xcc9e2d51;
  const guint32 c2 = 0x1b873593;
  const guint32 *blocks;
  int i, nblocks;
  guint32 h1 = seed;
  guint32 k1;

  if (len < 0)
    len = strlen ((const char*)key);

  nblocks = len / 4;
  blocks = (const guint32 *)(data + nblocks*4);

  for(i = -nblocks; i; i++)
  {
    k1 = getblock(blocks,i);

    k1 *= c1;
    k1 = ROTL32(k1,15);
    k1 *= c2;

    h1 ^= k1;
    h1 = ROTL32(h1,13);
    h1 = h1*5+0xe6546b64;
  }

  tail = (const guint8*)(data + nblocks*4);
  k1 = 0;

  switch(len & 3)
    {
    case 3: k1 ^= tail[2] << 16;
    case 2: k1 ^= tail[1] << 8;
    case 1: k1 ^= tail[0];
      k1 *= c1; k1 = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
    };

  h1 ^= len;

  h1 = fmix(h1);

  return h1;
}

BloomFilter *
bloom_filter_new_full (gsize     width,
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

void
bloom_filter_remove_all (BloomFilter *filter)
{
   g_return_if_fail(filter);
   memset(filter->data, 0, (filter->width / CHAR_BIT) + 1);
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
