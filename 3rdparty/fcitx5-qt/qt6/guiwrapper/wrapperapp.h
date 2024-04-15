/*
 * SPDX-FileCopyrightText: 2012~2012 CSSlayer <wengxt@gmail.com>
 * SPDX-FileCopyrightText: 2017~2017 xzhao <i@xuzhao.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef _GUIWRAPPER_WRAPPERAPP_H_
#define _GUIWRAPPER_WRAPPERAPP_H_

#include "mainwindow.h"
#include <QApplication>

namespace fcitx {

class FcitxQtConfigUIFactory;
class WrapperApp : public QApplication {
    Q_OBJECT
public:
    WrapperApp(int &argc, char **argv);
    virtual ~WrapperApp();

    void init();
public Q_SLOTS:
    void run();

private Q_SLOTS:
    void errorExit();

private:
    FcitxQtConfigUIFactory *factory_;
    MainWindow *mainWindow_;
};
} // namespace fcitx

#endif // _GUIWRAPPER_WRAPPERAPP_H_
