#include "radix.h"
#include <stdio.h>
#include <string.h>

// Flip two values.
// If your compiler doesn't support __typeof__, then add
// a type parameter to the macro.
#define swap(ptr1, ptr2)             \
    do                               \
    {                                \
        __typeof__(ptr1) tmp = ptr1; \
        ptr1 = ptr2;                 \
        ptr2 = tmp;                  \
    } while (0)

// Helper functions. They all work on uint64_t, but you can manipulate
// other types with them.

// Reverse the values in [from, to).
static void reverse(uint64_t *from, uint64_t *to)
{
    for (to--; from < to; from++, to--)
    {
        swap(*from, *to);
    }
}

// Rotate [from, p)[p, to) to [p, to)[from, to)
static void rotate(uint64_t *from, uint64_t *p, uint64_t *to)
{
    //  from    p
    //  [.....][...............]
    //  [...............][.....]
    //                   q
    uint64_t *q = to - (p - from);
    reverse(from, to);
    reverse(from, q);
    reverse(q, to);
}

// Find the first negative number in [from,to), where
// negative means that the "sign bit" (bit 63) is set.
// If no negative number is found, return to.
#define SIGN_BIT(WORD) ((WORD >> 63) & 0x01)
static uint64_t *first_neg(uint64_t *from, uint64_t *to)
{
    for (; from != to; from++)
        if (SIGN_BIT(*from))
            break;
    return from;
}

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

void rsort_int64(size_t n, int64_t keys_[n])
{
    uint64_t *keys = (uint64_t *)keys_; // We actually work on uint64_t

    // Sort as unsigned
    rsort_uint64(n, keys);
    // Then rotate the negative numbers first
    rotate(keys, first_neg(keys, keys + n), keys + n);
}

void rsort_float64(size_t n, float64_t keys_[n])
{
    // Just sort as integers. This requires that the representation is big-endian
    // so the bytes are in the same order as IEEE 754.
    uint64_t *keys = (uint64_t *)keys_; // We actually work on uint64_t
    rsort_uint64(n, keys);

    // Then rotate to get the negative/positive numbers in the right order
    size_t no_pos = first_neg(keys, keys + n) - keys;
    size_t no_neg = n - no_pos;
    rotate(keys, keys + no_pos, keys + n);
    // and reverse the negative numbers (they are not two's compliment so
    // they are bit-wise sorted and thus in the wrong order).
    reverse(keys, keys + no_neg);
}
