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

#include <memory>

#include <boost/filesystem.hpp>

#include "wxwidgets_sink.h"

namespace fs = boost::filesystem;

void BuildDB(const fs::path fonts_path, const fs::path db_path,
             std::shared_ptr<mylog::sinks::wxwidgets_sink_mt> sink);

void Run(const fs::path input_path, const fs::path output_path,
         const fs::path fonts_path, const fs::path db_path, bool is_subset_only,
         bool is_embed_only,
         std::shared_ptr<mylog::sinks::wxwidgets_sink_mt> sink);

#endif