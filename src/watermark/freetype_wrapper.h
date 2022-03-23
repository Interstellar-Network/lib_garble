#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_BITMAP_H

#include <cassert>
#include <map>
#include <memory>
#include <stdexcept>

/**
 * Singleton !
 *
 * Wrapper for the Freetype library
 *
 * WARNING: currently the Font cache is read-only(ie completely init in the
 * ctor) We could alternatively choose to init a given size when needed, and
 * same thing for a given Glyph, but in pratice we need only one font size per
 * circuit size.
 *
 * WARNING: NOT thread-safe(technically)
 * Currently not a problem because it is "read only".
 */
class FreeType {
 public:
  /**
   * Returns the instance
   * NOTE: NOT const, the FT_* functions work directly on the FT_Library/FT_Face
   *
   */
  static FreeType &GetInstance() {
    static FreeType instance;
    return instance;
  }

  // NO COPY
  // NO MOVE
  FreeType(FreeType const &) = delete;
  void operator=(FreeType const &x) = delete;

  FT_Bitmap *GetChar(wchar_t char_to_draw, unsigned int font_size,
                     int *advance_x, int *bitmap_left, int *bitmap_top);

  /**
   * one "line_height" per font size
   */
  size_t GetLineHeight(unsigned int font_size) const {
    assert(line_heights_.find(font_size) !=
           line_heights_.end());  // font_size must be in line_heights_
    return line_heights_.at(font_size);
  }

  /**
   * Convert a bit bitmap(=monochrome) to a byte bitmap.
   * Called when converting a monochrome bitmap into a CImg.
   */
  std::shared_ptr<FT_Bitmap> FreeType_Bitmap_Convert(
      const FT_Bitmap *source) const {
    FT_Bitmap *new_bitmap = new FT_Bitmap;

    // Works without this, but valgrind will have lots of "Conditional jump or
    // move depends on uninitialised value(s)"
    FT_Bitmap_Init(new_bitmap);

    // NOTE: to have a proper cleanup, we need a custom deleter, so we need
    // can't use make_shared
    auto target = std::shared_ptr<FT_Bitmap>(
        new_bitmap, [library = library_](FT_Bitmap *bitmap) {
          FT_Bitmap_Done(library, bitmap);
          delete bitmap;  // not done by FT_Bitmap_Done...
        });

    // If not 1, the caller has to handle it when creating a CImg from the
    // FT_Bitmap
    unsigned int alignment = 1;

    FT_Bitmap_Convert(library_, source, target.get(), alignment);

    assert(target->width == source->width && target->rows == source->rows);

    return target;
  }

 private:
  FT_Library library_;
  FT_Face face_;

  // Cache the FT_Glyph
  // because the call to FT_Load_Glyph is expensive
  // We can't do it with only a vector :
  // - count: 2846, max_index: 3247, max_charcode : 65532
  //
  // As we don't handle a lot of circuit sizes, we have a per-font-size cache
  // ie a per-font-size map : wchar_t -> FT_Glyph
  //
  // We would need both a glyphindex -> FT_Glyph vector AND a char -> glyphindex
  // (std::map since it's sparse max_charcode : 65532)
  // -> The easiest solution is to have ONE map: charcode -> FT_Glyph
  // TODO use shared_ptr<FT_Glyph>
  // TODO(unordered_map?)
  using MapWcharGlyph = std::map<unsigned int, FT_Glyph>;
  std::map<unsigned int, MapWcharGlyph> glyphs_;

  // Needed for the Kerning
  // TODO(kerning) test with a font that supports kerning
  FT_Bool font_has_kerning_;
  FT_UInt previous_glyph_index_;

  // Updated when loading the .ttf
  // NOTE: one "line_height" per font size
  // TODO(unordered_map?)
  std::map<unsigned int, size_t> line_heights_;

  /**
   * One cache per font size
   */
  void InitGlyphsCache(unsigned int font_size);

  /**
   * private constructor since this is a singleton
   */
  FreeType();

  ~FreeType();
};