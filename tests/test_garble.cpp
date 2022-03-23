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

#include "garble_helper.h"
#include "resources.h"
#include "serialize_pgc/serialize.h"
#include "utils/utils_files.h"

using namespace interstellar;

// IMPORTANT this is the API used by api_garble!
TEST(GarbleTest, Adder) {
  auto skcd_buf = interstellar_testing::utils::ReadFile(
      std::filesystem::path(interstellar_testing::data_dir) /
      std::string("adder.skcd.pb.bin"));

  auto pgc = garble::GarbleSkcdFromBuffer(skcd_buf);
  auto pgc_buf = garble::Serialize(pgc);

  ASSERT_NEAR(1017, pgc_buf.size(), 10);
}