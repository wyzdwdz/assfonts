#pragma once

#include <assfonts.h>

class AssfontsWrapper {
 public: 
     AssfontsWrapper()

  void BuildDB(const char* fonts_path, const char* db_path,
               const AssfontsLogCallback cb,
               const enum ASSFONTS_LOG_LEVEL log_level) {
    AssfontsBuildDB(fonts_path, db_path, log_callback_, log_level);
  }

  void Run(const char** input_paths, const unsigned int num_paths,
           const char* output_path, const char* fonts_path, const char* db_path,
           const unsigned int brightness, const unsigned int is_subset_only,
           const unsigned int is_embed_only, const unsigned int is_rename,
           const AssfontsLogCallback cb,
           const enum ASSFONTS_LOG_LEVEL log_level) {
    AssfontsRun(input_paths, num_paths, output_path, fonts_path, db_path,
                brightness, is_subset_only, is_embed_only, is_rename,
                log_callback_, log_level);
  }

 private:
  void (*log_callback_);
};