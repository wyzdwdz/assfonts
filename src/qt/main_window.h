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

#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QMainWindow>
#include <QMutex>
#include <QPushButton>
#include <QTextEdit>
#include <QThread>

#include "check_window.h"
#include "checkable_button.h"
#include "drop_lineedit.h"
#include "log_highlighter.h"
#include "task_runner.h"

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget* parent = nullptr,
             Qt::WindowFlags flags = Qt::WindowFlags())
      : QMainWindow(parent, flags) {

    setWindowTitle(tr("assfonts"));
    setWindowIcon(QIcon(":/icon.png"));

    InitMenu();
    InitMainWindowLayout();
    InitToolTips();
    InitWorker();
    InitAllConnects();

    ResizeHelper(this, 700, 700);

    LoadSettings();
  }

 protected:
  void closeEvent(QCloseEvent* event) override;
  void changeEvent(QEvent* event) override;

 signals:
  void OnSendBuild(QString fonts_path, QString db_path);
  void OnSendStart(QString inputs_path, QString output_path, QString fonts_path,
                   QString db_path, unsigned int brightness,
                   bool is_subset_only, bool is_embed_only, bool is_rename,
                   bool is_font_combined, unsigned int num_thread);

 private:
  struct LogItem {
    ASSFONTS_LOG_LEVEL level;
    QString text;
  };

  QMenu* log_menu_;
  QMenu* info_menu_;

  QAction* clear_action_;
  QAction* space_action_;
  QAction* reset_action_;
  QAction* check_action_;

  QMenu* min_level_menu_;
  QAction* min_info_action_;
  QAction* min_warn_action_;
  QAction* min_error_action_;

  QAction* mt_action_;

  QAction* combined_action_;

  QLabel* input_label_;
  QLabel* output_label_;
  QLabel* font_label_;
  QLabel* database_label_;

  DropLineEdit* input_line_;
  DropLineEdit* output_line_;
  DropLineEdit* font_line_;
  DropLineEdit* database_line_;

  QPushButton* input_button_;
  QPushButton* output_button_;
  QPushButton* font_button_;
  QPushButton* database_button_;

  QComboBox* hdr_combo_;
  QCheckBox* subset_checkbox_;
  QCheckBox* embed_checkbox_;
  QCheckBox* rename_checkbox_;

  CheckableButton* build_button_;
  CheckableButton* start_button_;

  QTextEdit* log_text_;
  LogHighlighter* log_highlighter_;
  QVector<LogItem> log_buffer_;

  QThread thread_;
  TaskRunner* worker_;

  CheckWindow* check_window_ = nullptr;

  QSettings* settings_;

  void InitMenu();
  void InitMainWindowLayout();

  void AddInputLayout(QVBoxLayout* layout);
  void AddOutputLayout(QVBoxLayout* layout);
  void AddFontLayout(QVBoxLayout* layout);
  void AddDatabaseLayout(QVBoxLayout* layout);
  void AddButtonsLayout(QVBoxLayout* layout);
  void AddLogLayout(QVBoxLayout* layout);

  void InitToolTips();

  void InitAllConnects();
  void InitWorker();

  void OnInputButtonClicked();
  void OnOutputButtonClicked();
  void OnFontButtonClicked();
  void OnDatabaseButtonClicked();

  void OnInputLineDrop(QList<QString> paths);
  void OnOutputLineDrop(QList<QString> paths);
  void OnFontLineDrop(QList<QString> paths);
  void OnDatabaseLineDrop(QList<QString> paths);

  void OnBuildButtonPressed();
  void OnStartButtonPressed();

  void OnReceiveLog(QString msg, ASSFONTS_LOG_LEVEL log_level);

  void OnClearActionTrigger();

  void OnInfoActionTrigger(bool checked);
  void OnWarnActionTrigger(bool checked);
  void OnErrorActionTrigger(bool checked);

  void OnResetActionTrigger();

  void OnCheckActionTrigger();

  void RefreshLogText();

  void ResizeHelper(QWidget* widget, const int width, const int height,
                    const bool is_fixed = false);

  void LoadSettings();
  void SaveSettings();
};