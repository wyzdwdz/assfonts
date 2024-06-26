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

#include "ass_string.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cwctype>
#include <exception>
#include <memory>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "iconv_wrapper.hh"

#include "ass_utf8.h"

namespace ass {

std::string Trim(const std::string& str) {
  std::string res = str;

  res.erase(res.begin(),
            std::find_if(res.begin(), res.end(),
                         [](unsigned char ch) { return !std::isspace(ch); }));

  res.erase(std::find_if(res.rbegin(), res.rend(),
                         [](unsigned char ch) { return !std::isspace(ch); })
                .base(),
            res.end());

  return res;
}

nonstd::string_view Trim(const nonstd::string_view sv) {
  auto first = std::find_if(sv.begin(), sv.end(),
                            [](unsigned char ch) { return !std::isspace(ch); });

  auto last = std::find_if(sv.rbegin(), sv.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
              }).base();

  if (first == sv.end()) {
    return "";
  }

  return nonstd::string_view(&(*first), last - first);
}

std::string ToLower(const std::string& str) {
  std::string res = str;
  StrToLwrExt(reinterpret_cast<unsigned char*>(&res[0]));
  return res;
}

std::wstring ToLower(const std::wstring& str) {
  std::wstring res = str;
  std::transform(res.begin(), res.end(), res.begin(),
                 [](wchar_t c) { return std::towlower(c); });
  return res;
}

AString ToAString(const long i) {
#ifdef _WIN32
  return std::to_wstring(i);
#else
  return std::to_string(i);
#endif
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

bool IconvConvert(const std::string& in, std::string& out,
                  const std::string& from_code, const std::string& to_code) {
  iconv_wrapper::iconv cvt;

  try {
    cvt.open(from_code, to_code);
  } catch (std::system_error&) {
    return false;
  }

  try {
    out = cvt.convert(in);
  } catch (std::system_error&) {
    return false;
  }

  return true;
}

std::u32string U8ToU32(const std::string& str_u8) {
  std::u32string str_u32;
  std::string str_u32_narrow;

  IconvConvert(str_u8, str_u32_narrow, "UTF-8", "UTF-32BE");

  str_u32.resize(str_u32_narrow.size() / 4);

  for (size_t i = 0; i < str_u32.size(); ++i) {
    if (endian::native == endian::big) {
      str_u32[i] =
          (static_cast<unsigned char>(str_u32_narrow[i * 4]) << 0) |
          (static_cast<unsigned char>(str_u32_narrow[i * 4 + 1]) << 8) |
          (static_cast<unsigned char>(str_u32_narrow[i * 4 + 2]) << 16) |
          (static_cast<unsigned char>(str_u32_narrow[i * 4 + 3]) << 24);

    } else {
      str_u32[i] =
          (static_cast<unsigned char>(str_u32_narrow[i * 4 + 3]) << 0) |
          (static_cast<unsigned char>(str_u32_narrow[i * 4 + 2]) << 8) |
          (static_cast<unsigned char>(str_u32_narrow[i * 4 + 1]) << 16) |
          (static_cast<unsigned char>(str_u32_narrow[i * 4 + 0]) << 24);
    }
  }

  return str_u32;
}

std::string U32ToU8(const std::u32string& str_u32) {
  std::string str_u8;
  std::string str_u32_narrow;

  str_u32_narrow.resize(str_u32.size() * 4);

  for (size_t i = 0; i < str_u32.size(); ++i) {
    if (endian::native == endian::big) {
      str_u32_narrow[i * 4] =
          static_cast<char>(static_cast<uint32_t>(str_u32[i]) >> 0);
      str_u32_narrow[i * 4 + 1] =
          static_cast<char>(static_cast<uint32_t>(str_u32[i]) >> 8);
      str_u32_narrow[i * 4 + 2] =
          static_cast<char>(static_cast<uint32_t>(str_u32[i]) >> 16);
      str_u32_narrow[i * 4 + 3] =
          static_cast<char>(static_cast<uint32_t>(str_u32[i]) >> 24);

    } else {
      str_u32_narrow[i * 4 + 3] =
          static_cast<char>(static_cast<uint32_t>(str_u32[i]) >> 0);
      str_u32_narrow[i * 4 + 2] =
          static_cast<char>(static_cast<uint32_t>(str_u32[i]) >> 8);
      str_u32_narrow[i * 4 + 1] =
          static_cast<char>(static_cast<uint32_t>(str_u32[i]) >> 16);
      str_u32_narrow[i * 4] =
          static_cast<char>(static_cast<uint32_t>(str_u32[i]) >> 24);
    }
  }

  IconvConvert(str_u32_narrow, str_u8, "UTF-32BE", "UTF-8");

  return str_u8;
}

std::wstring U8ToWide(const std::string& str) {
#ifdef _WIN32
  std::wstring w_str;
  auto w_len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(),
                                   static_cast<int>(str.size()), NULL, 0);
  w_str.resize(w_len);
  MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()),
                      const_cast<wchar_t*>(w_str.c_str()),
                      static_cast<int>(w_str.size()));
  return w_str;
#else
  std::wstring w_str;
  int offset = 0;
  size_t index = 0;

  for (wchar_t wc; (offset = std::mbtowc(&wc, str.c_str() + index,
                                         str.length() - index)) > 0;
       index += offset) {
    w_str.push_back(wc);
  }

  return w_str;
#endif
}

std::string WideToU8(const std::wstring& w_str) {
#ifdef _WIN32
  std::string str;
  auto len =
      WideCharToMultiByte(CP_UTF8, 0, w_str.c_str(),
                          static_cast<int>(w_str.size()), NULL, 0, NULL, NULL);
  str.resize(len);
  WideCharToMultiByte(CP_UTF8, 0, w_str.c_str(), static_cast<int>(w_str.size()),
                      const_cast<char*>(str.c_str()),
                      static_cast<int>(str.size()), NULL, NULL);
  return str;
#else
  std::string str;

  for (const auto wc : w_str) {
    char c[MB_CUR_MAX];
    int len = std::wctomb(c, wc);

    if (len > 0) {
      str.append(std::string(c, len));
    };
  }

  return str;
#endif
}

int StringToInt(const std::string& str) {
  int res = 0;

  try {
    res = std::stoi(str);
  } catch (const std::exception&) {
    res = 0;
  }

  return res;
}

}  // namespace ass