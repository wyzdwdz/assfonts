#pragma once

#include <QDialog>
#include <QGridLayout>
#include <QIcon>

class CheckWindow : public QDialog {
  Q_OBJECT

 public:
  CheckWindow(QWidget* parent = nullptr) : QDialog(parent) {
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Check update"));
    setWindowIcon(QIcon(":/icon.png"));
    setFixedSize(QSize(380, 140));

    InitLayout();
  }

 private:
  void InitLayout();

  void AddLabels(QGridLayout* layout);

  bool GetLatestVersion(QString& latest_version);
};