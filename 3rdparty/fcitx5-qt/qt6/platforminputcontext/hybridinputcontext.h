/*
 * SPDX-FileCopyrightText: 2023~2023 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef HYBRIDINPUTCONTEXT_H
#define HYBRIDINPUTCONTEXT_H

#include "fcitx4inputcontextproxy.h"
#include "fcitx4watcher.h"
#include "fcitxqtdbustypes.h"
#include "fcitxqtinputcontextproxy.h"
#include "fcitxqtwatcher.h"
#include <QDBusPendingReply>
#include <QObject>
#include <QString>
#include <QTimer>

namespace fcitx {

class HybridInputContext : public QObject {
    Q_OBJECT
public:
    HybridInputContext(FcitxQtWatcher *watcher, Fcitx4Watcher *fcitx4Watcher,
                       QObject *parent);

    void focusIn();
    void focusOut();
    void setSurroundingText(const QString &text, unsigned int cursor,
                            unsigned int anchor);
    void setSurroundingTextPosition(unsigned int cursor, unsigned int anchor);

    void prevPage();
    void nextPage();
    void reset();
    void selectCandidate(int index);

    bool supportInvokeAction() const;
    void invokeAction(unsigned int action, int cursor);

    bool isValid() const;

    void showVirtualKeyboard();
    void hideVirtualKeyboard();
    bool isVirtualKeyboardVisible() const;

    void setDisplay(const QString &display);

    void setCapability(quint64 capability);
    void setCursorRect(int x, int y, int w, int h);
    void setCursorRectV2(int x, int y, int w, int h, double scale);
    void setSupportedCapability(quint64 supported);
    QDBusPendingCall processKeyEvent(unsigned int keyval, unsigned int keycode,
                                     unsigned int state, bool type,
                                     unsigned int time);
    static bool processKeyEventResult(const QDBusPendingCall &call);

Q_SIGNALS:
    void commitString(const QString &str);
    void currentIM(const QString &name, const QString &uniqueName,
                   const QString &langCode);
    void deleteSurroundingText(int offset, unsigned int nchar);
    void forwardKey(unsigned int keyval, unsigned int state, bool isRelease);
    void updateFormattedPreedit(const FcitxQtFormattedPreeditList &str,
                                int cursorpos);
    void updateClientSideUI(const FcitxQtFormattedPreeditList &preedit,
                            int cursorpos,
                            const FcitxQtFormattedPreeditList &auxUp,
                            const FcitxQtFormattedPreeditList &auxDown,
                            const FcitxQtStringKeyValueList &candidates,
                            int candidateIndex, int layoutHint, bool hasPrev,
                            bool hasNext);
    void inputContextCreated(const QByteArray &uuid);
    void notifyFocusOut();
    void virtualKeyboardVisibilityChanged(bool visible);

private Q_SLOTS:
    void recheck();
    void doRecheck(bool skipFcitx5);
    void inputContextCreationFailed();

private:
    QTimer timer_;
    FcitxQtWatcher *watcher_;
    Fcitx4Watcher *fcitx4Watcher_;
    FcitxQtInputContextProxy *proxy_ = nullptr;
    Fcitx4InputContextProxy *proxy4_ = nullptr;
    QString display_;
};

} // namespace fcitx

#endif
