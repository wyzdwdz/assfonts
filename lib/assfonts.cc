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

#include <memory>

#include <ghc/filesystem.hpp>

#include "ass_font_embedder.h"
#include "ass_logger.h"
#include "ass_parser.h"
#include "ass_string.h"
#include "font_parser.h"
#include "font_subsetter.h"

namespace fs = ghc::filesystem;

void AssfontsBuildDB(const char* fonts_path, const char* db_path,
                     const AssfontsLogCallback cb,
                     const enum ASSFONTS_LOG_LEVEL log_level) {
  auto logger = std::make_shared<ass::Logger>(ass::Logger(cb, log_level));

  if (*fonts_path == '\0') {
    logger->Error("No font directory.");
    return;
  }

  if (*db_path == '\0') {
    logger->Error("No database directory.");
    return;
  }

  ass::FontParser fp(logger);

  fs::path db(db_path);
  fs::path fonts(fonts_path);

  fp.LoadDB(db.native() + fs::path::preferred_separator + _ST("fonts.json"));

  fp.LoadFonts(fonts.native());

  fp.SaveDB(db.native() + fs::path::preferred_separator + _ST("fonts.json"));
}

void AssfontsRun(const char** input_paths, const unsigned int num_paths,
                 const char* output_path, const char* fonts_path,
                 const char* db_path, const unsigned int brightness,
                 const unsigned int is_subset_only,
                 const unsigned int is_embed_only, const unsigned int is_rename,
                 const AssfontsLogCallback cb,
                 const enum ASSFONTS_LOG_LEVEL log_level) {
  auto logger = std::make_shared<ass::Logger>(ass::Logger(cb, log_level));

  if (num_paths == 0) {
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

  ass::AssParser ap(logger);
  ass::FontParser fp(logger);
  ass::FontSubsetter fs(ap, fp, logger);
  ass::AssFontEmbedder afe(fs, logger);

  fs::path fonts(fonts_path);
  fs::path db(db_path);
  fs::path output(output_path);

  if (!fonts.empty()) {
    fp.LoadFonts(fonts.native());
  }

  fp.LoadDB(db.native() + fs::path::preferred_separator + _ST("fonts.json"));

  for (unsigned int idx = 0; idx < num_paths; ++idx) {
    if (idx != 0) {
      logger->Text("");
    }

    fs::path input(input_paths[idx]);

    ap.set_output_dir_path(output.native());

    if (brightness != 0) {
      if (!ap.Recolorize(input.native(), brightness)) {
        continue;
      }

      AString hdr_filename =
          input.stem().native() + _ST(".hdr") + input.extension().native();
      input = output / hdr_filename;
    }

    if (!ap.ReadFile(input.native())) {
      continue;
    }

    if (is_embed_only && is_subset_only) {
      ap.Clear();
      fs.Clear();
      afe.Clear();
      continue;
    }

    if (!is_embed_only) {
      fs.SetSubfontDir(output.native() + fs::path::preferred_separator +
                       input.stem().native() + _ST("_subsetted"));
    }

    if (!fs.Run(is_embed_only, is_rename)) {
      continue;
    }

    afe.set_output_dir_path(output.native());
    if (!afe.Run(is_subset_only, is_embed_only, is_rename)) {
      continue;
    }

    ap.Clear();
    fs.Clear();
    afe.Clear();
  }
}