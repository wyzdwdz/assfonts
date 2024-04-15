/*
 * SPDX-FileCopyrightText: 2023~2023 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#include <QGuiApplication>
#include <iostream>
#include <private/qguiapplication_p.h>
#include <qpa/qplatforminputcontext.h>
#include <qpa/qplatforminputcontextfactory_p.h>
#include <qpa/qplatformintegration.h>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    std::cout << "QT_QPA_PLATFORM=" << app.platformName().toStdString()
              << std::endl;
    std::cout << "QT_IM_MODULE="
              << QPlatformInputContextFactory::requested().toStdString()
              << std::endl;
    auto inputContext =
        QGuiApplicationPrivate::platformIntegration()->inputContext();
    std::cout << "IM_MODULE_CLASSNAME=";
    if (inputContext) {
        std::cout << inputContext->metaObject()->className();
    }
    std::cout << std::endl;
    return 0;
}
