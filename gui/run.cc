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

#include "run.h"

#include "ass_font_embedder.h"
#include "ass_parser.h"
#include "ass_string.h"
#include "font_parser.h"
#include "font_subsetter.h"

void BuildDB(const fs::path fonts_path, const fs::path db_path,
             std::shared_ptr<mylog::sinks::wxwidgets_sink_mt> sink) {
  ass::FontParser fp(sink);
  fp.LoadFonts(fonts_path.native());
  fp.SaveDB(db_path.native() + _ST("/fonts.db"));
}

void Run(const fs::path input_path, const fs::path output_path,
         const fs::path fonts_path, const fs::path db_path, bool is_subset_only,
         bool is_embed_only,
         std::shared_ptr<mylog::sinks::wxwidgets_sink_mt> sink) {
  ass::AssParser ap(sink);
  ass::FontParser fp(sink);
  ass::FontSubsetter fs(ap, fp, sink);
  ass::AssFontEmbedder afe(fs, sink);
  ap.set_output_dir_path(output_path.native());
  if (!fonts_path.empty()) {
    fp.LoadFonts(fonts_path.native());
  }
  fp.LoadDB(db_path.native() + _ST("/fonts.db"));
  if (!ap.ReadFile(input_path.native())) {
    return;
  }
  if (is_embed_only && is_subset_only) {
    return;
  }
  if (!is_embed_only) {
    fs.SetSubfontDir(output_path.native() + _ST("/") +
                     input_path.stem().native() + _ST("_subsetted"));
  }
  if (!fs.Run(is_embed_only)) {
    return;
  }
  if (!is_subset_only) {
    afe.set_output_dir_path(output_path.native());
    if (!afe.Run()) {
      return;
    }
  }
}