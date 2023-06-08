#include "atlas_cache.h"

#include <ghc/filesystem.hpp>

#include "assfonts.h"

namespace fs = ghc::filesystem;

void AtlasCache::SaveFontAtlas(const std::string& path, const float font_size,
                               bool override_if_exists,
                               ImFontAtlas* font_atlas) {
  if (font_atlas == nullptr) {
    font_atlas = ImGui::GetIO().Fonts;
  }

  if (fs::exists(path) && !override_if_exists) {
    throw std::runtime_error("Target already exists");
  }

  std::ofstream file(path.c_str(), std::ios::out | std::ios::binary);

  int major = ASSFONTS_VERSION_MAJOR, minor = ASSFONTS_VERSION_MINOR,
      patch = ASSFONTS_VERSION_PATCH;
  file.write(reinterpret_cast<const char*>(&major), sizeof(major));
  file.write(reinterpret_cast<const char*>(&minor), sizeof(minor));
  file.write(reinterpret_cast<const char*>(&patch), sizeof(patch));

  float fontsize = font_size;
  file.write(reinterpret_cast<const char*>(&fontsize), sizeof(fontsize));

  file.write(reinterpret_cast<const char*>(&font_atlas->TexUvWhitePixel),
             sizeof(font_atlas->TexUvWhitePixel));
  file.write(reinterpret_cast<const char*>(font_atlas->TexUvLines),
             sizeof(font_atlas->TexUvLines));

  auto size = font_atlas->Fonts[0]->Glyphs.size();
  file.write(reinterpret_cast<const char*>(&size), sizeof(int));
  file.write(reinterpret_cast<const char*>(font_atlas->Fonts[0]->Glyphs.Data),
             sizeof(ImFontGlyph) * size);

  unsigned char* out_pixels;
  int out_width;
  int out_height;
  font_atlas->GetTexDataAsAlpha8(&out_pixels, &out_width, &out_height);
  file.write(reinterpret_cast<const char*>(&out_width), sizeof(int));
  file.write(reinterpret_cast<const char*>(&out_height), sizeof(int));
  file.write(reinterpret_cast<const char*>(out_pixels), out_width * out_height);
}

bool AtlasCache::LoadFontAtlasCache(const std::string& path,
                                    const float font_size) {
  if (!fs::exists(path)) {
    return false;
  }

  std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);

  int major, minor, patch;
  file.read(reinterpret_cast<char*>(&major), sizeof(major));
  file.read(reinterpret_cast<char*>(&minor), sizeof(minor));
  file.read(reinterpret_cast<char*>(&patch), sizeof(patch));
  if (major != ASSFONTS_VERSION_MAJOR || minor != ASSFONTS_VERSION_MINOR ||
      patch != ASSFONTS_VERSION_PATCH) {
    return false;
  }

  float fontsize;
  file.read(reinterpret_cast<char*>(&fontsize), sizeof(fontsize));
  if (fontsize != font_size) {
    return false;
  }

  file.read(reinterpret_cast<char*>(&tex_uv_white_pixel_),
            sizeof(tex_uv_white_pixel_));
  file.read(reinterpret_cast<char*>(tex_uv_lines_), sizeof(tex_uv_lines_));

  int glyph_size;
  file.read(reinterpret_cast<char*>(&glyph_size), sizeof(int));
  glyphs_.resize(glyph_size);
  glyphs_.shrink_to_fit();
  file.read(reinterpret_cast<char*>(glyphs_.data()),
            sizeof(ImFontGlyph) * glyph_size);

  file.read(reinterpret_cast<char*>(&out_width_), sizeof(int));
  file.read(reinterpret_cast<char*>(&out_height_), sizeof(int));

  image_data_ = std::unique_ptr<unsigned char>(
      new unsigned char[out_width_ * out_height_]);
  file.read(reinterpret_cast<char*>(image_data_.get()),
            out_width_ * out_height_);

  return true;
}

void AtlasCache::RestoreFontAtlas(const float font_size,
                                  ImFontAtlas* font_atlas) {
  if (font_atlas == nullptr) {
    font_atlas = ImGui::GetIO().Fonts;
  }

  ImFontConfig dummy_config;
  dummy_config.FontData = new unsigned char;
  dummy_config.FontDataSize = 1;
  dummy_config.SizePixels = 1;
  auto* font = font_atlas->AddFont(&dummy_config);
  font->FontSize = font_size;
  font->ConfigData = &dummy_config;
  font->ConfigDataCount = 1;
  font->ContainerAtlas = font_atlas;

  font_atlas->ClearTexData();
  font_atlas->TexPixelsAlpha8 = image_data_.release();
  font_atlas->TexWidth = out_width_;
  font_atlas->TexHeight = out_height_;
  font_atlas->TexUvWhitePixel = tex_uv_white_pixel_;
  std::memcpy(font_atlas->TexUvLines, tex_uv_lines_, sizeof(tex_uv_lines_));

  for (const auto& glyph : glyphs_) {
    font->AddGlyph(&dummy_config, glyph.Codepoint, glyph.X0, glyph.Y0, glyph.X1,
                   glyph.Y1, glyph.U0, glyph.V0, glyph.U1, glyph.V1,
                   glyph.AdvanceX);
    font->SetGlyphVisible(glyph.Codepoint, glyph.Visible);
  }

  font->BuildLookupTable();
  font_atlas->TexReady = true;
}