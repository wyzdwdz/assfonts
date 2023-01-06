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

#ifndef ASSFONTS_RUN_H_
#define ASSFONTS_RUN_H_

#include <filesystem>
#include <memory>
#include <vector>

#include "wxwidgets_sink.h"

namespace fs = std::filesystem;

void BuildDB(const fs::path& fonts_path, const fs::path& db_path,
             const std::shared_ptr<mylog::sinks::wxwidgets_sink_mt>& sink);

void Run(const std::vector<fs::path>& input_paths, const fs::path& output_path,
         const fs::path& fonts_path, const fs::path& db_path,
         const unsigned int& brightness, const bool& is_subset_only,
         const bool& is_embed_only,
         const std::shared_ptr<mylog::sinks::wxwidgets_sink_mt>& sink);

#endif