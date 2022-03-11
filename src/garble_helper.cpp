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

#include "garble_helper.h"

#include "parallel_garbled_circuit/parallel_garbled_circuit.h"
#include "serialize/serialize.h"

namespace interstellar {

namespace garblehelper {

/**
 * Garble a ".skcd"(in pratice a .blif.blif, given as BlifParser)
 * NOTE: contrary to its name, a "GarbledCircuit" here is basically just a SKCD
 * circuit.
 *
 * archive version: "Base" function: garble a .skcd given by path.
 *
 * return a ParallelGarbledCircuit
 */
std::string GarbleSkcdToBuffer(boost::filesystem::path skcd_input_path) {
  GarbledCircuit garbledCircuit(skcd_input_path);

  garbledCircuit.garbleCircuit();

  // Now for the parallel part
  garble::ParallelGarbledCircuit pgc{std::move(garbledCircuit)};

  return garble::Serialize(pgc);
}

void GarbleSkcdToFile(boost::filesystem::path skcd_input_path,
                      boost::filesystem::path pgarbled_output_path) {
  GarbledCircuit garbledCircuit(skcd_input_path);

  garbledCircuit.garbleCircuit();

  // Now for the parallel part
  garble::ParallelGarbledCircuit pgc{std::move(garbledCircuit)};

  garble::Serialize(pgc, pgarbled_output_path);
}

}  // namespace garblehelper

}  // namespace interstellar