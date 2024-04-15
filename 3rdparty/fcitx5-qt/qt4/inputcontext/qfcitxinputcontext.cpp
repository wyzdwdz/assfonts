/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <QApplication>
#include <QDBusConnection>
#include <QDateTime>
#include <QDebug>
#include <QKeyEvent>
#include <QPalette>
#include <QTextCharFormat>
#include <QTextCodec>
#include <QWidget>
#ifdef ENABLE_X11
#include <QX11Info>
#endif
#include <array>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include "qtkey.h"

#include "fcitxflags.h"
#include "fcitxqtinputcontextproxy.h"
#include "fcitxqtwatcher.h"
#include "qfcitxinputcontext.h"

#ifdef ENABLE_X11
#include <X11/Xlib.h>
#endif
#include <memory>
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut

namespace fcitx {

#ifdef ENABLE_X11
void setFocusGroupForX11(const QByteArray &uuid) {
    if (uuid.size() != 16) {
        return;
    }

    Display *xdisplay = QX11Info::display();
    if (!xdisplay) {
        return;
    }

    Atom atom = XInternAtom(xdisplay, "_FCITX_SERVER", False);
    if (!atom) {
        return;
    }
    Window window = XGetSelectionOwner(xdisplay, atom);
    if (!window) {
        return;
    }
    XEvent ev;

    memset(&ev, 0, sizeof(ev));
    ev.xclient.type = ClientMessage;
    ev.xclient.window = window;
    ev.xclient.message_type = atom;
    ev.xclient.format = 8;
    memcpy(ev.xclient.data.b, uuid.constData(), 16);

    XSendEvent(xdisplay, window, False, NoEventMask, &ev);
    XSync(xdisplay, False);
}
#endif

static bool key_filtered = false;

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

struct xkb_context *_xkb_context_new_helper() {
    struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (context) {
        xkb_context_set_log_level(context, XKB_LOG_LEVEL_CRITICAL);
    }

    return context;
}

QFcitxInputContext::QFcitxInputContext()
    : watcher_(new FcitxQtWatcher(
          QDBusConnection::connectToBus(QDBusConnection::SessionBus, "fcitx"),
          this)),
      cursorPos_(0), useSurroundingText_(false),
      syncMode_(get_boolean_env("FCITX_QT_USE_SYNC", false)), destroy_(false),
      xkbContext_(_xkb_context_new_helper()),
      xkbComposeTable_(xkbContext_ ? xkb_compose_table_new_from_locale(
                                         xkbContext_.data(), get_locale(),
                                         XKB_COMPOSE_COMPILE_NO_FLAGS)
                                   : 0),
      xkbComposeState_(xkbComposeTable_
                           ? xkb_compose_state_new(xkbComposeTable_.data(),
                                                   XKB_COMPOSE_STATE_NO_FLAGS)
                           : 0) {
    registerFcitxQtDBusTypes();
    watcher_->setWatchPortal(true);
    watcher_->watch();
}

QFcitxInputContext::~QFcitxInputContext() {
    destroy_ = true;
    watcher_->unwatch();
    cleanUp();
    delete watcher_;
}

void QFcitxInputContext::cleanUp() {
    icMap_.clear();

    if (!destroy_) {
        commitPreedit();
    }
}

void QFcitxInputContext::mouseHandler(int cursorPosition, QMouseEvent *event) {
    if (event->type() != QEvent::MouseButtonRelease) {
        return;
    }

    unsigned int action;
    if (event->button() == Qt::LeftButton) {
        action = 0;
    } else if (event->button() == Qt::RightButton) {
        action = 1;
    } else {
        return;
    }
    if (FcitxQtInputContextProxy *proxy = validIC();
        proxy->supportInvokeAction()) {
        proxy->invokeAction(action, cursorPosition);
    } else {
        if (cursorPosition <= 0 || cursorPosition >= preedit_.length()) {
            // qDebug() << action << cursorPosition;
            reset();
        }
    }
}

void QFcitxInputContext::commitPreedit(QPointer<QWidget> input) {
    if (!input)
        return;
    if (commitPreedit_.length() <= 0)
        return;
    QInputMethodEvent e;
    e.setCommitString(commitPreedit_);
    QCoreApplication::sendEvent(input, &e);
    commitPreedit_.clear();
    preeditList_.clear();
}

bool checkUtf8(const QByteArray &byteArray) {
    QTextCodec::ConverterState state;
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    const QString text =
        codec->toUnicode(byteArray.constData(), byteArray.size(), &state);
    return state.invalidChars == 0;
}

void QFcitxInputContext::reset() {
    commitPreedit();
    if (FcitxQtInputContextProxy *proxy = validIC()) {
        proxy->reset();
    }
    if (xkbComposeState_) {
        xkb_compose_state_reset(xkbComposeState_.data());
    }
}

void QFcitxInputContext::update() {
    QWidget *window = qApp->focusWidget();
    FcitxQtInputContextProxy *proxy = validICByWindow(window);
    if (!proxy)
        return;

    FcitxQtICData &data = *static_cast<FcitxQtICData *>(
        proxy->property("icData").value<void *>());

    QWidget *input = qApp->focusWidget();
    if (!input)
        return;

    cursorRectChanged();

    if (true) {
        Qt::InputMethodHints hints = input->inputMethodHints();

#define CHECK_HINTS(_HINTS, _CAPABILITY)                                       \
    if (hints & _HINTS)                                                        \
        addCapability(data, FcitxCapabilityFlag_##_CAPABILITY);                \
    else                                                                       \
        removeCapability(data, FcitxCapabilityFlag_##_CAPABILITY);

        CHECK_HINTS(Qt::ImhHiddenText, Password)
        CHECK_HINTS(Qt::ImhNoAutoUppercase, NoAutoUpperCase)
        CHECK_HINTS(Qt::ImhPreferNumbers, Number)
        CHECK_HINTS(Qt::ImhPreferUppercase, Uppercase)
        CHECK_HINTS(Qt::ImhPreferLowercase, Lowercase)
        CHECK_HINTS(Qt::ImhNoPredictiveText, NoSpellCheck)
        CHECK_HINTS(Qt::ImhDigitsOnly, Digit)
        CHECK_HINTS(Qt::ImhFormattedNumbersOnly, Number)
        CHECK_HINTS(Qt::ImhUppercaseOnly, Uppercase)
        CHECK_HINTS(Qt::ImhLowercaseOnly, Lowercase)
        CHECK_HINTS(Qt::ImhDialableCharactersOnly, Dialable)
        CHECK_HINTS(Qt::ImhEmailCharactersOnly, Email)
        CHECK_HINTS(Qt::ImhUrlCharactersOnly, Url)
    }

    bool setSurrounding = false;
    do {
        if (!useSurroundingText_)
            break;
        if ((data.capability & FcitxCapabilityFlag_Password) ||
            (data.capability & FcitxCapabilityFlag_Sensitive))
            break;
        QVariant var = input->inputMethodQuery(Qt::ImSurroundingText);
        QVariant var1 = input->inputMethodQuery(Qt::ImCursorPosition);
        QVariant var2 = input->inputMethodQuery(Qt::ImAnchorPosition);
        if (!var.isValid() || !var1.isValid())
            break;
        QString text = var.toString();
/* we don't want to waste too much memory here */
#define SURROUNDING_THRESHOLD 4096
        if (text.length() < SURROUNDING_THRESHOLD) {
            if (checkUtf8(text.toUtf8())) {
                addCapability(data, FcitxCapabilityFlag_SurroundingText);

                int cursor = var1.toInt();
                int anchor;
                if (var2.isValid()) {
                    anchor = var2.toInt();
                } else {
                    anchor = cursor;
                }

                // adjust it to real character size
#if QT_VERSION >= QT_VERSION_CHECK(4, 8, 0)
                QVector<unsigned int> tempUCS4 = text.leftRef(cursor).toUcs4();
#else
                QVector<unsigned int> tempUCS4 = text.left(cursor).toUcs4();
#endif
                cursor = tempUCS4.size();
#if QT_VERSION >= QT_VERSION_CHECK(4, 8, 0)
                tempUCS4 = text.leftRef(anchor).toUcs4();
#else
                tempUCS4 = text.left(anchor).toUcs4();
#endif
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
            removeCapability(data, FcitxCapabilityFlag_SurroundingText);
        }
    } while (0);
}

void QFcitxInputContext::setFocusWidget(QWidget *object) {
    Q_UNUSED(object);
    FcitxQtInputContextProxy *proxy = validICByWindow(lastWindow_);
    if (proxy) {
        proxy->focusOut();
    }

    QWidget *window = qApp->focusWidget();
    lastWindow_ = window;
    if (!window) {
        return;
    }
    proxy = validICByWindow(window);
    if (proxy) {
        cursorRectChanged();
        proxy->focusIn();
    } else {
        createICData(window);
    }
    QInputContext::setFocusWidget(object);
}

void QFcitxInputContext::widgetDestroyed(QWidget *w) {
    QInputContext::widgetDestroyed(w);

    icMap_.erase(w);
}

void QFcitxInputContext::windowDestroyed(QObject *object) {
    /* access QWindow is not possible here, so we use our own map to do so */
    icMap_.erase(reinterpret_cast<QWidget *>(object));
    // qDebug() << "Window Destroyed and we destroy IC correctly, horray!";
}

void QFcitxInputContext::cursorRectChanged() {
    QWidget *inputWindow = qApp->focusWidget();
    if (!inputWindow)
        return;
    FcitxQtInputContextProxy *proxy = validICByWindow(inputWindow);
    if (!proxy)
        return;

    FcitxQtICData &data = *static_cast<FcitxQtICData *>(
        proxy->property("icData").value<void *>());

    QRect r = inputWindow->inputMethodQuery(Qt::ImMicroFocus).toRect();

    auto point = inputWindow->mapToGlobal(r.topLeft());
    QRect newRect(point, r.size());

    if (data.rect != newRect) {
        data.rect = newRect;
        proxy->setCursorRect(newRect.x(), newRect.y(), newRect.width(),
                             newRect.height());
    }
}

void QFcitxInputContext::createInputContextFinished(const QByteArray &uuid) {
    auto proxy = qobject_cast<FcitxQtInputContextProxy *>(sender());
    if (!proxy) {
        return;
    }
    auto w =
        reinterpret_cast<QWidget *>(proxy->property("wid").value<void *>());
    FcitxQtICData *data =
        static_cast<FcitxQtICData *>(proxy->property("icData").value<void *>());
    data->rect = QRect();

    if (proxy->isValid()) {
        QWidget *window = qApp->focusWidget();
#ifdef ENABLE_X11
        setFocusGroupForX11(uuid);
#else
        Q_UNUSED(uuid);
#endif
        if (window && window == w) {
            cursorRectChanged();
            proxy->focusIn();
        }
    }

    quint64 flag = 0;
    flag |= FcitxCapabilityFlag_Preedit;
    flag |= FcitxCapabilityFlag_FormattedPreedit;
    flag |= FcitxCapabilityFlag_ClientUnfocusCommit;
    flag |= FcitxCapabilityFlag_GetIMInfoOnFocus;
    flag |= FcitxCapabilityFlag_KeyEventOrderFix;
    flag |= FcitxCapabilityFlag_ReportKeyRepeat;
    useSurroundingText_ =
        get_boolean_env("FCITX_QT_ENABLE_SURROUNDING_TEXT", true);
    if (useSurroundingText_)
        flag |= FcitxCapabilityFlag_SurroundingText;

    addCapability(*data, flag, true);
}

void QFcitxInputContext::updateCapability(const FcitxQtICData &data) {
    if (!data.proxy || !data.proxy->isValid())
        return;

    QDBusPendingReply<void> result = data.proxy->setCapability(data.capability);
}

void QFcitxInputContext::commitString(const QString &str) {
    cursorPos_ = 0;
    preeditList_.clear();
    commitPreedit_.clear();
    QWidget *input = qApp->focusWidget();
    if (!input)
        return;

    QInputMethodEvent event;
    event.setCommitString(str);
    QCoreApplication::sendEvent(input, &event);
}

void QFcitxInputContext::updateFormattedPreedit(
    const FcitxQtFormattedPreeditList &preeditList, int cursorPos) {
    QWidget *input = qApp->focusWidget();
    if (!input)
        return;
    if (cursorPos == cursorPos_ && preeditList == preeditList_)
        return;
    preeditList_ = preeditList;
    cursorPos_ = cursorPos;
    QString str, commitStr;
    int pos = 0;
    QList<QInputMethodEvent::Attribute> attrList;
    Q_FOREACH (const FcitxQtFormattedPreedit &preedit, preeditList) {
        str += preedit.string();
        if (!(preedit.format() & FcitxTextFormatFlag_DontCommit))
            commitStr += preedit.string();
        QTextCharFormat format;
        if (preedit.format() & FcitxTextFormatFlag_Underline) {
            format.setUnderlineStyle(QTextCharFormat::DashUnderline);
        }
        if (preedit.format() & FcitxTextFormatFlag_Strike) {
            format.setFontStrikeOut(true);
        }
        if (preedit.format() & FcitxTextFormatFlag_Bold) {
            format.setFontWeight(QFont::Bold);
        }
        if (preedit.format() & FcitxTextFormatFlag_Italic) {
            format.setFontItalic(true);
        }
        if (preedit.format() & FcitxTextFormatFlag_HighLight) {
            QBrush brush;
            QPalette palette;
            palette = QApplication::palette();
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
    preedit_ = str;
    commitPreedit_ = commitStr;
    QInputMethodEvent event(str, attrList);
    QCoreApplication::sendEvent(input, &event);
    update();
}

void QFcitxInputContext::deleteSurroundingText(int offset,
                                               unsigned int _nchar) {
    QWidget *input = qApp->focusWidget();
    if (!input)
        return;

    QInputMethodEvent event;

    FcitxQtInputContextProxy *proxy =
        qobject_cast<FcitxQtInputContextProxy *>(sender());
    if (!proxy) {
        return;
    }

    FcitxQtICData *data =
        static_cast<FcitxQtICData *>(proxy->property("icData").value<void *>());
    QVector<unsigned int> ucsText = data->surroundingText.toUcs4();

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
        cursor + offset + nchar <= ucsText.size()) {
        // order matters
        QVector<unsigned int> replacedChars =
            ucsText.mid(cursor + offset, nchar);
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

        QVector<unsigned int> prefixedChars = ucsText.mid(start, len);
        offset = QString::fromUcs4(prefixedChars.data(), prefixedChars.size())
                     .size() *
                 (offset >= 0 ? 1 : -1);
        event.setCommitString("", offset, nchar);
        QCoreApplication::sendEvent(input, &event);
    }
}

void QFcitxInputContext::forwardKey(unsigned int keyval, unsigned int state,
                                    bool type) {
    auto proxy = qobject_cast<FcitxQtInputContextProxy *>(sender());
    if (!proxy) {
        return;
    }
    FcitxQtICData &data = *static_cast<FcitxQtICData *>(
        proxy->property("icData").value<void *>());
    QWidget *input = qApp->focusWidget();
    if (input != nullptr) {
        key_filtered = true;
        QKeyEvent *keyevent =
            createKeyEvent(keyval, state, type, data.event.get());

        QCoreApplication::sendEvent(input, keyevent);
        delete keyevent;
        key_filtered = false;
    }
}

void QFcitxInputContext::serverSideFocusOut() {
    if (lastWindow_ == qApp->focusWidget()) {
        commitPreedit();
    }
}

void QFcitxInputContext::createICData(QWidget *w) {
    auto iter = icMap_.find(w);
    if (iter == icMap_.end()) {
        auto result =
            icMap_.emplace(std::piecewise_construct, std::forward_as_tuple(w),
                           std::forward_as_tuple(watcher_));
        iter = result.first;
        auto &data = iter->second;

        data.proxy->setDisplay("x11:");
        data.proxy->setProperty("wid",
                                qVariantFromValue(static_cast<void *>(w)));
        data.proxy->setProperty("icData",
                                qVariantFromValue(static_cast<void *>(&data)));
        connect(data.proxy, SIGNAL(inputContextCreated(QByteArray)), this,
                SLOT(createInputContextFinished(QByteArray)));
        connect(data.proxy, SIGNAL(commitString(QString)), this,
                SLOT(commitString(QString)));
        connect(data.proxy,
                SIGNAL(forwardKey(unsigned int, unsigned int, bool)), this,
                SLOT(forwardKey(unsigned int, unsigned int, bool)));
        connect(
            data.proxy,
            SIGNAL(updateFormattedPreedit(FcitxQtFormattedPreeditList, int)),
            this,
            SLOT(updateFormattedPreedit(FcitxQtFormattedPreeditList, int)));
        connect(data.proxy, SIGNAL(deleteSurroundingText(int, uint)), this,
                SLOT(deleteSurroundingText(int, uint)));
        connect(data.proxy, SIGNAL(notifyFocusOut()), this,
                SLOT(serverSideFocusOut()));
    }
}

QKeyEvent *QFcitxInputContext::createKeyEvent(unsigned int keyval,
                                              unsigned int state,
                                              bool isRelease,
                                              const QKeyEvent *event) {
    QKeyEvent *newEvent = nullptr;
    if (event && event->nativeVirtualKey() == keyval &&
        event->nativeModifiers() == state &&
        isRelease == (event->type() == QEvent::KeyRelease)) {
        newEvent = new QKeyEvent(*event);
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

        auto unicode = xkb_keysym_to_utf32(keyval);
        QString text;
        if (unicode) {
            text = QString::fromUcs4(&unicode, 1);
        }

        int key = keysymToQtKey(keyval, text);

        newEvent = QKeyEvent::createExtendedKeyEvent(
            isRelease ? (QEvent::KeyRelease) : (QEvent::KeyPress), key, qstate,
            0, keyval, state, text, false, count);
    }

    return newEvent;
}

bool QFcitxInputContext::filterEvent(const QEvent *event) {
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

        if (key_filtered) {
            break;
        }

        QWidget *input = qApp->focusWidget();

        if (!input) {
            break;
        }

        FcitxQtInputContextProxy *proxy = validICByWindow(input);

        if (!proxy) {
            if (filterEventFallback(keyval, keycode, state, isRelease)) {
                return true;
            } else {
                break;
            }
        }

        proxy->focusIn();
        update();

        auto stateToFcitx = state;
        if (keyEvent->isAutoRepeat()) {
            // KeyState::Repeat
            stateToFcitx |= (1u << 31);
        }
        auto reply =
            proxy->processKeyEvent(keyval, keycode, stateToFcitx, isRelease,
                                   QDateTime::currentDateTime().toTime_t());

#if QT_VERSION >= QT_VERSION_CHECK(4, 8, 0)
        if (Q_UNLIKELY(syncMode_)) {
#else
        if (syncMode_) {
#endif
            reply.waitForFinished();

            if (reply.isError() || !reply.value()) {
                if (filterEventFallback(keyval, keycode, state, isRelease)) {
                    return true;
                } else {
                    break;
                }
            } else {
                update();
                return true;
            }
        } else {
            ProcessKeyWatcher *watcher = new ProcessKeyWatcher(
                *keyEvent, qApp->focusWidget(), reply, proxy);
            connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher *)), this,
                    SLOT(processKeyEventFinished(QDBusPendingCallWatcher *)));
            return true;
        }
    } while (0);
    return QInputContext::filterEvent(event);
}

void QFcitxInputContext::processKeyEventFinished(QDBusPendingCallWatcher *w) {
    ProcessKeyWatcher *watcher = static_cast<ProcessKeyWatcher *>(w);
    QDBusPendingReply<bool> result(*watcher);
    bool filtered = false;

    QWidget *window = watcher->window();
    // if window is already destroyed, we can only throw this event away.
    if (!window) {
        delete watcher;
        return;
    }

    QKeyEvent &keyEvent = watcher->keyEvent();

    // use same variable name as in QXcbKeyboard::handleKeyEvent
    QEvent::Type type = keyEvent.type();
    quint32 code = keyEvent.nativeScanCode();
    quint32 sym = keyEvent.nativeVirtualKey();
    quint32 state = keyEvent.nativeModifiers();
    QString string = keyEvent.text();

    if (result.isError() || !result.value()) {
        filtered =
            filterEventFallback(sym, code, state, type == QEvent::KeyRelease);
    } else {
        filtered = true;
    }

    if (!watcher->isError()) {
        update();
    }

    if (!filtered) {
        key_filtered = true;
        QCoreApplication::sendEvent(window, &keyEvent);
        key_filtered = false;
    } else {
        auto proxy =
            qobject_cast<FcitxQtInputContextProxy *>(watcher->parent());
        if (proxy) {
            FcitxQtICData &data = *static_cast<FcitxQtICData *>(
                proxy->property("icData").value<void *>());
            data.event = std::make_unique<QKeyEvent>(keyEvent);
        }
    }

    delete watcher;
}

bool QFcitxInputContext::filterEventFallback(unsigned int keyval,
                                             unsigned int keycode,
                                             unsigned int state,
                                             bool isRelease) {
    Q_UNUSED(keycode);
    if (processCompose(keyval, state, isRelease)) {
        return true;
    }
    return false;
}

FcitxQtInputContextProxy *QFcitxInputContext::validIC() {
    if (icMap_.empty()) {
        return nullptr;
    }
    QWidget *window = qApp->focusWidget();
    return validICByWindow(window);
}

FcitxQtInputContextProxy *QFcitxInputContext::validICByWindow(QWidget *w) {
    if (!w) {
        return nullptr;
    }

    if (icMap_.empty()) {
        return nullptr;
    }
    auto iter = icMap_.find(w);
    if (iter == icMap_.end())
        return nullptr;
    auto &data = iter->second;
    if (!data.proxy || !data.proxy->isValid()) {
        return nullptr;
    }
    return data.proxy;
}

bool QFcitxInputContext::processCompose(unsigned int keyval, unsigned int state,
                                        bool isRelease) {
    Q_UNUSED(state);

    if (!xkbComposeTable_ || isRelease)
        return false;

    struct xkb_compose_state *xkbComposeState = xkbComposeState_.data();

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
        std::array<char, 256> buffer;
        int length = xkb_compose_state_get_utf8(xkbComposeState, buffer.data(),
                                                buffer.size());
        xkb_compose_state_reset(xkbComposeState);
        if (length != 0) {
            commitString(QString::fromUtf8(buffer.data(), length));
        }
    } else if (status == XKB_COMPOSE_CANCELLED) {
        xkb_compose_state_reset(xkbComposeState);
    }

    return true;
}

QString QFcitxInputContext::identifierName() { return "fcitx5"; }

QString QFcitxInputContext::language() { return ""; }
} // namespace fcitx

// kate: indent-mode cstyle; space-indent on; indent-width 0;
