#include <absl/flags/flag.h>
#include <absl/flags/parse.h>

#include "serialize/serialize.h"
#include "utils_eval.h"

ABSL_FLAG(std::string, pgarbled_input_path, "./pgarbled.pb.bin",
          "path to a pgarbled.pb.bin");
ABSL_FLAG(uint32_t, width, 224, "display width");
ABSL_FLAG(uint32_t, height, 96, "display height");
ABSL_FLAG(uint32_t, nb_evals, 20, "number of evaluations to combine");

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  auto pgarbled_input_path_str = absl::GetFlag(FLAGS_pgarbled_input_path);
  auto width = absl::GetFlag(FLAGS_width);
  auto height = absl::GetFlag(FLAGS_height);
  auto nb_evals = absl::GetFlag(FLAGS_nb_evals);

  ParallelGarbledCircuit parallel_garbled_circuit;
  interstellar::garble::DeserializeFromFile(&parallel_garbled_circuit,
                                            pgarbled_input_path_str);

  interstellar::testing::EvalAndDisplay(parallel_garbled_circuit, width, height,
                                        nb_evals);

  return 0;
}
