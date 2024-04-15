/*
 * SPDX-FileCopyrightText: 2013~2020 CSSlayer <wengxt@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; see the file COPYING. If not,
 * see <http://www.gnu.org/licenses/>.
 */

/* this is forked from kdelibs/kdeui/kkeysequencewidget.cpp */

/*
    Original Copyright header
    SPDX-FileCopyrightText: 1998 Mark Donohoe <donohoe@kde.org>
    SPDX-FileCopyrightText: 2001 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "fcitxqtkeysequencewidget.h"
#include "fcitxqtkeysequencewidget_p.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QHash>
#include <QKeyEvent>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QTimer>
#include <QToolButton>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/key.h>

Q_LOGGING_CATEGORY(fcitx5qtKeysequenceWidget, "fcitx5.qt.keysequencewidget")

namespace fcitx {

namespace {

bool isX11LikePlatform() {
    return QGuiApplication::platformName() == "xcb" ||
           QGuiApplication::platformName().startsWith("wayland");
}

} // namespace

class FcitxQtKeySequenceWidgetPrivate {
public:
    FcitxQtKeySequenceWidgetPrivate(FcitxQtKeySequenceWidget *q);

    void init();

    static bool isOkWhenModifierless(int keyQt);

    void updateShortcutDisplay();
    void startRecording();

    void controlModifierlessTimout() {
        if (keySequence_.size() != 0 && !modifierKeys_) {
            // No modifier key pressed currently. Start the timout
            modifierlessTimeout_.start(600);
        } else {
            // A modifier is pressed. Stop the timeout
            modifierlessTimeout_.stop();
        }
    }

    void cancelRecording() {
        keySequence_ = oldKeySequence_;
        doneRecording();
    }

    // private slot
    void doneRecording();

    // members
    FcitxQtKeySequenceWidget *const q;
    QHBoxLayout *layout_ = nullptr;
    FcitxQtKeySequenceButton *keyButton_ = nullptr;
    QToolButton *clearButton_ = nullptr;
    QAction *keyCodeModeAction_ = nullptr;

    QList<Key> keySequence_;
    QList<Key> oldKeySequence_;
    QTimer modifierlessTimeout_;
    bool allowModifierless_ = false;
    KeyStates modifierKeys_;
    unsigned int qtModifierKeys_ = 0;
    bool isRecording_ = false;
    bool multiKeyShortcutsAllowed_ = false;
    bool allowModifierOnly_ = false;
    bool modifierAllowed_ = true;
    bool keycodeAllowed_ = true;
};

FcitxQtKeySequenceWidgetPrivate::FcitxQtKeySequenceWidgetPrivate(
    FcitxQtKeySequenceWidget *q)
    : q(q) {}

FcitxQtKeySequenceWidget::FcitxQtKeySequenceWidget(QWidget *parent)
    : QWidget(parent), d(new FcitxQtKeySequenceWidgetPrivate(this)) {
    d->init();
    setFocusProxy(d->keyButton_);
    connect(d->keyButton_, &QPushButton::clicked, this,
            &FcitxQtKeySequenceWidget::captureKeySequence);
    connect(d->clearButton_, &QPushButton::clicked, this,
            &FcitxQtKeySequenceWidget::clearKeySequence);
    connect(&d->modifierlessTimeout_, &QTimer::timeout, this,
            [this]() { d->doneRecording(); });
    d->updateShortcutDisplay();
}

void FcitxQtKeySequenceWidgetPrivate::init() {
    layout_ = new QHBoxLayout(q);
    layout_->setContentsMargins(0, 0, 0, 0);

    keyButton_ = new FcitxQtKeySequenceButton(this, q);
    keyButton_->setFocusPolicy(Qt::StrongFocus);
    keyButton_->setIcon(QIcon::fromTheme("configure"));
    layout_->addWidget(keyButton_);

    clearButton_ = new QToolButton(q);
    layout_->addWidget(clearButton_);

    keyCodeModeAction_ = new QAction(_("Key code mode"));
    keyCodeModeAction_->setCheckable(true);
    keyCodeModeAction_->setEnabled(isX11LikePlatform());
    q->setContextMenuPolicy(Qt::ActionsContextMenu);
    q->addAction(keyCodeModeAction_);

    if (qApp->isLeftToRight())
        clearButton_->setIcon(QIcon::fromTheme("edit-clear-locationbar-rtl"));
    else
        clearButton_->setIcon(QIcon::fromTheme("edit-clear-locationbar-ltr"));

    q->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
}

FcitxQtKeySequenceWidget::~FcitxQtKeySequenceWidget() { delete d; }

bool FcitxQtKeySequenceWidget::multiKeyShortcutsAllowed() const {
    return d->multiKeyShortcutsAllowed_;
}

void FcitxQtKeySequenceWidget::setMultiKeyShortcutsAllowed(bool allowed) {
    d->multiKeyShortcutsAllowed_ = allowed;
}

bool FcitxQtKeySequenceWidget::isModifierAllowed() const {
    return d->modifierAllowed_;
}

void FcitxQtKeySequenceWidget::setModifierAllowed(bool allowed) {
    d->modifierAllowed_ = allowed;
}

bool FcitxQtKeySequenceWidget::isKeycodeAllowed() const {
    return d->keycodeAllowed_;
}

void FcitxQtKeySequenceWidget::setKeycodeAllowed(bool allowed) {
    if (d->keycodeAllowed_ == allowed) {
        return;
    }
    d->keycodeAllowed_ = allowed;
    if (allowed) {
        d->keyCodeModeAction_->setChecked(false);
        addAction(d->keyCodeModeAction_);
    } else {
        removeAction(d->keyCodeModeAction_);
    }
}

void FcitxQtKeySequenceWidget::setModifierlessAllowed(bool allow) {
    d->allowModifierless_ = allow;
}

bool FcitxQtKeySequenceWidget::isModifierlessAllowed() {
    return d->allowModifierless_;
}

bool FcitxQtKeySequenceWidget::isModifierOnlyAllowed() {
    return d->allowModifierOnly_;
}

bool FcitxQtKeySequenceWidget::isModifierlessAllowed() const {
    return d->allowModifierless_;
}

bool FcitxQtKeySequenceWidget::isModifierOnlyAllowed() const {
    return d->allowModifierOnly_;
}

void FcitxQtKeySequenceWidget::setModifierOnlyAllowed(bool allow) {
    d->allowModifierOnly_ = allow;
}

void FcitxQtKeySequenceWidget::setClearButtonShown(bool show) {
    d->clearButton_->setVisible(show);
}

bool FcitxQtKeySequenceWidget::isClearButtonVisible() const {
    return d->clearButton_->isVisible();
}

// slot
void FcitxQtKeySequenceWidget::captureKeySequence() { d->startRecording(); }

const QList<Key> &FcitxQtKeySequenceWidget::keySequence() const {
    return d->keySequence_;
}

// slot
void FcitxQtKeySequenceWidget::setKeySequence(const QList<Key> &seq) {
    // oldKeySequence holds the key sequence before recording started, if
    // setKeySequence()
    // is called while not recording then set oldKeySequence to the existing
    // sequence so
    // that the keySequenceChanged() signal is emitted if the new and previous
    // key
    // sequences are different
    if (!d->isRecording_) {
        d->oldKeySequence_ = d->keySequence_;
    }

    d->keySequence_ = QList<Key>();
    for (auto key : seq) {
        d->keySequence_ << key;
    }
    d->doneRecording();
}

// slot
void FcitxQtKeySequenceWidget::clearKeySequence() {
    setKeySequence(QList<Key>());
}

void FcitxQtKeySequenceWidgetPrivate::startRecording() {
    modifierKeys_ = 0;
    oldKeySequence_ = keySequence_;
    keySequence_ = QList<Key>();
    isRecording_ = true;
    keyButton_->grabKeyboard();

    if (!QWidget::keyboardGrabber()) {
        qWarning() << "Failed to grab the keyboard! Most likely qt's nograb "
                      "option is active";
    }

    keyButton_->setDown(true);
    updateShortcutDisplay();
}

void FcitxQtKeySequenceWidgetPrivate::doneRecording() {
    modifierlessTimeout_.stop();
    isRecording_ = false;
    keyButton_->releaseKeyboard();
    keyButton_->setDown(false);

    if (keySequence_ == oldKeySequence_ && !allowModifierOnly_) {
        // The sequence hasn't changed
        updateShortcutDisplay();
        return;
    }

    Q_EMIT q->keySequenceChanged(keySequence_);

    updateShortcutDisplay();
}

void FcitxQtKeySequenceWidgetPrivate::updateShortcutDisplay() {
    QString s = QString::fromUtf8(
        Key::keyListToString(keySequence_, KeyStringFormat::Localized).c_str());
    s.replace('&', QLatin1String("&&"));

    if (isRecording_) {
        if (modifierKeys_) {
            if (!s.isEmpty())
                s.append(",");
            if (modifierKeys_ & KeyState::Super)
                s += "Super+";
            if (modifierKeys_ & KeyState::Ctrl)
                s += "Control+";
            if (modifierKeys_ & KeyState::Alt)
                s += "Alt+";
            if (modifierKeys_ & KeyState::Shift)
                s += "Shift+";
            if (modifierKeys_ & KeyState::Hyper)
                s += "Hyper+";

        } else if (keySequence_.size() == 0) {
            s = "...";
        }
        // make it clear that input is still going on
        s.append(" ...");
    }

    if (s.isEmpty()) {
        s = _("Empty");
    }

    s.prepend(' ');
    s.append(' ');
    keyButton_->setText(s);
}

FcitxQtKeySequenceButton::~FcitxQtKeySequenceButton() {}

// prevent Qt from special casing Tab and Backtab
bool FcitxQtKeySequenceButton::event(QEvent *e) {
    if (d->isRecording_ && e->type() == QEvent::KeyPress) {
        keyPressEvent(static_cast<QKeyEvent *>(e));
        return true;
    }

    // The shortcut 'alt+c' ( or any other dialog local action shortcut )
    // ended the recording and triggered the action associated with the
    // action. In case of 'alt+c' ending the dialog.  It seems that those
    // ShortcutOverride events get sent even if grabKeyboard() is active.
    if (d->isRecording_ && e->type() == QEvent::ShortcutOverride) {
        e->accept();
        return true;
    }

    return QPushButton::event(e);
}

void FcitxQtKeySequenceButton::keyPressEvent(QKeyEvent *e) {
    int keyQt = e->key();
    if (keyQt == -1) {
        // Qt sometimes returns garbage keycodes, I observed -1, if it doesn't
        // know a key. We cannot do anything useful with those (several keys
        // have -1, indistinguishable) and QKeySequence.toString() will also
        // yield a garbage string.
        QMessageBox::warning(
            this, _("The key you just pressed is not supported by Qt."),
            _("Unsupported Key"));
        return d->cancelRecording();
    }

    // Same as Key::normalize();
    unsigned int newQtModifiers =
        e->modifiers() & (Qt::META | Qt::ALT | Qt::CTRL | Qt::SHIFT);
    KeyStates newModifiers;
    if (isX11LikePlatform()) {
        newModifiers = KeyStates(e->nativeModifiers()) &
                       KeyStates{KeyState::Ctrl_Alt_Shift, KeyState::Hyper,
                                 KeyState::Super};
        newModifiers |=
            Key::keySymToStates(static_cast<KeySym>(e->nativeVirtualKey()));
    } else {
        if (newQtModifiers & Qt::META) {
            newModifiers |= KeyState::Super;
        }
        if (newQtModifiers & Qt::ALT) {
            newModifiers |= KeyState::Alt;
        }
        if (newQtModifiers & Qt::CTRL) {
            newModifiers |= KeyState::Ctrl;
        }
        if (newQtModifiers & Qt::SHIFT) {
            newModifiers |= KeyState::Shift;
        }
    }

    // don't have the return or space key appear as first key of the sequence
    // when they
    // were pressed to start editing - catch and them and imitate their effect
    if (!d->isRecording_ &&
        ((keyQt == Qt::Key_Return || keyQt == Qt::Key_Space))) {
        d->startRecording();
        d->modifierKeys_ = newModifiers;
        d->qtModifierKeys_ = newQtModifiers;
        d->updateShortcutDisplay();
        return;
    }

    // We get events even if recording isn't active.
    if (!d->isRecording_)
        return QPushButton::keyPressEvent(e);

    e->accept();
    d->modifierKeys_ = newModifiers;
    d->qtModifierKeys_ = newQtModifiers;

    switch (keyQt) {
    case Qt::Key_AltGr: // or else we get unicode salad
        return;
    case Qt::Key_Shift:
    case Qt::Key_Control:
    case Qt::Key_Alt:
    case Qt::Key_Super_L:
    case Qt::Key_Super_R:
    case Qt::Key_Hyper_L:
    case Qt::Key_Hyper_R:
    case Qt::Key_Meta:
    case Qt::Key_Menu: // unused (yes, but why?)
        d->controlModifierlessTimout();
        d->updateShortcutDisplay();
        break;
    default:
        // We now have a valid key press.
        if (keyQt) {
            if ((keyQt == Qt::Key_Backtab) &&
                d->modifierKeys_.test(KeyState::Shift)) {
                keyQt = Qt::Key_Tab | d->qtModifierKeys_;
            } else {
                keyQt |= d->qtModifierKeys_;
            }

            Key key;
            if (d->keyCodeModeAction_->isChecked()) {
                key = Key::fromKeyCode(e->nativeScanCode(), d->modifierKeys_);
            } else {
                if (isX11LikePlatform()) {
                    key = Key(static_cast<KeySym>(e->nativeVirtualKey()),
                              KeyStates(e->nativeModifiers()))
                              .normalize();
                } else {
                    qCWarning(fcitx5qtKeysequenceWidget)
                        << "Unsupported platform";
                }
            }

            // Check the first key.
            if (d->keySequence_.size() == 0) {
                if (!d->allowModifierless_ && key.states() == 0) {
                    return;
                }
                if (!d->modifierAllowed_ && key.states() != 0) {
                    return;
                }
            }

            if (key.isValid()) {
                d->keySequence_ << key;
            }

            if ((!d->multiKeyShortcutsAllowed_) ||
                (d->keySequence_.size() >= 4)) {
                d->doneRecording();
                return;
            }
            d->controlModifierlessTimout();
            d->updateShortcutDisplay();
        }
    }
}

void FcitxQtKeySequenceButton::keyReleaseEvent(QKeyEvent *e) {
    if (e->key() == -1) {
        // ignore garbage, see keyPressEvent()
        return;
    }

    if (!d->isRecording_)
        return QPushButton::keyReleaseEvent(e);

    e->accept();

    if (!d->multiKeyShortcutsAllowed_ && d->allowModifierOnly_ &&
        (e->key() == Qt::Key_Shift || e->key() == Qt::Key_Control ||
         e->key() == Qt::Key_Meta || e->key() == Qt::Key_Alt)) {

        Key key;
        if (isX11LikePlatform()) {
            if (d->keyCodeModeAction_->isChecked()) {
                key = Key::fromKeyCode(e->nativeScanCode(), key.states());
            } else {
                key = Key(static_cast<KeySym>(e->nativeVirtualKey()),
                          KeyStates(e->nativeModifiers()))
                          .normalize();
            }
            d->keySequence_ = QList<Key>({key});
        }
        d->doneRecording();
        return;
    }

    unsigned int newQtModifiers =
        e->modifiers() & (Qt::META | Qt::ALT | Qt::CTRL | Qt::SHIFT);
    KeyStates newModifiers;
    if (isX11LikePlatform()) {
        newModifiers = KeyStates(e->nativeModifiers()) &
                       KeyStates{KeyState::Ctrl_Alt_Shift, KeyState::Hyper,
                                 KeyState::Super};
        newModifiers &=
            ~Key::keySymToStates(static_cast<KeySym>(e->nativeVirtualKey()));
    } else {
        if (newQtModifiers & Qt::META) {
            newModifiers |= KeyState::Super;
        }
        if (newQtModifiers & Qt::ALT) {
            newModifiers |= KeyState::Alt;
        }
        if (newQtModifiers & Qt::CTRL) {
            newModifiers |= KeyState::Ctrl;
        }
        if (newQtModifiers & Qt::SHIFT) {
            newModifiers |= KeyState::Shift;
        }
    }

    // if a modifier that belongs to the shortcut was released...
    if ((newModifiers & d->modifierKeys_) < d->modifierKeys_) {
        d->modifierKeys_ = newModifiers;
        d->controlModifierlessTimout();
        d->updateShortcutDisplay();
    }
}

// static
bool FcitxQtKeySequenceWidgetPrivate::isOkWhenModifierless(int keyQt) {
    // this whole function is a hack, but especially the first line of code
    if (QKeySequence(keyQt).toString().length() == 1)
        return false;

    switch (keyQt) {
    case Qt::Key_Return:
    case Qt::Key_Space:
    case Qt::Key_Tab:
    case Qt::Key_Backtab: // does this ever happen?
    case Qt::Key_Backspace:
    case Qt::Key_Delete:
        return false;
    default:
        return true;
    }
}
} // namespace fcitx

#include "moc_fcitxqtkeysequencewidget.cpp"
