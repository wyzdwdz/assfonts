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

#include <memory>

#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>
#include <ghc/filesystem.hpp>
#include <nowide/args.hpp>
#include <nowide/iostream.hpp>
#include <rang.hpp>

#ifdef __APPLE__
#include "get_app_support_dir.h"
#elif _WIN32
#include <Shlobj.h>
#else
#endif

#include <assfonts.h>

namespace fs = ghc::filesystem;

static std::string save_files_path = []() {
  fs::path path = fs::current_path();

#ifdef __APPLE__
  path = fs::path(GetAppSupportDir()) / "assfonts";
#elif __linux__
  if (getenv("HOME")) {
    path = fs::path(getenv("HOME")) / ".local" / "share" / "assfonts";
  }
#elif _WIN32
  TCHAR sz_path[MAX_PATH];
  SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, sz_path);

  path = fs::path(sz_path) / "assfonts";
#endif

  std::error_code ec;
  fs::create_directory(path, ec);

  return path.u8string();
}();

static void log_callback(const char* msg, const ASSFONTS_LOG_LEVEL log_level) {
  switch (log_level) {
    case ASSFONTS_INFO:
      nowide::cout << "[INFO] " << msg << std::endl;
      break;

    case ASSFONTS_WARN:
      nowide::cout << rang::style::bold << rang::fg::cyan << "[WARN] " << msg
                   << rang::fg::reset << rang::style::reset << std::endl;
      break;

    case ASSFONTS_ERROR:
      nowide::cout << rang::style::bold << rang::fg::red << "[ERROR] " << msg
                   << rang::fg::reset << rang::style::reset << std::endl;
      break;

    default:
      nowide::cout << msg << std::endl;
      break;
  }
}

struct FileValidator : public CLI::Validator {
  FileValidator() {
    name_ = "FILE";
    func_ = [](std::string& filename) {
      fs::path path(filename);
      if (!fs::exists(path)) {
        return "File does not exist: \"" + filename + "\"";
      }
      if (fs::is_directory(path)) {
        return "File is actually a directory: \"" + filename + "\"";
      }
      return std::string();
    };
  }
};
const static FileValidator file_validator;

struct DirectoryValidator : public CLI::Validator {
  DirectoryValidator() {
    name_ = "DIR";
    func_ = [](std::string& filename) {
      fs::path path(filename);
      if (!fs::exists(path)) {
        return "Directory does not exist: \"" + filename + "\"";
      }
      if (fs::is_regular_file(path)) {
        return "Directory is actually a file: \"" + filename + "\"";
      }
      return std::string();
    };
  }
};
const static DirectoryValidator directory_validator;

int main(int argc, char** argv) {
  auto loc = std::setlocale(LC_ALL, ".UTF8");
  if (loc == nullptr) {
    loc = std::setlocale(LC_ALL, "");
  }
  if (loc == nullptr) {
    std::cout << "Error! Locale not set." << std::endl;
    return -1;
  }

  nowide::args _(argc, argv);

  std::vector<std::string> inputs;
  std::string output;
  std::string fonts;
  std::string database = save_files_path;

  bool is_build = false;
  bool is_embed_only = false;
  bool is_subset_only = false;
  bool is_rename = false;
  bool is_help = false;

  unsigned int brightness = 203;
  int verbose = 3;

  CLI::App app{"Subset fonts and embed them into an ASS subtitle."};

  auto* p_opt_i =
      app.add_option("-i,--input,input", inputs, "Input .ass files");

  auto* p_opt_o = app.add_option("-o,--output", output, "Output directory");

  auto* p_opt_f = app.add_option("-f,--fontpath", fonts, "Set fonts directory");

  auto* p_opt_d =
      app.add_option("-d,--dbpath", database, "Set fonts database path");

  auto* p_opt_l = app.add_option("-l,--luminance", brightness,
                                 "Set brightness for HDR contents");

  auto* p_opt_b =
      app.add_flag("-b,--build", is_build, "Build or update fonts database");

  app.add_flag("-e,--embed-only", is_embed_only, "Do not subset fonts");

  app.add_flag("-s,--subset-only", is_subset_only,
               "Subset fonts but not embed them into subtitle");

  app.add_flag("-r,--rename", is_rename, "Rename subsetted fonts");

  auto* p_opt_v = app.add_option("-v,--verbose", verbose, "Set logging level.");

  app.set_help_flag("");
  app.add_flag("-h,--help", is_help, "Get help info");

  p_opt_i->type_name("<file>");
  p_opt_i->check(file_validator);

  p_opt_o->type_name("<dir>");
  p_opt_o->check(directory_validator);

  p_opt_f->type_name("<dir>");
  p_opt_f->check(directory_validator);

  p_opt_d->type_name("<dir>");
  p_opt_d->check(directory_validator);

  p_opt_l->type_name("<num>");
  p_opt_l->expected(0, 1);
  p_opt_l->check(CLI::Range(0, 1000));

  p_opt_b->needs(p_opt_f);

  p_opt_v->type_name("<num>");
  p_opt_v->check(CLI::Range(0, 3));

  app.failure_message(
      [=](const CLI::App* app, const CLI::Error& e) -> std::string {
        if (verbose > 0) {
          std::string str;
          str.append(CLI::FailureMessage::simple(app, e));
          str.pop_back();
          str += ". See --help for more info.";
          log_callback(str.c_str(), ASSFONTS_ERROR);
        }
        return std::string();
      });

  CLI11_PARSE(app, argc, argv);

  if (is_help || (!is_build && inputs.empty())) {
    // clang-format off
    nowide::cout << "assfonts v" << ASSFONTS_VERSION_MAJOR << "." << ASSFONTS_VERSION_MINOR << "." << ASSFONTS_VERSION_PATCH << "\n"
    << "Subset fonts and embed them into an ASS subtitle.\n"
    << "Usage:     assfonts [options...] [<files>]\n"
    << "Examples:  assfonts <files>                  Embed subset fonts into ASS script\n"
    << "           assfonts -i <files>               Same as above\n"
    << "           assfonts -o <dir> -s -i <files>   Only subset fonts but not embed\n"
    << "           assfonts -f <dir> -e -i <files>   Only embed fonts without subset\n"
    << "           assfonts -f <dir> -b              Build or update fonts database only\n"
    << "           assfonts -l <num> -i <files>      Recolorize the subtitle for HDR contents\n"
    << "Options:\n"
    << "  -i, --input,      <files>   Input .ass files\n"
    << "  -o, --output      <dir>     Output directory  (Default: same directory as input)\n"
    << "  -f, --fontpath    <dir>     Set fonts directory\n"
    << "  -b, --build                 Build or update fonts database  (Require: --fontpath)\n"
    << "  -d, --dbpath      <dir>     Set fonts database path  (Default: current path)\n"
    << "  -s, --subset-only <bool>    Subset fonts but not embed them into subtitle  (Default: False)\n"
    << "  -e, --embed-only  <bool>    Embed fonts into subtitle but not subset them (Default: False)\n"
    << "  -r, --rename      <bool>    !!!Experimental!!! Rename subsetted fonts (Default: False)\n"
    << "  -l, --luminance   <num>     Set subtitle brightness for HDR contents  (Default: 203)\n"
    << "  -v, --verbose     <num>     Set logging level (0 to 3), 0 is off  (Default: 3)\n"
    << "  -h, --help                  Get help info\n" << std::endl;
    // clang-format on
  }

  ASSFONTS_LOG_LEVEL max_log_level = ASSFONTS_INFO;
  switch (verbose) {
    case 0:
      max_log_level = ASSFONTS_NONE;
      break;
    case 1:
      max_log_level = ASSFONTS_ERROR;
      break;
    case 2:
      max_log_level = ASSFONTS_WARN;
      break;
    case 3:
      max_log_level = ASSFONTS_INFO;
      break;
  }

  if (is_build) {
    AssfontsBuildDB(fonts.c_str(), database.c_str(), log_callback,
                    max_log_level);
    if (verbose > 0) {
      nowide::cout << std::endl;
    }
  }

  if (inputs.empty()) {
    return 0;
  }

  auto inputs_char_list = std::unique_ptr<char*[]>(new char*[inputs.size()]);

  for (size_t idx = 0; idx < inputs.size(); ++idx) {
    inputs_char_list[idx] = const_cast<char*>(inputs.at(idx).c_str());
  }

  if (output.empty()) {
    fs::path path(inputs.at(0));
    output = path.parent_path().u8string();
  }

  if (p_opt_l->empty()) {
    brightness = 0;
  }

  AssfontsRun(const_cast<const char**>(inputs_char_list.get()), inputs.size(),
              output.c_str(), fonts.c_str(), database.c_str(), brightness,
              is_subset_only, is_embed_only, is_rename, log_callback,
              max_log_level);

  return 0;
}