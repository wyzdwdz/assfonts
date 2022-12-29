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

#include <exception>

#include <fmt/core.h>
#include <spdlog/async.h>
#include <spdlog/spdlog.h>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "ass_font_embedder.h"
#include "ass_parser.h"
#include "ass_string.h"
#include "fmt_sink.h"
#include "font_parser.h"
#include "font_subsetter.h"

#include "ver.h"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

#ifdef _WIN32
#define PoStringValue po::wvalue<AString>
#else
#define PoStringValue po::value<AString>
#endif

#ifdef _WIN32
int wmain(int argc, wchar_t** argv) {
#else
int main(int argc, char** argv) {
#endif
  spdlog::init_thread_pool(512, 1);
  auto fmt_sink = std::make_shared<mylog::sinks::fmt_sink_mt>();
  auto logger = std::make_shared<spdlog::async_logger>("main", fmt_sink,
                                                       spdlog::thread_pool());
  spdlog::register_logger(logger);

  ass::AssParser ap(fmt_sink);
  ass::FontParser fp(fmt_sink);
  ass::FontSubsetter fs(ap, fp, fmt_sink);
  ass::AssFontEmbedder afe(fs, fmt_sink);

  spdlog::set_pattern("[%^%l%$] %v");

  po::options_description desc("all options");
  // clang-format off
  desc.add_options()
    ("input,i",       PoStringValue(),  "Input .ass file")
    ("output,o",      PoStringValue(),  "Output directory")
    ("fontpath,f",    PoStringValue(),  "Set fonts directory")
    ("dbpath,d",      PoStringValue(),  "Set fonts database path")
    ("build,b",                         "Build or update fonts database")
    ("embed-only,e",  po::bool_switch()->default_value(false),                 
                                        "Do not subset fonts")
    ("subset-only,s", po::bool_switch()->default_value(false),               
                                        "Subset fonts but not embed them into subtitle")
    ("verbose,v",     po::value<int>(), "Set logging level.")
    ("help,h",                          "Get help info");
  // clang-format on
  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
  } catch (const po::error& e) {
#ifdef _WIN32
    logger->error(_ST("{}. See --help for more info."),
                  ass::U8ToWide(e.what()));
#else
    logger->error(_ST("{}. See --help for more info."), e.what());
#endif
    spdlog::shutdown();
    return 0;
  }
  po::notify(vm);
  if (vm.count("help")) {
    // clang-format off
    fmt::print(_ST("assfonts v{}.{}.{}\n"
      "Subset fonts and embed them into an ASS subtitle.\n"
      "Usage:     assfonts [options...] [<file>]\n"
      "Examples:  assfonts <file>                  Embed subset fonts into ASS script\n"
      "           assfonts -i <file>               Same as above\n"
      "           assfonts -o <dir> -s -i <file>   Only subset fonts but not embed\n"
      "           assfonts -f <dir> -e -i <file>   Only embed fonts without subset\n"
      "           assfonts -f <dir> -b             Build or update fonts database only\n"
      "Options:\n"
      "  -i, --input,      <file>   Input .ass file\n"
      "  -o, --output      <dir>    Output directory  (Default: same directory as input)\n"
      "  -f, --fontpath    <dir>    Set fonts directory\n"
      "  -b, --build                Build or update fonts database  (Require --fontpath)\n"
      "  -d, --dbpath      <dir>    Set fonts database path  (Default: current path)\n"
      "  -s, --subset-only <bool>   Subset fonts but not embed them into subtitle  (default: False)\n"
      "  -e, --embed-only  <bool>   Do not subset fonts  (default: False)\n"
      "  -v, --verbose     <num>    Set logging level (0 to 3), 0 is off  (Default: 3)\n"
      "  -h, --help                 Get help info\n\n"), VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    // clang-format on
    if (argc == 2) {
      spdlog::shutdown();
      return 0;
    }
  }
  if (vm.count("verbose")) {
    switch (vm["verbose"].as<int>()) {
      case 0:
        spdlog::set_level(spdlog::level::off);
        break;
      case 1:
        spdlog::set_level(spdlog::level::err);
        break;
      case 2:
        spdlog::set_level(spdlog::level::warn);
        break;
      case 3:
        spdlog::set_level(spdlog::level::info);
        break;
      default:
        logger->error(_ST("Wrong verbose level. See --help for more info."));
        spdlog::shutdown();
        return 0;
    }
  }

  AString input;
  AString output;
  AString fonts;
  AString database(_ST("."));
  bool is_build = false;
  bool is_subset_only = false;
  bool is_embed_only = false;
  if (vm.count("input")) {
    input = vm["input"].as<AString>();
  }
  if (vm.count("output")) {
    output = vm["output"].as<AString>();
  }
  if (vm.count("fontpath")) {
    fonts = vm["fontpath"].as<AString>();
  }
  if (vm.count("dbpath")) {
    database = vm["dbpath"].as<AString>();
  }
  if (vm.count("build")) {
    is_build = true;
  }
  if (vm.count("subset-only")) {
    is_subset_only = vm["subset-only"].as<bool>();
  }
  if (vm.count("embed-only")) {
    is_embed_only = vm["embed-only"].as<bool>();
  }
  fs::path input_path = fs::system_complete(input);
  fs::path output_path = fs::system_complete(output);
  fs::path fonts_path = fs::system_complete(fonts);
  fs::path db_path = fs::system_complete(database);

  if (!input.empty() && !fs::is_regular_file(input_path)) {
    logger->error(_ST("\"{}\" is not a file. See --help for more info."),
                  input);
    spdlog::shutdown();
    return 0;
  }
  if (!output.empty() && !fs::is_directory(output_path)) {
    logger->error(
        _ST("\"{}\" is not a legal directory path. See --help for more info."),
        output);
    spdlog::shutdown();
    return 0;
  }
  if (!fonts.empty() && !fs::is_directory(fonts_path)) {
    logger->error(
        _ST("\"{}\" is not a legal directory path. See --help for more info."),
        fonts);
    spdlog::shutdown();
    return 0;
  }
  if (!fs::is_directory(db_path)) {
    logger->error(
        _ST("\"{}\" is not a legal directory path. See --help for more info."),
        database);
    spdlog::shutdown();
    return 0;
  }
  if (is_build && fonts.empty()) {
    logger->error(_ST("No fontpath is found. See --help for more info."));
    spdlog::shutdown();
    return 0;
  }
  if (input.empty() && fonts.empty()) {
    logger->error(_ST("No input is found. See --help for more info."));
    spdlog::shutdown();
    return 0;
  }
  if (input.empty() && !is_build) {
    logger->error(_ST("Do nothing. See --help for more info."));
    spdlog::shutdown();
    return 0;
  }

  if (!fonts.empty()) {
    fp.LoadFonts(fonts_path.native());
  }

  if (is_build) {
    fp.SaveDB(db_path.native() + _ST("/fonts.db"));
  } else {
    fp.LoadDB(db_path.native() + _ST("/fonts.db"));
  }

  if (input.empty()) {
    spdlog::shutdown();
    return 0;
  }

  ap.set_output_dir_path(output_path.native());
  if (!ap.ReadFile(input_path.native())) {
    spdlog::shutdown();
    return 0;
  }

  if (is_embed_only && is_subset_only) {
    spdlog::shutdown();
    return 0;
  }

  if (output.empty()) {
    output_path = input_path.parent_path();
  }

  if (!is_embed_only) {
    fs.SetSubfontDir(output_path.native() + _ST("/") +
                     input_path.stem().native() + _ST("_subsetted"));
  }
  if (!fs.Run(is_embed_only)) {
    spdlog::shutdown();
    return 0;
  }

  if (!is_subset_only) {
    afe.set_output_dir_path(output_path.native());
    if (!afe.Run()) {
      spdlog::shutdown();
      return 0;
    }
  }

  spdlog::shutdown();
  return 0;
}