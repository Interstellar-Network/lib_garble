#pragma once

#include <vector>

#include "justgarble/parallel.h"

namespace interstellar {

namespace garble {

std::vector<uint8_t> EvaluateWithInputs(
    const ParallelGarbledCircuit &parallel_garbled_circuit,
    const std::vector<uint8_t> &inputs);

}  // namespace garble

}  // namespace interstellar