#pragma once

#include <boost/filesystem.hpp>
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
               boost::filesystem::path pgarbled_output_path);

/**
 * Deserialize from a file
 */
void DeserializeFromFile(ParallelGarbledCircuit* parallel_garbled_circuit,
                         boost::filesystem::path pgarbled_input_path);

/**
 * Deserialize from a buffer
 */
void DeserializeFromBuffer(ParallelGarbledCircuit* parallel_garbled_circuit,
                           const std::string& buffer);

}  // namespace garble

}  // namespace interstellar