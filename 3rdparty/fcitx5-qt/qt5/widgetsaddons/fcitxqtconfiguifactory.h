/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef _WIDGETSADDONS_FCITXQTCONFIGUIFACTORY_H_
#define _WIDGETSADDONS_FCITXQTCONFIGUIFACTORY_H_

#include <QMap>
#include <QObject>
#include <QStringList>

#include "fcitx5qt5widgetsaddons_export.h"
#include "fcitxqtconfiguiplugin.h"
#include "fcitxqtconfiguiwidget.h"

namespace fcitx {

class FcitxQtConfigUIFactoryPrivate;
/**
 * ui plugin factory.
 **/
class FCITX5QT5WIDGETSADDONS_EXPORT FcitxQtConfigUIFactory : public QObject {
    Q_OBJECT
public:
    /**
     * create a plugin factory
     *
     * @param parent object parent
     **/
    explicit FcitxQtConfigUIFactory(QObject *parent = 0);
    virtual ~FcitxQtConfigUIFactory();
    /**
     * create widget based on file name, it might return 0 if there is no match
     *
     * @param file file name need to be configured
     * @return FcitxQtConfigUIWidget*
     **/
    FcitxQtConfigUIWidget *create(const QString &file);
    /**
     * a simplified version of create, but it just test if there is a valid
     *entry or not
     *
     * @param file file name
     * @return bool
     **/
    bool test(const QString &file);

private:
    FcitxQtConfigUIFactoryPrivate *d_ptr;
    Q_DECLARE_PRIVATE(FcitxQtConfigUIFactory);
};
} // namespace fcitx

#endif // _WIDGETSADDONS_FCITXQTCONFIGUIFACTORY_H_
