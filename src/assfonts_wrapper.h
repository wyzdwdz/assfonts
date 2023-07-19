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

#pragma once

#include <assfonts.h>

class AssfontsWrapper {
 public: 
     AssfontsWrapper()

  void BuildDB(const char* fonts_path, const char* db_path,
               const AssfontsLogCallback cb,
               const enum ASSFONTS_LOG_LEVEL log_level) {
    AssfontsBuildDB(fonts_path, db_path, log_callback_, log_level);
  }

  void Run(const char** input_paths, const unsigned int num_paths,
           const char* output_path, const char* fonts_path, const char* db_path,
           const unsigned int brightness, const unsigned int is_subset_only,
           const unsigned int is_embed_only, const unsigned int is_rename,
           const AssfontsLogCallback cb,
           const enum ASSFONTS_LOG_LEVEL log_level) {
    AssfontsRun(input_paths, num_paths, output_path, fonts_path, db_path,
                brightness, is_subset_only, is_embed_only, is_rename,
                log_callback_, log_level);
  }

 private:
  void (*log_callback_);
};