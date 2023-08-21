/*  This file is part of assfonts.
 *
 *  assfonts is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation,
 *  either version 3 of the License,
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

#pragma once

#include <QObject>

#include "assfonts.h"

class TaskRunner : public QObject {
  Q_OBJECT

 public:
  static TaskRunner& GetInstance() {
    static TaskRunner instance;
    return instance;
  }

  TaskRunner(const TaskRunner&) = delete;
  TaskRunner(TaskRunner&&) = delete;
  TaskRunner& operator=(const TaskRunner&) = delete;
  TaskRunner& operator=(TaskRunner&&) = delete;

  inline bool IsRunning() { return is_running_; }

  void OnBuildRun(const QString fonts_path, const QString db_path);

  void OnStartRun(const QString inputs_path, const QString output_path,
                  const QString fonts_path, const QString db_path,
                  const unsigned int brightness, const bool is_subset_only,
                  const bool is_embed_only, const bool is_rename,
                  const unsigned int num_thread);

 signals:
  void OnSendLog(QString msg, ASSFONTS_LOG_LEVEL log_level);
  void OnSendClearFont();
  void OnSendBuildRelease();
  void OnSendStartRelease();

 private:
  bool is_running_ = false;

  TaskRunner(QObject* parent = nullptr) : QObject(parent){};
  ~TaskRunner() = default;
};