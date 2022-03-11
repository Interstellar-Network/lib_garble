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

#include "xor_gate_processor.h"

#include <algorithm>
#include <cassert>

XorGateProcessor::XorGateProcessor(std::vector<char> &&allXORGates)
    : to_be_processed_(std::move(allXORGates)) {
  seed_gate_it_ =
      std::find(to_be_processed_.begin(), to_be_processed_.end(), 1);
}

int XorGateProcessor::GetNextSeedGate() const {
  if (seed_gate_it_ == to_be_processed_.end()) {
    return -1;
  }

  return seed_gate_it_ - to_be_processed_.begin();
}

// clang-format off
/**
 * Given this "no loop" = "no duplicate between calls": we do not need to handle
 * a real set for the "yet to be processed".
 * We simply need to compute the next gate here, that will be returned by
 * GetNextSeedGate().
 *
 * PRECOND:
 * - the "current seedGate"(=last returned value of GetNextSeedGate()) SHOULD be
 * in subgraph_gates
 * - subgraph_gates SHOULD be sorted
 *
 * simple example:
 *
 * allXorGates: 				      [2,3,5,6,7,9,10] = gatesToProcess
 * initial seedGate:          2
 * subgraph_gates: 		        [6,7,10,2]
 * end first loop: seedGate:  3
 * subgraph_gates: 		        [5,3]
 * end second loop: seedGate: 9
 *
 * -> After "end first loop": GetNextSeedGate should return 3, and after
 * "end second loop" it should return 9.
 * There is no need to compute the set: gatesToProcess - {[2,6,7,10]+[3,5]} for
 * this.
 */
// clang-format on
void XorGateProcessor::MarkAsProcessed(
    const std::vector<uint32_t> &subgraph_gates) {
  assert(std::find(subgraph_gates.begin(), subgraph_gates.end(),
                   seed_gate_it_ - to_be_processed_.begin()) !=
             subgraph_gates.end() &&
         "seedGate is NOT in subgraph_gates!");

  // NOTE: tried an implementation using std::set_difference, but that was
  // slower that the original code using unordered_set for gatesToProcess

  // As we don't get a sorted set as input here, we need to manually
  // keep track of the "min gate_id that was processed", to have a reasonable
  // fast lookup when updating seed_gate_it_
  uint32_t min_processed_gate = std::numeric_limits<uint32_t>::max();

  // mark all the gates from "subgraph_gates" as processed
  for (auto subgraph_gate : subgraph_gates) {
    assert(to_be_processed_[subgraph_gate] == 1 &&
           "subgraph_gates: duplicate(loop?)");
    assert(
        subgraph_gate >= seed_gate_it_ - to_be_processed_.begin() &&
        "MarkAsProcessed: given a gate BEFORE the previous GetNextSeedGate()!");
    to_be_processed_[subgraph_gate] = 0;

    min_processed_gate = std::min(min_processed_gate, subgraph_gate);
  }

  // that one is already processed, so we can skip it
  min_processed_gate++;

  // find the next seedGate
  // IMPORTANT: DO NOT start at to_be_processed_.begin()
  // We DO NOT want to lookup in 55k elements vector each time! (and we don't
  // need to)
  // Comp:
  // - when starting at "to_be_processed_.begin()": each call is looking up
  // further into "to_be_processed_": eg first is ~321(index of the first XOR
  // gate), next is 531, next is 532, etc, up to potentially the full vect
  // on avg: ~5 elems(excluding THE outlier at half vect size)
  // - when starting at "to_be_processed_.begin() + min_processed_gate":
  // the usual "lookup distance" is 0-10 elems, with only a few outliers
  // on avg: ~680 elems(excluding THE outlier at half vect size)
  seed_gate_it_ = std::find(to_be_processed_.begin() + min_processed_gate,
                            to_be_processed_.end(), 1);
}