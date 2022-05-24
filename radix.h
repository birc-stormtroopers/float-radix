#ifndef RADIX_H
#define RADIX_H

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

// this is a bit risky, but it is typically true
typedef double float64_t;
_Static_assert(sizeof(uint64_t) == sizeof(float64_t),
               "These should have the same size.");

void rsort_int64(size_t n, uint64_t keys[n]);
void rsort_float64(size_t n, float64_t keys[n]);

#endif // RADIX_H
