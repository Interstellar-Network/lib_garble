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

#include "stripped_circuit.h"

#include <absl/container/flat_hash_set.h>
#include <absl/random/bit_gen_ref.h>
#include <absl/random/random.h>

#include "packmsg_utils.h"

namespace {
/**
 * static
 *
 * Strip a circuit.
 * To be evaluable, so resulting circuit will have to be patched with a Packmsg.
 *
 * strip garbled file from all non random labels
 *
 * TODO fix: we should not need to return a copy of the circuit but the
 * evaluation fails when trying with an in-place version...
 */
#define DEBUG_Strip 0
void StripParallelGarbledCircuit(
    interstellar::garble::ParallelGarbledCircuit *pgc, absl::BitGenRef bitgen) {
  /* Python function for reference
  (lib_python.stripgarbled.stripgarbled_in_memory)

  # read random inputs first bit offset and size from config
  rndfirst = config[b'SEG'] * config[b'MSGSIZE']
  rndsize = config[b'RNDSIZE']
  assert g.n == rndfirst + rndsize + 1
  # patch input labels
  labels = g.inputLabels
  for i in range(g.n):
      if rndfirst <= i < rndfirst+rndsize: # random input
          if my_system_random.random() > 0.5: # randomly shuffle label0 and
  label1 labels[2*i], labels[2*i+1] = labels[2*i+1], labels[2*i] else:
          labels[2*i] = [0, 0]
          labels[2*i+1] = [0, 0]

  # patch output labels
  for i in range(2*g.m):
      g.outputLabels[i] = [0, 0]

  return g.get_data_old_format()
  */

  // TODO TOREMOVE rndsize/rndfirst?
  uint32_t rndfirst =
      pgc->config_.at("BITMAP_NB_SEGMENTS");  // historically: "SEG" * "MSGSIZE"
  uint32_t rndsize = pgc->config_.at("RNDSIZE");
  assert(pgc->nb_inputs_ == rndfirst + rndsize + 1);

  for (uint32_t i = 0; i < pgc->nb_inputs_; i++) {
    // TODO TOREMOVE rndsize/rndfirst?
    if ((rndfirst <= i) && (i < rndfirst + rndsize)) {  // random input
      bool rand_switch = absl::Bernoulli(bitgen, 0.5);
      if (rand_switch) {  // randomly shuffle label0 and label1
        std::swap(pgc->input_labels_[2 * i], pgc->input_labels_[2 * i + 1]);
      }
    } else {
      pgc->input_labels_[2 * i].Zero();
      pgc->input_labels_[2 * i + 1].Zero();
    }
  }

  assert(pgc->output_labels_.size() == 2 * pgc->nb_outputs_ &&
         "Strip: size mismatch!");
  pgc->output_labels_.clear();
}

std::vector<uint8_t> PrepareXormask(
    const interstellar::garble::ParallelGarbledCircuit &pgc) {
  assert(!pgc.output_labels_.empty() &&
         "called GetXormask on a stripped circuit!?");
  assert(pgc.output_labels_.size() ==
             static_cast<size_t>(2 * pgc.nb_outputs_) &&
         "outputLabels: wrong size!");

  std::vector<uint8_t> bits;
  bits.reserve(pgc.nb_outputs_);

  for (uint32_t i = 0; i < pgc.nb_outputs_; i++) {
    bits.emplace_back(pgc.output_labels_[2 * i].GetLow() & 1);
  }

  return bits;
}

}  // anonymous namespace

namespace interstellar::packmsg {

void internal::StripCircuit(garble::ParallelGarbledCircuit *pgc,
                            PrePackmsg *pre_packmsg,
                            const std::vector<uint8_t> &digits,
                            absl::BitGenRef bitgen) {
  // TODO switch to correct internal::PackmsgDigitSegmentsType based on Circuit
  // Metadata (eg config?)
  auto labels = internal::PrepareInputLabels(
      *pgc, digits, internal::PackmsgDigitSegmentsType::seven_segs);

  auto xormask = PrepareXormask(*pgc);

  StripParallelGarbledCircuit(pgc, bitgen);

  pre_packmsg->SetLabels(std::move(labels));
  pre_packmsg->SetXormask(std::move(xormask));
  pre_packmsg->SetConfig(std::move(pgc->config_));

  // config: remove most fields from the pgc, but keep ONLY the ones needed for
  // evaluation
  absl::flat_hash_set<std::string_view> config_whitelist{"WIDTH", "HEIGHT"};
  for (auto const &[key, val] : pre_packmsg->GetConfig()) {
    if (config_whitelist.contains(key)) {
      pgc->config_.try_emplace(key, val);
    }
  }
}

void StripCircuit(garble::ParallelGarbledCircuit *pgc, PrePackmsg *pre_packmsg,
                  const std::vector<uint8_t> &digits) {
  absl::BitGen bitgen;
  return internal::StripCircuit(pgc, pre_packmsg, digits, bitgen);
}

}  // namespace interstellar::packmsg