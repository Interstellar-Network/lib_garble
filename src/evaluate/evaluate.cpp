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

void PatchInputs(const std::vector<Block> &garbled_values,
                 std::vector<Block> *extractedLabels) {
  size_t n = garbled_values.size();

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
 * output_map: PGC's outputLabels(ie PGC class member)
 * output_results: return of ParallelEvaluate
 *
 * trust: when false: assumes the evaluate_outputs match the output_labels
 *  else will check and log an error if it's not the case
 * return: vals: the final results
 */
int MapOutputs(const std::vector<Block> &output_labels,
               const std::vector<Block> &evaluate_outputs_map,
               std::vector<uint8_t> *final_ouputs, bool trust) {
  size_t outputs_size = final_ouputs->size();
  assert(evaluate_outputs_map.size() == outputs_size && "size mismatch[1]!");
  assert(!output_labels.empty() &&
         "trying to eval with an unpatched stripped circuit?");
  assert(output_labels.size() == outputs_size * 2 && "size mismatch[2]!");

  if (trust) {
#pragma omp parallel for
    for (unsigned int i = 0; i < outputs_size; i++) {
      if (evaluate_outputs_map[i] == output_labels[2 * i]) {
        (*final_ouputs)[i] = 0;
      } else {
        (*final_ouputs)[i] = 1;
      }
    }
  } else {
#pragma omp parallel for
    for (unsigned int i = 0; i < outputs_size; i++) {
      if (evaluate_outputs_map[i] == output_labels[2 * i]) {
        (*final_ouputs)[i] = 0;
        continue;
      }
      if (evaluate_outputs_map[i] == output_labels[2 * i + 1]) {
        (*final_ouputs)[i] = 1;
        continue;
      }

      throw std::range_error("MAP FAILED!");
    }
  }
  return 0;
}

/**
 * return: an output "MAP"; it needs MapOutputs/ApplyXormask to be displayed
 */
#define DEBUG_ParallelEvaluate 0
std::vector<Block> ParallelEvaluate(
    const interstellar::garble::ParallelGarbledCircuit &garbledCircuit,
    const std::vector<Block> &extractedLabels) {
  std::vector<Block> outputs_map(garbledCircuit.nb_outputs_);

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
        int64_t a, b;

        Block val;

        Block A, B;

        int garbledGate_input0 = garbledGate >> 42ul & 0x1FFFFFul;
        int garbledGate_input1 = (garbledGate >> 21ul) & 0x1FFFFFul;
        int garbledGate_output = (garbledGate >> 0) & 0x1FFFFFul;
        int garbledGate_xor = garbledGate >> 63;

        const Block &i0 = wires[garbledGate_input0];
        const Block &i1 = wires[garbledGate_input1];

        if (garbledGate_xor != 0) {
          wires[garbledGate_output].Xor(i0, i1);
        } else {
          int tableIndex = startTableIndex + i - startGate;
          Block tweak(garbledGate, static_cast<int64_t>(0));
          A.Double(i0);
          B.Quadruple(i1);

          a = i0.GetLsb();
          b = i1.GetLsb();

          Block temp;

          val.Xor(A, B);
          val.Xor(val, tweak);

          assert(static_cast<uint32_t>(tableIndex) <
                     garbledCircuit.non_xor_count_ &&
                 "tableIndex: out of range!");
          temp.Xor(val, garbledCircuit.garbled_table_[2 * a + b][tableIndex]);
          val.Aes(aes_key_cipher.rounds, aes_key_cipher.rd_key);

          wires[garbledGate_output].Xor(val, temp);
        }
      }
#pragma omp barrier
    }
  }

  unsigned int garbledCircuit_m = garbledCircuit.nb_outputs_;

#pragma omp parallel for
  for (unsigned int i = 0; i < garbledCircuit_m; i++) {
    outputs_map[i] = wires[garbledCircuit.outputs_[i]];
  }

  return outputs_map;
}

/**
 * IMPORTANT : xormask must be a vector of "packed bits", typically from a
 * packmsg It WILL NOT work if xormask is a vector of 0/1
 */
// REFERENCE: version with packed bits into uint64_t
#if 1
// TODO remove pack_bits64
void ApplyXormask(const std::vector<uint64_t> &xormask,
                  std::vector<uint8_t> *final_result,
                  const std::vector<Block> &output_map) {
  size_t output_size = final_result->size();
  assert(output_size && "MUST set the size to PGC nb_outputs(m)!");

  for (unsigned int i = 0; i < output_size; i++) {
    int j = i / 64, k = i % 64;
    (*final_result)[i] = (output_map[i].GetLow() & 1) ^ ((xormask[j] >> k) & 1);
  }
}
#else
// TODO remove pack_bits64
#error "// TODO remove pack_bits64"
#endif

}  // anonymous namespace

namespace interstellar {

namespace garble {

/**
 * REFERENCE lib_python
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

/**
 * Contrary to EvaluateWithPackmsg:
 * - not using a Packmsg
 * -> the final result is obtained directly from MapOutputs instead of via
 * ApplyXormask
 *
 * This allow to check the evaluation results with a truth table(eg for
 * full_adder.v)
 */
std::vector<uint8_t> EvaluateWithInputs(
    const ParallelGarbledCircuit &parallel_garbled_circuit,
    const std::vector<uint8_t> &inputs) {
  std::vector<Block> extracted_labels =
      parallel_garbled_circuit.ExtractLabels(inputs);

  std::vector<Block> outputs_map =
      ParallelEvaluate(parallel_garbled_circuit, extracted_labels);

  // map computed_output_map to get the final results
  std::vector<uint8_t> final_outputs;
  final_outputs.resize(outputs_map.size());
  MapOutputs(parallel_garbled_circuit.output_labels_, outputs_map,
             &final_outputs, false);

  return final_outputs;
}

std::vector<uint8_t> EvaluateWithPackmsg(
    const ParallelGarbledCircuit &parallel_garbled_circuit,
    const std::vector<uint8_t> &inputs, const packmsg::Packmsg &packmsg) {
  std::vector<Block> extracted_labels =
      parallel_garbled_circuit.ExtractLabels(inputs);

  PatchInputs(packmsg.GetGarbledValues(), &extracted_labels);

  std::vector<Block> outputs_map =
      ParallelEvaluate(parallel_garbled_circuit, extracted_labels);

  // map computed_output_map to get the final results
  std::vector<uint8_t> final_outputs;
  final_outputs.resize(outputs_map.size());
  ApplyXormask(packmsg.GetXormask(), &final_outputs, outputs_map);

  return final_outputs;
}

}  // namespace garble

}  // namespace interstellar