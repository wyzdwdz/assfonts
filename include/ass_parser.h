#ifndef ASSFONTS_ASSPARSER_H_
#define ASSFONTS_ASSPARSER_H_

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "error_proxy_sink.h"

namespace ass {

class AssParser {
 public:
  AssParser() {
    auto color_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto error_sink =
        std::make_shared<mylog::sinks::error_proxy_sink_mt>(color_sink);
    logger_ = std::make_shared<spdlog::logger>("ass_parser", error_sink);
    spdlog::register_logger(logger_);
  };
  AssParser(const std::string& ass_file_path) : AssParser() {
    ReadFile(ass_file_path);
  }
  ~AssParser() { spdlog::drop("ass_parser"); };

  void ReadFile(const std::string& ass_file_path);

 private:
  std::shared_ptr<spdlog::logger> logger_;
  std::string ass_path_;
  std::vector<std::string> text_;
  std::vector<std::vector<std::string>> styles_;
  std::vector<std::vector<std::string>> dialogues_;
  std::map<std::string, std::set<char32_t>> font_sets_;

  bool IsUTF8(const std::string& line);
  bool FindTitle(const std::string& line, const std::string& title);
  std::vector<std::string> ParseLine(const std::string& line,
                                     const unsigned int num_field);
  void ParseAss();
  void set_font_sets();

  friend class FontSubsetter;
};

}  // namespace ass

#endif