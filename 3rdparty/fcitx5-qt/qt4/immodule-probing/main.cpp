/*
 * SPDX-FileCopyrightText: 2023~2023 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#include <QApplication>
#include <QInputContext>
#include <iostream>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    auto *inputContext = app.inputContext();
    std::cout << "QT_IM_MODULE=";
    if (inputContext) {
        std::cout << inputContext->identifierName().toUtf8().constData();
    }
    std::cout << std::endl;
    return 0;
}
