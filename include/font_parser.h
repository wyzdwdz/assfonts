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
#include <unordered_map>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#ifdef __cplusplus
}
#endif

#include <spdlog/async.h>
#include <spdlog/spdlog.h>

#include "ass_string.h"

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
    long index = 0;
    std::string last_write_time;
  };

  std::shared_ptr<spdlog::async_logger> logger_;
  std::unordered_multimap<AString, FontInfo> font_list_;
  std::unordered_multimap<AString, FontInfo> font_list_in_db_;
  std::vector<AString> fonts_path_;

  std::vector<AString> FindFileInDir(const AString& dir,
                                     const AString& pattern);

  std::unordered_multimap<AString, FontInfo> GetFontInfo(
      const AString& font_path);
  std::unordered_multimap<AString, FontInfo> GetFontInfoFromDB(
      const AString& font_path, const std::string& last_write_time,
      const std::vector<std::unordered_multimap<AString, FontInfo>::iterator>&
          iters_found);
  bool OpenFontFace(FT_Library& ft_library, const FT_Open_Args& open_args,
                    FT_Face& ft_face, const AString& font_path);
  void GetFontInfoFromFace(
      FT_Library& ft_library, FT_Face& ft_face, const FT_Open_Args& open_args,
      const long face_idx,
      std::unordered_multimap<AString, FontInfo>& font_list,
      const AString& font_path, const std::string& last_write_time);
  void ParseFontName(const FT_Face& ft_face, const unsigned int name_idx,
                     std::vector<std::string>& families,
                     std::vector<std::string>& fullnames,
                     std::vector<std::string>& psnames);
  int AssFaceGetWeight(const FT_Face& face);
  bool ExistInDB(
      const AString& font_path, std::string& last_write_time,
      std::vector<std::unordered_multimap<AString, FontInfo>::iterator>&
          iters_found);
  std::string GetLastWriteTime(const AString& font_path);

  friend class FontSubsetter;
};

}  // namespace ass

#endif