#include <gtest/gtest.h>

#include "evaluate/evaluate.h"
#include "justgarble/justGarble.h"
#include "justgarble/parallel.h"
#include "resources.h"

// Demonstrate some basic assertions.
TEST(EvaluateTest, Adder) {
  GarbledCircuit add_garbled(std::string(interstellar::testing::data_dir) +
                             std::string("/adder.skcd.pb.bin"));
  add_garbled.garbleCircuit();
  ParallelGarbledCircuit parallel_add_garbled(std::move(add_garbled));

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
  for (const auto &inputs : all_inputs) {
    all_outputs.emplace_back(
        interstellar::garble::EvaluateWithInputs(parallel_add_garbled, inputs));
  }

  EXPECT_EQ(all_expected_outputs, all_outputs);
}