//   Copyright [2022] [https://github.com/wyzdwdz]
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

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
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "ass_parser.h"
#include "error_proxy_sink.h"
#include "font_parser.h"

namespace ass {

class FontSubsetter {
 public:
  FontSubsetter(const AssParser& ap, const FontParser& fp) : ap_(ap), fp_(fp) {
    auto color_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto error_sink =
        std::make_shared<mylog::sinks::error_proxy_sink_mt>(color_sink);
    logger_ = std::make_shared<spdlog::logger>("font_subsetter", error_sink);
    spdlog::register_logger(logger_);
    FT_Init_FreeType(&ft_library_);
  };
  FontSubsetter(const AssParser& ap, const FontParser& fp,
                const std::string& subfont_dir)
      : FontSubsetter(ap, fp) {
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
  bool FindFont(const std::string& font_name,
                const std::vector<FontParser::FontInfo>& font_list,
                std::string& found_path, long& found_index);
  void set_subset_font_codepoint_sets();
  bool FontSubsetter::CreateSubfont(
      const std::pair<FontPath, std::set<uint32_t>>& subset_font);

  friend class AssFontEmbedder;
};

}  // namespace ass

#endif