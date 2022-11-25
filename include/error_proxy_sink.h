//   Copyright [2022] [https://github.com/wyzdwdz]
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#ifndef ASSFONTS_ERRORPROXYSINK_H_
#define ASSFONTS_ERRORPROXYSINK_H_

#include <cstdlib>
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