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

#include <curl/curl.h>

#include <assfonts.h>

constexpr char VERSION_URL[] =
    "https://raw.githubusercontent.com/wyzdwdz/assfonts/HEAD/VERSION";
constexpr char DOWNLOAD_URL[] =
    "https://github.com/wyzdwdz/assfonts/releases/latest";

class Curl {
 public:
  Curl() { curl_ = curl_easy_init(); }
  ~Curl() { curl_easy_cleanup(curl_); }

  bool Download(const std::string& url, std::string& data) {
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &data);
    curl_easy_setopt(curl_, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    CURLcode res = curl_easy_perform(curl_);
    qDebug() << res;
    if (res == CURLE_OK) {
      return true;
    } else {
      return false;
    }
  }

 private:
  CURL* curl_;

  static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb,
                                    void* userp) {
    size_t realsize = size * nmemb;
    auto& mem = *static_cast<std::string*>(userp);
    mem.append(static_cast<char*>(contents), realsize);
    return realsize;
  }
};

void CheckWindow::InitLayout() {
  QGridLayout* main_layout = new QGridLayout;
  main_layout->setAlignment(Qt::AlignCenter);
  main_layout->setVerticalSpacing(15);
  main_layout->setContentsMargins(25, 15, 25, 20);

  AddLabels(main_layout);

  setLayout(main_layout);
}

void CheckWindow::AddLabels(QGridLayout* layout) {
  QLabel* current_version_label = new QLabel(tr("Current version: "));
  QLabel* current_version_label_num =
      new QLabel(QString::number(ASSFONTS_VERSION_MAJOR) + "." +
                     QString::number(ASSFONTS_VERSION_MINOR) + "." +
                     QString::number(ASSFONTS_VERSION_PATCH),
                 this);

  layout->addWidget(current_version_label, 0, 0);
  layout->addWidget(current_version_label_num, 0, 1);

  QLabel* latest_version_label = new QLabel(tr("Latest version: "));
  latest_version_label_num_ = new QLabel(tr("Fetching ..."));

  layout->addWidget(latest_version_label, 1, 0);
  layout->addWidget(latest_version_label_num_, 1, 1);

  QLabel* link = new QLabel(QString("<a href=\"") + DOWNLOAD_URL + "\">" +
                            DOWNLOAD_URL + "</a>");
  link->setTextFormat(Qt::RichText);
  link->setTextInteractionFlags(Qt::TextBrowserInteraction);
  link->setOpenExternalLinks(true);

  layout->addWidget(link, 2, 0, 1, 2);
}

void CheckWindow::InitFetching() {
  watcher_ = new QFutureWatcher<QString>(this);
  connect(watcher_, &QFutureWatcher<QString>::finished, this,
          [this]() { latest_version_label_num_->setText(watcher_->result()); });

  QFuture<QString> future =
      QtConcurrent::run(this, &CheckWindow::GetLatestVersion);
  watcher_->setFuture(future);
}

QString CheckWindow::GetLatestVersion() {
  Curl curl;

  std::string data;
  if (!curl.Download(VERSION_URL, data)) {
    return tr("Not found");
  }

  return QString::fromStdString(data).trimmed();
}