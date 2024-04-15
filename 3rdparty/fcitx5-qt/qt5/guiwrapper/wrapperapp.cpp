/*
 * SPDX-FileCopyrightText: 2012~2012 CSSlayer <wengxt@gmail.com>
 * SPDX-FileCopyrightText: 2017~2017 xzhao <i@xuzhao.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <QDebug>

#include "fcitxqtconfiguifactory.h"
#include "mainwindow.h"
#include "wrapperapp.h"
#include <QCommandLineParser>
#include <QWindow>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/standardpath.h>

namespace fcitx {

WrapperApp::WrapperApp(int &argc, char **argv)
    : QApplication(argc, argv), factory_(new FcitxQtConfigUIFactory(this)),
      mainWindow_(0) {
    setApplicationName(QLatin1String(
        "fcitx5-qt" QT_STRINGIFY(QT_VERSION_MAJOR) "-gui-wrapper"));
    setApplicationVersion(QLatin1String(FCITX5_QT_VERSION));
    setOrganizationDomain("fcitx.org");
}

void WrapperApp::init() {
    QCommandLineParser parser;
    parser.setApplicationDescription(_("A launcher for Fcitx Gui plugin."));
    parser.addHelpOption();
    parser.addOptions({
        {{"w", "winid"}, _("Parent window ID"), _("winid")},
        {{"t", "test"}, _("Test if config exists")},
    });
    parser.addPositionalArgument(_("path"), _("Config path"));
    parser.process(*this);

    auto args = parser.positionalArguments();
    if (args.empty()) {
        qWarning("Missing path argument.");
        ::exit(1);
        return;
    }

    QString path = args[0];
    if (!path.startsWith("fcitx://config/addon/")) {
        path.prepend("fcitx://config/addon/");
    }
    if (parser.isSet("test")) {
        if (factory_->test(path)) {
            ::exit(0);
        } else {
            ::exit(1);
        }
    } else {
        WId winid = 0;
        bool ok = false;
        if (parser.isSet("winid")) {
            winid = parser.value("winid").toLong(&ok, 0);
        }
        FcitxQtConfigUIWidget *widget = factory_->create(path);
        if (!widget) {
            qWarning("Could not find plugin for file.");
            QMetaObject::invokeMethod(this, "errorExit", Qt::QueuedConnection);
            return;
        }
        mainWindow_ = new MainWindow(path, widget);
        if (ok && winid) {
            mainWindow_->setParentWindow(winid);
        }
        QMetaObject::invokeMethod(this, "run", Qt::QueuedConnection);
    }
}

void WrapperApp::run() {
    mainWindow_->exec();
    QMetaObject::invokeMethod(this, "quit", Qt::QueuedConnection);
}

WrapperApp::~WrapperApp() {
    if (mainWindow_) {
        delete mainWindow_;
    }
}

void WrapperApp::errorExit() { exit(1); }
} // namespace fcitx
