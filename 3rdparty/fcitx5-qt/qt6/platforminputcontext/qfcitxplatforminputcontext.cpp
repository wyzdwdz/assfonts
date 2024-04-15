/*
 * SPDX-FileCopyrightText: 2011~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <QDBusConnection>
#include <QDebug>
#include <QInputMethod>
#include <QKeyEvent>
#include <QMetaMethod>
#include <QPalette>
#include <QTextCharFormat>
#include <QWidget>
#include <QWindow>
#include <qnamespace.h>
#include <qpa/qplatformcursor.h>
#include <qpa/qplatformnativeinterface.h>
#include <qpa/qplatformscreen.h>
#include <qpa/qwindowsysteminterface.h>

#include <cerrno>
#include <csignal>
#include <unistd.h>

#include "fcitx4watcher.h"
#include "fcitxcandidatewindow.h"
#include "fcitxflags.h"
#include "fcitxtheme.h"
#include "hybridinputcontext.h"
#include "qfcitxplatforminputcontext.h"
#include "qtkey.h"

#include <array>
#include <memory>
#ifdef ENABLE_X11
#include <xcb/xcb.h>
#endif

namespace fcitx {

namespace {

template <typename T>
using XCBReply = std::unique_ptr<T, decltype(&std::free)>;

template <typename T>
XCBReply<T> makeXCBReply(T *ptr) noexcept {
    return {ptr, &std::free};
}

#ifdef ENABLE_X11
void setFocusGroupForX11(const QByteArray &uuid) {
    if (uuid.size() != 16) {
        return;
    }

    if (QGuiApplication::platformName() != QLatin1String("xcb")) {
        return;
    }

    auto *native = QGuiApplication::platformNativeInterface();
    if (!native) {
        return;
    }

    auto *connection = static_cast<xcb_connection_t *>(
        native->nativeResourceForIntegration(QByteArray("connection")));

    if (!connection) {
        return;
    }

    xcb_atom_t result = XCB_ATOM_NONE;
    {
        char atomName[] = "_FCITX_SERVER";
        xcb_intern_atom_cookie_t cookie =
            xcb_intern_atom(connection, false, strlen(atomName), atomName);
        auto reply =
            makeXCBReply(xcb_intern_atom_reply(connection, cookie, nullptr));
        if (reply) {
            result = reply->atom;
        }

        if (result == XCB_ATOM_NONE) {
            return;
        }
    }

    xcb_window_t owner = XCB_WINDOW_NONE;
    {
        auto cookie = xcb_get_selection_owner(connection, result);
        auto reply = makeXCBReply(
            xcb_get_selection_owner_reply(connection, cookie, nullptr));
        if (reply) {
            owner = reply->owner;
        }
    }
    if (owner == XCB_WINDOW_NONE) {
        return;
    }

    xcb_client_message_event_t ev;

    memset(&ev, 0, sizeof(ev));
    ev.response_type = XCB_CLIENT_MESSAGE;
    ev.window = owner;
    ev.type = result;
    ev.format = 8;
    memcpy(ev.data.data8, uuid.constData(), 16);

    xcb_send_event(connection, false, owner, XCB_EVENT_MASK_NO_EVENT,
                   reinterpret_cast<char *>(&ev));
    xcb_flush(connection);
}
#endif

bool get_boolean_env(const char *name, bool defval) {
    const char *value = getenv(name);

    if (value == nullptr) {
        return defval;
    }

    if (strcmp(value, "") == 0 || strcmp(value, "0") == 0 ||
        strcmp(value, "false") == 0 || strcmp(value, "False") == 0 ||
        strcmp(value, "FALSE") == 0) {
        return false;
    }

    return true;
}

inline const char *get_locale() {
    const char *locale = getenv("LC_ALL");
    if (!locale) {
        locale = getenv("LC_CTYPE");
    }
    if (!locale) {
        locale = getenv("LANG");
    }
    if (!locale) {
        locale = "C";
    }

    return locale;
}

struct xkb_context *_xkb_context_new_helper() {
    struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (context) {
        xkb_context_set_log_level(context, XKB_LOG_LEVEL_CRITICAL);
    }

    return context;
}

QObject *deepestFocusProxy(QObject *object) {
    auto *widget = qobject_cast<QWidget *>(object);
    if (!widget) {
        return object;
    }

    while (auto *proxy = widget->focusProxy()) {
        widget = proxy;
    }
    return widget;
}

} // namespace

FcitxQtICData::FcitxQtICData(QFcitxPlatformInputContext *context,
                             QWindow *window)
    : proxy(new HybridInputContext(context->watcher(), context->fcitx4Watcher(),
                                   context)),
      context_(context), window_(window) {
    proxy->setProperty("icData",
                       QVariant::fromValue(static_cast<void *>(this)));
    QObject::connect(window, &QWindow::visibilityChanged, proxy,
                     [this](bool visible) {
                         if (!visible) {
                             resetCandidateWindow();
                         }
                     });
    QObject::connect(context_->watcher(), &FcitxQtWatcher::availabilityChanged,
                     proxy, [this](bool avail) {
                         if (!avail) {
                             resetCandidateWindow();
                         }
                     });
    window->installEventFilter(this);
}
FcitxQtICData::~FcitxQtICData() {
    if (window_) {
        window_->removeEventFilter(this);
    }
    delete proxy;
    resetCandidateWindow();
}

bool FcitxQtICData::eventFilter(QObject * /*watched*/, QEvent *event) {
    if (event->type() != QMouseEvent::MouseButtonPress) {
        return false;
    }
    auto *focusObject = context_->focusObjectWrapper();
    if (!focusObject) {
        return false;
    }
    if (!window_ || window_ != context_->focusWindowWrapper() ||
        !context_->hasPreedit()) {
        return false;
    }
    if (focusObject->metaObject()->className() ==
            QLatin1String("KateViewInternal") ||
        (focusObject->metaObject()->className() == QLatin1String("QtWidget") &&
         QCoreApplication::applicationFilePath().endsWith("soffice.bin")) ||
        focusObject->metaObject()->className() ==
            QLatin1String("Konsole::TerminalDisplay")) {
        if (context_->commitPreedit()) {
            if (proxy->isValid()) {
                proxy->reset();
            }
        }
    }
    return false;
}

FcitxCandidateWindow *FcitxQtICData::candidateWindow() {
    if (!candidateWindow_) {
        candidateWindow_ = new FcitxCandidateWindow(window(), context_);
        QObject::connect(
            candidateWindow_, &FcitxCandidateWindow::candidateSelected, proxy,
            [proxy = proxy](int index) { proxy->selectCandidate(index); });
        QObject::connect(candidateWindow_, &FcitxCandidateWindow::prevClicked,
                         proxy, [proxy = proxy]() { proxy->prevPage(); });
        QObject::connect(candidateWindow_, &FcitxCandidateWindow::nextClicked,
                         proxy, [proxy = proxy]() { proxy->nextPage(); });
    }
    return candidateWindow_;
}

void FcitxQtICData::resetCandidateWindow() {
    if (auto *w = candidateWindow_.data()) {
        candidateWindow_ = nullptr;
        w->deleteLater();
        return;
    }
}

QFcitxPlatformInputContext::QFcitxPlatformInputContext()
    : watcher_(new FcitxQtWatcher(
          QDBusConnection::connectToBus(QDBusConnection::SessionBus, "fcitx"),
          this)),
      fcitx4Watcher_(new Fcitx4Watcher(watcher_->connection(), this)),
      cursorPos_(0), useSurroundingText_(false),
      syncMode_(get_boolean_env("FCITX_QT_USE_SYNC", false)), destroy_(false),
      xkbContext_(_xkb_context_new_helper()),
      xkbComposeTable_(xkbContext_ ? xkb_compose_table_new_from_locale(
                                         xkbContext_.data(), get_locale(),
                                         XKB_COMPOSE_COMPILE_NO_FLAGS)
                                   : nullptr),
      xkbComposeState_(xkbComposeTable_
                           ? xkb_compose_state_new(xkbComposeTable_.data(),
                                                   XKB_COMPOSE_STATE_NO_FLAGS)
                           : nullptr) {
    registerFcitxQtDBusTypes();
    watcher_->setWatchPortal(true);

    // Input context may be created without QApplication with wayland, defer it
    // to event loop to ensure event dispatcher is avaiable.
    QTimer::singleShot(0, this, [this]() {
        watcher_->watch();
        fcitx4Watcher_->watch();
    });
}

QFcitxPlatformInputContext::~QFcitxPlatformInputContext() {
    destroy_ = true;
    watcher_->unwatch();
    fcitx4Watcher_->unwatch();
    cleanUp();
    delete watcher_;
    delete fcitx4Watcher_;
}

bool QFcitxPlatformInputContext::objectAcceptsInputMethod() const {
    bool enabled = false;
    QObject *object = qGuiApp->focusObject();
    if (object) {
        QInputMethodQueryEvent query(Qt::ImEnabled);
        QGuiApplication::sendEvent(object, &query);
        enabled = query.value(Qt::ImEnabled).toBool();
    }

    QObject *realFocusObject = focusObjectWrapper();
    // Make sure we don't query same object twice.
    if (realFocusObject && realFocusObject != object && !enabled) {
        QInputMethodQueryEvent query(Qt::ImEnabled);
        QGuiApplication::sendEvent(realFocusObject, &query);
        enabled = query.value(Qt::ImEnabled).toBool();
    }

    return enabled;
}

bool QFcitxPlatformInputContext::shouldDisableInputMethod() const {
    return !inputMethodAccepted() && !objectAcceptsInputMethod();
}

void QFcitxPlatformInputContext::cleanUp() {
    icMap_.clear();

    if (!destroy_) {
        commitPreedit();
    }
}

bool QFcitxPlatformInputContext::isValid() const { return true; }

void QFcitxPlatformInputContext::invokeAction(QInputMethod::Action imAction,
                                              int cursorPosition) {
    unsigned int action;
    if (imAction == QInputMethod::Click) {
        action = 0;
    } else if (imAction == QInputMethod::ContextMenu) {
        action = 1;
    } else {
        return;
    }
    if (auto *proxy = validIC(); proxy->supportInvokeAction()) {
        if (cursorPosition >= 0 && cursorPosition <= preedit_.length()) {
            auto ucs4Cursor = preedit_.left(cursorPosition).toUcs4().length();
            proxy->invokeAction(action, ucs4Cursor);
        }
    } else {
        if (cursorPosition <= 0 || cursorPosition >= preedit_.length()) {
            // qDebug() << action << cursorPosition;
            reset();
        }
    }
}

bool QFcitxPlatformInputContext::commitPreedit(QPointer<QObject> input) {
    if (!input) {
        return false;
    }
    if (preeditList_.isEmpty()) {
        return false;
    }
    QInputMethodEvent e;
    if (!commitPreedit_.isEmpty()) {
        e.setCommitString(commitPreedit_);
    }
    commitPreedit_.clear();
    preeditList_.clear();
    QCoreApplication::sendEvent(input, &e);
    return true;
}

bool checkUtf8(const QByteArray &byteArray) {
    QString s = QString::fromUtf8(byteArray);
    return !s.contains(QChar::ReplacementCharacter);
}

void QFcitxPlatformInputContext::reset() {
    commitPreedit();
    if (auto *proxy = validIC()) {
        proxy->reset();
    }
    if (xkbComposeState_) {
        xkb_compose_state_reset(xkbComposeState_.data());
    }
    QPlatformInputContext::reset();
}

void QFcitxPlatformInputContext::update(Qt::InputMethodQueries queries) {
    QWindow *window = focusWindowWrapper();
    auto *proxy = validICByWindow(window);
    if (!proxy) {
        return;
    }

    FcitxQtICData &data = *static_cast<FcitxQtICData *>(
        proxy->property("icData").value<void *>());

    QObject *input = focusObjectWrapper();
    if (!input) {
        return;
    }

    QInputMethodQueryEvent query(queries);
    QGuiApplication::sendEvent(input, &query);

    if (queries & Qt::ImCursorRectangle) {
        cursorRectChanged();
    }

    if (queries & Qt::ImEnabled) {
        if (shouldDisableInputMethod()) {
            addCapability(data, FcitxCapabilityFlag_Disable);
        } else {
            removeCapability(data, FcitxCapabilityFlag_Disable);
        }
    }

    if (queries & Qt::ImHints) {
        Qt::InputMethodHints hints =
            Qt::InputMethodHints(query.value(Qt::ImHints).toUInt());

#define CHECK_HINTS(_HINTS, _CAPABILITY)                                       \
    if (hints & (_HINTS))                                                      \
        addCapability(data, FcitxCapabilityFlag_##_CAPABILITY);                \
    else                                                                       \
        removeCapability(data, FcitxCapabilityFlag_##_CAPABILITY);

        CHECK_HINTS(Qt::ImhHiddenText, Password)
        CHECK_HINTS(Qt::ImhSensitiveData, Sensitive)
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
        CHECK_HINTS(Qt::ImhPreferLatin, Alpha)
        CHECK_HINTS(Qt::ImhUrlCharactersOnly, Url)
        CHECK_HINTS(Qt::ImhMultiLine, Multiline)
    }

    bool setSurrounding = false;
    do {
        if (!useSurroundingText_) {
            break;
        }
        if (!(queries & (Qt::ImSurroundingText | Qt::ImCursorPosition |
                         Qt::ImAnchorPosition))) {
            break;
        }
        if ((data.capability & FcitxCapabilityFlag_Password) ||
            (data.capability & FcitxCapabilityFlag_Sensitive)) {
            break;
        }
        QVariant var = query.value(Qt::ImSurroundingText);
        QVariant var1 = query.value(Qt::ImCursorPosition);
        QVariant var2 = query.value(Qt::ImAnchorPosition);
        if (!var.isValid() || !var1.isValid()) {
            break;
        }
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
                QVector<unsigned int> tempUCS4 = text.left(cursor).toUcs4();
                cursor = tempUCS4.size();
                tempUCS4 = text.left(anchor).toUcs4();
                anchor = tempUCS4.size();
                if (data.surroundingText != text) {
                    data.surroundingText = text;
                    proxy->setSurroundingText(text, cursor, anchor);
                } else {
                    if (data.surroundingAnchor != anchor ||
                        data.surroundingCursor != cursor) {
                        proxy->setSurroundingTextPosition(cursor, anchor);
                    }
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
    } while (false);
}

void QFcitxPlatformInputContext::commit() {
    auto *proxy = validICByWindow(lastWindow_);
    commitPreedit(lastObject_);
    if (proxy) {
        proxy->reset();
        FcitxQtICData &data = *static_cast<FcitxQtICData *>(
            proxy->property("icData").value<void *>());
        data.resetCandidateWindow();
    }
}

void QFcitxPlatformInputContext::setFocusObject(QObject *object) {
    Q_UNUSED(object);

    // Since we have a wrapper, it's possible that real focus object is not
    // changed. Do not emit focusOut and focusIn if:
    // realFocusObject does not change.
    QObject *realFocusObject = focusObjectWrapper();
    if (lastObject_ == realFocusObject) {
        return;
    }

    auto *proxy = validICByWindow(lastWindow_);
    commitPreedit(lastObject_);
    if (proxy) {
        proxy->focusOut();
        FcitxQtICData &data = *static_cast<FcitxQtICData *>(
            proxy->property("icData").value<void *>());
        data.resetCandidateWindow();
    }

    QWindow *window = focusWindowWrapper();
    lastWindow_ = window;
    lastObject_ = realFocusObject;
    // Always create IC Data for window.
    if (window) {
        proxy = validICByWindow(window);
        if (!proxy) {
            createICData(window);
        }
    }
    if (!window) {
        lastWindow_ = nullptr;
        lastObject_ = nullptr;
        return;
    }

    if (proxy) {
        proxy->focusIn();
        // We need to delegate this otherwise it may cause self-recursion in
        // certain application like libreoffice.
        QTimer::singleShot(0, this,
                           [this, window = QPointer<QWindow>(lastWindow_)]() {
                               if (window != lastWindow_) {
                                   return;
                               }
                               update(Qt::ImHints | Qt::ImEnabled);
                               updateCursorRect();
                           });
    }

    updateInputPanelVisible();
}

void QFcitxPlatformInputContext::updateCursorRect() {
    if (validICByWindow(lastWindow_.data())) {
        cursorRectChanged();
    }
}

void QFcitxPlatformInputContext::windowDestroyed(QObject *object) {
    /* access QWindow is not possible here, so we use our own map to do so */
    icMap_.erase(static_cast<QWindow *>(object));
}

void QFcitxPlatformInputContext::cursorRectChanged() {
    QWindow *inputWindow = focusWindowWrapper();
    if (!inputWindow) {
        return;
    }
    auto *proxy = validICByWindow(inputWindow);
    if (!proxy) {
        return;
    }

    FcitxQtICData &data = *static_cast<FcitxQtICData *>(
        proxy->property("icData").value<void *>());

    QRect r = cursorRectangleWrapper();
    if (!r.isValid()) {
        return;
    }

    // not sure if this is necessary but anyway, qt's screen used to be buggy.
    if (!inputWindow->screen()) {
        return;
    }

    qreal scale = inputWindow->devicePixelRatio();
    if (data.capability & FcitxCapabilityFlag_RelativeRect) {
        auto margins = inputWindow->frameMargins();
        r.translate(margins.left(), margins.top());
        r = QRect(r.topLeft() * scale, r.size() * scale);
        if (data.rect != r) {
            data.rect = r;
            proxy->setCursorRectV2(r.x(), r.y(), r.width(), r.height(), scale);
        }
        return;
    }
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

void QFcitxPlatformInputContext::createInputContextFinished(
    const QByteArray &uuid) {
    auto *proxy = qobject_cast<HybridInputContext *>(sender());
    if (!proxy) {
        return;
    }
    FcitxQtICData *data =
        static_cast<FcitxQtICData *>(proxy->property("icData").value<void *>());
    auto *w = data->window();
    data->rect = QRect();

    if (proxy->isValid() && !uuid.isEmpty()) {
        QWindow *window = focusWindowWrapper();
#ifdef ENABLE_X11
        setFocusGroupForX11(uuid);
#else
        Q_UNUSED(uuid);
#endif
        if (window && window == w) {
            cursorRectChanged();
            proxy->focusIn();
        }
        updateInputPanelVisible();
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
    if (useSurroundingText_) {
        flag |= FcitxCapabilityFlag_SurroundingText;
    }

    if (QGuiApplication::platformName().startsWith("wayland")) {
        flag |= FcitxCapabilityFlag_RelativeRect;
    }
    flag |= FcitxCapabilityFlag_ClientSideInputPanel;

    if (shouldDisableInputMethod()) {
        flag |= FcitxCapabilityFlag_Disable;
    }

    // Notify fcitx of the effective bits from 0bit to 40bit
    // (FcitxCapabilityFlag_Disable)
    data->proxy->setSupportedCapability(0x1ffffffffffULL);

    addCapability(*data, flag, true);
}

void QFcitxPlatformInputContext::updateCapability(const FcitxQtICData &data) {
    if (!data.proxy || !data.proxy->isValid()) {
        return;
    }
    data.proxy->setCapability(data.capability);
}

void QFcitxPlatformInputContext::commitString(const QString &str) {
    cursorPos_ = 0;
    preeditList_.clear();
    commitPreedit_.clear();
    QObject *input = qGuiApp->focusObject();
    if (!input) {
        return;
    }

    QInputMethodEvent event;
    event.setCommitString(str);
    QCoreApplication::sendEvent(input, &event);
}

void QFcitxPlatformInputContext::updateFormattedPreedit(
    const FcitxQtFormattedPreeditList &preeditList, int cursorPos) {
    QObject *input = qGuiApp->focusObject();
    if (!input) {
        return;
    }
    if (cursorPos == cursorPos_ && preeditList == preeditList_) {
        return;
    }
    preeditList_ = preeditList;
    cursorPos_ = cursorPos;
    QString str;
    QString commitStr;
    int pos = 0;
    QList<QInputMethodEvent::Attribute> attrList;
    Q_FOREACH (const FcitxQtFormattedPreedit &preedit, preeditList) {
        str += preedit.string();
        if (!(preedit.format() & FcitxTextFormatFlag_DontCommit)) {
            commitStr += preedit.string();
        }
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
    preedit_ = str;
    commitPreedit_ = commitStr;
    QInputMethodEvent event(str, attrList);
    QCoreApplication::sendEvent(input, &event);
    update(Qt::ImCursorRectangle);
}

void QFcitxPlatformInputContext::updateClientSideUI(
    const FcitxQtFormattedPreeditList &preedit, int cursorpos,
    const FcitxQtFormattedPreeditList &auxUp,
    const FcitxQtFormattedPreeditList &auxDown,
    const FcitxQtStringKeyValueList &candidates, int candidateIndex,
    int layoutHint, bool hasPrev, bool hasNext) {
    QObject *input = qGuiApp->focusObject();
    if (!input) {
        return;
    }

    auto *proxy = qobject_cast<HybridInputContext *>(sender());
    if (!proxy) {
        return;
    }
    FcitxQtICData *data =
        static_cast<FcitxQtICData *>(proxy->property("icData").value<void *>());
    auto *w = data->window();
    auto *window = focusWindowWrapper();
    if (window && w == window) {
        data->candidateWindow()->updateClientSideUI(
            preedit, cursorpos, auxUp, auxDown, candidates, candidateIndex,
            layoutHint, hasPrev, hasNext);
    }
}

void QFcitxPlatformInputContext::deleteSurroundingText(int offset,
                                                       unsigned int _nchar) {
    QObject *input = qGuiApp->focusObject();
    if (!input) {
        return;
    }

    QInputMethodEvent event;

    auto *proxy = qobject_cast<HybridInputContext *>(sender());
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

        int start;
        int len;
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

void QFcitxPlatformInputContext::forwardKey(unsigned int keyval,
                                            unsigned int state, bool type) {
    auto *proxy = qobject_cast<HybridInputContext *>(sender());
    if (!proxy) {
        return;
    }
    FcitxQtICData &data = *static_cast<FcitxQtICData *>(
        proxy->property("icData").value<void *>());
    auto *w = data.window();
    QObject *input = qGuiApp->focusObject();
    auto *window = focusWindowWrapper();
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
    if (locale_ != newLocale) {
        locale_ = newLocale;
        emitLocaleChanged();
    }
}

void QFcitxPlatformInputContext::serverSideFocusOut() {
    if (lastObject_ == focusObjectWrapper()) {
        commitPreedit();
    }
}

QLocale QFcitxPlatformInputContext::locale() const { return locale_; }

bool QFcitxPlatformInputContext::hasCapability(
    Capability /*capability*/) const {
    return true;
}

void QFcitxPlatformInputContext::showInputPanel() {
    auto *proxy = validIC();
    if (proxy == nullptr) {
        return;
    }
    proxy->showVirtualKeyboard();
}

void QFcitxPlatformInputContext::hideInputPanel() {
    auto *proxy = validIC();
    if (proxy == nullptr) {
        return;
    }
    proxy->hideVirtualKeyboard();
}

bool QFcitxPlatformInputContext::isInputPanelVisible() const {
    return inputPanelVisible_;
}

void QFcitxPlatformInputContext::updateInputPanelVisible() {
    // We have to use two levels of cache here, one level is from
    // DBus to proxy, another level is from proxy to the one read by Qt.
    //
    // Because the API is designed in a way that "input panel" visibility is a
    // per-input context thing. We have to update the value when focus is
    // changed.
    bool oldVisible = inputPanelVisible_;

    bool newVisible = false;
    if (auto *proxy = validIC()) {
        newVisible = proxy->isVirtualKeyboardVisible();
    }
    if (newVisible != oldVisible) {
        inputPanelVisible_ = newVisible;
        emitInputPanelVisibleChanged();
    }
}

void QFcitxPlatformInputContext::createICData(QWindow *w) {
    auto iter = icMap_.find(w);
    if (iter == icMap_.end()) {
        auto result =
            icMap_.emplace(std::piecewise_construct, std::forward_as_tuple(w),
                           std::forward_as_tuple(this, w));
        connect(w, &QObject::destroyed, this,
                &QFcitxPlatformInputContext::windowDestroyed);
        iter = result.first;
        auto &data = iter->second;

        if (QGuiApplication::platformName() == QLatin1String("xcb")) {
            data.proxy->setDisplay("x11:");
        } else if (QGuiApplication::platformName().startsWith("wayland")) {
            data.proxy->setDisplay("wayland:");
        }
        connect(data.proxy, &HybridInputContext::inputContextCreated, this,
                &QFcitxPlatformInputContext::createInputContextFinished);
        connect(data.proxy, &HybridInputContext::commitString, this,
                &QFcitxPlatformInputContext::commitString);
        connect(data.proxy, &HybridInputContext::forwardKey, this,
                &QFcitxPlatformInputContext::forwardKey);
        connect(data.proxy, &HybridInputContext::updateFormattedPreedit, this,
                &QFcitxPlatformInputContext::updateFormattedPreedit);
        connect(data.proxy, &HybridInputContext::deleteSurroundingText, this,
                &QFcitxPlatformInputContext::deleteSurroundingText);
        connect(data.proxy, &HybridInputContext::currentIM, this,
                &QFcitxPlatformInputContext::updateCurrentIM);
        connect(data.proxy, &HybridInputContext::updateClientSideUI, this,
                &QFcitxPlatformInputContext::updateClientSideUI);
        connect(data.proxy, &HybridInputContext::notifyFocusOut, this,
                &QFcitxPlatformInputContext::serverSideFocusOut);
        connect(data.proxy,
                &HybridInputContext::virtualKeyboardVisibilityChanged, this,
                [this]() {
                    if (validIC() == sender()) {
                        updateInputPanelVisible();
                    }
                });
    }
}

QKeyEvent *QFcitxPlatformInputContext::createKeyEvent(unsigned int keyval,
                                                      unsigned int state,
                                                      bool isRelease,
                                                      const QKeyEvent *event) {
    QKeyEvent *newEvent = nullptr;
    state &= (~(1U << 31));
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
        QPoint globalPos;
        QPoint pos;
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

        const auto *keyEvent = static_cast<const QKeyEvent *>(event);
        quint32 keyval = keyEvent->nativeVirtualKey();
        quint32 keycode = keyEvent->nativeScanCode();
        quint32 state = keyEvent->nativeModifiers();
        bool isRelease = keyEvent->type() == QEvent::KeyRelease;

        if (shouldDisableInputMethod()) {
            break;
        }

        QObject *input = qGuiApp->focusObject();

        if (!input) {
            break;
        }

        auto *proxy = validICByWindow(focusWindowWrapper());

        if (!proxy) {
            if (filterEventFallback(keyval, keycode, state, isRelease)) {
                return true;
            }
            break;
        }

        update(Qt::ImHints | Qt::ImEnabled);
        proxy->focusIn();
        updateInputPanelVisible();

        auto stateToFcitx = state;
        if (keyEvent->isAutoRepeat()) {
            // KeyState::Repeat
            stateToFcitx |= (1U << 31);
        }
        auto reply = proxy->processKeyEvent(keyval, keycode, stateToFcitx,
                                            isRelease, keyEvent->timestamp());

        if (Q_UNLIKELY(syncMode_)) {
            reply.waitForFinished();

            if (reply.isError() ||
                !HybridInputContext::processKeyEventResult(reply)) {
                if (filterEventFallback(keyval, keycode, state, isRelease)) {
                    return true;
                }
                break;
            }
            update(Qt::ImCursorRectangle);
            return true;
        }
        auto *watcher = new ProcessKeyWatcher(*keyEvent, focusWindowWrapper(),
                                              reply, proxy);
        connect(watcher, &QDBusPendingCallWatcher::finished, this,
                &QFcitxPlatformInputContext::processKeyEventFinished);
        return true;

    } while (false);
    return QPlatformInputContext::filterEvent(event);
}

void QFcitxPlatformInputContext::processKeyEventFinished(
    QDBusPendingCallWatcher *w) {
    auto *watcher = static_cast<ProcessKeyWatcher *>(w);
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

    if (watcher->isError() ||
        !HybridInputContext::processKeyEventResult(*watcher)) {
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
        auto *proxy = qobject_cast<HybridInputContext *>(watcher->parent());
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
            data.event = std::make_unique<QKeyEvent>(keyEvent);
#endif
        }
    }

    delete watcher;
}

bool QFcitxPlatformInputContext::filterEventFallback(unsigned int keyval,
                                                     unsigned int keycode,
                                                     unsigned int state,
                                                     bool isRelease) {
    Q_UNUSED(keycode);
    return processCompose(keyval, state, isRelease);
}

HybridInputContext *QFcitxPlatformInputContext::validIC() const {
    if (icMap_.empty()) {
        return nullptr;
    }
    QWindow *window = focusWindowWrapper();
    return validICByWindow(window);
}

HybridInputContext *
QFcitxPlatformInputContext::validICByWindow(QWindow *w) const {
    if (!w) {
        return nullptr;
    }

    if (icMap_.empty()) {
        return nullptr;
    }
    auto iter = icMap_.find(w);
    if (iter == icMap_.end()) {
        return nullptr;
    }
    const auto &data = iter->second;
    if (!data.proxy || !data.proxy->isValid()) {
        return nullptr;
    }
    return data.proxy;
}

bool QFcitxPlatformInputContext::processCompose(unsigned int keyval,
                                                unsigned int state,
                                                bool isRelease) {
    Q_UNUSED(state);

    if (!xkbComposeTable_ || isRelease) {
        return false;
    }

    struct xkb_compose_state *xkbComposeState = xkbComposeState_.data();

    enum xkb_compose_feed_result result =
        xkb_compose_state_feed(xkbComposeState, keyval);
    if (result == XKB_COMPOSE_FEED_IGNORED) {
        return false;
    }

    enum xkb_compose_status status =
        xkb_compose_state_get_status(xkbComposeState);
    if (status == XKB_COMPOSE_NOTHING) {
        return false;
    }
    if (status == XKB_COMPOSE_COMPOSED) {
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

QWindow *QFcitxPlatformInputContext::focusWindowWrapper() const {
    QWindow *focusWindow = qGuiApp->focusWindow();
    do {
        if (!focusWindow) {
            break;
        }
        QObject *realFocusObject = focusObjectWrapper();
        if (qGuiApp->focusObject() == realFocusObject) {
            break;
        }
        auto *widget = qobject_cast<QWidget *>(realFocusObject);
        if (!widget) {
            break;
        }
        auto *window = widget->topLevelWidget()->windowHandle();
        if (!window) {
            break;
        }
        focusWindow = window;
    } while (false);
    return focusWindow;
}

QObject *QFcitxPlatformInputContext::focusObjectWrapper() const {
    return deepestFocusProxy(qGuiApp->focusObject());
}

QRect QFcitxPlatformInputContext::cursorRectangleWrapper() const {
    QObject *object = focusObjectWrapper();
    QRect r;
    if (object && object != qGuiApp->focusObject() && object->isWidgetType()) {
        // Logic is borrowed from QWidgetPrivate::updateWidgetTransform.
        // If focusObject mismatches, the inputItemTransform will also mismatch,
        // so we need to do our own calculation.
        auto *widget = qobject_cast<QWidget *>(object);
        QTransform t;
        const QPoint p = widget->mapTo(widget->topLevelWidget(), QPoint(0, 0));
        t.translate(p.x(), p.y());
        r = widget->inputMethodQuery(Qt::ImCursorRectangle).toRect();
        if (r.isValid()) {
            r = t.mapRect(r);
        }
    } else {
        r = qGuiApp->inputMethod()->cursorRectangle().toRect();
    }
    return r;
}

FcitxTheme *QFcitxPlatformInputContext::theme() {
    if (!theme_) {
        theme_ = new FcitxTheme(this);
    }
    return theme_;
}

} // namespace fcitx
