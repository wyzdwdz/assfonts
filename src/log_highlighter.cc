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