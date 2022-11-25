#include "ass_font_embedder.h"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/locale.hpp>

namespace fs = boost::filesystem;

namespace ass {

void AssFontEmbedder::set_input_ass_path(const std::string& input_ass_path) {
  input_ass_path_ = input_ass_path;
}

void AssFontEmbedder::set_output_ass_path(const std::string& output_ass_path) {
  output_ass_path_ = output_ass_path;
}

void AssFontEmbedder::Run() {
  fs::path input_path(input_ass_path_);
  fs::path output_path(output_ass_path_);
  std::ifstream input_ass(input_path.generic_string());
  std::ofstream output_ass(output_path.generic_string());
  std::string line;
  if (!input_ass.is_open()) {
    logger_->error("\"{}\" cannot be opened.", input_path.generic_string());
    return;
  }
  if (!output_ass.is_open()) {
    logger_->error("\"{}\" cannot be created.", output_path.generic_string());
    return;
  }
  while (getline(input_ass, line)) {
    if (boost::algorithm::trim_copy(boost::algorithm::to_lower_copy(line)) ==
        "[events]") {
      output_ass << "[Fonts]";
      for (const auto& font : fs_.subfonts_path_) {
        fs::path font_path(font);
        if (boost::algorithm::to_lower_copy(font_path.extension().string()) !=
            ".ttf") {
          logger_->warn(
              "\"{}\" is not a .ttf font. Based on ASS Specs, only Truetype "
              "fonts can be embedded in ASS scripts. Ignore this error, but "
              "this font may not be loaded by some video players.",
              font_path.generic_string());
        }
        std::wstring w_fontname = fs::path(font_path).stem().wstring() + L"_0" +
                                  fs::path(font_path).extension().wstring();
        std::string fontname =
            boost::locale::conv::utf_to_utf<char, wchar_t>(w_fontname);
        std::ifstream font(font, std::ios::binary);
        std::ostringstream ostrm;
        ostrm << font.rdbuf();
        std::string font_data(ostrm.str());
        std::string uu_str = UUEncode(
            font_data.c_str(), font_data.c_str() + font_data.size(), true);
        output_ass << "\nfontname: " << fontname << "\n";
        output_ass << uu_str;
        font.close();
      }
      output_ass << "\n\n" << line << "\n";
    } else {
      output_ass << line << "\n";
    }
  }
  input_ass.close();
  output_ass.close();
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

}  // namespace ass