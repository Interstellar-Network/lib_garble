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

#include <gtest/gtest.h>

#include "evaluate/evaluate.h"
#include "garble_helper.h"
#include "justgarble/justGarble.h"
#include "parallel_garbled_circuit/parallel_garbled_circuit.h"
#include "resources.h"
#include "serialize_pgc/serialize.h"
#include "utils/utils_files.h"

using namespace interstellar;

TEST(EvaluateTest, Adder) {
  // deserialize from reference instead of re-garbling
  garble::ParallelGarbledCircuit pgc;
  garble::DeserializeFromFile(
      &pgc, std::filesystem::path(interstellar_testing::data_dir) /
                std::string("adder.pgarbled.pb.bin"));

  // input  i_bit1;
  // input  i_bit2;
  // input  i_carry;
  std::vector<std::vector<uint8_t>> all_inputs = {
      {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0},
      {0, 0, 1}, {1, 0, 1}, {0, 1, 1}, {1, 1, 1},
  };

  // output o_sum;
  // output o_carry;
  std::vector<std::vector<uint8_t>> all_expected_outputs = {
      {0, 0}, {1, 0}, {1, 0}, {0, 1}, {1, 0}, {0, 1}, {0, 1}, {1, 1},
  };

  std::vector<std::vector<uint8_t>> all_outputs;
  for (const auto& inputs : all_inputs) {
    all_outputs.emplace_back(EvaluateWithInputs(pgc, inputs));
  }

  EXPECT_EQ(all_expected_outputs, all_outputs);
}