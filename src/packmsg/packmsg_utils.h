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

#pragma once

#include <vector>

#include "justgarble/block.h"
#include "packmsg.h"
#include "parallel_garbled_circuit/parallel_garbled_circuit.h"
#include "stripped_circuit.h"

namespace interstellar::packmsg::internal {

/**
 * Used by PrepareInputLabels to control what segment will be on and off.
 * That is b/c we need to know how to draw a 0, a 1, etc
 * for 7 segments display, for 14 segments display, and any variant we want.
 **/
// DO NOT MODIFY THE ORDER OR REMOVE ONE
// It MUST MATCH lib_circuits/src/drawable/drawable.h
enum class PackmsgDigitSegmentsType { seven_segs, fourteen_segs, iching };

std::vector<Block> PrepareInputLabels(const garble::ParallelGarbledCircuit &pgc,
                                      const std::vector<uint8_t> &digits,
                                      PackmsgDigitSegmentsType digit_seg_type);

std::vector<uint8_t> XorBits(const std::vector<uint8_t> &bits1,
                             const std::vector<uint8_t> &bits2);

std::vector<uint64_t> pack_bits64(std::vector<uint8_t> &bits);

}  // namespace interstellar::packmsg::internal
