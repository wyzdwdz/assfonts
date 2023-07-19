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
                  const bool is_embed_only, const bool is_rename);

 signals:
  void OnSendLog(QString msg, ASSFONTS_LOG_LEVEL log_level);
  void OnSendClearFont();

 private:
  bool is_running_ = false;

  TaskRunner() = default;
  ~TaskRunner() = default;
};