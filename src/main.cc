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

#include <cstdlib>
#include <memory>
#include <string>

#include <fmt/core.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "ass_font_embedder.h"
#include "ass_parser.h"
#include "error_proxy_sink.h"
#include "font_parser.h"
#include "font_subsetter.h"

namespace fs = boost::filesystem;

int main(int argc, char** argv) {
  auto color_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto error_sink =
      std::make_shared<mylog::sinks::error_proxy_sink_mt>(color_sink);
  auto logger = std::make_shared<spdlog::logger>("main", error_sink);
  spdlog::register_logger(logger);

  ass::AssParser ap;
  ass::FontParser fp;
  ass::FontSubsetter fs(ap, fp);
  ass::AssFontEmbedder afe(fs);

  spdlog::set_pattern("[%n] [%^%l%$] %v");

  std::string input;
  std::string output;
  std::string fonts;
  std::string database(".");
  bool is_build = false;
  bool is_no_subset = false;
  bool is_subset_only = false;
  bool is_help = false;
  int verbose = 3;

  CLI::App app{"Subset fonts and embed them into an ASS subtitle."};
  auto* p_opt_i = app.add_option("-i,--input,input", input, "Input .ass file");
  auto* p_opt_o = app.add_option("-o,--output", output, "Output directory");
  auto* p_opt_f = app.add_option("-f,--fontpath", fonts, "Set fonts directory");
  auto* p_opt_d =
      app.add_option("-d,--dbpath", database, "Set fonts database path");
  auto* p_opt_b =
      app.add_flag("-b,--build", is_build, "Build or update fonts database");
  auto* p_opt_n =
      app.add_flag("-n,--no-subset", is_no_subset, "Do not subset fonts");
  auto* p_opt_s = app.add_flag("-s,--subset-only", is_subset_only,
                               "Subset fonts but not embed them into subtitle");
  auto* p_opt_v = app.add_option("-v,--verbose", verbose, "Set logging level.");
  app.set_help_flag("");
  auto* p_opt_h = app.add_flag("-h,--help", is_help, "Get help info");
  p_opt_i->type_name("<file>");
  p_opt_o->type_name("<dir>");
  p_opt_f->type_name("<dir>");
  p_opt_d->type_name("<dir>");
  p_opt_v->type_name("<num>");
  app.failure_message(
      [=](const CLI::App* app, const CLI::Error& e) -> std::string {
        std::string str = CLI::FailureMessage::simple(app, e);
        str.pop_back();
        str += ". See --help for more info.";
        logger->error(str);
        return "";
      });
  CLI11_PARSE(app, argc, argv);

  // clang-format off
  if (is_help) {
    fmt::print("Subset fonts and embed them into an ASS subtitle.\n"
               "Usage:     assfonts [options...] [<file>]\n"
               "Examples:  assfonts <file>                  Embed subset fonts into ASS script\n"
               "           assfonts -i <file>               Same as above\n"
               "           assfonts -f <dir> -n -i <file>   Only embed fonts without subset\n"
               "           assfonts -o <dir> -s -i <file>   Only subset fonts but not embed\n"
               "           assfonts -f <dir> -b             Build or update fonts database only\n"
               "Options:\n"
               "  -i, --input,      <file>  Input .ass file\n"
               "  -o, --output      <dir>   Output directory    (Default: same directory as input)\n"
               "  -f, --fontpath    <dir>   Set fonts directory\n"
               "  -b, --build               Build or update fonts database    (Require --fontpath)\n"
               "  -d, --dbpath      <dir>   Set fonts database path    (Default: current path)\n"
               "  -n, --no-subset           Do not subset fonts\n"
               "  -s, --subset-only         Subset fonts but not embed them into subtitle\n"
               "  -v, --verbose     <num>   Set logging level (0 to 3), 0 is off    (Default: 3)\n"
               "  -h, --help                Get help info\n\n");
    if (argc == 2) {
      exit(EXIT_SUCCESS);
    }
  }
  // clang-format on

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
  }

  fs::path input_path = fs::system_complete(input);
  fs::path output_path = fs::system_complete(output);
  fs::path fonts_path = fs::system_complete(fonts);
  fs::path db_path = fs::system_complete(database);

  if (!input.empty() && !fs::is_regular_file(input_path)) {
    logger->error("\"{}\" is not found. See --help for more info.", input);
  }
  if (!output.empty() && !fs::is_directory(output_path)) {
    logger->error(
        "\"{}\" is not a legal directory path. See --help for more info.",
        output);
  }
  if (!fonts.empty() && !fs::is_directory(fonts_path)) {
    logger->error(
        "\"{}\" is not a legal directory path. See --help for more info.",
        fonts);
  }
  if (!fs::is_directory(db_path)) {
    logger->error(
        "\"{}\" is not a legal directory path. See --help for more info.",
        database);
  }
  if (is_build && fonts.empty()) {
    logger->error("No fontpath is found. See --help for more info.");
  }
  if (input.empty() && fonts.empty()) {
    logger->error("No input is found. See --help for more info.");
  }
  if (input.empty() && !is_build) {
    logger->error("Do nothing. See --help for more info.");
  }

  if (!fonts.empty()) {
    fp.LoadFonts(fonts_path.generic_string());
  }
  if (is_build) {
    fp.SaveDB(db_path.generic_string() + "/fonts.db");
  } else {
    fp.LoadDB(db_path.generic_string() + "/fonts.db");
  }

  if (input.empty()) {
    exit(EXIT_SUCCESS);
  }

  ap.ReadFile(input_path.generic_string());

  if (output.empty()) {
    output_path = input_path.parent_path();
  }

  if (!is_no_subset) {
    fs.SetSubfontDir(output_path.generic_string() + "/" +
                     input_path.stem().generic_string() + "_subsetted");
  }
  fs.Run(is_no_subset);

  if (!is_subset_only) {
    afe.set_input_ass_path(input_path.generic_string());

    afe.set_output_ass_path(output_path.generic_string() + "/" +
                            input_path.stem().generic_string() + "_assfonts" +
                            input_path.extension().generic_string());
    afe.Run();
  }

  spdlog::drop("main");
  return 0;
}
