// Copyright (c) 2018 Skeyecode. All rights reserved.
// author : Nathan Prat
// date : 2018-03-20

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