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

#include "packmsg.h"

namespace interstellar::packmsg {

Packmsg::Packmsg(const std::vector<Block> &garbled_values,
                 const std::vector<uint64_t> &xormask)
    : garbled_values_(garbled_values), xormask_(xormask){};

}  // namespace interstellar::packmsg