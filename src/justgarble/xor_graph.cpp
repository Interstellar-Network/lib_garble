#include "xor_graph.h"

// TODO Bitsets: optimize alloc: we should be able to share some/most between
// various function calls

#ifndef NDEBUG
#include <unordered_set>
#endif

#if 0
GraphLabel::GraphLabel(uint32_t id) : id_(id) {}

GraphLabel::GraphLabel() : id_(0) {}

uint32_t GraphLabel::GetId() const { return id_; }

GraphGate::GraphGate(uint32_t id) : id_(id) {}

GraphGate::GraphGate() : id_(0) {}

uint32_t GraphGate::GetId() const { return id_; }
#endif

GateInputs::GateInputs(uint32_t arg_input0, uint32_t arg_input1)
    : input0(arg_input0), input1(arg_input1) {}

XorGraph::XorGraph(uint32_t nb_labels, uint32_t nb_gates,
                   const std::vector<uint32_t> &gate_inputs0,
                   const std::vector<uint32_t> &gate_inputs1,
                   const std::vector<uint32_t> &gate_outputs,
                   const std::vector<interstellar::SkcdGateType> &gate_types,
                   const CircuitData &circuit_data)
    : nb_labels_(nb_labels), nb_gates_(nb_gates) {
  // reserve those that are constructed iteratively
  gate_to_inputs_.reserve(nb_gates_);
  gate_to_outputs_.reserve(nb_gates_);

  // TEMP? alternatively: same as input_to_gates = construct a temp then
  // cosntruct final 2D vector
  output_to_gates_.resize(nb_labels_);

  // NOTE/REMINDER: most input0/input1 are shared between lots of gates
  // Therefore, even if the vector.size() is 'nb_gates'(eg ~55k), only a
  // typically a few 100s unique values.
  //
  // To construct the final 2D input_to_gates, we need to allocate the size
  // based on the number of unique inputs
  // Good thing is: typically the "max input0/1" is just a bit bigger than
  // "number of unique input0/1";
  // eg: temp_input_to_gates size = 723, gate_input_min = 28, max = 805
  // So the simplest way is to alloc a vector of size 805 elem. Yes it means
  // (805-723) elem are useless, but it's fine.
  // TODO? We could use the min for an offset: eg if min = 555, we can say
  // input_to_gates_[0] is for gate_id = 555
  //
  // Tried various strategies:
  // - full prealloc input_to_gates_: REALLY SLOW(4x slower)
  // - temp input_to_gates std::unordered_map<uint32_t, std::vector<GraphGate>>:
  // OK: ~5% slower
  // - temp input_to_gates std::unordered_map<uint32_t,
  // std::unordered_set<GraphGate>>: SLOW(2x slower)
  // New strategy in two phases(FASTEST):
  // - first loop on all inputs and count how many times a gates appear for an
  // input
  // - alloc the correct size based on this count
  // Then the "final" loop on "nb_gates" can emplace_back without realloc

  // now construct the final : input0/1 -> list of gates
  // resize: reminder: the outer one is accessed randomly!
  // +1: because gate_input_max IS a valid value
  input_to_gates_.resize(circuit_data.gate_input_max + 1);
  for (uint32_t i = 0, s = input_to_gates_.size(); i < s; ++i) {
    input_to_gates_[i].reserve(circuit_data.input_gate_count[i]);
  }

  for (uint32_t gate_id = 0; gate_id < nb_gates; ++gate_id) {
    // skip NON-XOR gates
    if (gate_types[gate_id] != interstellar::SkcdGateType::XOR) {
      // IMPORTANT: this is expected by all the internal functions
      // a 0 means 'not an input/output'
      gate_to_inputs_.emplace_back(0, 0);
      gate_to_outputs_.emplace_back(0);
      continue;
    }

    // input0/1 -> list of gates
    assert(input_to_gates_[gate_inputs0[gate_id]].size() + 1 <=
               input_to_gates_[gate_inputs0[gate_id]].capacity() &&
           "input_to_gates_ inputs0: wrong capacity!");
    assert(input_to_gates_[gate_inputs1[gate_id]].size() + 1 <=
               input_to_gates_[gate_inputs1[gate_id]].capacity() &&
           "input_to_gates_ inputs1: wrong capacity!");
    input_to_gates_[gate_inputs0[gate_id]].emplace_back(gate_id);
    input_to_gates_[gate_inputs1[gate_id]].emplace_back(gate_id);

    // gate -> list of inputs = inverse of inputs_to_gate_
    gate_to_inputs_.emplace_back(gate_inputs0[gate_id], gate_inputs1[gate_id]);

    // gate -> list of outputs
    gate_to_outputs_.emplace_back(gate_outputs[gate_id]);

    // output -> list of gates = inverse of gates_to_outputs_
    assert(output_to_gates_[gate_outputs[gate_id]] == 0 &&
           "output_to_gates_: already set!");
    output_to_gates_[gate_outputs[gate_id]] = gate_id;
  }

  // CHECK: the reserve() sizes should match the final size
  // ie MUST have emplaced_back/pushed_back even for NON-XOR gate
  assert(gate_to_inputs_.size() == nb_gates_ && "gate_to_inputs_: wrong size!");
  assert(gate_to_outputs_.size() == nb_gates_ &&
         "gate_to_outputs_: wrong size!");

  // all 0's by default
  gates_indexes_.resize(nb_gates_);

#ifndef NDEBUG
  outputs_indexes_.resize(nb_labels_);
#endif
}

XorGraph::~XorGraph() = default;

#if 0
void XorGraph::AddGarbledGate(const GarbledGate &gate, uint32_t gate_id) {
  // TODO assert input0/1 & output NOT arleady in the vectors

  // input0/1 -> list of gates
  // resize if needed, usually 200-300 elements
  if (input_to_gates_[gate.input0].capacity() < 256) {
    input_to_gates_[gate.input0].reserve(256);
  }
  if (input_to_gates_[gate.input1].capacity() < 256) {
    input_to_gates_[gate.input1].reserve(256);
  }
  input_to_gates_[gate.input0].emplace_back(gate_id);
  input_to_gates_[gate.input1].emplace_back(gate_id);

  // gate -> list of inputs = inverse of inputs_to_gate_
  assert(gate_to_inputs_[gate_id].input0 == 0 &&
         "gate_to_inputs_ input0 already set!");
  assert(gate_to_inputs_[gate_id].input1 == 0 &&
         "gate_to_inputs_ input1 already set!");
  gate_to_inputs_[gate_id] = GateInputs(gate.input0, gate.input1);

  // gate -> list of outputs
  assert(gate_to_outputs_[gate_id] == 0 && "gate_to_outputs_ already set!");
  gate_to_outputs_[gate_id] = gate.output;

  // output -> list of gates = inverse of gates_to_outputs_
  assert(output_to_gates_[gate.output] == 0 &&
         "output_to_gates_: already set!");
  output_to_gates_[gate.output] = gate_id;
}
#endif

/**
 * Helper function used by FindAllGates
 * From a list of gates, return:
 * - the list of all inputs
 * - and, if any, the list of "grand-parent" gates: gates whose outputs are
 * the returned 'list of all inputs'
 *
 * This is used to "recursively" get the ROOT inputs from a gate/set of gates.
 */
void XorGraph::GetParentsGate(uint32_t gate_id,
                              std::vector<uint32_t> *parent_gates,
                              std::vector<uint32_t> *parent_inputs) const {
  // 'parent_gates': usually empty
  // 'parent_inputs': usually 2 elem, sometimes 3, one 9-13
  parent_inputs->reserve(16);

  auto current_parent_inputs = GetParentAndStepParentInputsFromGate(gate_id);

  for (auto parent_input : current_parent_inputs) {
    // add the current input to the parent_inputs, if not yet present
    if (std::find(parent_inputs->begin(), parent_inputs->end(), gate_id) ==
        parent_inputs->end()) {
      parent_inputs->push_back(parent_input);
    }

    // find the "grand-parent" gates, if any
    // it can be null if the parent_inputs are "root inputs"
    uint32_t parent_gate_current = GetGatesFromOutput(parent_input);

    if ((parent_gate_current != 0) &&
        (std::find(parent_gates->begin(), parent_gates->end(),
                   parent_gate_current) == parent_gates->end())) {
      parent_gates->push_back(parent_gate_current);
    }
  }

  // if this is triggered:
  // - if size is still low(32 elems?): bump reserve() above
  // - if it is big: use a BitSet
  assert(parent_inputs->size() < 16 && "all_parents: was resized!");
}

std::vector<uint32_t> XorGraph::FindAllGates(uint32_t gate_id) {
  std::vector<uint32_t> root_inputs;

  // Fist step: go the "root inputs" from the given gate
  {
    std::vector<uint32_t> parent_gates;
    GetParentsGate(gate_id, &parent_gates, &root_inputs);

    // if parent_gates is NOT empty: "recursively" go back to the root inputs
    // if it is, parent_inputs ARE the "root inputs"
    if (!parent_gates.empty()) {
      assert(false && "xorgraph: NotImplemented");
    }
  }

  // Now that we have the "root inputs", we can gather all the children gates
  std::vector<uint32_t> all_gates;

  // usually between 250-350 elements
  all_gates.reserve(384);

  std::vector<uint32_t> children_outputs;
  GetAllOutputsFromInputs(root_inputs, &children_outputs, &all_gates);

  // TODO aux function
  // "recurse" using the same principle as the block above
  if (!children_outputs.empty()) {
    std::vector<uint32_t> grand_children_outputs;
    GetAllOutputsFromInputs(children_outputs, &grand_children_outputs,
                            &all_gates);

    if (!grand_children_outputs.empty()) {
      // "recurse", same as children -> grand_children
      std::vector<uint32_t> grand_grand_children_outputs;
      GetAllOutputsFromInputs(grand_children_outputs,
                              &grand_grand_children_outputs, &all_gates);

      if (!grand_grand_children_outputs.empty()) {
        // "recurse", same as children -> grand_children
        std::vector<uint32_t> grand_grand_grand_children_outputs;
        GetAllOutputsFromInputs(grand_grand_children_outputs,
                                &grand_grand_grand_children_outputs,
                                &all_gates);

        assert(grand_grand_grand_children_outputs.empty() &&
               "xorgraph: NotImplemented");
      }
    }
  }

  return all_gates;
}

std::vector<uint32_t> XorGraph::GetParentAndStepParentInputsFromGate(
    uint32_t gate_id) const {
  // first gather the (direct) parents
  // (eg: labels_id = [28, 29])
  const auto &parent_inputs = gate_to_inputs_[gate_id];
  assert((parent_inputs.input0 != 0 || parent_inputs.input1 != 0) &&
         "parent_inputs: both are 0(which means output)!");

  // then gather all the CHILDREN from the parents labels
  // those will be the "siblings" of gate_id, including half-siblings
  std::vector<uint32_t> siblings_gates;

  // usually 200-350 elements
  siblings_gates.reserve(384);

  // replaces std::find: store which gate(indexes) are in siblings_gates
  BitSet siblings_gates_indexes(nb_gates_);  // all 0's by default
  {
    // parent's input0
    if (parent_inputs.input0) {
      const auto &sibling_gates_0 = GetGatesFromInput(parent_inputs.input0);
      for (auto sibling_gate : sibling_gates_0) {
        if (!siblings_gates_indexes[sibling_gate]) {
          siblings_gates.push_back(sibling_gate);
          siblings_gates_indexes[sibling_gate] = true;
        }
      }
    }

    // parent's input1
    if (parent_inputs.input1) {
      const auto &sibling_gates_1 = GetGatesFromInput(parent_inputs.input1);
      for (auto sibling_gate : sibling_gates_1) {
        if (!siblings_gates_indexes[sibling_gate]) {
          siblings_gates.push_back(sibling_gate);
          siblings_gates_indexes[sibling_gate] = true;
        }
      }
    }
  }

  // then gather the PARENT of the siblings
  // This will add all the (step) parents of all the half-siblings.
  std::vector<uint32_t> all_parents;

  // usually 2 parents, sometimes 3, one 9-13
  all_parents.reserve(16);

  for (auto sibling_gate_id : siblings_gates) {
    const auto &parents = gate_to_inputs_[sibling_gate_id];

    assert(
        gates_indexes_[sibling_gate_id] == 0 &&
        "GetParentAndStepParentInputsFromGate: sibling_gate_id already there!");

    // parent's input0
    // NOTE: only a few elems, so skip the BitSet
    if (parent_inputs.input0) {
      if (std::find(all_parents.begin(), all_parents.end(), parents.input0) ==
          all_parents.end()) {
        all_parents.emplace_back(parents.input0);
      }
    }

    // parent's input1
    // NOTE: only a few elems, so skip the BitSet
    if (parent_inputs.input1) {
      if (std::find(all_parents.begin(), all_parents.end(), parents.input1) ==
          all_parents.end()) {
        all_parents.emplace_back(parents.input1);
      }
    }
  }

  // if this is triggered:
  // - if size is still low(32 elems?): bump reserve() above
  // - if it is big: use a BitSet
  assert(all_parents.size() < 16 && "all_parents: was resized!");

  return all_parents;
}

/**
 * param: outputs: OUT only: can be empty,
 * param: gates: IN-OUT: SHOULD be be the gates on the previous layer,
 * corresponding to 'inputs' This allows to cut down on processing time A LOT.
 */
void XorGraph::GetAllOutputsFromInputs(const std::vector<uint32_t> &inputs,
                                       std::vector<uint32_t> *outputs,
                                       std::vector<uint32_t> *gates) {
  // 'gates' is usually 200-360 elem
  // 'outputs' is either 0, or same nb elems as gates
  gates->reserve(384);
  outputs->reserve(384);

  size_t input_max = input_to_gates_.size();

  // replaces std::find: store which gate(indexes) are in children_gates
  // BitSet children_gates_indexes(nb_gates_);  // all 0's by default
  for (auto input : inputs) {
    // shortcut: if input is outside of the valid input range, we can skip it
    if (input >= input_max) {
      continue;
    }

    const auto &current_gates = GetGatesFromInput(input);
    for (auto gate_id : current_gates) {
      // we CAN process the same gate twice, because we are looping on a random
      // set of inputs, which can share gates
      if (gates_indexes_[gate_id]) {
        continue;
      }

      // assert(std::find(gates->begin(), gates->end(), gate_id) == gates->end()
      // &&
      //        "GetAllOutputsFromInputs: gate_id already processed(1)!");
      assert(gates_indexes_[gate_id] == 0 &&
             "GetAllOutputsFromInputs: gate_id already processed(2)!");

      gates->push_back(gate_id);
      gates_indexes_[gate_id] = true;

      // gather the outputs from each of the "child gate"
      // If we are here, it is a new gate(ie one that has NOT been traversed
      // before)
      // Only in this case we need to add its output to the result.
      uint32_t child_output = GetOutputsFromGate(gate_id);

      // as we are basically processing layer by layer, a "candidate output"
      // SHOULD NOT already be in 'inputs'
      assert(std::find(inputs.begin(), inputs.end(), child_output) ==
                 inputs.end() &&
             "GetAllOutputsFromInputs: child_output already processed!");
      // a gate only has one output, and outputs are not shared, so it should
      // not happen. (if we indeed skip processing the same gate_id, see
      // 'continue;')
      assert(outputs_indexes_[child_output] == false &&
             outputs_indexes_[child_output].flip() &&
             "child_output already in result!");

      outputs->push_back(child_output);
    }
  }

  // previously asserted: set<gates> == gates.size(): means there is no
  // duplicate b/w calls
  // But it also checked at "gate_id already processed(2)"
}

/**
 * Return all the connected {gate} for this [label]
 */
// std::vector<uint32_t> XorGraph::GetGatesFromLabel(uint32_t label_id) {
//   std::vector<uint32_t> gates_id;

//   // TODO exclude self!

//   // we care about an EdgeInput's INPUT
//   for (const auto &edge_in : edges_in_) {
//     if (edge_in.GetInput().GetId() == label_id) {
//       gates_id.push_back(edge_in.GetGate().GetId());
//     }
//   }

//   // we care about an EdgeOutput's GATE
//   for (const auto &edge_out : edges_out_) {
//     if (edge_out.GetGate().GetId() == label_id) {
//       gates_id.push_back(edge_out.GetGate().GetId());
//     }
//   }

//   return gates_id;
// }

#if 0
  /**
   * input0 or input1
   * Connects label TO gate: eg [input0/1] --> {gate}
   */
  class EdgeInput {
   public:
    EdgeInput(const GraphLabel &input, const GraphGate &gate)
        : input_(input), gate_(gate) {}

    const GraphLabel &GetInput() const { return input_; }

    const GraphGate &GetGate() const { return gate_; }

   private:
    const GraphLabel &input_;
    const GraphGate &gate_;
  };

  /**
   * output
   * Connects gate TO label: eg {gate} --> [output]
   */
  struct EdgeOutput {
   public:
    EdgeOutput(const GraphGate &gate, const GraphLabel &output)
        : gate_(gate), output_(output) {}

    const GraphLabel &GetOutput() const { return output_; }

    const GraphGate &GetGate() const { return gate_; }

   private:
    const GraphGate &gate_;
    const GraphLabel &output_;
  };

#endif

#if 0
  /**
   * Represents either:
   * - label TO gate: eg [input0/1] --> {gate}
   * - gate TO label: eg {gate} --> [output]
   */
  class Edge {
   public:
    Edge(uint32_t target) : target_(target) {}

   private:
    uint32_t target_;
  };
#endif