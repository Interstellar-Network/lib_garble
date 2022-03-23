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

#include <string>
#include <vector>

namespace interstellar::internal::watermark {

/**
 * return: an "image"(as a raw buffer)
 */
std::vector<uint8_t> DrawText(const std::wstring &text_to_draw,
                              uint32_t font_size, uint32_t width,
                              uint32_t height);

std::vector<uint8_t> WatermarkToBits(const std::vector<uint8_t> &img);

}  // namespace interstellar::internal::watermark