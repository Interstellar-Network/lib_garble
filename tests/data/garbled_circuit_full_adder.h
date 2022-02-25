#pragma once

#include "justgarble/justGarble.h"

namespace interstellar {

namespace test {

// pratn@DESKTOP-D8U39SO:~/Documents/interstellar/lib_server$ git status
// HEAD detached at 2ce062c
//
// step 2: Commit: 2e880007f2c149e082d5f3c8f165f65b
// "#error "need to fix evaluateWithInputs to use packed bits(like a packmsg)"
// at tests/test_evaluate.cpp
//
// step 1.5: 8b03cfa

/**
 * Return a manually constucted "full adder"
 * All those hardcoded values were obtained using a debugger with "lib_server"
 * (cf commit hash above).
 *
 * This is NOT directly equivalent to tests/data/adder.skcd.pb.bin!
 */
void GetFullAdder(GarbledCircuit *garbled_circuit) {
  garbled_circuit->nb_outputs_ = 2;  // m
  garbled_circuit->nb_inputs_ = 3;   // n
  garbled_circuit->nb_gates_ = 9;    // q
  garbled_circuit->nb_wires_ = 14;   // r = nb wires = n+q+2

  //   for (uint32_t i = 0; i < garbled_circuit->nb_wires_; ++i) {
  // wires[i].id = 0;
  //   }

  // = (*p).via.i64
  //   garbledGates[0].input1 = 0;
  //   garbledGates[1].input1 = 0;
  //   garbledGates[2].input1 = 0;
  //   garbledGates[3].input1 = 0;
  //   garbledGates[4].input1 = 1;
  //   garbledGates[5].input1 = 7;
  //   garbledGates[6].input1 = 1;
  //   garbledGates[7].input1 = 7;
  //   garbledGates[8].input1 = 10;
  garbled_circuit->gate_inputs1_.resize(garbled_circuit->nb_gates_);
  garbled_circuit->gate_inputs1_[0] = 0;
  garbled_circuit->gate_inputs1_[1] = 0;
  garbled_circuit->gate_inputs1_[2] = 0;
  garbled_circuit->gate_inputs1_[3] = 0;
  garbled_circuit->gate_inputs1_[4] = 1;
  garbled_circuit->gate_inputs1_[5] = 7;
  garbled_circuit->gate_inputs1_[6] = 1;
  garbled_circuit->gate_inputs1_[7] = 7;
  garbled_circuit->gate_inputs1_[8] = 10;

  // = (*p).via.i64
  //   garbledGates[0].input0 = 0;
  //   garbledGates[1].input0 = 0;
  //   garbledGates[2].input0 = 0;
  //   garbledGates[3].input0 = 0;
  //   garbledGates[4].input0 = 0;
  //   garbledGates[5].input0 = 2;
  //   garbledGates[6].input0 = 0;
  //   garbledGates[7].input0 = 2;
  //   garbledGates[8].input0 = 9;
  garbled_circuit->gate_inputs0_.resize(garbled_circuit->nb_gates_);
  garbled_circuit->gate_inputs0_[0] = 0;
  garbled_circuit->gate_inputs0_[1] = 0;
  garbled_circuit->gate_inputs0_[2] = 0;
  garbled_circuit->gate_inputs0_[3] = 0;
  garbled_circuit->gate_inputs0_[4] = 0;
  garbled_circuit->gate_inputs0_[5] = 2;
  garbled_circuit->gate_inputs0_[6] = 0;
  garbled_circuit->gate_inputs0_[7] = 2;
  garbled_circuit->gate_inputs0_[8] = 9;

  //   garbledCircuit->garbledGates[0].id = 0;      // =0
  //   garbledCircuit->garbledGates[0].output = 3;  // = (*p).via.i64
  //   garbledCircuit->garbledGates[1].id = 0;
  //   garbledCircuit->garbledGates[1].output = 4;
  //   garbledCircuit->garbledGates[2].id = 0;
  //   garbledCircuit->garbledGates[2].output = 5;
  //   garbledCircuit->garbledGates[3].id = 0;
  //   garbledCircuit->garbledGates[3].output = 6;
  //   garbledCircuit->garbledGates[4].id = 0;
  //   garbledCircuit->garbledGates[4].output = 7;
  //   garbledCircuit->garbledGates[5].id = 0;
  //   garbledCircuit->garbledGates[5].output = 8;
  //   garbledCircuit->garbledGates[6].id = 0;
  //   garbledCircuit->garbledGates[6].output = 6;
  //   garbledCircuit->garbledGates[7].id = 0;
  //   garbledCircuit->garbledGates[7].output = 10;
  //   garbledCircuit->garbledGates[8].id = 0;
  //   garbledCircuit->garbledGates[8].output = 11;
  garbled_circuit->gate_outputs_.resize(garbled_circuit->nb_gates_);
  //   garbledCircuit->garbledGates[0].id = 0;        // =0
  garbled_circuit->gate_outputs_[0] = 3;  // = (*p).via.i64
  //   garbledCircuit->garbledGates[1].id = 0;
  garbled_circuit->gate_outputs_[1] = 4;
  //   garbledCircuit->garbledGates[2].id = 0;
  garbled_circuit->gate_outputs_[2] = 5;
  //   garbledCircuit->garbledGates[3].id = 0;
  garbled_circuit->gate_outputs_[3] = 6;
  //   garbledCircuit->garbledGates[4].id = 0;
  garbled_circuit->gate_outputs_[4] = 7;
  //   garbledCircuit->garbledGates[5].id = 0;
  garbled_circuit->gate_outputs_[5] = 8;
  //   garbledCircuit->garbledGates[6].id = 0;
  garbled_circuit->gate_outputs_[6] = 9;
  //   garbledCircuit->garbledGates[7].id = 0;
  garbled_circuit->gate_outputs_[7] = 10;
  //   garbledCircuit->garbledGates[8].id = 0;
  garbled_circuit->gate_outputs_[8] = 11;

  // = (*p).via.i64;
  garbled_circuit->gate_types_.resize(garbled_circuit->nb_gates_);
  garbled_circuit->gate_types_[0] = interstellar::SkcdGateType(0);
  garbled_circuit->gate_types_[1] = interstellar::SkcdGateType(0);
  garbled_circuit->gate_types_[2] = interstellar::SkcdGateType(15);
  garbled_circuit->gate_types_[3] = interstellar::SkcdGateType(15);
  garbled_circuit->gate_types_[4] = interstellar::SkcdGateType(6);
  garbled_circuit->gate_types_[5] = interstellar::SkcdGateType(6);
  garbled_circuit->gate_types_[6] = interstellar::SkcdGateType(7);
  garbled_circuit->gate_types_[7] = interstellar::SkcdGateType(7);
  garbled_circuit->gate_types_[8] = interstellar::SkcdGateType(7);

  // = (*p).via.i64;
  garbled_circuit->outputs_.resize(garbled_circuit->nb_outputs_);
  garbled_circuit->outputs_[0] = 8;
  garbled_circuit->outputs_[1] = 11;

  // CUSTOM(ie not in old lib_server)

  garbled_circuit->circuit_data_.gate_input_min = 0;
  garbled_circuit->circuit_data_.gate_input_max = 10;
  garbled_circuit->circuit_data_.gate_output_min = 7;
  garbled_circuit->circuit_data_.gate_output_max = 11;
  garbled_circuit->circuit_data_.layer_count.resize(3);
  garbled_circuit->circuit_data_.layer_count[0] = 6;
  garbled_circuit->circuit_data_.layer_count[1] = 2;
  garbled_circuit->circuit_data_.layer_count[2] = 1;
  garbled_circuit->circuit_data_.input_gate_count.resize(
      garbled_circuit->nb_gates_ + 2);
  garbled_circuit->circuit_data_.input_gate_count[0] = 10;
  garbled_circuit->circuit_data_.input_gate_count[1] = 2;
  garbled_circuit->circuit_data_.input_gate_count[2] = 2;
  garbled_circuit->circuit_data_.input_gate_count[3] = 0;
  garbled_circuit->circuit_data_.input_gate_count[4] = 0;
  garbled_circuit->circuit_data_.input_gate_count[5] = 0;
  garbled_circuit->circuit_data_.input_gate_count[6] = 0;
  garbled_circuit->circuit_data_.input_gate_count[7] = 2;
  garbled_circuit->circuit_data_.input_gate_count[8] = 0;
  garbled_circuit->circuit_data_.input_gate_count[9] = 1;
  garbled_circuit->circuit_data_.input_gate_count[10] = 1;
}

}  // namespace test

}  // namespace interstellar