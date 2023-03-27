/*  This file is part of assfonts.
 *
 *  assfonts is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation,
 *  either version 2 of the License,
 *  or (at your option) any later version.
 *
 *  assfonts is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with assfonts. If not, see <https://www.gnu.org/licenses/>.
 *  
 *  written by wyzdwdz (https://github.com/wyzdwdz)
 */

#include "font_subsetter.h"

#include <algorithm>
#include <climits>
#include <exception>
#include <filesystem>
#include <fstream>

#include <fmt/core.h>
#include <harfbuzz/hb-subset.h>

static const std::u32string ADDITIONAL_CODEPOINTS =
    U"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

namespace fs = std::filesystem;

namespace ass {

void FontSubsetter::SetSubfontDir(const AString& subfont_dir) {
  subfont_dir_ = subfont_dir;
}

bool FontSubsetter::Run(const bool& is_no_subset) {
  if (is_no_subset) {
    bool have_missing = false;
    AString path;
    long index = 0;
    for (const auto& font_set : ap_.font_sets_) {
#ifdef _WIN32
      AString fontname = U8ToWide(font_set.first.fontname);
#else
      AString fontname(font_set.first.fontname);
#endif
      if (!FindFont(font_set, fp_.font_list_, path, index) &&
          !FindFont(font_set, fp_.font_list_in_db_, path, index)) {
        logger_->warn(_ST("Missing the font: \"{}\" ({},{})"), fontname,
                      font_set.first.bold, font_set.first.italic);
        have_missing = true;
      } else {
        logger_->info(_ST("Found font: \"{}\" ({},{}) --> \"{}\"[{}]"),
                      fontname, font_set.first.bold, font_set.first.italic,
                      path, index);
        CheckGlyph(path, index, font_set.second);
      }
      subfonts_path_.emplace_back(path);
    }
    if (have_missing) {
      logger_->error(_ST("Found missing fonts. Check warning info above."));
      return false;
    }
    return true;
  }
  if (!set_subset_font_codepoint_sets()) {
    return false;
  }
  fs::path dir_path(subfont_dir_);
  if (!fs::exists(dir_path)) {
    logger_->info(_ST("Create subset fonts directory: \"{}\""),
                  dir_path.native());
    try {
      fs::create_directory(dir_path);
    } catch (const fs::filesystem_error& e) {
      logger_->error("Create subset fonts directory failed. Error code: {}",
                     e.what());
      return false;
    }
  }
  for (const auto& subset_font : subset_font_codepoint_sets_) {
    if (!CreateSubfont(subset_font)) {
      logger_->error(_ST("Subset failed: \"{}\"[{}]"), subset_font.first.path,
                     subset_font.first.index);
      return false;
    }
  }
  return true;
}

void FontSubsetter::Clear() {
  subfont_dir_.clear();
  subset_font_codepoint_sets_.clear();
  subfonts_path_.clear();
}

bool FontSubsetter::FindFont(
    const std::pair<AssParser::FontDesc, std::set<char32_t>>& font_set,
    const std::vector<FontParser::FontInfo>& font_list, AString& found_path,
    long& found_index) {
  bool is_found = false;
  unsigned int min_score = UINT_MAX;
  unsigned int score = UINT_MAX;
  AString tmp_path;
  long tmp_index = 0;
  unsigned int ttf_score = UINT_MAX;
  AString ttf_path;
  long ttf_index = 0;
  unsigned int otf_score = UINT_MAX;
  AString otf_path;
  long otf_index = 0;
  for (const auto& font : font_list) {
    if (std::find(font.families.begin(), font.families.end(),
                  font_set.first.fontname) != font.families.end()) {
      if (ToLower(font.path.substr(font.path.size() - 4, 4)) == _ST(".otf") ||
          ToLower(font.path.substr(font.path.size() - 4, 4)) == _ST(".otc")) {
        continue;
      }
      score = 0;
      score += std::abs(font_set.first.bold - font.weight);
      score += std::abs(font_set.first.italic - font.slant);
    } else if ((std::find(font.fullnames.begin(), font.fullnames.end(),
                          font_set.first.fontname) != font.fullnames.end()) ||
               (std::find(font.psnames.begin(), font.psnames.end(),
                          font_set.first.fontname) != font.psnames.end())) {
      if (ToLower(font.path.substr(font.path.size() - 4, 4)) == _ST(".otf") ||
          ToLower(font.path.substr(font.path.size() - 4, 4)) == _ST(".otc")) {
        continue;
      }
      score = 0;
    }
    if (score < min_score) {
      min_score = score;
      tmp_path = font.path;
      tmp_index = font.index;
    }
    if (score == 0) {
      found_path = tmp_path;
      found_index = tmp_index;
      is_found = true;
      return is_found;
    }
  }
  ttf_score = score;
  ttf_path = tmp_path;
  ttf_index = tmp_index;
  min_score = UINT_MAX;
  score = UINT_MAX;
  tmp_path.clear();
  tmp_index = 0;
  for (const auto& font : font_list) {
    if (std::find(font.families.begin(), font.families.end(),
                  font_set.first.fontname) != font.families.end()) {
      if (ToLower(font.path.substr(font.path.size() - 4, 4)) == _ST(".ttf") ||
          ToLower(font.path.substr(font.path.size() - 4, 4)) == _ST(".ttc")) {
        continue;
      }
      score = 0;
      score += std::abs(font_set.first.bold - font.weight);
      score += std::abs(font_set.first.italic - font.slant);
    } else if ((std::find(font.fullnames.begin(), font.fullnames.end(),
                          font_set.first.fontname) != font.fullnames.end()) ||
               (std::find(font.psnames.begin(), font.psnames.end(),
                          font_set.first.fontname) != font.psnames.end())) {
      if (ToLower(font.path.substr(font.path.size() - 4, 4)) == _ST(".ttf") ||
          ToLower(font.path.substr(font.path.size() - 4, 4)) == _ST(".ttc")) {
        continue;
      }
      score = 0;
    }
    if (score < min_score) {
      min_score = score;
      tmp_path = font.path;
      tmp_index = font.index;
    }
    if (score == 0) {
      break;
    }
  }
  otf_score = score;
  otf_path = tmp_path;
  otf_index = tmp_index;
  if (ttf_score < UINT_MAX || otf_score < UINT_MAX) {
    is_found = true;
    if (ttf_score <= otf_score) {
      found_path = ttf_path;
      found_index = ttf_index;
    } else {
      found_path = otf_path;
      found_index = otf_index;
    }
  }
  return is_found;
}

bool FontSubsetter::set_subset_font_codepoint_sets() {
  bool have_missing = false;
  for (const auto& font_set : ap_.font_sets_) {
    FontPath font_path;
    std::set<uint32_t> codepoint_set;
#ifdef _WIN32
    AString fontname = U8ToWide(font_set.first.fontname);
#else
    AString fontname(font_set.first.fontname);
#endif
    if (!FindFont(font_set, fp_.font_list_, font_path.path, font_path.index) &&
        !FindFont(font_set, fp_.font_list_in_db_, font_path.path,
                  font_path.index)) {
      logger_->warn(_ST("Missing the font: \"{}\" ({},{})"), fontname,
                    font_set.first.bold, font_set.first.italic);
      have_missing = true;
    } else {
      logger_->info(_ST("Found font: \"{}\" ({},{}) --> \"{}\"[{}]"), fontname,
                    font_set.first.bold, font_set.first.italic, font_path.path,
                    font_path.index);
      if (!CheckGlyph(font_path.path, font_path.index, font_set.second)) {
        return false;
      }
    }
    for (const char32_t& wch : font_set.second) {
      codepoint_set.insert(static_cast<uint32_t>(wch));
    }
    codepoint_set.insert(ADDITIONAL_CODEPOINTS.begin(),
                         ADDITIONAL_CODEPOINTS.end());
    if (subset_font_codepoint_sets_.find(font_path) ==
        subset_font_codepoint_sets_.end()) {
      subset_font_codepoint_sets_[font_path] = codepoint_set;
    } else {
      subset_font_codepoint_sets_[font_path].insert(codepoint_set.begin(),
                                                    codepoint_set.end());
    }
  }
  if (have_missing) {
    logger_->error(_ST("Found missing fonts. Check warning info above."));
    return false;
  }
  return true;
}

bool FontSubsetter::CreateSubfont(
    const std::pair<FontPath, std::set<uint32_t>>& subset_font) {
  fs::path input_filepath(subset_font.first.path);
  fs::path output_filepath(
      subfont_dir_ + fs::path::preferred_separator +
      input_filepath.stem().native() + _ST("[") +
      ToAString(subset_font.first.index) + _ST("]_subset") +
      ((ToLower(input_filepath.extension().native()) == _ST(".otf") ||
        ToLower(input_filepath.extension().native()) == _ST(".otc"))
           ? _ST(".otf")
           : _ST(".ttf")));
  std::ifstream is(subset_font.first.path, std::ios::binary);
  const auto font_size = fs::file_size(subset_font.first.path);
  std::string font_data(font_size, '\0');
  is.read(&font_data[0], font_size);
  hb_blob_t* hb_blob = hb_blob_create_or_fail(
      &font_data[0], static_cast<unsigned int>(font_size),
      HB_MEMORY_MODE_READONLY, NULL, NULL);
  hb_face_t* hb_face = hb_face_create(hb_blob, subset_font.first.index);
  hb_set_t* codepoint_set = hb_set_create();
  for (const auto& codepoint : subset_font.second) {
    hb_set_add(codepoint_set, codepoint);
  }
  hb_subset_input_t* input = hb_subset_input_create_or_fail();
  hb_set_t* input_codepoints =
      hb_subset_input_set(input, HB_SUBSET_SETS_UNICODE);
  hb_set_t* input_namelangid =
      hb_subset_input_set(input, HB_SUBSET_SETS_NAME_LANG_ID);
  hb_set_clear(input_namelangid);
  hb_set_invert(input_namelangid);
  hb_set_union(input_codepoints, codepoint_set);
  hb_face_t* subset_face = hb_subset_or_fail(hb_face, input);
  if (subset_face == nullptr) {
    hb_blob_destroy(hb_blob);
    hb_face_destroy(subset_face);
    hb_face_destroy(hb_face);
    hb_set_destroy(codepoint_set);
    hb_subset_input_destroy(input);
    return false;
  }
  hb_blob_t* subset_blob = hb_face_reference_blob(subset_face);
  unsigned int len = 0;
  const char* subset_data = hb_blob_get_data(subset_blob, &len);
  std::ofstream subset_file(output_filepath.native(), std::ios::binary);
  subset_file.write(subset_data, len);
  hb_blob_destroy(subset_blob);
  hb_blob_destroy(hb_blob);
  hb_face_destroy(subset_face);
  hb_face_destroy(hb_face);
  hb_set_destroy(codepoint_set);
  hb_subset_input_destroy(input);
  if (len == 0) {
    return false;
  }
  subfonts_path_.emplace_back(output_filepath.native());
  return true;
}

bool FontSubsetter::CheckGlyph(const AString& font_path, const long& font_index,
                               const std::set<char32_t>& codepoint_set) {
  std::vector<uint32_t> missing_codepoints;
  FT_Face ft_face;
  std::ifstream is(font_path, std::ios::binary);
  if (!is.is_open()) {
    logger_->error(_ST("\"{}\" is inaccessible."), font_path);
    return false;
  }
  const auto font_size = fs::file_size(font_path);
  std::string font_data(font_size, '\0');
  is.read(&font_data[0], font_size);
  FT_New_Memory_Face(ft_library_, reinterpret_cast<FT_Byte*>(&font_data[0]),
                     static_cast<FT_Long>(font_size), font_index, &ft_face);
  for (const auto& codepoint : codepoint_set) {
    if (!codepoint) {
      continue;
    }
    if (!FT_Get_Char_Index(ft_face, codepoint)) {
      missing_codepoints.emplace_back(codepoint);
      break;
    }
  }
  if (!missing_codepoints.empty()) {
    logger_->warn(_ST("Missing codepoints: {:#06x}"),
                  fmt::join(missing_codepoints, _ST("  ")));
  }
  FT_Done_Face(ft_face);
  return true;
}

}  // namespace ass
