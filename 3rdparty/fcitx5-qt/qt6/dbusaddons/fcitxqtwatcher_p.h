/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _DBUSADDONS_FCITXQTWATCHER_P_H_
#define _DBUSADDONS_FCITXQTWATCHER_P_H_

#include "fcitxqtwatcher.h"
#include <QDBusServiceWatcher>

#define FCITX_MAIN_SERVICE_NAME "org.fcitx.Fcitx5"
#define FCITX_PORTAL_SERVICE_NAME "org.freedesktop.portal.Fcitx"

namespace fcitx {

class FcitxQtWatcherPrivate {
public:
    FcitxQtWatcherPrivate(FcitxQtWatcher *q) : serviceWatcher_(q) {}

    QDBusServiceWatcher serviceWatcher_;
    bool watchPortal_ = false;
    bool availability_ = false;
    bool mainPresent_ = false;
    bool portalPresent_ = false;
    bool watched_ = false;
};
} // namespace fcitx

#endif // _DBUSADDONS_FCITXQTWATCHER_P_H_
