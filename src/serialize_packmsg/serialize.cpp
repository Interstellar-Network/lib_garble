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

#include "serialize.h"

#include <glog/logging.h>

#include "packmsg.pb.h"
#include "prepackmsg.pb.h"

namespace {

void WriteProtobufPrePackmsg(
    const interstellar::packmsg::PrePackmsg &pre_packmsg,
    interstellarpbcircuits::PrePackmsg *protobuf_pre_packmsg) {
  for (const auto &block : pre_packmsg.GetLabels()) {
    interstellarpbcircuits::Block *pb_block =
        protobuf_pre_packmsg->add_labels();
    pb_block->set_high(block.GetHigh());
    pb_block->set_low(block.GetLow());
  }

  protobuf_pre_packmsg->mutable_xormask()->Assign(
      pre_packmsg.GetXormask().cbegin(), pre_packmsg.GetXormask().cend());

  for (auto const &[key, val] : pre_packmsg.GetConfig()) {
    (*protobuf_pre_packmsg->mutable_config())[key] = val;
  }
}

interstellar::packmsg::PrePackmsg ReadProtobufPrePackmsg(
    const interstellarpbcircuits::PrePackmsg &protobuf_pre_packmsg) {
  // std::transform b/c we MUST convert interstellarpbcircuits::Block -> Block
  std::vector<Block> labels;
  std::transform(protobuf_pre_packmsg.labels().cbegin(),
                 protobuf_pre_packmsg.labels().cend(),
                 std::back_inserter(labels),
                 [](const interstellarpbcircuits::Block &pb_block) {
                   return Block(pb_block.high(), pb_block.low());
                 });

  std::vector<uint8_t> xormask(protobuf_pre_packmsg.xormask().cbegin(),
                               protobuf_pre_packmsg.xormask().cend());

  absl::flat_hash_map<std::string, uint32_t> config;
  for (auto const &[key, val] : protobuf_pre_packmsg.config()) {
    config.try_emplace(key, val);
  }

  return interstellar::packmsg::PrePackmsg(
      std::move(labels), std::move(xormask), std::move(config));
}

void WriteProtobufPackmsg(const interstellar::packmsg::Packmsg &packmsg,
                          interstellarpbcircuits::Packmsg *protobuf_packmsg) {
  for (const auto &block : packmsg.GetGarbledValues()) {
    interstellarpbcircuits::Block *pb_block =
        protobuf_packmsg->add_garbled_values();
    pb_block->set_high(block.GetHigh());
    pb_block->set_low(block.GetLow());
  }

  protobuf_packmsg->mutable_xormask()->Assign(packmsg.GetXormask().cbegin(),
                                              packmsg.GetXormask().cend());
}

interstellar::packmsg::Packmsg ReadProtobufPackmsg(
    const interstellarpbcircuits::Packmsg &protobuf_packmsg) {
  // std::transform b/c we MUST convert interstellarpbcircuits::Block -> Block
  std::vector<Block> garbled_values;
  std::transform(protobuf_packmsg.garbled_values().cbegin(),
                 protobuf_packmsg.garbled_values().cend(),
                 std::back_inserter(garbled_values),
                 [](const interstellarpbcircuits::Block &pb_block) {
                   return Block(pb_block.high(), pb_block.low());
                 });

  std::vector<uint64_t> xormask(protobuf_packmsg.xormask().cbegin(),
                                protobuf_packmsg.xormask().cend());

  return interstellar::packmsg::Packmsg(garbled_values, xormask);
}

}  // anonymous namespace

namespace interstellar::packmsg {

/**
 * Serialize to a buffer using Protobuf
 */
std::string SerializePrepackmsg(const PrePackmsg &pre_packmsg) {
  interstellarpbcircuits::PrePackmsg protobuf_pre_packmsg;
  WriteProtobufPrePackmsg(pre_packmsg, &protobuf_pre_packmsg);

  std::string buf;
  protobuf_pre_packmsg.SerializeToString(&buf);

  return buf;
}

/**
 * Deserialize from a buffer
 */
PrePackmsg DeserializePrepackmsgFromBuffer(const std::string &buffer) {
  interstellarpbcircuits::PrePackmsg protobuf_pre_packmsg;
  auto ok = protobuf_pre_packmsg.ParseFromString(buffer);
  if (!ok) {
    LOG(ERROR) << "DeserializeFromBuffer: parsing failed";
    throw std::runtime_error("DeserializeFromBuffer: parsing failed");
  }

  return ReadProtobufPrePackmsg(protobuf_pre_packmsg);
}

/**
 * Serialize Packmsgto a buffer using Protobuf
 */
std::string SerializePackmsg(const Packmsg &packmsg) {
  interstellarpbcircuits::Packmsg protobuf_packmsg;
  WriteProtobufPackmsg(packmsg, &protobuf_packmsg);

  std::string buf;
  protobuf_packmsg.SerializeToString(&buf);

  return buf;
}

/**
 * Deserialize Packmsg from a buffer
 */
Packmsg DeserializePackmsgFromBuffer(const std::string &buffer) {
  interstellarpbcircuits::Packmsg protobuf_packmsg;
  auto ok = protobuf_packmsg.ParseFromString(buffer);
  if (!ok) {
    LOG(ERROR) << "DeserializeFromBuffer: parsing failed";
    throw std::runtime_error("DeserializeFromBuffer: parsing failed");
  }

  return ReadProtobufPackmsg(protobuf_packmsg);
}

}  // namespace interstellar::packmsg
