/*
 * SPDX-FileCopyrightText: 2013~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "batchdialog.h"
#include "ui_batchdialog.h"
#include <QIcon>
#include <fcitx-utils/i18n.h>

namespace fcitx {
BatchDialog::BatchDialog(QWidget *parent) : QDialog(parent) {
    setupUi(this);
    iconLabel->setPixmap(QIcon::fromTheme("dialog-information").pixmap(22, 22));
}

BatchDialog::~BatchDialog() {}

void BatchDialog::setText(const QString &s) { plainTextEdit->setPlainText(s); }

QString BatchDialog::text() const { return plainTextEdit->toPlainText(); }
} // namespace fcitx
