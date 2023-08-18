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
    text = fs_.ap_.get_text();
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
  for (auto& line : text) {
    ++num_line;
    if (Trim(ToLower(line)) == "[events]") {
      output_ass << "[Fonts]";
      bool has_none_ttf = false;
      for (const auto& font : fs_.subfonts_info_) {
        fs::path font_path(font.subfont_path);
        if (ToLower(font_path.extension().native()) != _ST(".ttf")) {
          logger_->Warn(_ST("\"{}\" is not a .ttf font."), font_path.native());
          has_none_ttf = true;
        }
        AString a_fontname = font_path.stem().native() + _ST("_0") +
                             font_path.extension().native();
#ifdef _WIN32
        std::string fontname = WideToU8(a_fontname);
#else
        std::string fontname(a_fontname);
#endif
        std::ifstream is(font.subfont_path, std::ios::binary);
        std::ostringstream ostrm;
        ostrm << is.rdbuf();
        std::string font_data(ostrm.str());
        std::string uu_str = UUEncode(
            font_data.c_str(), font_data.c_str() + font_data.size(), true);
        output_ass << "\nfontname: " << fontname << '\n';
        output_ass << uu_str;
        ostrm.clear();
      }
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
  logger_->Info(_ST("Create font-embeded subtitle: \"{}\""),
                output_path.native());
  return true;
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
  RegexInit();
  WriteRenameInfo(font_info);
  text = fs_.ap_.get_text();
  for (auto& line : text) {
    FontRename(line);
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

void AssFontEmbedder::RegexInit() {
  for (const auto& subfont_info : fs_.subfonts_info_) {
    for (const auto& font_desc : subfont_info.fonts_desc) {
      jp::Regex re(font_desc.fontname, "S");
      re_list_.emplace_back(std::make_pair(re, subfont_info.newname));
    }
  }
}

void AssFontEmbedder::WriteRenameInfo(std::vector<std::string>& text) {
  text.emplace_back(std::string("[Assfonts Rename Info]"));
  for (const auto& re : re_list_) {
    text.emplace_back(
        std::string(re.first.getPattern() + " ---- " + re.second));
  }
  text.emplace_back(std::string(""));
}

void AssFontEmbedder::FontRename(std::string& line) {
  for (auto& re : re_list_) {
    line = re.first.replace(line, re.second, "g");
  }
}

}  // namespace ass