# Radix sorting signed integers and floats

We know how to [bucket sort](https://github.com/birc-stormtroopers/bucket-sort) which means we also know how to radix sort. That's because to radix sort, we just need to iteratively bucket sort through a key.

You can do this in two directions, from left to right or right to left (also known as *most-significant digit first* (MSD) or *least-significant digit first* (LSD)). [Here's some description from a GSA exercise, but without the code (it is an exercise, after all)](https://github.com/birc-gsa/radix-sa).

I find that the least-significant digit first radix sort is the easiest, because it really just is repeating a bucket sort some $k$ times. If you have a key with `w` bits, you can split it into `k` sub-keys of size `w/k` and then bucket sort from right to left `k` times.

Say your keys are 64 bit integers. We can't bucket sort these directly, since we would need $2^{64} \approx 1.84\times 10^{19}$ buckets, and we don't have that kind of memory on our computers. But 64 bits is just 8 bytes of 8 bits, and sorting bits require only 256 buckets. If we split our keys into eight sub-keys, we can bucket sort with 256 buckets and be done after eight bucket sorts.

A C implementation of this could look like this:

```c
// How many bits do we sort at a time
// and the mask we use to extract them
#define BUCKET_BITS 8
#define BUCKET_MASK 0xFF

// The number of buckets we need for that
#define NO_BUCKETS (1 << BUCKET_BITS)

// And the number of keys that leaves in a word
#define NO_KEYS(WSIZE) (WSIZE / BUCKET_BITS)

// Give us the K'th sub-key for a key
#define BUCKET_KEY(KEY, K, WSIZE) \
    (((KEY) >> (K * BUCKET_BITS)) & BUCKET_MASK)

// Sort n keys into buf using the sub-key k. Buckets is a helper
// buffer that the caller must allocate.
void bsort(size_t n, uint64_t keys[n], uint64_t buf[n],
           int k, size_t buckets[NO_BUCKETS])
{
    // Count the keys
    memset(buckets, 0, NO_BUCKETS * sizeof *buckets);
    for (size_t i = 0; i < n; i++)
        buckets[BUCKET_KEY(keys[i], k, 64)]++;

    // Then compute the cumulative sum
    unsigned int acc = 0, b;
    for (size_t i = 0; i < NO_BUCKETS; i++)
    {
        b = buckets[i];
        buckets[i] = acc;
        acc += b;
    }

    // Place the keys into buf.
    for (size_t i = 0; i < n; i++)
        buf[buckets[BUCKET_KEY(keys[i], k, 64)]++] = keys[i];
}

void rsort_uint64(size_t n, uint64_t keys[n])
{
    uint64_t *buf = malloc(n * sizeof *buf);
    size_t *buckets = malloc(NO_BUCKETS * sizeof *buckets);

    // It's important that we run an even number of times, but we
    // will for reasonable choices of word sizes and bucket sizes.
    uint64_t **b1 = &keys, **b2 = &buf;
    for (int b = 0; b < NO_KEYS(64); b++)
    {
        bsort(n, *b1, *b2, b, buckets);
        swap(b1, b2);
    }

    free(buckets);
    free(buf);
}
```

