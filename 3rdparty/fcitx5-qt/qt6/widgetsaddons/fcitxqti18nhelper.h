/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#ifndef _WIDGETSADDONS_FCITXQTI18NHELPER_H_
#define _WIDGETSADDONS_FCITXQTI18NHELPER_H_

#include <QString>
#include <fcitx-utils/i18n.h>

namespace fcitx {

inline QString tr2fcitx(const char *message, const char *comment = nullptr) {
    if (comment && comment[0] && message && message[0]) {
        return QString(C_(comment, message));
    } else if (message && message[0]) {
        return QString(_(message));
    } else {
        return QString();
    }
}
} // namespace fcitx

#endif // _WIDGETSADDONS_FCITXQTI18NHELPER_H_
