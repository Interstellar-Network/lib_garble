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

#include "prepackmsg.h"

namespace interstellar::packmsg {

PrePackmsg::PrePackmsg(std::vector<Block> &&labels,
                       std::vector<uint8_t> &&xormask,
                       const absl::flat_hash_map<std::string, uint32_t> &config)
    : labels_(std::move(labels)),
      xormask_(std::move(xormask)),
      config_(config) {}

PrePackmsg::PrePackmsg() {}

void PrePackmsg::SetLabels(std::vector<Block> &&labels) {
  labels_ = std::move(labels);
}

void PrePackmsg::SetXormask(std::vector<uint8_t> &&xormask) {
  xormask_ = std::move(xormask);
}

void PrePackmsg::SetConfig(
    absl::flat_hash_map<std::string, uint32_t> &&config) {
  config_ = std::move(config);
}

}  // namespace interstellar::packmsg