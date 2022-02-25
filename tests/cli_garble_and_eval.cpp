#include <absl/flags/flag.h>
#include <absl/flags/parse.h>

#include "justgarble/garble_helper.h"
#include "serialize/serialize.h"
#include "utils_eval.h"

// TODO width and height SHOULD come from the .skcd/.pgarbled
ABSL_FLAG(uint32_t, width, 224, "display width");
ABSL_FLAG(uint32_t, height, 96, "display height");
ABSL_FLAG(std::string, skcd_input_path, "./skcd.pb.bin",
          "path to a skcd.pb.bin");
ABSL_FLAG(uint32_t, nb_evals, 20, "number of evaluations to combine");
ABSL_FLAG(std::string, pgarbled_output_path, "./pgarbled.pb.bin",
          "output pgarbled.pb.bin");

/**
 * This is just a way to have an all in one cli that garbles then eval.
 *
 * It pretty stupidely serialize to a buffer then deserialize it right after...
 */
int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  auto skcd_input_path_str = absl::GetFlag(FLAGS_skcd_input_path);
  auto width = absl::GetFlag(FLAGS_width);
  auto height = absl::GetFlag(FLAGS_height);
  auto nb_evals = absl::GetFlag(FLAGS_nb_evals);
  auto pgarbled_output_path_str = absl::GetFlag(FLAGS_pgarbled_output_path);

  auto buf =
      interstellar::garblehelper::GarbleSkcdToBuffer(skcd_input_path_str);
  printf("garbling done\n");

  ParallelGarbledCircuit pgc;
  interstellar::garble::DeserializeFromBuffer(&pgc, buf);

  interstellar::testing::EvalAndDisplay(pgc, width, height, nb_evals);

  return 0;
}
