#include "radix.h"
#include <assert.h>
#include <stdio.h>

static void test_rsort_int(void)
{
    size_t n = 10;
    uint64_t keys[n];

    for (int rep = 0; rep < 5; rep++)
    {
        for (size_t i = 0; i < n; i++)
            keys[i] = rand();
        rsort_int64(n, keys);
        for (size_t i = 0; i < n; i++)
        {
            printf("%llu\n", keys[i]);
        }
        printf("\n");
        for (size_t i = 1; i < n; i++)
        {
            assert(keys[i - 1] <= keys[i]);
        }
    }
}

static void test_rsort_float(void)
{
    size_t n = 10;
    float64_t keys[n];

    for (int rep = 0; rep < 5; rep++)
    {
        for (size_t i = 0; i < n; i++)
            keys[i] = (double)rand() / RAND_MAX; // Random in [0,1)

        rsort_float64(n, keys);

        for (size_t i = 0; i < n; i++)
        {
            printf("%lf\n", keys[i]);
        }
        printf("\n");

        for (size_t i = 1; i < n; i++)
        {
            assert(keys[i - 1] <= keys[i]);
        }
    }
}

int main(void)
{
    test_rsort_int();
    test_rsort_float();
    return 0;
}
