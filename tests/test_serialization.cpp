#include <gtest/gtest.h>

#include "evaluate/evaluate.h"
#include "justgarble/justGarble.h"
#include "justgarble/parallel.h"
#include "resources.h"
#include "serialize/serialize.h"

// Demonstrate some basic assertions.
TEST(Serialization, Adder) {
  GarbledCircuit add_garbled(std::string(interstellar::testing::data_dir) +
                             std::string("/adder.skcd.pb.bin"));
  add_garbled.garbleCircuit();
  const ParallelGarbledCircuit reference_parallel_add_garbled(
      std::move(add_garbled));

  auto buf = interstellar::garble::Serialize(reference_parallel_add_garbled);

  ParallelGarbledCircuit res_parallel_add_garbled;
  interstellar::garble::DeserializeFromBuffer(&res_parallel_add_garbled, buf);

  // easier to debug doing it field by field
  EXPECT_EQ(res_parallel_add_garbled.nb_inputs_,
            reference_parallel_add_garbled.nb_inputs_);
  EXPECT_EQ(res_parallel_add_garbled.nb_outputs_,
            reference_parallel_add_garbled.nb_outputs_);
  EXPECT_EQ(res_parallel_add_garbled.nb_gates_,
            reference_parallel_add_garbled.nb_gates_);
  EXPECT_EQ(res_parallel_add_garbled.nb_wires_,
            reference_parallel_add_garbled.nb_wires_);
  EXPECT_EQ(res_parallel_add_garbled.nb_layers_,
            reference_parallel_add_garbled.nb_layers_);
  EXPECT_EQ(res_parallel_add_garbled.non_xor_count_,
            reference_parallel_add_garbled.non_xor_count_);
  EXPECT_EQ(res_parallel_add_garbled.layer_counts_,
            reference_parallel_add_garbled.layer_counts_);
  EXPECT_EQ(res_parallel_add_garbled.layer_nonxor_counts_,
            reference_parallel_add_garbled.layer_nonxor_counts_);
  EXPECT_EQ(res_parallel_add_garbled.input_labels_,
            reference_parallel_add_garbled.input_labels_);
  EXPECT_EQ(res_parallel_add_garbled.output_labels_,
            reference_parallel_add_garbled.output_labels_);
  EXPECT_EQ(res_parallel_add_garbled.garbled_gates_,
            reference_parallel_add_garbled.garbled_gates_);
  EXPECT_EQ(res_parallel_add_garbled.garbled_table_[0],
            reference_parallel_add_garbled.garbled_table_[0]);
  EXPECT_EQ(res_parallel_add_garbled.garbled_table_[1],
            reference_parallel_add_garbled.garbled_table_[1]);
  EXPECT_EQ(res_parallel_add_garbled.garbled_table_[2],
            reference_parallel_add_garbled.garbled_table_[2]);
  EXPECT_EQ(res_parallel_add_garbled.garbled_table_[3],
            reference_parallel_add_garbled.garbled_table_[3]);
  EXPECT_EQ(res_parallel_add_garbled.outputs_,
            reference_parallel_add_garbled.outputs_);
  EXPECT_EQ(res_parallel_add_garbled.global_key_,
            reference_parallel_add_garbled.global_key_);

  // for peace of mind
  EXPECT_EQ(res_parallel_add_garbled, reference_parallel_add_garbled);
}