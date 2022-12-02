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

#ifndef ASSFONTS_ASSFONTEMBEDDER_H_
#define ASSFONTS_ASSFONTEMBEDDER_H_

#include <memory>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>

#include "font_subsetter.h"

namespace ass {

class AssFontEmbedder {
 public:
  template <typename T>
  AssFontEmbedder(const FontSubsetter& fs, std::shared_ptr<T> sink) : fs_(fs) {
    logger_ = std::make_shared<spdlog::logger>("ass_font_embedder", sink);
    spdlog::register_logger(logger_);
  };
  ~AssFontEmbedder() { spdlog::drop("ass_font_embedder"); };

  void set_input_ass_path(const std::string& input_ass_path);
  void set_output_dir_path(const std::string& output_ass_path);
  void Run(bool is_clean_only);

 private:
  const FontSubsetter& fs_;
  std::shared_ptr<spdlog::logger> logger_;
  std::string input_ass_path_;
  std::string output_dir_path_;
  std::string UUEncode(const char* begin, const char* end,
                       bool insert_linebreaks);
  bool CleanFonts();
};

};  // namespace ass

#endif