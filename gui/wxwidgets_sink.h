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

#include <spdlog/sinks/base_sink.h>
#include <wx/wx.h>
#include <boost/locale.hpp>

namespace mylog {
namespace sinks {

template <typename Mutex>
class wxwidgets_sink : public spdlog::sinks::base_sink<Mutex> {
 public:
  wxwidgets_sink(wxTextCtrl* log_text) : log_text_(log_text){};

 protected:
  void sink_it_(const spdlog::details::log_msg& msg) override {
    spdlog::memory_buf_t formatted;
    formatter_->format(msg, formatted);
    wxString text(boost::locale::conv::utf_to_utf<wchar_t, char>(
        std::string(formatted.data(), formatted.size())));
    *log_text_ << text;
  }
  void flush_() override { log_text_->Clear(); }
  void set_formatter_(
      std::unique_ptr<spdlog::formatter> sink_formatter) override {
    formatter_ = std::move(sink_formatter);
  };

 private:
  wxTextCtrl* log_text_;
  std::unique_ptr<spdlog::formatter> formatter_;
};

using wxwidgets_sink_mt = wxwidgets_sink<std::mutex>;
using wxwidgets_sink_st = wxwidgets_sink<spdlog::details::null_mutex>;

}  // namespace sinks
}  // namespace mylog

#endif