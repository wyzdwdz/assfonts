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

#ifndef ASSFONTS_FONTSUBSETTER_H_
#define ASSFONTS_FONTSUBSETTER_H_

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SFNT_NAMES_H
#ifdef __cplusplus
}
#endif

#include <spdlog/spdlog.h>

#include "ass_parser.h"
#include "font_parser.h"

namespace ass {

class FontSubsetter {
 public:
  template <typename T>
  FontSubsetter(const AssParser& ap, const FontParser& fp,
                std::shared_ptr<T> sink)
      : ap_(ap), fp_(fp) {
    logger_ = std::make_shared<spdlog::logger>("font_subsetter", sink);
    spdlog::register_logger(logger_);
    FT_Init_FreeType(&ft_library_);
  };
  template <typename T>
  FontSubsetter(const AssParser& ap, const FontParser& fp,
                const std::string& subfont_dir, std::shared_ptr<T> sink)
      : FontSubsetter(sink, ap, fp) {
    SetSubfontDir(subfont_dir);
  };
  ~FontSubsetter() {
    FT_Done_FreeType(ft_library_);
    spdlog::drop("font_subsetter");
  }
  void SetSubfontDir(const std::string& subfont_dir);
  void Run(bool is_no_subset);

 private:
  struct FontPath {
    std::string path;
    long index = 0;

    bool operator<(const FontPath& s) const {
      return (path < s.path) || (path == s.path && index < s.index);
    }
  };
  FT_Library ft_library_;
  std::shared_ptr<spdlog::logger> logger_;
  const AssParser& ap_;
  const FontParser& fp_;
  std::string subfont_dir_;
  std::map<FontPath, std::set<uint32_t>> subset_font_codepoint_sets_;
  std::vector<std::string> subfonts_path_;

  bool FindFont(
      const std::pair<AssParser::FontDesc, std::set<char32_t>> font_set,
      const std::vector<FontParser::FontInfo>& font_list,
      std::string& found_path, long& found_index);
  void set_subset_font_codepoint_sets();
  bool CreateSubfont(
      const std::pair<FontPath, std::set<uint32_t>>& subset_font);
  bool CheckGlyph(std::string font_path, long font_index,
                  std::set<char32_t> codepoint_set);

  friend class AssFontEmbedder;
};

}  // namespace ass

#endif