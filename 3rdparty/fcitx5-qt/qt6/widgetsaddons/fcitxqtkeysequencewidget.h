/*
 * SPDX-FileCopyrightText: 2013~2017 CSSlayer <wengxt@gmail.com>
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
#ifndef _WIDGETSADDONS_FCITXQTKEYSEQUENCEWIDGET_H_
#define _WIDGETSADDONS_FCITXQTKEYSEQUENCEWIDGET_H_

/* this is forked from kdelibs/kdeui/kkeysequencewidget.h */

/*
    Original Copyright header
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2001, 2002 Ellis Whitehead <ellis@kde.org>
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

#include <QList>
#include <QPushButton>
#include <fcitx-utils/key.h>

#include "fcitx5qt6widgetsaddons_export.h"

namespace fcitx {

class FcitxQtKeySequenceWidgetPrivate;

class FCITX5QT6WIDGETSADDONS_EXPORT FcitxQtKeySequenceWidget : public QWidget {
    Q_OBJECT

    Q_PROPERTY(bool multiKeyShortcutsAllowed READ multiKeyShortcutsAllowed WRITE
                   setMultiKeyShortcutsAllowed)

    Q_PROPERTY(bool modifierlessAllowed READ isModifierlessAllowed WRITE
                   setModifierlessAllowed)

    Q_PROPERTY(
        bool modifierAllowed READ isModifierAllowed WRITE setModifierAllowed)

    Q_PROPERTY(
        bool keycodeAllowed READ isKeycodeAllowed WRITE setKeycodeAllowed)

    Q_PROPERTY(bool modifierOnlyAllowed READ isModifierOnlyAllowed WRITE
                   setModifierOnlyAllowed)

public:
    /**
     * Constructor.
     */
    explicit FcitxQtKeySequenceWidget(QWidget *parent = 0);

    /**
     * Destructs the widget.
     */
    virtual ~FcitxQtKeySequenceWidget();

    /**
     * @brief Set whether allow multiple shortcuts.
     *
     * @param  allow
     */
    void setMultiKeyShortcutsAllowed(bool allow);
    bool multiKeyShortcutsAllowed() const;

    /**
     * @brief Set whether allow modifier less that produce text, such as just
     * key A.
     *
     * @param allow
     */
    void setModifierlessAllowed(bool allow);
    // FIXME: remove this
    bool isModifierlessAllowed();
    bool isModifierlessAllowed() const;

    /**
     * @brief Set whether allow key that has modifier.
     *
     * @param allow
     * @since 5.0.12
     */
    void setModifierAllowed(bool allow);
    bool isModifierAllowed() const;

    /**
     * @brief Set whether allow key to use key code.
     *
     * @param allow
     * @since 5.0.12
     */
    void setKeycodeAllowed(bool allow);
    bool isKeycodeAllowed() const;

    /**
     * @brief Set whether allow modifier only key, such as only left control.
     *
     * @param allow allow modifier only key to be captured.
     */
    void setModifierOnlyAllowed(bool allow);
    // FIXME: remove this
    bool isModifierOnlyAllowed();
    bool isModifierOnlyAllowed() const;

    void setClearButtonShown(bool show);
    bool isClearButtonVisible() const;

    const QList<Key> &keySequence() const;

Q_SIGNALS:
    void keySequenceChanged(const QList<Key> &seq);

public Q_SLOTS:
    void captureKeySequence();
    void setKeySequence(const QList<Key> &seq);
    void clearKeySequence();

private:
    friend class FcitxQtKeySequenceWidgetPrivate;
    FcitxQtKeySequenceWidgetPrivate *const d;

    Q_DISABLE_COPY(FcitxQtKeySequenceWidget)
};
} // namespace fcitx

#endif // _WIDGETSADDONS_FCITXQTKEYSEQUENCEWIDGET_H_
