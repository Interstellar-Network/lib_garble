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

#include "packmsg/packmsg.h"
#include "parallel_garbled_circuit/parallel_garbled_circuit.h"

namespace interstellar {

namespace garble {

std::vector<uint8_t> EvaluateWithInputs(
    const ParallelGarbledCircuit &parallel_garbled_circuit,
    const std::vector<uint8_t> &inputs);

std::vector<uint8_t> EvaluateWithPackmsg(
    const ParallelGarbledCircuit &parallel_garbled_circuit,
    const std::vector<uint8_t> &inputs, const packmsg::Packmsg &packmsg);

}  // namespace garble

}  // namespace interstellar