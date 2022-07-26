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

#include "watermark.h"

#include <glog/logging.h>
#include <utf8.h>

#include "freetype_wrapper.h"

namespace {

/**
 * Converts a FT Bitmap into a valid CImg.
 * Useful when handling monochrome FT Bitmap, because we need to convert a 0-1
 * bitmap into a valid image, which CImg expects to be 0-255.
 */
// TODO Should probably be cached
// TODO should return a vector<bool>
std::vector<uint8_t> FTBitmapToCImg(const FT_Bitmap *bitmap,
                                    const FreeType &free_type) {
  if (bitmap->pixel_mode == FT_PIXEL_MODE_MONO) {
    // FT_Bitmap in FT_PIXEL_MODE_MONO is 1 bit per pixel, and CImg wants 1 byte
    // per pixel cf
    // https://www.freetype.org/freetype2/docs/reference/ft2-basic_types.html#FT_Bitmap

    std::shared_ptr<FT_Bitmap> converted_bitmap =
        free_type.FreeType_Bitmap_Convert(bitmap);

    // Read from buffer, NOT bitmap->buffer
    std::vector<uint8_t> img_from_bitmap;
    img_from_bitmap.assign(
        converted_bitmap->buffer,
        converted_bitmap->buffer +
            converted_bitmap->width * converted_bitmap->rows);

    return img_from_bitmap;
  }
  if (bitmap->pixel_mode == FT_PIXEL_MODE_GRAY) {
    LOG(ERROR) << "FTBitmapToCImg : NotImplementedError : FT_PIXEL_MODE_GRAY";
    abort();
  } else {
    LOG(ERROR) << "FTBitmapToCImg : unsupported bitmap->pixel_mode : "
               << bitmap->pixel_mode;
    abort();
  }
}

}  // anonymous namespace

namespace interstellar::internal::watermark {

// TODO randomize position
// TODO layout text with harfbuzz(cf skia code), and justify?
std::vector<uint8_t> DrawText(const std::string &text_to_draw,
                              uint32_t font_size, uint32_t width,
                              uint32_t height) {
  // No need to work if there is not text to draw
  std::vector<uint8_t> img;
  // NOTE: ToBits can be called before doing any kind of drawing(ie just after
  // the ctor) so we init the image
  img.assign(width * height, 0);

  if (text_to_draw.length() == 0u) {
    return img;
  }

  // Force upper case
  // TODO option
  // std::wstring text_to_draw_upper;
  // text_to_draw_upper.resize(text_to_draw.length());
  // std::transform(text_to_draw.begin(), text_to_draw.end(),
  //                text_to_draw_upper.begin(), ::toupper);

  // convert to UTF8
  std::string text_to_draw_copy;
  utf8::replace_invalid(text_to_draw.begin(), text_to_draw.end(),
                        back_inserter(text_to_draw_copy));

  // TODO call find max font size and others

  unsigned int img_width = width, img_height = height;

  int pen_x = 0, advance_x = 0;

  // controls the horizontal offset for the whole message
  // helps centering it a bit
  // NOTE: DO NOT use a fixed offset, it needs to depends on the font size
  unsigned int offset_x = 0;

  // Handle multiline: += line_height each time we encounter a '\n'
  size_t offset_height = 0;  // img_height / 2

  FreeType &free_type = FreeType::GetInstance();

  uint32_t curr_char = 0;
  auto text_begin = text_to_draw_copy.begin();
  auto text_end = text_to_draw_copy.end();

  while (text_begin != text_end) {
    int bitmap_left, bitmap_top;

    curr_char = utf8::next(text_begin, text_end);

    FT_Bitmap *bitmap = free_type.GetChar(curr_char, font_size, &advance_x,
                                          &bitmap_left, &bitmap_top);

    // Handle multiline: += line_height each time we encounter a '\n'
    if (curr_char == '\n') {
      offset_height += free_type.GetLineHeight(font_size);
      pen_x = 0;
      continue;  // DO NOT draw in this case
    }

    // WARNING: bitmap will be a nullptr in case of newline -> MUST "continue;"

    // If the glyph is not found(but it's not a newline)
    // replace it with some placeholder glyph
    if (bitmap == nullptr) {
      LOG(ERROR) << "DrawText : glyph not found : ";
      bitmap = free_type.GetChar(L'¶', font_size, &advance_x, &bitmap_left,
                                 &bitmap_top);
    }

    // Fix the vertical offset for the first time
    if (offset_height == 0u) {
      offset_height = free_type.GetLineHeight(font_size);
    }

    // using line height as horizontal offset looks quite good
    if (offset_x == 0u) {
      offset_x = 2 * offset_height;
    }

    std::vector<uint8_t> letter = FTBitmapToCImg(bitmap, free_type);

    unsigned int letter_height = bitmap->rows;

    // loop on letter_width, NOT cimg_forY(letter,x)(=letter.width()) !
    for (unsigned int letter_width_it = 0; letter_width_it < bitmap->width;
         letter_width_it++) {
      // DO NOT add advance_x here !
      // advance_x is what is needed to draw the NEXT char
      // without bitmap_left, some chars would end up without any space between
      // them
      unsigned draw_pos_x = offset_x + letter_width_it + pen_x + bitmap_left;

      // make sure we don't try to draw out of the image
      if (draw_pos_x >= img_width) {
        continue;
      }

      for (unsigned int letter_height_it = 0; letter_height_it < letter_height;
           letter_height_it++) {
        // skip the drawing if there is nothing to draw
        uint8_t letter_px =
            letter.at(letter_width_it + letter_height_it * bitmap->width);
        if (letter_px != 0u) {
          unsigned int final_draw_y =
              letter_height_it + offset_height - bitmap_top;

          // make sure we don't try to draw out of the image
          if (final_draw_y >= img_height) {
            continue;
          }

          img.at(draw_pos_x + final_draw_y * width) = 255;
        }
      }
    }
    pen_x += advance_x;
  }

  return img;
}

/**
 * IMPORTANT : the resulted is used to construct the Packmsg, and so used by
 * xor_bits: the outputs MUST be 0 OR 1
 */
// TODO could probably return a binary image in "DrawText" and skip
// this
std::vector<uint8_t> WatermarkToBits(const std::vector<uint8_t> &img) {
  std::vector<uint8_t> pixels;
  size_t img_size = img.size();
  pixels.resize(img_size);

  for (unsigned int i = 0; i < img_size; i++) {
    pixels[i] = img[i] > 128 ? 1 : 0;
  }

  return pixels;
}

}  // namespace interstellar::internal::watermark