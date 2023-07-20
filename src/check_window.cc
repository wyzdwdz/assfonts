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

#include "check_window.h"

#include <QEventLoop>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

#include <assfonts.h>

constexpr char VERSION_URL[] =
    "https://raw.githubusercontent.com/wyzdwdz/assfonts/qt/VERSION";
constexpr char DOWNLOAD_URL[] =
    "https://github.com/wyzdwdz/assfonts/releases/latest";

void CheckWindow::InitLayout() {
  QGridLayout* main_layout = new QGridLayout;
  main_layout->setAlignment(Qt::AlignCenter);
  main_layout->setVerticalSpacing(18);

  AddLabels(main_layout);

  setLayout(main_layout);
}

void CheckWindow::AddLabels(QGridLayout* layout) {
  QLabel* current_version_label = new QLabel(tr("Current version: "));
  QLabel* current_version_label_num =
      new QLabel(QString::number(ASSFONTS_VERSION_MAJOR) + "." +
                 QString::number(ASSFONTS_VERSION_MINOR) + "." +
                 QString::number(ASSFONTS_VERSION_PATCH));

  layout->addWidget(current_version_label, 0, 0);
  layout->addWidget(current_version_label_num, 0, 1);

  QString latest_version;
  GetLatestVersion(latest_version);

  QLabel* latest_version_label = new QLabel(tr("Latest version: "));
  QLabel* latest_version_label_num = new QLabel(latest_version);

  layout->addWidget(latest_version_label, 1, 0);
  layout->addWidget(latest_version_label_num, 1, 1);

  QLabel* link = new QLabel(QString("<a href=\"") + DOWNLOAD_URL + "\">" +
                            DOWNLOAD_URL + "</a>");
  link->setTextFormat(Qt::RichText);
  link->setTextInteractionFlags(Qt::TextBrowserInteraction);
  link->setOpenExternalLinks(true);

  layout->addWidget(link, 2, 0, 1, 2);
}

bool CheckWindow::GetLatestVersion(QString& latest_version) {
  QNetworkAccessManager manager;
  QNetworkReply* reply = manager.get(QNetworkRequest(QUrl(VERSION_URL)));

  QEventLoop loop;
  connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  if (reply->error() == QNetworkReply::NoError) {
    QByteArray data = reply->readLine();
    latest_version = QString::fromUtf8(data);

    return true;
  } else {
    latest_version = "Not found";
    return false;
  }
}