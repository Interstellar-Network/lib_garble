#pragma once

#include "parallel_garbled_circuit/parallel_garbled_circuit.h"

namespace interstellar {

namespace testing {

void EvalAndDisplay(const garble::ParallelGarbledCircuit &parallel_garbled_circuit,
                    uint32_t width, uint32_t height, u_int32_t nb_evals);

}  // namespace testing

}  // namespace interstellar