//
// Created by nathanprat on 5/12/16.
//

#include <absl/base/attributes.h>
#include <glog/logging.h>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <numeric>

#include "aes-sse.h"
#include "defines.h"
#include "garble_utils.h"
#include "justGarble.h"
#include "skcd.pb.h"
#include "utils_block.h"
#include "xor_gate_processor.h"
#include "xor_graph.h"

namespace {

struct Wire {
  Block label0, label1;

  /**
   * This ctor is used when initializing the wires, in garbleCircuit()
   */
  Wire(const Block &label0, const Block &label1)
      : label0(label0), label1(label1) {}

  // DEBUG ONLY needed for assert
  bool operator==(const Wire &other) const {
    return label0 == other.label0 && label1 == other.label1;
  }
};

/**
 * input: a XOR gate input0 or input1
 *
 * Instead of being random, an input's label1 is R ^ label0.
 * This is related to the free XOR optimization, and indirectly to "Multiple R
 * in Garble Circuit", see Confluence.
 *
 * read: input(0/1) label0
 * write: input(0/1) label1
 */
ABSL_ATTRIBUTE_ALWAYS_INLINE inline void SetXorGateInput(
    const Block &R, std::vector<Wire> *wires, uint32_t input) {
  (*wires)[input].label1.Xor(R, (*wires)[input].label0);
}

/**
 * output: a XOR gate output
 *
 * read: input0 label0, input1 label0, input1 label1
 * write: output label0, output label1
 */
ABSL_ATTRIBUTE_ALWAYS_INLINE inline void SetXorGateOutput(
    uint32_t output, uint32_t input0, uint32_t input1,
    std::vector<Wire> *wires) {
  assert(output != input0 && output != input1 &&
         "SetXorGateOutput: output is an input!");

  (*wires)[output].label0.Xor((*wires)[input0].label0, (*wires)[input1].label0);
  (*wires)[output].label1.Xor((*wires)[input0].label0, (*wires)[input1].label1);
}

}  // anonymous namespace

void GarbledCircuit::garbleCircuit() { garbleCircuit(time(nullptr)); }

void GarbledCircuit::garbleCircuit(uint32_t seed) {
  AES_KEY aes_key;

  std::vector<Wire> wires;

  {
    static constexpr int64_t rnds = 10;

    // TODO(garble) proper random(and probably remove srand_sse + intel random
    // SSE)
    // TODO allow seed to be passed as arg, useful for tests
    srand_sse(seed);

    // sched: in JustGarble sources(official ones), it points to "K.rd_key" of
    // an UNinitialized DKCipherContext
    Block sched[15];
    for (auto &i : sched) {
      i.Randomize();
    }

    Block key = Block::NewRandom();
    Block rkey = Block::NewRandom();
    AES_KEY KR;
    AES_set_encrypt_key(reinterpret_cast<unsigned char *>(&rkey), 128, &KR);
    // WTF is this?
    // It was in original lib_server code; so we keep to be able to compare
    // generated files b/w versions(removing it would modify wires values)
    const __m128i *sched2 = ((__m128i *)(KR.rd_key));

    global_key_ = key;

    AES_set_encrypt_key(reinterpret_cast<unsigned char *>(&key), 128, &aes_key);

    // Compute labels for every wire
    // First set all wires to random states

    // construct 'wires'
    wires.reserve(nb_wires_);
    for (uint32_t i = 0; i < nb_wires_; i++) {
      Block tweak = Block::NewRandom();
      Block R = Block::NewR();
      // TODO remove; already done by NewR...
      // WTF is this?
      // It was in original lib_server code; so we keep to be able to compare
      // generated files b/w versions(removing it would modify wires values)
      *((char *)&(R)) = 1;

      // TODO investigate; pretty sure there is something wrong with the whole
      // KR vs aes_key, rkey vs key, and the AES call here
      tweak.Aes(rnds, ((__m128i *)&sched[0]), sched2);

      wires.emplace_back(tweak, R.Xor(tweak));
    }
  }

  // "Now deal with XOR !
  // Collect all XOR gates
  // Collect all XOR connected wires
  // Create Wire -> Gates map"
  // Previously: there was and allXORWires(unordered_set), but not used so
  // removed; was costing around ~7k circuits/hour for nothing!
  // It used to be an unordered_set, but it can be a basic vector, see the loop
  // on "q" below: no duplicate.
  // Replacing unordered_set by vector: +2-3k circuits/hour
  std::vector<char> allXORGates;
  // NOT all gates are XOR, but most of them(almost all); so "q" works just fine
  allXORGates.reserve(nb_gates_);

  // as we are looping on all gates here, we might as well store the ones which
  // are NON-xor
  // Typically only ~1% are NON-XOR, but take some margin
  std::vector<uint32_t> NONXORGates;
  NONXORGates.reserve(nb_gates_ / 50);

  XorGraph xor_graph(nb_wires_, nb_gates_, gate_inputs0_, gate_inputs1_,
                     gate_outputs_, gate_types_, circuit_data_);

  for (uint32_t i = 0; i < nb_gates_; i++) {
    // skip non-XOR gates
    if (gate_types_[i] != interstellar::SkcdGateType::XOR) {
      allXORGates.push_back(0);
      NONXORGates.push_back(i);
      continue;
    }

    allXORGates.push_back(1);
  }

  XorGateProcessor xor_gate_processor(std::move(allXORGates));

  // "Create RemainingGates from all XOR Gates
  // Start with first remaining gate, find all connected Gates recursively
  // (there are no loops in a DAG), choose a R, set the labels, remove from
  // remaining"

  // will be break-ed out when XorGateProcessor::GetNextSeedGate() returns -1
  // Set the garbled_gate for XOR gates
  while (true) {
    int seedGates = xor_gate_processor.GetNextSeedGate();

    // nothing more to process
    if (seedGates < 0) {
      break;
    }

    std::vector<uint32_t> subgraph_gates = xor_graph.FindAllGates(seedGates);

    Block R = Block::NewRandom();
    // TODO remove; already done by NewR...
    // WTF is this?
    // It was in original lib_server code; so we keep to be able to compare
    // generated files b/w versions(removing it would modify wires values)
    *((char *)&(R)) = 1;

    // "Setting wires"
    //
    // Modifies wires:
    // - input0 & input1:   label1
    // - output:            label0 & label1
    // NOTE: Previously "FindAllGates" returned a list of gates(unique=set)
    // and the algo looked like:
    // for each gate:
    // - gate_input0.label1 = R XOR gate_input0.label0
    // - gate_input1.label1 = R XOR gate_input1.label0
    // - gate_output.label(0/1) = some other XOR ops
    //
    // But in a "subgraph", most input0/input1 are shared by multiple gates, so
    // it was overwriting a lot!
    // eg on avg an input was written 100-200+ times to the same value!
    //
    // NOTE2: there is NO duplicate between output, both in this loop, and b/w
    // calls to "FindAllGates".
    // NOTE3: reminder: an output CAN appear as input IN A GIVEN "SUBGRAPH"
    // NOTE4: reminder: "FindAllGates" return a UNIQUE SET of gates b/w calls,
    // which also means NO input/output will be duplicated b/w the results.

    for (auto gate_id_it = subgraph_gates.begin();
         gate_id_it != subgraph_gates.end(); ++gate_id_it) {
      uint32_t input0 = gate_inputs0_[*gate_id_it];
      uint32_t input1 = gate_inputs1_[*gate_id_it];
      uint32_t output = gate_outputs_[*gate_id_it];

      // TODO(garble) Most inputs are shared b/w gates, so try to avoid
      // overwriting them to the same value repeatedly. We use the fact that
      // gates(and so also the inputs) are mostly ordered, and detect plateaux
      // here. if ((gate_id_it != subgraph_gates.begin()) &&
      //     (input0 != gate_inputs0_[*(gate_id_it - 1)])) {
      //   SetXorGateInput(R, &wires, input0);
      // }
      // if ((gate_id_it != subgraph_gates.begin()) &&
      //     (input1 != gate_inputs1_[*(gate_id_it - 1)])) {
      //   SetXorGateInput(R, &wires, input1);
      // }
      SetXorGateInput(R, &wires, input0);
      SetXorGateInput(R, &wires, input1);

      SetXorGateOutput(output, input0, input1, &wires);
    }

    // remove the gates from the "to be processed" set
    xor_gate_processor.MarkAsProcessed(subgraph_gates);
  }

  // Now process the NON-xor gates
  // It will set:
  // - the garbled_gate for the NON-XOR gate(so those are set for both XOR(see
  // above) and NON-XOR)
  // - the garbled_table; it is set ONLY for NON-XOR gates
  // Careful with indexing
  // - we know we have NONXORGates.size() elements to add
  // - BUT they are NOT consecutive: toParallelGarbleCircuit needs to access
  // them in a non-sequential order.
  // --> we need to a map: gate_id -> garble_table_index
  // Previously we were allocating garbled_table_ for the whole gates so it was
  // not needed, but this was wasteful b/c we only need GarbledTable for
  // NON-XOR gates, and those are ~1% of the gates.
  {
    Block blocks[4];
    Block keys[4];

    garbled_table_.reserve(NONXORGates.size());

    // NOTE: the IDs('non_xor_gate_idx' below) are mostly sequential, so we DO
    // NOT need a hash-map. A vector will do.
    // eg(new_size2): NONXORGates.size() = 712, id_min = 0, id_max = 767
    garbled_table_index_map_.resize(NONXORGates.size() * 2, -1);

    size_t non_xor_counter = 0;

    for (auto non_xor_gate_idx : NONXORGates) {
      assert(non_xor_gate_idx < nb_gates_ && "NONXORGates: out of range!");

      uint32_t input0 = gate_inputs0_[non_xor_gate_idx];
      uint32_t input1 = gate_inputs1_[non_xor_gate_idx];
      uint32_t output = gate_outputs_[non_xor_gate_idx];
      int type = static_cast<int>(gate_types_[non_xor_gate_idx]);

      assert(type != XORGATE && "NONXORGates: contains a XORGATE!");

      Block tweak(((static_cast<uint64_t>(input0) << 42llu) |
                   (static_cast<uint64_t>(input1) << 21llu) |
                   (static_cast<uint64_t>(output))),
                  static_cast<int64_t>(0));

      int64_t lsb0 = wires[input0].label0.GetLsb();
      int64_t lsb1 = wires[input1].label0.GetLsb();

      Block A0, A1, B0, B1;
      A0.Double(wires[input0].label0);
      A1.Double(wires[input0].label1);
      B0.Quadruple(wires[input1].label0);
      B1.Quadruple(wires[input1].label1);

      keys[0].Xor(A0.Xor(B0), tweak);
      keys[1].Xor(A0.Xor(B1), tweak);
      keys[2].Xor(A1.Xor(B0), tweak);
      keys[3].Xor(A1.Xor(B1), tweak);

      Block *temp[2];
      temp[0] = &wires[output].label0;
      temp[1] = &wires[output].label1;

      int bp = 0;
      blocks[0].Xor(keys[0], (*(temp[(type & (1 << bp)) >> bp])));
      bp++;
      blocks[1].Xor(keys[1], (*(temp[(type & (1 << bp)) >> bp])));
      bp++;
      blocks[2].Xor(keys[2], (*(temp[(type & (1 << bp)) >> bp])));
      bp++;
      blocks[3].Xor(keys[3], (*(temp[(type & (1 << bp)) >> bp])));

      {
        unsigned rnds = aes_key.rounds;
        const __m128i *sched = aes_key.rd_key;

        keys[0].Aes(rnds, sched);
        keys[1].Aes(rnds, sched);
        keys[2].Aes(rnds, sched);
        keys[3].Aes(rnds, sched);
      }

      unsigned char lsb_index0 = 2 * lsb0 + lsb1;
      unsigned char lsb_index1 = 2 * lsb0 + 1 - lsb1;
      unsigned char lsb_index2 = 2 * (1 - lsb0) + lsb1;
      unsigned char lsb_index3 = 2 * (1 - lsb0) + (1 - lsb1);

      // 4 possibilities total
      // each is between 0-3(included), and all of them are different
      assert((lsb_index0 == 3 && lsb_index1 == 2 && lsb_index2 == 1 &&
              lsb_index3 == 0) ||
             (lsb_index0 == 0 && lsb_index1 == 1 && lsb_index2 == 2 &&
              lsb_index3 == 3) ||
             (lsb_index0 == 1 && lsb_index1 == 0 && lsb_index2 == 3 &&
              lsb_index3 == 2) ||
             (lsb_index0 == 2 && lsb_index1 == 3 && lsb_index2 == 0 &&
              lsb_index3 == 1));

      // Previously it was done the other way around:
      // eg: table[lsb_index0] = Xor(block[0], keys[0])
      // But that required garbleTables to be already initialized(eg resize)
      // So we do it the other way around now, and use reserve + push_back
      garbled_table_.emplace_back(blocks[lsb_index0].Xor(keys[lsb_index0]),
                                  blocks[lsb_index1].Xor(keys[lsb_index1]),
                                  blocks[lsb_index2].Xor(keys[lsb_index2]),
                                  blocks[lsb_index3].Xor(keys[lsb_index3]));

      // add the entry to the index map
      garbled_table_index_map_[non_xor_gate_idx] = non_xor_counter;
      non_xor_counter++;
    }

    assert(garbled_table_.size() == NONXORGates.size());
    assert(non_xor_counter == NONXORGates.size());
  }

  // Set n+1 input labels
  // Will be copied as-is(ie swapped) to the PGC
  uint32_t nb_inputs_labels = 2 * (nb_inputs_ + 1);
  input_labels_.reserve(nb_inputs_labels);
  for (uint32_t i = 0; i < nb_inputs_labels; i += 2) {
    input_labels_.emplace_back(wires[i / 2].label0);
    input_labels_.emplace_back(wires[i / 2].label1);
  }

  // Set the outputLabels
  // Will be copied as-is(ie swapped) to the PGC
  // NOTE: 2 emplaced each iteration, so loop until 'nb_outputs_' NOT '2 *
  // nb_outputs_'
  output_labels_.reserve(2 * nb_outputs_);
  for (uint32_t i = 0; i < nb_outputs_; i++) {
    output_labels_.emplace_back(wires[outputs_[i]].label0);
    output_labels_.emplace_back(wires[outputs_[i]].label1);
  }

  assert(output_labels_.size() == static_cast<size_t>(2 * nb_outputs_) &&
         "outputLabels: wrong size!");
}

/**
 * Previously: a free function 'skcdToGarbled'
 * "replaces readCircuitFromFile
 * Get a Skcd instance init from a .blif.blif, and return the corresponding
 * GarbledCircuit"
 */
// TODO the deserialization SHOULD be in a separate lib
GarbledCircuit::GarbledCircuit(boost::filesystem::path skcd_input_path) {
  std::fstream input_stream(skcd_input_path.generic_string(),
                            std::ios::in | std::ios::binary);
  // if (!input_stream) {
  //   LOG(ERROR) << "GarbledCircuit: invalid file : " << skcd_input_path;
  //   throw std::runtime_error("GarbledCircuit: input_stream failed");
  // }

  interstellarpbskcd::Skcd skcd_pb;
  auto ok = skcd_pb.ParseFromIstream(&input_stream);
  if (!ok) {
    LOG(ERROR) << "GarbledCircuit: parsing failed : " << skcd_input_path;
    throw std::runtime_error("GarbledCircuit: parsing failed");
  }

  // "lib_python: a msgpacked skcd is ONE list: [MAGIC_SKC0, n, m, q] + A + B +
  // GO + GT + O"

  nb_outputs_ = skcd_pb.m();
  nb_inputs_ = skcd_pb.n();
  nb_gates_ = skcd_pb.q();
  nb_wires_ = skcd_pb.n() + skcd_pb.q() + 2;

  // TODO replace at() by []

  assert(static_cast<u_int32_t>(skcd_pb.b().size()) == skcd_pb.q() &&
         "skcd.B size is NOT skcd.q!");
  assert(static_cast<u_int32_t>(skcd_pb.a().size()) == skcd_pb.q() &&
         "skcd.A size is NOT skcd.q!");
  assert(static_cast<u_int32_t>(skcd_pb.go().size()) == skcd_pb.q() &&
         "skcd.GO size is NOT skcd.q!");
  assert(static_cast<u_int32_t>(skcd_pb.gt().size()) == skcd_pb.q() &&
         "skcd.GT size is NOT skcd.q!");

  gate_inputs0_.assign(skcd_pb.a().begin(), skcd_pb.a().end());
  gate_inputs1_.assign(skcd_pb.b().begin(), skcd_pb.b().end());
  gate_outputs_.assign(skcd_pb.go().begin(), skcd_pb.go().end());
  gate_types_.reserve(skcd_pb.gt_size());
  for (uint32_t gate_type_int : skcd_pb.gt()) {
    gate_types_.emplace_back(interstellar::SkcdGateType(gate_type_int));
  }
  assert(gate_types_.size() == static_cast<u_int32_t>(skcd_pb.gt_size()) &&
         "GT: wrong size!");

  assert(static_cast<u_int32_t>(skcd_pb.o().size()) == skcd_pb.m() &&
         "skcd.O size is NOT skcd.m!");

  outputs_.assign(skcd_pb.o().begin(), skcd_pb.o().end());

  circuit_data_.gate_input_min = skcd_pb.circuit_data().gate_input_min();
  circuit_data_.gate_input_max = skcd_pb.circuit_data().gate_input_max();
  circuit_data_.gate_output_min = skcd_pb.circuit_data().gate_output_min();
  circuit_data_.gate_output_max = skcd_pb.circuit_data().gate_output_max();
  circuit_data_.layer_count.assign(skcd_pb.circuit_data().layer_count().begin(),
                                   skcd_pb.circuit_data().layer_count().end());
  circuit_data_.input_gate_count.assign(
      skcd_pb.circuit_data().input_gate_count().begin(),
      skcd_pb.circuit_data().input_gate_count().end());
}

GarbledCircuit::~GarbledCircuit() = default;

GarbledTable::GarbledTable(Block &&table0, Block &&table1, Block &&table2,
                           Block &&table3) {
  table[0] = table0;
  table[1] = table1;
  table[2] = table2;
  table[3] = table3;
}