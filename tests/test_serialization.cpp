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

#include "justgarble/justGarble.h"
#include "parallel_garbled_circuit/parallel_garbled_circuit.h"
#include "resources.h"
#include "serialize_pgc/serialize.h"

TEST(Serialization, DeserializeAdder) {
  GarbledCircuit garbled(
      std::filesystem::path(interstellar::interstellar_testing::data_dir) /
      std::string("adder.skcd.pb.bin"));
  garbled.Garble();
  const interstellar::garble::ParallelGarbledCircuit
      reference_parallel_add_garbled(std::move(garbled));

  auto buf = interstellar::garble::Serialize(reference_parallel_add_garbled);

  interstellar::garble::ParallelGarbledCircuit res_parallel_add_garbled;
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