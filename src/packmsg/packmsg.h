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

#include <string>
#include <vector>

#include "justgarble/block.h"

namespace interstellar {

namespace packmsg {

class Packmsg {
 public:
  Packmsg(const std::vector<Block> &garbled_values,
          const std::vector<uint64_t> &xormask);

  // NO COPY
  // NO MOVE
  Packmsg(Packmsg const &) = delete;
  void operator=(Packmsg const &x) = delete;

  auto const &GetXormask() const { return xormask_; };
  auto const &GetGarbledValues() const { return garbled_values_; };

  // INTERNAL/TEST ONLY
  bool operator==(const Packmsg &other) const {
    return (garbled_values_ == other.garbled_values_) &&
           (xormask_ == other.xormask_);
  };

 private:
  std::vector<Block> garbled_values_;
  std::vector<uint64_t> xormask_;
};

}  // namespace packmsg

}  // namespace interstellar