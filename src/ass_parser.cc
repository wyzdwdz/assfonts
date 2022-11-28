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
  set_stylename_fontdesc();
  set_font_sets();
  if (font_sets_.empty()) {
    logger_->error("\"{}\" may not be a legal ASS subtitle file.",
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
  if (field < num_field - 1) {
    logger_->error(
        "\"{}\" is not a legal ASS subtitle file. Incorrect number of field.",
        ass_path_);
  }
  return words;
}

void AssParser::ParseAss() {
  bool has_style = false;
  bool has_event = false;
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
      has_style = true;
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
      has_event = true;
    }
    if (line != text_.end()) {
      ++line;
    }
  }
  if (!has_style) {
    logger_->error(
        "\"{}\" is not a legal ASS subtitle file. No Style Title found.",
        ass_path_);
  }
  if (!has_event) {
    logger_->error(
        "\"{}\" is not a legal ASS subtitle file. No Event Title found.",
        ass_path_);
  }
}

void AssParser::set_stylename_fontdesc() {
  FontDesc empty_font_desc;
  for (const auto& style : styles_) {
    if (style[1] == "Default") {
      has_default_style_ = true;
    }
    stylename_fontdesc_[style[1]] = empty_font_desc;
    std::string fontname = style[2];
    if (fontname[0] == '@') {
      fontname.erase(0, 1);
    }
    stylename_fontdesc_[style[1]].fontname = fontname;
    int val = std::stoi(style[8]);
    if (val == 1 || val == -1) {
      val = 700;
    } else if (val <= 0) {
      val = 400;
    }
    stylename_fontdesc_[style[1]].bold = val;
    val = std::stoi(style[9]);
    if (val == 1) {
      val = 100;
    } else if (val <= 0) {
      val = 0;
    }
    stylename_fontdesc_[style[1]].italic = val;
  }
}

void AssParser::set_font_sets() {
  const std::set<char32_t> empty_set;
  for (const auto& dialogue : dialogues_) {
    FontDesc font_desc_style;
    FontDesc font_desc;
    if (stylename_fontdesc_.find(dialogue[4]) == stylename_fontdesc_.end()) {
      if (has_default_style_) {
        font_desc_style = stylename_fontdesc_["Default"];
      } else {
        logger_->error("Style \"{}\" not found.", dialogue[4]);
      }
    } else {
      font_desc_style = stylename_fontdesc_[dialogue[4]];
    }
    if (font_sets_.find(font_desc_style) == font_sets_.end()) {
      font_sets_[font_desc_style] = empty_set;
    }
    std::u32string w_dialogue =
        boost::locale::conv::utf_to_utf<char32_t>(dialogue[10]);
    auto wch = w_dialogue.begin();
    font_desc = font_desc_style;
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
        std::u32string override(wch, w_dialogue.end());
        auto pos = override.find(U'}', 0);
        if (pos == std::u32string::npos) {
          font_sets_[font_desc].insert(*wch);
          ++wch;
          continue;
        } else {
          override = std::u32string(wch + 1, wch + pos);
          StyleOverride(override, &font_desc, font_desc_style);
          wch += (pos + 1);
          continue;
        }
      }
      if (wch != w_dialogue.end()) {
        font_sets_[font_desc].insert(*wch);
        ++wch;
      }
    }
  }
}

void AssParser::StyleOverride(const std::u32string& code, FontDesc* font_desc,
                              const FontDesc& font_desc_style) {
  auto iter = code.begin();
  size_t pos = 0;
  while (true) {
    pos = code.find(U"\\fn", pos);
    if (pos != std::u32string::npos) {
      pos += 3;
      iter = code.begin() + pos;
      std::u32string font;
      while (iter != code.end() && *iter != U'\\') {
        font.push_back(*iter);
        ++iter;
        ++pos;
      }
      if (!boost::algorithm::trim_copy(font).empty()) {
        std::string fontname = boost::locale::conv::utf_to_utf<char>(
            boost::algorithm::trim_copy(font));
        if (fontname[0] == '@') {
          fontname.erase(0, 1);
        }
        font_desc->fontname = fontname;
      } else {
        font_desc->fontname = font_desc_style.fontname;
      }
    } else {
      break;
    }
  }
  pos = 0;
  while (true) {
    pos = code.find(U"\\b", pos);
    if (pos != std::u32string::npos) {
      pos += 2;
      iter = code.begin() + pos;
      if (iter == code.end() ||
          !((*iter >= U'0' && *iter <= U'9') || *iter == U'-')) {
        continue;
      }
      std::u32string bold;
      while (iter != code.end() && *iter != U'\\') {
        bold.push_back(*iter);
        ++iter;
        ++pos;
      }
      if (!boost::algorithm::trim_copy(bold).empty()) {
        int val = std::stoi(boost::locale::conv::utf_to_utf<char>(
            boost::algorithm::trim_copy(bold)));
        if (val == 1 || val == -1) {
          val = 700;
        } else if (val <= 0) {
          val = 400;
        }
        font_desc->bold = val;
      } else {
        font_desc->bold = font_desc_style.bold;
      }
    } else {
      break;
    }
  }
  pos = 0;
  while (true) {
    pos = code.find(U"\\i", pos);
    if (pos != std::u32string::npos) {
      pos += 2;
      iter = code.begin() + pos;
      if (iter == code.end() ||
          !((*iter >= U'0' && *iter <= U'9') || *iter == U'-')) {
        continue;
      }
      std::u32string italic;
      while (iter != code.end() && *iter != U'\\') {
        italic.push_back(*iter);
        ++iter;
        ++pos;
      }
      if (!boost::algorithm::trim_copy(italic).empty()) {
        int val = std::stoi(boost::locale::conv::utf_to_utf<char>(
            boost::algorithm::trim_copy(italic)));
        if (val == 1) {
          val = 100;
        } else if (val <= 0) {
          val = 0;
        }
        font_desc->italic = val;
      } else {
        font_desc->italic = font_desc_style.italic;
      }
    } else {
      break;
    }
  }
  pos = 0;
  while (true) {
    pos = code.find(U"\\r", pos);
    if (pos != std::u32string::npos) {
      pos += 2;
      iter = code.begin() + pos;
      std::u32string style;
      while (iter != code.end() && *iter != U'\\') {
        style.push_back(*iter);
        ++iter;
        ++pos;
      }
      std::string style_name = boost::locale::conv::utf_to_utf<char>(
          boost::algorithm::trim_copy(style));
      if (stylename_fontdesc_.find(style_name) == stylename_fontdesc_.end()) {
        if (has_default_style_) {
          *font_desc = stylename_fontdesc_["Default"];
        } else {
          logger_->error("Style \"{}\" not found.", style_name);
        }
      } else {
        *font_desc = stylename_fontdesc_[style_name];
      }
    } else {
      break;
    }
  }
}

}  // namespace ass