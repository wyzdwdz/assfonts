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

#ifndef ASSFONTS_ERRORPROXYSINK_H_
#define ASSFONTS_ERRORPROXYSINK_H_

#include <cstdlib>
#include <memory>
#include <mutex>

#include <spdlog/details/null_mutex.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/base_sink.h>

namespace mylog {
namespace sinks {

template <typename Mutex>
class error_proxy_sink : public spdlog::sinks::base_sink<Mutex> {
 public:
  error_proxy_sink(std::shared_ptr<spdlog::sinks::sink> sink) : sink_(sink){};
  ~error_proxy_sink() = default;
  error_proxy_sink(const error_proxy_sink&) = delete;
  error_proxy_sink& operator=(const error_proxy_sink&) = delete;

 protected:
  void sink_it_(const spdlog::details::log_msg& msg) override {
    if (sink_->should_log(msg.level)) {
      sink_->log(msg);
    }
    if (spdlog::level::err == msg.level) {
      spdlog::shutdown();
      exit(EXIT_FAILURE);
    }
  }
  void flush_() { sink_->flush(); }
  void set_pattern_(const std::string& pattern) {
    set_formatter_(
        spdlog::details::make_unique<spdlog::pattern_formatter>(pattern));
  }
  void set_formatter_(std::unique_ptr<spdlog::formatter> sink_formatter) {
    spdlog::sinks::base_sink<Mutex>::formatter_ = std::move(sink_formatter);
    sink_->set_formatter(spdlog::sinks::base_sink<Mutex>::formatter_->clone());
  }

 private:
  std::shared_ptr<spdlog::sinks::sink> sink_;
};

using error_proxy_sink_mt = error_proxy_sink<std::mutex>;
using error_proxy_sink_st = error_proxy_sink<spdlog::details::null_mutex>;

}  // namespace sinks
}  // namespace mylog

#endif