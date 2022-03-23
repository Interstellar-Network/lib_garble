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

#include "utils_files.h"

#include <fstream>

namespace interstellar::interstellar_testing::utils {

std::string ReadFile(std::filesystem::path path) {
  // TODO avoid useless string copy x2?
  // TODO error handling
  std::ifstream input_stream_v(path, std::ios_base::binary);
  std::ostringstream sstr;
  sstr << input_stream_v.rdbuf();
  return sstr.str();
}

}  // namespace interstellar::interstellar_testing::utils