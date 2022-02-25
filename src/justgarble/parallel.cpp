//
// Created by nathanprat on 5/12/16.
//

#include "parallel.h"

#include <algorithm>
#include <cinttypes>
#include <iostream>
#include <random>
#include <set>
#include <sstream>

#ifndef NDEBUG
#include <unordered_set>
#endif

#include "defines.h"

namespace {

// TODO remove unused fields(and maybe replaced by simple uint8)
struct MoveableWire {
  uint32_t id;  // Original position
  uint32_t position;
  uint32_t layer{0};
  // bool fixed{false};

  explicit MoveableWire(uint32_t id) : id(id), position(id) {}

  // Needed for COMPILATION but SHOULD NOT be called
  // by "wiring.reserve(nb_wires_);"
  MoveableWire(const MoveableWire &) { std::terminate(); }
  MoveableWire &operator=(const MoveableWire &) { std::terminate(); }
};

}  // anonymous namespace

ParallelGarbledCircuit::ParallelGarbledCircuit() {}

#define DEBUG_toParallelGarbledCircuit 0
ParallelGarbledCircuit::ParallelGarbledCircuit(
    GarbledCircuit &&garbledCircuit) {
  // TODO assert garbledCircuit IsGarbled()

  // 1 - Initialize circuit meta data
  nb_inputs_ = garbledCircuit.GetNbInputs();    // Circuit Input wires
  nb_outputs_ = garbledCircuit.GetNbOutputs();  // Circuit Output wires
  nb_gates_ = garbledCircuit.GetNbGates();      // Gates
  nb_wires_ = garbledCircuit.GetNbWires();      // Wires
  global_key_ = garbledCircuit.GetGlobalKey();

  // 2 - Find Layers

  std::vector<MoveableWire> wiring;
  wiring.reserve(nb_wires_);

  // Init wiring
  // Previously also: "Circuit inputs and dummy wire are fixed"
  // and "Circuit outputs are fixed"
  for (uint32_t i = 0; i < nb_wires_; i++) {
    wiring.emplace_back(i);
  }

  // Compute Gates layer (stack to the left)
  // Gate ordering is a topological ordering, so no need to worry about wire
  // layer computation referring a non-initialized element

  layer_counts_.resize(4, 0);
  layer_nonxor_counts_.resize(4, 0);
  non_xor_count_ = 0;

  // compute the max, so init to min
  nb_layers_ = std::numeric_limits<uint32_t>::min();

  // To sort the NON-XOR gates by their input0, we need to store the min/max of
  // the input0
  // We do it now b/c we are already looping on "q" here
  // eg: typically input0_min is 0, input0_max is 494
  uint16_t input0_min = std::numeric_limits<uint16_t>::max();
  uint16_t input0_max = std::numeric_limits<uint16_t>::min();
  std::vector<std::pair<uint16_t, uint16_t>> layer_input0_minmax(
      4, {std::numeric_limits<uint16_t>::max(),
          std::numeric_limits<uint16_t>::min()});

  for (uint32_t i = 0; i < nb_gates_; i++) {
    uint32_t gate_input0 = garbledCircuit.GetGateInput0(i);
    uint32_t gate_input1 = garbledCircuit.GetGateInput1(i);
    uint32_t gate_output = garbledCircuit.GetGateOutput(i);
    interstellar::SkcdGateType gate_type = garbledCircuit.GetGateType(i);

    wiring[gate_output].layer =
        std::max(wiring[gate_input0].layer, wiring[gate_input1].layer) + 1;
    nb_layers_ = std::max(nb_layers_, wiring[gate_output].layer + 1);

    int gateLayer = wiring[gate_output].layer;

    assert(gateLayer < 4 && "more than 4 layers[1]!");
    assert(nb_layers_ <= 4 && "nbLayers is NOT 4!");

    if (gate_type == interstellar::SkcdGateType::XOR) {
      assert(gate_input0 < std::numeric_limits<uint16_t>::max() &&
             "input0 > uint16_t::max !");
      auto gate_input0_16 = static_cast<uint16_t>(gate_input0);

      input0_min = std::min(input0_min, gate_input0_16);
      input0_max = std::max(input0_max, gate_input0_16);

      auto &layer_min_max_pair = layer_input0_minmax[gateLayer];
      layer_min_max_pair.first =
          std::min(layer_min_max_pair.first, gate_input0_16);
      layer_min_max_pair.second =
          std::max(layer_min_max_pair.second, gate_input0_16);
    } else {
      layer_nonxor_counts_[gateLayer]++;
      non_xor_count_++;
    }

    layer_counts_[gateLayer]++;
  }

  // To be able to quickly sort the gates in the next step:
  // - sort the gates by the their OUTPUT's WIRING's layer
  // - sort each sublayer by: first NON-XOR then XOR
  // - sort the XOR "subsublayer" by their input0
  // The Step 3 requires gate IDs, so layer is a pair<pointer on a gate, id>.
  //
  // NOTE: XorGates will use quite some RAM, so to limit that we use 16 bits
  // GateID
  // "new_size2" requires 32bits: q = 125481
  using GateID = uint32_t;
  assert(nb_gates_ < std::numeric_limits<GateID>::max() &&
         "q does not fit on GateID size!");
  using NonXorGates = std::vector<GateID>;
  // XOR gates need to be ordered by their input0
  // To do that in one pass(counting sort), we should store the min/max of
  // input0 in each subsublayer; that allow to save memory, but we could also
  // use the fact that each subsublayer has q as max size; or even
  // maxsize(layer[N]) = sum(size, layerN-1, layer0)
  using XorGates = std::vector<std::vector<GateID>>;
  using Layer = std::pair<NonXorGates, XorGates>;

  std::vector<Layer> layers(nb_layers_);

  // XorGates are accessed non-sequentially, so we must init it beforehand
  for (unsigned char i = 0, layers_size = layers.size(); i < layers_size; ++i) {
    // skip the layer is it contains no XOR gate
    // In this case, the "min" would still be at its initial value
    if (layer_input0_minmax[i].first == std::numeric_limits<uint16_t>::max()) {
      continue;
    }

    // XorGates is a vector<vector>
    // the outer one depends on how many "input0" values there are in this layer
    // NOTE: if min = max: it still means we have ONE input0: therefore + 1
    layers[i].second.resize(1 + layer_input0_minmax[i].second -
                            layer_input0_minmax[i].first);

    // the inner one is the list of gates for each input0
    // we can't know in advance how many there are, so we reserve() for the
    // worst case
    // layerCount - layerNonXORCount = count of XOR gate in this layer
    for (auto &layer_xor_gates : layers[i].second) {
      layer_xor_gates.reserve(layer_counts_[i] - layer_nonxor_counts_[i]);
    }

    // we might as well reserve for the NonXorGate too
    layers[i].first.reserve(layer_nonxor_counts_[i]);
  }

#ifndef NDEBUG
  std::unordered_set<uint32_t> CHECK_layer_gates;
#endif

  // Construct the 4 layers
  // Also set the layerCount, nonXorCount & layerNonXORCount
  // NOTE: here layer can refer to two different things:
  // - it can be the gate's layer: needed to update layerCount etc
  // - or the gate's output's wiring layer: to construct the "layer" vector,
  // for Step 3
  for (uint32_t i = 0; i < nb_gates_; i++) {
    uint32_t gate_input0 = garbledCircuit.GetGateInput0(i);
    uint32_t gate_output = garbledCircuit.GetGateOutput(i);
    interstellar::SkcdGateType gate_type = garbledCircuit.GetGateType(i);

    int outputLayer = wiring[gate_output].layer;

    assert(CHECK_layer_gates.find(i) == CHECK_layer_gates.end() &&
           "CHECK_layer_gates: duplicate!");

    // "first non-xor gates, then xorgates"
    if (gate_type != interstellar::SkcdGateType::XOR) {
      layers[outputLayer].first.emplace_back(i);
#ifndef NDEBUG
      CHECK_layer_gates.emplace(i);
#endif
    } else {
      // To save some memory, we start the the vect at min(all input0 for
      // current layer XOR gates)
      // eg: if in the current layer, input0 is between 28 & 32: vect[0] will
      // correspond to input0 = 28
      size_t input0_index =
          gate_input0 - layer_input0_minmax[outputLayer].first;
      layers[outputLayer].second[input0_index].emplace_back(i);
#ifndef NDEBUG
      CHECK_layer_gates.emplace(i);
#endif
    }
  }

  assert(CHECK_layer_gates.size() == nb_gates_ &&
         "CHECK_layer_gates: sizes mistmach!");

  // Now we can finally construct an ordered list of gates
  // The "struct" we have built is already sorted, so we only need to iterate
  std::vector<GateID> newGatesId;
  newGatesId.reserve(nb_gates_);

  for (const auto &layer : layers) {
    // first NON-XOR gates
    for (GateID layer_non_xor_gate : layer.first) {
      assert(static_cast<size_t>(layer_non_xor_gate) <
                 static_cast<size_t>(std::numeric_limits<GateID>::max()) &&
             "layer_non_xor_gate does not fit as newGatesId element!");
      newGatesId.push_back(layer_non_xor_gate);
    }

    // then XOR gates
    for (const auto &layer_xor_gates : layer.second) {
      for (GateID layer_xor_gate : layer_xor_gates) {
        assert(static_cast<size_t>(layer_xor_gate) <
                   static_cast<size_t>(std::numeric_limits<GateID>::max()) &&
               "layer_non_xor_gate does not fit as newGatesId element!");
        newGatesId.push_back(layer_xor_gate);
      }
    }
  }

  assert(newGatesId.size() == static_cast<size_t>(nb_gates_) &&
         "newGatesId: wrong size!");
  assert(
      std::unordered_set<GateID>(newGatesId.begin(), newGatesId.end()).size() ==
          newGatesId.size() &&
      "newGatesId: duplicates!");

  // 3 - Setup wires placeholder and gates / garbletable
  garbled_gates_.reserve(nb_gates_);

  for (auto &i : garbled_table_) {
    i.resize(non_xor_count_);
  }

  // construct output's "garbledGates" & "garbledTable"
  for (uint32_t i = 0, garbledTableIndex = 0; i < nb_gates_; i++) {
    GateID newId = newGatesId[i];

    uint32_t gate_input0 = garbledCircuit.GetGateInput0(newId);
    uint32_t gate_input1 = garbledCircuit.GetGateInput1(newId);
    uint32_t gate_output = garbledCircuit.GetGateOutput(newId);
    interstellar::SkcdGateType gate_type = garbledCircuit.GetGateType(newId);

    bool is_xor = (gate_type == interstellar::SkcdGateType::XOR);

    // "pack" the gate as a single 64 bits(we already know(=assert) that each
    // component fits on 21bits)
    // The MSB is set to 1 if it XOR gate, 0 if not.
    // Previously each field was masked with 0x1FFFFFllu =
    // 0b111111111111111111111 (21 bits)
    // But it is useless because we already asserted they fit.
    // TODO(garbledGates) replace this packing by basic struct(+bitfield if
    // useful)
    garbled_gates_.push_back((static_cast<uint64_t>(is_xor) << 63) |
                             (static_cast<uint64_t>(gate_input0) << 42) |
                             (static_cast<uint64_t>(gate_input1) << 21) |
                             (static_cast<uint64_t>(gate_output)));

    if (!is_xor) {
      const auto &garbled_table = garbledCircuit.GetGarbledTable(newId);

      assert(garbledTableIndex < non_xor_count_ &&
             "garbledTableIndex >= nonXorCount!");
      garbled_table_[0][garbledTableIndex] = garbled_table.table[0];
      garbled_table_[1][garbledTableIndex] = garbled_table.table[1];
      garbled_table_[2][garbledTableIndex] = garbled_table.table[2];
      garbled_table_[3][garbledTableIndex] = garbled_table.table[3];
      garbledTableIndex++;
    }
  }

  assert(static_cast<size_t>(nb_outputs_) == garbledCircuit.GetNbOutputs() &&
         "m: wrong size!");
  outputs_.swap(garbledCircuit.GetOutputs());

  auto &input_labels = garbledCircuit.GetInputLabels();
  assert(input_labels.size() == static_cast<size_t>(2 * (nb_inputs_ + 1)) &&
         "inputLabels: size does not match!");
  input_labels_.swap(input_labels);

  auto &output_labels = garbledCircuit.GetOuputsLabels();
  assert(output_labels.size() == static_cast<size_t>(2 * nb_outputs_) &&
         "outputLabels: size does not match!");
  output_labels_.swap(output_labels);
}

/**
 * This is used by:
 * - to prepare the Packmsg
 * - [dev/test] when evaluating a circuit directly(ie WITHOUT a .packmsg)
 *
 * input_bits: vector of 0/1
 */
std::vector<Block> ParallelGarbledCircuit::ExtractLabels(
    const std::vector<uint8_t> &input_bits) const {
  assert(input_bits.size() == nb_inputs_ && "input size MUST nb nb_inputs!");

  std::vector<Block> extracted_labels;

  size_t input_bits_size = input_bits.size();

  // TODO? n+1 is needed for the Packmsg, cf patch_inputs
  extracted_labels.reserve(input_bits_size);

  for (unsigned int i = 0; i < input_bits_size; i++) {
    assert(input_bits[i] == 0 || input_bits[i] == 1);
    extracted_labels.push_back(input_labels_[2 * i + input_bits[i]]);
  }

  assert(extracted_labels.size() == input_bits_size &&
         "ExtractLabels: wrong final size!");

  return extracted_labels;
}
