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

#include <spdlog/async.h>
#include <spdlog/spdlog.h>

#include "ass_string.h"

namespace ass {

class AssParser {
 public:
  template <typename T>
  AssParser(std::shared_ptr<T> sink) {
    logger_ = std::make_shared<spdlog::async_logger>("ass_parser", sink,
                                                     spdlog::thread_pool());
    spdlog::register_logger(logger_);
  };
  template <typename T>
  AssParser(const AString& ass_file_path, std::shared_ptr<T> sink)
      : AssParser(sink) {
    ReadFile(ass_file_path);
  }
  ~AssParser() { spdlog::drop("ass_parser"); };

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
  std::shared_ptr<spdlog::logger> logger_;
  AString ass_path_;
  AString output_dir_path_;
  std::vector<std::string> text_;
  std::vector<std::vector<std::string>> styles_;
  bool has_default_style_ = false;
  bool has_fonts_ = false;
  std::vector<std::vector<std::string>> dialogues_;
  std::map<FontDesc, std::set<char32_t>> font_sets_;
  std::map<std::string, FontDesc> stylename_fontdesc_;

  bool GetUTF8(const std::ifstream& is, std::string& res);
  bool FindTitle(const std::string& line, const std::string& title);
  bool ParseLine(const std::string& line, const unsigned int num_field,
                 std::vector<std::string>& res);
  bool ParseAss();
  void set_stylename_fontdesc();
  bool set_font_sets();
  bool StyleOverride(const std::u32string& code, FontDesc* font_desc,
                     const FontDesc& font_desc_style);
  void CleanFonts();

  friend class FontSubsetter;
};

}  // namespace ass

#endif
