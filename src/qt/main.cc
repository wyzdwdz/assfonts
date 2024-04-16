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

#include <QApplication>
#include <QFile>
#include <QStyleFactory>
#include <QtPlugin>

#include "main_window.h"

#ifdef __linux__
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
Q_IMPORT_PLUGIN(QIbusPlatformInputContextPlugin)
Q_IMPORT_PLUGIN(QComposePlatformInputContextPlugin)
Q_IMPORT_PLUGIN(QFcitx5PlatformInputContextPlugin)
#elif _WIN32
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#elif __APPLE__
Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin)
#endif

int main(int argc, char** argv) {
  qputenv("QT_QPA_PLATFORM", "windows:darkmode=0");
  
  QApplication::setHighDpiScaleFactorRoundingPolicy(
      Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

  QApplication app(argc, argv);

  app.setStyle(QStyleFactory::create("fusion"));

  QFile style_file(":/style.qss");
  style_file.open(QFile::ReadOnly);

  QString style = style_file.readAll();
  app.setStyleSheet(style);

  MainWindow main_window;

  main_window.show();

  return app.exec();
}