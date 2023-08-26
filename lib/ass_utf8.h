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

#ifndef ASSFONTS_ASSUTF8_H_
#define ASSFONTS_ASSUTF8_H_

#include <cstdlib>
#include <iterator>
#include <string>
#include <type_traits>

#include <nonstd/string_view.hpp>

#include "ass_string.h"

namespace ass {

unsigned char* StrToLwrExt(unsigned char* pString);

template <typename StringType,
          typename = typename std::enable_if<
              std::is_same<StringType, std::string>::value ||
              std::is_same<StringType, nonstd::string_view>::value>::type>
class U8Iterator {
 public:
  using iterator_type = U8Iterator;
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = char32_t;
  using difference_type = int;
  using reference = value_type const&;
  using pointer = value_type const*;

  static const value_type EOS = static_cast<value_type>(EOF);
  static const difference_type NPOS =
      static_cast<difference_type>(StringType::npos);

  U8Iterator(const StringType& str, bool end = false)
      : str_(&str), pos_(end ? str.size() : 0){};

  value_type operator*() {
    if (pos_ >= str_->size()) {
      return EOS;
    }

    unsigned char c = static_cast<const unsigned char>(*(str_->data() + pos_));
    unsigned int len;

    if ((len = GetCharSize(c)) == 0) {
      is_valid_ = false;
      return U'\0';
    }

    if (len == 1) {
      return static_cast<value_type>(c);
    }

    static const unsigned char table_unicode[] = {0, 0, 0x1F, 0xF, 0x7};

    value_type wc = U'\0';

    wc = table_unicode[len] & c;

    for (unsigned int i = 1; i < len; i++) {
      wc = wc << 6;
      wc = wc | (static_cast<const unsigned char>(*(str_->data() + pos_ + i)) &
                 0x3F);
    }

    return wc;
  }

  inline iterator_type& operator++() {
    ToNext();
    return *this;
  }

  inline iterator_type operator++(int) {
    iterator_type res(*this);
    ToNext();
    return res;
  }

  inline iterator_type& operator--() {
    ToPrev();
    return *this;
  }

  inline iterator_type operator--(int) {
    iterator_type res(*this);
    ToPrev();
    return res;
  }

  inline value_type operator[](difference_type diff) const {
    iterator_type iter(*this);
    iter += diff;
    return *iter;
  }

  inline iterator_type& operator+=(difference_type diff) {
    for (difference_type i = 0; i < diff; ++i) {
      ToNext();
    }
    return *this;
  }

  inline iterator_type operator+(difference_type diff) const {
    iterator_type iter(*this);
    iter += diff;
    return iter;
  }

  inline iterator_type& operator-=(difference_type diff) {
    for (difference_type i = 0; i < diff; ++i) {
      ToPrev();
    }
    return *this;
  }

  inline iterator_type operator-(difference_type diff) const {
    iterator_type iter(*this);
    iter -= diff;
    return iter;
  }

  template <typename T>
  inline
      typename std::enable_if<std::is_same<T, iterator_type>::value, bool>::type
      operator==(const T rhs) const {
    return (str_ == rhs.str_) && (pos_ == rhs.pos_);
  }

  template <typename T>
  inline typename std::enable_if<
      std::is_same<T, typename StringType::iterator>::value ||
          std::is_same<T, typename StringType::const_iterator>::value,
      bool>::type
  operator==(const T rhs) const {
    return pos_ ==
           static_cast<typename StringType::size_type>(rhs - str_->begin());
  }

  template <typename T>
  inline
      typename std::enable_if<std::is_same<T, iterator_type>::value, bool>::type
      operator!=(const T rhs) const {
    return (str_ != rhs.str_) || (pos_ != rhs.pos_);
  }

  template <typename T>
  inline typename std::enable_if<
      std::is_same<T, typename StringType::iterator>::value ||
          std::is_same<T, typename StringType::const_iterator>::value,
      bool>::type
  operator!=(const T rhs) const {
    return pos_ !=
           static_cast<typename StringType::size_type>(rhs - str_->begin());
  }

  inline bool IsValid() const { return is_valid_; }

  inline typename StringType::const_iterator ToStdIter() const {
    return str_->begin() + pos_;
  }

 private:
  const StringType* str_;
  typename StringType::size_type pos_;
  bool is_valid_ = true;

  unsigned int GetCharSize(const char c) {
    if (c == '\0') {
      return 0;
    }

    if ((c & 0x80) == 0) {
      return 1;
    } else if ((c & 0xE0) == 0xC0) {
      return 2;
    } else if ((c & 0xF0) == 0xE0) {
      return 3;
    } else if ((c & 0xF8) == 0xF0) {
      return 4;
    } else {
      return 0;
    }
  }

  void ToNext() {
    unsigned char c = static_cast<const unsigned char>(*(str_->data() + pos_));
    unsigned int len = 0;

    if ((len = GetCharSize(c)) == 0) {
      pos_ = str_->size();
      is_valid_ = false;
    } else {
      pos_ += len;
    }

    if (pos_ > str_->size()) {
      pos_ = str_->size();
      is_valid_ = false;
    }
  }

  void ToPrev() {
    if (pos_ == 0) {
      return;
    }

    while (pos_ > 0) {
      --pos_;
      unsigned char c =
          static_cast<const unsigned char>(*(str_->data() + pos_));
      if (c < 0x80 || c >= 0xC0) {
        break;
      }
    }
  }
};

template <typename StringType>
typename U8Iterator<StringType>::difference_type U8Find(
    const StringType& str, const std::u32string& value,
    const typename U8Iterator<StringType>::difference_type pos) {

  auto res = U8Iterator<StringType>::NPOS;
  auto it = U8Iterator<StringType>(str) + pos;

  if (value.size() >
      static_cast<typename StringType::size_type>(str.end() - it.ToStdIter())) {
    return res;
  }

  typename U8Iterator<StringType>::difference_type diff = pos;

  for (; it != str.end(); ++it, ++diff) {
    if (*it != value.at(0)) {
      continue;
    }

    bool is_found = true;

    for (typename U8Iterator<StringType>::difference_type i = 0;
         i < static_cast<typename U8Iterator<StringType>::difference_type>(
                 value.size());
         ++i) {
      if (*(it + i) != value.at(i)) {
        is_found = false;
        break;
      }
    }

    if (is_found) {
      res = diff;
      break;
    }
  }

  return res;
}

}  // namespace ass

#endif