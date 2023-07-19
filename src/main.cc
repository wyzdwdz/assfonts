#include <QApplication>
#include <QFile>
#include <QStyleFactory>
#include <QtPlugin>

#include "main_window.h"

#ifdef __linux__
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
#elif _WIN32
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#elif __APPLE__
Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin)
#endif

int main(int argc, char** argv) {
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
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