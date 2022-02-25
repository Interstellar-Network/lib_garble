#pragma once

// MUST (mostly) match lib_circuit
// /interstellar/lib_circuits/src/blif/circuit_data.h
// TODO move to shared submodule

#include <cstdint>
#include <limits>

enum class GateLayer : uint8_t {
  // All inputs are root(= 'msg' and/or 'rnd' and/or 'z')
  ROOT,
  // One input is root, the other is the output from another gate
  INTERMEDIATE,
  // All inputs are outputs from another gate
  // NOTE: this is never the case in pratice, but have this just in case
  SECOND,
  // ZERO: currently a special case
  // no input, only one output: 0
  // ZERO,
  // INV: same as ZERO: currently a special case
  // one input: 'z', one output: 'pix[NNN]'
  // (only one of those in a circuit)
  // INV,
  // DO NOT USE in switch etc;
  // https://stackoverflow.com/questions/2102582/how-can-i-count-the-items-in-an-enum
  GateLayer_MAX = SECOND,
};

/**
 * This is an "helper struct" init in BlifParser, and passed around all the way
 * to (Parallel)GarbledCircuit.
 * It allows to avoid various loops on gates/labels to recompute the same
 * things again and again.
 */
struct CircuitData {
  uint32_t gate_input_min = std::numeric_limits<uint32_t>::max();
  uint32_t gate_input_max = std::numeric_limits<uint32_t>::min();
  uint32_t gate_output_min = std::numeric_limits<uint32_t>::max();
  uint32_t gate_output_max = std::numeric_limits<uint32_t>::min();

  std::vector<uint32_t> layer_count = std::vector<uint32_t>(
      static_cast<uint8_t>(GateLayer::GateLayer_MAX) + 1, 0);

  // map an 'input gate_id' to a number(of gate that has this input)
  // After init: input_gate_count.size() = gate_input_max + 1;
  // NOTE: the inputs are typically init from 0, so this vector is NOT sparse.
  // ie: typically the "max input0/1" is just a bit bigger than "number of
  // unique input0/1"
  std::vector<uint32_t> input_gate_count;

  // TODO add AT LEAST 'input_to_gates'(init in BlifParser, used in XorGraph)
};