/*
 * SPDX-FileCopyrightText: 2023~2023 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "hybridinputcontext.h"
#include "fcitx4inputcontextproxy.h"
#include "fcitxqtinputcontextproxy.h"
#include "fcitxqtwatcher.h"
#include <cstddef>

namespace fcitx {

HybridInputContext::HybridInputContext(FcitxQtWatcher *watcher,
                                       Fcitx4Watcher *fcitx4Watcher,
                                       QObject *parent)
    : QObject(parent), watcher_(watcher), fcitx4Watcher_(fcitx4Watcher) {
    // We use a timer here to avoid recheck being triggered multiple times.
    // For fcitx5, the fcitx4frontend and dbusfrontend will be initialized
    // around the same time. 50ms is specifically selected to be smaller than
    // the 100ms timer within input context.
    timer_.setInterval(50);
    timer_.setSingleShot(true);
    connect(&timer_, &QTimer::timeout, this,
            [this]() { doRecheck(/*skipFcitx5=*/false); });
    connect(watcher_, &FcitxQtWatcher::availabilityChanged, this,
            &HybridInputContext::recheck);
    connect(fcitx4Watcher_, &Fcitx4Watcher::availabilityChanged, this,
            &HybridInputContext::recheck);

    recheck();
}

void HybridInputContext::recheck() { timer_.start(); }

void HybridInputContext::doRecheck(bool skipFcitx5) {
    const bool fcitx5Available = !skipFcitx5 && watcher_->availability();
    if (!fcitx5Available) {
        delete proxy_;
        proxy_ = nullptr;
    }

    if (!fcitx4Watcher_->availability()) {
        delete proxy4_;
        proxy4_ = nullptr;
    }

    if (fcitx5Available) {
        delete proxy4_;
        proxy4_ = nullptr;
        if (!proxy_) {
            proxy_ = new FcitxQtInputContextProxy(watcher_, this);
            proxy_->setDisplay(display_);
            connect(proxy_, &FcitxQtInputContextProxy::commitString, this,
                    &HybridInputContext::commitString);
            connect(proxy_, &FcitxQtInputContextProxy::currentIM, this,
                    &HybridInputContext::currentIM);
            connect(proxy_, &FcitxQtInputContextProxy::deleteSurroundingText,
                    this, &HybridInputContext::deleteSurroundingText);
            connect(proxy_, &FcitxQtInputContextProxy::forwardKey, this,
                    &HybridInputContext::forwardKey);
            connect(proxy_, &FcitxQtInputContextProxy::updateFormattedPreedit,
                    this, &HybridInputContext::updateFormattedPreedit);
            connect(proxy_, &FcitxQtInputContextProxy::updateClientSideUI, this,
                    &HybridInputContext::updateClientSideUI);
            connect(proxy_, &FcitxQtInputContextProxy::inputContextCreated,
                    this, &HybridInputContext::inputContextCreated);
            connect(proxy_,
                    &FcitxQtInputContextProxy::inputContextCreationFailed, this,
                    &HybridInputContext::inputContextCreationFailed);
            connect(proxy_, &FcitxQtInputContextProxy::notifyFocusOut, this,
                    &HybridInputContext::notifyFocusOut);
            connect(proxy_,
                    &FcitxQtInputContextProxy::virtualKeyboardVisibilityChanged,
                    this,
                    &HybridInputContext::virtualKeyboardVisibilityChanged);
        }
    } else if (fcitx4Watcher_->availability()) {
        if (!proxy4_) {
            proxy4_ = new Fcitx4InputContextProxy(fcitx4Watcher_, this);
            connect(proxy4_, &Fcitx4InputContextProxy::commitString, this,
                    &HybridInputContext::commitString);
            connect(proxy4_, &Fcitx4InputContextProxy::currentIM, this,
                    &HybridInputContext::currentIM);
            connect(proxy4_, &Fcitx4InputContextProxy::deleteSurroundingText,
                    this, &HybridInputContext::deleteSurroundingText);
            connect(proxy4_, &Fcitx4InputContextProxy::forwardKey, this,
                    &HybridInputContext::forwardKey);
            connect(
                proxy4_, &Fcitx4InputContextProxy::updateFormattedPreedit, this,
                [this](const FcitxQtFormattedPreeditList &list, int cursorpos) {
                    auto newList = list;
                    for (auto item : newList) {
                        const qint32 underlineBit = (1 << 3);
                        // revert non underline and "underline"
                        item.setFormat(item.format() ^ underlineBit);
                    }

                    Q_EMIT updateFormattedPreedit(newList, cursorpos);
                });
            connect(proxy4_, &Fcitx4InputContextProxy::inputContextCreated,
                    this,
                    [this]() { Q_EMIT inputContextCreated(QByteArray()); });
        }
    }
}

void HybridInputContext::inputContextCreationFailed() {
    doRecheck(/*skipFcitx5=*/true);
}

bool HybridInputContext::isValid() const {
    if (proxy_) {
        return proxy_->isValid();
    } else if (proxy4_) {
        return proxy4_->isValid();
    }
    return false;
}

void HybridInputContext::reset() {
    if (proxy_) {
        proxy_->reset();
    } else if (proxy4_) {
        proxy4_->reset();
    }
}

void HybridInputContext::selectCandidate(int i) {
    if (proxy_) {
        proxy_->selectCandidate(i);
    }
}

void HybridInputContext::prevPage() {
    if (proxy_) {
        proxy_->prevPage();
    }
}

void HybridInputContext::nextPage() {
    if (proxy_) {
        proxy_->nextPage();
    }
}

bool HybridInputContext::supportInvokeAction() const {
    if (proxy_) {
        return proxy_->supportInvokeAction();
    }
    return false;
}

void HybridInputContext::invokeAction(unsigned int action, int cursor) {
    if (proxy_) {
        proxy_->invokeAction(action, cursor);
    }
}

void HybridInputContext::setSurroundingText(const QString &text,
                                            unsigned int cursor,
                                            unsigned int anchor) {
    if (proxy_) {
        proxy_->setSurroundingText(text, cursor, anchor);
    } else if (proxy4_) {
        proxy4_->setSurroundingText(text, cursor, anchor);
    }
}

void HybridInputContext::setSurroundingTextPosition(unsigned int cursor,
                                                    unsigned int anchor) {
    if (proxy_) {
        proxy_->setSurroundingTextPosition(cursor, anchor);
    } else if (proxy4_) {
        proxy4_->setSurroundingTextPosition(cursor, anchor);
    }
}

void HybridInputContext::focusIn() {
    if (proxy_) {
        proxy_->focusIn();
    } else if (proxy4_) {
        proxy4_->focusIn();
    }
}

void HybridInputContext::focusOut() {
    if (proxy_) {
        proxy_->focusOut();
    } else if (proxy4_) {
        proxy4_->focusOut();
    }
}

void HybridInputContext::setCursorRectV2(int x, int y, int w, int h,
                                         double scale) {
    if (proxy_) {
        proxy_->setCursorRectV2(x, y, w, h, scale);
    } else if (proxy4_) {
        proxy4_->setCursorRect(x, y, w, h);
    }
}

void HybridInputContext::setCursorRect(int x, int y, int w, int h) {
    if (proxy_) {
        proxy_->setCursorRect(x, y, w, h);
    } else if (proxy4_) {
        proxy4_->setCursorRect(x, y, w, h);
    }
}

void HybridInputContext::setSupportedCapability(quint64 supported) {
    if (proxy_) {
        proxy_->setSupportedCapability(supported);
    }
}

void HybridInputContext::setCapability(quint64 capability) {
    if (proxy_) {
        proxy_->setCapability(capability);
    } else if (proxy4_) {
        proxy4_->setCapability(capability & 0xfffffffffULL);
    }
}

void HybridInputContext::showVirtualKeyboard() {
    if (proxy_) {
        proxy_->showVirtualKeyboard();
    }
}

void HybridInputContext::hideVirtualKeyboard() {
    if (proxy_) {
        proxy_->hideVirtualKeyboard();
    }
}

bool HybridInputContext::isVirtualKeyboardVisible() const {
    if (proxy_) {
        return proxy_->isVirtualKeyboardVisible();
    }
    return false;
}

void HybridInputContext::setDisplay(const QString &display) {
    if (proxy_) {
        proxy_->setDisplay(display);
    }
    display_ = display;
}

QDBusPendingCall HybridInputContext::processKeyEvent(unsigned int keyval,
                                                     unsigned int keycode,
                                                     unsigned int state,
                                                     bool type,
                                                     unsigned int time) {
    if (proxy_) {
        return proxy_->processKeyEvent(keyval, keycode, state, type, time);
    }
    return proxy4_->processKeyEvent(keyval, keycode, state, type ? 1 : 0, time);
}

bool HybridInputContext::processKeyEventResult(const QDBusPendingCall &call) {
    if (call.isError()) {
        return false;
    }

    if (call.reply().signature() == "b") {
        QDBusPendingReply<bool> reply = call;
        return reply.value();
    } else if (call.reply().signature() == "i") {
        QDBusPendingReply<int> reply = call;
        return reply.value() > 0;
    }
    return false;
}

} // namespace fcitx
