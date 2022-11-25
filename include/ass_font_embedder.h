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

#ifndef ASSFONTS_ASSFONTEMBEDDER_H_
#define ASSFONTS_ASSFONTEMBEDDER_H_

#include <memory>
#include <string>
#include <vector>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "error_proxy_sink.h"
#include "font_subsetter.h"

namespace ass {

class AssFontEmbedder {
 public:
  AssFontEmbedder(const FontSubsetter& fs) : fs_(fs) {
    auto color_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto error_sink =
        std::make_shared<mylog::sinks::error_proxy_sink_mt>(color_sink);
    logger_ = std::make_shared<spdlog::logger>("ass_font_embedder", error_sink);
    spdlog::register_logger(logger_);
  };
  ~AssFontEmbedder() { spdlog::drop("ass_font_embedder"); };

  void set_input_ass_path(const std::string& input_ass_path);
  void set_output_ass_path(const std::string& output_ass_path);
  void Run();

 private:
  const FontSubsetter& fs_;
  std::shared_ptr<spdlog::logger> logger_;
  std::string input_ass_path_;
  std::string output_ass_path_;
  std::string UUEncode(const char* begin, const char* end,
                       bool insert_linebreaks);
};

};  // namespace ass

#endif