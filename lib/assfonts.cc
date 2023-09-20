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

#include "assfonts.h"

#include <functional>
#include <future>
#include <memory>
#include <string>
#include <vector>

#include <ThreadPool/ThreadPool.h>
#include <ghc/filesystem.hpp>

#include "ass_font_embedder.h"
#include "ass_logger.h"
#include "ass_parser.h"
#include "ass_string.h"
#include "font_parser.h"
#include "font_subsetter.h"

namespace fs = ghc::filesystem;

using LogType = struct {
  ASSFONTS_LOG_LEVEL level;
  std::string msg;
};

void AssfontsBuildDB(const char** fonts_paths, const unsigned int num_fonts,
                     const char* db_path, const AssfontsLogCallback cb,
                     const enum ASSFONTS_LOG_LEVEL log_level) {
  auto logger = std::make_shared<ass::Logger>(ass::Logger(cb, log_level));

  if (num_fonts == 0 || **fonts_paths == '\0') {
    logger->Error("No font directory.");
    return;
  }

  if (*db_path == '\0') {
    logger->Error("No database directory.");
    return;
  }

  ass::FontParser fp(logger);

  fs::path db(db_path);

  std::vector<AString> paths;

  for (unsigned int idx = 0; idx < num_fonts; ++idx) {
    fs::path path(fonts_paths[idx]);
    paths.emplace_back(path.native());
  }

  fp.LoadDB(db.native() + fs::path::preferred_separator + _ST("fonts.json"));

  fp.LoadFonts(paths);

  fp.SaveDB(db.native() + fs::path::preferred_separator + _ST("fonts.json"));
}

void AssfontsRun(const char** input_paths, const unsigned int num_paths,
                 const char* output_path, const char** fonts_paths,
                 const unsigned int num_fonts, const char* db_path,
                 const unsigned int brightness,
                 const unsigned int is_subset_only,
                 const unsigned int is_embed_only, const unsigned int is_rename,
                 const unsigned int num_thread, const AssfontsLogCallback cb,
                 const enum ASSFONTS_LOG_LEVEL log_level) {
  auto logger = std::make_shared<ass::Logger>(ass::Logger(cb, log_level));

  if (num_paths == 0 || **input_paths == '\0') {
    logger->Error("No input ASS file.");
    return;
  }

  if (*output_path == '\0') {
    logger->Error("No output directory.");
    return;
  }

  if (*db_path == '\0') {
    logger->Error("No database directory.");
    return;
  }

  ass::FontParser fp(logger);

  fs::path db(db_path);
  fs::path output(output_path);

  std::vector<AString> paths;

  for (unsigned int idx = 0; idx < num_fonts; ++idx) {
    if (*fonts_paths[idx] != '\0') {
      fs::path path(fonts_paths[idx]);
      paths.emplace_back(path.native());
    }
  }

  if (!paths.empty()) {
    fp.LoadFonts(paths, false);
  }

  fp.LoadDB(db.native() + fs::path::preferred_separator + _ST("fonts.json"));

  ThreadPool pool(num_thread);
  std::vector<std::future<std::vector<LogType>>> results;

  for (unsigned int idx = 0; idx < num_paths; ++idx) {
    results.emplace_back(pool.enqueue([=, &fp]() {
      std::vector<LogType> logs;

      auto log_callback = [&](const char* msg,
                              const ASSFONTS_LOG_LEVEL log_level) {
        LogType log = {log_level, std::string(msg)};
        logs.emplace_back(log);
      };

      auto t_logger =
          std::make_shared<ass::Logger>(ass::Logger(log_callback, log_level));

      ass::AssParser ap(t_logger);
      ass::FontSubsetter fs(ap, fp, t_logger);
      ass::AssFontEmbedder afe(fs, t_logger);

      fs::path input(input_paths[idx]);

      ap.set_output_dir_path(output.native());

      if (brightness != 0) {
        if (!ap.Recolorize(input.native(), brightness)) {
          return logs;
        }

        AString hdr_filename =
            input.stem().native() + _ST(".hdr") + input.extension().native();
        input = output / hdr_filename;
      }

      if (!ap.ReadFile(input.native())) {
        return logs;
      }

      if (is_embed_only && is_subset_only) {
        return logs;
      }

      if (!is_embed_only) {
        fs.SetSubfontDir(output.native() + fs::path::preferred_separator +
                         input.stem().native() + _ST("_subsetted"));
      }

      if (!fs.Run(is_embed_only, is_rename)) {
        return logs;
      }

      afe.set_output_dir_path(output.native());
      if (!afe.Run(is_subset_only, is_embed_only, is_rename)) {
        return logs;
      }

      return logs;
    }));
  }

  int idx = 0;
  for (auto&& result : results) {
    if (idx != 0) {
      logger->Text("");
    }

    auto logs = result.get();

    for (auto& log : logs) {
      switch (log.level) {
        case ASSFONTS_INFO:
          logger->Info(log.msg);
          break;
        case ASSFONTS_WARN:
          logger->Warn(log.msg);
          break;
        case ASSFONTS_ERROR:
          logger->Error(log.msg);
          break;
        case ASSFONTS_TEXT:
          logger->Text(log.msg);
        case ASSFONTS_NONE:
          break;
        default:
          break;
      }
    }

    ++idx;
  }
}