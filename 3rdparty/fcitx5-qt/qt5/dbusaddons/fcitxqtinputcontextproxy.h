/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#ifndef _DBUSADDONS_FCITXQTINPUTCONTEXTPROXY_H_
#define _DBUSADDONS_FCITXQTINPUTCONTEXTPROXY_H_

#include "fcitx5qt5dbusaddons_export.h"

#include "fcitxqtdbustypes.h"
#include <QDBusPendingReply>


namespace fcitx {

class FcitxQtWatcher;
class FcitxQtInputContextProxyPrivate;

class FCITX5QT5DBUSADDONS_EXPORT FcitxQtInputContextProxy : public QObject {
    Q_OBJECT
public:
    FcitxQtInputContextProxy(FcitxQtWatcher *watcher, QObject *parent);
    ~FcitxQtInputContextProxy();

    bool isValid() const;
    void setDisplay(const QString &display);
    const QString &display() const;

public Q_SLOTS:
    QDBusPendingReply<> focusIn();
    QDBusPendingReply<> focusOut();
    QDBusPendingReply<> hideVirtualKeyboard();
    QDBusPendingReply<bool> processKeyEvent(unsigned int keyval,
                                            unsigned int keycode,
                                            unsigned int state, bool type,
                                            unsigned int time);
    QDBusPendingReply<> reset();
    QDBusPendingReply<> setSupportedCapability(qulonglong caps);
    QDBusPendingReply<> setCapability(qulonglong caps);
    QDBusPendingReply<> setCursorRect(int x, int y, int w, int h);
    QDBusPendingReply<> setCursorRectV2(int x, int y, int w, int h,
                                        double scale);
    QDBusPendingReply<> setSurroundingText(const QString &text,
                                           unsigned int cursor,
                                           unsigned int anchor);
    QDBusPendingReply<> setSurroundingTextPosition(unsigned int cursor,
                                                   unsigned int anchor);
    QDBusPendingReply<> showVirtualKeyboard();
    QDBusPendingReply<> prevPage();
    QDBusPendingReply<> nextPage();
    QDBusPendingReply<> selectCandidate(int i);
    QDBusPendingReply<> invokeAction(unsigned int action, int cursor);
    bool isVirtualKeyboardVisible();

    bool supportInvokeAction() const;

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
    void inputContextCreationFailed();
    void notifyFocusOut();
    void virtualKeyboardVisibilityChanged(bool visible);

private:
    FcitxQtInputContextProxyPrivate *const d_ptr;
    Q_DECLARE_PRIVATE(FcitxQtInputContextProxy);
};

} // namespace fcitx

#endif // _DBUSADDONS_FCITXQTINPUTCONTEXTPROXY_H_
