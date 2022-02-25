#include "garble_helper.h"

#include "serialize/serialize.h"

namespace interstellar {

namespace garblehelper {

/**
 * Garble a ".skcd"(in pratice a .blif.blif, given as BlifParser)
 * NOTE: contrary to its name, a "GarbledCircuit" here is basically just a SKCD
 * circuit.
 *
 * archive version: "Base" function: garble a .skcd given by path.
 *
 * return a ParallelGarbledCircuit
 */
std::string GarbleSkcdToBuffer(boost::filesystem::path skcd_input_path) {
  GarbledCircuit garbledCircuit(skcd_input_path);

  garbledCircuit.garbleCircuit();

  // Now for the parallel part
  ParallelGarbledCircuit pgc{std::move(garbledCircuit)};

  return garble::Serialize(pgc);
}

void GarbleSkcdToFile(boost::filesystem::path skcd_input_path,
                      boost::filesystem::path pgarbled_output_path) {
  GarbledCircuit garbledCircuit(skcd_input_path);

  garbledCircuit.garbleCircuit();

  // Now for the parallel part
  ParallelGarbledCircuit pgc{std::move(garbledCircuit)};

  garble::Serialize(pgc, pgarbled_output_path);
}

}  // namespace garblehelper

}  // namespace interstellar