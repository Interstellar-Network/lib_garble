#pragma once

#include "justgarble/parallel.h"

namespace interstellar {

namespace testing {

void EvalAndDisplay(const ParallelGarbledCircuit &parallel_garbled_circuit,
                    uint32_t width, uint32_t height, u_int32_t nb_evals);

}  // namespace testing

}  // namespace interstellar