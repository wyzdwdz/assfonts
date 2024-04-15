/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#include "qtkeytrans.h"
#include <QApplication>
#include <fcitx-utils/keysym.h>
#include <fcitx-utils/log.h>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    int sym;
    unsigned int states;
    fcitx::keyQtToSym(Qt::Key_Space, {}, " ", sym, states);

    FCITX_ASSERT(sym == FcitxKey_space) << sym;
    FCITX_ASSERT(static_cast<fcitx::KeyState>(states) ==
                 fcitx::KeyState::NoState);

    fcitx::keyQtToSym(Qt::Key_Space, {}, "", sym, states);

    FCITX_ASSERT(sym == FcitxKey_space) << sym;
    FCITX_ASSERT(static_cast<fcitx::KeyState>(states) ==
                 fcitx::KeyState::NoState);

    fcitx::keyQtToSym(Qt::Key_Space, Qt::ControlModifier, "", sym, states);

    FCITX_ASSERT(sym == FcitxKey_space) << sym;
    FCITX_ASSERT(static_cast<fcitx::KeyState>(states) == fcitx::KeyState::Ctrl);

    fcitx::keyQtToSym(Qt::Key_F, Qt::ControlModifier, "", sym, states);

    FCITX_ASSERT(sym == FcitxKey_F) << sym;
    FCITX_ASSERT(static_cast<fcitx::KeyState>(states) == fcitx::KeyState::Ctrl);

    fcitx::keyQtToSym(Qt::Key_F, Qt::ControlModifier, "\x06", sym, states);

    FCITX_ASSERT(sym == FcitxKey_F) << sym;
    FCITX_ASSERT(static_cast<fcitx::KeyState>(states) == fcitx::KeyState::Ctrl);
    return 0;
}
