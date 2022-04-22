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

#include "packmsg/packmsg.h"
#include "parallel_garbled_circuit/parallel_garbled_circuit.h"

namespace interstellar {

namespace testing {

/**
 * param png_output_path: if empty it will use cimg.display() instead of write
 * to file. Therefore:
 * - png_output_path set: MUST use libpng
 * - png_output_path unset: MUST use X11
 */
void EvalAndDisplay(
    const garble::ParallelGarbledCircuit &parallel_garbled_circuit,
    u_int32_t nb_evals, std::string_view png_output_path);

/**
 * param png_output_path: if empty it will use cimg.display() instead of write
 * to file. Therefore:
 * - png_output_path set: MUST use libpng
 * - png_output_path unset: MUST use X11
 */
void EvalAndDisplayWithPackmsg(
    const garble::ParallelGarbledCircuit &parallel_garbled_circuit,
    const packmsg::Packmsg &packmsg, u_int32_t nb_evals,
    std::string_view png_output_path);

}  // namespace testing

}  // namespace interstellar