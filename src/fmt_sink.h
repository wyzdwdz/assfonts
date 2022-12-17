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

#ifndef ASSFONTS_FMTSINK_H_
#define ASSFONTS_FMTSINK_H_

#include <memory>
#include <mutex>

#ifdef _WIN32
#include <Windows.h>
#include "ass_string.h"
#else
#include <fmt/core.h>
#endif

#include <spdlog/sinks/base_sink.h>

namespace mylog {
namespace sinks {

template <typename Mutex>
class fmt_sink : public spdlog::sinks::base_sink<Mutex> {
 public:
  fmt_sink() = default;
  fmt_sink(const fmt_sink&) = delete;
  fmt_sink& operator=(const fmt_sink&) = delete;

 protected:
  void sink_it_(const spdlog::details::log_msg& msg) override {
    spdlog::memory_buf_t formatted;
    formatter_->format(msg, formatted);
#ifdef _WIN32
    std::wstring text(
        ass::U8ToWide(std::string(formatted.data(), formatted.size())));
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), text.c_str(),
                  static_cast<DWORD>(text.size()), NULL, NULL);
#else
    fmt::print("{}", std::string(formatted.data(), formatted.size()));
#endif
  }
  void flush_() override {}
  void set_formatter_(
      std::unique_ptr<spdlog::formatter> sink_formatter) override {
    formatter_ = std::move(sink_formatter);
  }

 private:
  std::unique_ptr<spdlog::formatter> formatter_;
};

using fmt_sink_mt = fmt_sink<std::mutex>;
using fmt_sink_st = fmt_sink<spdlog::details::null_mutex>;

}  // namespace sinks
}  // namespace mylog

#endif