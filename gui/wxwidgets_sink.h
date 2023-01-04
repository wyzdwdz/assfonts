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
#include <string>

#include <spdlog/details/log_msg.h>
#include <spdlog/formatter.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/sink.h>
#include <spdlog/spdlog.h>
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "ass_string.h"

namespace mylog {
namespace sinks {

class WxSink : public wxTextCtrl, public spdlog::sinks::sink {
 public:
  WxSink(wxWindow* parent)
      : wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition,
                   wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH){};

 protected:
  void log(const spdlog::details::log_msg& msg) {
    spdlog::memory_buf_t formatted;
    formatter_->format(msg, formatted);
#ifdef _WIN32
    wxString text(
        ass::U8ToWide(std::string(formatted.data(), formatted.size())));
#else
    wxString text(formatted.data(), wxConvUTF8, formatted.size());
#endif
    if (msg.level == spdlog::level::info) {
      SetDefaultStyle(wxTextAttr(wxNullColour));
    } else if (msg.level == spdlog::level::warn) {
      SetDefaultStyle(wxTextAttr(*wxGREEN));
    } else if (msg.level == spdlog::level::err) {
      SetDefaultStyle(wxTextAttr(*wxRED));
    } else {
      SetDefaultStyle(wxTextAttr(wxNullColour));
    }
    AppendText(text);
  }
  void flush() { Clear(); }
  void set_pattern(const std::string& pattern){};
  void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) {
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
  spdlog::level_t level_{spdlog::level::trace};
  std::unique_ptr<spdlog::formatter> formatter_;
};

using wxwidgets_sink = WxSink;

struct WxSinkDeleter {
  void operator()(mylog::sinks::WxSink* p) const {};
};

}  // namespace sinks
}  // namespace mylog

#endif