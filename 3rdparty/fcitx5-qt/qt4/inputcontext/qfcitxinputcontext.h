/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef QFCITXINPUTCONTEXT_H
#define QFCITXINPUTCONTEXT_H

#include "fcitxqtinputcontextproxy.h"
#include "fcitxqtwatcher.h"
#include <QApplication>
#include <QDBusConnection>
#include <QDBusServiceWatcher>
#include <QInputContext>
#include <QKeyEvent>
#include <QPointer>
#include <QRect>
#include <QWidget>
#include <memory>
#include <unordered_map>
#include <xkbcommon/xkbcommon-compose.h>

namespace fcitx {

class FcitxQtConnection;

struct FcitxQtICData {
    FcitxQtICData(FcitxQtWatcher *watcher)
        : proxy(new FcitxQtInputContextProxy(watcher, watcher)) {}
    FcitxQtICData(const FcitxQtICData &that) = delete;
    ~FcitxQtICData() { delete proxy; }
    quint64 capability = 0;
    FcitxQtInputContextProxy *proxy;
    QRect rect;
    // Last key event forwarded.
    std::unique_ptr<QKeyEvent> event;
    QString surroundingText;
    int surroundingAnchor = -1;
    int surroundingCursor = -1;
};

class ProcessKeyWatcher : public QDBusPendingCallWatcher {
    Q_OBJECT
public:
    ProcessKeyWatcher(const QKeyEvent &event, QWidget *window,
                      const QDBusPendingCall &call, QObject *parent = 0)
        : QDBusPendingCallWatcher(call, parent),
          event_(QKeyEvent::createExtendedKeyEvent(
              event.type(), event.key(), event.modifiers(),
              event.nativeScanCode(), event.nativeVirtualKey(),
              event.nativeModifiers(), event.text(), event.isAutoRepeat(),
              event.count())),
          window_(window) {}

    virtual ~ProcessKeyWatcher() { delete event_; }

    QKeyEvent &keyEvent() { return *event_; }

    QWidget *window() { return window_.data(); }

private:
    QKeyEvent *event_;
    QPointer<QWidget> window_;
};

struct XkbContextDeleter {
    static inline void cleanup(struct xkb_context *pointer) {
        if (pointer)
            xkb_context_unref(pointer);
    }
};

struct XkbComposeTableDeleter {
    static inline void cleanup(struct xkb_compose_table *pointer) {
        if (pointer)
            xkb_compose_table_unref(pointer);
    }
};

struct XkbComposeStateDeleter {
    static inline void cleanup(struct xkb_compose_state *pointer) {
        if (pointer)
            xkb_compose_state_unref(pointer);
    }
};

class FcitxQtInputMethodProxy;

class QFcitxInputContext : public QInputContext {
    Q_OBJECT
public:
    QFcitxInputContext();
    virtual ~QFcitxInputContext();

    QString identifierName() override;
    QString language() override;
    void reset() override;
    bool isComposing() const override { return false; };
    void update() override;
    void setFocusWidget(QWidget *w) override;

    void widgetDestroyed(QWidget *w) override;

    bool filterEvent(const QEvent *event) override;
    void mouseHandler(int x, QMouseEvent *event) override;

public Q_SLOTS:
    void cursorRectChanged();
    void commitString(const QString &str);
    void updateFormattedPreedit(const FcitxQtFormattedPreeditList &preeditList,
                                int cursorPos);
    void deleteSurroundingText(int offset, unsigned int nchar);
    void forwardKey(unsigned int keyval, unsigned int state, bool type);
    void createInputContextFinished(const QByteArray &uuid);
    void serverSideFocusOut();
    void cleanUp();
    void windowDestroyed(QObject *object);

private:
    bool processCompose(unsigned int keyval, unsigned int state,
                        bool isRelaese);
    QKeyEvent *createKeyEvent(unsigned int keyval, unsigned int state,
                              bool isRelaese, const QKeyEvent *event);

    void addCapability(FcitxQtICData &data, quint64 capability,
                       bool forceUpdate = false) {
        auto newcaps = data.capability | capability;
        if (data.capability != newcaps || forceUpdate) {
            data.capability = newcaps;
            updateCapability(data);
        }
    }

    void removeCapability(FcitxQtICData &data, quint64 capability,
                          bool forceUpdate = false) {
        auto newcaps = data.capability & (~capability);
        if (data.capability != newcaps || forceUpdate) {
            data.capability = newcaps;
            updateCapability(data);
        }
    }

    void updateCapability(const FcitxQtICData &data);
    void commitPreedit(QPointer<QWidget> input = qApp->focusWidget());
    void createICData(QWidget *w);
    FcitxQtInputContextProxy *validIC();
    FcitxQtInputContextProxy *validICByWindow(QWidget *window);
    bool filterEventFallback(unsigned int keyval, unsigned int keycode,
                             unsigned int state, bool isRelaese);

    FcitxQtWatcher *watcher_;
    QString preedit_;
    QString commitPreedit_;
    FcitxQtFormattedPreeditList preeditList_;
    int cursorPos_;
    bool useSurroundingText_;
    bool syncMode_;
    std::unordered_map<QWidget *, FcitxQtICData> icMap_;
    QPointer<QWidget> lastWindow_;
    bool destroy_;
    QScopedPointer<struct xkb_context, XkbContextDeleter> xkbContext_;
    QScopedPointer<struct xkb_compose_table, XkbComposeTableDeleter>
        xkbComposeTable_;
    QScopedPointer<struct xkb_compose_state, XkbComposeStateDeleter>
        xkbComposeState_;
private Q_SLOTS:
    void processKeyEventFinished(QDBusPendingCallWatcher *);
};
} // namespace fcitx

#endif // QFCITXINPUTCONTEXT_H
