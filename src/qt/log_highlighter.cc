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
  warn_format_.setForeground(Qt::blue);
  error_format_.setForeground(Qt::red);

  info_expression_ = QRegularExpression("^\\[INFO\\].*$");
  warn_expression_ = QRegularExpression("^\\[WARN\\].*$");
  error_expression_ = QRegularExpression("^\\[ERROR\\].*$");
}

void LogHighlighter::highlightBlock(const QString& text) {
  QRegularExpressionMatchIterator i = info_expression_.globalMatch(text);
  while (i.hasNext()) {
    QRegularExpressionMatch match = i.next();
    setFormat(match.capturedStart(), match.capturedLength(), info_format_);
  }

  i = warn_expression_.globalMatch(text);
  while (i.hasNext()) {
    QRegularExpressionMatch match = i.next();
    setFormat(match.capturedStart(), match.capturedLength(), warn_format_);
  }

  i = error_expression_.globalMatch(text);
  while (i.hasNext()) {
    QRegularExpressionMatch match = i.next();
    setFormat(match.capturedStart(), match.capturedLength(), error_format_);
  }
}