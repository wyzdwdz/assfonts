/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "fcitxqtinputcontextproxy_p.h"

namespace fcitx {

FcitxQtInputContextProxy::FcitxQtInputContextProxy(FcitxQtWatcher *watcher,
                                                   QObject *parent)
    : QObject(parent),
      d_ptr(new FcitxQtInputContextProxyPrivate(watcher, this)) {}

FcitxQtInputContextProxy::~FcitxQtInputContextProxy() { delete d_ptr; }

void FcitxQtInputContextProxy::setDisplay(const QString &display) {
    Q_D(FcitxQtInputContextProxy);
    d->display_ = display;
}

const QString &FcitxQtInputContextProxy::display() const {
    Q_D(const FcitxQtInputContextProxy);
    return d->display_;
}

bool FcitxQtInputContextProxy::isValid() const {
    Q_D(const FcitxQtInputContextProxy);
    return d->isValid();
}

QDBusPendingReply<> FcitxQtInputContextProxy::focusIn() {
    Q_D(FcitxQtInputContextProxy);
    return d->icproxy_->FocusIn();
}

QDBusPendingReply<> FcitxQtInputContextProxy::focusOut() {
    Q_D(FcitxQtInputContextProxy);
    return d->icproxy_->FocusOut();
}

QDBusPendingReply<> FcitxQtInputContextProxy::hideVirtualKeyboard() {
    Q_D(FcitxQtInputContextProxy);
    return d->icproxy_->HideVirtualKeyboard();
}

QDBusPendingReply<bool> FcitxQtInputContextProxy::processKeyEvent(
    unsigned int keyval, unsigned int keycode, unsigned int state, bool type,
    unsigned int time) {
    Q_D(FcitxQtInputContextProxy);
    return d->icproxy_->ProcessKeyEvent(keyval, keycode, state, type, time);
}

QDBusPendingReply<> FcitxQtInputContextProxy::reset() {
    Q_D(FcitxQtInputContextProxy);
    return d->icproxy_->Reset();
}

QDBusPendingReply<>
FcitxQtInputContextProxy::setSupportedCapability(qulonglong caps) {
    Q_D(FcitxQtInputContextProxy);
    return d->icproxy_->SetSupportedCapability(caps);
}

QDBusPendingReply<> FcitxQtInputContextProxy::setCapability(qulonglong caps) {
    Q_D(FcitxQtInputContextProxy);
    return d->icproxy_->SetCapability(caps);
}

QDBusPendingReply<> FcitxQtInputContextProxy::setCursorRect(int x, int y, int w,
                                                            int h) {
    Q_D(FcitxQtInputContextProxy);
    return d->icproxy_->SetCursorRect(x, y, w, h);
}

QDBusPendingReply<> FcitxQtInputContextProxy::setCursorRectV2(int x, int y,
                                                              int w, int h,
                                                              double scale) {
    Q_D(FcitxQtInputContextProxy);
    return d->icproxy_->SetCursorRectV2(x, y, w, h, scale);
}

QDBusPendingReply<> FcitxQtInputContextProxy::setSurroundingText(
    const QString &text, unsigned int cursor, unsigned int anchor) {
    Q_D(FcitxQtInputContextProxy);
    return d->icproxy_->SetSurroundingText(text, cursor, anchor);
}

QDBusPendingReply<>
FcitxQtInputContextProxy::setSurroundingTextPosition(unsigned int cursor,
                                                     unsigned int anchor) {
    Q_D(FcitxQtInputContextProxy);
    return d->icproxy_->SetSurroundingTextPosition(cursor, anchor);
}

QDBusPendingReply<> FcitxQtInputContextProxy::showVirtualKeyboard() {
    Q_D(FcitxQtInputContextProxy);
    return d->icproxy_->ShowVirtualKeyboard();
}

QDBusPendingReply<> FcitxQtInputContextProxy::prevPage() {
    Q_D(FcitxQtInputContextProxy);
    return d->icproxy_->PrevPage();
}

QDBusPendingReply<> FcitxQtInputContextProxy::nextPage() {
    Q_D(FcitxQtInputContextProxy);
    return d->icproxy_->NextPage();
}

QDBusPendingReply<> FcitxQtInputContextProxy::selectCandidate(int i) {
    Q_D(FcitxQtInputContextProxy);
    return d->icproxy_->SelectCandidate(i);
}

QDBusPendingReply<> FcitxQtInputContextProxy::invokeAction(unsigned int action,
                                                           int cursor) {
    Q_D(FcitxQtInputContextProxy);
    return d->icproxy_->InvokeAction(action, cursor);
}

bool FcitxQtInputContextProxy::isVirtualKeyboardVisible() {
    Q_D(FcitxQtInputContextProxy);
    return d->isVirtualKeyboardVisible_;
}

bool FcitxQtInputContextProxy::supportInvokeAction() const {
    Q_D(const FcitxQtInputContextProxy);
    return d->supportInvokeAction_;
}

} // namespace fcitx
