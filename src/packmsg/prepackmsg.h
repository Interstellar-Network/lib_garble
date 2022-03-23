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

#include <absl/container/flat_hash_map.h>

#include <vector>

#include "justgarble/block.h"

namespace interstellar::packmsg {

/**
 * It is NOT constructed directly; it is returned by StripCircuit instead
 *
 * Strip a given PGC; that is a DESTRUCTIVE operation!
 * It CAN NOT be evaluated without the corresponding .packmsg after this.
 * At this point we "hardcode" eg an OTP in the circuit(or the permutation for
 * the pinpad).
 * NOTE: for now it is (too) specific to the Display circuits; but it COULD be
 * generalized to other circuits(AES, adder, etc?)
 *
 * It is basically patching the original circuit(verilog/skcd) with a given set
 * of inputs(labels_: for the OTP/pinpad) and/or outputs(xormask_: for a
 * message).
 * For this reason, it REQUIRES a bunch of variables that are in
 * Verilog `define! They are passed around from the Verilog all the way to the
 * PGC in a "config" field.
 * Another way to see it is we could achieve the same result by skipping the
 * whole PrePackmsg/Packmsg and instead hardcoding a bunch of inputs/outputs in
 * the original .skcd if we generated them on the fly(ie at transaction time
 * instead of ahead of time).
 * Or better: add the equivalent of PrePackmsg but for a .skcd, and apply the
 * patch BEFORE garbling. That way we can keep a .skcd and reuse it many times,
 * while still having the possibility to "burn" a given OTP/message in the
 * .pgarbled file.
 *
 * NOTE: stripping a circuit CAN also be used to send a big circuit ahead of
 * time, to avoid having to DL several MB at tx time!
 *
 * return: a PrePackmsg that is used to create the .packmsg with a given
 * Watermark(eg a transaction message)
 *
 * Typically:
 * - the Prepackmsg is stored next to a "circuit ID"
 * - the stripped circuit is sent to a user (ahead of time or not)
 * - [LATER] a user request a circuit for eval; at this point we use the
 * PrePackmsg to generate the Watermark into a Packmsg and send it to the user
 * - client: combine the StrippedCircuit with the Packmsg and eval
 */
class PrePackmsg {
 public:
  /**
   * INTERNAL USE ONLY ctor
   * The public API SHOULD construct using "StripCircuit"
   */
  // TODO private?
  PrePackmsg(std::vector<Block> &&labels, std::vector<uint8_t> &&xormask,
             const absl::flat_hash_map<std::string, uint32_t> &config);

  /**
   * DefaultConstructible
   * Needed by the caller of this b/c PrePackmsg is passed by pointer in
   * packmsg_helper.h
   */
  PrePackmsg();

  // NO COPY
  // NO MOVE
  PrePackmsg(PrePackmsg const &) = delete;
  void operator=(PrePackmsg const &x) = delete;

  const auto &GetLabels() const { return labels_; };
  const auto &GetXormask() const { return xormask_; };
  const auto &GetConfig() const { return config_; };

  void SetLabels(std::vector<Block> &&labels);
  void SetXormask(std::vector<uint8_t> &&xormask);
  void SetConfig(absl::flat_hash_map<std::string, uint32_t> &&config);

  // INTERNAL/TEST ONLY
  bool operator==(const PrePackmsg &other) const {
    return (labels_ == other.labels_) && (xormask_ == other.xormask_) &&
           (config_ == other.config_);
  };

 private:
  std::vector<Block> labels_;
  std::vector<uint8_t> xormask_;
  // copy of the config passed all the way from Verilog/.skcd to here
  absl::flat_hash_map<std::string, uint32_t> config_;
};

}  // namespace interstellar::packmsg