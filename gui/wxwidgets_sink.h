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

#include <array>
#include <cmath>
#include <memory>
#include <mutex>
#include <string>

#include <spdlog/sinks/base_sink.h>
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "ass_string.h"

namespace mylog {
namespace sinks {

template <typename Mutex>
class wxwidgets_sink : public spdlog::sinks::base_sink<Mutex> {
 public:
  wxwidgets_sink(wxTextCtrl* log_text) : log_text_(log_text) { SetColours(); };
  wxwidgets_sink(const wxwidgets_sink&) = delete;
  wxwidgets_sink& operator=(const wxwidgets_sink&) = delete;

 protected:
  void sink_it_(const spdlog::details::log_msg& msg) override {
    spdlog::memory_buf_t formatted;
    formatter_->format(msg, formatted);
#ifdef _WIN32
    wxString text(
        ass::U8ToWide(std::string(formatted.data(), formatted.size())));
#else
    wxString text(formatted.data(), wxConvUTF8, formatted.size());
#endif
    if (msg.level == spdlog::level::info) {
      log_text_->SetDefaultStyle(wxTextAttr(wxNullColour));
    } else if (msg.level == spdlog::level::warn) {
      log_text_->SetDefaultStyle(wxTextAttr(warn_colour_));
    } else if (msg.level == spdlog::level::err) {
      log_text_->SetDefaultStyle(wxTextAttr(err_colour_));
    } else {
      log_text_->SetDefaultStyle(wxTextAttr(wxNullColour));
    }
    log_text_->AppendText(text);
  }

  void flush_() override {
    log_text_->Clear();
  }

  void set_formatter_(
      std::unique_ptr<spdlog::formatter> sink_formatter) override {
    formatter_ = std::move(sink_formatter);
  }

 private:
  wxTextCtrl* log_text_;
  std::unique_ptr<spdlog::formatter> formatter_;
  wxColour warn_colour_;
  wxColour err_colour_;

  void SetColours() {
    wxColour background_colour = log_text_->GetBackgroundColour();
    std::array<double, 3> rgb;
    rgb[0] = background_colour.GetRed() / 255.0;
    rgb[1] = background_colour.GetGreen() / 255.0;
    rgb[2] = background_colour.GetBlue() / 255.0;
    for (double& v : rgb) {
      if (v <= 0.04045) {
        v = v / 12.92;
      } else {
        v = std::pow(((v + 0.055) / 1.055), 2.4);
      }
    }
    double Y = 0.2126 * rgb[0] + 0.7152 * rgb[1] + 0.0722 * rgb[2];
    double L;
    if (Y <= (216.0 / 24389.0)) {
      L = Y * (24389.0 / 27.0);
    } else {
      L = std::pow(Y, (1.0 / 3.0)) * 116.0 - 16.0;
    }
    if (L > 50.0) {
      warn_colour_ = *wxBLUE;
      err_colour_ = *wxRED;
    } else {
      warn_colour_ = *wxYELLOW;
      err_colour_ = *wxCYAN;
    }
  }
};

using wxwidgets_sink_mt = wxwidgets_sink<std::mutex>;
using wxwidgets_sink_st = wxwidgets_sink<spdlog::details::null_mutex>;

}  // namespace sinks
}  // namespace mylog

#endif