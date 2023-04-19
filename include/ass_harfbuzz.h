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

#ifndef ASSFONTS_ASSHARFBUZZ_H_
#define ASSFONTS_ASSHARFBUZZ_H_

#include <harfbuzz/hb-subset.h>
#include <harfbuzz/hb.h>

namespace ass {

class HbBlob {
 public:
  HbBlob(hb_blob_t* const hb_blob) { set(hb_blob); }
  ~HbBlob() { hb_blob_destroy(hb_blob_); }

  HbBlob(const HbBlob&) = delete;
  HbBlob& operator=(const HbBlob&) = delete;

  inline hb_blob_t*& get() { return hb_blob_; }
  inline void set(hb_blob_t* const hb_blob) { hb_blob_ = hb_blob; }

 private:
  hb_blob_t* hb_blob_;
};

class HbFace {
 public:
  HbFace(hb_face_t* const hb_face) { set(hb_face); }
  ~HbFace() { hb_face_destroy(hb_face_); }

  HbFace(const HbFace&) = delete;
  HbFace& operator=(const HbFace&) = delete;

  inline hb_face_t*& get() { return hb_face_; }
  inline void set(hb_face_t* const hb_face) { hb_face_ = hb_face; }

 private:
  hb_face_t* hb_face_;
};

class HbSet {
 public:
  HbSet(hb_set_t* const hb_set) { set(hb_set); }
  ~HbSet() { hb_set_destroy(hb_set_); }

  HbSet(const HbSet&) = delete;
  HbSet& operator=(const HbSet&) = delete;

  inline hb_set_t*& get() { return hb_set_; }
  inline void set(hb_set_t* const hb_set) { hb_set_ = hb_set; }

 private:
  hb_set_t* hb_set_;
};

class HbSubsetInput {
 public:
  HbSubsetInput(hb_subset_input_t* const hb_subset_input) {
    set(hb_subset_input);
  }
  ~HbSubsetInput() { hb_subset_input_destroy(hb_subset_input_); }

  HbSubsetInput(const HbSubsetInput&) = delete;
  HbSubsetInput& operator=(const HbSubsetInput&) = delete;

  inline hb_subset_input_t*& get() { return hb_subset_input_; }
  inline void set(hb_subset_input_t* const hb_subset_input) {
    hb_subset_input_ = hb_subset_input;
  }

 private:
  hb_subset_input_t* hb_subset_input_;
};

}  // namespace ass

#endif