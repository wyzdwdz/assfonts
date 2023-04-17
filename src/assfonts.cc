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

#include <clocale>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string>
#include <system_error>
#include <vector>

#include <fmt/core.h>
#include <spdlog/async.h>
#include <spdlog/spdlog.h>
#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>

#include "ass_font_embedder.h"
#include "ass_parser.h"
#include "ass_string.h"
#include "fmt_sink.h"
#include "font_parser.h"
#include "font_subsetter.h"

#include "ver.h"

namespace fs = std::filesystem;

#ifdef _WIN32
int wmain(int argc, wchar_t** wargv) {
  std::vector<std::wstring> v_wstr;
  std::vector<std::string> v_str;
  auto argv = std::make_unique<char*[]>(static_cast<size_t>(argc) + 1);
  for (int i = 0; i < argc; ++i) {
    v_wstr.emplace_back(std::wstring(wargv[i]));
  }
  for (const std::wstring& wstr : v_wstr) {
    v_str.emplace_back(ass::WideToU8(wstr));
  }
  for (int i = 0; i < argc; ++i) {
    argv[i] = const_cast<char*>(v_str[i].c_str());
  }
  argv[argc] = NULL;
#else
int main(int argc, char** argv) {
#endif
  spdlog::init_thread_pool(512, 1);
  auto fmt_sink = std::make_shared<mylog::sinks::fmt_sink_mt>();
  auto logger = std::make_shared<spdlog::async_logger>("main", fmt_sink,
                                                       spdlog::thread_pool());
  spdlog::register_logger(logger);

  auto loc = std::setlocale(LC_ALL, "");
  if (loc == nullptr) {
    logger->error("Install system locale failed.");
    spdlog::shutdown();
    return 0;
  }

  ass::AssParser ap(fmt_sink);
  ass::FontParser fp(fmt_sink);
  ass::FontSubsetter fs(ap, fp, fmt_sink);
  ass::AssFontEmbedder afe(fs, fmt_sink);

  spdlog::set_pattern("[%^%l%$] %v");

  std::vector<std::string> inputs;
  std::string output;
  std::string fonts;
  std::string database(".");
  bool is_build = false;
  bool is_embed_only = false;
  bool is_subset_only = false;
  bool is_help = false;
  int verbose = 3;
  unsigned int brightness = 203;

  CLI::App app{"Subset fonts and embed them into an ASS subtitle."};
  auto* p_opt_i =
      app.add_option("-i,--input,input", inputs, "Input .ass files");
  auto* p_opt_o = app.add_option("-o,--output", output, "Output directory");
  auto* p_opt_f = app.add_option("-f,--fontpath", fonts, "Set fonts directory");
  auto* p_opt_d =
      app.add_option("-d,--dbpath", database, "Set fonts database path");
  auto* p_opt_l = app.add_option("-l,--luminance", brightness,
                                 "Set brightness for HDR contents");
  app.add_flag("-b,--build", is_build, "Build or update fonts database");
  app.add_flag("-e,--embed-only", is_embed_only, "Do not subset fonts");
  app.add_flag("-s,--subset-only", is_subset_only,
               "Subset fonts but not embed them into subtitle");
  auto* p_opt_v = app.add_option("-v,--verbose", verbose, "Set logging level.");
  app.set_help_flag("");
  app.add_flag("-h,--help", is_help, "Get help info");
  p_opt_i->type_name("<file>");
  p_opt_o->type_name("<dir>");
  p_opt_f->type_name("<dir>");
  p_opt_d->type_name("<dir>");
  p_opt_l->type_name("<num>");
  p_opt_l->expected(0, 1);
  p_opt_l->check(CLI::Range(0, 1000));
  p_opt_v->type_name("<num>");
  app.failure_message(
      [=](const CLI::App* app, const CLI::Error& e) -> std::string {
        std::string str = CLI::FailureMessage::simple(app, e);
        str.pop_back();
        str += ". See --help for more info.";
        logger->error(str);
        spdlog::shutdown();
        return "";
      });
#ifdef _WIN32
  CLI11_PARSE(app, argc, argv.get());
#else
  CLI11_PARSE(app, argc, argv);
#endif

  if (is_help) {
    // clang-format off
      fmt::print(_ST("assfonts v{}.{}.{}\n"
      "Subset fonts and embed them into an ASS subtitle.\n"
      "Usage:     assfonts [options...] [<files>]\n"
      "Examples:  assfonts <files>                  Embed subset fonts into ASS script\n"
      "           assfonts -i <files>               Same as above\n"
      "           assfonts -o <dir> -s -i <files>   Only subset fonts but not embed\n"
      "           assfonts -f <dir> -e -i <files>   Only embed fonts without subset\n"
      "           assfonts -f <dir> -b              Build or update fonts database only\n"
      "           assfonts -l <num> -i <files>      Recolorize the subtitle for HDR contents\n"
      "Options:\n"
      "  -i, --input,      <files>   Input .ass files\n"
      "  -o, --output      <dir>     Output directory  (Default: same directory as input)\n"
      "  -f, --fontpath    <dir>     Set fonts directory\n"
      "  -b, --build                 Build or update fonts database  (Require --fontpath)\n"
      "  -d, --dbpath      <dir>     Set fonts database path  (Default: current path)\n"
      "  -s, --subset-only <bool>    Subset fonts but not embed them into subtitle  (default: False)\n"
      "  -e, --embed-only  <bool>    Do not subset fonts  (default: False)\n"
      "  -l, --luminance   <num>     Set subtitle brightness for HDR contents  (default: 203)\n"
      "  -v, --verbose     <num>     Set logging level (0 to 3), 0 is off  (Default: 3)\n"
      "  -h, --help                  Get help info\n\n"), VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    // clang-format on
    if (argc == 2) {
      spdlog::shutdown();
      return 0;
    }
  }

  switch (verbose) {
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
      logger->error("Wrong verbose level. See --help for more info.");
      spdlog::shutdown();
      return 0;
  }

  if (inputs.empty()) {
    std::string empty_input;
    inputs.push_back(empty_input);
  }

  for (const auto& input : inputs) {
#ifdef _WIN32
    std::error_code ec;
    fs::path input_path = fs::absolute(ass::U8ToWide(input), ec);
    fs::path output_path = fs::absolute(ass::U8ToWide(output), ec);
    fs::path fonts_path = fs::absolute(ass::U8ToWide(fonts), ec);
    fs::path db_path = fs::absolute(ass::U8ToWide(database), ec);
#else
    std::error_code ec;
    fs::path input_path = fs::absolute(input, ec);
    fs::path output_path = fs::absolute(output, ec);
    fs::path fonts_path = fs::absolute(fonts, ec);
    fs::path db_path = fs::absolute(database, ec);
#endif

    if (!input.empty() && !fs::is_regular_file(input_path)) {
      logger->error("\"{}\" is not a file. See --help for more info.", input);
      spdlog::shutdown();
      return 0;
    }
    if (!output.empty() && !fs::is_directory(output_path)) {
      logger->error(
          "\"{}\" is not a legal directory path. See --help for more info.",
          output);
      spdlog::shutdown();
      return 0;
    }
    if (!fonts.empty() && !fs::is_directory(fonts_path)) {
      logger->error(
          "\"{}\" is not a legal directory path. See --help for more info.",
          fonts);
      spdlog::shutdown();
      return 0;
    }
    if (!fs::is_directory(db_path)) {
      logger->error(
          "\"{}\" is not a legal directory path. See --help for more info.",
          database);
      spdlog::shutdown();
      return 0;
    }
    if (is_build && fonts.empty()) {
      logger->error("No fontpath is found. See --help for more info.");
      spdlog::shutdown();
      return 0;
    }
    if (input.empty() && fonts.empty()) {
      logger->error("No input is found. See --help for more info.");
      spdlog::shutdown();
      return 0;
    }
    if (input.empty() && !is_build) {
      logger->error("Do nothing. See --help for more info.");
      return 0;
    }

    if (!fonts.empty()) {
      fp.LoadDB(db_path.native() + fs::path::preferred_separator +
                _ST("fonts.json"));
      fp.LoadFonts(fonts_path.native());
    }

    if (is_build) {
      fp.SaveDB(db_path.native() + fs::path::preferred_separator +
                _ST("fonts.json"));
    } else {
      fp.LoadDB(db_path.native() + fs::path::preferred_separator +
                _ST("fonts.json"));
    }

    if (input.empty()) {
      spdlog::shutdown();
      return 0;
    }

    if (output.empty()) {
      output_path = input_path.parent_path();
    }

    ap.set_output_dir_path(output_path.native());

    if (!p_opt_l->empty()) {
      if (!ap.Recolorize(input_path.native(), brightness)) {
        spdlog::shutdown();
        return 0;
      }
      input_path =
          fs::path(output_path.native() + fs::path::preferred_separator +
                   input_path.stem().native() + _ST(".hdr") +
                   input_path.extension().native());
    }

    if (!ap.ReadFile(input_path.native())) {
      spdlog::shutdown();
      return 0;
    }

    if (is_embed_only && is_subset_only) {
      spdlog::shutdown();
      return 0;
    }

    if (!is_embed_only) {
      fs.SetSubfontDir(output_path.native() + fs::path::preferred_separator +
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

    ap.Clear();
    fp.clean_font_list();
    fs.Clear();
    afe.Clear();
  }

  spdlog::shutdown();
  return 0;
}