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

#include <spdlog/sinks/sink.h>
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "ass_string.h"

namespace mylog {
namespace sinks {

class WxSink : public wxTextCtrl, public spdlog::sinks::sink {
 public:
  WxSink(wxWindow* parent);
  ~WxSink() = default;

 protected:
  void log(const spdlog::details::log_msg& msg);
  void flush();
  void set_pattern(const std::string& pattern){};
  void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter);
  void set_level(spdlog::level::level_enum log_level);
  spdlog::level::level_enum level() const;
  bool should_log(spdlog::level::level_enum msg_level) const;

 private:
  spdlog::level_t level_{spdlog::level::trace};
  std::unique_ptr<spdlog::formatter> formatter_;
  wxColour warn_colour_;
  wxColour err_colour_;

  void SetColours();
};

using wxwidgets_sink = WxSink;

struct WxSinkDeleter {
  void operator()(mylog::sinks::WxSink* p) const {};
};

}  // namespace sinks
}  // namespace mylog

#endif