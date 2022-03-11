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

#pragma once

#include <algorithm>
#include <boost/dynamic_bitset.hpp>
#include <cstdint>
#include <vector>

#include "../circuit_data.h"
#include "../gate_types.h"

/**
 * Part of the XOR Directed Acyclic Graph
 * Represents either 'input0', 'input1' or 'output'.
 * Corresponds to the DATA in garbled_gate_.
 */
#if 0
class GraphLabel {
 public:
  explicit GraphLabel(uint32_t id);

  // Default ctor, needed for vector.resize()
  GraphLabel();

  uint32_t GetId() const;

  friend std::ostream &operator<<(std::ostream &os, const GraphLabel &other) {
    os << "(" << other.id_ << ")";
    return os;
  }

 private:
  uint32_t id_;
};

/**
 * Part of the XOR DAG
 * Represents a "gate"; corresponds to the INDEX of an entry in garbled_gate_.
 */
class GraphGate {
 public:
  explicit GraphGate(uint32_t id);

  // Default ctor, needed for vector.resize()
  GraphGate();

  uint32_t GetId() const;

  friend std::ostream &operator<<(std::ostream &os, const GraphGate &other) {
    os << "(" << other.id_ << ")";
    return os;
  }

 private:
  uint32_t id_;
};

#endif

struct GateInputs {
  uint32_t input0;
  uint32_t input1;

  GateInputs(uint32_t arg_input0, uint32_t arg_input1);

  // Needed for COMPILATION but SHOULD NOT be called
  // by "gate_to_inputs_.reserve(nb_gates_);"
  GateInputs(const GateInputs &) { std::terminate(); }
  GateInputs &operator=(const GateInputs &) { std::terminate(); }

  friend std::ostream &operator<<(std::ostream &os, const GateInputs &other) {
    os << "(" << other.input0 << "," << other.input1 << ")";
    return os;
  }
};

/**
 * This is used as a replacement for the 'wireGateMap'.
 * With the benefits of being easy to traverse by 'findAllGates'.
 *
 * NOTE: this is not a "standard" graph with vertices connecting nodes directly:
 * eg A --> B
 *    |
 *    C
 *
 * Instead each "element" looks like this:
 *
 * [28]  --> {457}  --> [495]
 * [29]  --/
 *
 * (Just to be clear: [28] & [29] are inputs of {457}, and [495] is its output)
 *
 * In this example: [28], [29] & [495] or 'labels'; (respectively input0, input1
 * and output), and {457} is a 'gate'.
 *
 * NOTE: we don't need to make the distinction b/w input0 & input1.
 * NOTE2: 'output' can be used as another layer's input!
 */
class XorGraph {
 public:
  /**
   * Init a new XorGraph with the given number of labels and gates
   */
  XorGraph(uint32_t nb_labels, uint32_t nb_gates,
           const std::vector<uint32_t> &gate_inputs0,
           const std::vector<uint32_t> &gate_inputs1,
           const std::vector<uint32_t> &gate_outputs,
           const std::vector<interstellar::SkcdGateType> &gate_types,
           const CircuitData &circuit_data);

  ~XorGraph();

  XorGraph(const XorGraph &) = delete;
  XorGraph &operator=(const XorGraph &) = delete;

  /**
   * Add vertices to the graph:
   * - [input0] -> {gate_id}
   * - [input1] -> {gate_id}
   * - {gate_id} -> [output]
   */
  // void AddGarbledGate(const GarbledGate &gate, uint32_t gate_id);

  /**
   * Find all the gates indirectely connected to the given gate.
   * ie traverse the whole graph, and return all the gate that shares at least
   * one input1, input0 or output with the given gate, "recursively!"
   *
   * return: two "sets"(ie no duplicate):
   * - a list of all inputs(NON-differentiated b/w input0 & input1) in the
   * "subgraph"
   * - a list of all output in the"subgraph"
   */
  // TODO outputs, same principle as 'all_gate_inputs' NOTE: requires merging
  // XorGateProcessor in this class!
  std::vector<uint32_t> FindAllGates(uint32_t gate_id);

 private:
  // Tried uint8_t: quite slower, uint32_t: on par
  using BitSet = boost::dynamic_bitset<uint64_t>;

  using GraphGate = uint32_t;
  using GraphLabel = uint32_t;

  /**
   * Return the list of GATES whose OUTPUT is label_id
   */
  uint32_t GetGatesFromOutput(uint32_t label_id) const {
    // DO NOT assert for output_to_gates_[label_id] != 0
    // This function is used to "go back to root inputs", and should return 0 if
    // label_id is NOT any gate's output
    return output_to_gates_[label_id];
  }

  /**
   * Return the list of GATES whose INPUT is label_id
   */
  const std::vector<uint32_t> &GetGatesFromInput(uint32_t label_id) const {
    // DO NOT assert for input_to_gates_[label_id].empty(): it SHOULD return an
    // empty vector if label_id is NOT an input
    // same as GetGatesFromOutput
    assert(label_id < input_to_gates_.size() &&
           "input_to_gates: out of range!");
    return input_to_gates_[label_id];
  }

  /**
   * Return the list of INPUTS whose GATE is gate_id
   * AND the "step parents" inputs: the inputs whose gate shares an INPUT with
   * the current gate.
   *
   * eg:
   * [28] --> {457}
   * [29] -/
   * [36] --> {460}
   *
   * ([28] & [29] inputs of {457}, [29] & [36] inputs of {460})
   *
   * This function should return: [28], [29] & [36] !
   */
  std::vector<uint32_t> GetParentAndStepParentInputsFromGate(
      uint32_t gate_id) const;

  /**
   * Return the OUTPUT whose GATE is gate_id
   */
  uint32_t GetOutputsFromGate(uint32_t gate_id) const {
    assert(gate_to_outputs_[gate_id] != 0 &&
           "GetOutputsFromGate: gate_to_outputs_ not set!");
    return gate_to_outputs_[gate_id];
  }

  /**
   * Gather all connected OUTPUTS from the given set of INPUTS
   * Basically:
   * - for each given input: gather all the gates
   *    - for each gate: add all outputs to the returned "set of outputs"
   *
   * return:
   * - the list of outputs
   * - the list of "traversed" gates
   */
  void GetAllOutputsFromInputs(const std::vector<uint32_t> &inputs,
                               std::vector<uint32_t> *outputs,
                               std::vector<uint32_t> *gates);

  void GetParentsGate(uint32_t gate_id, std::vector<uint32_t> *parent_gates,
                      std::vector<uint32_t> *parent_inputs) const;

  size_t nb_labels_;
  size_t nb_gates_;

  // input0/1 -> list of gates
  std::vector<std::vector<GraphGate>> input_to_gates_;
  // gate -> list of inputs = inverse of inputs_to_gate_
  // a gate HAS two inputs, so 1D vector of pair
  std::vector<GateInputs> gate_to_inputs_;
  // gate -> list of outputs
  // a gate only has ONE output, so 1D vector of GraphLabel
  std::vector<GraphLabel> gate_to_outputs_;
  // output -> list of gates = inverse of gates_to_outputs_
  // an output is NOT shared between gates, so 1D vector of GraphLabel
  std::vector<GraphGate> output_to_gates_;

  BitSet gates_indexes_;

#ifndef NDEBUG
  BitSet outputs_indexes_;  // used only as a check
#endif
};
