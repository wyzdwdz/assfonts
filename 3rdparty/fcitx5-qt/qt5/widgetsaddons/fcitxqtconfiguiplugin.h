/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef _WIDGETSADDONS_FCITXQTCONFIGUIPLUGIN_H_
#define _WIDGETSADDONS_FCITXQTCONFIGUIPLUGIN_H_

#include "fcitx5qt5widgetsaddons_export.h"
#include <QObject>
#include <QString>
#include <QStringList>

namespace fcitx {

class FcitxQtConfigUIWidget;

/**
 * interface for qt config ui
 */
struct FCITX5QT5WIDGETSADDONS_EXPORT FcitxQtConfigUIFactoryInterface {
    /**
     * create new widget based on key
     *
     * @see FcitxQtConfigUIPlugin::files
     *
     * @return plugin name
     */
    virtual FcitxQtConfigUIWidget *create(const QString &key) = 0;
};

#define FcitxQtConfigUIFactoryInterface_iid                                    \
    "org.fcitx.Fcitx.FcitxQtConfigUIFactoryInterface"
} // namespace fcitx

Q_DECLARE_INTERFACE(fcitx::FcitxQtConfigUIFactoryInterface,
                    FcitxQtConfigUIFactoryInterface_iid)
namespace fcitx {

/**
 * base class for qt config ui
 */
class FCITX5QT5WIDGETSADDONS_EXPORT FcitxQtConfigUIPlugin
    : public QObject,
      public FcitxQtConfigUIFactoryInterface {
    Q_OBJECT
    Q_INTERFACES(fcitx::FcitxQtConfigUIFactoryInterface)
public:
    explicit FcitxQtConfigUIPlugin(QObject *parent = 0);
    virtual ~FcitxQtConfigUIPlugin();
};
} // namespace fcitx

#endif // _WIDGETSADDONS_FCITXQTCONFIGUIPLUGIN_H_
