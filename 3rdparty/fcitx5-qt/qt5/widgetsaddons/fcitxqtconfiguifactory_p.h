/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef _WIDGETSADDONS_FCITXQTCONFIGUIFACTORY_P_H_
#define _WIDGETSADDONS_FCITXQTCONFIGUIFACTORY_P_H_

#include "fcitxqtconfiguifactory.h"
#include <QObject>
#include <QPluginLoader>
#include <qpluginloader.h>

namespace fcitx {

class FcitxQtConfigUIFactoryPrivate : public QObject {
    Q_OBJECT
public:
    FcitxQtConfigUIFactoryPrivate(FcitxQtConfigUIFactory *conn);
    virtual ~FcitxQtConfigUIFactoryPrivate();
    FcitxQtConfigUIFactory *const q_ptr;
    Q_DECLARE_PUBLIC(FcitxQtConfigUIFactory);

private:
    void scan();
    QMap<QString, QPluginLoader *> plugins_;
};
} // namespace fcitx

#endif // _WIDGETSADDONS_FCITXQTCONFIGUIFACTORY_P_H_
