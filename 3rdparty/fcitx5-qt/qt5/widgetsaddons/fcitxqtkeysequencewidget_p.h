/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
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
#ifndef _WIDGETSADDONS_FCITXQTKEYSEQUENCEWIDGET_P_H_
#define _WIDGETSADDONS_FCITXQTKEYSEQUENCEWIDGET_P_H_

/* this is forked from kdelibs/kdeui/kkeysequencewidget_p.h */

/*
    Original Copyright header
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

#include <QAction>
#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>

namespace fcitx {

class FcitxQtKeySequenceWidgetPrivate;
class FcitxQtKeySequenceButton : public QPushButton {
    Q_OBJECT

public:
    explicit FcitxQtKeySequenceButton(FcitxQtKeySequenceWidgetPrivate *d,
                                      QWidget *parent)
        : QPushButton(parent), d(d) {}

    virtual ~FcitxQtKeySequenceButton();

protected:
    /**
     * Reimplemented for internal reasons.
     */
    bool event(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    FcitxQtKeySequenceWidgetPrivate *const d;
};
} // namespace fcitx

#endif // _WIDGETSADDONS_FCITXQTKEYSEQUENCEWIDGET_P_H_
