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
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#ifdef __cplusplus
}
#endif

#include <spdlog/spdlog.h>

#include "ass_string.h"

namespace ass {

class FontParser {
 public:
  template <typename T>
  FontParser(std::shared_ptr<T> sink) {
    logger_ = std::make_shared<spdlog::logger>("font_parser", sink);
    spdlog::register_logger(logger_);
  };
  template <typename T>
  FontParser(const std::string& fonts_dir, std::shared_ptr<T> sink)
      : FontParser(sink) {
    LoadFonts(fonts_dir);
  };
  ~FontParser() { spdlog::drop("font_parser"); };

  void LoadFonts(const AString& fonts_dir);
  void SaveDB(const AString& db_path);
  void LoadDB(const AString& db_path);
  void clean_font_list();

 private:
  struct FontInfo {
    std::vector<std::string> families;
    std::vector<std::string> fullnames;
    std::vector<std::string> psnames;
    int weight = 400;
    int slant = 0;
    AString path;
    long index = 0;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
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
  std::mutex font_list_mtx_;
  std::mutex logger_mtx_;
  std::vector<FontInfo> font_list_;
  std::vector<FontInfo> font_list_in_db_;
  std::vector<AString> fonts_path_;

  std::vector<AString> FindFileInDir(const AString& dir,
                                     const AString& pattern);
  void GetFontInfo(const AString& font_path);
  int AssFaceGetWeight(FT_Face face);

  friend class FontSubsetter;
};

}  // namespace ass

#endif