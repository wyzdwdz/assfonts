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

#ifndef ASSFONTS_FONTSUBSETTER_H_
#define ASSFONTS_FONTSUBSETTER_H_

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
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

#include "ass_logger.h"
#include "ass_parser.h"
#include "ass_string.h"
#include "font_parser.h"

namespace ass {

class FontSubsetter {
 public:
  FontSubsetter(const FontParser& fp,
                const std::map<AssParser::FontDesc,
                               std::unordered_set<char32_t>>& font_sets,
                std::shared_ptr<Logger> logger)
      : fp_(fp), font_sets_(font_sets), logger_(logger) {
    FT_Init_FreeType(&ft_library_);
  };

  FontSubsetter(const FontParser& fp,
                const std::map<AssParser::FontDesc,
                               std::unordered_set<char32_t>>& font_sets,
                const AString& subfont_dir, std::shared_ptr<Logger> logger)
      : FontSubsetter(fp, font_sets, logger) {
    SetSubfontDir(subfont_dir);
  };

  ~FontSubsetter() { FT_Done_FreeType(ft_library_); }

  struct FontPath {
    AString path;
    long index = 0;

    bool operator<(const FontPath& s) const {
      return (path < s.path) || (path == s.path && index < s.index);
    }

    bool operator==(const FontPath& s) const {
      return (path == s.path) && (index == s.index);
    }
  };

  struct FontSubsetInfo {
    std::vector<AssParser::FontDesc> fonts_desc;
    std::unordered_set<uint32_t> codepoints;
    FontPath font_path;
    std::string newname;
    AString subfont_path;
  };

  FontSubsetter(const FontSubsetter&) = delete;
  FontSubsetter& operator=(const FontSubsetter&) = delete;

  void SetSubfontDir(const AString& subfont_dir);

  bool Run(const bool is_no_subset, const bool is_rename = false);

  std::vector<FontSubsetInfo> get_subfonts_info() const;

  void Clear();

 private:
  FT_Library ft_library_;
  const FontParser& fp_;
  std::map<AssParser::FontDesc, std::unordered_set<char32_t>> font_sets_;
  std::shared_ptr<Logger> logger_;
  AString subfont_dir_;
  std::vector<FontSubsetInfo> subfonts_info_;

  bool FindFont(
      const std::pair<AssParser::FontDesc, std::unordered_set<char32_t>>&
          font_set,
      const std::unordered_multimap<AString, FontParser::FontInfo>& font_list,
      AString& found_path, long& found_index);

  bool set_subfonts_info();

  bool CreateSubfont(FontSubsetInfo& subset_font, const bool is_rename);

  bool CheckGlyph(const AString& font_path, const long& font_index,
                  const std::unordered_set<char32_t>& codepoint_set,
                  const AString& fontname, int bold, int italic);
  bool LowerCmp(const std::string& a, const std::string& b);

  void SetNewname();

  std::string RandomName(const int len);
};

}  // namespace ass

#endif