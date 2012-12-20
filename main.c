#include "bloom-filter.h"

static guint
djb_hash (gconstpointer data)
{
   const gchar *str = data;
   guint hash = 5381;
   guint i;

   for (i = 0; str[i]; i++) {
      hash = ((hash << 5) + hash) + str[i];
   }

   return hash;
}

static void
test1 (void)
{
   BloomFilter *filter;
   guint i;
   static const gchar *strings[] = {
      "abcdef",
      "ghijkl",
      "mnopqr",
      "stuvwx",
      "yz",
      "012345",
      "6789",
   };

   filter = bloom_filter_new(2048, 1, g_str_hash);

   for (i = 0; i < G_N_ELEMENTS(strings); i++) {
      g_assert(!bloom_filter_contains(filter, strings[i]));
      bloom_filter_insert(filter, strings[i]);
      g_assert(bloom_filter_contains(filter, strings[i]));
   }

   bloom_filter_remove_all(filter);

   for (i = 0; i < G_N_ELEMENTS(strings); i++) {
      g_assert(!bloom_filter_contains(filter, strings[i]));
   }

   bloom_filter_unref(filter);
}

static void
test2 (void)
{
   BloomFilter *filter;
   guint i;
   static const gchar *strings[] = {
      "abcdef",
      "ghijkl",
      "mnopqr",
      "stuvwx",
      "yz",
      "012345",
      "6789",
   };

   filter = bloom_filter_new(2048, 2, g_str_hash, djb_hash);

   for (i = 0; i < G_N_ELEMENTS(strings); i++) {
      g_assert(!bloom_filter_contains(filter, strings[i]));
      bloom_filter_insert(filter, strings[i]);
      g_assert(bloom_filter_contains(filter, strings[i]));
   }

   bloom_filter_unref(filter);
}

gint
main (gint   argc,
      gchar *argv[])
{
   g_test_init(&argc, &argv, NULL);
   g_test_add_func("/BloomFilter/g_str_hash", test1);
   g_test_add_func("/BloomFilter/g_str_hash+djb_hash", test2);
   return g_test_run();
}
