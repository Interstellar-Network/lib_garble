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

#include <cstdint>
#include <vector>

/**
 * This is used during the "main garbling loop", when processing all XOR gates.
 * We use a class(ie with an internal state) to avoid using an unordered_set to
 * track the "yet to be processed gates".
 * This is quite faster.
 *
 * NOTE: the garbling algo DOES NOT care of the processing order of the XOR
 * gate, just that all the XOR gate are processed in the end.
 * But for performance & algo reasons, in this class we NEED subgraph_gates &
 * allXORGates given sorted.
 */
class XorGateProcessor {
 public:
  /**
   * Can only by constructed from an allXORGates vector.
   */
  explicit XorGateProcessor(std::vector<char> &&allXORGates);

  XorGateProcessor(const XorGateProcessor &) = delete;
  XorGateProcessor &operator=(const XorGateProcessor &) = delete;

  /**
   * return: -1 if there is nothing more to process
   */
  int GetNextSeedGate() const;

  /**
   * Remove the given gates from the "to be processed" set.
   * It is assumed the given gates DO NOT contain ANY already processed one.
   * See "there are no loops in a DAG" comment in garble().
   *
   * subgraph_gates: result of GarbleUtils::findAllGates, AFTER SORTING;
   * "subgraph_gates" must be processed in order, so it had to be sorted anyway.
   */
  void MarkAsProcessed(const std::vector<uint32_t> &subgraph_gates);

 private:
  std::vector<char> to_be_processed_;

  // points to a gate in to_be_processed_;
  std::vector<char>::iterator seed_gate_it_;
};