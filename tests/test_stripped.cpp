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

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "packmsg_helper.h"
#include "resources.h"
#include "serialize_packmsg/serialize.h"
#include "utils/utils_files.h"

using namespace interstellar;

// IMPORTANT this is the API used by api_garble!
TEST(StrippedTest, DisplayOk) {
  // MUST be a "display" circuit b/c the generic(eg adder etc) DO NOT have a
  // "config"
  auto skcd_buf = interstellar_testing::utils::ReadFile(
      std::filesystem::path(interstellar_testing::data_dir) /
      std::string("display_message_120x52_2digits.skcd.pb.bin"));

  garble::ParallelGarbledCircuit pgc;
  packmsg::PrePackmsg pre_packmsg;
  std::vector<uint8_t> digits{0, 1};
  packmsg::GarbleAndStrippedSkcdFromBuffer(skcd_buf, &pgc, &pre_packmsg,
                                           digits);

  auto prepackmsg_buf = packmsg::SerializePrepackmsg(pre_packmsg);

  ASSERT_NEAR(6722, prepackmsg_buf.size(), 10);
}

// Calling GarbleAndStrippedSkcdFromBuffer should throw/fail gracefully NOT
// crash
TEST(StrippedTest, GenericMustNotCrash) {
  // MUST be a "display" circuit b/c the generic(eg adder etc) DO NOT have a
  // "config"
  auto skcd_buf = interstellar_testing::utils::ReadFile(
      std::filesystem::path(interstellar_testing::data_dir) /
      std::string("adder.skcd.pb.bin"));

  garble::ParallelGarbledCircuit pgc;
  packmsg::PrePackmsg pre_packmsg;
  std::vector<uint8_t> digits;

  EXPECT_THROW(packmsg::GarbleAndStrippedSkcdFromBuffer(skcd_buf, &pgc,
                                                        &pre_packmsg, digits),
               std::logic_error);

  EXPECT_EQ(pgc.nb_gates_, 9);
  // TODO ASSERT for the rest of the PGC?
  //
  // everything else SHOULD NOT have been modified
  EXPECT_EQ(pre_packmsg.GetLabels().size(), 0);
  EXPECT_EQ(pre_packmsg.GetXormask().size(), 0);
  EXPECT_EQ(pre_packmsg.GetConfig().size(), 0);
  EXPECT_EQ(digits.size(), 0);
}

// Test Serialize + Deserialize for PrePackmsg
TEST(StrippedTest, PrePackmsgSerializationOk) {
  // MUST be a "display" circuit b/c the generic(eg adder etc) DO NOT have a
  // "config"
  auto skcd_buf = interstellar_testing::utils::ReadFile(
      std::filesystem::path(interstellar_testing::data_dir) /
      std::string("display_message_120x52_2digits.skcd.pb.bin"));

  garble::ParallelGarbledCircuit pgc;
  packmsg::PrePackmsg pre_packmsg;
  std::vector<uint8_t> digits{0, 1};
  packmsg::GarbleAndStrippedSkcdFromBuffer(skcd_buf, &pgc, &pre_packmsg,
                                           digits);

  auto prepackmsg_buf = packmsg::SerializePrepackmsg(pre_packmsg);
  auto prepackmsg_copy =
      packmsg::DeserializePrepackmsgFromBuffer(prepackmsg_buf);

  EXPECT_EQ(pre_packmsg, prepackmsg_copy);
}

// Test Serialize + Deserialize for Packmsg
TEST(StrippedTest, PackmsgSerializationOk) {
  // MUST be a "display" circuit b/c the generic(eg adder etc) DO NOT have a
  // "config"
  auto skcd_buf = interstellar_testing::utils::ReadFile(
      std::filesystem::path(interstellar_testing::data_dir) /
      std::string("display_message_120x52_2digits.skcd.pb.bin"));

  garble::ParallelGarbledCircuit pgc;
  packmsg::PrePackmsg pre_packmsg;
  std::vector<uint8_t> digits{0, 1};
  packmsg::GarbleAndStrippedSkcdFromBuffer(skcd_buf, &pgc, &pre_packmsg,
                                           digits);

  packmsg::Packmsg packmsg =
      packmsg::PackmsgFromPrepacket(pre_packmsg, "test message");

  auto packmsg_buf = packmsg::SerializePackmsg(packmsg);
  auto packmsg_copy = packmsg::DeserializePackmsgFromBuffer(packmsg_buf);

  EXPECT_EQ(packmsg, packmsg_copy);
}