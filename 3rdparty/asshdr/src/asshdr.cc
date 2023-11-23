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

#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <CLI/CLI.hpp>
#include <nowide/args.hpp>
#include <nowide/fstream.hpp>
#include <nowide/iostream.hpp>

#include "asshdr/ass_recolorize.h"
#include "ver.h"

namespace fs = std::filesystem;

bool is_valid_utf8(const std::string& str);

int main(int argc, char** argv) {
  nowide::args _(argc, argv);

  std::vector<std::string> inputs;
  std::string output;
  unsigned int brightness = 203;
  bool is_help = false;

  CLI::App app{"Recolorize ASS subtitle for HDR contents."};
  auto* p_opt_i =
      app.add_option("-i,--input,input", inputs, "Input .ass files");
  auto* p_opt_o = app.add_option("-o,--output", output, "Output directory");
  auto* p_opt_b =
      app.add_option("-b,--brightness", brightness, "Target brightness");
  app.set_help_flag("");
  app.add_flag("-h,--help", is_help, "Get help info");
  p_opt_i->type_name("<files>");
  p_opt_o->type_name("<dir>");
  p_opt_b->type_name("<num>");
  p_opt_b->check(CLI::Range(0, 1000));
  app.failure_message(
      [=](const CLI::App* app, const CLI::Error& e) -> std::string {
        std::string str = CLI::FailureMessage::simple(app, e);
        str.pop_back();
        str.append(". See --help for more info.");
        nowide::cout << "[ERROR] " << str << std::endl;
        return "";
      });
  CLI11_PARSE(app, argc, argv);

  if (is_help) {
    // clang-format off
    nowide::cout 
    << "asshdr v" << VERSION_MAJOR << '.' << VERSION_MINOR << '.' << VERSION_PATCH << '\n'
    << "Recolorize ASS subtitle for HDR contents.\n"
    << "Usage:     asshdr [options...] [<files>]\n"
    << "Examples:  asshdr <files>\n"
    << "           asshdr -i <files>\n"
    << "           asshdr -o <dir> -i <files>\n"
    << "           asshdr -b <num> -o <dir> -i <files>\n"
    << "Options:\n"
    << "  -i, --input,       <files>           Input .ass files\n"
    << "  -o, --output       <dir>             Output directory\n"
    << "                                      (Default: same directory as input)\n"
    << "  -b, --brightness   <num (0-1000)>    Target brightness for recoloring\n"  
    << "                                      (Default: 203)\n"
    << "  -h, --help                           Get help info\n" << std::endl;
    // clang-format on
  }

  if (!is_help && inputs.empty()) {
    nowide::cout << "[ERROR] --input is required. See --help for more info."
                 << std::endl;
    return 0;
  }

  fs::path output_path = fs::u8path(output);
  if (!output.empty() && !fs::is_directory(output_path)) {
    nowide::cout << '\"' << output << '\"'
                 << " is not a legal directory path. See --help for more info."
                 << std::endl;
    return 0;
  }

  for (const std::string& input : inputs) {
    fs::path input_path = fs::u8path(input);
    if (!input.empty() && !fs::is_regular_file(input_path)) {
      nowide::cout << "[ERROR] \"" << input << '\"'
                   << " is not a file. See --help for more info." << std::endl;
      continue;
    }
    if (!fs::is_directory(output_path)) {
      output_path = input_path.parent_path();
    }
    nowide::ifstream is(input_path.u8string());
    std::stringstream sstream;
    sstream << is.rdbuf();
    std::string ass_text(sstream.str());

    if (!is_valid_utf8(ass_text)) {
      nowide::cout << "[ERROR] \"" << input << "\" must be UTF-8 encoded."
                   << std::endl;
      continue;
    }

    unsigned int out_size = ass_text.size() * 2;
    std::unique_ptr<char[]> out_text(new char[out_size]);
    asshdr::AssRecolor(ass_text.c_str(), ass_text.size(), out_text.get(),
                       out_size, brightness);

    fs::path output_file_path =
        fs::u8path(output_path.u8string() + '/' + input_path.stem().u8string() +
                   ".hdr" + input_path.extension().u8string());
    nowide::ofstream os(output_file_path.u8string());
    os << std::string(out_text.get(), out_size);
    nowide::cout << "[INFO] Output file has been saved in \""
                 << output_file_path.make_preferred().u8string() << "\""
                 << std::endl;
  }

  return 0;
}

bool is_valid_utf8(const std::string& str) {
  if (str.empty()) {
    return true;
  }

  const unsigned char* bytes =
      reinterpret_cast<const unsigned char*>(str.c_str());
  const unsigned char* const bytes_end = bytes + str.size();
  unsigned int cp;
  int num;

  while (bytes != bytes_end) {
    if ((*bytes & 0x80) == 0x00) {
      // U+0000 to U+007F
      cp = (*bytes & 0x7F);
      num = 1;
    } else if ((*bytes & 0xE0) == 0xC0) {
      // U+0080 to U+07FF
      cp = (*bytes & 0x1F);
      num = 2;
    } else if ((*bytes & 0xF0) == 0xE0) {
      // U+0800 to U+FFFF
      cp = (*bytes & 0x0F);
      num = 3;
    } else if ((*bytes & 0xF8) == 0xF0) {
      // U+10000 to U+10FFFF
      cp = (*bytes & 0x07);
      num = 4;
    } else {
      return false;
    }

    bytes += 1;
    for (int i = 1; i < num; ++i) {
      if ((*bytes & 0xC0) != 0x80) {
        return false;
      }
      cp = (cp << 6) | (*bytes & 0x3F);
      bytes += 1;
    }

    if ((cp > 0x10FFFF) || ((cp >= 0xD800) && (cp <= 0xDFFF)) ||
        ((cp <= 0x007F) && (num != 1)) ||
        ((cp >= 0x0080) && (cp <= 0x07FF) && (num != 2)) ||
        ((cp >= 0x0800) && (cp <= 0xFFFF) && (num != 3)) ||
        ((cp >= 0x10000) && (cp <= 0x1FFFFF) && (num != 4))) {
      return false;
    }
  }

  return true;
}
