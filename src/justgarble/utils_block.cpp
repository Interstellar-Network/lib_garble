//
// Created by nathanprat on 5/12/16.
//

#include "utils_block.h"

#include <absl/strings/str_cat.h>
#include <glog/logging.h>

#include "block.h"

// TODO move into rand_sse, and make sure it is thread safe !
// static block cur_seed;
alignas(16) static __m128i cur_seed;

void srand_sse(unsigned int seed) {
  cur_seed = _mm_set_epi32(seed, seed + 1, seed, seed + 1);
  DLOG(INFO) << "srand_sse : seed : " << seed << " " << seed + 1 << " " << seed
             << " " << seed + 1;
}

/*
    Apparently based on code by Intel
    https://software.intel.com/en-us/articles/fast-random-number-generator-on-the-intel-pentiumr-4-processor
*/
// TODO replace by secure PRNG(probably AES)
#if 0
block randomBlock() {

    static const unsigned int mult[4] = { 214013, 17405, 214013, 69069 };
    static const unsigned int gadd[4] = { 2531011, 10395331, 13737667, 1 };
    static const unsigned int mask[4] = { 0xFFFFFFFF, 0, 0xFFFFFFFF, 0 };
    // static const unsigned int masklo[4] = { 0x00007FFF, 0x00007FFF, 0x00007FFF,
    //                                         0x00007FFF };

    __m128i adder = _mm_load_si128((__m128i *) gadd);
    __m128i multiplier = _mm_load_si128((__m128i *) mult);
    __m128i mod_mask = _mm_load_si128((__m128i *) mask);
    // block sra_mask = _mm_load_si128((block *) masklo);
    __m128i cur_seed_split = _mm_shuffle_epi32(cur_seed.val, _MM_SHUFFLE(2, 3, 0, 1));
    cur_seed.val = _mm_mul_epu32(cur_seed.val, multiplier);
    multiplier = _mm_shuffle_epi32(multiplier, _MM_SHUFFLE(2, 3, 0, 1));
    cur_seed_split = _mm_mul_epu32(cur_seed_split, multiplier);
    cur_seed.val = _mm_and_si128(cur_seed.val, mod_mask);
    cur_seed_split = _mm_and_si128(cur_seed_split, mod_mask);
    cur_seed_split = _mm_shuffle_epi32(cur_seed_split, _MM_SHUFFLE(2, 3, 0, 1));
    cur_seed.val = _mm_or_si128(cur_seed.val, cur_seed_split);
    cur_seed.val = _mm_add_epi32(cur_seed.val, adder);

    return cur_seed;

}
#endif

__m128i rand_sse() {
  alignas(16) __m128i result;
  alignas(16) __m128i cur_seed_split;
  alignas(16) __m128i multiplier;
  alignas(16) __m128i adder;
  alignas(16) __m128i mod_mask;
  alignas(16) static const unsigned int mult[4] = {214013, 17405, 214013,
                                                   69069};
  alignas(16) static const unsigned int gadd[4] = {2531011, 10395331, 13737667,
                                                   1};
  alignas(16) static const unsigned int mask[4] = {0xFFFFFFFF, 0, 0xFFFFFFFF,
                                                   0};

  adder = _mm_load_si128(reinterpret_cast<const __m128i*>(gadd));
  multiplier = _mm_load_si128(reinterpret_cast<const __m128i*>(mult));
  mod_mask = _mm_load_si128(reinterpret_cast<const __m128i*>(mask));
  cur_seed_split = _mm_shuffle_epi32(cur_seed, _MM_SHUFFLE(2, 3, 0, 1));

  // NOLINTNEXTLINE(portability-simd-intrinsics)
  cur_seed = _mm_mul_epu32(cur_seed, multiplier);
  multiplier = _mm_shuffle_epi32(multiplier, _MM_SHUFFLE(2, 3, 0, 1));
  // NOLINTNEXTLINE(portability-simd-intrinsics)
  cur_seed_split = _mm_mul_epu32(cur_seed_split, multiplier);

  cur_seed = _mm_and_si128(cur_seed, mod_mask);
  cur_seed_split = _mm_and_si128(cur_seed_split, mod_mask);
  cur_seed_split = _mm_shuffle_epi32(cur_seed_split, _MM_SHUFFLE(2, 3, 0, 1));
  cur_seed = _mm_or_si128(cur_seed, cur_seed_split);
  // NOLINTNEXTLINE(portability-simd-intrinsics)
  cur_seed = _mm_add_epi32(cur_seed, adder);

  // _mm_storeu_si128( (__m128i*) result, cur_seed);
  _mm_storeu_si128(&result, cur_seed);

  return result;
}
