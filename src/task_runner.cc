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

#include "task_runner.h"

#include <memory>

static void log_callback(const char* msg, const ASSFONTS_LOG_LEVEL log_level) {
  QString msg_string(msg);
  emit TaskRunner::GetInstance().OnSendLog(msg_string, log_level);
}

void TaskRunner::OnBuildRun(const QString fonts_path, const QString db_path) {
  is_running_ = true;

  AssfontsBuildDB(fonts_path.toUtf8().constData(), db_path.toUtf8().constData(),
                  log_callback, ASSFONTS_INFO);

  log_callback("", ASSFONTS_TEXT);

  emit OnSendClearFont();

  is_running_ = false;
}

void TaskRunner::OnStartRun(const QString inputs_path,
                            const QString output_path, const QString fonts_path,
                            const QString db_path,
                            const unsigned int brightness,
                            const bool is_subset_only, const bool is_embed_only,
                            const bool is_rename) {
  is_running_ = true;

  QStringList inputs_list = inputs_path.split(";");

  QVector<QByteArray> inputs_byte;
  for (const auto& str : inputs_list) {
    inputs_byte.push_back(str.trimmed().toUtf8());
  }

  auto inputs_char_list =
      std::unique_ptr<char*[]>(new char*[inputs_list.size()]);

  for (int idx = 0; idx < inputs_list.size(); ++idx) {
    inputs_char_list[idx] = const_cast<char*>(inputs_byte.at(idx).constData());
  }

  AssfontsRun(const_cast<const char**>(inputs_char_list.get()),
              inputs_list.size(), output_path.toUtf8().constData(),
              fonts_path.toUtf8().constData(), db_path.toUtf8().constData(),
              brightness, is_subset_only, is_embed_only, is_rename,
              log_callback, ASSFONTS_INFO);

  log_callback("", ASSFONTS_TEXT);

  is_running_ = false;
}