#ifndef ASSFONTS_ASSSTRING_H_
#define ASSFONTS_ASSSTRING_H_

#include <algorithm>
#include <cctype>
#include <cwctype>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/locale.hpp>

#ifdef _WIN32

using AChar = wchar_t;
using AString = std::wstring;
#define _ST(A) L##A

#else

using AChar = char;
using AString = std::string;
#define _ST(A) A

#endif

namespace ass {

inline std::string Trim(const std::string& str) {
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

inline std::u32string Trim(const std::u32string& str) {
  std::string res_u8 = boost::locale::conv::utf_to_utf<char, char32_t>(str);
  std::u32string res =
      boost::locale::conv::utf_to_utf<char32_t, char>(Trim(res_u8));
  return res;
}

inline std::string ToLower(const std::string& str) {
  std::string res = str;
  std::transform(res.begin(), res.end(), res.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return res;
}

inline std::wstring ToLower(const std::wstring& str) {
  std::wstring res = str;
  std::transform(res.begin(), res.end(), res.begin(),
                 [](wchar_t c) { return std::towlower(c); });
  return res;
}

inline AString ToAString(const long i) {
#ifdef _WIN32
  return std::to_wstring(i);
#else
  return std::to_string(i);
#endif
}

std::istream& SafeGetLine(std::istream& is, std::string& t);

#ifdef _WIN32
std::wstring U8ToWide(const std::string& str);
#endif

}  // namespace ass

#endif
