#include "serialize.h"

#include <glog/logging.h>

#include "circuit.pb.h"

// TODO?
#undef OUTPUTS_ENCODE_NEW

namespace {

#ifdef OUTPUTS_ENCODE_NEW

std::vector<std::pair<unsigned int, size_t>> VectorGetConsecutiveAscending(
    const std::vector<uint32_t> &vect) {
  std::vector<std::pair<unsigned int, size_t>> res;

  const size_t vect_size = vect.size();

  int last_value = vect[0];
  size_t count = 1;

  for (unsigned int i = 1; i < vect_size; ++i) {  // can skip the first one

    int current_value = vect[i];

    if (current_value != last_value + 1) {
      // plateau end
      res.emplace_back(last_value, count);

      // reset
      count = 1;
      last_value = current_value;
    } else {
      count++;
    }
  }

  // don't forget the last plateau
  res.emplace_back(last_value, count);

  return res;
}

#endif  // OUTPUTS_ENCODE_NEW

void WriteProtobuf(
    const interstellar::garble::ParallelGarbledCircuit
        &parallel_garbled_circuit,
    interstellarpbcircuits::ParallelGarbledCircuit *protobuf_pgc) {
  protobuf_pgc->set_n(parallel_garbled_circuit.nb_inputs_);
  protobuf_pgc->set_m(parallel_garbled_circuit.nb_outputs_);
  protobuf_pgc->set_q(parallel_garbled_circuit.nb_gates_);
  protobuf_pgc->set_r(parallel_garbled_circuit.nb_wires_);
  protobuf_pgc->set_nblayers(parallel_garbled_circuit.nb_layers_);
  protobuf_pgc->set_nonxorcount(parallel_garbled_circuit.non_xor_count_);

  for (int c : parallel_garbled_circuit.layer_counts_) {
    protobuf_pgc->add_layercount(c);
  }

  for (int c : parallel_garbled_circuit.layer_nonxor_counts_) {
    protobuf_pgc->add_layernonxorcount(c);
  }

  for (const auto &block : parallel_garbled_circuit.input_labels_) {
    interstellarpbcircuits::Block *pb_block = protobuf_pgc->add_input_labels();
    pb_block->set_high(block.GetHigh());
    pb_block->set_low(block.GetLow());
  }

  for (const auto &block : parallel_garbled_circuit.output_labels_) {
    interstellarpbcircuits::Block *pb_block = protobuf_pgc->add_output_labels();
    pb_block->set_high(block.GetHigh());
    pb_block->set_low(block.GetLow());
  }

  // Don't use set_allocated_garbledgates, it will crash in the dtor
  for (uint64_t garbledGate : parallel_garbled_circuit.garbled_gates_) {
    protobuf_pgc->add_garbledgates(garbledGate);
  }

  interstellarpbcircuits::GarbledTables *garble_tables =
      protobuf_pgc->mutable_garbletables();
  for (u_int32_t gid = 0; gid < parallel_garbled_circuit.non_xor_count_;
       gid++) {
    interstellarpbcircuits::Block *gt0 = garble_tables->add_gt0();
    gt0->set_high(parallel_garbled_circuit.garbled_table_[0][gid].GetHigh());
    gt0->set_low(parallel_garbled_circuit.garbled_table_[0][gid].GetLow());

    interstellarpbcircuits::Block *gt1 = garble_tables->add_gt1();
    gt1->set_high(parallel_garbled_circuit.garbled_table_[1][gid].GetHigh());
    gt1->set_low(parallel_garbled_circuit.garbled_table_[1][gid].GetLow());

    interstellarpbcircuits::Block *gt2 = garble_tables->add_gt2();
    gt2->set_high(parallel_garbled_circuit.garbled_table_[2][gid].GetHigh());
    gt2->set_low(parallel_garbled_circuit.garbled_table_[2][gid].GetLow());

    interstellarpbcircuits::Block *gt3 = garble_tables->add_gt3();
    gt3->set_high(parallel_garbled_circuit.garbled_table_[3][gid].GetHigh());
    gt3->set_low(parallel_garbled_circuit.garbled_table_[3][gid].GetLow());
  }

#ifdef OUTPUTS_ENCODE_NEW
#error "mutable_outputs not supported cf VectorGetConsecutiveAscending"
#else
  protobuf_pgc->mutable_outputs()->Assign(
      parallel_garbled_circuit.outputs_.cbegin(),
      parallel_garbled_circuit.outputs_.cend());
#endif  // OUTPUTS_ENCODE_NEW

  // Don't use set_allocated_globalkey, it will crash in the dtor
  interstellarpbcircuits::Block *global_key = protobuf_pgc->mutable_globalkey();
  global_key->set_high(parallel_garbled_circuit.global_key_.GetHigh());
  global_key->set_low(parallel_garbled_circuit.global_key_.GetLow());
}

void ReadProtobuf(
    const interstellarpbcircuits::ParallelGarbledCircuit &protobuf_pgc,
    interstellar::garble::ParallelGarbledCircuit *parallel_garbled_circuit) {
  parallel_garbled_circuit->nb_inputs_ = protobuf_pgc.n();
  parallel_garbled_circuit->nb_outputs_ = protobuf_pgc.m();
  parallel_garbled_circuit->nb_gates_ = protobuf_pgc.q();
  parallel_garbled_circuit->nb_wires_ = protobuf_pgc.r();
  parallel_garbled_circuit->nb_layers_ = protobuf_pgc.nblayers();
  parallel_garbled_circuit->non_xor_count_ = protobuf_pgc.nonxorcount();

  parallel_garbled_circuit->layer_counts_.assign(
      protobuf_pgc.layercount().cbegin(), protobuf_pgc.layercount().cend());

  parallel_garbled_circuit->layer_nonxor_counts_.assign(
      protobuf_pgc.layernonxorcount().cbegin(),
      protobuf_pgc.layernonxorcount().cend());

  parallel_garbled_circuit->garbled_gates_.assign(
      protobuf_pgc.garbledgates().cbegin(), protobuf_pgc.garbledgates().cend());

  // std::transform b/c we MUST convert interstellarpbcircuits::Block -> Block

  std::transform(protobuf_pgc.input_labels().cbegin(),
                 protobuf_pgc.input_labels().cend(),
                 std::back_inserter(parallel_garbled_circuit->input_labels_),
                 [](const interstellarpbcircuits::Block &pb_block) {
                   return Block(pb_block.high(), pb_block.low());
                 });

  std::transform(protobuf_pgc.output_labels().cbegin(),
                 protobuf_pgc.output_labels().cend(),
                 std::back_inserter(parallel_garbled_circuit->output_labels_),
                 [](const interstellarpbcircuits::Block &pb_block) {
                   return Block(pb_block.high(), pb_block.low());
                 });

  std::transform(
      protobuf_pgc.garbletables().gt0().cbegin(),
      protobuf_pgc.garbletables().gt0().cend(),
      std::back_inserter(parallel_garbled_circuit->garbled_table_[0]),
      [](const interstellarpbcircuits::Block &pb_block) {
        return Block(pb_block.high(), pb_block.low());
      });
  std::transform(
      protobuf_pgc.garbletables().gt1().cbegin(),
      protobuf_pgc.garbletables().gt1().cend(),
      std::back_inserter(parallel_garbled_circuit->garbled_table_[1]),
      [](const interstellarpbcircuits::Block &pb_block) {
        return Block(pb_block.high(), pb_block.low());
      });
  std::transform(
      protobuf_pgc.garbletables().gt2().cbegin(),
      protobuf_pgc.garbletables().gt2().cend(),
      std::back_inserter(parallel_garbled_circuit->garbled_table_[2]),
      [](const interstellarpbcircuits::Block &pb_block) {
        return Block(pb_block.high(), pb_block.low());
      });
  std::transform(
      protobuf_pgc.garbletables().gt3().cbegin(),
      protobuf_pgc.garbletables().gt3().cend(),
      std::back_inserter(parallel_garbled_circuit->garbled_table_[3]),
      [](const interstellarpbcircuits::Block &pb_block) {
        return Block(pb_block.high(), pb_block.low());
      });

#ifdef OUTPUTS_ENCODE_NEW
#error "mutable_outputs not supported cf VectorGetConsecutiveAscending"
#else
  parallel_garbled_circuit->outputs_.assign(protobuf_pgc.outputs().cbegin(),
                                            protobuf_pgc.outputs().cend());
#endif  // OUTPUTS_ENCODE_NEW

  parallel_garbled_circuit->global_key_ =
      Block(protobuf_pgc.globalkey().high(), protobuf_pgc.globalkey().low());
}

}  // anonymous namespace

namespace interstellar {

namespace garble {

/**
 * Serialize to a buffer using Protobuf
 */
std::string Serialize(const ParallelGarbledCircuit &parallel_garbled_circuit) {
  interstellarpbcircuits::ParallelGarbledCircuit protobuf_pgc;
  WriteProtobuf(parallel_garbled_circuit, &protobuf_pgc);

  std::string buf;
  protobuf_pgc.SerializeToString(&buf);

  return buf;
}

/**
 * Serialize to a file using Protobuf
 */
void Serialize(const ParallelGarbledCircuit &parallel_garbled_circuit,
               boost::filesystem::path pgarbled_output_path) {
  interstellarpbcircuits::ParallelGarbledCircuit protobuf_pgc;
  WriteProtobuf(parallel_garbled_circuit, &protobuf_pgc);

  std::fstream output(pgarbled_output_path.generic_string(),
                      std::ios::out | std::ios::trunc | std::ios::binary);
  auto ok = protobuf_pgc.SerializeToOstream(&output);
  if (!ok) {
    throw std::runtime_error("Serialization failed");
  }

  LOG(INFO) << "wrote pgarbled : " << pgarbled_output_path;
}

/**
 * Deserialize from a file
 */
void DeserializeFromFile(ParallelGarbledCircuit *parallel_garbled_circuit,
                         boost::filesystem::path pgarbled_input_path) {
  std::fstream input_stream(pgarbled_input_path.generic_string(),
                            std::ios::in | std::ios::binary);
  // if (!input_stream) {
  //   LOG(ERROR) << "GarbledCircuit: invalid file : " << skcd_input_path;
  //   throw std::runtime_error("GarbledCircuit: input_stream failed");
  // }

  interstellarpbcircuits::ParallelGarbledCircuit protobuf_pgc;
  auto ok = protobuf_pgc.ParseFromIstream(&input_stream);
  if (!ok) {
    LOG(ERROR) << "DeserializeFromFile: parsing failed : "
               << pgarbled_input_path;
    throw std::runtime_error("DeserializeFromFile: parsing failed");
  }

  ReadProtobuf(protobuf_pgc, parallel_garbled_circuit);
}

/**
 * Deserialize from a buffer
 */
void DeserializeFromBuffer(ParallelGarbledCircuit *parallel_garbled_circuit,
                           const std::string &buffer) {
  interstellarpbcircuits::ParallelGarbledCircuit protobuf_pgc;
  auto ok = protobuf_pgc.ParseFromString(buffer);
  if (!ok) {
    LOG(ERROR) << "DeserializeFromBuffer: parsing failed";
    throw std::runtime_error("DeserializeFromBuffer: parsing failed");
  }

  ReadProtobuf(protobuf_pgc, parallel_garbled_circuit);
}

}  // namespace garble

}  // namespace interstellar