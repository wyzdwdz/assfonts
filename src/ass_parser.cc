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

#include "ass_parser.h"

#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/locale.hpp>

namespace fs = boost::filesystem;

namespace ass {

void AssParser::ReadFile(const std::string& ass_file_path) {
  fs::path ass_path(ass_file_path);
  std::ifstream ass_file(ass_path.generic_string());
  if (ass_file.is_open()) {
    ass_path_ = ass_file_path;
    std::string line;
    while (getline(ass_file, line)) {
      if (!IsUTF8(line)) {
        logger_->error("Only support UTF-8 subtitle.",
                       ass_path.generic_string());
      }
      text_.push_back(line);
    }
  } else {
    logger_->error("\"{}\" cannot be opened.", ass_path.generic_string());
  }
  ass_file.close();
  ParseAss();
  set_font_sets();
  if (font_sets_.empty()) {
    logger_->error("\"{}\" is not a legal ASS subtitle file.",
                   ass_path.generic_string());
  }
}

bool AssParser::IsUTF8(const std::string& line) {
  if (line.empty())
    return true;
  const unsigned char* bytes =
      reinterpret_cast<const unsigned char*>(line.c_str());
  unsigned int cp;
  int num;
  while (*bytes != 0x00) {
    if ((*bytes & 0x80) == 0x00) {
      // U+0000 to U+007F
      cp = (*bytes & 0x7F);
      num = 1;
    } else if ((*bytes & 0xE0) == 0xC0) {
      // U+0080 to U+07FF
      cp = (*bytes & 0x1F);
      num = 2;
    } else if ((*bytes & 0xF0) == 0xE0) {
      // U+0800 to U+FFFF
      cp = (*bytes & 0x0F);
      num = 3;
    } else if ((*bytes & 0xF8) == 0xF0) {
      // U+10000 to U+10FFFF
      cp = (*bytes & 0x07);
      num = 4;
    } else
      return false;
    bytes += 1;
    for (int i = 1; i < num; ++i) {
      if ((*bytes & 0xC0) != 0x80)
        return false;
      cp = (cp << 6) | (*bytes & 0x3F);
      bytes += 1;
    }
    if ((cp > 0x10FFFF) || ((cp >= 0xD800) && (cp <= 0xDFFF)) ||
        ((cp <= 0x007F) && (num != 1)) ||
        ((cp >= 0x0080) && (cp <= 0x07FF) && (num != 2)) ||
        ((cp >= 0x0800) && (cp <= 0xFFFF) && (num != 3)) ||
        ((cp >= 0x10000) && (cp <= 0x1FFFFF) && (num != 4)))
      return false;
  }
  return true;
}

bool AssParser::FindTitle(const std::string& line, const std::string& title) {
  if (boost::algorithm::to_lower_copy(line).substr(0, title.size()) ==
      boost::algorithm::to_lower_copy(title)) {
    return true;
  } else {
    return false;
  }
}

std::vector<std::string> AssParser::ParseLine(const std::string& line,
                                              const unsigned int num_field) {
  std::vector<std::string> words;
  std::string word;
  auto ch = line.begin();
  unsigned int field = 0;
  for (; ch != line.end(); ++ch) {
    if (*ch != ':') {
      word.push_back(*ch);
    } else {
      words.push_back(boost::algorithm::trim_copy(word));
      word.clear();
      ++ch;
      break;
    }
  }
  for (; ch != line.end(); ++ch) {
    if (*ch != ',') {
      word.push_back(*ch);
    } else {
      words.push_back(boost::algorithm::trim_copy(word));
      word.clear();
      ++field;
      if (field >= num_field - 1) {
        ++ch;
        break;
      }
    }
  }
  for (; ch != line.end(); ++ch) {
    word.push_back(*ch);
  }
  if (num_field == 10) {
    words.push_back(word);
  } else {
    words.push_back(boost::algorithm::trim_copy(word));
  }
  return words;
}

void AssParser::ParseAss() {
  auto line = text_.begin();
  while (true) {
    if (line == text_.end()) {
      break;
    }
    if (FindTitle(*line, "[V4+ Styles]") || FindTitle(*line, "[V4 Styles]")) {
      ++line;
      for (; line != text_.end(); ++line) {
        if (FindTitle(*line, "[")) {
          break;
        }
        if (FindTitle(*line, "Style:")) {
          styles_.push_back(ParseLine(*line, 23));
        }
      }
    }
    if (FindTitle(*line, "[Events]")) {
      ++line;
      for (; line != text_.end(); ++line) {
        if (FindTitle(*line, "[")) {
          break;
        }
        if (FindTitle(*line, "Dialogue:")) {
          dialogues_.push_back(ParseLine(*line, 10));
        }
      }
    }
    if (line != text_.end()) {
      ++line;
    }
  }
}

void AssParser::set_font_sets() {
  std::map<std::string, unsigned int> style_name_table;
  unsigned int style_index = 0;
  const std::set<char32_t> empty_set;
  for (const auto& style : styles_) {
    style_name_table[style[1]] = style_index;
    ++style_index;
  }
  for (const auto& dialogue : dialogues_) {
    std::string font = styles_[style_name_table[dialogue[4]]][2];
    if (font[0] == '@') {
      font.erase(0, 1);
    }
    if (font_sets_.find(font) == font_sets_.end()) {
      font_sets_[font] = empty_set;
    }
    std::u32string w_dialogue =
        boost::locale::conv::utf_to_utf<char32_t>(dialogue[10]);
    auto wch = w_dialogue.begin();
    while (true) {
      if (wch == w_dialogue.end()) {
        break;
      }
      if (*wch == U'\\') {
        if ((wch + 1) != w_dialogue.end() &&
            (*(wch + 1) == U'h' || *(wch + 1) == U'n' || *(wch + 1) == U'N')) {
          wch = wch + 2;
        }
      }
      if (*wch == U'{') {
        do {
          ++wch;
          if (w_dialogue.end() - wch < 3) {
            continue;
          }
          std::u32string tmp(wch, wch + 3);
          if (tmp == U"\\fn") {
            wch = wch + 3;
            std::u32string w_font;
            font.clear();
            for (; wch != w_dialogue.end(); ++wch) {
              if (*wch == U'\\' || *wch == U'}') {
                break;
              }
              w_font.push_back(*wch);
            }
            w_font = boost::algorithm::trim_copy(w_font);
            if (w_font[0] == U'@') {
              w_font.erase(0, 1);
            }
            font = boost::locale::conv::utf_to_utf<char>(w_font);
            if (font_sets_.find(font) == font_sets_.end()) {
              font_sets_[font] = empty_set;
            }
          }
        } while ((wch != w_dialogue.end()) && (*wch != U'}'));
        ++wch;
        continue;
      }
      if (wch != w_dialogue.end()) {
        font_sets_[font].insert(*wch);
        ++wch;
      }
    }
  }
}

}  // namespace ass