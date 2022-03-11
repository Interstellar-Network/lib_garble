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

#include "evaluate.h"

#include "justgarble/aes-sse.h"  // for AES_KEY
#include "justgarble/block.h"    // for typedef OutputMap etc

namespace {

void patch_inputs(const std::vector<Block> &garbled_values,
                  std::vector<Block> *extractedLabels) {
  size_t n = garbled_values.size();
  printf("patch_inputs : size : %zu\n", n);

  size_t patch_size = garbled_values.size();
  assert(n == patch_size);

  for (unsigned int i = 0; i < patch_size; i++) {
    if (static_cast<int>(static_cast<int>(garbled_values[i].GetHigh() != 0) &
                         static_cast<int>(garbled_values[i].GetLow() != 0)) !=
        0) {
      (*extractedLabels)[i] = garbled_values[i];
    }
  }
}

/**
 * For dev/test
 * Production is using apply_xormask instead
 *
 * output_map: PGC's outputLabels
 * output_results: results of parallelEvaluate
 *
 * trust: when false: assumes the evaluate_outputs match the output_labels
 *  else will check and log an error if it's not the case
 * return: vals: the final results
 */
int mapOutputs(const std::vector<Block> &output_labels,
               const std::vector<Block> &evaluate_outputs,
               std::vector<uint8_t> *final_ouputs, bool trust) {
  size_t outputs_size = final_ouputs->size();
  assert(evaluate_outputs.size() == outputs_size && "size mismatch[1]!");
  assert(output_labels.size() == outputs_size * 2 && "size mismatch[2]!");

  if (trust) {
#pragma omp parallel for
    for (unsigned int i = 0; i < outputs_size; i++) {
      if (evaluate_outputs[i] == output_labels[2 * i]) {
        (*final_ouputs)[i] = 0;
      } else {
        (*final_ouputs)[i] = 1;
      }
    }
  } else {
#pragma omp parallel for
    for (unsigned int i = 0; i < outputs_size; i++) {
      if (evaluate_outputs[i] == output_labels[2 * i]) {
        (*final_ouputs)[i] = 0;
        continue;
      }
      if (evaluate_outputs[i] == output_labels[2 * i + 1]) {
        (*final_ouputs)[i] = 1;
        continue;
      }

      throw std::range_error("MAP FAILED!");
    }
  }
  return 0;
}

#define DEBUG_parallelEvaluate 0
int parallelEvaluate(
    const interstellar::garble::ParallelGarbledCircuit &garbledCircuit,
    const std::vector<Block> &extractedLabels, std::vector<Block> *outputMap) {
  AES_KEY aes_key_cipher;  // originally a DKCipherContext
  // previously: const __m128i *sched = ((__m128i *)(aes_key_cipher.rd_key));
  AES_set_encrypt_key(
      reinterpret_cast<const unsigned char *>(&(garbledCircuit.global_key_)),
      128,
      &aes_key_cipher);  // originally DKCipherInit

  // TODO (optionally/overload) pass by arg pointer; useful when doing multiple
  // evals
  std::vector<Block> wires(garbledCircuit.nb_wires_);

  assert(extractedLabels.size() == garbledCircuit.nb_inputs_ &&
         "wrong extractedLabels size!");
#pragma omp parallel for
  for (unsigned int i = 0; i < garbledCircuit.nb_inputs_; i++) {
    wires[i] = extractedLabels[i];
  }

// layer 0 is just input wires
#pragma omp parallel
  {
    unsigned int layer = 0;
    unsigned int endGate = 0;
    unsigned int endTableIndex = 0;
    // LOGIFDEF(DEBUG_parallelEvaluate, "SCHED %llx %llx", *((uint64_t*)sched),
    // *((uint64_t*)sched + 1));

    unsigned int garbledCircuit_nbLayers = garbledCircuit.nb_layers_;

    for (layer = 1; layer < garbledCircuit_nbLayers; layer++) {
      unsigned int startGate = endGate;
      unsigned int startTableIndex = endTableIndex;
      endGate = startGate + garbledCircuit.layer_counts_[layer];
      endTableIndex =
          startTableIndex + garbledCircuit.layer_nonxor_counts_[layer];

#pragma omp for schedule(static, 1)
      //#pragma omp parallel for schedule(static,16)
      for (unsigned int i = startGate; i < endGate; i++) {
        uint64_t garbledGate = garbledCircuit.garbled_gates_[i];
        // LOGIFDEF(DEBUG_parallelEvaluate, "garbledGate @ %d/%d : %lx", i,
        // layer, garbledGate);
        int64_t a, b;

        Block val;

        Block A, B;

        int garbledGate_input0 = garbledGate >> 42ul & 0x1FFFFFul;
        int garbledGate_input1 = (garbledGate >> 21ul) & 0x1FFFFFul;
        int garbledGate_output = (garbledGate >> 0) & 0x1FFFFFul;
        int garbledGate_xor = garbledGate >> 63;
        // LOGIFDEF(DEBUG_parallelEvaluate, "garbledGate_input0 @ %d/%d : %x",
        // i, layer, garbledGate_input0); LOGIFDEF(DEBUG_parallelEvaluate,
        // "garbledGate_input1 @ %d/%d : %x", i, layer, garbledGate_input1);
        // LOGIFDEF(DEBUG_parallelEvaluate, "garbledGate_output garbledGate_xor
        // @ %d/%d : %x %x", i, layer, garbledGate_output, garbledGate_xor);
#ifdef DEBUG
        LOGIFDEF(DEBUG_parallelEvaluate, "Gate %d i0=%d, i1=%d, o=%d, xor=%d",
                 i, garbledGate_input0, garbledGate_input1, garbledGate_output,
                 garbledGate_xor);
        uint64_t *ig0 = (uint64_t *)&garbledCircuit.wires[garbledGate_input0];
        uint64_t *ig1 = (uint64_t *)&garbledCircuit.wires[garbledGate_input1];
        LOGIFDEF(DEBUG_parallelEvaluate, "Input0 %llx %llx Input1 %llx %llx",
                 *ig0, *(ig0 + 1), *ig1, *(ig1 + 1));
#endif
        const Block &i0 = wires[garbledGate_input0];
        const Block &i1 = wires[garbledGate_input1];
        // LOGIFDEF(DEBUG_parallelEvaluate, "i0 @ %d/%d : %llx %llx", i, layer,
        // *((uint64_t*)i0), *((uint64_t*)i0 + 1));
        // LOGIFDEF(DEBUG_parallelEvaluate, "i1 @ %d/%d : %llx %llx", i, layer,
        // *((uint64_t*)i1), *((uint64_t*)i1 + 1));

        if (garbledGate_xor != 0) {
          wires[garbledGate_output].Xor(i0, i1);
          // LOGIFDEF(DEBUG_parallelEvaluate, "XORGATE wires[garbledGate_output]
          // @ %d/%d : %llx %llx", i, layer,
          // *((uint64_t*)&garbledCircuit.wires[garbledGate_output]),
          //        *((uint64_t*)&garbledCircuit.wires[garbledGate_output] +
          //        1));
        } else {
          int tableIndex = startTableIndex + i - startGate;
          Block tweak(garbledGate, static_cast<int64_t>(0));
          A.Double(i0);
          B.Quadruple(i1);
          // LOGIFDEF(DEBUG_parallelEvaluate, "A @ %d/%d : %llx %llx", i, layer,
          // *((uint64_t*)&A), *((uint64_t*)&A + 1));
          // LOGIFDEF(DEBUG_parallelEvaluate, "B @ %d/%d : %llx %llx", i, layer,
          // *((uint64_t*)&B), *((uint64_t*)&B + 1));

          a = i0.GetLsb();
          b = i1.GetLsb();
          // LOGIFDEF(DEBUG_parallelEvaluate, "a / b @ %d/%d : %lx / %lx", i,
          // layer, a, b);

          Block temp;

          val.Xor(A, B);
          // LOGIFDEF(DEBUG_parallelEvaluate, "VAL0 @ %d/%d : %llx %llx", i,
          // layer, *((uint64_t*)&val), *((uint64_t*)&val + 1));
#ifdef DEBUG
          LOGIFDEF(DEBUG_parallelEvaluate, "Tweak for gate %ld is %llx", i,
                   garbledGate);
#endif
          // LOGIFDEF(DEBUG_parallelEvaluate, "TWEAK @ %d/%d : %llx %llx", i,
          // layer, *((uint64_t*)&tweak), *((uint64_t*)&tweak + 1));
          val.Xor(val, tweak);
          // LOGIFDEF(DEBUG_parallelEvaluate, "VAL1 @ %d/%d : %llx %llx", i,
          // layer, *((uint64_t*)&val), *((uint64_t*)&val + 1));

          // LOGIFDEF(DEBUG_parallelEvaluate, "gt idx / tableIndex @ %d/%d : %lx
          // %d", i, layer, 2 * a + b, tableIndex);
          //                    printf("TEMP0 @ %d/%d : %llx %llx\n", i, layer,
          //                    *((uint64_t*)&temp), *((uint64_t*)&temp + 1));

          assert(static_cast<uint32_t>(tableIndex) <
                     garbledCircuit.non_xor_count_ &&
                 "tableIndex: out of range!");
          temp.Xor(val, garbledCircuit.garbled_table_[2 * a + b][tableIndex]);

          // LOGIFDEF(DEBUG_parallelEvaluate, "TEMP1 @ %d/%d : %llx %llx", i,
          // layer, *((uint64_t*)&temp), *((uint64_t*)&temp + 1));

          val.Aes(aes_key_cipher.rounds, aes_key_cipher.rd_key);

          // LOGIFDEF(DEBUG_parallelEvaluate, "VAL1 aes @ %d/%d : %llx %llx", i,
          // layer, *((uint64_t*)&val), *((uint64_t*)&val + 1));

          wires[garbledGate_output].Xor(val, temp);
          // LOGIFDEF(DEBUG_parallelEvaluate, "NONXOR result @ %d/%d : %llx
          // %llx", i, layer,
          // *((uint64_t*)&garbledCircuit.wires[garbledGate_output]),
          // *((uint64_t*)&garbledCircuit.wires[garbledGate_output] + 1));
        }
      }
#pragma omp barrier
    }
  }

  unsigned int garbledCircuit_m = garbledCircuit.nb_outputs_;

#pragma omp parallel for
  for (unsigned int i = 0; i < garbledCircuit_m; i++) {
#ifdef DEBUG
    uint64_t *o = (uint64_t *)&garbledCircuit.wires[garbledCircuit.outputs[i]];
    LOGIFDEF(DEBUG_parallelEvaluate, "Output %ld : %llx %llx", i, *o, *(o + 1));
#endif
    (*outputMap)[i] = wires[garbledCircuit.outputs_[i]];
  }

  return 0;
}

}  // anonymous namespace

namespace interstellar {

namespace garble {

/**
 * For dev/test.
 *
 * Contrary to evaluateWithPackmsgStripped:
 * - inputs are given
 * - not using a Packmsg
 * - the final result is obtained directly from mapOutputs instead of via
 * apply_xormask
 *
 * This allow to check the evaluation results with a truth table(eg for
 * full_adder.v)
 */
std::vector<uint8_t> EvaluateWithInputs(
    const ParallelGarbledCircuit &parallel_garbled_circuit,
    const std::vector<uint8_t> &inputs) {
  unsigned int n = parallel_garbled_circuit.nb_inputs_;
  unsigned int m = parallel_garbled_circuit.nb_outputs_;
  if (n == 0 || m == 0) {
    throw std::runtime_error("Circuit read failed");
  }

  std::vector<Block> extracted_labels =
      parallel_garbled_circuit.ExtractLabels(inputs);

  patch_inputs(extracted_labels, &extracted_labels);

  std::vector<Block> outputs;
  outputs.resize(m);

  parallelEvaluate(parallel_garbled_circuit, extracted_labels, &outputs);

  std::vector<uint8_t> final_outputs;
  final_outputs.resize(m);
  // final_outputs2.resize(m);

  /**
   * From testaddgarbled.py:

      # two ways to decode outputs:
      # first, map outputs using output labels
      outputs = g.map_outputs(garbled_outputs)
      c = outputs[0] | (outputs[1]<<1) | (outputs[2]<<2)
      if a+b != c:
          print('Failed for a =', a, ' and b =', b,' (using map_outputs)')
          exit(1)

      # second, apply xormask to garbled values LSB
      outputs = [(gv[1]&1)^mask for gv, mask in zip(garbled_outputs, xormask)]
      c = outputs[0] | (outputs[1]<<1) | (outputs[2]<<2)
      if a+b != c:
          print('Failed for a =', a, ' and b =', b,' (using xormask)')
          exit(1)
   */

  // map computed_output_map to get the final results
  mapOutputs(parallel_garbled_circuit.output_labels_, outputs, &final_outputs,
             false);

  // CAN NOT use the xormask here, or at least not GetXormask directly; the
  // xormask must be packed into 64bits words before apply_xormask !
  // // second way: with apply_xor_mask
  // std::vector<uint64_t> xor_mask = parallel_garbled_circuit.GetXormask();
  // apply_xormask(xor_mask, final_outputs2.data(), outputs,
  // parallel_garbled_circuit.m);

  // LOGD("final_outputs2 : %d %d", final_outputs2[0], final_outputs2[1]);

  return final_outputs;
}

}  // namespace garble

}  // namespace interstellar