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

/*
 This file is part of JustGarble.

    JustGarble is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    JustGarble is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with JustGarble.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef justGarble
#define justGarble 1

#include <boost/filesystem.hpp>
#include <map>
#include <memory>
#include <vector>

#include "../circuit_data.h"
#include "../gate_types.h"
#include "block.h"

class GarbledGate {
 public:
  uint32_t input0, input1, output;
  int type;

  GarbledGate() : input0(0), input1(0), output(0) {}

  GarbledGate(uint32_t arg_input0, uint32_t arg_input1, uint32_t arg_output,
              uint32_t arg_type)
      : input0(arg_input0),
        input1(arg_input1),
        output(arg_output),
        type(arg_type) {
    // (input0, input1 & output) will later be "packed" as a single 64 bits
    // it is assumed each of them fits on 21 bits
    assert(arg_input0 < 0b111111111111111111111 &&
           "input0 does not fit on 21bits!");
    assert(arg_input1 < 0b111111111111111111111 &&
           "input1 does not fit on 21bits!");
    assert(arg_output < 0b111111111111111111111 &&
           "output does not fit on 21bits!");
  }

  GarbledGate(const GarbledGate&) = delete;
  GarbledGate& operator=(const GarbledGate&) = delete;

  bool operator==(const GarbledGate& other) const {
    return input0 == other.input0 && input1 == other.input1 &&
           output == other.output && type == other.type;
  }

  friend std::ostream& operator<<(std::ostream& os, const GarbledGate& other) {
    os << "(" << other.input0 << "," << other.input1 << "," << other.output
       << ")";
    return os;
  }
};

/**
 * In justGarble: case #undef TRUNCATED
 */
struct GarbledTable {
  Block table[4];

  GarbledTable(Block&& table0, Block&& table1, Block&& table2, Block&& table3);

  // Needed for COMPILATION but SHOULD NOT be called
  // by "garbled_table_.reserve(NONXORGates.size());"
  GarbledTable(const GarbledTable&) { std::terminate(); }
  GarbledTable& operator=(const GarbledTable&) { std::terminate(); }

  friend std::ostream& operator<<(std::ostream& os, const GarbledTable& other) {
    os << "(" << other.table[0] << "," << other.table[1] << ","
       << other.table[2] << "," << other.table[3] << ")";
    return os;
  }
};

// needed for FRIEND_TEST
class GarbledCircuit;
namespace interstellar {
namespace test {
void GetFullAdder(GarbledCircuit* garbled_circuit);
}
}  // namespace interstellar

/**
 * Part of garbling pipeline: Skcd -> GarbleCircuit -> ParallelGarbleCircuit.
 * The ctor only accepts a Skcd.
 *
 * changes:
 * - remove field 'wires': only needed during garbleCircuit()
 */
// TODO? convert all array to vectors, useful for tests(need serialization)
class GarbledCircuit {
 public:
  explicit GarbledCircuit(boost::filesystem::path skcd_input_path);

  GarbledCircuit(const GarbledCircuit&) = delete;
  GarbledCircuit& operator=(const GarbledCircuit&) = delete;

  // TEST ONLY
  void garbleCircuit();

  ~GarbledCircuit();

  // Various getters, used by ParallelGarbledCircuit

  uint32_t GetNbInputs() const { return nb_inputs_; };
  uint32_t GetNbOutputs() const {
    assert(static_cast<size_t>(nb_outputs_) == outputs_.size() &&
           "GetNbOutputs: mismatch!");
    return nb_outputs_;
  };
  uint32_t GetNbGates() const { return nb_gates_; };
  uint32_t GetNbWires() const { return nb_wires_; };
  Block GetGlobalKey() const { return global_key_; };

  uint32_t GetGateInput0(uint32_t index) const {
    assert(static_cast<size_t>(index) < gate_inputs0_.size() &&
           "GetGateInput0: out of range!");
    return gate_inputs0_[index];
  }

  uint32_t GetGateInput1(uint32_t index) const {
    assert(static_cast<size_t>(index) < gate_inputs1_.size() &&
           "GetGateInput1: out of range!");
    return gate_inputs1_[index];
  }

  uint32_t GetGateOutput(uint32_t index) const {
    assert(static_cast<size_t>(index) < gate_outputs_.size() &&
           "GetGateOutput: out of range!");
    return gate_outputs_[index];
  }

  interstellar::SkcdGateType GetGateType(uint32_t index) const {
    assert(static_cast<size_t>(index) < gate_types_.size() &&
           "GetGateType: out of range!");
    return gate_types_[index];
  }

  /**
   * param: new_id: is NOT sequential, it is 'newId' in the code
   * see 'NONXORGates' processing in garble.cpp for details
   */
  const auto& GetGarbledTable(uint32_t new_id) const {
    // convert new_id into an index in 'garbled_table_'
    int16_t index = garbled_table_index_map_[new_id];
    assert(index != -1 && "GetGarbledTable: index not init!");

    assert(static_cast<size_t>(index) < garbled_table_.size() &&
           "GetGarbledTable: out of range!");
    return garbled_table_[index];
  }

  // NOTE the 4 below return NON-const: used in PGC to swap the data
  auto& GetOutputs() { return outputs_; }
  auto& GetInputLabels() { return input_labels_; }
  auto& GetOuputsLabels() { return output_labels_; }

 private:
  uint32_t nb_inputs_, nb_outputs_, nb_gates_, nb_wires_;
  // Both inputLabels & outputLabels reference a Wire's label0 or label1, from
  // the "wires" vector
  std::vector<Block> input_labels_, output_labels_;
  // we get them as A,B,GO from Skcd/BlifParser, and we only access them one
  // field at a time; so to avoid a conversion, we store them the same way
  // NOTE: 'O' is stored in 'outputs_'
  std::vector<uint32_t> gate_inputs0_;
  std::vector<uint32_t> gate_inputs1_;
  std::vector<uint32_t> gate_outputs_;
  std::vector<interstellar::SkcdGateType> gate_types_;
  // TODO garbledTable is only used when processing XOR gate, so a few elements
  // of this vector are not init; But it's less than 1%, so...
  std::vector<GarbledTable> garbled_table_;
  // signed: we use '-1' to mark as invalid
  std::vector<int16_t> garbled_table_index_map_;
  std::vector<uint32_t> outputs_;
  Block global_key_;

  CircuitData circuit_data_;

  // TEST ONLY, cf "friend class"/FRIEND_TEST
  GarbledCircuit() {}

  void garbleCircuit(uint32_t seed);

  // gtest_prod.h
  // #define FRIEND_TEST(test_case_name, test_name)
  //  friend class test_case_name##_##test_name##_Test
  // FRIEND_TEST(GarbleTest, Adder);
  friend class GarbleTest_Adder_Test;
  friend void interstellar::test::GetFullAdder(GarbledCircuit* garbled_circuit);
};

#endif
