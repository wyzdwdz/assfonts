/*  This file is part of asshdr.
 *
 *  asshdr is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation,
 *  either version 2 of the License,
 *  or (at your option) any later version.
 *
 *  asshdr is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with asshdr. If not, see <https://www.gnu.org/licenses/>.
 *  
 *  written by wyzdwdz (https://github.com/wyzdwdz)
 */

#include "asshdr/ass_recolorize.h"

#include <array>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <ios>
#include <sstream>
#include <string>

#include "jpcre2.hpp"

using jp = jpcre2::select<char>;

static jp::Regex r_style_color(
    R"((?<prefix>&H[0-9a-fA-F]{2})(?<color>[0-9a-fA-F]{6}))", "uS");
static jp::Regex r_event_color(
    R"((?<prefix>\\[1-4]?c&?H?)(?<color>[0-9a-fA-F]{2,})(?<suffix>&?))", "uS");
static jp::Regex r_style_title(R"(^\s*\[v4\+? Styles\]\s*$)", "iuS");
static jp::Regex r_event_title(R"(^\s*\[Events\]\s*$)", "iuS");
static jp::Regex r_title(R"(^\s*\[.*\]\s*$)", "uS");

static unsigned int brightness = 0;

namespace asshdr {

std::istream& SafeGetLine(std::istream& is, std::string& t);

void StyleRecolor(std::string& line);

void DialogueRecolor(std::string& line);

jp::String style_callback(void*, const jp::MapNas& substr_map, void*);

jp::String event_callback(void*, const jp::MapNas& substr_map, void*);

std::array<unsigned int, 3> RecolorHex(const std::string& red_hex,
                                       const std::string& green_hex,
                                       const std::string& blue_hex);

std::array<unsigned int, 3> Recolor(const unsigned int& red,
                                    const unsigned int& green,
                                    const unsigned int& blue);

std::array<std::string, 3> IntToHex(const std::array<unsigned int, 3>& int_arr);

double Spow(const double& a, const double& b);

std::array<double, 3> SrgbOetfInverse(const std::array<double, 3>& rgb);

std::array<double, 3> SrgbToXyz(const std::array<double, 3>& rgb);

std::array<double, 3> XyzToXyy(const std::array<double, 3>& xyz);

std::array<double, 3> XyyToXyz(const std::array<double, 3>& xyy);

std::array<double, 3> XyzToBt2100(const std::array<double, 3>& rgb);

std::array<double, 3> Bt2100Oetf(const std::array<double, 3>& rgb);

bool AssRecolor(const char* input_str, const unsigned int& input_size,
                char* output_str, unsigned int& output_size,
                const unsigned int input_brightness) {
  if (!r_style_color || !r_event_color || !r_style_title || !r_event_title ||
      !r_title) {
    return false;
  }

  const std::string input_string(input_str, input_size);
  std::string output_string;
  std::istringstream is(input_string);
  std::string line;

  brightness = input_brightness;
  if (brightness < 0 || brightness > 1000) {
    return false;
  }

  if (!SafeGetLine(is, line)) {
    return false;
  }

  while (true) {
    if (r_style_title.match(line)) {
      output_string += line + '\n';
      while (SafeGetLine(is, line)) {
        if (r_title.match(line)) {
          break;
        }
        StyleRecolor(line);
        output_string += line + '\n';
      }
    }

    if (r_event_title.match(line)) {
      output_string += line + '\n';
      while (SafeGetLine(is, line)) {
        if (r_title.match(line)) {
          break;
        }
        DialogueRecolor(line);
        output_string += line + '\n';
      }
    }

    output_string += line + '\n';

    if (!SafeGetLine(is, line)) {
      break;
    }
  }

  std::string input_endblank;
  for (auto it = input_string.end() - 1; it > input_string.begin(); --it) {
    if (*it == '\n' || *it == '\r' || *it == ' ') {
      input_endblank.insert(input_endblank.begin(), *it);
    } else {
      break;
    }
  }
  for (auto it = output_string.end() - 1; it > output_string.begin(); --it) {
    if (*it == '\n' || *it == '\r' || *it == ' ') {
      output_string.pop_back();
    } else {
      break;
    }
  }
  output_string.append(input_endblank);

  if (output_size < output_string.size()) {
    return false;
  }

  std::memmove(output_str, output_string.c_str(), output_string.size());
  output_size = output_string.size();
  return true;
}

std::istream& SafeGetLine(std::istream& is, std::string& t) {
  t.clear();
  std::istream::sentry se(is, true);
  std::streambuf* sb = is.rdbuf();
  while (true) {
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
        t += static_cast<char>(c);
    }
  }
}

void StyleRecolor(std::string& line) {
  std::string orig = line;
  line = jp::MatchEvaluator(style_callback)
             .setSubject(&orig)
             .setRegexObject(&r_style_color)
             .setModifier("g")
             .replace();
}

void DialogueRecolor(std::string& line) {
  std::string orig = line;
  line = jp::MatchEvaluator(event_callback)
             .setSubject(&orig)
             .setRegexObject(&r_event_color)
             .setModifier("g")
             .replace();
}

jp::String style_callback(void*, const jp::MapNas& substr_map, void*) {
  const std::string blue_hex = substr_map.at("color").substr(0, 2);
  const std::string green_hex = substr_map.at("color").substr(2, 2);
  const std::string red_hex = substr_map.at("color").substr(4, 2);
  std::array<unsigned int, 3> rgb_recolored =
      RecolorHex(red_hex, green_hex, blue_hex);
  std::array<std::string, 3> rgbhex_recolored = IntToHex(rgb_recolored);
  return substr_map.at("prefix") + rgbhex_recolored[2] + rgbhex_recolored[1] +
         rgbhex_recolored[0];
}

jp::String event_callback(void*, const jp::MapNas& substr_map, void*) {
  std::string color_str = substr_map.at("color");
  for (int i = 6 - color_str.size(); i > 0; --i) {
    color_str.push_back('0');
  }
  const std::string blue_hex = substr_map.at("color").substr(0, 2);
  const std::string green_hex = substr_map.at("color").substr(2, 2);
  const std::string red_hex = substr_map.at("color").substr(4, 2);
  std::array<unsigned int, 3> rgb_recolored =
      RecolorHex(red_hex, green_hex, blue_hex);
  std::array<std::string, 3> rgbhex_recolored = IntToHex(rgb_recolored);
  return substr_map.at("prefix") + rgbhex_recolored[2] + rgbhex_recolored[1] +
         rgbhex_recolored[0] + substr_map.at("suffix");
}

std::array<unsigned int, 3> RecolorHex(const std::string& red_hex,
                                       const std::string& green_hex,
                                       const std::string& blue_hex) {
  std::array<unsigned int, 3> res = {0, 0, 0};
  unsigned int blue =
      static_cast<unsigned int>(std::strtol(blue_hex.c_str(), nullptr, 16));
  unsigned int green =
      static_cast<unsigned int>(std::strtol(green_hex.c_str(), nullptr, 16));
  unsigned int red =
      static_cast<unsigned int>(std::strtol(red_hex.c_str(), nullptr, 16));
  res = Recolor(red, green, blue);
  return res;
}

std::array<unsigned int, 3> Recolor(const unsigned int& red,
                                    const unsigned int& green,
                                    const unsigned int& blue) {
  std::array<unsigned int, 3> res = {0, 0, 0};
  std::array<double, 3> rgb_norm;
  rgb_norm[0] = red / 255.0;
  rgb_norm[1] = green / 255.0;
  rgb_norm[2] = blue / 255.0;
  std::array<double, 3> rgb_linear = SrgbOetfInverse(rgb_norm);
  std::array<double, 3> xyz = SrgbToXyz(rgb_linear);
  std::array<double, 3> xyy = XyzToXyy(xyz);
  xyy[2] = xyy[2] * brightness / 1000.0;
  xyz = XyyToXyz(xyy);
  rgb_linear = XyzToBt2100(xyz);
  rgb_norm = Bt2100Oetf(rgb_linear);
  res[0] = std::round(rgb_norm[0] * 255.0);
  res[1] = std::round(rgb_norm[1] * 255.0);
  res[2] = std::round(rgb_norm[2] * 255.0);
  return res;
}

std::array<std::string, 3> IntToHex(
    const std::array<unsigned int, 3>& int_arr) {
  std::array<std::string, 3> res;
  for (size_t i = 0; i < int_arr.size(); ++i) {
    std::stringstream sstream;
    sstream << std::uppercase << std::setfill('0') << std::setw(2) << std::hex
            << int_arr[i];
    res[i] = sstream.str();
  }
  return res;
}

double Spow(const double& a, const double& b) {
  double res = 0.0;
  if (a > 0.0) {
    res = std::pow(std::abs(a), b);
  }
  if (a == 0.0) {
    res = 0.0;
  }
  if (a < 0.0) {
    res = -1.0 * std::pow(std::abs(a), b);
  }
  return res;
}

std::array<double, 3> SrgbOetfInverse(const std::array<double, 3>& rgb) {
  std::array<double, 3> res = {0.0, 0.0, 0.0};
  for (size_t i = 0; i < rgb.size(); ++i) {
    if (rgb[i] < 1.099 * Spow(0.018, 0.45) - 0.099) {
      res[i] = rgb[i] / 4.5;
    } else {
      res[i] = Spow((rgb[i] + 0.099) / 1.099, 1.0 / 0.45);
    }
  }
  return res;
}

std::array<double, 3> SrgbToXyz(const std::array<double, 3>& rgb) {
  std::array<double, 3> res = {0.0, 0.0, 0.0};
  res[0] = 0.4360413 * rgb[0] + 0.3851129 * rgb[1] + 0.1430458 * rgb[2];
  res[1] = 0.2224845 * rgb[0] + 0.7169051 * rgb[1] + 0.0606104 * rgb[2];
  res[2] = 0.0139202 * rgb[0] + 0.0970672 * rgb[1] + 0.7139126 * rgb[2];
  return res;
}

std::array<double, 3> XyzToXyy(const std::array<double, 3>& xyz) {
  std::array<double, 3> res = {0.0, 0.0, 0.0};
  res[0] = xyz[0] / (xyz[0] + xyz[1] + xyz[2]);
  res[1] = xyz[1] / (xyz[0] + xyz[1] + xyz[2]);
  res[2] = xyz[1];
  return res;
}

std::array<double, 3> XyyToXyz(const std::array<double, 3>& xyy) {
  std::array<double, 3> res = {0.0, 0.0, 0.0};
  res[0] = xyy[0] * xyy[2] / xyy[1];
  res[1] = xyy[2];
  res[2] = (1.0 - xyy[0] - xyy[1]) * xyy[2] / xyy[1];
  return res;
}

std::array<double, 3> XyzToBt2100(const std::array<double, 3>& xyz) {
  std::array<double, 3> res = {0.0, 0.0, 0.0};
  res[0] = 1.6472324 * xyz[0] + -0.3936125 * xyz[1] + -0.2359668 * xyz[2];
  res[1] = -0.6826173 * xyz[0] + 1.6476124 * xyz[1] + 0.0128103 * xyz[2];
  res[2] = 0.0296815 * xyz[0] + -0.0629493 * xyz[1] + 1.2538858 * xyz[2];
  return res;
}

std::array<double, 3> Bt2100Oetf(const std::array<double, 3>& rgb) {
  std::array<double, 3> res = rgb;
  double c_1 = 3424.0 / 4096.0;
  double c_2 = 2413.0 / 4096.0 * 32.0;
  double c_3 = 2392.0 / 4096.0 * 32.0;
  double m_1 = 2610.0 / 4096.0 * (1.0 / 4.0);
  double m_2 = 2523.0 / 4096.0 * 128.0;
  for (size_t i = 0; i < res.size(); ++i) {
    res[i] = 59.5208 * res[i];
    if (res[i] < 0.018) {
      res[i] = res[i] * 4.5;
    } else {
      res[i] = 1.099 * Spow(res[i], 0.45) - 0.099;
    }
    res[i] = 100 * std::pow(std::max(res[i], 0.0), 2.4);
    double Y_p = Spow(res[i] / 10000.0, m_1);
    res[i] = Spow((c_1 + c_2 * Y_p) / (c_3 * Y_p + 1), m_2);
  }
  return res;
}

}  // namespace asshdr