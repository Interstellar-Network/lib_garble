/*
 This file is part of JustGarble.

    JustGarble is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    JustGarble is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with JustGarble.  If not, see <http://www.gnu.org/licenses/>.

*/

#pragma once

#include <emmintrin.h>
#include <immintrin.h>
#include <smmintrin.h>

#include <cassert>
#include <ostream>

#include "utils_block.h"

/**
 * Historically a simple "__m128i Block"
 * But it's better to have a proper class(operators overload, etc)
 *
 * NOTE: the original impl was directly using a __m128i field.
 * This is SLOWER than using 2 uint64! Probably due to the fact that the m128i
 * field is copied everywhere when XORing etc
 * See bench/bench_xor_Block.cpp for comparison.
 *
 * WARNING: Those functions REALLY need to be inline:
 * circuit_display_gen_bench [1000], avg of 5 runs:
 * inline:                412k circuits/h
 * out of line(in .cpp):  385k circuits/h
 */
// TODO noncopyable
// TODO use absl int128.h
class Block {
 public:
  Block() {}

  Block(uint64_t high, uint64_t low) : low_(low), high_(high) {}

  /**
   * XorBlocks: XOR two elem & store in-place
   */
  void Xor(const Block &x, const Block &y) {
    high_ = x.high_ ^ y.high_;
    low_ = x.low_ ^ y.low_;
  }

  /**
   * XorBlocks: XOR self with x, and return; const so self not modified
   */
  Block Xor(const Block &other) const {
    return Block(high_ ^ other.high_, low_ ^ other.low_);
  }

  /**
   * Replaces getFromBlock(__m128i, 0/1)
   *
   * getHigh: return [127:64]
   * getLow: return [64:0]
   */
  uint64_t GetHigh() const { return high_; }

  uint64_t GetLow() const { return low_; }

  unsigned char GetLsb() const {
    // CHECK: we assume Block are [low, high]; we means &block == &block.low
    assert((GetLow() & 1) == (*((unsigned short *)this) & 1) &&
           "Block: wrong memory layout!");
    return GetLow() & 1;
  }

  /**
   * In place AES, with SSE intrinsics
   */
  // TODO why "sched2"???
  void Aes(unsigned int rnds, const __m128i *sched, const __m128i *sched2) {
    // convert ourself into a 128bits, to be used by AES-NI
    __m128i val = _mm_set_epi64x(high_, low_);

    // BEGIN standard AES
    val = _mm_xor_si128(val, sched[0]);

    for (unsigned int j = 1; j < rnds; j++) {
      val = _mm_aesenc_si128(val, sched2[j]);
    }

    val = _mm_aesenclast_si128(val, sched[rnds]);
    // END standard AES

    // finally, copy the AES result back into ourself
    high_ = _mm_extract_epi64(val, 1);
    low_ = _mm_extract_epi64(val, 0);
  }

  void Aes(unsigned int rnds, const __m128i *sched) {
    // convert ourself into a 128bits, to be used by AES-NI
    __m128i val = _mm_set_epi64x(high_, low_);

    // BEGIN standard AES
    val = _mm_xor_si128(val, sched[0]);

    for (unsigned int j = 1; j < rnds; j++) {
      val = _mm_aesenc_si128(val, sched[j]);
    }

    val = _mm_aesenclast_si128(val, sched[rnds]);
    // END standard AES

    // finally, copy the AES result back into ourself
    high_ = _mm_extract_epi64(val, 1);
    low_ = _mm_extract_epi64(val, 0);
  }

  /**
   * Double 'b' and store in ourself
   */
  void Double(const Block &b) {
    high_ = b.high_ * 2;
    low_ = b.low_ * 2;
  }

  /**
   * Quadruple 'b' and store in ourself
   * Same as calling double twice.
   */
  void Quadruple(const Block &b) {
    high_ = b.high_ * 4;
    low_ = b.low_ * 4;
  }

  /**
   * Zero ourself
   * Basically the same as: blk = Block(0, 0)
   */
  void Zero() {
    high_ = 0;
    low_ = 0;
  }

  /**
   * Randomize the Block in-place
   * Currently using intel's rand_sse
   */
  void Randomize() {
    __m128i val = rand_sse();

    // finally, copy the result back into ourself
    high_ = _mm_extract_epi64(val, 1);
    low_ = _mm_extract_epi64(val, 0);
  }

  /**
   * Create a new Randomize Block
   */
  static Block NewRandom() {
    Block b;
    b.Randomize();
    return b;
  }

  /**
   * Like NewRandom, but with a tweak
   */
  // TODO find out why it is needed, and document!
  static Block NewR() {
    Block b = NewRandom();
    *(reinterpret_cast<char *>(&b.low_)) = 1;
    return b;
  }

  // Needed for Boost test
  // And also mapOutputs [dev only]
  bool operator==(const Block &other) const {
    return high_ == other.high_ && low_ == other.low_;
  }

  friend std::ostream &operator<<(std::ostream &os, const Block &other) {
    os << "(" << other.low_ << "," << other.high_ << ")";
    return os;
  }

 private:
  // the order is relevant for NewR() & GetLsb() !
  // And also for Aes() b/c of the intermediate m128i.
  uint64_t low_;
  uint64_t high_;
};
