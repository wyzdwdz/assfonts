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

#include "main_window.h"

#include <thread>

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenuBar>
#include <QMessageBox>
#include <QMetaType>
#include <QRegularExpression>
#include <ghc/filesystem.hpp>

#ifdef __APPLE__
#include "get_app_support_dir.h"
#elif _WIN32
#include <Shlobj.h>
#else
#endif

namespace fs = ghc::filesystem;

static std::string SAVE_FILES_PATH = []() {
  fs::path path = fs::current_path();

#ifdef __APPLE__
  path = fs::path(GetAppSupportDir()) / "assfonts";
#elif __linux__
  if (getenv("HOME")) {
    path = fs::path(getenv("HOME")) / ".local" / "share" / "assfonts";
  }
#elif _WIN32
  TCHAR sz_path[MAX_PATH];
  SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, sz_path);

  path = fs::path(sz_path) / "assfonts";
#endif

  std::error_code ec;
  fs::create_directory(path, ec);

  return path.u8string();
}();

void MainWindow::closeEvent(QCloseEvent* event) {
  if (worker_->IsRunning()) {
    QMessageBox::warning(this, tr("Warning"), tr("Program is still running"));

    event->ignore();

  } else {
    thread_.quit();
    thread_.wait();

    SaveSettings();

    event->accept();
  }
}

void MainWindow::InitMenu() {
  QMenuBar* menu_bar = new QMenuBar;
  log_menu_ = menu_bar->addMenu(tr("&Log"));
  info_menu_ = menu_bar->addMenu(tr("&Other"));

  log_menu_->setToolTipsVisible(true);
  info_menu_->setToolTipsVisible(true);

  clear_action_ = new QAction(tr("&Clear all"));

  space_action_ = new QAction(tr("&Print space"));
  space_action_->setCheckable(true);
  space_action_->setChecked(true);

  mt_action_ = new QAction(tr("&Multi thread"));
  mt_action_->setCheckable(true);
  mt_action_->setChecked(false);

  combined_action_ = new QAction(tr("&Font combined"));
  combined_action_->setCheckable(true);
  combined_action_->setChecked(false);

  reset_action_ = new QAction(tr("&Reset all"));

  check_action_ = new QAction(tr("&Check update"));

  log_menu_->addAction(clear_action_);
  log_menu_->addAction(space_action_);

  min_level_menu_ = log_menu_->addMenu(tr("&Show log level"));

  min_info_action_ = new QAction(tr(">= INFO"));
  min_info_action_->setCheckable(true);
  min_info_action_->setChecked(true);

  min_warn_action_ = new QAction(tr(">= WARN"));
  min_warn_action_->setCheckable(true);
  min_warn_action_->setChecked(false);

  min_error_action_ = new QAction(tr(">= ERROR"));
  min_error_action_->setCheckable(true);
  min_error_action_->setChecked(false);

  min_level_menu_->addAction(min_info_action_);
  min_level_menu_->addAction(min_warn_action_);
  min_level_menu_->addAction(min_error_action_);

  info_menu_->addAction(mt_action_);
  info_menu_->addAction(combined_action_);
  info_menu_->addAction(reset_action_);
  info_menu_->addAction(check_action_);

  setMenuBar(menu_bar);
}

void MainWindow::InitMainWindowLayout() {
  QHBoxLayout* main_window_hlayout = new QHBoxLayout;

  main_window_hlayout->addSpacing(4);

  QVBoxLayout* main_window_vlayout = new QVBoxLayout;
  main_window_vlayout->setAlignment(Qt::AlignTop);

  AddInputLayout(main_window_vlayout);
  AddOutputLayout(main_window_vlayout);
  AddFontLayout(main_window_vlayout);
  AddDatabaseLayout(main_window_vlayout);
  AddButtonsLayout(main_window_vlayout);
  AddLogLayout(main_window_vlayout);

  main_window_hlayout->addLayout(main_window_vlayout);
  main_window_hlayout->addSpacing(4);

  QWidget* widget = new QWidget;
  widget->setLayout(main_window_hlayout);
  setCentralWidget(widget);
}

void MainWindow::AddInputLayout(QVBoxLayout* layout) {
  QVBoxLayout* vlayout = new QVBoxLayout;

  input_label_ = new QLabel(tr("Input ASS files"));

  QFont font;
  font.setWeight(QFont::Bold);
  input_label_->setFont(font);

  vlayout->addWidget(input_label_);

  QHBoxLayout* hlayout = new QHBoxLayout;

  input_line_ = new DropLineEdit;
  input_line_->setMinimumHeight(25);
  hlayout->addWidget(input_line_);

  input_button_ = new QPushButton("...");
  input_button_->setFixedSize(25, 25);
  hlayout->addWidget(input_button_);

  vlayout->addLayout(hlayout);
  vlayout->setAlignment(Qt::AlignTop);

  layout->addLayout(vlayout);
  layout->addSpacing(8);
}

void MainWindow::AddOutputLayout(QVBoxLayout* layout) {
  QVBoxLayout* vlayout = new QVBoxLayout;

  output_label_ = new QLabel(tr("Output directory"));

  QFont font;
  font.setWeight(QFont::Bold);
  output_label_->setFont(font);

  vlayout->addWidget(output_label_);

  QHBoxLayout* hlayout = new QHBoxLayout;

  output_line_ = new DropLineEdit;
  output_line_->setMinimumHeight(25);
  hlayout->addWidget(output_line_);

  output_button_ = new QPushButton("...");
  output_button_->setFixedSize(25, 25);
  hlayout->addWidget(output_button_);

  vlayout->addLayout(hlayout);
  vlayout->setAlignment(Qt::AlignTop);

  layout->addLayout(vlayout);
  layout->addSpacing(8);
}

void MainWindow::AddFontLayout(QVBoxLayout* layout) {
  QVBoxLayout* vlayout = new QVBoxLayout;

  font_label_ = new QLabel(tr("Font directory"));

  QFont font;
  font.setWeight(QFont::Bold);
  font_label_->setFont(font);

  vlayout->addWidget(font_label_);

  QHBoxLayout* hlayout = new QHBoxLayout;

  font_line_ = new DropLineEdit;
  font_line_->setMinimumHeight(25);
  hlayout->addWidget(font_line_);

  font_button_ = new QPushButton("...");
  font_button_->setFixedSize(25, 25);
  hlayout->addWidget(font_button_);

  vlayout->addLayout(hlayout);
  vlayout->setAlignment(Qt::AlignTop);

  layout->addLayout(vlayout);
  layout->addSpacing(8);
}

void MainWindow::AddDatabaseLayout(QVBoxLayout* layout) {
  QVBoxLayout* vlayout = new QVBoxLayout;

  database_label_ = new QLabel(tr("Database directory"));

  QFont font;
  font.setWeight(QFont::Bold);
  database_label_->setFont(font);

  vlayout->addWidget(database_label_);

  QHBoxLayout* hlayout = new QHBoxLayout;

  database_line_ = new DropLineEdit;
  database_line_->setMinimumHeight(25);

  QDir appdata_dir(QString::fromStdString(SAVE_FILES_PATH));

  database_line_->setText(QDir::toNativeSeparators(appdata_dir.absolutePath()));

  hlayout->addWidget(database_line_);

  database_button_ = new QPushButton("...");
  database_button_->setFixedSize(25, 25);
  hlayout->addWidget(database_button_);

  vlayout->addLayout(hlayout);
  vlayout->setAlignment(Qt::AlignTop);

  layout->addLayout(vlayout);
  layout->addSpacing(16);
}

void MainWindow::AddButtonsLayout(QVBoxLayout* layout) {
  QHBoxLayout* hlayout = new QHBoxLayout;

  hlayout->addSpacing(8);

  hdr_combo_ = new QComboBox;
  hdr_combo_->addItem(tr("No HDR"));
  hdr_combo_->addItem(tr("HDR Low"));
  hdr_combo_->addItem(tr("HDR High"));
  hdr_combo_->setMinimumSize(90, 25);
  hlayout->addWidget(hdr_combo_, 0, Qt::AlignVCenter);
  hlayout->addSpacing(16);

  subset_checkbox_ = new QCheckBox(tr("Subset only"));
  hlayout->addWidget(subset_checkbox_, 0, Qt::AlignVCenter);
  hlayout->addSpacing(8);

  embed_checkbox_ = new QCheckBox(tr("Embed only"));
  hlayout->addWidget(embed_checkbox_, 0, Qt::AlignVCenter);
  hlayout->addSpacing(8);

  rename_checkbox_ = new QCheckBox(tr("Subfonts rename"));
  hlayout->addWidget(rename_checkbox_, 0, Qt::AlignVCenter);
  hlayout->addSpacing(16);
  hlayout->addStretch();

  build_button_ = new CheckableButton(tr("Build Database"));
  build_button_->setMinimumSize(110, 50);
  hlayout->addWidget(build_button_, 0, Qt::AlignVCenter);
  hlayout->addSpacing(10);

  start_button_ = new CheckableButton(tr("Start"));

  QFont font;
  font.setWeight(QFont::Bold);
  start_button_->setFont(font);

  start_button_->setMinimumSize(80, 50);
  hlayout->addWidget(start_button_, 0, Qt::AlignVCenter);

  hlayout->addSpacing(8);

  layout->addLayout(hlayout);
  layout->addSpacing(8);
}

void MainWindow::AddLogLayout(QVBoxLayout* layout) {
  log_text_ = new QTextEdit;

  log_highlighter_ = new LogHighlighter(log_text_->document());

  log_text_->setReadOnly(true);

  QString version_info = "assfonts -- version " +
                         QString::number(ASSFONTS_VERSION_MAJOR) + "." +
                         QString::number(ASSFONTS_VERSION_MINOR) + "." +
                         QString::number(ASSFONTS_VERSION_PATCH);

  log_buffer_.push_back({ASSFONTS_TEXT, version_info});
  log_buffer_.push_back({ASSFONTS_TEXT, ""});
  RefreshLogText();

  layout->addWidget(log_text_, 1);
}

void MainWindow::InitToolTips() {
  hdr_combo_->setToolTip(
      tr("Change HDR subtitle brightness\n"
         " HDR Low  targets 100 nit\n"
         " HDR High  targets 203 nit"));

  subset_checkbox_->setToolTip(
      tr("Subset fonts but not embed them into subtitle"));

  embed_checkbox_->setToolTip(
      tr("Embed fonts into subtitle but not subset them"));

  rename_checkbox_->setToolTip(
      tr("Rename subsetted fonts to ensure the one-to-one\n"
         "correspondence between one subtitle and one series of fonts"));

  build_button_->setToolTip(tr("Build fonts database"));

  start_button_->setToolTip(tr("Start program"));

  mt_action_->setToolTip(
      tr("Enable multi thread mode. May cause huge RAM usage"));

  combined_action_->setToolTip(
      tr("!!Experimental!! When there are multiple input files,\n"
         "combine the subsetted fonts with the same fontname together"));
}

void MainWindow::InitAllConnects() {
  qRegisterMetaType<ASSFONTS_LOG_LEVEL>("ASSFONTS_LOG_LEVEL");

  connect(input_button_, &QPushButton::clicked, this,
          &MainWindow::OnInputButtonClicked);
  connect(output_button_, &QPushButton::clicked, this,
          &MainWindow::OnOutputButtonClicked);
  connect(font_button_, &QPushButton::clicked, this,
          &MainWindow::OnFontButtonClicked);
  connect(database_button_, &QPushButton::clicked, this,
          &MainWindow::OnDatabaseButtonClicked);

  connect(input_line_, &DropLineEdit::OnSendDrop, this,
          &MainWindow::OnInputLineDrop);
  connect(output_line_, &DropLineEdit::OnSendDrop, this,
          &MainWindow::OnOutputLineDrop);
  connect(font_line_, &DropLineEdit::OnSendDrop, this,
          &MainWindow::OnFontLineDrop);
  connect(database_line_, &DropLineEdit::OnSendDrop, this,
          &MainWindow::OnDatabaseLineDrop);

  connect(build_button_, &CheckableButton::OnSendPressed, this,
          &MainWindow::OnBuildButtonPressed);
  connect(start_button_, &CheckableButton::OnSendPressed, this,
          &MainWindow::OnStartButtonPressed);

  connect(this, &MainWindow::OnSendBuild, worker_, &TaskRunner::OnBuildRun);
  connect(this, &MainWindow::OnSendStart, worker_, &TaskRunner::OnStartRun);

  connect(worker_, &TaskRunner::OnSendBuildRelease, this,
          [this]() { build_button_->setChecked(false); });
  connect(worker_, &TaskRunner::OnSendStartRelease, this,
          [this]() { start_button_->setChecked(false); });

  connect(worker_, &TaskRunner::OnSendLog, this, &MainWindow::OnReceiveLog);

  connect(worker_, &TaskRunner::OnSendClearFont, this,
          [this]() { font_line_->clear(); });

  connect(clear_action_, &QAction::triggered, this,
          &MainWindow::OnClearActionTrigger);

  connect(space_action_, &QAction::triggered, this,
          [this]() { RefreshLogText(); });

  connect(min_info_action_, &QAction::triggered, this,
          &MainWindow::OnInfoActionTrigger);
  connect(min_warn_action_, &QAction::triggered, this,
          &MainWindow::OnWarnActionTrigger);
  connect(min_error_action_, &QAction::triggered, this,
          &MainWindow::OnErrorActionTrigger);

  connect(reset_action_, &QAction::triggered, this,
          &MainWindow::OnResetActionTrigger);

  connect(check_action_, &QAction::triggered, this,
          &MainWindow::OnCheckActionTrigger);
}

void MainWindow::InitWorker() {
  worker_ = &TaskRunner::GetInstance();
  worker_->moveToThread(&thread_);

  thread_.start();
}

void MainWindow::OnInputButtonClicked() {
  QString start_path;
  QFileInfo file(input_line_->text().split(";").at(0).trimmed());
  QDir parent_dir = file.dir();
  if (parent_dir.path() != "." && parent_dir.exists()) {
    start_path = parent_dir.path();
  } else {
    start_path = QDir::homePath();
  }

  QStringList path_list = QFileDialog::getOpenFileNames(
      this, tr("Choose Files - Input ASS Files"), start_path,
      tr("Sub Station Alpha (*.ass *.ssa)"));

  if (path_list.empty()) {
    return;
  }

  input_line_->clear();

  for (auto it = path_list.begin(); it != path_list.end(); ++it) {
    if (it != path_list.begin()) {
      input_line_->insert("; ");
    }

    input_line_->insert(QDir::toNativeSeparators(*it));
  }

  QFileInfo file_info(path_list.at(0));
  output_line_->setText(
      QDir::toNativeSeparators(file_info.dir().absolutePath()));
}

void MainWindow::OnOutputButtonClicked() {
  QString start_path;
  QDir dir(output_line_->text().trimmed());
  if (dir.path() != "." && dir.exists()) {
    start_path = dir.path();
  } else {
    start_path = QDir::homePath();
  }

  QString path = QFileDialog::getExistingDirectory(
      this, tr("Choose Directory - Output directory"), start_path);

  if (!path.isEmpty()) {
    output_line_->setText(QDir::toNativeSeparators(path));
  }
}

void MainWindow::OnFontButtonClicked() {
  QString start_path;
  QDir dir(font_line_->text().trimmed());
  if (dir.path() != "." && dir.exists()) {
    start_path = dir.path();
  } else {
    start_path = QDir::homePath();
  }

  QString path = QFileDialog::getExistingDirectory(
      this, tr("Choose Directory - Font directory"), start_path);

  if (!path.isEmpty()) {
    font_line_->setText(QDir::toNativeSeparators(path));
  }
}

void MainWindow::OnDatabaseButtonClicked() {
  QString start_path;
  QDir dir(database_line_->text().trimmed());
  if (dir.path() != "." && dir.exists()) {
    start_path = dir.path();
  } else {
    start_path = QDir::homePath();
  }

  QString path = QFileDialog::getExistingDirectory(
      this, tr("Choose Directory - Database directory"), start_path);

  if (!path.isEmpty()) {
    database_line_->setText(QDir::toNativeSeparators(path));
  }
}

void MainWindow::OnInputLineDrop(QList<QString> paths) {
  for (auto it = paths.begin(); it != paths.end();) {
    QFileInfo file_info(*it);
    QString suffix = file_info.suffix().toLower();

    if (!file_info.isFile() || (suffix != "ass" && suffix != "ssa")) {
      it = paths.erase(it);
    } else {
      ++it;
    }
  }

  if (!paths.isEmpty()) {
    input_line_->clear();
  } else {
    return;
  }

  for (auto it = paths.begin(); it != paths.end(); ++it) {
    if (it != paths.begin()) {
      input_line_->insert("; ");
    }

    input_line_->insert(QDir::toNativeSeparators(*it));
  }

  QFileInfo file_info(paths.at(0));
  output_line_->setText(
      QDir::toNativeSeparators(file_info.dir().absolutePath()));
}

void MainWindow::OnOutputLineDrop(QList<QString> paths) {
  for (auto it = paths.begin(); it != paths.end();) {
    QDir dir(*it);

    if (!dir.exists()) {
      it = paths.erase(it);
    } else {
      ++it;
    }
  }

  if (!paths.isEmpty()) {
    output_line_->setText(QDir::toNativeSeparators(paths.at(0)));
  }
}

void MainWindow::OnFontLineDrop(QList<QString> paths) {
  for (auto it = paths.begin(); it != paths.end();) {
    QDir dir(*it);

    if (!dir.exists()) {
      it = paths.erase(it);
    } else {
      ++it;
    }
  }

  if (!paths.isEmpty()) {
    font_line_->clear();
  } else {
    return;
  }

  for (auto it = paths.begin(); it != paths.end(); ++it) {
    if (it != paths.begin()) {
      font_line_->insert("; ");
    }

    font_line_->insert(QDir::toNativeSeparators(*it));
  }
}

void MainWindow::OnDatabaseLineDrop(QList<QString> paths) {
  for (auto it = paths.begin(); it != paths.end();) {
    QDir dir(*it);

    if (!dir.exists()) {
      it = paths.erase(it);
    } else {
      ++it;
    }
  }

  if (!paths.isEmpty()) {
    database_line_->setText(QDir::toNativeSeparators(paths.at(0)));
  }
}

void MainWindow::OnBuildButtonPressed() {
  if (worker_->IsRunning()) {
    build_button_->setChecked(false);
    return;
  }

  auto font_line = font_line_->text().trimmed();

  emit OnSendBuild(font_line, database_line_->text().trimmed());
}

void MainWindow::OnStartButtonPressed() {
  if (worker_->IsRunning()) {
    start_button_->setChecked(false);
    return;
  }

  unsigned int brightness = 0;
  switch (hdr_combo_->currentIndex()) {
    case 0:
      brightness = 0;
      break;
    case 1:
      brightness = 100;
      break;
    case 2:
      brightness = 203;
      break;
    default:
      brightness = 0;
      break;
  }

  unsigned int num_thread = 1;
  if (mt_action_->isChecked()) {
    num_thread = std::thread::hardware_concurrency() + 1;
  }

  emit OnSendStart(
      input_line_->text().trimmed(), output_line_->text().trimmed(),
      font_line_->text().trimmed(), database_line_->text().trimmed(),
      brightness, subset_checkbox_->isChecked(), embed_checkbox_->isChecked(),
      rename_checkbox_->isChecked(), combined_action_->isChecked(), num_thread);
}

void MainWindow::OnReceiveLog(QString msg, ASSFONTS_LOG_LEVEL log_level) {
  QString log;

  switch (log_level) {
    case ASSFONTS_INFO:
      log = "[INFO] ";
      break;

    case ASSFONTS_WARN:
      log = "[WARN] ";
      break;

    case ASSFONTS_ERROR:
      log = "[ERROR] ";
      break;

    default:
      break;
  }

  log.append(msg);

  log_buffer_.push_back({log_level, log});

  ASSFONTS_LOG_LEVEL show_min_level = ASSFONTS_INFO;

  if (min_info_action_->isChecked()) {
    show_min_level = ASSFONTS_INFO;
  } else if (min_warn_action_->isChecked()) {
    show_min_level = ASSFONTS_WARN;
  } else if (min_error_action_->isChecked()) {
    show_min_level = ASSFONTS_ERROR;
  }

  if (!space_action_->isChecked() && log.isEmpty()) {
    return;
  }

  if (log_level < show_min_level) {
    return;
  }

  QTextCursor cursor(log_text_->document());
  cursor.movePosition(QTextCursor::End);
  cursor.beginEditBlock();
  cursor.insertText(log);
  cursor.insertBlock();
  cursor.endEditBlock();
  cursor.movePosition(QTextCursor::End);
  log_text_->setTextCursor(cursor);
}

void MainWindow::OnClearActionTrigger() {
  log_buffer_.clear();
  RefreshLogText();
}

void MainWindow::OnInfoActionTrigger(bool checked) {
  if (checked) {
    min_warn_action_->setChecked(false);
    min_error_action_->setChecked(false);
  } else {
    min_info_action_->setChecked(true);
  }
  RefreshLogText();
}

void MainWindow::OnWarnActionTrigger(bool checked) {
  if (checked) {
    min_info_action_->setChecked(false);
    min_error_action_->setChecked(false);
  } else {
    min_warn_action_->setChecked(true);
  }
  RefreshLogText();
}

void MainWindow::OnErrorActionTrigger(bool checked) {
  if (checked) {
    min_info_action_->setChecked(false);
    min_warn_action_->setChecked(false);
  } else {
    min_error_action_->setChecked(true);
  }
  RefreshLogText();
}

void MainWindow::OnCheckActionTrigger() {
  if (check_window_ == nullptr) {
    check_window_ = new CheckWindow(this);
    check_window_->setAttribute(Qt::WA_DeleteOnClose);
    connect(check_window_, &QDialog::destroyed, this,
            [this]() { check_window_ = nullptr; });
  }

  check_window_->show();
  check_window_->activateWindow();
}

void MainWindow::RefreshLogText() {
  log_text_->setUpdatesEnabled(false);
  log_text_->clear();

  QTextBlockFormat bf = log_text_->textCursor().blockFormat();
  bf.setLineHeight(3, QTextBlockFormat::LineDistanceHeight);
  log_text_->textCursor().setBlockFormat(bf);

  ASSFONTS_LOG_LEVEL show_min_level = ASSFONTS_INFO;

  if (min_info_action_->isChecked()) {
    show_min_level = ASSFONTS_INFO;
  } else if (min_warn_action_->isChecked()) {
    show_min_level = ASSFONTS_WARN;
  } else if (min_error_action_->isChecked()) {
    show_min_level = ASSFONTS_ERROR;
  }

  QTextCursor cursor(log_text_->document());
  cursor.beginEditBlock();

  for (auto iter = log_buffer_.begin(); iter != log_buffer_.end(); ++iter) {
    if (!space_action_->isChecked() && (*iter).text.isEmpty()) {
      continue;
    }

    if ((*iter).level < show_min_level) {
      continue;
    }

    cursor.insertText((*iter).text);
    cursor.insertBlock();
  }

  cursor.endEditBlock();
  log_text_->setUpdatesEnabled(true);

  cursor.movePosition(QTextCursor::End);
  log_text_->setTextCursor(cursor);
}

void MainWindow::ResizeHelper(QWidget* widget, const int width,
                              const int height, const bool is_fixed) {
  QSize hint_size = widget->sizeHint();

  if (width > hint_size.width()) {
    hint_size.setWidth(width);
  }

  if (height > hint_size.height()) {
    hint_size.setHeight(height);
  }

  if (is_fixed) {
    widget->setFixedSize(hint_size);
  } else {
    widget->resize(hint_size);
  }
}

void MainWindow::LoadSettings() {
  settings_ = new QSettings(
      (fs::path(SAVE_FILES_PATH) / "settings.ini").u8string().c_str(),
      QSettings::IniFormat, this);

  settings_->beginGroup("MainWindow");
  if (settings_->contains("WSize") && settings_->contains("HSize") &&
      settings_->contains("WPos") && settings_->contains("HPos")) {
    int w_size = settings_->value("WSize").toInt();
    int h_size = settings_->value("HSize").toInt();
    int w_pos = settings_->value("WPos").toInt();
    int h_pos = settings_->value("HPos").toInt();
    resize(w_size, h_size);
    move(w_pos, h_pos);
  }
  settings_->endGroup();

  settings_->beginGroup("InputsText");
  if (settings_->contains("Input")) {
    input_line_->setText(settings_->value("Input").toString());
  }
  if (settings_->contains("Output")) {
    output_line_->setText(settings_->value("Output").toString());
  }
  if (settings_->contains("Font")) {
    font_line_->setText(settings_->value("Font").toString());
  }
  if (settings_->contains("Database")) {
    database_line_->setText(settings_->value("Database").toString());
  }
  settings_->endGroup();

  settings_->beginGroup("Options");
  if (settings_->contains("Hdr")) {
    hdr_combo_->setCurrentIndex(settings_->value("Hdr").toInt());
  }
  if (settings_->contains("Subset")) {
    subset_checkbox_->setChecked(settings_->value("Subset").toBool());
  }
  if (settings_->contains("Embed")) {
    embed_checkbox_->setChecked(settings_->value("Embed").toBool());
  }
  if (settings_->contains("Rename")) {
    rename_checkbox_->setChecked(settings_->value("Rename").toBool());
  }
  settings_->endGroup();

  settings_->beginGroup("Menu");

  if (settings_->contains("LogSpace")) {
    space_action_->setChecked(settings_->value("LogSpace").toBool());
    RefreshLogText();
  }

  if (settings_->contains("LogLevel")) {
    switch (settings_->value("LogLevel").toInt()) {
      case 0:
        min_info_action_->trigger();
        break;
      case 1:
        min_warn_action_->trigger();
        break;
      case 2:
        min_error_action_->trigger();
        break;
      default:
        break;
    }
  }

  if (settings_->contains("MultiThread")) {
    mt_action_->setChecked(settings_->value("MultiThread").toBool());
  }

  if (settings_->contains("CombinedFonts")) {
    combined_action_->setChecked(settings_->value("CombinedFonts").toBool());
  }

  settings_->endGroup();
}

void MainWindow::SaveSettings() {
  settings_->beginGroup("MainWindow");
  settings_->setValue("WSize", width());
  settings_->setValue("HSize", height());
  settings_->setValue("WPos", pos().x());
  settings_->setValue("HPos", pos().y());
  settings_->endGroup();

  settings_->beginGroup("InputsText");
  settings_->setValue("Input", input_line_->text());
  settings_->setValue("Output", output_line_->text());
  settings_->setValue("Font", font_line_->text());
  settings_->setValue("Database", database_line_->text());
  settings_->endGroup();

  settings_->beginGroup("Options");
  settings_->setValue("Hdr", hdr_combo_->currentIndex());
  settings_->setValue("Subset", subset_checkbox_->isChecked());
  settings_->setValue("Embed", embed_checkbox_->isChecked());
  settings_->setValue("Rename", rename_checkbox_->isChecked());
  settings_->endGroup();

  settings_->beginGroup("Menu");
  settings_->setValue("LogSpace", space_action_->isChecked());

  if (min_info_action_->isChecked()) {
    settings_->setValue("LogLevel", 0);
  } else if (min_warn_action_->isChecked()) {
    settings_->setValue("LogLevel", 1);
  } else if (min_error_action_->isChecked()) {
    settings_->setValue("LogLevel", 2);
  }

  settings_->setValue("MultiThread", mt_action_->isChecked());

  settings_->setValue("CombinedFonts", combined_action_->isChecked());
  settings_->endGroup();
}

void MainWindow::OnResetActionTrigger() {
  input_line_->clear();
  output_line_->clear();
  font_line_->clear();
  database_line_->clear();

  hdr_combo_->setCurrentIndex(0);
  subset_checkbox_->setChecked(false);
  embed_checkbox_->setChecked(false);
  rename_checkbox_->setChecked(false);

  space_action_->setChecked(true);

  min_info_action_->setChecked(true);
  min_warn_action_->setChecked(false);
  min_error_action_->setChecked(false);

  mt_action_->setChecked(false);
  combined_action_->setChecked(false);

  log_buffer_.clear();
  QString version_info = "assfonts -- version " +
                         QString::number(ASSFONTS_VERSION_MAJOR) + "." +
                         QString::number(ASSFONTS_VERSION_MINOR) + "." +
                         QString::number(ASSFONTS_VERSION_PATCH);
  log_buffer_.push_back({ASSFONTS_TEXT, version_info});
  log_buffer_.push_back({ASSFONTS_TEXT, ""});
  RefreshLogText();
  RefreshLogText();
}