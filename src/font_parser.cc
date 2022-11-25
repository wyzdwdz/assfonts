#include "font_parser.h"

#include <fstream>
#include <regex>

#ifdef __cplusplus
extern "C" {
#endif
#include FT_MODULE_H
#include FT_TYPE1_TABLES_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_IDS_H
#ifdef __cplusplus
}
#endif
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/filesystem.hpp>
#include <boost/locale.hpp>
#include <boost/serialization/vector.hpp>

namespace fs = boost::filesystem;

namespace ass {

void FontParser::LoadFonts(const std::string& fonts_dir) {
  fonts_path_ = FindFileInDir(fonts_dir, ".+\\.(ttf|otf|ttc|otc)$");
  for (const auto& font_path : fonts_path_) {
    GetFontInfo(font_path);
  }
}

void FontParser::SaveDB(const std::string& db_path) {
  fs::path file_path(db_path);
  if (fs::is_regular_file(file_path)) {
    logger_->warn("\"{}\" already exists. It will be overwritten.",
                  file_path.generic_string());
    fs::remove(file_path);
  }
  std::ofstream db_file(file_path.generic_string(), std::ios::binary);
  boost::archive::binary_oarchive oa(db_file);
  oa << font_list_;
  db_file.close();
  logger_->info("Fonts database has been saved in \"{}\"",
                file_path.generic_string());
}

void FontParser::LoadDB(const std::string& db_path) {
  fs::path file_path(db_path);
  std::ifstream db_file(file_path.generic_string(), std::ios::binary);
  if (db_file.is_open()) {
    boost::archive::binary_iarchive ia(db_file);
    std::vector<FontInfo> tmp_font_list;
    ia >> font_list_in_db_;
    db_file.close();
    logger_->info("Load fonts database \"{}\"", file_path.generic_string());
  } else {
    logger_->warn("Fonts database \"{}\" doesn't exists.",
                  file_path.generic_string());
  }
}

void FontParser::clean_font_list() {
  font_list_.clear();
}

std::vector<std::string> FontParser::FindFileInDir(const std::string& dir,
                                                   const std::string& pattern) {
  std::vector<std::string> res;
  const std::regex r(pattern, std::regex::icase);
  const fs::path dir_path(dir);
  const fs::recursive_directory_iterator iter(dir_path);
  for (const auto& dir_entry : iter) {
    if (std::regex_match(dir_entry.path().generic_string(), r)) {
      res.push_back(dir_entry.path().generic_string());
    }
  }
  return res;
}

bool FontParser::GetFontInfo(const std::string& font_path) {
  std::vector<std::string> families;
  std::vector<std::string> fullnames;
  std::vector<std::string> psnames;
  FontInfo font_info;
  FT_Face ft_face;
  if (FT_New_Face(ft_library_, font_path.c_str(), -1, &ft_face)) {
    logger_->warn("\"{}\" cannot be opened.", font_path);
    return false;
  }
  const long n_face = ft_face->num_faces;
  for (long face_idx = 0; face_idx < n_face; ++face_idx) {
    FT_New_Face(ft_library_, font_path.c_str(), face_idx, &ft_face);
    if (!(ft_face->face_flags & FT_FACE_FLAG_SCALABLE)) {
      logger_->warn("\"{}\"[{}] contains no scalable font face.", font_path,
                    face_idx);
      continue;
    }
    const unsigned int num_names = FT_Get_Sfnt_Name_Count(ft_face);
    for (unsigned int i = 0; i < num_names; i++) {
      FT_SfntName name;
      std::u16string wbuf;
      std::string buf;
      char16_t wch = 0;
      if (FT_Get_Sfnt_Name(ft_face, i, &name)) {
        continue;
      }
      if (name.name_id != TT_NAME_ID_FULL_NAME &&
          name.name_id != TT_NAME_ID_FONT_FAMILY &&
          name.name_id != TT_NAME_ID_PS_NAME) {
        continue;
      }
      if (!(name.platform_id == TT_PLATFORM_MICROSOFT)) {
        continue;
      }
      if (name.string_len % 2 != 0) {
        continue;
      }
      for (unsigned int p_str = 0; p_str < name.string_len / 2; ++p_str) {
        wch = *(name.string + p_str * 2) << 8;
        wch |= *(name.string + p_str * 2 + 1);
        wbuf.push_back(wch);
      }
      buf = boost::locale::conv::utf_to_utf<char>(wbuf);
      if (buf.empty()) {
        continue;
      }
      switch (name.name_id) {
        case TT_NAME_ID_FONT_FAMILY:
          if (std::find(families.begin(), families.end(), buf) ==
              families.end()) {
            families.push_back(buf);
          }
          break;
        case TT_NAME_ID_FULL_NAME:
          if (std::find(fullnames.begin(), fullnames.end(), buf) ==
              fullnames.end()) {
            fullnames.push_back(buf);
          }
          break;
        case TT_NAME_ID_PS_NAME:
          if (std::find(psnames.begin(), psnames.end(), buf) == psnames.end()) {
            psnames.push_back(buf);
          }
          break;
        default:
          break;
      }
    }
    if (families.empty() && fullnames.empty() && psnames.empty()) {
      continue;
    }
    font_info.families = families;
    font_info.fullnames = fullnames;
    font_info.psnames = psnames;
    font_info.path = font_path;
    font_info.index = face_idx;
    font_list_.push_back(font_info);
    FT_Done_Face(ft_face);
  }
  if (font_info.families.empty() && font_info.fullnames.empty() &&
      font_info.psnames.empty()) {
    logger_->warn("\"{}\" has no parsable name.", font_path);
    return false;
  } else {
    return true;
  }
}

}  // namespace ass