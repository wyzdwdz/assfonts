/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef QFCITXPLATFORMINPUTCONTEXT_H
#define QFCITXPLATFORMINPUTCONTEXT_H

#include "fcitxcandidatewindow.h"
#include "fcitxqtwatcher.h"
#include "hybridinputcontext.h"
#include <QDBusConnection>
#include <QDBusPendingCallWatcher>
#include <QDBusServiceWatcher>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QPointer>
#include <QRect>
#include <QWindow>
#include <memory>
#include <qpa/qplatforminputcontext.h>
#include <unordered_map>
#include <xkbcommon/xkbcommon-compose.h>

namespace fcitx {

class FcitxQtConnection;
class QFcitxPlatformInputContext;

class FcitxQtICData : public QObject {
public:
    FcitxQtICData(QFcitxPlatformInputContext *context, QWindow *window);
    FcitxQtICData(const FcitxQtICData &that) = delete;
    ~FcitxQtICData() override;

    FcitxCandidateWindow *candidateWindow();

    QWindow *window() { return window_.data(); }

    void resetCandidateWindow();

    quint64 capability = 0;
    HybridInputContext *proxy;
    QRect rect;
    // Last key event forwarded.
    std::unique_ptr<QKeyEvent> event;
    QString surroundingText;
    int surroundingAnchor = -1;
    int surroundingCursor = -1;
    bool expectingMicroFocusChange = false;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QFcitxPlatformInputContext *context_;
    QPointer<QWindow> window_;
    QPointer<FcitxCandidateWindow> candidateWindow_;
};

class ProcessKeyWatcher : public QDBusPendingCallWatcher {
    Q_OBJECT
public:
    ProcessKeyWatcher(const QKeyEvent &event, QWindow *window,
                      const QDBusPendingCall &call, QObject *parent = nullptr)
        : QDBusPendingCallWatcher(call, parent),
          event_(event.type(), event.key(), event.modifiers(),
                 event.nativeScanCode(), event.nativeVirtualKey(),
                 event.nativeModifiers(), event.text(), event.isAutoRepeat(),
                 event.count()),
          window_(window) {}

    const QKeyEvent &keyEvent() { return event_; }

    QWindow *window() { return window_.data(); }

private:
    QKeyEvent event_;
    QPointer<QWindow> window_;
};

struct XkbContextDeleter {
    static inline void cleanup(struct xkb_context *pointer) {
        if (pointer) {
            xkb_context_unref(pointer);
        }
    }
};

struct XkbComposeTableDeleter {
    static inline void cleanup(struct xkb_compose_table *pointer) {
        if (pointer) {
            xkb_compose_table_unref(pointer);
        }
    }
};

struct XkbComposeStateDeleter {
    static inline void cleanup(struct xkb_compose_state *pointer) {
        if (pointer) {
            xkb_compose_state_unref(pointer);
        }
    }
};

class QFcitxPlatformInputContext : public QPlatformInputContext {
    Q_OBJECT
public:
    QFcitxPlatformInputContext();
    ~QFcitxPlatformInputContext() override;

    bool isValid() const override;
    void setFocusObject(QObject *object) override;
    void invokeAction(QInputMethod::Action imAction,
                      int cursorPosition) override;
    void reset() override;
    void commit() override;
    void update(Qt::InputMethodQueries queries) override;
    bool filterEvent(const QEvent *event) override;
    QLocale locale() const override;
    bool hasCapability(Capability capability) const override;
    void showInputPanel() override;
    void hideInputPanel() override;
    bool isInputPanelVisible() const override;

    auto *watcher() { return watcher_; }
    auto *fcitx4Watcher() { return fcitx4Watcher_; }

    // Use Wrapper as suffix to avoid upstream add function with same name.
    QObject *focusObjectWrapper() const;
    QWindow *focusWindowWrapper() const;
    QRect cursorRectangleWrapper() const;

    // Initialize theme object on demand.
    FcitxTheme *theme();
    bool hasPreedit() const { return !preeditList_.isEmpty(); }

public Q_SLOTS:
    void cursorRectChanged();
    void commitString(const QString &str);
    void updateFormattedPreedit(const FcitxQtFormattedPreeditList &preeditList,
                                int cursorPos);
    void deleteSurroundingText(int offset, unsigned int nchar);
    void forwardKey(unsigned int keyval, unsigned int state, bool type);
    void createInputContextFinished(const QByteArray &uuid);
    void cleanUp();
    void windowDestroyed(QObject *object);
    void updateCurrentIM(const QString &name, const QString &uniqueName,
                         const QString &langCode);
    void updateClientSideUI(const FcitxQtFormattedPreeditList &preedit,
                            int cursorpos,
                            const FcitxQtFormattedPreeditList &auxUp,
                            const FcitxQtFormattedPreeditList &auxDown,
                            const FcitxQtStringKeyValueList &candidates,
                            int candidateIndex, int layoutHint, bool hasPrev,
                            bool hasNext);
    void serverSideFocusOut();
    bool commitPreedit(QPointer<QObject> input = qApp->focusObject());
private Q_SLOTS:
    void processKeyEventFinished(QDBusPendingCallWatcher *);

private:
    bool processCompose(unsigned int keyval, unsigned int state,
                        bool isRelease);
    QKeyEvent *createKeyEvent(unsigned int keyval, unsigned int state,
                              bool isRelease, const QKeyEvent *event);
    void forwardEvent(QWindow *window, const QKeyEvent &event);

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
    void createICData(QWindow *w);
    HybridInputContext *validIC() const;
    HybridInputContext *validICByWindow(QWindow *window) const;
    bool filterEventFallback(unsigned int keyval, unsigned int keycode,
                             unsigned int state, bool isRelease);

    void updateCursorRect();
    bool objectAcceptsInputMethod() const;
    bool shouldDisableInputMethod() const;

    void updateInputPanelVisible();

    FcitxQtWatcher *watcher_;
    Fcitx4Watcher *fcitx4Watcher_;
    QString preedit_;
    QString commitPreedit_;
    FcitxQtFormattedPreeditList preeditList_;
    int cursorPos_;
    bool useSurroundingText_;
    bool syncMode_;
    std::unordered_map<QWindow *, FcitxQtICData> icMap_;
    QPointer<QWindow> lastWindow_;
    QPointer<QObject> lastObject_;
    bool destroy_;
    bool virtualKeyboardVisible_;
    QScopedPointer<struct xkb_context, XkbContextDeleter> xkbContext_;
    QScopedPointer<struct xkb_compose_table, XkbComposeTableDeleter>
        xkbComposeTable_;
    QScopedPointer<struct xkb_compose_state, XkbComposeStateDeleter>
        xkbComposeState_;
    QLocale locale_;
    FcitxTheme *theme_ = nullptr;
    bool inputPanelVisible_ = false;
};
} // namespace fcitx

#endif // QFCITXPLATFORMINPUTCONTEXT_H
