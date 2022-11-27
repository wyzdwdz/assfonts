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

#ifndef ASSFONTS_FONTPARSER_H_
#define ASSFONTS_FONTPARSER_H_

#include <memory>
#include <string>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif
#include <ft2build.h>
#include FT_FREETYPE_H
#ifdef __cplusplus
}
#endif
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "error_proxy_sink.h"

namespace ass {

class FontParser {
 public:
  FontParser() {
    auto color_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto error_sink =
        std::make_shared<mylog::sinks::error_proxy_sink_mt>(color_sink);
    logger_ = std::make_shared<spdlog::logger>("font_parser", error_sink);
    spdlog::register_logger(logger_);
    FT_Init_FreeType(&ft_library_);
  };
  FontParser(const std::string& fonts_dir) : FontParser() {
    LoadFonts(fonts_dir);
  };
  ~FontParser() {
    FT_Done_FreeType(ft_library_);
    spdlog::drop("font_parser");
  };
  void LoadFonts(const std::string& fonts_dir);
  void SaveDB(const std::string& db_path);
  void LoadDB(const std::string& db_path);
  void clean_font_list();

 private:
  struct FontInfo {
    std::vector<std::string> families;
    std::vector<std::string> fullnames;
    std::vector<std::string> psnames;
    int weight = 400;
    int slant = 0;
    std::string path;
    long index = 0;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar& families;
      ar& fullnames;
      ar& psnames;
      ar& path;
      ar& index;
    }
  };

  FT_Library ft_library_;
  std::shared_ptr<spdlog::logger> logger_;
  std::vector<FontInfo> font_list_;
  std::vector<FontInfo> font_list_in_db_;
  std::vector<std::string> fonts_path_;

  std::vector<std::string> FindFileInDir(const std::string& dir,
                                         const std::string& pattern);
  bool GetFontInfo(const std::string& font_path);
  int FontParser::AssFaceGetWeight(FT_Face face);

  friend class FontSubsetter;
};

}  // namespace ass

#endif