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

#ifndef ASSFONTS_WXWIDGETSSINK_H_
#define ASSFONTS_WXWIDGETSSINK_H_

#include <memory>
#include <mutex>
#include <string>

#include <spdlog/formatter.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/base_sink.h>
#include <wx/wx.h>

namespace mylog {
namespace sinks {

template <typename Mutex>
class wxwidgets_sink : public spdlog::sinks::base_sink<Mutex> {
 public:
  wxwidgets_sink(wxTextCtrl* log_text) : log_text_(log_text){};
  ~wxwidgets_sink() = default;
  wxwidgets_sink(const wxwidgets_sink&) = delete;
  wxwidgets_sink& operator=(const wxwidgets_sink&) = delete;

 protected:
  void sink_it_(const spdlog::details::log_msg& msg) override {
    spdlog::memory_buf_t formatted;
    formatter_->format(msg, formatted);
    log_text_->AppendText(std::string(formatted.data(), formatted.size()));
    log_text_->ScrollLines(1);
  }
  void flush_() { log_text_->Clear(); }
  void set_pattern_(const std::string&){};
  void set_formatter_(std::unique_ptr<spdlog::formatter> sink_formatter) {
    formatter_ = std::move(sink_formatter);
  }
  void set_level(spdlog::level::level_enum log_level) {
    level_.store(log_level, std::memory_order_relaxed);
  }
  spdlog::level::level_enum level() const {
    return static_cast<spdlog::level::level_enum>(
        level_.load(std::memory_order_relaxed));
  }
  bool should_log(spdlog::level::level_enum msg_level) const {
    return msg_level >= level_.load(std::memory_order_relaxed);
  }

 private:
  wxTextCtrl* log_text_;
  std::unique_ptr<spdlog::formatter> formatter_;
  spdlog::level_t level_{spdlog::level::trace};
};

using wxwidgets_sink_mt = wxwidgets_sink<std::mutex>;
using wxwidgets_sink_st = wxwidgets_sink<spdlog::details::null_mutex>;

}  // namespace sinks
}  // namespace mylog

#endif