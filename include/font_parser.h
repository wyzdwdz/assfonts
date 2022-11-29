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
#include <mutex>
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

#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "error_proxy_sink.h"

namespace ass {

class FontParser {
 public:
  template <typename T>
  FontParser(std::shared_ptr<T> sink) {
    logger_ = std::make_shared<spdlog::async_logger>("font_parser", sink,
                                                     spdlog::thread_pool());
    spdlog::register_logger(logger_);
  };
  template <typename T>
  FontParser(const std::string& fonts_dir, std::shared_ptr<T> sink)
      : FontParser(sink) {
    LoadFonts(fonts_dir);
  };
  ~FontParser() { spdlog::drop("font_parser"); };
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
      ar& weight;
      ar& slant;
      ar& path;
      ar& index;
    }
  };

  std::shared_ptr<spdlog::logger> logger_;
  std::mutex mtx_;
  std::vector<FontInfo> font_list_;
  std::vector<FontInfo> font_list_in_db_;
  std::vector<std::string> fonts_path_;

  std::vector<std::string> FindFileInDir(const std::string& dir,
                                         const std::string& pattern);
  void GetFontInfo(const std::string& font_path);
  int AssFaceGetWeight(FT_Face face);

  friend class FontSubsetter;
};

}  // namespace ass

#endif