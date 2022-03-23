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

#include <absl/random/bit_gen_ref.h>

#include <string>
#include <vector>

#include "parallel_garbled_circuit/parallel_garbled_circuit.h"
#include "prepackmsg.h"

namespace interstellar::packmsg {

/**
 * return:
 * - a MODIFIED PGC; this is DESTRUCTIVE
 * - the corresponding PrePackmsg
 */
void StripCircuit(garble::ParallelGarbledCircuit *pgc, PrePackmsg *pre_packmsg,
                  const std::vector<uint8_t> &digits);

namespace internal {

/**
 * TEST/INTERNAL USE ONLY
 * overload StripCircuit with a user-given absl::BitGen
 */
void StripCircuit(garble::ParallelGarbledCircuit *pgc, PrePackmsg *pre_packmsg,
                  const std::vector<uint8_t> &digits, absl::BitGenRef bitgen);

}  // namespace internal

}  // namespace interstellar::packmsg
