/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef _WIDGETSADDONS_FCITXQTCONFIGUIWIDGET_H_
#define _WIDGETSADDONS_FCITXQTCONFIGUIWIDGET_H_

#include "fcitx5qt5widgetsaddons_export.h"
#include <QtWidgets/QWidget>

namespace fcitx {

/**
 * embedded gui for custom configuration
 **/
class FCITX5QT5WIDGETSADDONS_EXPORT FcitxQtConfigUIWidget : public QWidget {
    Q_OBJECT
public:
    explicit FcitxQtConfigUIWidget(QWidget *parent = 0);

    /**
     * load the configuration, usually, this is being called upon a "reset"
     *button clicked
     * the outer gui will not call it for you for the first time, your
     *initialization might
     * want to call it by yourself.
     *
     * @return void
     **/
    virtual void load() = 0;

    /**
     * save the configuration
     *
     * @see asyncSave saveFinished
     **/
    virtual void save() = 0;

    /**
     * window title
     *
     * @return window title
     **/
    virtual QString title() = 0;

    /**
     * return the icon name of the window, see QIcon::fromTheme
     *
     * @return icon name
     **/
    virtual QString icon();

    /**
     * return the save function is async or not, default implementation is false
     *
     * @return bool
     **/
    virtual bool asyncSave();

Q_SIGNALS:
    /**
     * the configuration is changed or not, used to indicate parent gui
     *
     * @param changed is config changed
     **/
    void changed(bool changed);

    /**
     * if asyncSave return true, be sure to Q_EMIT this signal on all case
     *
     * @see asyncSave
     **/
    void saveFinished();

    /// Save config for a specified path.
    void saveSubConfig(const QString &path);
};
} // namespace fcitx

#endif // _WIDGETSADDONS_FCITXQTCONFIGUIWIDGET_H_
