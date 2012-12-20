# Bloom-GLib

This is just a quick attempt at writing a bloom filter. I don't even know if it
is correct, but it seems to work for my very trivial test program. If you are
interested in using it, I highly suggest you verify its correctness.

Bloom filters can give false-positives, but never false-negatives. You use it
by inserting the hash of the data you are interested in. The bits are stored
for those hashes. You can check to see if an item exists if all of the bits are
on. You can see how if there are collisions in your hashes that it might say
that something exists even when it wasn't actually inserted. This is the
false-positive outcome.

## Example

```c
BloomFilter *filter;

filter = bloom_filter_new(2048 /* bits */,
                          1,   /* Number of following hash funcs */
                          g_str_hash);

/* populate your bloom filter */
bloom_filter_insert(filter, "some key");

/* ... */

if (bloom_filter_contains(filter, "some key")) {
    /* do some long running work */
}

bloom_filter_unref(filter);
```
