#include "freetype_wrapper.h"

#include <absl/strings/str_cat.h>
#include <glog/logging.h>

#include <vector>

#include "resources.h"

FreeType::FreeType() {
  DLOG(INFO) << "FreeType: constructor START";

  int error;

  error = FT_Init_FreeType(&library_);
  if (error != 0) {
    throw std::runtime_error("FtInit: FT_Init_FreeType failed");
  }

  // Step 2: init the FT_Face from the .ttf font

  // TODO: correct font path(put the ttf in lib_server)
  error = FT_New_Face(
      library_,
      absl::StrCat(interstellar::garble::data_dir, "/Beatles Light Light.ttf")
          .c_str(),
      0, &face_);
  if (error == FT_Err_Unknown_File_Format) {
    throw std::runtime_error(
        "FtGetChar: error: the font file could be opened and read, but it "
        "appears, that its font format is unsupported");
  }
  if (error != 0) {
    // File not found, etc
    throw std::runtime_error("FtGetChar: error: could not open the font");
  }

  font_has_kerning_ = FT_HAS_KERNING(face_);
  DLOG(INFO) << "Font support kerning : " << font_has_kerning_;

  // WARNING: should probably be done after all calls to FT_Set_Char_Size
  // TODO have a "per char size" glyphs cache

  DLOG(INFO) << "FreeType: constructor END";
}

FreeType::~FreeType() {
  DLOG(INFO) << "FreeType: destructor START";

  for (const auto &map_wchar_glyph : glyphs_) {
    for (auto glyph : map_wchar_glyph.second) {
      FT_Done_Glyph(glyph.second);
    }
  }

  FT_Done_Face(face_);
  FT_Done_FreeType(library_);
  DLOG(INFO) << "FreeType: destructor END";
}

/**
 * Init the internal cache for the glyphs by looping on all glyphs in the
 * charmap
 *
 * NOTE: we could choose to load only the most used(eg ascii+fr) glyphs now, and
 * add the missing ones in an on-demand fashion (= add a "load if not exists" in
 * GetChar)
 *
 * http://refspecs.linux-foundation.org/freetype/freetype-doc-2.1.10/docs/reference/ft2-base_interface.html#FT_Get_First_Char
 */
#define DEBUG_InitGlyphsCache 1
void FreeType::InitGlyphsCache(unsigned int font_size) {
  FT_ULong charcode;
  FT_UInt gindex;

  // "screen dpi"
  // alternatively we could use FT_Set_Pixel_Sizes since we are not drawing to a
  // screen
  unsigned int horizontal_device_resolution = 240,
               vertical_device_resolution = 240;

  // warning: this is quite slow: DO NOT do it for each char/draw
  int error = FT_Set_Char_Size(
      face_, /* handle to face object           */
      0,     /* char_width in 1/64th of points ; 0 means same as char_height */
      font_size * 64, /* char_height in 1/64th of points */
      horizontal_device_resolution, vertical_device_resolution);
  if (error != 0) {
    LOG(ERROR) << "FreeType::InitGlyphsCache: could not FT_Set_Char_Size: "
               << error;
    throw std::runtime_error(
        "FreeType::InitGlyphsCache: could not FT_Set_Char_Size");
  }

  // First loop to count the number of glyphs
  // This we can correctly size the cache
  // eg: InitGlyphsCache : count: 2846, max_index: 3247, max_charcode : 65532
  unsigned int count = 0, max_index = 0, max_charcode = 0;
  charcode = FT_Get_First_Char(face_, &gindex);
  while (gindex != 0) {
    charcode = FT_Get_Next_Char(face_, charcode, &gindex);
    count++;

    if (gindex > max_index) {
      max_index = gindex;
    }

    if (charcode > max_charcode) {
      max_charcode = charcode;
    }
  }

  // Second loop to init the cache
  // https://www.freetype.org/freetype2/docs/reference/ft2-glyph_management.html#FT_Glyph_To_Bitmap
  charcode = FT_Get_First_Char(face_, &gindex);
  while (gindex != 0) {
    // DO NOT set FT_LOAD_RENDER, that would be pointless...
    // TODO use flag FT_LOAD_RENDER|FT_LOAD_MONOCHROME to disable AA
    error =
        FT_Load_Glyph(face_, gindex,
                      FT_LOAD_RENDER | FT_LOAD_MONOCHROME);  // FT_LOAD_NO_SCALE
    if (error != 0) {
      // TODO
      LOG(ERROR) << "InitGlyphsCache: could not FT_Load_Glyph : %#04x" << error;
    }

    FT_Glyph glyph;
    error = FT_Get_Glyph(face_->glyph, &glyph);
    if (error != 0) {
      // TODO
      LOG(ERROR) << "InitGlyphsCache: could not FT_Get_Glyph : %#04x" << error;
    }

    // NO NEED for FT_Glyph_To_Bitmap : the rendering is done when drawing
    // TODO have a "per font size" bitmap cache
    // error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_MONO, 0, 1); //
    // FT_RENDER_MODE_MONO, FT_RENDER_MODE_NORMAL if(error){
    //     // TODO
    //     LOGE("InitGlyphsCache: could not FT_Glyph_To_Bitmap : %#04x", error);
    // }

    // Don't call FT_Done_Glyph here, it's done in the dtor
    // else "double free or corruption"

    glyphs_[font_size][charcode] = glyph;

    charcode = FT_Get_Next_Char(face_, charcode, &gindex);
  }

  // "metrics" needs a glyph to be loaded
  line_heights_[font_size] =
      face_->size->metrics.height /
      64;  // FT_Size_Metrics_.height; text height in 26.6 frac. pixels
}

/**
 * cf https://www.freetype.org/freetype2/docs/tutorial/step1.html
 * "b.Refined code"
 *
 * Prepare the font_face to draw the given char.
 * The char bitmap can then be accessed with: "face->glyph->bitmap"
 *
 * wchar_t: because we want to support unicode
 *
 * return:
 * - advance_x : the advance to apply to the pen(already in pixel coords),
 *  including kerning if applicable
 *
 * bench:[release]
 * - without cache: BM_Watermark_cimg_sin/iterations:1000           382863 ns
 * 382875 ns       1000
 * - with         : BM_Watermark_cimg_sin/iterations:1000            81649 ns
 * 81638 ns       1000
 */
#define DEBUG_ft_getchar 0
FT_Bitmap *FreeType::GetChar(wchar_t char_to_draw, unsigned int font_size,
                             int *advance_x, int *bitmap_left,
                             int *bitmap_top) {
#if 0  // direct or cache

    FT_GlyphSlot  slot = face_->glyph;
    FT_UInt       glyph_index;
    int           n;

    int error;

    /* load glyph image into the slot (erase previous one) */
    // TODO FT_LOAD_MONOCHROME
    error = FT_Load_Char(face_, char_to_draw, FT_LOAD_RENDER); // FT_LOAD_RENDER, FT_LOAD_MONOCHROME
    if (error){
        LOGE("FreeType::GetChar: could not load the char: %#04x", error);
        return nullptr;
    }

     /* increment pen position */
    pen_x += slot->advance.x >> 6; // >> 6 = * 64; cf "char_height in 1/64th of points"

    // vertical offset for the glyph
    // because not all glyphs have the same bounding box
    bitmap_top = slot->bitmap_top; // =, NOT += because it's valid per glyph

    // "metrics" needs a glyph to be loaded
    // we could alternatively load any glyph in the ctor
    line_height = face_->size->metrics.height / 64; // FT_Size_Metrics_.height; text height in 26.6 frac. pixels

    return &(slot->bitmap);

#else
  // Generate the glyphs cache if needed(reminder: this class is a singleton)
  if (glyphs_.find(font_size) == glyphs_.end()) {
    LOG(INFO) << "FreeType::GetChar : font size not in cache; generating...";
    InitGlyphsCache(font_size);
    LOG(INFO) << "FreeType::GetChar : cache ready!";
  }

  FT_Glyph glyph = glyphs_[font_size][char_to_draw];

  *advance_x = 0;

  if (glyph == nullptr) {
    return nullptr;
  }

  // TODO(kerning) no need to call FT_Get_Char_Index if !font_has_kerning_
  FT_UInt glyph_index = FT_Get_Char_Index(face_, char_to_draw);
  if ((font_has_kerning_ != 0u) && (previous_glyph_index_ != 0u) &&
      (glyph_index != 0u)) {
    FT_Vector delta;

    FT_Get_Kerning(face_, previous_glyph_index_, glyph_index,
                   FT_KERNING_DEFAULT, &delta);

    *advance_x += delta.x >> 6;
  }

  // increment pen position
  // NOTE:slot->advance.x is "in 26.6 fixed-point format"
  //  and glyph->advance.x is "16.16 fixed-point numbers"
  // so instead of divising by 64 like when using slot, we divide by 65536
  *advance_x += glyph->advance.x / 65536;

  auto glyph_bitmap = reinterpret_cast<FT_BitmapGlyph>(glyph);

  // Return the correct offsets to apply when drawing
  *bitmap_top = glyph_bitmap->top;
  *bitmap_left = glyph_bitmap->left;

  // Store the previous, for the kerning computation
  previous_glyph_index_ = glyph_index;

  return &(glyph_bitmap->bitmap);

#endif
}
