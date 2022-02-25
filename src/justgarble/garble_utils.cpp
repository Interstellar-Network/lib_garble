#include "garble_utils.h"

#include <glog/logging.h>

#include <algorithm>
#include <boost/dynamic_bitset.hpp>
#include <iostream>

namespace {

// Tried uint8_t: quite slower, uint32_t: on par
using BitSet = boost::dynamic_bitset<uint64_t>;

/**
 * Again, this function is critical performance-wise.
 *
 * This is REALLY IMPORTANT to track the already processed input0, input1 and
 * output. Without tracking, we end up calling this function hundreds of
 * thousands of time when garbling big circuits(eg even old size2) and this
 * takes several seconds for NOTHING.
 *
 * NOTE: this is different from "tracking" the seedGates, which is inherently
 * done when constructing subgraphGates
 *
 * eg: (size1 pinpad, RelWithDebInfo)
 * with tracking:
 * real    0m0.013s user    0m0.005s
 * real    0m0.011s user    0m0.007s
 * without tracking:
 * real    0m1.131s user    0m1.127s
 * real    0m1.126s user    0m1.118s
 *
 * NOTE: already_processed_gates & subgraphGates are different "views" of the
 * same date.
 * subgraphGates: contains a list of indexes
 * already_processed_gates: contains a vect of bool: 1 if in subgraphGates, 0 if
 * not.
 * eg:
 * subgraphGates = {1, 3}
 * already_processed_gates = {0, 1, 0, 1}
 * We need to do it this way because:
 * - subgraph is REALLY sparse: typically 2-3 gates of all 55k+
 * - in this function: we need to check if already in subgraph_gates before
 * recursing, and a vect access is faster than std::find(vect) or
 * unordered_set.find()
 * - in garbleCircuit: we need to process it in order
 * - in garbleCircuit: we want to avoid set/unordered_set
 */
template <typename ContainerOfContainerInt>
void findAllGatesAux(int seedGate, std::vector<int> *subgraph_gates,
                     BitSet *subgraph_gates_indexes,
                     const ContainerOfContainerInt &wireGateMap,
                     const std::vector<GarbledGate> &gates,
                     BitSet *already_processed_seedgates) {
  assert((*subgraph_gates_indexes)[seedGate] == 0 &&
         "seedGate is already in subgraph_gates:");
  subgraph_gates->push_back(seedGate);
  (*subgraph_gates_indexes)[seedGate] = true;

  VLOG(2) << "findAllGatesAux : seedGate = " << seedGate;

  // TODO refactor to use an aux function, this is 3 times the same code

  int64_t input0 = gates[seedGate].input0;

  VLOG(2) << "findAllGatesAux : input0 = " << input0;

  if ((*already_processed_seedgates)[input0] == 0) {
    const auto &i0 = wireGateMap[input0];

    // VLOG(2) << "wireGateMap[input0] " << i0; // TODO log

    // BEFORE findAllGatesNoWBS or it is useless
    (*already_processed_seedgates)[input0] = true;

    for (int gate : i0) {
      if ((*subgraph_gates_indexes)[gate] == 0) {
        findAllGatesAux(gate, subgraph_gates, subgraph_gates_indexes,
                        wireGateMap, gates, already_processed_seedgates);
      }
    }

  } else {
    // already_processed: nothing to do
  }

  int64_t input1 = gates[seedGate].input1;

  VLOG(2) << "findAllGatesAux : input1 = " << input1;

  if ((*already_processed_seedgates)[input1] == 0) {
    const auto &i1 = wireGateMap[input1];

    // VLOG(2) << "wireGateMap[input1]" << i1; // TODO log

    // BEFORE findAllGatesNoWBS or it is useless
    (*already_processed_seedgates)[input1] = true;

    for (int gate : i1) {
      if ((*subgraph_gates_indexes)[gate] == 0) {
        findAllGatesAux(gate, subgraph_gates, subgraph_gates_indexes,
                        wireGateMap, gates, already_processed_seedgates);
      }
    }

  } else {
    // already_processed: nothing to do
  }

  int64_t output = gates[seedGate].output;

  VLOG(2) << "findAllGatesAux : output = %ld\n" << output;

  if ((*already_processed_seedgates)[output] == 0) {
    const auto &o = wireGateMap[output];

    // VLOG(2) << "wireGateMap[output] " << o;  // TODO log

    // BEFORE findAllGatesNoWBS or it is useless
    (*already_processed_seedgates)[output] = true;

    for (int gate : o) {
      if ((*subgraph_gates_indexes)[gate] == 0) {
        findAllGatesAux(gate, subgraph_gates, subgraph_gates_indexes,
                        wireGateMap, gates, already_processed_seedgates);
      }
    }

  } else {
    // already_processed: nothing to do
  }
}

// findAllGates aux: explicit instantiation.
template void findAllGatesAux(int seedGate, std::vector<int> *subgraph_gates,
                              BitSet *subgraph_gates_indexes,
                              const std::vector<std::vector<int>> &wireGateMap,
                              const std::vector<GarbledGate> &gates,
                              BitSet *already_processed_seedgatest);
template void findAllGatesAux(
    int seedGate, std::vector<int> *subgraph_gates,
    BitSet *subgraph_gates_indexes,
    const std::vector<std::unordered_set<int>> &wireGateMap,
    const std::vector<GarbledGate> &gates, BitSet *already_processed_seedgates);

}  // namespace

namespace GarbleUtils {

/**
 * Add "gate" to wireGateMap of "i0", "i1" and "o".
 * "gate" is the gate index in wireGateMap, NOT a GarbledGate
 *
 * "gate" MUST NOT already be in the subcontainer, see assert.
 */
void updateWires2(std::vector<std::vector<int>> *wireGateMap, int64_t i0,
                  int64_t i1, int64_t o, int gate) {
  // CHECK that the gate is NOT already in any of the vector
  assert(std::find((*wireGateMap)[i0].begin(), (*wireGateMap)[i0].end(),
                   gate) == (*wireGateMap)[i0].end());
  assert(std::find((*wireGateMap)[i1].begin(), (*wireGateMap)[i1].end(),
                   gate) == (*wireGateMap)[i1].end());
  assert(std::find((*wireGateMap)[o].begin(), (*wireGateMap)[o].end(), gate) ==
         (*wireGateMap)[o].end());

  (*wireGateMap)[i0].push_back(gate);
  (*wireGateMap)[i1].push_back(gate);
  (*wireGateMap)[o].push_back(gate);
}

/**
 * [overload]
 */
void updateWires2(std::vector<std::unordered_set<int>> *wireGateMap, int64_t i0,
                  int64_t i1, int64_t o, int gate) {
  // CHECK that the gate is NOT already in any of the vector
  assert(std::find((*wireGateMap)[i0].begin(), (*wireGateMap)[i0].end(),
                   gate) == (*wireGateMap)[i0].end());
  assert(std::find((*wireGateMap)[i1].begin(), (*wireGateMap)[i1].end(),
                   gate) == (*wireGateMap)[i1].end());
  assert(std::find((*wireGateMap)[o].begin(), (*wireGateMap)[o].end(), gate) ==
         (*wireGateMap)[o].end());

  (*wireGateMap)[i0].emplace(gate);
  (*wireGateMap)[i1].emplace(gate);
  (*wireGateMap)[o].emplace(gate);
}

/**
 * see comment at findAllGatesAux for details
 */
template <typename ContainerOfContainerInt>
void findAllGates(int seedGate, const ContainerOfContainerInt &wireGateMap,
                  const std::vector<GarbledGate> &gates,
                  std::vector<int> *subgraph_gates) {
  // previously it was an unordered_set
  // testing with a vector of char: already_processed[idx] = 1 if already
  // processed, 0 if not
  BitSet already_processed_seedgates(wireGateMap.size());  // all 0's by default

  // same principle for subgraph_gates_indexes: 1 if gate is in (the returned)
  // 'subgraph_gates', 0 if not
  BitSet subgraph_gates_indexes(wireGateMap.size());  // all 0's by default

  // typically 1-3, so 16 is a bit arbitrary
  subgraph_gates->reserve(16);

  findAllGatesAux(seedGate, subgraph_gates, &subgraph_gates_indexes,
                  wireGateMap, gates, &already_processed_seedgates);
}

// findAllGates: explicit instantiation.
template void findAllGates(int seedGate,
                           const std::vector<std::vector<int>> &wireGateMap,
                           const std::vector<GarbledGate> &gates,
                           std::vector<int> *subgraph_gates);
template void findAllGates(
    int seedGate, const std::vector<std::unordered_set<int>> &wireGateMap,
    const std::vector<GarbledGate> &gates, std::vector<int> *subgraph_gates);

}  // namespace GarbleUtils