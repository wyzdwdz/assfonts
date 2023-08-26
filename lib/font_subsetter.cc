/*  This file is part of assfonts.
 *
 *  assfonts is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation,
 *  either version 3 of the License,
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
#include <fstream>
#include <random>

#include <fmt/format.h>

#define HB_EXPERIMENTAL_API
#include <harfbuzz/hb-subset.h>

#include <ghc/filesystem.hpp>

#include "ass_harfbuzz.h"
#include "assfonts.h"

static const std::u32string ADDITIONAL_CODEPOINTS = []() {
  std::u32string codepoints;
  for (char32_t ch = 0x0020; ch <= 0x007e; ++ch) {
    codepoints.push_back(ch);
  }
  for (char32_t ch = 0xff01; ch <= 0xff5e; ++ch) {
    codepoints.push_back(ch);
  }
  return codepoints;
}();

namespace fs = ghc::filesystem;

namespace ass {

void FontSubsetter::SetSubfontDir(const AString& subfont_dir) {
  subfont_dir_ = subfont_dir;
}

bool FontSubsetter::Run(const bool is_no_subset, const bool is_rename) {
  if (is_no_subset) {
    bool have_missing = false;
    FontSubsetInfo subfont_info;
    for (const auto& font_set : ap_.font_sets_) {
#ifdef _WIN32
      AString fontname = U8ToWide(font_set.first.fontname);
#else
      AString fontname(font_set.first.fontname);
#endif
      if (!FindFont(font_set, fp_.font_list_, subfont_info.font_path.path,
                    subfont_info.font_path.index) &&
          !FindFont(font_set, fp_.font_list_in_db_, subfont_info.font_path.path,
                    subfont_info.font_path.index)) {
        logger_->Warn(_ST("Missing the font: \"{}\" ({},{})"), fontname,
                      font_set.first.bold, font_set.first.italic);
        have_missing = true;
      } else {
        logger_->Info(_ST("Found font: \"{}\" ({},{}) --> \"{}\"[{}]"),
                      fontname, font_set.first.bold, font_set.first.italic,
                      subfont_info.font_path.path,
                      subfont_info.font_path.index);
        CheckGlyph(subfont_info.font_path.path, subfont_info.font_path.index,
                   font_set.second, fontname, font_set.first.bold,
                   font_set.first.italic);
      }
      subfont_info.fonts_desc.emplace_back(font_set.first);
      subfont_info.subfont_path = subfont_info.font_path.path;
      subfonts_info_.emplace_back(subfont_info);
    }
    if (have_missing) {
      logger_->Error(_ST("Found missing fonts. Check warning info above."));
      return false;
    }
    return true;
  }
  if (!set_subfonts_info()) {
    return false;
  }
  if (is_rename) {
    SetNewname();
  }
  fs::path dir_path(subfont_dir_);
  if (!fs::exists(dir_path)) {
    logger_->Info(_ST("Create subset fonts directory: \"{}\""),
                  dir_path.native());
    try {
      fs::create_directory(dir_path);
    } catch (const fs::filesystem_error& e) {
      logger_->Error("Create subset fonts directory failed. Error code: {}",
                     e.what());
      return false;
    }
  }
  for (auto& subset_font : subfonts_info_) {
    if (!CreateSubfont(subset_font, is_rename)) {
      logger_->Error(_ST("Subset failed: \"{}\"[{}]"),
                     subset_font.font_path.path, subset_font.font_path.index);
      return false;
    }
  }
  return true;
}

void FontSubsetter::Clear() {
  subfont_dir_.clear();
  subfonts_info_.clear();
}

bool FontSubsetter::FindFont(
    const std::pair<AssParser::FontDesc, std::unordered_set<char32_t>>&
        font_set,
    const std::unordered_multimap<AString, FontParser::FontInfo>& font_list,
    AString& found_path, long& found_index) {
  auto fontname = ToLower(font_set.first.fontname);
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
    if (std::find(font.second.families.begin(), font.second.families.end(),
                  fontname) != font.second.families.end()) {
      if (ToLower(font.first.substr(font.first.size() - 4, 4)) == _ST(".otf") ||
          ToLower(font.first.substr(font.first.size() - 4, 4)) == _ST(".otc")) {
        continue;
      }
      score = 0;
      score += std::abs(font_set.first.bold - font.second.weight);
      score += std::abs(font_set.first.italic - font.second.slant);
    } else if ((std::find(font.second.fullnames.begin(),
                          font.second.fullnames.end(),
                          fontname) != font.second.fullnames.end()) ||
               (std::find(font.second.psnames.begin(),
                          font.second.psnames.end(),
                          fontname) != font.second.psnames.end())) {
      if (ToLower(font.first.substr(font.first.size() - 4, 4)) == _ST(".otf") ||
          ToLower(font.first.substr(font.first.size() - 4, 4)) == _ST(".otc")) {
        continue;
      }
      score = 0;
    }
    if (score < min_score) {
      min_score = score;
      tmp_path = font.first;
      tmp_index = font.second.index;
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
    if (std::find(font.second.families.begin(), font.second.families.end(),
                  fontname) != font.second.families.end()) {
      if (ToLower(font.first.substr(font.first.size() - 4, 4)) == _ST(".ttf") ||
          ToLower(font.first.substr(font.first.size() - 4, 4)) == _ST(".ttc")) {
        continue;
      }
      score = 0;
      score += std::abs(font_set.first.bold - font.second.weight);
      score += std::abs(font_set.first.italic - font.second.slant);
    } else if ((std::find(font.second.fullnames.begin(),
                          font.second.fullnames.end(),
                          fontname) != font.second.fullnames.end()) ||
               (std::find(font.second.psnames.begin(),
                          font.second.psnames.end(),
                          fontname) != font.second.psnames.end())) {
      if (ToLower(font.first.substr(font.first.size() - 4, 4)) == _ST(".ttf") ||
          ToLower(font.first.substr(font.first.size() - 4, 4)) == _ST(".ttc")) {
        continue;
      }
      score = 0;
    }
    if (score < min_score) {
      min_score = score;
      tmp_path = font.first;
      tmp_index = font.second.index;
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

bool FontSubsetter::set_subfonts_info() {
  bool have_missing = false;
  for (const auto& font_set : ap_.font_sets_) {
    FontPath font_path;
    std::unordered_set<uint32_t> codepoint_set;
#ifdef _WIN32
    AString fontname = U8ToWide(font_set.first.fontname);
#else
    AString fontname(font_set.first.fontname);
#endif
    if (!FindFont(font_set, fp_.font_list_, font_path.path, font_path.index) &&
        !FindFont(font_set, fp_.font_list_in_db_, font_path.path,
                  font_path.index)) {
      logger_->Warn(_ST("Missing the font: \"{}\" ({},{})"), fontname,
                    font_set.first.bold, font_set.first.italic);
      have_missing = true;
    } else {
      logger_->Info(_ST("Found font: \"{}\" ({},{}) --> \"{}\"[{}]"), fontname,
                    font_set.first.bold, font_set.first.italic, font_path.path,
                    font_path.index);
      if (!CheckGlyph(font_path.path, font_path.index, font_set.second,
                      fontname, font_set.first.bold, font_set.first.italic)) {
        return false;
      }
    }
    for (const char32_t& wch : font_set.second) {
      codepoint_set.insert(static_cast<uint32_t>(wch));
    }
    codepoint_set.insert(ADDITIONAL_CODEPOINTS.begin(),
                         ADDITIONAL_CODEPOINTS.end());
    auto subfonts_info_iter =
        std::find_if(subfonts_info_.begin(), subfonts_info_.end(),
                     [&](const FontSubsetInfo& f) -> bool {
                       return f.font_path == font_path;
                     });
    if (subfonts_info_iter == subfonts_info_.end()) {
      FontSubsetInfo subfont_info;
      subfont_info.fonts_desc.emplace_back(font_set.first);
      subfont_info.codepoints = codepoint_set;
      subfont_info.font_path = font_path;
      subfonts_info_.emplace_back(subfont_info);
    } else {
      (*subfonts_info_iter).fonts_desc.emplace_back(font_set.first);
      (*subfonts_info_iter)
          .codepoints.insert(codepoint_set.begin(), codepoint_set.end());
    }
  }
  if (have_missing) {
    logger_->Error(_ST("Found missing fonts. Check warning info above."));
    return false;
  }
  return true;
}

bool FontSubsetter::CreateSubfont(FontSubsetInfo& subset_font,
                                  const bool is_rename) {
  AString subfont_name_suffix;
  if (is_rename) {
#ifdef _WIN32
    subfont_name_suffix = U8ToWide(subset_font.newname);
#else
    subfont_name_suffix = subset_font.newname;
#endif
  } else {
    subfont_name_suffix = _ST("subset");
  }
  fs::path input_filepath(subset_font.font_path.path);
  fs::path output_filepath(
      subfont_dir_ + fs::path::preferred_separator +
      input_filepath.stem().native() + _ST("[") +
      ToAString(subset_font.font_path.index) + _ST("]_") + subfont_name_suffix +
      ((ToLower(input_filepath.extension().native()) == _ST(".otf") ||
        ToLower(input_filepath.extension().native()) == _ST(".otc"))
           ? _ST(".otf")
           : _ST(".ttf")));
  std::ifstream is(subset_font.font_path.path, std::ios::binary);
  const auto font_size = fs::file_size(subset_font.font_path.path);
  std::string font_data(font_size, '\0');
  is.read(&font_data[0], font_size);
  HbBlob hb_blob(hb_blob_create_or_fail(&font_data[0],
                                        static_cast<unsigned int>(font_size),
                                        HB_MEMORY_MODE_READONLY, NULL, NULL));
  HbFace hb_face(hb_face_create(hb_blob.get(), subset_font.font_path.index));
  HbSet codepoint_set(hb_set_create());
  for (const auto& codepoint : subset_font.codepoints) {
    hb_set_add(codepoint_set.get(), codepoint);
  }
  HbSubsetInput input(hb_subset_input_create_or_fail());
  hb_set_t* input_codepoints =
      hb_subset_input_set(input.get(), HB_SUBSET_SETS_UNICODE);
  hb_set_union(input_codepoints, codepoint_set.get());
  if (is_rename) {
    if (!hb_subset_input_override_name_table(
            input.get(), HB_OT_NAME_ID_FONT_FAMILY, 3, 1, 0x0409,
            subset_font.newname.c_str(), subset_font.newname.length())) {
      return false;
    }
    if (!hb_subset_input_override_name_table(
            input.get(), HB_OT_NAME_ID_FULL_NAME, 3, 1, 0x0409,
            subset_font.newname.c_str(), subset_font.newname.length())) {
      return false;
    }
    if (!hb_subset_input_override_name_table(
            input.get(), HB_OT_NAME_ID_UNIQUE_ID, 3, 1, 0x0409,
            subset_font.newname.c_str(), subset_font.newname.length())) {
      return false;
    }
    if (!hb_subset_input_override_name_table(
            input.get(), HB_OT_NAME_ID_COPYRIGHT, 3, 1, 0x0409,
            fmt::format("Processed by assfonts v{}.{}.{}",
                        ASSFONTS_VERSION_MAJOR, ASSFONTS_VERSION_MINOR,
                        ASSFONTS_VERSION_PATCH)
                .c_str(),
            -1)) {
      return false;
    }
  } else {
    hb_set_t* input_namelangid =
        hb_subset_input_set(input.get(), HB_SUBSET_SETS_NAME_LANG_ID);
    hb_set_clear(input_namelangid);
    hb_set_invert(input_namelangid);
  }
  HbFace subset_face(hb_subset_or_fail(hb_face.get(), input.get()));
  if (subset_face.get() == nullptr) {
    return false;
  }
  HbBlob subset_blob(hb_face_reference_blob(subset_face.get()));
  unsigned int len = 0;
  const char* subset_data = hb_blob_get_data(subset_blob.get(), &len);
  std::ofstream subset_file(output_filepath.native(), std::ios::binary);
  if (!subset_file.is_open()) {
    return false;
  }
  subset_file.write(subset_data, len);
  if (len == 0) {
    return false;
  }
  subset_font.subfont_path = output_filepath.native();
  return true;
}

bool FontSubsetter::CheckGlyph(
    const AString& font_path, const long& font_index,
    const std::unordered_set<char32_t>& codepoint_set, const AString& fontname,
    int bold, int italic) {
  std::vector<uint32_t> missing_codepoints;
  FT_Face ft_face;
  std::ifstream is(font_path, std::ios::binary);
  if (!is.is_open()) {
    logger_->Error(_ST("\"{}\" is inaccessible."), font_path);
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
    }
  }
  if (!missing_codepoints.empty()) {
    logger_->Warn(_ST("Missing codepoints for \"{}\" ({},{}): {:#06x}"),
                  fontname, bold, italic,
                  fmt::join(missing_codepoints, _ST("  ")));
  }
  FT_Done_Face(ft_face);
  return true;
}

bool FontSubsetter::LowerCmp(const std::string& a, const std::string& b) {
  return (ToLower(a) == ToLower(b));
}

void FontSubsetter::SetNewname() {
  for (auto subfonts_info_iter = subfonts_info_.begin();
       subfonts_info_iter != subfonts_info_.end(); ++subfonts_info_iter) {
    if ((*subfonts_info_iter).newname.empty()) {
      (*subfonts_info_iter).newname = RandomName(8);
    } else {
      continue;
    }
    for (const auto& font_desc : (*subfonts_info_iter).fonts_desc) {
      for (auto iter_tmp = subfonts_info_iter; iter_tmp != subfonts_info_.end();
           ++iter_tmp) {
        if (!(*iter_tmp).newname.empty()) {
          continue;
        }
        for (const auto& font_desc_tmp : (*iter_tmp).fonts_desc) {
          if (font_desc_tmp.fontname == font_desc.fontname) {
            (*iter_tmp).newname = (*subfonts_info_iter).newname;
            break;
          }
        }
      }
    }
  }
}

std::string FontSubsetter::RandomName(const int len) {
  auto e = std::mt19937{};
  const auto hes = std::random_device{}();
  e.seed(hes);
  std::string random_name;
  for (int i = 0; i < len; ++i) {
    auto char_distr = std::uniform_int_distribution<int>{65, 90};
    const char ch = static_cast<char>(char_distr(e));
    random_name.push_back(ch);
  }
  return random_name;
}

}  // namespace ass
