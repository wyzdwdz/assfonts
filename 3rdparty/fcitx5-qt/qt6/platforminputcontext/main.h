/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef MAIN_H
#define MAIN_H

#include <QStringList>
#include <qpa/qplatforminputcontextplugin_p.h>

#include "qfcitxplatforminputcontext.h"

class QFcitx5PlatformInputContextPlugin : public QPlatformInputContextPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QPlatformInputContextFactoryInterface_iid FILE
                          FCITX_PLUGIN_DATA_FILE_PATH)
public:
    fcitx::QFcitxPlatformInputContext *
    create(const QString &system, const QStringList &paramList) override;
};

#endif // MAIN_H
