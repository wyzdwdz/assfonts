/*
 * Copyright (C) 2011~2017 by CSSlayer
 * wengxt@gmail.com
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above Copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above Copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the authors nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 */

#include <QDBusConnection>
#include <QDebug>
#include <QGuiApplication>
#include <QInputMethod>
#include <QKeyEvent>
#include <QPalette>
#include <QTextCharFormat>
#include <QWindow>
#include <qpa/qplatformcursor.h>
#include <qpa/qplatformscreen.h>
#include <qpa/qwindowsysteminterface.h>

#include "qtkey.h"

#include "fcitxinputcontextproxy.h"
#include "fcitxwatcher.h"
#include "qfcitxplatforminputcontext.h"

static bool get_boolean_env(const char *name, bool defval) {
    const char *value = getenv(name);

    if (value == nullptr)
        return defval;

    if (strcmp(value, "") == 0 || strcmp(value, "0") == 0 ||
        strcmp(value, "false") == 0 || strcmp(value, "False") == 0 ||
        strcmp(value, "FALSE") == 0)
        return false;

    return true;
}

static inline const char *get_locale() {
    const char *locale = getenv("LC_ALL");
    if (!locale)
        locale = getenv("LC_CTYPE");
    if (!locale)
        locale = getenv("LANG");
    if (!locale)
        locale = "C";

    return locale;
}

static bool objectAcceptsInputMethod() {
    bool enabled = false;
    QObject *object = qApp->focusObject();
    if (object) {
        QInputMethodQueryEvent query(Qt::ImEnabled);
        QGuiApplication::sendEvent(object, &query);
        enabled = query.value(Qt::ImEnabled).toBool();
    }

    return enabled;
}

struct xkb_context *_xkb_context_new_helper() {
    struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (context) {
        xkb_context_set_log_level(context, XKB_LOG_LEVEL_CRITICAL);
    }

    return context;
}

QFcitxPlatformInputContext::QFcitxPlatformInputContext()
    : m_watcher(new FcitxWatcher(
          QDBusConnection::connectToBus(QDBusConnection::SessionBus,
                                        "fcitx-platform-input-context"),
          this)),
      m_cursorPos(0), m_useSurroundingText(false),
      m_syncMode(get_boolean_env("FCITX_QT_USE_SYNC", false)), m_destroy(false),
      m_xkbContext(_xkb_context_new_helper()),
      m_xkbComposeTable(m_xkbContext ? xkb_compose_table_new_from_locale(
                                           m_xkbContext.data(), get_locale(),
                                           XKB_COMPOSE_COMPILE_NO_FLAGS)
                                     : 0),
      m_xkbComposeState(m_xkbComposeTable
                            ? xkb_compose_state_new(m_xkbComposeTable.data(),
                                                    XKB_COMPOSE_STATE_NO_FLAGS)
                            : 0) {
    m_watcher->watch();
}

QFcitxPlatformInputContext::~QFcitxPlatformInputContext() {
    m_destroy = true;
    m_watcher->unwatch();
    cleanUp();
    delete m_watcher;
}

void QFcitxPlatformInputContext::cleanUp() {
    m_icMap.clear();

    if (!m_destroy) {
        commitPreedit();
    }
}

bool QFcitxPlatformInputContext::isValid() const { return true; }

void QFcitxPlatformInputContext::invokeAction(QInputMethod::Action action,
                                              int cursorPosition) {
    if (action == QInputMethod::Click &&
        (cursorPosition <= 0 || cursorPosition >= m_preedit.length())) {
        // qDebug() << action << cursorPosition;
        commitPreedit();
    }
}

void QFcitxPlatformInputContext::commitPreedit(QPointer<QObject> input) {
    if (!input)
        return;
    if (m_commitPreedit.length() <= 0)
        return;
    QInputMethodEvent e;
    e.setCommitString(m_commitPreedit);
    QCoreApplication::sendEvent(input, &e);
    m_commitPreedit.clear();
    m_preeditList.clear();
}

bool checkUtf8(const QByteArray &byteArray) {
    QString s = QString::fromUtf8(byteArray);
    return !s.contains(QChar::ReplacementCharacter);
}

void QFcitxPlatformInputContext::reset() {
    commitPreedit();
    if (FcitxInputContextProxy *proxy = validIC()) {
        proxy->reset();
    }
    if (m_xkbComposeState) {
        xkb_compose_state_reset(m_xkbComposeState.data());
    }
    QPlatformInputContext::reset();
}

void QFcitxPlatformInputContext::update(Qt::InputMethodQueries queries) {
    // ignore the boring query
    if (!(queries & (Qt::ImCursorRectangle | Qt::ImHints |
                     Qt::ImSurroundingText | Qt::ImCursorPosition))) {
        return;
    }

    QWindow *window = qApp->focusWindow();
    FcitxInputContextProxy *proxy = validICByWindow(window);
    if (!proxy)
        return;

    FcitxQtICData &data = *static_cast<FcitxQtICData *>(
        proxy->property("icData").value<void *>());

    QObject *input = qApp->focusObject();
    if (!input)
        return;

    QInputMethodQueryEvent query(queries);
    QGuiApplication::sendEvent(input, &query);

    if (queries & Qt::ImCursorRectangle) {
        cursorRectChanged();
    }

    if (queries & Qt::ImHints) {
        Qt::InputMethodHints hints =
            Qt::InputMethodHints(query.value(Qt::ImHints).toUInt());

#define CHECK_HINTS(_HINTS, _CAPACITY)                                         \
    if (hints & _HINTS)                                                        \
        addCapability(data, _CAPACITY);                                        \
    else                                                                       \
        removeCapability(data, _CAPACITY);

        CHECK_HINTS(Qt::ImhHiddenText, CAPACITY_PASSWORD)
        CHECK_HINTS(Qt::ImhNoAutoUppercase, CAPACITY_NOAUTOUPPERCASE)
        CHECK_HINTS(Qt::ImhPreferNumbers, CAPACITY_NUMBER)
        CHECK_HINTS(Qt::ImhPreferUppercase, CAPACITY_UPPERCASE)
        CHECK_HINTS(Qt::ImhPreferLowercase, CAPACITY_LOWERCASE)
        CHECK_HINTS(Qt::ImhNoPredictiveText, CAPACITY_NO_SPELLCHECK)
        CHECK_HINTS(Qt::ImhDigitsOnly, CAPACITY_DIGIT)
        CHECK_HINTS(Qt::ImhFormattedNumbersOnly, CAPACITY_NUMBER)
        CHECK_HINTS(Qt::ImhUppercaseOnly, CAPACITY_UPPERCASE)
        CHECK_HINTS(Qt::ImhLowercaseOnly, CAPACITY_LOWERCASE)
        CHECK_HINTS(Qt::ImhDialableCharactersOnly, CAPACITY_DIALABLE)
        CHECK_HINTS(Qt::ImhEmailCharactersOnly, CAPACITY_EMAIL)
    }

    bool setSurrounding = false;
    do {
        if (!m_useSurroundingText)
            break;
        if (!((queries & Qt::ImSurroundingText) &&
              (queries & Qt::ImCursorPosition)))
            break;
        if (data.capability.testFlag(CAPACITY_PASSWORD))
            break;
        QVariant var = query.value(Qt::ImSurroundingText);
        QVariant var1 = query.value(Qt::ImCursorPosition);
        QVariant var2 = query.value(Qt::ImAnchorPosition);
        if (!var.isValid() || !var1.isValid())
            break;
        QString text = var.toString();
/* we don't want to waste too much memory here */
#define SURROUNDING_THRESHOLD 4096
        if (text.length() < SURROUNDING_THRESHOLD) {
            if (checkUtf8(text.toUtf8())) {
                addCapability(data, CAPACITY_SURROUNDING_TEXT);

                int cursor = var1.toInt();
                int anchor;
                if (var2.isValid())
                    anchor = var2.toInt();
                else
                    anchor = cursor;

                // adjust it to real character size
                QVector<uint> tempUCS4 = text.left(cursor).toUcs4();
                cursor = tempUCS4.size();
                tempUCS4 = text.left(anchor).toUcs4();
                anchor = tempUCS4.size();
                if (data.surroundingText != text) {
                    data.surroundingText = text;
                    proxy->setSurroundingText(text, cursor, anchor);
                } else {
                    if (data.surroundingAnchor != anchor ||
                        data.surroundingCursor != cursor)
                        proxy->setSurroundingTextPosition(cursor, anchor);
                }
                data.surroundingCursor = cursor;
                data.surroundingAnchor = anchor;
                setSurrounding = true;
            }
        }
        if (!setSurrounding) {
            data.surroundingAnchor = -1;
            data.surroundingCursor = -1;
            data.surroundingText = QString();
            removeCapability(data, CAPACITY_SURROUNDING_TEXT);
        }
    } while (0);
}

void QFcitxPlatformInputContext::commit() { QPlatformInputContext::commit(); }

void QFcitxPlatformInputContext::setFocusObject(QObject *object) {
    Q_UNUSED(object);
    FcitxInputContextProxy *proxy = validICByWindow(m_lastWindow);
    commitPreedit(m_lastObject);
    if (proxy) {
        proxy->focusOut();
    }

    QWindow *window = qApp->focusWindow();
    m_lastWindow = window;
    m_lastObject = object;
    // Always create IC Data for window.
    if (window) {
        proxy = validICByWindow(window);
        if (!proxy) {
            createICData(window);
        }
    }
    if (!window || (!inputMethodAccepted() && !objectAcceptsInputMethod())) {
        m_lastWindow = nullptr;
        m_lastObject = nullptr;
        return;
    }
    if (proxy) {
        proxy->focusIn();
        // We need to delegate this otherwise it may cause self-recursion in
        // certain application like libreoffice.
        auto window = m_lastWindow;
        QMetaObject::invokeMethod(
            this,
            [this, window]() {
                if (window != m_lastWindow) {
                    return;
                }
                if (validICByWindow(window.data())) {
                    cursorRectChanged();
                }
            },
            Qt::QueuedConnection);
    }
}

void QFcitxPlatformInputContext::windowDestroyed(QObject *object) {
    /* access QWindow is not possible here, so we use our own map to do so */
    m_icMap.erase(reinterpret_cast<QWindow *>(object));
    // qDebug() << "Window Destroyed and we destroy IC correctly, horray!";
}

void QFcitxPlatformInputContext::cursorRectChanged() {
    QWindow *inputWindow = qApp->focusWindow();
    if (!inputWindow)
        return;
    FcitxInputContextProxy *proxy = validICByWindow(inputWindow);
    if (!proxy)
        return;

    FcitxQtICData &data = *static_cast<FcitxQtICData *>(
        proxy->property("icData").value<void *>());

    QRect r = qApp->inputMethod()->cursorRectangle().toRect();
    if (!r.isValid())
        return;

    // not sure if this is necessary but anyway, qt's screen used to be buggy.
    if (!inputWindow->screen()) {
        return;
    }

    if (data.capability & CAPACITY_RELATIVE_CURSOR_RECT) {
        auto margins = inputWindow->frameMargins();
        r.translate(margins.left(), margins.top());
        if (data.rect != r) {
            data.rect = r;
            proxy->setCursorRect(r.x(), r.y(), r.width(), r.height());
        }
        return;
    }
    qreal scale = inputWindow->devicePixelRatio();
    auto screenGeometry = inputWindow->screen()->geometry();
    auto point = inputWindow->mapToGlobal(r.topLeft());
    auto native =
        (point - screenGeometry.topLeft()) * scale + screenGeometry.topLeft();
    QRect newRect(native, r.size() * scale);

    if (data.rect != newRect) {
        data.rect = newRect;
        proxy->setCursorRect(newRect.x(), newRect.y(), newRect.width(),
                             newRect.height());
    }
}

void QFcitxPlatformInputContext::createInputContextFinished() {
    FcitxInputContextProxy *proxy =
        qobject_cast<FcitxInputContextProxy *>(sender());
    if (!proxy) {
        return;
    }
    auto w = static_cast<QWindow *>(proxy->property("wid").value<void *>());
    FcitxQtICData *data =
        static_cast<FcitxQtICData *>(proxy->property("icData").value<void *>());
    data->rect = QRect();

    if (proxy->isValid()) {
        QWindow *window = qApp->focusWindow();
        if (window && window == w && inputMethodAccepted() &&
            objectAcceptsInputMethod()) {
            cursorRectChanged();
            proxy->focusIn();
        }
    }

    QFlags<FcitxCapabilityFlags> flag;
    flag |= CAPACITY_PREEDIT;
    flag |= CAPACITY_FORMATTED_PREEDIT;
    flag |= CAPACITY_CLIENT_UNFOCUS_COMMIT;
    flag |= CAPACITY_GET_IM_INFO_ON_FOCUS;
    m_useSurroundingText =
        get_boolean_env("FCITX_QT_ENABLE_SURROUNDING_TEXT", true);
    if (m_useSurroundingText) {
        flag |= CAPACITY_SURROUNDING_TEXT;
    }

    if (qApp && qApp->platformName() == "wayland") {
        flag |= CAPACITY_RELATIVE_CURSOR_RECT;
    }

    addCapability(*data, flag, true);
}

void QFcitxPlatformInputContext::updateCapability(const FcitxQtICData &data) {
    if (!data.proxy || !data.proxy->isValid())
        return;

    QDBusPendingReply<void> result =
        data.proxy->setCapability((uint)data.capability);
}

void QFcitxPlatformInputContext::commitString(const QString &str) {
    m_cursorPos = 0;
    m_preeditList.clear();
    m_commitPreedit.clear();
    QObject *input = qApp->focusObject();
    if (!input)
        return;

    QInputMethodEvent event;
    event.setCommitString(str);
    QCoreApplication::sendEvent(input, &event);
}

void QFcitxPlatformInputContext::updateFormattedPreedit(
    const FcitxFormattedPreeditList &preeditList, int cursorPos) {
    QObject *input = qApp->focusObject();
    if (!input)
        return;
    if (cursorPos == m_cursorPos && preeditList == m_preeditList)
        return;
    m_preeditList = preeditList;
    m_cursorPos = cursorPos;
    QString str, commitStr;
    int pos = 0;
    QList<QInputMethodEvent::Attribute> attrList;

    // Fcitx 5's flags support.
    enum TextFormatFlag : int {
        TextFormatFlag_Underline = (1 << 3), /**< underline is a flag */
        TextFormatFlag_HighLight = (1 << 4), /**< highlight the preedit */
        TextFormatFlag_DontCommit = (1 << 5),
        TextFormatFlag_Bold = (1 << 6),
        TextFormatFlag_Strike = (1 << 7),
        TextFormatFlag_Italic = (1 << 8),
    };

    Q_FOREACH (const FcitxFormattedPreedit &preedit, preeditList) {
        str += preedit.string();
        if (!(preedit.format() & TextFormatFlag_DontCommit))
            commitStr += preedit.string();
        QTextCharFormat format;
        if (preedit.format() & TextFormatFlag_Underline) {
            format.setUnderlineStyle(QTextCharFormat::DashUnderline);
        }
        if (preedit.format() & TextFormatFlag_Strike) {
            format.setFontStrikeOut(true);
        }
        if (preedit.format() & TextFormatFlag_Bold) {
            format.setFontWeight(QFont::Bold);
        }
        if (preedit.format() & TextFormatFlag_Italic) {
            format.setFontItalic(true);
        }
        if (preedit.format() & TextFormatFlag_HighLight) {
            QBrush brush;
            QPalette palette;
            palette = QGuiApplication::palette();
            format.setBackground(QBrush(
                QColor(palette.color(QPalette::Active, QPalette::Highlight))));
            format.setForeground(QBrush(QColor(
                palette.color(QPalette::Active, QPalette::HighlightedText))));
        }
        attrList.append(
            QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, pos,
                                         preedit.string().length(), format));
        pos += preedit.string().length();
    }

    QByteArray array = str.toUtf8();
    array.truncate(cursorPos);
    cursorPos = QString::fromUtf8(array).length();

    attrList.append(QInputMethodEvent::Attribute(QInputMethodEvent::Cursor,
                                                 cursorPos, 1, 0));
    m_preedit = str;
    m_commitPreedit = commitStr;
    QInputMethodEvent event(str, attrList);
    QCoreApplication::sendEvent(input, &event);
    update(Qt::ImCursorRectangle);
}

void QFcitxPlatformInputContext::deleteSurroundingText(int offset,
                                                       uint _nchar) {
    QObject *input = qApp->focusObject();
    if (!input)
        return;

    QInputMethodEvent event;

    FcitxInputContextProxy *proxy =
        qobject_cast<FcitxInputContextProxy *>(sender());
    if (!proxy) {
        return;
    }

    FcitxQtICData *data =
        static_cast<FcitxQtICData *>(proxy->property("icData").value<void *>());
    auto ucsText = data->surroundingText.toStdU32String();

    int cursor = data->surroundingCursor;
    // make nchar signed so we are safer
    int nchar = _nchar;
    // Qt's reconvert semantics is different from gtk's. It doesn't count the
    // current
    // selection. Discard selection from nchar.
    if (data->surroundingAnchor < data->surroundingCursor) {
        nchar -= data->surroundingCursor - data->surroundingAnchor;
        offset += data->surroundingCursor - data->surroundingAnchor;
        cursor = data->surroundingAnchor;
    } else if (data->surroundingAnchor > data->surroundingCursor) {
        nchar -= data->surroundingAnchor - data->surroundingCursor;
        cursor = data->surroundingCursor;
    }

    // validates
    if (nchar >= 0 && cursor + offset >= 0 &&
        cursor + offset + nchar <= static_cast<int>(ucsText.size())) {
        // order matters
        auto replacedChars = ucsText.substr(cursor + offset, nchar);
        nchar = QString::fromUcs4(replacedChars.data(), replacedChars.size())
                    .size();

        int start, len;
        if (offset >= 0) {
            start = cursor;
            len = offset;
        } else {
            start = cursor + offset;
            len = -offset;
        }

        auto prefixedChars = ucsText.substr(start, len);
        offset = QString::fromUcs4(prefixedChars.data(), prefixedChars.size())
                     .size() *
                 (offset >= 0 ? 1 : -1);
        event.setCommitString("", offset, nchar);
        QCoreApplication::sendEvent(input, &event);
    }
}

void QFcitxPlatformInputContext::forwardKey(uint keyval, uint state,
                                            bool type) {
    auto proxy = qobject_cast<FcitxInputContextProxy *>(sender());
    if (!proxy) {
        return;
    }
    FcitxQtICData &data = *static_cast<FcitxQtICData *>(
        proxy->property("icData").value<void *>());
    auto w = static_cast<QWindow *>(proxy->property("wid").value<void *>());
    QObject *input = qApp->focusObject();
    auto window = qApp->focusWindow();
    if (input && window && w == window) {
        std::unique_ptr<QKeyEvent> keyevent{
            createKeyEvent(keyval, state, type, data.event.get())};

        forwardEvent(window, *keyevent);
    }
}

void QFcitxPlatformInputContext::updateCurrentIM(const QString &name,
                                                 const QString &uniqueName,
                                                 const QString &langCode) {
    Q_UNUSED(name);
    Q_UNUSED(uniqueName);
    QLocale newLocale(langCode);
    if (m_locale != newLocale) {
        m_locale = newLocale;
        emitLocaleChanged();
    }
}

QLocale QFcitxPlatformInputContext::locale() const { return m_locale; }

void QFcitxPlatformInputContext::createICData(QWindow *w) {
    auto iter = m_icMap.find(w);
    if (iter == m_icMap.end()) {
        auto result =
            m_icMap.emplace(std::piecewise_construct, std::forward_as_tuple(w),
                            std::forward_as_tuple(m_watcher));
        connect(w, &QObject::destroyed, this,
                &QFcitxPlatformInputContext::windowDestroyed);
        iter = result.first;
        auto &data = iter->second;

        if (QGuiApplication::platformName() == QLatin1String("xcb")) {
            data.proxy->setDisplay("x11:");
        } else if (QGuiApplication::platformName() ==
                   QLatin1String("wayland")) {
            data.proxy->setDisplay("wayland:");
        }
        data.proxy->setProperty("wid",
                                QVariant::fromValue(static_cast<void *>(w)));
        data.proxy->setProperty(
            "icData", QVariant::fromValue(static_cast<void *>(&data)));
        connect(data.proxy, &FcitxInputContextProxy::inputContextCreated, this,
                &QFcitxPlatformInputContext::createInputContextFinished);
        connect(data.proxy, &FcitxInputContextProxy::commitString, this,
                &QFcitxPlatformInputContext::commitString);
        connect(data.proxy, &FcitxInputContextProxy::forwardKey, this,
                &QFcitxPlatformInputContext::forwardKey);
        connect(data.proxy, &FcitxInputContextProxy::updateFormattedPreedit,
                this, &QFcitxPlatformInputContext::updateFormattedPreedit);
        connect(data.proxy, &FcitxInputContextProxy::deleteSurroundingText,
                this, &QFcitxPlatformInputContext::deleteSurroundingText);
        connect(data.proxy, &FcitxInputContextProxy::currentIM, this,
                &QFcitxPlatformInputContext::updateCurrentIM);
    }
}

QKeyEvent *QFcitxPlatformInputContext::createKeyEvent(uint keyval, uint state,
                                                      bool isRelease,
                                                      const QKeyEvent *event) {
    QKeyEvent *newEvent = nullptr;
    if (event && event->nativeVirtualKey() == keyval &&
        event->nativeModifiers() == state &&
        isRelease == (event->type() == QEvent::KeyRelease)) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        newEvent = new QKeyEvent(
            event->type(), event->key(), event->modifiers(),
            event->nativeScanCode(), event->nativeVirtualKey(),
            event->nativeModifiers(), event->text(), event->isAutoRepeat(),
            event->count(), event->device());
#else
        newEvent = new QKeyEvent(*event);
#endif
    } else {
        Qt::KeyboardModifiers qstate = Qt::NoModifier;

        int count = 1;
        if (state & FcitxKeyState_Alt) {
            qstate |= Qt::AltModifier;
            count++;
        }

        if (state & FcitxKeyState_Shift) {
            qstate |= Qt::ShiftModifier;
            count++;
        }

        if (state & FcitxKeyState_Ctrl) {
            qstate |= Qt::ControlModifier;
            count++;
        }

        char32_t unicode = xkb_keysym_to_utf32(keyval);
        QString text;
        if (unicode) {
            text = QString::fromUcs4(&unicode, 1);
        }

        int key = keysymToQtKey(keyval, text);

        newEvent =
            new QKeyEvent(isRelease ? (QEvent::KeyRelease) : (QEvent::KeyPress),
                          key, qstate, 0, keyval, state, text, false, count);
        if (event) {
            newEvent->setTimestamp(event->timestamp());
        }
    }

    return newEvent;
}

void QFcitxPlatformInputContext::forwardEvent(QWindow *window,
                                              const QKeyEvent &keyEvent) {
    // use same variable name as in QXcbKeyboard::handleKeyEvent
    QEvent::Type type = keyEvent.type();
    int qtcode = keyEvent.key();
    Qt::KeyboardModifiers modifiers = keyEvent.modifiers();
    quint32 code = keyEvent.nativeScanCode();
    quint32 sym = keyEvent.nativeVirtualKey();
    quint32 state = keyEvent.nativeModifiers();
    QString string = keyEvent.text();
    bool isAutoRepeat = keyEvent.isAutoRepeat();
    ulong time = keyEvent.timestamp();
    // copied from QXcbKeyboard::handleKeyEvent()
    if (type == QEvent::KeyPress && qtcode == Qt::Key_Menu) {
        QPoint globalPos, pos;
        if (window->screen()) {
            globalPos = window->screen()->handle()->cursor()->pos();
            pos = window->mapFromGlobal(globalPos);
        }
        QWindowSystemInterface::handleContextMenuEvent(window, false, pos,
                                                       globalPos, modifiers);
    }
    QWindowSystemInterface::handleExtendedKeyEvent(window, time, type, qtcode,
                                                   modifiers, code, sym, state,
                                                   string, isAutoRepeat);
}

bool QFcitxPlatformInputContext::filterEvent(const QEvent *event) {
    do {
        if (event->type() != QEvent::KeyPress &&
            event->type() != QEvent::KeyRelease) {
            break;
        }

        const QKeyEvent *keyEvent = static_cast<const QKeyEvent *>(event);
        quint32 keyval = keyEvent->nativeVirtualKey();
        quint32 keycode = keyEvent->nativeScanCode();
        quint32 state = keyEvent->nativeModifiers();
        bool isRelease = keyEvent->type() == QEvent::KeyRelease;

        if (!inputMethodAccepted() && !objectAcceptsInputMethod())
            break;

        QObject *input = qApp->focusObject();

        if (!input) {
            break;
        }

        FcitxInputContextProxy *proxy = validICByWindow(qApp->focusWindow());

        if (!proxy) {
            if (filterEventFallback(keyval, keycode, state, isRelease)) {
                return true;
            } else {
                break;
            }
        }

        proxy->focusIn();

        auto reply = proxy->processKeyEvent(keyval, keycode, state, isRelease,
                                            keyEvent->timestamp());

        if (Q_UNLIKELY(m_syncMode)) {
            reply.waitForFinished();

            auto filtered = proxy->processKeyEventResult(reply);
            if (!filtered) {
                if (filterEventFallback(keyval, keycode, state, isRelease)) {
                    return true;
                } else {
                    break;
                }
            } else {
                update(Qt::ImCursorRectangle);
                return true;
            }
        } else {
            ProcessKeyWatcher *watcher = new ProcessKeyWatcher(
                *keyEvent, qApp->focusWindow(), reply, proxy);
            connect(watcher, &QDBusPendingCallWatcher::finished, this,
                    &QFcitxPlatformInputContext::processKeyEventFinished);
            return true;
        }
    } while (0);
    return QPlatformInputContext::filterEvent(event);
}

void QFcitxPlatformInputContext::processKeyEventFinished(
    QDBusPendingCallWatcher *w) {
    ProcessKeyWatcher *watcher = static_cast<ProcessKeyWatcher *>(w);
    auto proxy = qobject_cast<FcitxInputContextProxy *>(watcher->parent());
    bool filtered = false;

    QWindow *window = watcher->window();
    // if window is already destroyed, we can only throw this event away.
    if (!window) {
        delete watcher;
        return;
    }

    const QKeyEvent &keyEvent = watcher->keyEvent();

    // use same variable name as in QXcbKeyboard::handleKeyEvent
    QEvent::Type type = keyEvent.type();
    quint32 code = keyEvent.nativeScanCode();
    quint32 sym = keyEvent.nativeVirtualKey();
    quint32 state = keyEvent.nativeModifiers();
    QString string = keyEvent.text();

    if (!proxy->processKeyEventResult(*watcher)) {
        filtered =
            filterEventFallback(sym, code, state, type == QEvent::KeyRelease);
    } else {
        filtered = true;
    }

    if (!watcher->isError()) {
        update(Qt::ImCursorRectangle);
    }

    if (!filtered) {
        forwardEvent(window, keyEvent);
    } else {
        auto proxy = qobject_cast<FcitxInputContextProxy *>(watcher->parent());
        if (proxy) {
            FcitxQtICData &data = *static_cast<FcitxQtICData *>(
                proxy->property("icData").value<void *>());
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            data.event = std::make_unique<QKeyEvent>(
                keyEvent.type(), keyEvent.key(), keyEvent.modifiers(),
                keyEvent.nativeScanCode(), keyEvent.nativeVirtualKey(),
                keyEvent.nativeModifiers(), keyEvent.text(),
                keyEvent.isAutoRepeat(), keyEvent.count(), keyEvent.device());
#else
            data.event = std::unique_ptr<QKeyEvent>(new QKeyEvent(keyEvent));
#endif
        }
    }

    delete watcher;
}

bool QFcitxPlatformInputContext::filterEventFallback(uint keyval, uint keycode,
                                                     uint state,
                                                     bool isRelease) {
    Q_UNUSED(keycode);
    if (processCompose(keyval, state, isRelease)) {
        return true;
    }
    return false;
}

FcitxInputContextProxy *QFcitxPlatformInputContext::validIC() {
    if (m_icMap.empty()) {
        return nullptr;
    }
    QWindow *window = qApp->focusWindow();
    return validICByWindow(window);
}

FcitxInputContextProxy *
QFcitxPlatformInputContext::validICByWindow(QWindow *w) {
    if (!w) {
        return nullptr;
    }

    if (m_icMap.empty()) {
        return nullptr;
    }
    auto iter = m_icMap.find(w);
    if (iter == m_icMap.end())
        return nullptr;
    auto &data = iter->second;
    if (!data.proxy || !data.proxy->isValid()) {
        return nullptr;
    }
    return data.proxy;
}

bool QFcitxPlatformInputContext::processCompose(uint keyval, uint state,
                                                bool isRelease) {
    Q_UNUSED(state);

    if (!m_xkbComposeTable || isRelease)
        return false;

    struct xkb_compose_state *xkbComposeState = m_xkbComposeState.data();

    enum xkb_compose_feed_result result =
        xkb_compose_state_feed(xkbComposeState, keyval);
    if (result == XKB_COMPOSE_FEED_IGNORED) {
        return false;
    }

    enum xkb_compose_status status =
        xkb_compose_state_get_status(xkbComposeState);
    if (status == XKB_COMPOSE_NOTHING) {
        return 0;
    } else if (status == XKB_COMPOSE_COMPOSED) {
        char buffer[] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0'};
        int length =
            xkb_compose_state_get_utf8(xkbComposeState, buffer, sizeof(buffer));
        xkb_compose_state_reset(xkbComposeState);
        if (length != 0) {
            commitString(QString::fromUtf8(buffer));
        }

    } else if (status == XKB_COMPOSE_CANCELLED) {
        xkb_compose_state_reset(xkbComposeState);
    }

    return true;
}

// kate: indent-mode cstyle; space-indent on; indent-width 0;
