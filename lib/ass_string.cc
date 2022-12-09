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

#include "ass_string.h"

#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <boost/locale.hpp>

namespace ass {

std::u32string Trim(const std::u32string& str) {
  std::string res_u8 = boost::locale::conv::utf_to_utf<char, char32_t>(str);
  std::u32string res =
      boost::locale::conv::utf_to_utf<char32_t, char>(Trim(res_u8));
  return res;
}

std::istream& SafeGetLine(std::istream& is, std::string& t) {
  t.clear();
  std::istream::sentry se(is, true);
  std::streambuf* sb = is.rdbuf();
  for (;;) {
    int c = sb->sbumpc();
    switch (c) {
      case '\n':
        return is;
      case '\r':
        if (sb->sgetc() == '\n')
          sb->sbumpc();
        return is;
      case std::streambuf::traits_type::eof():
        if (t.empty())
          is.setstate(std::ios::eofbit);
        return is;
      default:
        t += (char)c;
    }
  }
}

#ifdef _WIN32
std::wstring U8ToWide(const std::string& str) {
  std::wstring w_str;
  int w_len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(),
                                  static_cast<int>(str.size()), NULL, 0);
  w_str.resize(w_len);
  MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()),
                      &w_str[0], static_cast<int>(w_str.size()));
  return w_str;
}

std::string WideToU8(const std::wstring& w_str) {
  std::string str;
  int len =
      WideCharToMultiByte(CP_UTF8, 0, w_str.c_str(),
                          static_cast<int>(w_str.size()), NULL, 0, NULL, NULL);
  str.resize(len);
  WideCharToMultiByte(CP_UTF8, 0, w_str.c_str(), static_cast<int>(w_str.size()),
                      &str[0], static_cast<int>(str.size()), NULL, NULL);
  return str;
}
#endif

}  // namespace ass