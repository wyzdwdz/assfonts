/*  This file is part of assfonts.
 *
 *  assfonts is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation,
 *  either version 3 of the License,
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
#include <sstream>

#include <asshdr/ass_recolorize.h>
#include <compact_enc_det/compact_enc_det.h>
#include <util/encodings/encodings.h>
#include <ghc/filesystem.hpp>

namespace fs = ghc::filesystem;

namespace ass {

void AssParser::set_output_dir_path(const AString& output_dir_path) {
  output_dir_path_ = output_dir_path;
}

bool AssParser::ReadFile(const AString& ass_file_path) {
  fs::path ass_path(ass_file_path);
  std::ifstream ass_file(ass_file_path, std::ios::binary);
  std::string buf_u8;
  unsigned int line_num = 0;

  if (!ass_file.is_open()) {
    logger_->Error(_ST("\"{}\" cannot be opened."), ass_path.native());
    return false;
  }

  logger_->Info(_ST("Reading input file: \"{}\""), ass_path.native());

  if (!GetUTF8(ass_file, buf_u8)) {
    return false;
  }

  ass_path_ = ass_file_path;
  std::istringstream isstream(buf_u8);
  std::string line;

  while (SafeGetLine(isstream, line)) {
    ++line_num;
    std::string trimmed_line = Trim(ToLower(line));

    if (trimmed_line == "[fonts]") {
      has_fonts_ = true;
      SkipFontsLines(isstream, line_num);
    } else {
      TextInfo text_info = {line_num, line};
      text_.emplace_back(text_info);
    }
  }

  if (!ParseAss()) {
    return false;
  }

  set_stylename_fontdesc();

  set_font_sets();

  if (font_sets_.empty()) {
    logger_->Error(_ST("Failed to parse \"{}\". Format error."),
                   ass_path.native());
    return false;
  }

  if (!CleanFonts()) {
    return false;
  }

  return true;
}

void AssParser::SkipFontsLines(std::istringstream& is, unsigned int line_num) {
  std::string line;

  while (SafeGetLine(is, line)) {
    ++line_num;
    std::string tmp = Trim(ToLower(line));

    if (tmp == "[events]" || tmp == "[script info]" || tmp == "[v4 styles]" ||
        tmp == "[v4+ styles]" || tmp == "[graphics]") {
      TextInfo text_info = {line_num, line};
      text_.emplace_back(text_info);
      break;
    }
  }
}

bool AssParser::get_has_fonts() const {
  return has_fonts_;
}

std::vector<AssParser::TextInfo> AssParser::get_text() const {
  return text_;
}

AString AssParser::get_ass_path() const {
  return ass_path_;
}

std::vector<AssParser::RenameInfo> AssParser::get_rename_infos() const {
  return rename_infos_;
}

std::map<AssParser::FontDesc, std::unordered_set<char32_t>>
AssParser::get_font_sets() const {
  return font_sets_;
}

bool AssParser::Recolorize(const AString& ass_file_path,
                           const unsigned int brightness) {
  fs::path ass_path(ass_file_path);
  std::ifstream ass_file(ass_file_path, std::ios::binary);
  std::string buf_u8;

  if (!ass_file.is_open()) {
    logger_->Error(_ST("\"{}\" cannot be opened."), ass_path.native());
    return false;
  }

  if (!GetUTF8(ass_file, buf_u8)) {
    return false;
  }

  unsigned int out_size = buf_u8.size() * 2;
  std::unique_ptr<char[]> out_text(new char[out_size]);

  if (!asshdr::AssRecolor(buf_u8.c_str(), buf_u8.size(), out_text.get(),
                          out_size, brightness)) {
    logger_->Error(_ST("Recolor failed: {}"), ass_path.native());
    return false;
  }

  fs::path output_file_path = fs::path(
      output_dir_path_ + fs::path::preferred_separator +
      ass_path.stem().native() + _ST(".hdr") + ass_path.extension().native());

  std::ofstream os(output_file_path.native());
  if (!os.is_open()) {
    logger_->Error(_ST("Failed to write the file: {}"), ass_path.native());
    return false;
  }
  os << std::string(out_text.get(), out_size);
  logger_->Info(_ST("Recolored ass file has been saved in \"{}\""),
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

  logger_->Info("Detect input file encoding:  \"{}\"", encode_name);

  if (encode_name == "UTF-8") {
    res = sstream.str();
    return true;
  }

  if (!IconvConvert(sstream.str(), res, encode_name, "UTF-8")) {
    logger_->Error("Recode to \"UTF-8\" failed.");
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

bool AssParser::ParseLine(const std::string& line, const unsigned int num_field,
                          std::vector<nonstd::string_view>& res) {
  nonstd::string_view word;
  auto ch = line.begin();
  unsigned int field = 0;

  auto GetWords = [&](const char delimiter) {
    nonstd::string_view substr(&(*ch), line.end() - ch);
    size_t pos = 0;
    while ((pos = substr.find(delimiter)) != nonstd::string_view::npos) {
      res.emplace_back(Trim(nonstd::string_view(&(*ch), pos)));

      pos += 1;

      if (delimiter == ':') {
        ch += pos;
        break;
      } else if (delimiter == ',') {
        ++field;

        if (field >= num_field - 1) {
          ch += pos;
          break;
        }
      }

      ch += pos;
      substr = nonstd::string_view(&(*ch), line.end() - ch);
    }
  };

  GetWords(':');
  GetWords(',');

  if (ch != line.end()) {
    word = nonstd::string_view(&(*ch), line.end() - ch);
  } else {
    word = nonstd::string_view("");
  }

  res.emplace_back(word);

  if (field < num_field - 1) {
    logger_->Error(_ST("Failed to parse \"{}\". Incorrect number of field."),
                   ass_path_);
    return false;
  }

  return true;
}

bool AssParser::ParseAss() {
  bool has_style = false;
  bool has_event = false;
  auto line = text_.begin();

  while (line != text_.end()) {
    if (!GetStyles(line, text_.end(), has_style)) {
      return false;
    }

    if (!GetEvents(line, text_.end(), has_event)) {
      return false;
    }

    if (line != text_.end()) {
      ++line;
    }
  }

  if (!has_style) {
    logger_->Error(_ST("Failed to parse \"{}\". No Style Title found."),
                   ass_path_);
    return false;
  }

  if (!has_event) {
    logger_->Error(_ST("Failed to parse \"{}\". No Event Title found."),
                   ass_path_);
    return false;
  }

  return true;
}

bool AssParser::GetStyles(std::vector<TextInfo>::iterator& line,
                          const std::vector<TextInfo>::iterator end,
                          bool& has_style) {
  if (!FindTitle((*line).text, "[V4+ Styles]") &&
      !FindTitle((*line).text, "[V4 Styles]")) {
    return true;
  }

  ++line;

  for (; line != text_.end(); ++line) {
    if (FindTitle((*line).text, "[")) {
      break;
    }

    if (!FindTitle((*line).text, "Style:")) {
      continue;
    }

    std::vector<nonstd::string_view> styles;
    if (!ParseLine((*line).text, 10, styles)) {
      return false;
    }
    StyleInfo res = {(*line).line_num, (*line).text.c_str(), styles};
    styles_.emplace_back(res);
  }
  has_style = true;

  return true;
}

bool AssParser::GetEvents(std::vector<TextInfo>::iterator& line,
                          const std::vector<TextInfo>::iterator end,
                          bool& has_event) {
  if (!FindTitle((*line).text, "[Events]")) {
    return true;
  }

  ++line;

  for (; line != text_.end(); ++line) {
    if (FindTitle((*line).text, "[")) {
      break;
    }

    if (!FindTitle((*line).text, "Dialogue:")) {
      continue;
    }

    std::vector<nonstd::string_view> res;
    if (!ParseLine((*line).text, 10, res)) {
      return false;
    }

    DialogueInfo dialogue_info = {(*line).line_num, (*line).text.c_str(), res};
    dialogues_.emplace_back(dialogue_info);
  }

  has_event = true;

  return true;
}

void AssParser::set_stylename_fontdesc() {
  FontDesc empty_font_desc;

  for (const auto& style : styles_) {
    if (style.style[1] == "Default") {
      has_default_style_ = true;
    }

    stylename_fontdesc_[std::string(style.style[1])] = empty_font_desc;
    nonstd::string_view fontname = style.style[2];

    if (fontname[0] == '@') {
      fontname = fontname.substr(1);
    }

    stylename_fontdesc_[std::string(style.style[1])].fontname =
        std::string(fontname);
    stylename_fontdesc_[std::string(style.style[1])].bold =
        CalculateBold(StringToInt(std::string(style.style[8])));
    stylename_fontdesc_[std::string(style.style[1])].italic =
        CalculateItalic(StringToInt(std::string(style.style[9])));

    RenameInfo rename_info = {
        style.line_num,
        static_cast<size_t>(&(*fontname.begin()) - style.line_beg),
        static_cast<size_t>(&(*fontname.end()) - style.line_beg),
        std::string(fontname), ""};
    rename_infos_.emplace_back(rename_info);
  }
}

int AssParser::CalculateBold(int value) {
  if (value == 1 || value == -1) {
    return 700;
  } else if (value <= 0) {
    return 400;
  }
  return value;
}

int AssParser::CalculateItalic(int value) {
  if (value == 1 || value == -1) {
    return 100;
  } else if (value <= 0) {
    return 0;
  }
  return value;
}

void AssParser::set_font_sets() {
  const std::unordered_set<char32_t> empty_set;

  for (const auto& dialogue : dialogues_) {
    FontDesc font_desc_style = GetFontDescStyle(dialogue);
    FontDesc font_desc;

    if (font_sets_.find(font_desc_style) == font_sets_.end()) {
      font_sets_[font_desc_style] = empty_set;
    }

    auto wch = Iterator(dialogue.dialogue[10]);
    font_desc = font_desc_style;

    const auto end = Iterator(dialogue.dialogue[10], true);

    while (wch != end) {
      GetCharacter(wch, end, font_desc_style, font_desc, dialogue.line_num,
                   dialogue.line_beg);
    }
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
}

AssParser::FontDesc AssParser::GetFontDescStyle(const DialogueInfo& dialogue) {
  FontDesc font_desc_style;

  if (stylename_fontdesc_.find(std::string(dialogue.dialogue[4])) ==
      stylename_fontdesc_.end()) {

    if (has_default_style_) {
      font_desc_style = stylename_fontdesc_["Default"];
    } else {
      logger_->Warn("Style \"{}\" not found. (Line {})",
                    std::string(dialogue.dialogue[4]), dialogue.line_num);
      font_desc_style.fontname = "";
    }

  } else {
    font_desc_style = stylename_fontdesc_[std::string(dialogue.dialogue[4])];
  }

  return font_desc_style;
}

void AssParser::GetCharacter(Iterator& wch, const Iterator end,
                             const FontDesc& font_desc_style,
                             FontDesc& font_desc, const unsigned int line_num,
                             const char* line_beg) {
  if (*wch == U'\\' && (wch + 1) != end &&
      (*(wch + 1) == U'h' || *(wch + 1) == U'n' || *(wch + 1) == U'N')) {
    wch += 2;
    return;
  }

  if (*wch == U'{') {
    nonstd::string_view override(&(*wch.ToStdIter()),
                                 end.ToStdIter() - wch.ToStdIter());
    auto pos = U8Find(override, U"}", 0);

    if (pos == Iterator::NPOS) {
      if (!font_desc_style.fontname.empty()) {
        font_sets_[font_desc].insert(*wch);
      }
      ++wch;
      return;

    } else {
      override = nonstd::string_view(
          &(*((wch + 1).ToStdIter())),
          (Iterator(override) += pos).ToStdIter() - (wch + 1).ToStdIter());

      StyleOverride(override, font_desc, font_desc_style, line_num, line_beg);
      wch += (pos + 1);
      return;
    }
  }

  if (wch != end) {
    if (!font_desc_style.fontname.empty()) {
      font_sets_[font_desc].insert(*wch);
    }
    ++wch;
  }
}

void AssParser::StyleOverride(const nonstd::string_view code,
                              FontDesc& font_desc,
                              const FontDesc& font_desc_style,
                              const unsigned int line_num,
                              const char* line_beg) {
  auto font_pos =
      ChangeFontname(code, font_desc, font_desc_style, line_num, line_beg);
  auto bold_pos = ChangeBold(code, font_desc, font_desc_style);
  auto italic_pos = ChangeItalic(code, font_desc, font_desc_style);
  ChangeStyle(code, font_desc, font_desc_style, line_num, font_pos, bold_pos,
              italic_pos);
}

AssParser::Iterator::difference_type AssParser::ChangeFontname(
    const nonstd::string_view code, FontDesc& font_desc,
    const FontDesc& font_desc_style, const unsigned int line_num,
    const char* line_beg) {
  Iterator::difference_type pos = 0;
  Iterator::difference_type last_pos = 0;

  while (true) {
    last_pos = pos;
    pos = U8Find(code, U"\\fn", pos);

    if (pos == Iterator::NPOS) {
      break;
    }

    pos += 3;
    auto iter = Iterator(code) + pos;
    auto view_beg = iter.ToStdIter();

    while (iter != code.end() && *iter != U'\\') {
      ++iter;
      ++pos;
    }

    nonstd::string_view font_view(&(*view_beg), iter.ToStdIter() - view_beg);
    font_view = Trim(font_view);

    if (font_view.empty() || font_view == '0') {
      font_desc.fontname = font_desc_style.fontname;
      continue;
    }

    if (font_view[0] == '@') {
      font_view = font_view.substr(1);
    }
    font_desc.fontname = std::string(font_view);

    RenameInfo rename_info = {
        line_num, static_cast<size_t>(&(*font_view.begin()) - line_beg),
        static_cast<size_t>(&(*font_view.end()) - line_beg),
        std::string(font_view), ""};
    rename_infos_.emplace_back(rename_info);
  }

  return last_pos;
}

AssParser::Iterator::difference_type AssParser::ChangeBold(
    const nonstd::string_view code, FontDesc& font_desc,
    const FontDesc& font_desc_style) {
  Iterator::difference_type pos = 0;
  Iterator::difference_type last_pos = 0;

  while (true) {
    last_pos = pos;
    pos = U8Find(code, U"\\b", pos);

    if (pos == Iterator::NPOS) {
      break;
    }

    pos += 2;
    auto iter = Iterator(code) + pos;

    if (iter == code.end() ||
        !((*iter >= U'0' && *iter <= U'9') || *iter == U'-' || *iter == U' ')) {
      continue;
    }

    std::u32string bold;

    while (iter != code.end() && *iter != U'\\') {
      bold.push_back(*iter);
      ++iter;
      ++pos;
    }

    std::string bold_u8 = U32ToU8(bold);

    if (!Trim(bold_u8).empty()) {
      font_desc.bold = CalculateBold(StringToInt(Trim(bold_u8)));
    } else {
      font_desc.bold = font_desc_style.bold;
    }
  }

  return last_pos;
}

AssParser::Iterator::difference_type AssParser::ChangeItalic(
    const nonstd::string_view code, FontDesc& font_desc,
    const FontDesc& font_desc_style) {
  Iterator::difference_type pos = 0;
  Iterator::difference_type last_pos = 0;

  while (true) {
    last_pos = pos;
    pos = U8Find(code, U"\\i", pos);

    if (pos == Iterator::NPOS) {
      break;
    }

    pos += 2;
    auto iter = Iterator(code) + pos;

    if (iter == code.end() ||
        !((*iter >= U'0' && *iter <= U'9') || *iter == U'-' || *iter == U' ')) {
      continue;
    }

    std::u32string italic;

    while (iter != code.end() && *iter != U'\\') {
      italic.push_back(*iter);
      ++iter;
      ++pos;
    }

    std::string italic_u8 = U32ToU8(italic);

    if (!Trim(italic_u8).empty()) {
      font_desc.italic = CalculateItalic(StringToInt(Trim(italic_u8)));
    } else {
      font_desc.italic = font_desc_style.italic;
    }
  }

  return last_pos;
}

void AssParser::ChangeStyle(const nonstd::string_view code, FontDesc& font_desc,
                            const FontDesc& font_desc_style,
                            const unsigned int line_num,
                            Iterator::difference_type font_pos,
                            Iterator::difference_type bold_pos,
                            Iterator::difference_type italic_pos) {
  Iterator::difference_type pos = 0;
  Iterator::difference_type last_pos = 0;
  FontDesc update_desc = font_desc;

  while (true) {
    last_pos = pos;
    pos = U8Find(code, U"\\r", pos);

    if (pos == Iterator::NPOS) {
      break;
    }

    pos += 2;
    auto iter = Iterator(code) + pos;

    if (code.end() - iter.ToStdIter() >= 2 &&
        std::string(iter.ToStdIter(), iter.ToStdIter() + 2) == "nd") {
      continue;
    }

    std::u32string style;

    while (iter != code.end() && *iter != U'\\') {
      style.push_back(*iter);
      ++iter;
      ++pos;
    }

    std::string style_name = Trim(U32ToU8(style));

    if (style_name.empty()) {
      update_desc = font_desc_style;
      continue;
    }

    if (stylename_fontdesc_.find(style_name) == stylename_fontdesc_.end()) {
      logger_->Warn("Style \"{}\" not found. (Line {})", style_name, line_num);
      update_desc = font_desc_style;
    } else {
      update_desc = stylename_fontdesc_[style_name];
    }
  }

  if (last_pos > font_pos) {
    font_desc.fontname = update_desc.fontname;
  }
  if (last_pos > bold_pos) {
    font_desc.bold = update_desc.bold;
  }
  if (last_pos > italic_pos) {
    font_desc.italic = update_desc.italic;
  }
}

bool AssParser::CleanFonts() {
  if (!has_fonts_) {
    return true;
  }

  fs::path input_path(ass_path_);
  fs::path output_path = fs::path(output_dir_path_) /
                         (input_path.stem().native() + _ST(".cleaned") +
                          input_path.extension().native());

  logger_->Info(
      _ST("Found fonts in \"{}\". Delete them and save new file in \"{}\""),
      input_path.native(), output_path.native());

  std::ofstream os(output_path.native());
  if (!os.is_open()) {
    logger_->Error(_ST("Failed to write the file: {}"), output_path.native());
    return false;
  }
  size_t num_line = 0;

  for (const auto& line : text_) {
    ++num_line;
    os << line.text;

    if (num_line != text_.size()) {
      os << '\n';
    }
  }

  return true;
}

}  // namespace ass
