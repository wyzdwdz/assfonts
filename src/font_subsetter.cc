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
#include <fstream>
#include <locale>

#include <fmt/core.h>
#include <harfbuzz/hb-subset.h>
#include <harfbuzz/hb.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace fs = boost::filesystem;

namespace ass {

void FontSubsetter::SetSubfontDir(const std::string& subfont_dir) {
  fs::path dir_path(subfont_dir);
  if (!fs::exists(dir_path)) {
    logger_->info("Create subset fonts directory: \"{}\"",
                  dir_path.generic_string());
    fs::create_directory(dir_path);
  }
  subfont_dir_ = subfont_dir;
}

void FontSubsetter::Run(bool is_no_subset) {
  if (is_no_subset) {
    bool have_missing = false;
    std::string path;
    long index = 0;
    for (const auto& font_set : ap_.font_sets_) {
#ifdef _WIN32
      std::wstring w_fontname;
      int w_len =
          MultiByteToWideChar(CP_UTF8, 0, font_set.first.fontname.c_str(),
                              font_set.first.fontname.size(), NULL, 0);
      w_fontname.resize(w_len);
      MultiByteToWideChar(CP_UTF8, 0, font_set.first.fontname.c_str(),
                          font_set.first.fontname.size(), &w_fontname[0],
                          w_fontname.size());
      std::string fontname;
      int len = WideCharToMultiByte(CP_ACP, 0, w_fontname.c_str(),
                                    w_fontname.size(), NULL, 0, NULL, NULL);
      fontname.resize(len);
      WideCharToMultiByte(CP_ACP, 0, w_fontname.c_str(), w_fontname.size(),
                          &fontname[0], fontname.size(), NULL, NULL);
#elif
      std::string fontname(font_set.first);
#endif
      if (!FindFont(font_set, fp_.font_list_, path, index) &&
          !FindFont(font_set, fp_.font_list_in_db_, path, index)) {
        logger_->warn("Missing the font: \"{}\" ({},{})", fontname,
                      font_set.first.bold, font_set.first.italic);
        have_missing = true;
      } else {
        logger_->info("Found font: \"{}\" ({},{}) --> \"{}\"[{}]", fontname,
                      font_set.first.bold, font_set.first.italic, path, index);
        CheckGlyph(path, index, font_set.second);
      }
      subfonts_path_.push_back(path);
    }
    if (have_missing) {
      logger_->error("Found missing fonts. Check warning info above.");
    }
    return;
  }
  set_subset_font_codepoint_sets();
  for (const auto& subset_font : subset_font_codepoint_sets_) {
    if (!CreateSubfont(subset_font)) {
      logger_->error("Subset failed: \"{}\"[{}]", subset_font.first.path,
                     subset_font.first.index);
    }
  }
}

bool FontSubsetter::FindFont(
    const std::pair<AssParser::FontDesc, std::set<char32_t>> font_set,
    const std::vector<FontParser::FontInfo>& font_list, std::string& found_path,
    long& found_index) {
  bool is_found = false;
  unsigned int min_score = UINT_MAX;
  unsigned int score = UINT_MAX;
  std::string tmp_path;
  long tmp_index = 0;
  unsigned int ttf_score = UINT_MAX;
  std::string ttf_path;
  long ttf_index = 0;
  unsigned int otf_score = UINT_MAX;
  std::string otf_path;
  long otf_index = 0;
  for (const auto& font : font_list) {
    if (std::find(font.families.begin(), font.families.end(),
                  font_set.first.fontname) != font.families.end()) {
      if (boost::algorithm::to_lower_copy(
              font.path.substr(font.path.size() - 4, 4)) == ".otf" ||
          boost::algorithm::to_lower_copy(
              font.path.substr(font.path.size() - 4, 4)) == ".otc") {
        continue;
      }
      score = 0;
      score += std::abs(font_set.first.bold - font.weight);
      score += std::abs(font_set.first.italic - font.slant);
    } else if ((std::find(font.fullnames.begin(), font.fullnames.end(),
                          font_set.first.fontname) != font.fullnames.end()) ||
               (std::find(font.psnames.begin(), font.psnames.end(),
                          font_set.first.fontname) != font.psnames.end())) {
      if (boost::algorithm::to_lower_copy(
              font.path.substr(font.path.size() - 4, 4)) == ".otf" ||
          boost::algorithm::to_lower_copy(
              font.path.substr(font.path.size() - 4, 4)) == ".otc") {
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
      if (boost::algorithm::to_lower_copy(
              font.path.substr(font.path.size() - 4, 4)) == ".ttf" ||
          boost::algorithm::to_lower_copy(
              font.path.substr(font.path.size() - 4, 4)) == ".ttc") {
        continue;
      }
      score = 0;
      score += std::abs(font_set.first.bold - font.weight);
      score += std::abs(font_set.first.italic - font.slant);
    } else if ((std::find(font.fullnames.begin(), font.fullnames.end(),
                          font_set.first.fontname) != font.fullnames.end()) ||
               (std::find(font.psnames.begin(), font.psnames.end(),
                          font_set.first.fontname) != font.psnames.end())) {
      if (boost::algorithm::to_lower_copy(
              font.path.substr(font.path.size() - 4, 4)) == ".ttf" ||
          boost::algorithm::to_lower_copy(
              font.path.substr(font.path.size() - 4, 4)) == ".ttc") {
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

void FontSubsetter::set_subset_font_codepoint_sets() {
  bool have_missing = false;
  for (const auto& font_set : ap_.font_sets_) {
    FontPath font_path;
    std::set<uint32_t> codepoint_set;
#ifdef _WIN32
    std::wstring w_fontname;
    int w_len = MultiByteToWideChar(CP_UTF8, 0, font_set.first.fontname.c_str(),
                                    font_set.first.fontname.size(), NULL, 0);
    w_fontname.resize(w_len);
    MultiByteToWideChar(CP_UTF8, 0, font_set.first.fontname.c_str(),
                        font_set.first.fontname.size(), &w_fontname[0],
                        w_fontname.size());
    std::string fontname;
    int len = WideCharToMultiByte(CP_ACP, 0, w_fontname.c_str(),
                                  w_fontname.size(), NULL, 0, NULL, NULL);
    fontname.resize(len);
    WideCharToMultiByte(CP_ACP, 0, w_fontname.c_str(), w_fontname.size(),
                        &fontname[0], fontname.size(), NULL, NULL);
#elif
    std::string fontname(font_set.first);
#endif
    if (!FindFont(font_set, fp_.font_list_, font_path.path, font_path.index) &&
        !FindFont(font_set, fp_.font_list_in_db_, font_path.path,
                  font_path.index)) {
      logger_->warn("Missing the font: \"{}\" ({},{})", fontname,
                    font_set.first.bold, font_set.first.italic);
      have_missing = true;
    } else {
      logger_->info("Found font: \"{}\" ({},{}) --> \"{}\"[{}]", fontname,
                    font_set.first.bold, font_set.first.italic, font_path.path,
                    font_path.index);
      CheckGlyph(font_path.path, font_path.index, font_set.second);
    }
    for (const char32_t& wch : font_set.second) {
      codepoint_set.insert(static_cast<uint32_t>(wch));
    }
    if (subset_font_codepoint_sets_.find(font_path) ==
        subset_font_codepoint_sets_.end()) {
      subset_font_codepoint_sets_[font_path] = codepoint_set;
    } else {
      subset_font_codepoint_sets_[font_path].insert(codepoint_set.begin(),
                                                    codepoint_set.end());
    }
  }
  if (have_missing) {
    logger_->error("Found missing fonts. Check warning info above.");
  }
}

bool FontSubsetter::CreateSubfont(
    const std::pair<FontPath, std::set<uint32_t>>& subset_font) {
  fs::path input_filepath(subset_font.first.path);
  fs::path output_filepath(
      subfont_dir_ + "/" + input_filepath.stem().generic_string() + "[" +
      std::to_string(subset_font.first.index) + "]_subset" +
      ((boost::algorithm ::to_lower_copy(
            input_filepath.extension().generic_string()) == ".otf" ||
        boost::algorithm ::to_lower_copy(
            input_filepath.extension().generic_string()) == ".otc")
           ? ".otf"
           : ".ttf"));
  hb_blob_t* hb_blob =
      hb_blob_create_from_file(input_filepath.generic_string().c_str());
  hb_face_t* hb_face = hb_face_create(hb_blob, subset_font.first.index);
  hb_font_t* hb_font = hb_font_create(hb_face);
  FT_Face ft_face;
  FT_New_Face(ft_library_, subset_font.first.path.c_str(),
              subset_font.first.index, &ft_face);
  const unsigned int num_names = FT_Get_Sfnt_Name_Count(ft_face);
  hb_set_t* langid_set = hb_set_create();
  for (unsigned int i = 0; i < num_names; i++) {
    FT_SfntName name;
    FT_Get_Sfnt_Name(ft_face, i, &name);
    hb_set_add(langid_set, static_cast<uint32_t>(name.language_id));
  }
  hb_set_t* codepoint_set = hb_set_create();
  for (const auto& codepoint : subset_font.second) {
    hb_set_add(codepoint_set, codepoint);
  }
  hb_subset_input_t* input = hb_subset_input_create_or_fail();
  hb_set_t* input_codepoints =
      hb_subset_input_set(input, HB_SUBSET_SETS_UNICODE);
  hb_set_t* input_langids =
      hb_subset_input_set(input, HB_SUBSET_SETS_NAME_LANG_ID);
  hb_set_union(input_codepoints, codepoint_set);
  hb_set_union(input_langids, langid_set);
  hb_face_t* subset_face = hb_subset_or_fail(hb_face, input);
  if (subset_face == nullptr) {
    FT_Done_Face(ft_face);
    hb_blob_destroy(hb_blob);
    hb_face_destroy(subset_face);
    hb_face_destroy(hb_face);
    hb_set_destroy(langid_set);
    hb_set_destroy(codepoint_set);
    hb_subset_input_destroy(input);
    return false;
  }
  hb_blob_t* subset_blob = hb_face_reference_blob(subset_face);
  unsigned int len = 0;
  const char* subset_data = hb_blob_get_data(subset_blob, &len);
  std::ofstream subset_file(output_filepath.generic_string(), std::ios::binary);
  subset_file.write(subset_data, len);
  subset_file.close();
  FT_Done_Face(ft_face);
  hb_blob_destroy(subset_blob);
  hb_blob_destroy(hb_blob);
  hb_face_destroy(subset_face);
  hb_face_destroy(hb_face);
  hb_set_destroy(langid_set);
  hb_set_destroy(codepoint_set);
  hb_subset_input_destroy(input);
  if (len == 0) {
    return false;
  }
  subfonts_path_.push_back(output_filepath.generic_string());
  return true;
}

bool FontSubsetter::CheckGlyph(std::string font_path, long font_index,
                               std::set<char32_t> codepoint_set) {
  bool have_all_glyph = true;
  FT_Face ft_face;
  std::vector<uint32_t> missing_codepoints;
  FT_New_Face(ft_library_, font_path.c_str(), font_index, &ft_face);
  for (const auto& codepoint : codepoint_set) {
    if (!codepoint) {
      continue;
    }
    if (!FT_Get_Char_Index(ft_face, codepoint)) {
      missing_codepoints.push_back(codepoint);
      have_all_glyph = false;
      break;
    }
  }
  if (!missing_codepoints.empty()) {
    logger_->warn("Missing codepoints: {:#06x}",
                  fmt::join(missing_codepoints, "  "));
  }
  FT_Done_Face(ft_face);
  return have_all_glyph;
}

}  // namespace ass