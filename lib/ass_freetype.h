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

#ifndef ASSFONTS_ASSFREETYPE_H_
#define ASSFONTS_ASSFREETYPE_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <ft2build.h>
#include FT_FREETYPE_H
#ifdef __cplusplus
}
#endif

#include "ass_string.h"

namespace ass {

class FTLibrary {
 public:
  FTLibrary() = default;
  ~FTLibrary() { Destroy(); }

  FTLibrary(const FTLibrary&) = delete;
  FTLibrary& operator=(const FTLibrary&) = delete;

  FTLibrary(FTLibrary&& prev) : ft_library_(prev.ft_library_) {
    prev.ft_library_ = nullptr;
  }
  FTLibrary& operator=(FTLibrary&& prev) {
    Destroy();
    ft_library_ = prev.ft_library_;
    prev.ft_library_ = nullptr;
    return *this;
  }

  inline FT_Library& get() { return ft_library_; }

 private:
  FT_Library ft_library_ = nullptr;

  void Destroy() {
    if (ft_library_) {
      FT_Done_FreeType(ft_library_);
    }
  }
};

class FTFace {
 public:
  FTFace() = default;
  ~FTFace() { Destroy(); }

  FTFace(FTFace&& prev) : ft_face_(prev.ft_face_) { prev.ft_face_ = nullptr; }
  FTFace& operator=(FTFace&& prev) {
    Destroy();
    ft_face_ = prev.ft_face_;
    prev.ft_face_ = nullptr;
    return *this;
  }

  inline FT_Face& get() { return ft_face_; }

 private:
  FT_Face ft_face_ = nullptr;

  void Destroy() {
    if (ft_face_) {
      FT_Done_Face(ft_face_);
    }
  }
};

FT_Error NewOpenArgs(const AString& filepathname, FT_StreamRec& stream,
                     FT_Open_Args& args);

}  // namespace ass

#endif