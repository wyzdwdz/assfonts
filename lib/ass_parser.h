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

#ifndef ASSFONTS_ASSPARSER_H_
#define ASSFONTS_ASSPARSER_H_

#include <fstream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "ass_logger.h"
#include "ass_string.h"

namespace ass {

class AssParser {
 public:
  AssParser(std::shared_ptr<Logger> logger) : logger_(logger){};

  AssParser(const AString& ass_file_path, std::shared_ptr<Logger> logger)
      : AssParser(logger) {
    ReadFile(ass_file_path);
  }

  ~AssParser() = default;

  void set_output_dir_path(const AString& output_dir_path);

  bool ReadFile(const AString& ass_file_path);

  bool get_has_fonts() const;

  std::vector<std::string> get_text() const;

  AString get_ass_path() const;

  bool Recolorize(const AString& ass_file_path, const unsigned int& brightness);

  void Clear();

 private:
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

  struct TextInfo {
    unsigned int line_num = 0;
    std::string text;
  };

  struct DialogueInfo {
    unsigned int line_num = 0;
    std::vector<std::string> dialogue;
  };

  std::shared_ptr<Logger> logger_;

  AString ass_path_;
  AString output_dir_path_;

  std::vector<TextInfo> text_;
  std::vector<std::vector<std::string>> styles_;
  std::vector<DialogueInfo> dialogues_;

  bool has_default_style_ = false;
  bool has_fonts_ = false;

  std::map<FontDesc, std::set<char32_t>> font_sets_;
  std::map<std::string, FontDesc> stylename_fontdesc_;

  void SkipFontsLines(std::istringstream& is, unsigned int& line_num);

  bool GetUTF8(const std::ifstream& is, std::string& res);

  bool FindTitle(const std::string& line, const std::string& title);

  bool ParseLine(const std::string& line, const unsigned int num_field,
                 std::vector<std::string>& res);

  bool ParseAss();
  bool GetStyles(std::vector<TextInfo>::iterator& line,
                 const std::vector<TextInfo>::iterator& end, bool& has_style);
  bool GetEvents(std::vector<TextInfo>::iterator& line,
                 const std::vector<TextInfo>::iterator& end, bool& has_event);

  void set_stylename_fontdesc();
  int CalculateBold(int value);
  int CalculateItalic(int value);

  void set_font_sets();
  FontDesc GetFontDescStyle(const DialogueInfo& dialogue);
  void GetCharacter(std::u32string::iterator& wch,
                    const std::u32string::iterator& end,
                    const FontDesc& font_desc_style, FontDesc& font_desc,
                    const unsigned int line_num);

  void StyleOverride(const std::u32string& code, FontDesc& font_desc,
                     const FontDesc& font_desc_style,
                     const unsigned int line_num);
  void ChangeFontname(const std::u32string& code, FontDesc& font_desc,
                      const FontDesc& font_desc_style);
  void ChangeBold(const std::u32string& code, FontDesc& font_desc,
                  const FontDesc& font_desc_style);
  void ChangeItalic(const std::u32string& code, FontDesc& font_desc,
                    const FontDesc& font_desc_style);
  void ChangeStyle(const std::u32string& code, FontDesc& font_desc,
                   const FontDesc& font_desc_style,
                   const unsigned int line_num);

  bool CleanFonts();

  friend class FontSubsetter;
};

}  // namespace ass

#endif
