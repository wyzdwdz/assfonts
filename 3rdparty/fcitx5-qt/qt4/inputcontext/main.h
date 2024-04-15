/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef MAIN_H
#define MAIN_H

#include <QInputContextPlugin>
#include <QStringList>

#include "qfcitxinputcontext.h"

namespace fcitx {

class QFcitxInputContextPlugin : public QInputContextPlugin {
    Q_OBJECT
public:
    QStringList keys() const override;

    QStringList languages(const QString &key) override;

    QString description(const QString &key) override;

    QInputContext *create(const QString &key) override;

    QString displayName(const QString &key) override;
};
} // namespace fcitx

#endif // MAIN_H
