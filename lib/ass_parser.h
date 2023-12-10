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

#ifndef ASSFONTS_ASSPARSER_H_
#define ASSFONTS_ASSPARSER_H_

#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include <nonstd/string_view.hpp>

#include "ass_logger.h"
#include "ass_string.h"
#include "ass_utf8.h"

namespace ass {

class AssParser {
 public:
  AssParser(std::shared_ptr<Logger> logger) : logger_(logger){};

  AssParser(const AString& ass_file_path, std::shared_ptr<Logger> logger)
      : AssParser(logger) {
    ReadFile(ass_file_path);
  }

  ~AssParser() = default;

  AssParser(const AssParser&) = delete;
  AssParser& operator=(const AssParser&) = delete;

  struct TextInfo {
    unsigned int line_num;
    std::string text;
  };

  struct RenameInfo {
    unsigned int line_num;
    size_t beg;
    size_t end;
    std::string fontname;
    std::string newname;
  };

  void set_output_dir_path(const AString& output_dir_path);

  bool ReadFile(const AString& ass_file_path);

  bool get_has_fonts() const;

  std::vector<TextInfo> get_text() const;

  AString get_ass_path() const;

  std::vector<RenameInfo> get_rename_infos() const;

  bool Recolorize(const AString& ass_file_path, const unsigned int brightness);

  void Clear();

 private:
  using Iterator = U8Iterator<nonstd::string_view>;

  struct FontDesc {
    std::string fontname;
    int bold = 400;
    int italic = 0;

    bool operator<(const FontDesc& r) const {
      if (fontname < r.fontname) {
        return true;
      } else if (fontname == r.fontname) {
        if (bold < r.bold) {
          return true;
        } else if (bold == r.bold) {
          if (italic < r.italic) {
            return true;
          }
        }
      }
      return false;
    }
  };

  struct StyleInfo {
    unsigned int line_num;
    const char* line_beg;
    std::vector<nonstd::string_view> style;
  };

  struct DialogueInfo {
    unsigned int line_num;
    const char* line_beg;
    std::vector<nonstd::string_view> dialogue;
  };

  std::shared_ptr<Logger> logger_;

  AString ass_path_;
  AString output_dir_path_;

  std::vector<TextInfo> text_;
  std::vector<StyleInfo> styles_;
  std::vector<DialogueInfo> dialogues_;

  bool has_default_style_ = false;
  bool has_fonts_ = false;

  std::map<FontDesc, std::unordered_set<char32_t>> font_sets_;
  std::map<std::string, FontDesc> stylename_fontdesc_;

  std::vector<RenameInfo> rename_infos_;

  void SkipFontsLines(std::istringstream& is, unsigned int line_num);

  bool GetUTF8(const std::ifstream& is, std::string& res);

  bool FindTitle(const std::string& line, const std::string& title);

  bool ParseLine(const std::string& line, const unsigned int num_field,
                 std::vector<nonstd::string_view>& res);

  bool ParseAss();
  bool GetStyles(std::vector<TextInfo>::iterator& line,
                 const std::vector<TextInfo>::iterator end, bool& has_style);
  bool GetEvents(std::vector<TextInfo>::iterator& line,
                 const std::vector<TextInfo>::iterator end, bool& has_event);

  void set_stylename_fontdesc();
  int CalculateBold(int value);
  int CalculateItalic(int value);

  void set_font_sets();
  FontDesc GetFontDescStyle(const DialogueInfo& dialogue);
  void GetCharacter(Iterator& wch, const Iterator end,
                    const FontDesc& font_desc_style, FontDesc& font_desc,
                    const unsigned int line_num, const char* line_beg);

  void StyleOverride(const nonstd::string_view code, FontDesc& font_desc,
                     const FontDesc& font_desc_style,
                     const unsigned int line_num, const char* line_beg);
  void ChangeFontname(const nonstd::string_view code, FontDesc& font_desc,
                      const FontDesc& font_desc_style,
                      const unsigned int line_num, const char* line_beg);
  void ChangeBold(const nonstd::string_view code, FontDesc& font_desc,
                  const FontDesc& font_desc_style);
  void ChangeItalic(const nonstd::string_view code, FontDesc& font_desc,
                    const FontDesc& font_desc_style);
  void ChangeStyle(const nonstd::string_view code, FontDesc& font_desc,
                   const FontDesc& font_desc_style,
                   const unsigned int line_num);

  bool CleanFonts();

  friend class FontSubsetter;
};

}  // namespace ass

#endif
