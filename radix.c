#include "radix.h"
#include <stdio.h>
#include <string.h>

#define BITS 8
#define NO_KEYS (1 << 8)
#define NO_BUCKETS(WSIZE) (WSIZE / BITS)
#define BUCKET_MASK 0xFF
#define BUCKET_KEY(KEY, BUCKET, WSIZE) \
    ((KEY >> (BUCKET * BITS)) & BUCKET_MASK)

void bsort(size_t n, uint64_t keys[n], uint64_t buf[n],
           int bucket, size_t buckets[NO_KEYS])
{
    // Count the keys
    memset(buckets, 0, NO_KEYS * sizeof *buckets);
    for (size_t i = 0; i < n; i++)
        buckets[BUCKET_KEY(keys[i], bucket, 64)]++;

    // Then compute the cumulative sum
    unsigned int acc = 0, b;
    for (size_t i = 0; i < NO_KEYS; i++)
    {
        b = buckets[i];
        buckets[i] = acc;
        acc += b;
    }

    // Place the keys
    for (size_t i = 0; i < n; i++)
        buf[buckets[BUCKET_KEY(keys[i], bucket, 64)]++] = keys[i];
}

#define swap(ptr1, ptr2)  \
    do                    \
    {                     \
        void *tmp = ptr1; \
        ptr1 = ptr2;      \
        ptr2 = tmp;       \
    } while (0)

void rsort_int64(size_t n, uint64_t keys[n])
{
    uint64_t *buf = malloc(n * sizeof *buf);
    size_t *buckets = malloc(NO_KEYS * sizeof *buckets);

    // It's important that we run an even number of times, but we
    // will for reasonable choices of word sizes and bucket sizes.
    uint64_t **b1 = &keys, **b2 = &buf;
    for (int b = 0; b < NO_BUCKETS(64); b++)
    {
        bsort(n, *b1, *b2, b, buckets);
        swap(b1, b2);
    }

    free(buckets);
    free(buf);
}

void rsort_float64(size_t n, float64_t keys[n])
{
    // Just sort as integers
    rsort_int64(n, (uint64_t *)keys);
}
