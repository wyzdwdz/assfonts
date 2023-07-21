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

#include "main_window.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenuBar>
#include <QMetaType>
#include <QStandardPaths>

constexpr char APP_NAME[] = "assfonts";

void MainWindow::InitMenu() {
  QMenuBar* menu_bar = new QMenuBar;
  log_menu_ = menu_bar->addMenu(tr("&Log"));
  info_menu_ = menu_bar->addMenu(tr("&Other"));

  clear_action_ = new QAction(tr("&Clear all"));

  space_action_ = new QAction(tr("&Print space"));
  space_action_->setCheckable(true);
  space_action_->setChecked(true);

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

  QWidget* widget = new QWidget();
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

  input_line_ = new QLineEdit;
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

  output_line_ = new QLineEdit;
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

  font_line_ = new QLineEdit;
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

  database_line_ = new QLineEdit;
  database_line_->setMinimumHeight(25);

  QString generic_data_path =
      QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation)
          .at(0);

  QDir appdata_dir(generic_data_path + QDir::separator() + APP_NAME);
  if (!appdata_dir.exists()) {
    appdata_dir.mkdir();
  }

  database_line_->setText(appdata_dir.cleanPath());

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

  build_button_ = new QPushButton(tr("Build Database"));
  build_button_->setMinimumSize(110, 50);
  hlayout->addWidget(build_button_, 0, Qt::AlignVCenter);
  hlayout->addSpacing(10);

  start_button_ = new QPushButton(tr("Start"));

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
}

void MainWindow::InitAllConnects() {
  qRegisterMetaType<ASSFONTS_LOG_LEVEL>("ASSFONTS_LOG_LEVEL");

  connect(input_button_, &QPushButton::released, this,
          &MainWindow::OnInputButtonReleased);
  connect(output_button_, &QPushButton::released, this,
          &MainWindow::OnOutputButtonReleased);
  connect(font_button_, &QPushButton::released, this,
          &MainWindow::OnFontButtonReleased);
  connect(database_button_, &QPushButton::released, this,
          &MainWindow::OnDatabaseButtonReleased);

  connect(build_button_, &QPushButton::released, this,
          &MainWindow::OnBuildButtonReleased);
  connect(start_button_, &QPushButton::released, this,
          &MainWindow::OnStartButtonReleased);

  connect(this, &MainWindow::OnSendBuild, worker_, &TaskRunner::OnBuildRun);
  connect(this, &MainWindow::OnSendStart, worker_, &TaskRunner::OnStartRun);

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

  connect(check_action_, &QAction::triggered, this,
          &MainWindow::OnCheckActionTrigger);
  connect(check_window_, &QDialog::destroyed, this,
          [this]() { check_window_ = nullptr; });
}

void MainWindow::InitWorker() {
  worker_ = &TaskRunner::GetInstance();
  worker_->moveToThread(&thread_);

  thread_.start();
}

void MainWindow::OnInputButtonReleased() {
  QString start_path;
  QFileInfo file(input_label_.text().split(";").at(0).trimmed());
  QDir parent_dir = file.dir();
  if (parent_dir.exists()) {
    start_path = parent_dir.cleanPath();
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

void MainWindow::OnOutputButtonReleased() {
  QString start_path;
  QDir dir(output_label_.text().trimmed());
  if (dir.exists()) {
    start_path = dir.cleanPath();
  } else {
    start_path = QDir::homePath();
  }

  QString path = QFileDialog::getExistingDirectory(
      this, tr("Choose Directory - Output directory"));

  if (!path.isEmpty()) {
    output_line_->setText(QDir::toNativeSeparators(path));
  }
}

void MainWindow::OnFontButtonReleased() {
  QString start_path;
  QDir dir(font_label_.text().trimmed());
  if (dir.exists()) {
    start_path = dir.cleanPath();
  } else {
    start_path = QDir::homePath();
  }

  QString path = QFileDialog::getExistingDirectory(
      this, tr("Choose Directory - Font directory"));

  if (!path.isEmpty()) {
    font_line_->setText(QDir::toNativeSeparators(path));
  }
}

void MainWindow::OnDatabaseButtonReleased() {
  QString start_path;
  QDir dir(database_label_.text().trimmed());
  if (dir.exists()) {
    start_path = dir.cleanPath();
  } else {
    start_path = QDir::homePath();
  }

  QString path = QFileDialog::getExistingDirectory(
      this, tr("Choose Directory - Database directory"));

  if (!path.isEmpty()) {
    database_line_->setText(QDir::toNativeSeparators(path));
  }
}

void MainWindow::OnBuildButtonReleased() {
  if (worker_->IsRunning()) {
    return;
  }

  emit OnSendBuild(font_line_->text().trimmed(),
                   database_line_->text().trimmed());
}

void MainWindow::OnStartButtonReleased() {
  if (worker_->IsRunning()) {
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

  emit OnSendStart(input_line_->text().trimmed(),
                   output_line_->text().trimmed(), font_line_->text().trimmed(),
                   database_line_->text().trimmed(), brightness,
                   subset_checkbox_->isChecked(), embed_checkbox_->isChecked(),
                   rename_checkbox_->isChecked());
}

void MainWindow::OnReceiveLog(QString msg, ASSFONTS_LOG_LEVEL log_level) {
  log_buffer_.push_back({log_level, msg});
  RefreshLogText();
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
  }

  check_window_->show();
  check_window_->activateWindow();
}

void MainWindow::RefreshLogText() {
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

  for (const auto& item : log_buffer_) {
    if (!space_action_->isChecked() && item.text.isEmpty()) {
      continue;
    }

    if (item.level < show_min_level) {
      continue;
    }

    log_text_->append(item.text);
  }
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
    widget->setMinimumSize(hint_size);
  }
}