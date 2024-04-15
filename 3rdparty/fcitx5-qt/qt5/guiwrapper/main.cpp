/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 * SPDX-FileCopyrightText: 2017~2017 xzhao <i@xuzhao.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "wrapperapp.h"
#include <QCommandLineParser>
#include <locale.h>

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");
    QGuiApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    fcitx::WrapperApp app(argc, argv);
    app.init();
    return app.exec();
}
