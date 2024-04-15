/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef _WIDGETSADDONS_QTKEYTRANS_H_
#define _WIDGETSADDONS_QTKEYTRANS_H_

#include "fcitx5qt5widgetsaddons_export.h"
#include <QString>
#include <qnamespace.h>

namespace fcitx {

FCITX5QT5WIDGETSADDONS_EXPORT bool keyQtToSym(int qtcode,
                                              Qt::KeyboardModifiers mod,
                                              int &sym, unsigned int &state);

FCITX5QT5WIDGETSADDONS_EXPORT bool keyQtToSym(int qtcode,
                                              Qt::KeyboardModifiers mod,
                                              const QString &text, int &sym,
                                              unsigned int &state);

FCITX5QT5WIDGETSADDONS_EXPORT bool symToKeyQt(int sym, unsigned int state,
                                              int &qtcode,
                                              Qt::KeyboardModifiers &mod);
} // namespace fcitx

#endif // _WIDGETSADDONS_QTKEYTRANS_H_
