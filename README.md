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

Easy-peasy. But this does rely on keys that we can split into sub-keys that we can sort as their bit-patterns interpreted as bytes. That isn't always the case.

If we were to sort signed 64-bit integers instead, we would run into a problem. Negative numbers are represented in [two's-complement](https://en.wikipedia.org/wiki/Two's_complement) and if we sorted signed integers as if they were unsigned, the negative numbers would be sorted after the positive, as the negative numbers have the highest bit set while the positive integers have not.

Other than that, though, the negative numbers would still be correctly sorted. The negative numbers in two's compliment are still ordered such that if `i < j` then the bit patterns interpreted as unsigned integers would also be ordered `i < j`. It is only that the negative numbers come after the positive.

We can fix that by rotating the array after we have sorted it, so the negative numbers are moved to the front and the positive numbers are put after.

**FIXME: figure here**

The easiest way to rotate an array in-place (thus saving some memory) is using reversal. You can reverse the whole array, and then reverse the two parts you are rotating, and, presto, you have rotated the array.

**FIXME: figure here**

Both operations are straightforward to implement:

```c
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
```

That means that once we have sorted our signed integers as if they were unsigned, we just need to find out where the negative numbers start, and rotate around that point.

You can always just search for the first value `x[i] < 0`, but I am going to need this for more than just signed integers, so I wrote a function that checks for the first value that has bit `63` set. It works as a sign bit both in two's-complement 64-bit signed integers and in 64-bit floats.

```c
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
```

With a sorting function for unsigned 64-bit integers and the rotation functionality, the sorting routine for signed integers practically writes itself:

```c
void rsort_int64(size_t n, int64_t keys_[n])
{
    uint64_t *keys = (uint64_t *)keys_; // We actually work on uint64_t

    // Sort as unsigned
    rsort_uint64(n, keys);
    // Then rotate the negative numbers first
    rotate(keys, first_neg(keys, keys + n), keys + n);
}
```

Now that we're comfortable with the idea of sorting keys as unsigned integers and then possibly do some rearrangement after, we might wonder if we could do the same thing with floating point numbers. After all, in many bioinformatics applications we are working with floats in various disguises, and sometimes we want an efficient sort of those.

The standard representation for floating point numbers today is [IEEE 754](https://en.wikipedia.org/wiki/IEEE_754#2019). It is a bit complicated, as most standards are, especially because it allows decimal floats, which is just crazy and a lot harder to work with than binary numbers. If those decimal floats are used anywhere, I don't know, but I plan to live a long and happy life ignoring their existance.

**FIXME: description of a float representation**

**FIXME: describe why we can just sort them as bit patterns**

**FIXME: describe the transformation we need to do**

If the exponent comes before the fraction in the representation, then we sort by magnitude and then fraction if we sort floats as bit-patterns, and that means that we are almost there if we just sort our array as unsigned ints. But the sign bit will still put negative numbers after the positive numbers, so we need a rotation to get the negative numbers first and the positive numbers second. With two's-complement integers, that woudl be enough to get all the values in order, but with floats we don't have two's-complement. The sign bit is just a sign bit. So the negative numbers go

```
-0:   1 ... 00000
-1:   1 ... 00001
-2:   1 ... 00010
-3:   1 ... 00011
...
```

which, when sorted as bit-patterns, would put the negative numbers in reverse.

No problem, though, we have a function for reversing parts of an array and we are not afraid to use it.

To sum up, if we sort 64-bit floating point numbers in IEEE 754, we can sort them as 64-bit unsigned integers, but then we need to rotate the negative numbers up front to get the sign ordered, and then we need to reverse the negative numbers to get them in order.[^1]

```c
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
```

A word of warning on casting is in order her. I am casting an array of `float64_t` to an array of `uint64_t`, which means that I tell C to consider that array of a different type. That means that whatever bits are in the original array are also in the new array--because it is the same array--they are just interpreted in a different way.

This is what I want here, but it is not always what a cast will give me. And for good reasons.

**FIXME: more here**


[^1]: If you are sorting more complicated values, and the floats are just keys, this last reversal will be unstable. You are reversing the values compared to the order they had in the input. To fix this, you will have to run through the negative numbers and for each block of identical keys you need to reverse the block. That brings the algorithm back to a stable sort.
