#pragma once

#include <memory>
#include <string>
#include <vector>

#include <imgui.h>

class AtlasCache {
 public:
  AtlasCache() = default;
  ~AtlasCache() = default;

  void SaveFontAtlas(const std::string& path, const float font_size,
                     bool override_if_exists = false,
                     ImFontAtlas* font_atlas = nullptr);

  bool LoadFontAtlasCache(const std::string& path, const float font_size);

  void RestoreFontAtlas(const float font_size,
                        ImFontAtlas* font_atlas = nullptr);

 private:
  ImVec2 tex_uv_white_pixel_;
  ImVec4 tex_uv_lines_[IM_DRAWLIST_TEX_LINES_WIDTH_MAX + 1];

  std::vector<ImFontGlyph> glyphs_;

  int out_width_;
  int out_height_;

  std::unique_ptr<unsigned char> image_data_;
};