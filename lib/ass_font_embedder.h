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

#ifndef ASSFONTS_ASSFONTEMBEDDER_H_
#define ASSFONTS_ASSFONTEMBEDDER_H_

#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ass_logger.h"
#include "ass_parser.h"
#include "ass_string.h"
#include "font_subsetter.h"

namespace ass {

class AssFontEmbedder {
 public:
  AssFontEmbedder(
      const AssParser& ap,
      const std::vector<FontSubsetter::FontSubsetInfo>& subfonts_info,
      std::shared_ptr<Logger> logger)
      : ap_(ap), subfonts_info_(subfonts_info), logger_(logger){};
  ~AssFontEmbedder() = default;

  AssFontEmbedder(const AssFontEmbedder&) = delete;
  AssFontEmbedder& operator=(const AssFontEmbedder&) = delete;

  void set_output_dir_path(const AString& output_ass_path);
  bool Run(const bool is_subset_only, const bool is_embed_only,
           const bool is_rename = false);
  void Clear();

 private:
  const AssParser& ap_;
  std::vector<FontSubsetter::FontSubsetInfo> subfonts_info_;
  std::shared_ptr<Logger> logger_;
  AString output_dir_path_;
  std::map<std::string, std::string> fontname_map_;

  void WriteOutput(const std::vector<std::string>& text, size_t& num_line,
                   std::ofstream& output_ass);
  void WriteFonts(bool& has_none_ttf, std::ofstream& output_ass);

  std::string UUEncode(const char* begin, const char* end,
                       bool insert_linebreaks);

  void WriteRenameInfo(std::vector<std::string>& text);
  void FontRename(std::vector<AssParser::TextInfo>& text);
  bool WriteRenamed(AString& path, std::vector<std::string>& text);
};

};  // namespace ass

#endif