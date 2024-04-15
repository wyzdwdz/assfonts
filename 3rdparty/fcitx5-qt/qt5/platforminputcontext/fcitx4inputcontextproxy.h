/*
 * SPDX-FileCopyrightText: 2012~2023 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _FCITX4INPUTCONTEXTPROXY_H_
#define _FCITX4INPUTCONTEXTPROXY_H_

#include "fcitxqtdbustypes.h"
#include <QDBusConnection>
#include <QDBusPendingReply>
#include <QDBusServiceWatcher>
#include <QObject>

class QDBusPendingCallWatcher;

namespace fcitx {

class Fcitx4Watcher;
class Fcitx4InputContextProxyPrivate;

class Fcitx4InputContextProxy : public QObject {
    Q_OBJECT
public:
    Fcitx4InputContextProxy(Fcitx4Watcher *watcher, QObject *parent);
    ~Fcitx4InputContextProxy();

    bool isValid() const;
    void setDisplay(const QString &display);
    const QString &display() const;

public Q_SLOTS:
    QDBusPendingReply<> focusIn();
    QDBusPendingReply<> focusOut();
    QDBusPendingReply<int> processKeyEvent(unsigned int keyval,
                                           unsigned int keycode,
                                           unsigned int state, int type,
                                           unsigned int time);
    QDBusPendingReply<> reset();
    QDBusPendingReply<> setCapability(unsigned int caps);
    QDBusPendingReply<> setCursorRect(int x, int y, int w, int h);
    QDBusPendingReply<> setSurroundingText(const QString &text,
                                           unsigned int cursor,
                                           unsigned int anchor);
    QDBusPendingReply<> setSurroundingTextPosition(unsigned int cursor,
                                                   unsigned int anchor);

Q_SIGNALS:
    void commitString(const QString &str);
    void currentIM(const QString &name, const QString &uniqueName,
                   const QString &langCode);
    void deleteSurroundingText(int offset, unsigned int nchar);
    void forwardKey(unsigned int keyval, unsigned int state, bool isRelease);
    void updateFormattedPreedit(const FcitxQtFormattedPreeditList &str,
                                int cursorpos);
    void inputContextCreated();

private:
    Fcitx4InputContextProxyPrivate *const d_ptr;
    Q_DECLARE_PRIVATE(Fcitx4InputContextProxy);
};

} // namespace fcitx

#endif // _FCITX4INPUTCONTEXTPROXY_H_
