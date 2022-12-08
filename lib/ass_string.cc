#include "ass_string.h"

#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace ass {

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
#endif

}  // namespace ass