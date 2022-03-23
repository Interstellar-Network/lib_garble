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

#include <absl/flags/flag.h>
#include <absl/flags/parse.h>

#include "serialize_pgc/serialize.h"
#include "utils_eval.h"

ABSL_FLAG(std::string, pgarbled_input_path, "./pgarbled.pb.bin",
          "path to a pgarbled.pb.bin");
ABSL_FLAG(uint32_t, nb_evals, 20, "number of evaluations to combine");

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  auto pgarbled_input_path_str = absl::GetFlag(FLAGS_pgarbled_input_path);
  auto nb_evals = absl::GetFlag(FLAGS_nb_evals);

  interstellar::garble::ParallelGarbledCircuit parallel_garbled_circuit;
  interstellar::garble::DeserializeFromFile(&parallel_garbled_circuit,
                                            pgarbled_input_path_str);

  interstellar::testing::EvalAndDisplay(parallel_garbled_circuit, nb_evals);

  return 0;
}
