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
#include <future>
#include <regex>
#include <sstream>
#include <thread>

#ifdef __cplusplus
extern "C" {
#endif
#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/stat.h>
#endif

#include FT_MODULE_H
#include FT_TYPE1_TABLES_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_IDS_H
#include FT_TRUETYPE_TABLES_H
#ifdef __cplusplus
}
#endif

#include <ThreadPool/ThreadPool.h>
#include <nlohmann/json.hpp>

#include "ass_freetype.h"

#ifdef _WIN32
constexpr int MAX_TCHAR = 128;
#endif

namespace fs = std::filesystem;

namespace ass {

void FontParser::LoadFonts(const AString& fonts_dir) {
  fonts_path_ = FindFileInDir(fonts_dir, _ST(".+\\.(ttf|otf|ttc|otc)$"));
  logger_->info(_ST("Found {} font files in \"{}\". Parsing font files."),
                fonts_path_.size(), fonts_dir);
  font_list_.reserve(fonts_path_.size());

  ThreadPool pool(std::thread::hardware_concurrency() + 1);
  std::vector<std::future<std::unordered_multimap<AString, FontInfo>>> results;

  for (const AString& font_path : fonts_path_) {
    results.emplace_back(
        pool.enqueue([this, font_path]() { return GetFontInfo(font_path); }));
  }

  for (auto&& result : results) {
    auto font_list = result.get();
    font_list_.insert(font_list.begin(), font_list.end());
  }
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

  for (const auto& font : font_list_) {
    nlohmann::ordered_json js_font;
    js_font["families"] = font.second.families;
    js_font["fullnames"] = font.second.fullnames;
    js_font["psnames"] = font.second.psnames;
    js_font["weight"] = font.second.weight;
    js_font["slant"] = font.second.slant;
#ifdef _WIN32
    js_font["path"] = WideToU8(font.first);
#else
    js_font["path"] = font.first;
#endif
    js_font["index"] = font.second.index;
    js_font["last_write_time"] = font.second.last_write_time;

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
      std::pair<AString, FontInfo> font;
      font.second.families = js_font["families"];
      font.second.fullnames = js_font["fullnames"];
      font.second.psnames = js_font["psnames"];
      font.second.weight = js_font["weight"];
      font.second.slant = js_font["slant"];
#ifdef _WIN32
      font.first = U8ToWide(js_font["path"]);
#else
      font.first = js_font["path"];
#endif
      font.second.index = js_font["index"];
      font.second.last_write_time = js_font["last_write_time"];
      font_list_in_db_.emplace(font);
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

  try {
    const fs::recursive_directory_iterator iter(dir_path);

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

std::unordered_multimap<AString, FontParser::FontInfo> FontParser::GetFontInfo(
    const AString& font_path) {
  std::string last_write_time;
  std::vector<std::unordered_multimap<AString, FontInfo>::iterator> iters_found;

  if (ExistInDB(font_path, last_write_time, iters_found)) {
    return GetFontInfoFromDB(font_path, last_write_time, iters_found);
  }

  std::unordered_multimap<AString, FontInfo> font_list;

  FT_Library ft_library;
  FT_Init_FreeType(&ft_library);

  FT_StreamRec ft_stream = {};
  FT_Open_Args open_args = {};
  NewOpenArgs(font_path, ft_stream, open_args);

  FT_Face ft_face;
  if (!OpenFontFace(ft_library, open_args, ft_face, font_path)) {
    return font_list;
  }

  const long n_face = ft_face->num_faces;
  for (long face_idx = 0; face_idx < n_face; ++face_idx) {
    GetFontInfoFromFace(ft_library, ft_face, open_args, face_idx, font_list,
                        font_path, last_write_time);
  }

  if (font_list.empty()) {
    logger_->warn(_ST("\"{}\" has no parsable name."), font_path);
  }

  FT_Done_Face(ft_face);
  FT_Done_FreeType(ft_library);

  return font_list;
}

std::unordered_multimap<AString, FontParser::FontInfo>
FontParser::GetFontInfoFromDB(
    const AString& font_path, const std::string& last_write_time,
    const std::vector<std::unordered_multimap<AString, FontInfo>::iterator>&
        iters_found) {
  std::unordered_multimap<AString, FontInfo> font_list;

  for (const auto& iter : iters_found) {
    font_list.emplace(*iter);
  }

  return font_list;
}

bool FontParser::OpenFontFace(FT_Library& ft_library,
                              const FT_Open_Args& open_args, FT_Face& ft_face,
                              const AString& font_path) {
  if (FT_Open_Face(ft_library, &open_args, -1, &ft_face)) {
    logger_->warn(_ST("\"{}\" cannot be opened."), font_path);
    return false;
  }
  return true;
}

void FontParser::GetFontInfoFromFace(
    FT_Library& ft_library, FT_Face& ft_face, const FT_Open_Args& open_args,
    const long face_idx, std::unordered_multimap<AString, FontInfo>& font_list,
    const AString& font_path, const std::string& last_write_time) {
  std::pair<AString, FontInfo> font_info;
  std::vector<std::string> families;
  std::vector<std::string> fullnames;
  std::vector<std::string> psnames;

  FT_Open_Face(ft_library, &open_args, face_idx, &ft_face);

  if (FT_Has_PS_Glyph_Names(ft_face)) {
    PS_FontInfo afont_info = nullptr;
    if (!FT_Get_PS_Font_Info(ft_face, afont_info)) {
      families.emplace_back(std::string(afont_info->family_name));
      fullnames.emplace_back(std::string(afont_info->full_name));
    }
  }

  const unsigned int num_names = FT_Get_Sfnt_Name_Count(ft_face);
  for (unsigned int name_idx = 0; name_idx < num_names; ++name_idx) {
    ParseFontName(ft_face, name_idx, families, fullnames, psnames);
  }

  if (families.empty() && fullnames.empty() && psnames.empty()) {
    return;
  }

  font_info.second.slant =
      110 * !!(ft_face->style_flags & FT_STYLE_FLAG_ITALIC);

  font_info.second.weight = AssFaceGetWeight(ft_face);

  if (font_info.second.slant < 0 || font_info.second.slant > 110) {
    font_info.second.slant = 0;
  }

  if (font_info.second.weight < 100 || font_info.second.weight > 900) {
    font_info.second.weight = 400;
  }

  font_info.second.families = families;
  font_info.second.fullnames = fullnames;
  font_info.second.psnames = psnames;

  font_info.first = font_path;

  font_info.second.index = face_idx;

  font_info.second.last_write_time = last_write_time;

  font_list.emplace(font_info);
}

void FontParser::ParseFontName(const FT_Face& ft_face,
                               const unsigned int name_idx,
                               std::vector<std::string>& families,
                               std::vector<std::string>& fullnames,
                               std::vector<std::string>& psnames) {
  FT_SfntName name;
  std::string wbuf;
  std::string buf;

  if (FT_Get_Sfnt_Name(ft_face, name_idx, &name)) {
    return;
  }

  if (name.name_id != TT_NAME_ID_FULL_NAME &&
      name.name_id != TT_NAME_ID_FONT_FAMILY &&
      name.name_id != TT_NAME_ID_PS_NAME) {
    return;
  }

  if (name.platform_id != TT_PLATFORM_MICROSOFT) {
    return;
  }

  wbuf = std::string(reinterpret_cast<char*>(name.string), name.string_len);
  if (!IconvConvert(wbuf, buf, "UTF-16BE", "UTF-8")) {
    return;
  }

  if (buf.empty()) {
    return;
  }

  switch (name.name_id) {
    case TT_NAME_ID_FONT_FAMILY:
      if (std::find(families.begin(), families.end(), buf) == families.end()) {
        families.emplace_back(ToLower(buf));
      }
      break;

    case TT_NAME_ID_FULL_NAME:
      if (std::find(fullnames.begin(), fullnames.end(), buf) ==
          fullnames.end()) {
        fullnames.emplace_back(ToLower(buf));
      }
      break;

    case TT_NAME_ID_PS_NAME:
      if (std::find(psnames.begin(), psnames.end(), buf) == psnames.end()) {
        psnames.emplace_back(ToLower(buf));
      }
      break;

    default:
      break;
  }
}

int FontParser::AssFaceGetWeight(const FT_Face& face) {
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

bool FontParser::ExistInDB(
    const AString& font_path, std::string& last_write_time,
    std::vector<std::unordered_multimap<AString, FontInfo>::iterator>&
        iters_found) {
  last_write_time = GetLastWriteTime(font_path);

  if (last_write_time.empty()) {
    return false;
  }

  auto iter_pair = font_list_in_db_.equal_range(font_path);
  for (auto iter = iter_pair.first; iter != iter_pair.second; ++iter) {
    iters_found.emplace_back(iter);
  }

  if (iters_found.empty()) {
    return false;
  }

  if (iters_found[0]->second.last_write_time == last_write_time) {
    return true;
  } else {
    return false;
  }
}

std::string FontParser::GetLastWriteTime(const AString& font_path) {
#ifdef _WIN32
  FILETIME file_time;
  SYSTEMTIME system_time;
  HANDLE file_handle =
      CreateFileW(font_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (file_handle == INVALID_HANDLE_VALUE) {
    return std::string();
  }

  TCHAR date[MAX_TCHAR], time[MAX_TCHAR];

  if (!GetFileTime(file_handle, NULL, NULL, &file_time)) {
    return std::string();
  }

  FileTimeToSystemTime(&file_time, &system_time);

  if (!GetDateFormat(LOCALE_SYSTEM_DEFAULT, NULL, &system_time,
                     "yyyy'-'MM'-'dd", date, MAX_TCHAR)) {
    return std::string();
  }

  if (!GetTimeFormat(LOCALE_SYSTEM_DEFAULT, NULL, &system_time, "HH':'mm':'ss",
                     time, MAX_TCHAR)) {
    return std::string();
  }

  return "UTC " + std::string(date) + " " + std::string(time);
#else
  auto buffer = std::make_unique<struct stat>();

  if (stat(font_path.c_str(), buffer.get())) {
    return std::string();
  }

  std::stringstream ss;
  auto gmt_time = std::gmtime(&buffer->st_mtime);
  auto last_write_time = std::put_time(gmt_time, "UTC %Y-%m-%d %H:%M:%S");
  ss << last_write_time;
  return ss.str();
#endif
}

}  // namespace ass