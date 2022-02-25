#include <absl/flags/flag.h>
#include <absl/flags/parse.h>

#include "justgarble/garble_helper.h"

ABSL_FLAG(std::string, skcd_input_path, "./skcd.pb.bin",
          "path to a skcd.pb.bin");
ABSL_FLAG(std::string, pgarbled_output_path, "./pgarbled.pb.bin",
          "output pgarbled.pb.bin");

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  auto skcd_input_path_str = absl::GetFlag(FLAGS_skcd_input_path);
  auto pgarbled_output_path_str = absl::GetFlag(FLAGS_pgarbled_output_path);

  interstellar::garblehelper::GarbleSkcdToFile(skcd_input_path_str,
                                               pgarbled_output_path_str);

  printf("garbling done\n");

  return 0;
}
