#pragma once

#include <vector>

#include "parallel_garbled_circuit/parallel_garbled_circuit.h"

namespace interstellar {

namespace garble {

std::vector<uint8_t> EvaluateWithInputs(
    const ParallelGarbledCircuit &parallel_garbled_circuit,
    const std::vector<uint8_t> &inputs);

}  // namespace garble

}  // namespace interstellar