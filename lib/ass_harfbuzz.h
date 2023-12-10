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

#ifndef ASSFONTS_ASSHARFBUZZ_H_
#define ASSFONTS_ASSHARFBUZZ_H_

#include <harfbuzz/hb-subset.h>
#include <harfbuzz/hb.h>

namespace ass {

class HbBlob {
 public:
  HbBlob(hb_blob_t* const hb_blob) { set(hb_blob); }
  ~HbBlob() { Destroy(); }

  HbBlob(const HbBlob&) = delete;
  HbBlob& operator=(const HbBlob&) = delete;

  HbBlob(HbBlob&& prev) : hb_blob_(prev.hb_blob_) { prev.hb_blob_ = nullptr; }
  HbBlob& operator=(HbBlob&& prev) {
    Destroy();
    hb_blob_ = prev.hb_blob_;
    prev.hb_blob_ = nullptr;
  }

  inline hb_blob_t*& get() { return hb_blob_; }
  inline void set(hb_blob_t* const hb_blob) { hb_blob_ = hb_blob; }

 private:
  hb_blob_t* hb_blob_;

  void Destroy() {
    if (hb_blob_) {
      hb_blob_destroy(hb_blob_);
    }
  }
};

class HbFace {
 public:
  HbFace(hb_face_t* const hb_face) { set(hb_face); }
  ~HbFace() { Destroy(); }

  HbFace(const HbFace&) = delete;
  HbFace& operator=(const HbFace&) = delete;

  HbFace(HbFace&& prev) : hb_face_(prev.hb_face_) { prev.hb_face_ = nullptr; }
  HbFace& operator=(HbFace&& prev) {
    Destroy();
    hb_face_ = prev.hb_face_;
    prev.hb_face_ = nullptr;
  }

  inline hb_face_t*& get() { return hb_face_; }
  inline void set(hb_face_t* const hb_face) { hb_face_ = hb_face; }

 private:
  hb_face_t* hb_face_;

  void Destroy() {
    if (hb_face_) {
      hb_face_destroy(hb_face_);
    }
  }
};

class HbSet {
 public:
  HbSet(hb_set_t* const hb_set) { set(hb_set); }
  ~HbSet() { Destroy(); }

  HbSet(const HbSet&) = delete;
  HbSet& operator=(const HbSet&) = delete;

  HbSet(HbSet&& prev) : hb_set_(prev.hb_set_) { prev.hb_set_ = nullptr; }
  HbSet& operator=(HbSet&& prev) {
    Destroy();
    hb_set_ = prev.hb_set_;
    prev.hb_set_ = nullptr;
  }

  inline hb_set_t*& get() { return hb_set_; }
  inline void set(hb_set_t* const hb_set) { hb_set_ = hb_set; }

 private:
  hb_set_t* hb_set_;

  void Destroy() {
    if (hb_set_) {
      hb_set_destroy(hb_set_);
    }
  }
};

class HbSubsetInput {
 public:
  HbSubsetInput(hb_subset_input_t* const hb_subset_input) {
    set(hb_subset_input);
  }
  ~HbSubsetInput() { Destroy(); }

  HbSubsetInput(const HbSubsetInput&) = delete;
  HbSubsetInput& operator=(const HbSubsetInput&) = delete;

  HbSubsetInput(HbSubsetInput&& prev)
      : hb_subset_input_(prev.hb_subset_input_) {
    prev.hb_subset_input_ = nullptr;
  }
  HbSubsetInput& operator=(HbSubsetInput&& prev) {
    Destroy();
    hb_subset_input_ = prev.hb_subset_input_;
    prev.hb_subset_input_ = nullptr;
  }

  inline hb_subset_input_t*& get() { return hb_subset_input_; }
  inline void set(hb_subset_input_t* const hb_subset_input) {
    hb_subset_input_ = hb_subset_input;
  }

 private:
  hb_subset_input_t* hb_subset_input_;

  void Destroy() {
    if (hb_subset_input_) {
      hb_subset_input_destroy(hb_subset_input_);
    }
  }
};

}  // namespace ass

#endif