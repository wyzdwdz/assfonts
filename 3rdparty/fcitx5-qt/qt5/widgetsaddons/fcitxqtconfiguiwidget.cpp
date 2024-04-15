/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "fcitxqtconfiguiwidget.h"

namespace fcitx {

FcitxQtConfigUIWidget::FcitxQtConfigUIWidget(QWidget *parent)
    : QWidget(parent) {}

QString FcitxQtConfigUIWidget::icon() { return QLatin1String("fcitx"); }

bool FcitxQtConfigUIWidget::asyncSave() { return false; }
} // namespace fcitx
