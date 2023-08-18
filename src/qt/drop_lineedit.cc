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

#include "drop_lineedit.h"

#include <QMimeData>

void DropLineEdit::dragEnterEvent(QDragEnterEvent* event) {
  if (event->mimeData()->hasUrls()) {
    event->acceptProposedAction();
  }
}

void DropLineEdit::dropEvent(QDropEvent* event) {
  const QMimeData* mime_data = event->mimeData();

  if (mime_data->hasUrls()) {
    QList<QUrl> url_list = mime_data->urls();
    QList<QString> path_list;

    for (const auto& url : url_list) {
      QString path = url.toLocalFile();

      if (!path.isEmpty()) {
        path_list.push_back(path);
      }
    }

    if (!path_list.isEmpty()) {
      emit OnSendDrop(path_list);
    }
  }
}