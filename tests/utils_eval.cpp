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

#include "utils_eval.h"

// TODO Define 'cimg_display' to: '0' to disable display capabilities.
//                           '1' to use the X-Window framework (X11).
//                           '2' to use the Microsoft GDI32 framework.
// else lots of eg "CImg.h:10322: undefined reference to `XMoveWindow'"
#define cimg_display 1
#include <CImg.h>

#include <random>

#include "evaluate/evaluate.h"

namespace {

using interstellar::garble::ParallelGarbledCircuit;

void FinalizeOutputsAndDisplay(std::vector<u_int8_t> *outputs, u_int32_t width,
                               uint32_t height) {
  // outputs are (0,1) but an image SHOULD be (0,255)
  std::transform(outputs->begin(), outputs->end(), outputs->begin(),
                 [](auto c) { return c * 255; });

  // Prepare the display with the desired dimensions
  // MUST use ctor with "value" else
  // "Warning
  //       The allocated pixel buffer is not filled with a default value, and is
  //       likely to contain garbage values. In order to initialize pixel values
  //       during construction (e.g. with 0), use constructor CImg(unsigned
  //       int,unsigned int,unsigned int,unsigned int,T) instead."
  auto display_img = cimg_library::CImg<uint8_t>(
      /* size_x */ width,
      /* size_y */ height,
      /* size_z */ 1,
      /* size_c = spectrum = nb of channels */ 1,
      /* value */ 0);
  assert(display_img.size() == outputs->size() && "wrong resolution!");

  std::copy(outputs->begin(), outputs->end(), display_img.begin());

  display_img.display();
}

template <typename F>
void BaseEvalAndDisplay(const ParallelGarbledCircuit &parallel_garbled_circuit,
                        u_int32_t nb_evals, F eval_func) {
  // TODO random?
  // TODO std::vector<block> PrepareInputLabels ?
  std::vector<u_int8_t> inputs(parallel_garbled_circuit.nb_inputs_);
  // RNG
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dis;

  auto width = parallel_garbled_circuit.config_.at("WIDTH");
  auto height = parallel_garbled_circuit.config_.at("HEIGHT");

  std::vector<u_int8_t> outputs;
  // combine multiple eval
  for (uint32_t i = 0; i < nb_evals; ++i) {
    for (unsigned int j = 0; j < parallel_garbled_circuit.nb_inputs_; j++) {
      inputs[j] = (dis(gen) % 2);
    }

    auto outputs_temp = eval_func(parallel_garbled_circuit, inputs);
    size_t outputs_temp_size = outputs_temp.size();

    if (outputs_temp_size != width * height) {
      throw std::runtime_error("wrong resolution!");
    }

    outputs.resize(outputs_temp_size);
    for (uint32_t j = 0; j < outputs_temp_size; ++j) {
      outputs[j] |= outputs_temp[j];
    }
  }

  FinalizeOutputsAndDisplay(&outputs, width, height);
}

}  // anonymous namespace

namespace interstellar {

namespace testing {

void EvalAndDisplay(
    const garble::ParallelGarbledCircuit &parallel_garbled_circuit,
    u_int32_t nb_evals) {
  BaseEvalAndDisplay(parallel_garbled_circuit, nb_evals,
                     [](const garble::ParallelGarbledCircuit &pgc,
                        const std::vector<u_int8_t> &in) {
                       return interstellar::garble::EvaluateWithInputs(pgc, in);
                     });
}

void EvalAndDisplayWithPackmsg(
    const garble::ParallelGarbledCircuit &parallel_garbled_circuit,
    const packmsg::Packmsg &packmsg, u_int32_t nb_evals) {
  BaseEvalAndDisplay(
      parallel_garbled_circuit, nb_evals,
      [&p = std::as_const(packmsg)](const garble::ParallelGarbledCircuit &pgc,
                                    const std::vector<u_int8_t> &in) {
        return interstellar::garble::EvaluateWithPackmsg(pgc, in, p);
      });
}

}  // namespace testing

}  // namespace interstellar