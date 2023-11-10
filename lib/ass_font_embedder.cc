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

#include "ass_font_embedder.h"

#include <algorithm>
#include <iterator>
#include <regex>
#include <sstream>
#include <unordered_set>

#include <ghc/filesystem.hpp>

namespace fs = ghc::filesystem;

namespace ass {

void AssFontEmbedder::set_output_dir_path(const AString& output_dir_path) {
  output_dir_path_ = output_dir_path;
}

bool AssFontEmbedder::Run(const bool is_subset_only, const bool is_embed_only,
                          const bool is_rename) {
  fs::path input_path;
  std::vector<std::string> text;

  if (!is_embed_only && is_rename) {
    AString path;
    if (!WriteRenamed(path, text)) {
      return false;
    }
    input_path = fs::path(path);

  } else {
    input_path = fs::path(fs_.ap_.get_ass_path());
    auto text_vec = fs_.ap_.get_text();
    for (const auto& line : text_vec) {
      text.emplace_back(line.text);
    }
  }

  if (is_subset_only) {
    return true;
  }

  fs::path output_path(output_dir_path_ + fs::path::preferred_separator +
                       input_path.stem().native() + _ST(".assfonts") +
                       input_path.extension().native());
  std::string buf_u8;
  std::ofstream output_ass(output_path.native());

  if (!output_ass.is_open()) {
    logger_->Error(_ST("\"{}\" cannot be created."), output_path.native());
    return false;
  }

  size_t num_line = 0;

  WriteOutput(text, num_line, output_ass);

  logger_->Info(_ST("Create font-embeded subtitle: \"{}\""),
                output_path.native());
  return true;
}

void AssFontEmbedder::WriteOutput(const std::vector<std::string>& text,
                                  size_t& num_line, std::ofstream& output_ass) {
  for (auto& line : text) {
    ++num_line;

    if (Trim(ToLower(line)) == "[events]") {
      output_ass << "[Fonts]";
      bool has_none_ttf = false;
      WriteFonts(has_none_ttf, output_ass);
      output_ass << "\n\n" << line;

      if (has_none_ttf) {
        logger_->Warn(
            _ST("Found non-TTF fonts. Check the warnings above. Based on ASS "
                "Specs, only Truetype fonts can be embedded in ASS scripts. "
                "Ignored this error, but these fonts may not be loaded by some "
                "video players."));
      }
    } else {
      output_ass << line;
    }

    if (num_line != text.size()) {
      output_ass << '\n';
    }
  }
}

void AssFontEmbedder::WriteFonts(bool& has_none_ttf,
                                 std::ofstream& output_ass) {
  for (const auto& font : fs_.subfonts_info_) {
    fs::path font_path(font.subfont_path);

    if (ToLower(font_path.extension().native()) != _ST(".ttf")) {
      logger_->Warn(_ST("\"{}\" is not a .ttf font."), font_path.native());
      has_none_ttf = true;
    }

    AString a_fontname =
        font_path.stem().native() + _ST("_0") + font_path.extension().native();

#ifdef _WIN32
    std::string fontname = WideToU8(a_fontname);
#else
    std::string fontname(a_fontname);
#endif

    std::ifstream is(font.subfont_path, std::ios::binary);
    std::ostringstream ostrm;
    ostrm << is.rdbuf();
    std::string font_data(ostrm.str());
    std::string uu_str =
        UUEncode(font_data.c_str(), font_data.c_str() + font_data.size(), true);
    output_ass << "\nfontname: " << fontname << '\n';
    output_ass << uu_str;
    ostrm.clear();
  }
}

bool AssFontEmbedder::WriteRenamed(AString& path,
                                   std::vector<std::string>& text) {
  fs::path input_path(fs_.ap_.get_ass_path());
  fs::path output_path(output_dir_path_ + fs::path::preferred_separator +
                       input_path.stem().native() + _ST(".rename") +
                       input_path.extension().native());
  path = output_path.native();
  std::ofstream output_ass(path);
  if (!output_ass.is_open()) {
    logger_->Error(_ST("\"{}\" cannot be created."), output_path.native());
    return false;
  }

  std::vector<std::string> font_info;
  WriteRenameInfo(font_info);
  auto text_vec = fs_.ap_.get_text();
  FontRename(text_vec);
  for (const auto& line : text_vec) {
    text.emplace_back(line.text);
  }

  for (auto iter = text.begin(); iter != text.end(); ++iter) {
    std::string tmp = Trim(ToLower(*iter));
    if (tmp == "[v4 styles]" || tmp == "[v4+ styles]") {
      text.insert(iter, font_info.begin(), font_info.end());
      break;
    }
  }

  unsigned int counter = 0;
  for (auto& line : text) {
    if (counter != 0) {
      output_ass << "\n";
    }
    output_ass << line;
    ++counter;
  }
  logger_->Info(_ST("Create font-renamed subtitle: \"{}\""),
                output_path.native());
  return true;
}

void AssFontEmbedder::Clear() {
  output_dir_path_.clear();
}

std::string AssFontEmbedder::UUEncode(const char* begin, const char* end,
                                      bool insert_linebreaks) {
  // Copyright (c) 2013, Thomas Goyne <plorkyeran@aegisub.org>
  //
  // Permission to use, copy, modify, and distribute this software for any
  // purpose with or without fee is hereby granted, provided that the above
  // copyright notice and this permission notice appear in all copies.
  //
  // THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  // WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  // MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  // ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  // WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  // ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  // OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
  //
  // Aegisub Project http://www.aegisub.org/

  size_t size = std::distance(begin, end);
  std::string ret;
  ret.reserve((size * 4 + 2) / 3 + size / 80 * 2);
  size_t written = 0;
  for (size_t pos = 0; pos < size; pos += 3) {
    unsigned char src[3] = {'\0', '\0', '\0'};
    memcpy(src, begin + pos, std::min<size_t>(3u, size - pos));
    unsigned char dst[4] = {static_cast<unsigned char>(src[0] >> 2),
                            static_cast<unsigned char>(((src[0] & 0x3) << 4) |
                                                       ((src[1] & 0xF0) >> 4)),
                            static_cast<unsigned char>(((src[1] & 0xF) << 2) |
                                                       ((src[2] & 0xC0) >> 6)),
                            static_cast<unsigned char>(src[2] & 0x3F)};
    for (size_t i = 0; i < std::min<size_t>(size - pos + 1, 4u); ++i) {
      ret += dst[i] + 33;
      if (insert_linebreaks && ++written == 80 && pos + 3 < size) {
        written = 0;
        ret += "\n";
      }
    }
  }
  return ret;
}

void AssFontEmbedder::WriteRenameInfo(std::vector<std::string>& text) {
  for (const auto& subfont_info : fs_.subfonts_info_) {
    std::unordered_set<std::string> fontname_set;
    for (const auto& font_desc : subfont_info.fonts_desc) {
      fontname_set.insert(font_desc.fontname);
    }

    for (const auto& fontname : fontname_set) {
      fontname_map_[fontname] = subfont_info.newname;
    }
  }

  text.emplace_back(std::string("[Assfonts Rename Info]"));
  for (const auto& name_info : fontname_map_) {
    text.emplace_back(
        std::string(name_info.first + " ---- " + name_info.second));
  }
  text.emplace_back(std::string(""));
}

void AssFontEmbedder::FontRename(std::vector<AssParser::TextInfo>& text) {
  auto rename_infos = fs_.ap_.get_rename_infos();
  for (auto& rename_info : rename_infos) {
    try {
      rename_info.newname = fontname_map_.at(rename_info.fontname);
    } catch (const std::out_of_range&) {
      continue;
    }
  }

  unsigned int line_num = 0;
  int offset = 0;
  for (const auto& rename_info : rename_infos) {
    if (rename_info.newname.empty()) {
      continue;
    }

    if (line_num != rename_info.line_num) {
      offset = 0;
    }

    auto check_same_line = [=](const AssParser::TextInfo& text_info) {
      return text_info.line_num == rename_info.line_num;
    };

    auto iter = std::find_if(text.begin(), text.end(), check_same_line);
    if (iter == text.end()) {
      continue;
    }
    (*iter).text.replace(rename_info.beg + offset,
                         (rename_info.end - rename_info.beg),
                         rename_info.newname);

    line_num = rename_info.line_num;
    offset = offset + rename_info.newname.size() - rename_info.fontname.size();
  }
}

}  // namespace ass