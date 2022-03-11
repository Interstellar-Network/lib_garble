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

#include <unordered_set>
#include <vector>

#include "justGarble.h"

#define DEBUG_findAllGates2 0

namespace GarbleUtils {

/**
 * WIP version: without willBeSeen
 *
 * wireGateMap: eg a  std::vector<std::vector<int>>
 * see updateWires2
 */
template <typename ContainerOfContainerInt>
void findAllGates(int seedGate, const ContainerOfContainerInt &wireGateMap,
                  const std::vector<GarbledGate> &gates,
                  std::vector<int> *subgraph_gates);

/**
 * All-in-one version of updateWires
 * to avoid the 3 calls in a row with i0, i1, then o.
 *
 * Severaly overloads to compare performance.
 *
 * wireGateMap: eg std::vector<std::vector<int>>
 */
void updateWires2(std::vector<std::vector<int>> *wireGateMap, int64_t i0,
                  int64_t i1, int64_t o, int gate);
void updateWires2(std::vector<std::unordered_set<int>> *wireGateMap, int64_t i0,
                  int64_t i1, int64_t o, int gate);

}  // namespace GarbleUtils