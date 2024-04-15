/*
 * SPDX-FileCopyrightText: 2012~2023 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "fcitx4inputcontextproxy_p.h"
#include <QCoreApplication>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QFileInfo>
#include <QTimer>

namespace fcitx {

Fcitx4InputContextProxy::Fcitx4InputContextProxy(Fcitx4Watcher *watcher,
                                                 QObject *parent)
    : QObject(parent),
      d_ptr(new Fcitx4InputContextProxyPrivate(watcher, this)) {}

Fcitx4InputContextProxy::~Fcitx4InputContextProxy() { delete d_ptr; }

void Fcitx4InputContextProxy::setDisplay(const QString &display) {
    Q_D(Fcitx4InputContextProxy);
    d->display_ = display;
}

const QString &Fcitx4InputContextProxy::display() const {
    Q_D(const Fcitx4InputContextProxy);
    return d->display_;
}

bool Fcitx4InputContextProxy::isValid() const {
    Q_D(const Fcitx4InputContextProxy);
    return d->isValid();
}

QDBusPendingReply<> Fcitx4InputContextProxy::focusIn() {
    Q_D(Fcitx4InputContextProxy);
    return d->icproxy_->FocusIn();
}

QDBusPendingReply<> Fcitx4InputContextProxy::focusOut() {
    Q_D(Fcitx4InputContextProxy);
    return d->icproxy_->FocusOut();
}

QDBusPendingReply<int> Fcitx4InputContextProxy::processKeyEvent(
    unsigned int keyval, unsigned int keycode, unsigned int state, int type,
    unsigned int time) {
    Q_D(Fcitx4InputContextProxy);
    return d->icproxy_->ProcessKeyEvent(keyval, keycode, state, type, time);
}

QDBusPendingReply<> Fcitx4InputContextProxy::reset() {
    Q_D(Fcitx4InputContextProxy);
    return d->icproxy_->Reset();
}

QDBusPendingReply<> Fcitx4InputContextProxy::setCapability(unsigned int caps) {
    Q_D(Fcitx4InputContextProxy);
    return d->icproxy_->SetCapacity(caps);
}

QDBusPendingReply<> Fcitx4InputContextProxy::setCursorRect(int x, int y, int w,
                                                           int h) {
    Q_D(Fcitx4InputContextProxy);
    return d->icproxy_->SetCursorRect(x, y, w, h);
}

QDBusPendingReply<> Fcitx4InputContextProxy::setSurroundingText(
    const QString &text, unsigned int cursor, unsigned int anchor) {
    Q_D(Fcitx4InputContextProxy);
    return d->icproxy_->SetSurroundingText(text, cursor, anchor);
}

QDBusPendingReply<>
Fcitx4InputContextProxy::setSurroundingTextPosition(unsigned int cursor,
                                                    unsigned int anchor) {
    Q_D(Fcitx4InputContextProxy);
    return d->icproxy_->SetSurroundingTextPosition(cursor, anchor);
}

} // namespace fcitx
