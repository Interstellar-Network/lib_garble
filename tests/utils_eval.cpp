#include "utils_eval.h"

// TODO Define 'cimg_display' to: '0' to disable display capabilities.
//                           '1' to use the X-Window framework (X11).
//                           '2' to use the Microsoft GDI32 framework.
// else lots of eg "CImg.h:10322: undefined reference to `XMoveWindow'"
#define cimg_display 1
#include <CImg.h>

#include <random>

#include "evaluate/evaluate.h"

namespace interstellar {

namespace testing {

void EvalAndDisplay(const ParallelGarbledCircuit &parallel_garbled_circuit,
                    uint32_t width, uint32_t height, u_int32_t nb_evals) {
  // TODO random?
  // TODO std::vector<block> PrepareInputLabels ?
  std::vector<u_int8_t> inputs(parallel_garbled_circuit.nb_inputs_);
  // RNG
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dis;

  std::vector<u_int8_t> outputs;
  // combine multiple eval
  for (uint32_t i = 0; i < nb_evals; ++i) {
    for (unsigned int j = 0; j < parallel_garbled_circuit.nb_inputs_; j++) {
      inputs[j] = (dis(gen) % 2);
    }

    auto outputs_temp = interstellar::garble::EvaluateWithInputs(
        parallel_garbled_circuit, inputs);
    size_t outputs_temp_size = outputs_temp.size();

    if (outputs_temp_size != width * height) {
      throw std::runtime_error("wrong resolution!");
    }

    outputs.resize(outputs_temp_size);
    for (uint32_t j = 0; j < outputs_temp_size; ++j) {
      outputs[j] |= outputs_temp[j];
    }
  }

  // outputs are (0,1) but an image SHOULD be (0,255)
  std::transform(outputs.begin(), outputs.end(), outputs.begin(),
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
  assert(display_img.size() == outputs.size() && "wrong resolution!");

  std::copy(outputs.begin(), outputs.end(), display_img.begin());

  display_img.display();
}

}  // namespace testing

}  // namespace interstellar