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

#include <QMouseEvent>
#include <QPushButton>

class CheckableButton : public QPushButton {
  Q_OBJECT

 public:
  CheckableButton(QWidget* parent = nullptr) : QPushButton(parent) {
    setCheckable(true);
  }

  CheckableButton(const QString& text, QWidget* parent = nullptr)
      : QPushButton(text, parent) {
    setCheckable(true);
  }

 signals:
  void OnSendPressed();

 protected:
  void mousePressEvent(QMouseEvent* e) override;
};