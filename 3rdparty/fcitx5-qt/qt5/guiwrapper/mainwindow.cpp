/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 * SPDX-FileCopyrightText: 2017~2017 xzhao <i@xuzhao.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "mainwindow.h"
#include "fcitxqtconfiguifactory.h"
#include "fcitxqtcontrollerproxy.h"
#include "fcitxqtwatcher.h"
#include <QDebug>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QWindow>
#include <fcitx-utils/i18n.h>

namespace fcitx {

MainWindow::MainWindow(const QString &path, FcitxQtConfigUIWidget *pluginWidget,
                       QWidget *parent)
    : QDialog(parent), path_(path), watcher_(new FcitxQtWatcher(this)),
      pluginWidget_(pluginWidget), proxy_(0) {
    setupUi(this);
    watcher_->setConnection(QDBusConnection::sessionBus());
    verticalLayout->insertWidget(0, pluginWidget_);
    buttonBox->button(QDialogButtonBox::Ok)->setText(_("&Ok"));
    buttonBox->button(QDialogButtonBox::Apply)->setText(_("&Apply"));
    buttonBox->button(QDialogButtonBox::Reset)->setText(_("&Reset"));
    buttonBox->button(QDialogButtonBox::Close)->setText(_("&Close"));
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
    buttonBox->button(QDialogButtonBox::Reset)->setEnabled(false);
    setWindowIcon(QIcon::fromTheme(pluginWidget_->icon()));
    setWindowTitle(pluginWidget_->title());

    connect(pluginWidget_, &FcitxQtConfigUIWidget::changed, this,
            &MainWindow::changed);
    if (pluginWidget_->asyncSave()) {
        connect(pluginWidget_, &FcitxQtConfigUIWidget::saveFinished, this,
                &MainWindow::saveFinished);
    }
    connect(pluginWidget_, &FcitxQtConfigUIWidget::saveSubConfig, this,
            &MainWindow::saveSubConfig);
    connect(buttonBox, &QDialogButtonBox::clicked, this, &MainWindow::clicked);
    connect(watcher_, &FcitxQtWatcher::availabilityChanged, this,
            &MainWindow::availabilityChanged);

    watcher_->watch();
}

void MainWindow::availabilityChanged(bool avail) {
    if (!avail) {
        return;
    }
    if (proxy_) {
        delete proxy_;
    }
    proxy_ = new FcitxQtControllerProxy(watcher_->serviceName(),
                                        QLatin1String("/controller"),
                                        watcher_->connection(), this);
}

void MainWindow::clicked(QAbstractButton *button) {
    QDialogButtonBox::StandardButton standardButton =
        buttonBox->standardButton(button);
    if (standardButton == QDialogButtonBox::Apply ||
        standardButton == QDialogButtonBox::Ok) {
        if (pluginWidget_->asyncSave())
            pluginWidget_->setEnabled(false);
        if (standardButton == QDialogButtonBox::Ok) {
            closeAfterSave_ = true;
        }
        pluginWidget_->save();
        if (!pluginWidget_->asyncSave())
            saveFinished();
    } else if (standardButton == QDialogButtonBox::Close) {
        qApp->quit();
    } else if (standardButton == QDialogButtonBox::Reset) {
        pluginWidget_->load();
    }
}

void MainWindow::saveFinished() {
    if (proxy_) {
        // Pass some arbitrary thing.
        auto watcher = new QDBusPendingCallWatcher(
            proxy_->SetConfig(path_, QDBusVariant(0)), this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this,
                &MainWindow::saveFinishedPhase2);
    } else {
        saveFinishedPhase2(nullptr);
    }
}

void MainWindow::saveFinishedPhase2(QDBusPendingCallWatcher *watcher) {
    if (watcher) {
        watcher->deleteLater();
    }
    if (pluginWidget_->asyncSave()) {
        pluginWidget_->setEnabled(true);
    }
    if (!watcher || watcher->isError()) {
        QMessageBox::warning(
            this, _("Failed to notify Fcitx"),
            _("Failed to notify Fcitx about the configuration change."));
        closeAfterSave_ = false;
        return;
    }
    if (closeAfterSave_) {
        qApp->quit();
    }
}

void MainWindow::saveSubConfig(const QString &path) {
    if (proxy_) {
        // Pass some arbitrary thing.
        proxy_->SetConfig(path, QDBusVariant(0));
    }
}

void MainWindow::changed(bool changed) {
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(changed);
    buttonBox->button(QDialogButtonBox::Apply)->setEnabled(changed);
    buttonBox->button(QDialogButtonBox::Reset)->setEnabled(changed);
}

void MainWindow::setParentWindow(WId id) { wid_ = id; }

void MainWindow::showEvent(QShowEvent *event) {
    if (!wid_) {
        return;
    }
    setAttribute(Qt::WA_NativeWindow, true);
    QWindow *subWindow = windowHandle();
    Q_ASSERT(subWindow);

    QWindow *mainWindow = QWindow::fromWinId(wid_);
    wid_ = 0;
    if (!mainWindow) {
        // foreign windows not supported on all platforms
        return;
    }
    connect(this, &QObject::destroyed, mainWindow, &QObject::deleteLater);
    subWindow->setTransientParent(mainWindow);

    QDialog::showEvent(event);
}
} // namespace fcitx
