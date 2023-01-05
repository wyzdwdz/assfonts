#include "wxwidgets_sink.h"

#include <array>
#include <cmath>

namespace mylog {
namespace sinks {

WxSink::WxSink(wxWindow* parent)
    : wxTextCtrl(parent, wxID_ANY, wxEmptyString, wxDefaultPosition,
                 wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH) {
  SetColours();
}

void WxSink::log(const spdlog::details::log_msg& msg) {
  spdlog::memory_buf_t formatted;
  formatter_->format(msg, formatted);
#ifdef _WIN32
  wxString text(ass::U8ToWide(std::string(formatted.data(), formatted.size())));
#else
  wxString text(formatted.data(), wxConvUTF8, formatted.size());
#endif
  if (msg.level == spdlog::level::info) {
    SetDefaultStyle(wxTextAttr(wxNullColour));
  } else if (msg.level == spdlog::level::warn) {
    SetDefaultStyle(wxTextAttr(warn_colour_));
  } else if (msg.level == spdlog::level::err) {
    SetDefaultStyle(wxTextAttr(err_colour_));
  } else {
    SetDefaultStyle(wxTextAttr(wxNullColour));
  }
  AppendText(text);
}

void WxSink::flush() {
  Clear();
}

void WxSink::set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) {
  formatter_ = std::move(sink_formatter);
}

void WxSink::set_level(spdlog::level::level_enum log_level) {
  level_.store(log_level, std::memory_order_relaxed);
}

spdlog::level::level_enum WxSink::level() const {
  return static_cast<spdlog::level::level_enum>(
      level_.load(std::memory_order_relaxed));
}

bool WxSink::should_log(spdlog::level::level_enum msg_level) const {
  return msg_level >= level_.load(std::memory_order_relaxed);
}

void WxSink::SetColours() {
  wxColour background_colour = GetBackgroundColour();
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

}  // namespace sinks
}  // namespace mylog