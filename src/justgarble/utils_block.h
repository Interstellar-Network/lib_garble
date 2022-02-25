/*
    Those functions were included in JustGarble, but are in fact based on
    https://software.intel.com/en-us/articles/fast-random-number-generator-on-the-intel-pentiumr-4-processor
*/

#ifndef UTIL_H_
#define UTIL_H_

#include <emmintrin.h>

__m128i rand_sse();

void srand_sse(unsigned int seed);

#endif /* UTIL_H_ */
