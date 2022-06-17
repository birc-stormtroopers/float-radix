#include "radix.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>

static void test_rsort_uint(void)
{
    size_t n = 10;
    uint64_t keys[n];

    for (int rep = 0; rep < 5; rep++)
    {
        for (size_t i = 0; i < n; i++)
            keys[i] = rand();
        rsort_uint64(n, keys);
        for (size_t i = 1; i < n; i++)
        {
            assert(keys[i - 1] <= keys[i]);
        }
    }
}

int rsign(void)
{
    return (rand() & 0x01) ? -1 : 1;
}

static void test_rsort_int(void)
{
    size_t n = 10;
    int64_t keys[n];

    for (int rep = 0; rep < 5; rep++)
    {
        for (size_t i = 0; i < n; i++)
            keys[i] = rsign() * rand();

        rsort_int64(n, keys);

        for (size_t i = 1; i < n; i++)
        {
            assert(keys[i - 1] <= keys[i]);
        }
    }
}

// Draw 64 bits at random. Not a uniform distribution on the
// floats, but good enough for testing.
static uint64_t rand_bits(void)
{
    uint64_t r = 0;
    for (int i = 0; i < 64; i++)
    {
        r <<= 1;
        r |= rand() & 0x01;
    }
    return r;
}

// Direct cast doesn't preserve the bit patterns but would cast
// 42 <-> 42.0 which isn't much use in these hacks.
static inline float64_t cast_itof(uint64_t i)
{
    return *(float64_t *)&i;
}

// Sample a random floating point number from a random
// bit pattern.
static float64_t rand_float(void)
{
    return cast_itof(rand_bits());
}

static void test_rsort_float(void)
{
    size_t m = 5, k = 3;
    size_t n = k * m + 3; // +2 for +/- inf + 1 for nan
    float64_t keys[n];

    for (int rep = 0; rep < 5; rep++)
    {
        keys[0] = -nan(0);
        for (size_t i = 1; i < m; i++)
            keys[0 * m + i] = rand_float();

        keys[1 * m] = 0.0; // get zero in there
        for (size_t i = 1; i < m; i++)
            keys[1 * m + i] = rand_float();

        keys[2 * m] = -0.0; // get negative zero in there
        for (size_t i = 1; i < m; i++)
            keys[2 * m + i] = rand_float();

        keys[n - 3] = INFINITY;
        keys[n - 2] = -INFINITY;
        keys[n - 1] = nan(0);

        rsort_float64(n, keys);

        for (size_t i = 0; i < n; i++)
        {
            printf("%lg\n", keys[i]);
        }

        // The "negative" nan should go first
        assert(isnan(keys[0]));
        for (size_t i = 2; i < n - 2; i++)
        {
            assert(keys[i - 1] <= keys[i]);
        }
        // The positive last
        assert(isnan(keys[n - 1]));
    }
}

int main(void)
{
    test_rsort_uint();
    test_rsort_int();
    test_rsort_float();
    return 0;
}
