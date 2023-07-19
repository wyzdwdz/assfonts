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