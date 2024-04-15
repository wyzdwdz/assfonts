/*
 * SPDX-FileCopyrightText: 2012~2012 CSSlayer <wengxt@gmail.com>
 * SPDX-FileCopyrightText: 2017~2017 xzhao
 *   i@xuzhao.net
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef FCITX5QT_GUIWRAPPER_MAINWINDOW_H
#define FCITX5QT_GUIWRAPPER_MAINWINDOW_H

#include <QDialog>

#include "fcitxqtconfiguiwidget.h"
#include "ui_mainwindow.h"
#include <QDBusPendingCallWatcher>

namespace fcitx {

class FcitxQtControllerProxy;
class FcitxQtWatcher;
class MainWindow : public QDialog, public Ui::MainWindow {
    Q_OBJECT
public:
    explicit MainWindow(const QString &path,
                        FcitxQtConfigUIWidget *pluginWidget,
                        QWidget *parent = 0);

    void setParentWindow(WId id);
public Q_SLOTS:
    void changed(bool changed);
    void clicked(QAbstractButton *button);
    void availabilityChanged(bool avail);
    void saveSubConfig(const QString &path);

protected:
    void showEvent(QShowEvent *event) override;

private Q_SLOTS:
    void saveFinished();
    void saveFinishedPhase2(QDBusPendingCallWatcher *watcher);

private:
    QString path_;
    FcitxQtWatcher *watcher_;
    FcitxQtConfigUIWidget *pluginWidget_;
    FcitxQtControllerProxy *proxy_;
    WId wid_ = 0;
    bool closeAfterSave_ = false;
};
} // namespace fcitx

#endif // FCITXQT5_GUIWRAPPER_MAINWINDOW_H
