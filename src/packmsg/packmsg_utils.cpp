// lib_garble
// Copyright (C) 2O22  Nathan Prat

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "packmsg_utils.h"

#include <glog/logging.h>

namespace {

// map: 0-9 -> 7 segments
// NOTE: the order must match the one used in SegmentedDigit constructor
// clang-format off
static const std::vector<std::vector<uint8_t>> kMapDigitTo7Segs = {
  // 0: all ON, except middle one(horizontal)
  {   1,
    1, 1,
      0,
    1, 1,
      1
  },
  // 1: only the 2 rightmost segments
  {   0,
    0, 1,
      0,
    0, 1,
      0
  },
  // 2
  {   1,
    0, 1,
      1,
    1, 0,
      1
  },
  // 3
  {   1,
    0, 1,
      1,
    0, 1,
      1
  },
  // 4
  {   0,
    1, 1,
      1,
    0, 1,
      0
  },
  // 5
  {   1,
    1, 0,
      1,
    0, 1,
      1
  },
  // 6
  {   1,
    1, 0,
      1,
    1, 1,
      1
  },
  // 7
  {   1,
    0, 1,
      0,
    0, 1,
      0
  },
  // 8
  {   1,
    1, 1,
      1,
    1, 1,
      1
  },
  // 9
  {   1,
    1, 1,
      1,
    0, 1,
      1
  }
  };
// clang-format on

// I Ching map: 0-63 -> 18 segments
// TODO(iching) currently harcoded based on the sample Skeyecards
static const std::vector<std::vector<uint8_t>> kMapIchingToSegs = {
    {1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1},
    {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1},
    {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1},
    {1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1},
    {1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1}};

/**
 * Map a I Ching(ie a number between 0 and 9) to a mask.
 * There are only 64 I-Chings, but as we use them to display OTP(ie digits), we
 * only need to map 0-9.
 */
std::vector<uint8_t> IChingToSegments(const std::vector<uint8_t> &digits) {
  assert(!digits.empty());

  std::vector<uint8_t> res;

  res.reserve(digits.size() * 18);

  for (auto digit : digits) {
    assert(digit <= 9);  // // digit is unsigned so already >= 0

    auto &segs = kMapIchingToSegs.at(digit);
    assert(segs.size() == 18);

    res.insert(res.end(), segs.begin(), segs.end());
  }

  assert(res.size() == 18 * digits.size());  // 18 segments

  return res;
}

// TODO should be passed by python/the webserver, and be per-user
// clang-format off
static const std::vector<std::vector<uint8_t>> kMapDigitTo14Segs = {
    // A
    {
      1,
      1,0,0,0,1,
      1,1,
      1,0,0,0,1,
      0
    },
    // B
    {
      1,
      0,0,1,0,1,
      0,1,
      0,0,1,0,1,
      1
    },
    // C
    {
      1,
      1,0,0,0,0,
      0,0,
      1,0,0,0,0,
      1
    },
    // D
    {
      1,
      0,0,1,0,1,
      0,0,
      0,0,1,0,1,
      1
    },
    // E
    {
      1,
      1,0,0,0,0,
      1,0,
      1,0,0,0,0,
      1
    },
    // F
    {
      1,
      1,0,0,0,0,
      1,0,
      1,0,0,0,0,
      0
    },
    // G
    {
      1,
      1,0,0,0,0,
      0,1,
      1,0,0,0,1,
      1
    },
    // H
    {
      0,
      1,0,0,0,1,
      1,1,
      1,0,0,0,1,
      0
    },
    // I
    {
      1,
      0,0,1,0,0,
      0,0,
      0,0,1,0,0,
      1
    },
    // J
    {
      0,
      0,0,0,0,1,
      0,0,
      1,0,0,0,1,
      1
    },
    // K
    {
      0,
      1,0,0,1,0,
      1,0,
      1,0,0,1,0,
      0
    },
    // L
    {
      0,
      1,0,0,0,0,
      0,0,
      1,0,0,0,0,
      1
    },
    // M
    {
      0,
      1,1,0,1,1,
      0,0,
      1,0,0,0,1,
      0
    },
    // N
    {
      0,
      1,1,0,0,1,
      0,0,
      1,0,0,1,1,
      0
    },
    // O
    {
      1,
      1,0,0,0,1,
      0,0,
      1,0,0,0,1,
      1
    },
    // P
    {
      1,
      1,0,0,0,1,
      1,1,
      1,0,0,0,0,
      0
    },
    // Q
    {
      1,
      1,0,0,0,1,
      0,0,
      1,0,0,1,1,
      1
    },
    // R
    {
      1,
      1,0,0,0,1,
      1,1,
      1,0,0,1,0,
      0
    },
    // S
    {
      1,
      0,1,0,0,0,
      0,1,
      0,0,0,0,1,
      1
    },
    // T
    {
      1,
      0,0,1,0,0,
      0,0,
      0,0,1,0,0,
      0
    },
    // U
    {
      0,
      1,0,0,0,1,
      0,0,
      1,0,0,0,1,
      1
    },
    // V
    {
      0,
      1,0,0,1,0,
      0,0,
      1,1,0,0,0,
      0
    },
    // W
    {
      0,
      1,0,0,0,1,
      0,0,
      1,1,0,1,1,
      0
    },
    // X
    {
      0,
      0,1,0,1,0,
      0,0,
      0,1,0,1,0,
      0
    },
    // Y
    {
      0,
      0,1,0,1,0,
      0,0,
      0,0,1,0,0,
      0
    },
    // Z
    {
      1,
      0,0,0,1,0,
      0,0,
      0,1,0,0,0,
      1
    },
    };
// clang-format on

/**
 * Map a digit to a Letter, same principle as the I-ching.
 * Letters are displayed as a 14 segments display.
 *
 * digits_map: per user map of the permutation, typically printed on the
 * Skeyecard
 * eg: 0 = B, 1 = X, 2 = R, ...
 */
std::vector<uint8_t> DigitTo14Segments(const std::vector<uint8_t> &digits,
                                       const std::vector<uint8_t> &digits_map) {
  assert(!digits.empty());
  assert(digits_map.size() == 10);

  std::vector<uint8_t> res;

  res.reserve(digits.size() * 14);

  for (auto digit : digits) {
    assert(digit <= 9);  // // digit is unsigned so already >= 0

    auto &segs = kMapDigitTo14Segs.at(digits_map.at(digit));
    assert(segs.size() == 14);

    res.insert(res.end(), segs.begin(), segs.end());
  }

  assert(res.size() == 14 * digits.size());  // 14 segments

  return res;
}

std::vector<uint8_t> DigitsTo7Segments(const std::vector<uint8_t> &digits) {
  assert(!digits.empty());

  std::vector<uint8_t> res;

  res.reserve(digits.size() * 7);

  for (auto digit : digits) {
    assert(digit <= 9);  // digit is unsigned so already >= 0

    auto &segs = kMapDigitTo7Segs.at(digit);
    assert(segs.size() == 7);

    res.insert(res.end(), segs.begin(), segs.end());
  }

  assert(res.size() == 7 * digits.size());  // 7 segments

  return res;
}

}  // anonymous namespace

/**
 * XOR two vect element by element
 */
std::vector<uint8_t> interstellar::packmsg::internal::XorBits(
    const std::vector<uint8_t> &bits1, const std::vector<uint8_t> &bits2) {
  assert(bits1.size() == bits2.size());

  size_t input_size = bits1.size();

  std::vector<uint8_t> result;

  result.resize(input_size);

  std::transform(bits1.begin(), bits1.end(), bits2.begin(), result.begin(),
                 std::bit_xor<>());

  return result;
}

// TODO remove pack_bits64
std::vector<uint64_t> interstellar::packmsg::internal::pack_bits64(
    std::vector<uint8_t> &bits) {
  std::vector<uint64_t> words;

  uint64_t word = 0;

  unsigned int i = 0;

  for (auto bit : bits) {
    // IMPORTANT: must cast bit to uint64_t or it will overflow
    word |= static_cast<uint64_t>(bit) << i;

    i++;
    if (i == 64) {
      words.push_back(word);
      i = 0;
      word = 0;
    }
  }

  // Don't forget the last one
  if (i > 0) {
    words.push_back(word);
  }

  // TODO
  // words.push_back(496592044773567255ull);
  // words.push_back(11101543203018415375ull);
  // words.push_back(8746037068609082586ull);
  // words.push_back(59ull);

  return words;
}

/**
 * lib_python: def _prepare_input_labels(digits, config, garble_wrapper)
 *
 */
std::vector<Block> interstellar::packmsg::internal::PrepareInputLabels(
    const garble::ParallelGarbledCircuit &pgc,
    const std::vector<uint8_t> &digits,
    PackmsgDigitSegmentsType digit_seg_type) {
  const size_t nb_digits = digits.size();

  // TODO TOREMOVE rndsize/rndfirst?
  const size_t rndsize = pgc.config_.at("RNDSIZE");

  // assert(pgc.n - rndsize - 1 == nb_digits * 7); // pgc.n counts
  // both inputs and random (+buf)

  // Handle a "no otp" message
  if (nb_digits == 0u) {
    return std::vector<Block>();
  }

  // Two options
  // - historic one: 7 segments: pass to DigitsToSegments and use the digit ->
  // segment map
  // - new version with digits that can be forms(square, triangle, etc):
  // inputbits is just "full 1"
  // TODO add a config_ variable to switch between 7 SEG and new version, and
  // remove the mod 7 check
  std::vector<uint8_t> inputbits;
  // TODO TOREMOVE rndsize/rndfirst?
  // unsigned int bitmap_nb_segments = pgc.config_.at("BITMAP_NB_SEGMENTS");

  // 7 for the classic digit(7 SEG display), 18 for the iching
  // TODO TOREMOVE rndsize/rndfirst?
  // unsigned int segments_per_digit =
  // bitmap_nb_segments / pgc.config_.at("MSGSIZE");

  switch (digit_seg_type) {
    case PackmsgDigitSegmentsType::seven_segs: {
      inputbits = DigitsTo7Segments(digits);
    } break;
    case PackmsgDigitSegmentsType::fourteen_segs: {
      // TODO get from args(pass from DB/python to cpp, on a per-user basis)
      std::vector<uint8_t> user_map = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
      inputbits = DigitTo14Segments(digits, user_map);
    } break;
    case PackmsgDigitSegmentsType::iching: {
      inputbits = IChingToSegments(digits);
    } break;
  }

  // TODO TOREMOVE rndsize/rndfirst?
  // DO NOT move, it's here to save the size of the inputs BEFORE adding the rnd
  size_t rndfirst = inputbits.size();

  // TODO TOREMOVE rndsize/rndfirst?
  // add space for random bits
  // inputbits.extend([0] * rndsize)
  for (unsigned int i = 0; i < rndsize; i++) {
    inputbits.push_back(0);
  }

  // TODO TOREMOVE rndsize/rndfirst?
  // add zero bit
  // inputbits.append(0)
  inputbits.push_back(0);

  // TODO TOREMOVE rndsize/rndfirst?
  assert(inputbits.size() ==
         pgc.config_.at("BITMAP_NB_SEGMENTS") + rndsize + 1);

  // garble input values
  // extracted_labels = garble_wrapper.extract_labels(inputbits)
  // for i in range(rndfirst, rndfirst + rndsize):
  //     extracted_labels[i] = [0, 0]  # patch random bits garbled values
  std::vector<Block> extracted_labels = pgc.ExtractLabels(inputbits);
  // TODO TOREMOVE rndsize/rndfirst?
  assert(extracted_labels.size() ==
         pgc.config_.at("BITMAP_NB_SEGMENTS") + rndsize + 1);

  // TODO TOREMOVE rndsize/rndfirst?
  for (unsigned int i = rndfirst; i < rndfirst + rndsize; i++) {
    extracted_labels[i].Zero();
  }

  return extracted_labels;
}
