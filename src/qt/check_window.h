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

#pragma once

#include <QDialog>
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QtConcurrent>

class CheckWindow : public QDialog {
  Q_OBJECT

 public:
  CheckWindow(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags())
      : QDialog(parent, f) {

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Check update"));
    setWindowIcon(QIcon(":/icon.png"));

    InitLayout();

    setFixedSize(sizeHint());

    InitFetching();
  }

 private:
  QLabel* latest_version_label_num_;
  QFutureWatcher<QString>* watcher_;

  void InitLayout();

  void AddLabels(QGridLayout* layout);

  void InitFetching();

  QString GetLatestVersion();
};