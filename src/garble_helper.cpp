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

#include "garble_helper.h"

#include "parallel_garbled_circuit/parallel_garbled_circuit.h"
#include "serialize_pgc/serialize.h"

namespace interstellar {

namespace garble {

ParallelGarbledCircuit GarbleSkcdFromBuffer(std::string_view skcd_buffer) {
  GarbledCircuit garbledCircuit(skcd_buffer);

  garbledCircuit.Garble();

  // Now for the parallel part
  return ParallelGarbledCircuit(std::move(garbledCircuit));
}

}  // namespace garble

}  // namespace interstellar