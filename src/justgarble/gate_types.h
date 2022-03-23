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

// MUST (mostly) match lib_circuit
// /interstellar/lib_circuits/src/blif/gate_types.h
// TODO move to shared submodule

namespace interstellar {

/**
From lib_python/lib_python/gen_skcd/skcd.py

NOTE: there is already a class GateType in ABC
*/
enum class SkcdGateType : u_int8_t {
  ZERO,
  NOR,
  AANB,
  INVB,
  NAAB,
  INV,
  XOR,
  NAND,
  AND,
  XNOR,
  BUF,
  AONB,
  BUFB,
  NAOB,
  OR,
  ONE
};

}  // namespace interstellar