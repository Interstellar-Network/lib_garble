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

#include <filesystem>
#include <string>

#include "parallel_garbled_circuit/parallel_garbled_circuit.h"

namespace interstellar {

namespace garble {

/**
 * Serialize to a buffer using Protobuf
 */
std::string Serialize(const ParallelGarbledCircuit& parallel_garbled_circuit);

/**
 * Serialize to a file using Protobuf
 */
void Serialize(const ParallelGarbledCircuit& parallel_garbled_circuit,
               std::filesystem::path pgarbled_output_path);

/**
 * Deserialize from a file
 */
void DeserializeFromFile(ParallelGarbledCircuit* parallel_garbled_circuit,
                         std::filesystem::path pgarbled_input_path);

/**
 * Deserialize from a buffer
 */
void DeserializeFromBuffer(ParallelGarbledCircuit* parallel_garbled_circuit,
                           const std::string& buffer);

}  // namespace garble

}  // namespace interstellar