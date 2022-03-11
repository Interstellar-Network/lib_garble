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

/*
    Those functions were included in JustGarble, but are in fact based on
    https://software.intel.com/en-us/articles/fast-random-number-generator-on-the-intel-pentiumr-4-processor
*/

#ifndef UTIL_H_
#define UTIL_H_

#include <emmintrin.h>

__m128i rand_sse();

void srand_sse(unsigned int seed);

#endif /* UTIL_H_ */
