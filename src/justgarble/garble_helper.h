// Copyright (c) 2018 Skeyecode. All rights reserved.
// author : Nathan Prat
// date : 2018-03-06

#pragma once

#include <boost/filesystem.hpp>

#include "parallel.h"

namespace interstellar {

namespace garblehelper {

/**
 * Garble a ".skcd"(in pratice a .blif.blif, given as BlifParser)
 * NOTE: contrary to its name, a "GarbledCircuit" here is basically just a SKCD
 * circuit.
 *
 * archive version: "Base" function: garble a .skcd given by path.
 *
 * return a ParallelGarbledCircuit serialized with Protobuf
 */
std::string GarbleSkcdToBuffer(boost::filesystem::path skcd_input_path);

void GarbleSkcdToFile(boost::filesystem::path skcd_input_path,
                      boost::filesystem::path pgarbled_output_path);

}  // namespace garblehelper

}  // namespace interstellar