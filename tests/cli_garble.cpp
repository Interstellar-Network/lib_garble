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

#include "garble_helper.h"
#include "serialize_pgc/serialize.h"
#include "utils/utils_files.h"

using namespace interstellar;

ABSL_FLAG(std::string, skcd_input_path, "./skcd.pb.bin",
          "path to a skcd.pb.bin");
ABSL_FLAG(std::string, pgarbled_output_path, "./pgarbled.pb.bin",
          "output pgarbled.pb.bin");

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  auto skcd_input_path_str = absl::GetFlag(FLAGS_skcd_input_path);
  auto pgarbled_output_path_str = absl::GetFlag(FLAGS_pgarbled_output_path);

  auto skcd_buf =
      interstellar::interstellar_testing::utils::ReadFile(skcd_input_path_str);
  garble::ParallelGarbledCircuit pgc = garble::GarbleSkcdFromBuffer(skcd_buf);

  garble::Serialize(pgc, pgarbled_output_path_str);

  printf("garbling done\n");

  return 0;
}
