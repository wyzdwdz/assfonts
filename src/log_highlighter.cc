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

#include "log_highlighter.h"

LogHighlighter::LogHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent) {
  info_format_.setForeground(Qt::black);
  warn_format_.setForeground(Qt::blue);
  error_format_.setForeground(Qt::red);

  info_expression_ = QRegExp("^\\[INFO\\].*$");
  warn_expression_ = QRegExp("^\\[WARN\\].*$");
  error_expression_ = QRegExp("^\\[ERROR\\].*$");
}

void LogHighlighter::highlightBlock(const QString& text) {
  int index = text.indexOf(info_expression_);
  while (index >= 0) {
    int length = info_expression_.matchedLength();
    setFormat(index, length, info_format_);
    index = text.indexOf(info_expression_, index + length);
  }

  index = text.indexOf(warn_expression_);
  while (index >= 0) {
    int length = warn_expression_.matchedLength();
    setFormat(index, length, warn_format_);
    index = text.indexOf(warn_expression_, index + length);
  }

  index = text.indexOf(error_expression_);
  while (index >= 0) {
    int length = error_expression_.matchedLength();
    setFormat(index, length, error_format_);
    index = text.indexOf(error_expression_, index + length);
  }
}