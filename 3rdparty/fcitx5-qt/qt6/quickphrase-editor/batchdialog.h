/*
 * SPDX-FileCopyrightText: 2013~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */
#ifndef _QUICKPHRASE_EDITOR_BATCHDIALOG_H_
#define _QUICKPHRASE_EDITOR_BATCHDIALOG_H_

#include "ui_batchdialog.h"
#include <QDialog>

namespace fcitx {
class BatchDialog : public QDialog, public Ui::BatchDialog {
    Q_OBJECT
public:
    explicit BatchDialog(QWidget *parent = 0);
    virtual ~BatchDialog();

    QString text() const;
    void setText(const QString &s);
};
} // namespace fcitx

#endif // _QUICKPHRASE_EDITOR_BATCHDIALOG_H_
