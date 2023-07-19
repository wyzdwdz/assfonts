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

#include <QSyntaxHighlighter>

class LogHighlighter : public QSyntaxHighlighter {
  Q_OBJECT

 public:
  LogHighlighter(QTextDocument* parent = 0);

 protected:
  void highlightBlock(const QString& text) override;

 private:
  QTextCharFormat info_format_, warn_format_, error_format_;
  QRegExp info_expression_, warn_expression_, error_expression_;
};