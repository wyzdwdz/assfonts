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

#include <exception>
#include <filesystem>
#include <sstream>

#include <asshdr/ass_recolorize.h>
#include <compact_enc_det/compact_enc_det.h>
#include <util/encodings/encodings.h>

namespace fs = std::filesystem;

namespace ass {

void AssParser::set_output_dir_path(const AString& output_dir_path) {
  output_dir_path_ = output_dir_path;
}

bool AssParser::ReadFile(const AString& ass_file_path) {
  fs::path ass_path(ass_file_path);
  std::ifstream ass_file(ass_file_path, std::ios::binary);
  std::string buf_u8;
  unsigned int line_num = 0;
  if (ass_file.is_open()) {
    if (!GetUTF8(ass_file, buf_u8)) {
      return false;
    }
    ass_path_ = ass_file_path;
    std::istringstream isstream(buf_u8);
    std::string line;
    while (SafeGetLine(isstream, line)) {
      ++line_num;
      if (Trim(ToLower(line)) == "[fonts]") {
        has_fonts_ = true;
        while (SafeGetLine(isstream, line)) {
          ++line_num;
          std::string tmp = Trim(ToLower(line));
          if (tmp == "[events]" || tmp == "[script info]" ||
              tmp == "[v4 styles]" || tmp == "[v4+ styles]" ||
              tmp == "[graphics]") {
            TextInfo text_info = {line_num, line};
            text_.emplace_back(text_info);
            break;
          }
        }
      } else {
        TextInfo text_info = {line_num, line};
        text_.emplace_back(text_info);
      }
    }
  } else {
    logger_->error(_ST("\"{}\" cannot be opened."), ass_path.native());
    return false;
  }
  if (!ParseAss()) {
    return false;
  }
  set_stylename_fontdesc();
  if (!set_font_sets()) {
    return false;
  }
  std::vector<FontDesc> keys_for_del;
  for (const auto& font_set : font_sets_) {
    if (font_set.second.size() == 0) {
      keys_for_del.emplace_back(font_set.first);
    }
  }
  for (const auto& key_del : keys_for_del) {
    font_sets_.erase(key_del);
  }
  if (font_sets_.empty()) {
    logger_->error(_ST("Failed to parse \"{}\". Format error."),
                   ass_path.native());
    return false;
  }
  CleanFonts();
  return true;
}

bool AssParser::get_has_fonts() const {
  return has_fonts_;
}

std::vector<std::string> AssParser::get_text() const {
  std::vector<std::string> text;
  for (const TextInfo& t : text_) {
    text.emplace_back(t.text);
  }
  return text;
}

AString AssParser::get_ass_path() const {
  return ass_path_;
}

bool AssParser::Recolorize(const AString& ass_file_path,
                           const unsigned int& brightness) {
  fs::path ass_path(ass_file_path);
  std::ifstream ass_file(ass_file_path, std::ios::binary);
  std::string buf_u8;
  if (!ass_file.is_open()) {
    logger_->error(_ST("\"{}\" cannot be opened."), ass_path.native());
    return false;
  }
  if (!GetUTF8(ass_file, buf_u8)) {
    return false;
  }
  unsigned int out_size = buf_u8.size() * 2;
  std::unique_ptr<char> out_text(new char[out_size]);
  if (!asshdr::AssRecolor(buf_u8.c_str(), buf_u8.size(), out_text.get(),
                          out_size, brightness)) {
    logger_->error(_ST("Recolor failed: {}"), ass_path.native());
    return false;
  }
  fs::path output_file_path = fs::path(
      output_dir_path_ + fs::path::preferred_separator +
      ass_path.stem().native() + _ST(".hdr") + ass_path.extension().native());
  std::ofstream os(output_file_path.native());
  os << std::string(out_text.get(), out_size);
  logger_->info(_ST("Recolored ass file has been saved in \"{}\""),
                output_file_path.native());
  return true;
}

void AssParser::Clear() {
  ass_path_.clear();
  output_dir_path_.clear();
  text_.clear();
  styles_.clear();
  has_default_style_ = false;
  has_fonts_ = false;
  dialogues_.clear();
  font_sets_.clear();
  stylename_fontdesc_.clear();
}

bool AssParser::GetUTF8(const std::ifstream& is, std::string& res) {
  std::stringstream sstream;
  sstream << is.rdbuf();
  bool is_reliable = false;
  int bytes_consumed;
  Encoding encoding = CompactEncDet::DetectEncoding(
      sstream.str().c_str(), static_cast<int>(sstream.str().size()), nullptr,
      nullptr, nullptr, UNKNOWN_ENCODING, UNKNOWN_LANGUAGE,
      CompactEncDet::QUERY_CORPUS, false, &bytes_consumed, &is_reliable);
  std::string encode_name = MimeEncodingName(encoding);
  if (encode_name == "GB2312") {
    encode_name = "GB18030";
  }
  logger_->info("Detect input file encoding:  \"{}\"", encode_name);
  if (encode_name == "UTF-8") {
    res = sstream.str();
  }
  if (!IconvConvert(sstream.str(), res, encode_name, "UTF-8")) {
    logger_->error("Recode to \"UTF-8\" failed.");
    return false;
  }
  return true;
}

bool AssParser::FindTitle(const std::string& line, const std::string& title) {
  if (ToLower(line).substr(0, title.size()) == ToLower(title)) {
    return true;
  } else {
    return false;
  }
}

bool AssParser::ParseLine(const std::string& line,
                          const unsigned int& num_field,
                          std::vector<std::string>& res) {
  std::vector<std::string> words;
  std::string word;
  auto ch = line.begin();
  unsigned int field = 0;
  for (; ch != line.end(); ++ch) {
    if (*ch != ':') {
      word.push_back(*ch);
    } else {
      words.emplace_back(Trim(word));
      word.clear();
      ++ch;
      break;
    }
  }
  for (; ch != line.end(); ++ch) {
    if (*ch != ',') {
      word.push_back(*ch);
    } else {
      words.emplace_back(Trim(word));
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
  words.emplace_back(word);
  if (field < num_field - 1) {
    logger_->error(_ST("Failed to parse \"{}\". Incorrect number of field."),
                   ass_path_);
    return false;
  }
  res = words;
  return true;
}

bool AssParser::ParseAss() {
  bool has_style = false;
  bool has_event = false;
  std::vector<std::string> res;
  auto line = text_.begin();
  while (true) {
    if (line == text_.end()) {
      break;
    }
    if (FindTitle((*line).text, "[V4+ Styles]") ||
        FindTitle((*line).text, "[V4 Styles]")) {
      ++line;
      for (; line != text_.end(); ++line) {
        if (FindTitle((*line).text, "[")) {
          break;
        }
        if (FindTitle((*line).text, "Style:")) {
          if (!ParseLine((*line).text, 10, res)) {
            return false;
          }
          styles_.emplace_back(res);
        }
      }
      has_style = true;
    }
    if (FindTitle((*line).text, "[Events]")) {
      ++line;
      for (; line != text_.end(); ++line) {
        if (FindTitle((*line).text, "[")) {
          break;
        }
        if (FindTitle((*line).text, "Dialogue:")) {
          if (!ParseLine((*line).text, 10, res)) {
            return false;
          }
          DialogueInfo dialogue_info = {(*line).line_num, res};
          dialogues_.emplace_back(dialogue_info);
        }
      }
      has_event = true;
    }
    if (line != text_.end()) {
      ++line;
    }
  }
  if (!has_style) {
    logger_->error(_ST("Failed to parse \"{}\". No Style Title found."),
                   ass_path_);
    return false;
  }
  if (!has_event) {
    logger_->error(_ST("Failed to parse \"{}\". No Event Title found."),
                   ass_path_);
    return false;
  }
  return true;
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
    int val = StringToInt(style[8]);
    if (val == 1 || val == -1) {
      val = 700;
    } else if (val <= 0) {
      val = 400;
    }
    stylename_fontdesc_[style[1]].bold = val;
    val = StringToInt(style[9]);
    if (val == 1) {
      val = 100;
    } else if (val <= 0) {
      val = 0;
    }
    stylename_fontdesc_[style[1]].italic = val;
  }
}

bool AssParser::set_font_sets() {
  const std::set<char32_t> empty_set;
  for (const auto& dialogue : dialogues_) {
    FontDesc font_desc_style;
    FontDesc font_desc;
    if (stylename_fontdesc_.find(dialogue.dialogue[4]) ==
        stylename_fontdesc_.end()) {
      if (has_default_style_) {
        font_desc_style = stylename_fontdesc_["Default"];
      } else {
        logger_->warn("Style \"{}\" not found. (Line {})", dialogue.dialogue[4],
                      dialogue.line_num);
        font_desc_style.fontname = "";
      }
    } else {
      font_desc_style = stylename_fontdesc_[dialogue.dialogue[4]];
    }
    if (font_sets_.find(font_desc_style) == font_sets_.end()) {
      font_sets_[font_desc_style] = empty_set;
    }
    std::u32string w_dialogue = U8ToU32(dialogue.dialogue[10]);
    auto wch = w_dialogue.begin();
    font_desc = font_desc_style;
    while (wch != w_dialogue.end()) {
      if (*wch == U'\\') {
        if ((wch + 1) != w_dialogue.end() &&
            (*(wch + 1) == U'h' || *(wch + 1) == U'n' || *(wch + 1) == U'N')) {
          wch = wch + 2;
          continue;
        }
      }
      if (*wch == U'{') {
        std::u32string override(wch, w_dialogue.end());
        auto pos = override.find(U'}', 0);
        if (pos == std::u32string::npos) {
          if (!font_desc_style.fontname.empty()) {
            font_sets_[font_desc].insert(*wch);
          }
          ++wch;
          continue;
        } else {
          override = std::u32string(wch + 1, wch + pos);
          if (!StyleOverride(override, font_desc, font_desc_style,
                             dialogue.line_num)) {
            return false;
          }
          wch += (pos + 1);
          continue;
        }
      }
      if (wch != w_dialogue.end()) {
        if (!font_desc_style.fontname.empty()) {
          font_sets_[font_desc].insert(*wch);
        }
        ++wch;
      }
    }
  }
  return true;
}

bool AssParser::StyleOverride(const std::u32string& code, FontDesc& font_desc,
                              const FontDesc& font_desc_style,
                              const unsigned int& line_num) {
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
      if (!Trim(font).empty()) {
        std::string fontname = U32ToU8(Trim(font));
        if (fontname[0] == '@') {
          fontname.erase(0, 1);
        }
        font_desc.fontname = fontname;
      } else {
        font_desc.fontname = font_desc_style.fontname;
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
      if (iter == code.end() || !((*iter >= U'0' && *iter <= U'9') ||
                                  *iter == U'-' || *iter == U' ')) {
        continue;
      }
      std::u32string bold;
      while (iter != code.end() && *iter != U'\\') {
        bold.push_back(*iter);
        ++iter;
        ++pos;
      }
      if (!Trim(bold).empty()) {
        int val = StringToInt(Trim(bold));
        if (val == 1 || val == -1) {
          val = 700;
        } else if (val <= 0) {
          val = 400;
        }
        font_desc.bold = val;
      } else {
        font_desc.bold = font_desc_style.bold;
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
      if (iter == code.end() || !((*iter >= U'0' && *iter <= U'9') ||
                                  *iter == U'-' || *iter == U' ')) {
        continue;
      }
      std::u32string italic;
      while (iter != code.end() && *iter != U'\\') {
        italic.push_back(*iter);
        ++iter;
        ++pos;
      }
      if (!Trim(italic).empty()) {
        int val = StringToInt(Trim(italic));
        if (val == 1) {
          val = 100;
        } else if (val <= 0) {
          val = 0;
        }
        font_desc.italic = val;
      } else {
        font_desc.italic = font_desc_style.italic;
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
      if (code.end() - iter >= 2 && std::string(iter, iter + 2) == "nd") {
        continue;
      }
      std::u32string style;
      while (iter != code.end() && *iter != U'\\') {
        style.push_back(*iter);
        ++iter;
        ++pos;
      }
      std::string style_name = U32ToU8(Trim(style));
      if (!style_name.empty()) {
        if (stylename_fontdesc_.find(style_name) == stylename_fontdesc_.end()) {
          logger_->warn("Style \"{}\" not found. (Line {})", style_name,
                        line_num);
        } else {
          font_desc = stylename_fontdesc_[style_name];
        }
      } else {
        font_desc = font_desc_style;
      }
    } else {
      break;
    }
  }
  return true;
}

void AssParser::CleanFonts() {
  if (!has_fonts_) {
    return;
  }
  fs::path input_path(ass_path_);
  fs::path output_path(output_dir_path_ + fs::path::preferred_separator +
                       input_path.stem().native() + _ST(".cleaned") +
                       input_path.extension().native());
  logger_->info(
      _ST("Found fonts in \"{}\". Delete them and save new file in \"{}\""),
      input_path.native(), output_path.native());
  std::ofstream os(output_path.native());
  size_t num_line = 0;
  for (const auto& line : text_) {
    ++num_line;
    os << line.text;
    if (num_line != text_.size()) {
      os << '\n';
    }
  }
}

}  // namespace ass