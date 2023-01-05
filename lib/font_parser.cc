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

#include "font_parser.h"

#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>
#include <regex>
#include <thread>

#ifdef __cplusplus
extern "C" {
#endif
#include FT_MODULE_H
#include FT_TYPE1_TABLES_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_IDS_H
#include FT_TRUETYPE_TABLES_H
#ifdef __cplusplus
}
#endif

#include <nlohmann/json.hpp>

#include "ass_freetype.h"
#include "ass_threadpool.h"

namespace fs = std::filesystem;

namespace ass {

void FontParser::LoadFonts(const AString& fonts_dir) {
  fonts_path_ = FindFileInDir(fonts_dir, _ST(".+\\.(ttf|otf|ttc|otc)$"));
  logger_->info(_ST("Found {} font files in \"{}\""), fonts_path_.size(),
                fonts_dir);
  font_list_.reserve(fonts_path_.size());
  unsigned int num_thread = std::thread::hardware_concurrency() + 1;
  ThreadPool pool(num_thread);
  for (const AString& font_path : fonts_path_) {
    pool.LoadJob([&]() { GetFontInfo(font_path); });
  }
  pool.Join();
}

void FontParser::SaveDB(const AString& db_path) {
  if (font_list_.size() == 0) {
    logger_->warn(_ST("No font is found. Nothing to save."));
    return;
  }
  fs::path file_path(db_path);
  if (fs::is_regular_file(file_path)) {
    logger_->warn(_ST("\"{}\" already exists. It will be overwritten."),
                  file_path.native());
  }
  std::ofstream db_file(file_path.native());
  if (!db_file.is_open()) {
    logger_->warn(_ST("\"{}\" is inaccessible."), file_path.native());
    return;
  }
  nlohmann::ordered_json json;
  for (const FontInfo& font : font_list_) {
    nlohmann::ordered_json js_font;
    js_font["families"] = font.families;
    js_font["fullnames"] = font.fullnames;
    js_font["psnames"] = font.psnames;
    js_font["weight"] = font.weight;
    js_font["slant"] = font.slant;
#ifdef _WIN32
    js_font["path"] = WideToU8(font.path);
#else
    js_font["path"] = font.path;
#endif
    js_font["index"] = font.index;
    json.emplace_back(js_font);
  }
  db_file << json.dump(4);
  logger_->info(_ST("Fonts database has been saved in \"{}\""),
                file_path.native());
}

void FontParser::LoadDB(const AString& db_path) {
  fs::path file_path(db_path);
  std::ifstream db_file(file_path.native());
  if (!db_file.is_open()) {
    logger_->warn(_ST("Fonts database \"{}\" doesn't exists."),
                  file_path.native());
    return;
  }
  try {
    nlohmann::json json;
    db_file >> json;
    for (const nlohmann::json& js_font : json) {
      FontInfo font;
      font.families = js_font["families"];
      font.fullnames = js_font["fullnames"];
      font.psnames = js_font["psnames"];
      font.weight = js_font["weight"];
      font.slant = js_font["slant"];
#ifdef _WIN32
      font.path = U8ToWide(js_font["path"]);
#else
      font.path = js_font["path"];
#endif
      font.index = js_font["index"];
      font_list_in_db_.emplace_back(font);
    }
  } catch (const nlohmann::json::exception&) {
    logger_->warn(_ST("Cannot load fonts database: \"{}\""),
                  file_path.native());
    font_list_in_db_.clear();
    return;
  }
  logger_->info(_ST("Load fonts database \"{}\""), file_path.native());
}

void FontParser::clean_font_list() {
  font_list_.clear();
}

std::vector<AString> FontParser::FindFileInDir(const AString& dir,
                                               const AString& pattern) {
  std::vector<AString> res;
  const std::basic_regex<AChar> r(pattern, std::regex::icase);
  const fs::path dir_path(dir);
  const fs::recursive_directory_iterator iter(dir_path);
  try {
    for (const auto& dir_entry : iter) {
      if (std::regex_match(dir_entry.path().native(), r)) {
        res.emplace_back(dir_entry.path().native());
      }
    }
  } catch (const fs::filesystem_error& e) {
    logger_->warn("Failed in searching font files. Error code: {}", e.what());
    res.clear();
    return res;
  }
  return res;
}

void FontParser::GetFontInfo(const AString& font_path) {
  std::unique_lock<std::mutex> logger_lock(logger_mtx_, std::defer_lock);
  std::vector<std::string> families;
  std::vector<std::string> fullnames;
  std::vector<std::string> psnames;
  FontInfo font_info;
  FT_Library ft_library;
  FT_Init_FreeType(&ft_library);
  FT_StreamRec ft_stream = {};
  FT_Open_Args open_args = {};
  NewOpenArgs(font_path, ft_stream, open_args);
  FT_Face ft_face;
  if (FT_Open_Face(ft_library, &open_args, -1, &ft_face)) {
    logger_lock.lock();
    logger_->warn(_ST("\"{}\" cannot be opened."), font_path);
    logger_lock.unlock();
    return;
  }
  const long n_face = ft_face->num_faces;
  for (long face_idx = 0; face_idx < n_face; ++face_idx) {
    FT_Open_Face(ft_library, &open_args, face_idx, &ft_face);
    const unsigned int num_names = FT_Get_Sfnt_Name_Count(ft_face);
    for (unsigned int i = 0; i < num_names; i++) {
      FT_SfntName name;
      std::string wbuf;
      std::string buf;
      if (FT_Get_Sfnt_Name(ft_face, i, &name)) {
        continue;
      }
      if (name.name_id != TT_NAME_ID_FULL_NAME &&
          name.name_id != TT_NAME_ID_FONT_FAMILY &&
          name.name_id != TT_NAME_ID_PS_NAME) {
        continue;
      }
      if (name.platform_id != TT_PLATFORM_MICROSOFT) {
        continue;
      }
      wbuf = std::string(reinterpret_cast<char*>(name.string), name.string_len);
      if (!IconvConvert(wbuf, buf, "UTF-16BE", "UTF-8")) {
        continue;
      }
      if (buf.empty()) {
        continue;
      }
      switch (name.name_id) {
        case TT_NAME_ID_FONT_FAMILY:
          if (std::find(families.begin(), families.end(), buf) ==
              families.end()) {
            families.emplace_back(buf);
          }
          break;
        case TT_NAME_ID_FULL_NAME:
          if (std::find(fullnames.begin(), fullnames.end(), buf) ==
              fullnames.end()) {
            fullnames.emplace_back(buf);
          }
          break;
        case TT_NAME_ID_PS_NAME:
          if (std::find(psnames.begin(), psnames.end(), buf) == psnames.end()) {
            psnames.emplace_back(buf);
          }
          break;
        default:
          break;
      }
    }
    if (families.empty() && fullnames.empty() && psnames.empty()) {
      continue;
    }
    font_info.slant = 110 * !!(ft_face->style_flags & FT_STYLE_FLAG_ITALIC);
    font_info.weight = AssFaceGetWeight(ft_face);
    if (font_info.slant < 0 || font_info.slant > 110)
      font_info.slant = 0;
    if (font_info.weight < 100 || font_info.weight > 900)
      font_info.weight = 400;
    font_info.families = families;
    font_info.fullnames = fullnames;
    font_info.psnames = psnames;
    font_info.path = font_path;
    font_info.index = face_idx;
    std::lock_guard font_list_lock(font_list_mtx_);
    font_list_.emplace_back(font_info);
  }
  if (font_info.families.empty() && font_info.fullnames.empty() &&
      font_info.psnames.empty()) {
    logger_lock.lock();
    logger_->warn(_ST("\"{}\" has no parsable name."), font_path);
    logger_lock.unlock();
  }
  FT_Done_Face(ft_face);
  FT_Done_FreeType(ft_library);
}

int FontParser::AssFaceGetWeight(FT_Face face) {
  /*
 * Copyright (C) 2006 Evgeniy Stepanov <eugeni.stepanov@gmail.com>
 *
 * This file is part of libass.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#if FREETYPE_MAJOR > 2 || (FREETYPE_MAJOR == 2 && FREETYPE_MINOR >= 6)
  TT_OS2* os2 = (TT_OS2*)FT_Get_Sfnt_Table(face, FT_SFNT_OS2);
#else
  // This old name is still included (as a macro), but deprecated as of 2.6, so avoid using it if we can
  TT_OS2* os2 = (TT_OS2*)FT_Get_Sfnt_Table(face, ft_sfnt_os2);
#endif
  if (os2 && os2->version != 0xffff && os2->usWeightClass)
    return os2->usWeightClass;
  else
    return 300 * !!(face->style_flags & FT_STYLE_FLAG_BOLD) + 400;
}

}  // namespace ass