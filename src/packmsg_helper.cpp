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

#include "packmsg_helper.h"

#include <absl/strings/str_join.h>
#include <glog/logging.h>

#include "garble_helper.h"
#include "packmsg/packmsg_utils.h"
#include "packmsg/stripped_circuit.h"
#include "parallel_garbled_circuit/parallel_garbled_circuit.h"
#include "watermark/watermark.h"

using namespace interstellar;

void interstellar::packmsg::GarbleAndStrippedSkcdFromBuffer(
    std::string_view skcd_buffer, garble::ParallelGarbledCircuit *pgc,
    PrePackmsg *pre_packmsg, const std::vector<uint8_t> &digits) {
  *pgc = garble::GarbleSkcdFromBuffer(skcd_buffer);

  // CHECK: is it a "display circuit"
  uint32_t nb_digits_expected;
  const auto config_it = pgc->config_.find("NB_DIGITS");
  if (config_it != pgc->config_.end()) {
    // element found;
    nb_digits_expected = config_it->second;
  } else {
    // TODO improve the check, check more fields? the pgc itself?
    // or better: handle generic circuits?
    LOG(ERROR) << "GarbleAndStrippedSkcdFromBuffer not a display circuit";
    throw std::logic_error(
        "GarbleAndStrippedSkcdFromBuffer: not a display circuit");
  }

  // CHECK: if digits are valid
  // - size MUST match NB_DIGITS
  if (digits.size() != nb_digits_expected) {
    LOG(ERROR) << "GarbleAndStrippedSkcdFromBuffer \"digits\" param MUST match "
                  "config[NB_DIGITS]";
    throw std::logic_error(
        "GarbleAndStrippedSkcdFromBuffer \"digits\" param MUST match "
        "config[NB_DIGITS]");
  }
  // - each MUST be [0-9]
  if (std::any_of(digits.cbegin(), digits.cend(), [](uint8_t elem) {
        // "elem < 0" NOT needed b/c uint8_t "error: comparison is always true
        // due to limited range of data type"
        return elem > 9;
      })) {
    LOG(ERROR) << "GarbleAndStrippedSkcdFromBuffer \"digits\" param MUST only "
                  "contain [0-9] ";
    throw std::logic_error(
        "GarbleAndStrippedSkcdFromBuffer \"digits\" param MUST only contain "
        "[0-9]");
  }

  StripCircuit(pgc, pre_packmsg, digits);
}

packmsg::Packmsg interstellar::packmsg::PackmsgFromPrepacket(
    const packmsg::PrePackmsg &pre_packmsg,
    const std::string &watermark_message) {
  // either set when deserializing, or directly by Prepackmsg
  unsigned int width = pre_packmsg.GetConfig().at("WIDTH");
  unsigned int height = pre_packmsg.GetConfig().at("HEIGHT");

  // select the correct font size
  // we could do it dynamically, but we only need to support a few circuit sizes
  uint32_t font_size = 0;
  if (width <= 224) {
    // old size 1
    font_size = 8;
  } else if (width <= 360) {
    // new size 1
    font_size = 10;
  } else if (width <= 224 * 2) {
    // old size 2
    font_size = 12;
  } else if (width <= 540) {
    // new size 2
    font_size = 16;
  } else {
    // ???
    font_size = 24;
    LOG(WARNING) << "Unexpected width found : " << width;
  }

  // create the message watermark
  std::vector<uint8_t> watermark_img = ::internal::watermark::DrawText(
      watermark_message, font_size, width, height);

  std::vector<uint8_t> watermark_bits =
      ::internal::watermark::WatermarkToBits(watermark_img);

  assert(watermark_bits.size() == width * height);

  // TODO xor in place; eg XorBits(pre_packmsg.GetXormask(), &watermark_bits)
  std::vector<uint8_t> xormask_bits =
      internal::XorBits(pre_packmsg.GetXormask(), watermark_bits);
  // TODO remove pack_bits64
  auto xormask_words = internal::pack_bits64(xormask_bits);

  return packmsg::Packmsg(pre_packmsg.GetLabels(), std::move(xormask_words));
}
