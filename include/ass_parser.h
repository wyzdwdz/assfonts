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

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "error_proxy_sink.h"

namespace ass {

class AssParser {
 public:
  AssParser() {
    auto color_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto error_sink =
        std::make_shared<mylog::sinks::error_proxy_sink_mt>(color_sink);
    logger_ = std::make_shared<spdlog::logger>("ass_parser", error_sink);
    spdlog::register_logger(logger_);
  };
  AssParser(const std::string& ass_file_path) : AssParser() {
    ReadFile(ass_file_path);
  }
  ~AssParser() { spdlog::drop("ass_parser"); };

  void ReadFile(const std::string& ass_file_path);

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
  std::string ass_path_;
  std::vector<std::string> text_;
  std::vector<std::vector<std::string>> styles_;
  std::vector<std::vector<std::string>> dialogues_;
  std::map<FontDesc, std::set<char32_t>> font_sets_;

  bool IsUTF8(const std::string& line);
  bool FindTitle(const std::string& line, const std::string& title);
  std::vector<std::string> ParseLine(const std::string& line,
                                     const unsigned int num_field);
  void ParseAss();
  void set_font_sets();
  void StyleOverride(const std::u32string& code,
                     FontDesc* font_desc, const FontDesc& font_desc_style);

  friend class FontSubsetter;
};

}  // namespace ass

#endif