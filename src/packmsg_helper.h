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
#include <string_view>
#include <vector>

#include "packmsg/packmsg.h"
#include "packmsg/prepackmsg.h"
#include "parallel_garbled_circuit/parallel_garbled_circuit.h"

namespace interstellar::packmsg {

/**
 * All-in-one: Garble then Strip a given .skcd buffer:
 *
 * return:
 * - ParallelGarbledCircuit: the garbled circuit
 * - PrePackmsg: it SHOULD be stored "next to" the return ParallelGarbledCircuit
 * - the random "digits" burned into the PGC
 *
 * NOTE: ideally the caller SHOULD provide the random digits instead of
 * generating here and returning them.
 * The problem is that requires knowing how many digits to burn(eg 2, 4, 10?)
 * and that requires deserializing the .skcd
 * So for now we do it that way.
 * TODO? in lib_circuits interstellar::skcd::Serialize : also return the
 * "config" to be stored next to the circuit buffer instead by the caller
 *
 * throw: if the PGC is NOT a "display circuit"(eg adder, etc)
 *
 */
void GarbleAndStrippedSkcdFromBuffer(std::string_view skcd_buffer,
                                     garble::ParallelGarbledCircuit *pgc,
                                     PrePackmsg *pre_packmsg,
                                     std::vector<uint8_t> *digits);

Packmsg PackmsgFromPrepacket(const PrePackmsg &pre_packmsg,
                             const std::string &watermark_message);

}  // namespace interstellar::packmsg