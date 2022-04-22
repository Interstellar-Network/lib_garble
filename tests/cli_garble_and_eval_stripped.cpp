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
#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>

#include "garble_helper.h"
#include "packmsg/stripped_circuit.h"
#include "packmsg_helper.h"
#include "serialize_pgc/serialize.h"
#include "utils/utils_files.h"
#include "utils_eval.h"

using namespace interstellar;

ABSL_FLAG(std::string, skcd_input_path, "./skcd.pb.bin",
          "path to a skcd.pb.bin");
ABSL_FLAG(uint32_t, nb_evals, 20, "number of evaluations to combine");
ABSL_FLAG(std::string, pgarbled_stripped_output_path, "./pgarbled.pb.bin",
          "output pgarbled.pb.bin");
ABSL_FLAG(std::vector<std::string>, digits,
          std::vector<std::string>({"4", "2"}),
          "digits for the message/pinpad");
ABSL_FLAG(std::string, png_output_path, "",
          "if not set it will display using X11 instead of write a .png");

/**
 * This is just a way to have an all in one cli that garbles then eval.
 *
 * It pretty stupidely serialize to a buffer then deserialize it right
 * after...
 */
int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  auto skcd_input_path_str = absl::GetFlag(FLAGS_skcd_input_path);
  auto nb_evals = absl::GetFlag(FLAGS_nb_evals);
  auto pgarbled_stripped_output_path_str =
      absl::GetFlag(FLAGS_pgarbled_stripped_output_path);
  auto digits_str = absl::GetFlag(FLAGS_digits);
  auto png_output_path_str = absl::GetFlag(FLAGS_png_output_path);

  auto skcd_buf =
      interstellar::interstellar_testing::utils::ReadFile(skcd_input_path_str);
  garble::ParallelGarbledCircuit pgc = garble::GarbleSkcdFromBuffer(skcd_buf);
  absl::PrintF("garbling done\n");

  // TODO digits: flag for random vs user-given?
  auto nb_digits_expected = pgc.config_.at("NB_DIGITS");
  if (nb_digits_expected != digits_str.size()) {
    absl::PrintF("size expected = %d vs given = %s\n", nb_digits_expected,
                 absl::StrJoin(digits_str, "-").c_str());
    throw std::logic_error("digits size mismatch");
  }

  // convert vector<string> to vector<char> for the digits
  std::vector<uint8_t> digits;
  std::transform(digits_str.begin(), digits_str.end(),
                 std::back_inserter(digits),
                 [](const std::string& s) { return std::stoi(s); });
  absl::PrintF("digits : %s\n", absl::StrJoin(digits, "-").c_str());

  packmsg::PrePackmsg prepackmsg;
  packmsg::StripCircuit(&pgc, &prepackmsg, digits);

  // typically prepackmsg stored in DB + pgc(stripped) sent to user
  /***************************** LATER
   * ****************************************/
  // at tx time: packmsg generated then sent to user
  auto packmsg = packmsg::PackmsgFromPrepacket(prepackmsg, L"test\nmessage");

  // TODO EvalAndDisplayWithPackg; EvalAndDisplay SHOULD return garbage values
  testing::EvalAndDisplayWithPackmsg(pgc, packmsg, nb_evals,
                                     png_output_path_str);

  return 0;
}
