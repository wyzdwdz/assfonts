/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "main.h"

fcitx::QFcitxPlatformInputContext *
QFcitx5PlatformInputContextPlugin::create(const QString &system,
                                          const QStringList &paramList) {
    Q_UNUSED(paramList);
    if (system.compare(system, QStringLiteral("fcitx5"), Qt::CaseInsensitive) ==
            0 ||
        system.compare(system, QStringLiteral("fcitx"), Qt::CaseInsensitive) ==
            0) {
        return new fcitx::QFcitxPlatformInputContext;
    }
    return 0;
}
