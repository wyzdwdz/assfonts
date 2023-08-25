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

#ifndef ASSFONTS_ASSSTRING_H_
#define ASSFONTS_ASSSTRING_H_

#include <iostream>
#include <string>

#include <nonstd/string_view.hpp>

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

enum class endian {
#ifdef _WIN32
  little = 0,
  big = 1,
  native = little
#else
  little = __ORDER_LITTLE_ENDIAN__,
  big = __ORDER_BIG_ENDIAN__,
  native = __BYTE_ORDER__
#endif
};

std::string Trim(const std::string& str);
nonstd::string_view Trim(const nonstd::string_view sv);

std::string ToLower(const std::string& str);
std::wstring ToLower(const std::wstring& str);

AString ToAString(const long i);

std::istream& SafeGetLine(std::istream& is, std::string& t);

bool IconvConvert(const std::string& in, std::string& out,
                  const std::string& from_code, const std::string& to_code);

std::u32string U8ToU32(const std::string& str_u8);
std::string U32ToU8(const std::u32string& str_u32);

std::wstring U8ToWide(const std::string& str);
std::string WideToU8(const std::wstring& str);

int StringToInt(const std::string& str);

}  // namespace ass

#endif
