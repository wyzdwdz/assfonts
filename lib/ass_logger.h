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

#ifndef ASSFONTS_ASSLOGGER_H_
#define ASSFONTS_ASSLOGGER_H_

#include <algorithm>
#include <functional>
#include <string>
#include <utility>

#include <fmt/core.h>
#include <fmt/xchar.h>

#include "ass_string.h"
#include "assfonts.h"

namespace ass {

using CallbackType = std::function<void(const char*, const ASSFONTS_LOG_LEVEL)>;

class Logger {
 public:
  Logger(CallbackType cb, ASSFONTS_LOG_LEVEL log_level)
      : cb_(cb), log_level_(log_level){};
  ~Logger() = default;

  template <typename... T>
  void Text(fmt::format_string<T...> fmt, T&&... args) {
    if (log_level_ <= ASSFONTS_TEXT) {
      Log(ASSFONTS_TEXT, fmt, std::forward<T>(args)...);
    }
  }
  template <typename... T>
  void Text(fmt::wformat_string<T...> fmt, T&&... args) {
    if (log_level_ <= ASSFONTS_TEXT) {
      Log(ASSFONTS_TEXT, fmt, std::forward<T>(args)...);
    }
  }

  template <typename... T>
  void Info(fmt::format_string<T...> fmt, T&&... args) {
    if (log_level_ <= ASSFONTS_INFO) {
      Log(ASSFONTS_INFO, fmt, std::forward<T>(args)...);
    }
  }
  template <typename... T>
  void Info(fmt::wformat_string<T...> fmt, T&&... args) {
    if (log_level_ <= ASSFONTS_INFO) {
      Log(ASSFONTS_INFO, fmt, std::forward<T>(args)...);
    }
  }

  template <typename... T>
  void Warn(fmt::format_string<T...> fmt, T&&... args) {
    if (log_level_ <= ASSFONTS_WARN) {
      Log(ASSFONTS_WARN, fmt, std::forward<T>(args)...);
    }
  }
  template <typename... T>
  void Warn(fmt::wformat_string<T...> fmt, T&&... args) {
    if (log_level_ <= ASSFONTS_WARN) {
      Log(ASSFONTS_WARN, fmt, std::forward<T>(args)...);
    }
  }

  template <typename... T>
  void Error(fmt::format_string<T...> fmt, T&&... args) {
    if (log_level_ <= ASSFONTS_ERROR) {
      Log(ASSFONTS_ERROR, fmt, std::forward<T>(args)...);
    }
  }
  template <typename... T>
  void Error(fmt::wformat_string<T...> fmt, T&&... args) {
    if (log_level_ <= ASSFONTS_ERROR) {
      Log(ASSFONTS_ERROR, fmt, std::forward<T>(args)...);
    }
  }

 private:
  CallbackType cb_;
  ASSFONTS_LOG_LEVEL log_level_;

  template <typename... T>
  void Log(ASSFONTS_LOG_LEVEL log_level, fmt::format_string<T...> fmt,
           T&&... args) {
    std::string msg;

    msg.append(fmt::format(fmt, std::forward<T>(args)...));

    ReplaceNewline(msg);

    cb_(msg.c_str(), log_level);
  }

  template <typename... T>
  void Log(ASSFONTS_LOG_LEVEL log_level, fmt::wformat_string<T...> fmt,
           T&&... args) {
    std::string msg;

    msg.append(WideToU8(fmt::format(fmt, std::forward<T>(args)...)));

    ReplaceNewline(msg);

    cb_(msg.c_str(), log_level);
  }

  void ReplaceNewline(std::string& str) {
    std::replace(str.begin(), str.end(), '\n', ' ');
    str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
  }
};

}  // namespace ass

#endif