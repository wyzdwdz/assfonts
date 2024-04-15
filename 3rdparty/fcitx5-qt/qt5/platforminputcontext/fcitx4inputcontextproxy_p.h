/*
 * SPDX-FileCopyrightText: 2012~2023 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#ifndef _FCITX4INPUTCONTEXTPROXY_P_H_
#define _FCITX4INPUTCONTEXTPROXY_P_H_

#include "fcitx4inputcontextproxy.h"
#include "fcitx4inputcontextproxyimpl.h"
#include "fcitx4inputmethodproxy.h"
#include "fcitx4watcher.h"
#include <QDBusServiceWatcher>
#include <cstddef>
#include <unistd.h>

namespace fcitx {

class Fcitx4InputContextProxyPrivate {
public:
    Fcitx4InputContextProxyPrivate(Fcitx4Watcher *watcher,
                                   Fcitx4InputContextProxy *q)
        : q_ptr(q), fcitxWatcher_(watcher), watcher_(q) {
        registerFcitxQtDBusTypes();
        QObject::connect(fcitxWatcher_, &Fcitx4Watcher::availabilityChanged, q,
                         [this]() { availabilityChanged(); });
        watcher_.setWatchMode(QDBusServiceWatcher::WatchForUnregistration);
        QObject::connect(&watcher_, &QDBusServiceWatcher::serviceUnregistered,
                         q, [this]() {
                             cleanUp();
                             availabilityChanged();
                         });
        availabilityChanged();
    }

    ~Fcitx4InputContextProxyPrivate() {
        if (isValid()) {
            icproxy_->DestroyIC();
        }
        cleanUp();
    }

    bool isValid() const { return (icproxy_ && icproxy_->isValid()); }

    void availabilityChanged() {
        QTimer::singleShot(100, q_ptr, [this]() { recheck(); });
    }

    void recheck() {
        if (!isValid() && fcitxWatcher_->availability()) {
            createInputContext();
        }
        if (!fcitxWatcher_->availability()) {
            cleanUp();
        }
    }

    void cleanUp() {
        auto services = watcher_.watchedServices();
        for (const auto &service : services) {
            watcher_.removeWatchedService(service);
        }

        delete improxy_;
        improxy_ = nullptr;
        delete icproxy_;
        icproxy_ = nullptr;
        delete createInputContextWatcher_;
        createInputContextWatcher_ = nullptr;
    }

    void createInputContext() {
        Q_Q(Fcitx4InputContextProxy);
        if (!fcitxWatcher_->availability()) {
            return;
        }

        cleanUp();

        auto service = fcitxWatcher_->service();
        auto connection = fcitxWatcher_->connection();

        auto owner = connection.interface()->serviceOwner(service);
        if (!owner.isValid()) {
            return;
        }

        watcher_.setConnection(connection);
        watcher_.setWatchedServices(QStringList() << owner);
        // Avoid race, query again.
        if (!connection.interface()->isServiceRegistered(owner)) {
            cleanUp();
            return;
        }

        QFileInfo info(QCoreApplication::applicationFilePath());
        improxy_ =
            new Fcitx4InputMethodProxy(owner, "/inputmethod", connection, q);

        auto result = improxy_->CreateICv3(info.fileName(), getpid());
        createInputContextWatcher_ = new QDBusPendingCallWatcher(result);
        QObject::connect(createInputContextWatcher_,
                         &QDBusPendingCallWatcher::finished, q,
                         [this]() { createInputContextFinished(); });
    }

    void createInputContextFinished() {
        Q_Q(Fcitx4InputContextProxy);
        if (createInputContextWatcher_->isError()) {
            cleanUp();
            return;
        }

        QDBusPendingReply<int, bool, unsigned int, unsigned int, unsigned int,
                          unsigned int>
            reply(*createInputContextWatcher_);

        QString path = QString("/inputcontext_%1").arg(reply.value());
        icproxy_ = new Fcitx4InputContextProxyImpl(improxy_->service(), path,
                                                   improxy_->connection(), q);
        QObject::connect(icproxy_, &Fcitx4InputContextProxyImpl::CommitString,
                         q, &Fcitx4InputContextProxy::commitString);
        QObject::connect(icproxy_, &Fcitx4InputContextProxyImpl::CurrentIM, q,
                         &Fcitx4InputContextProxy::currentIM);
        QObject::connect(icproxy_,
                         &Fcitx4InputContextProxyImpl::DeleteSurroundingText, q,
                         &Fcitx4InputContextProxy::deleteSurroundingText);
        QObject::connect(icproxy_, &Fcitx4InputContextProxyImpl::ForwardKey, q,
                         &Fcitx4InputContextProxy::forwardKey);
        QObject::connect(icproxy_,
                         &Fcitx4InputContextProxyImpl::UpdateFormattedPreedit,
                         q, &Fcitx4InputContextProxy::updateFormattedPreedit);

        delete createInputContextWatcher_;
        createInputContextWatcher_ = nullptr;
        Q_EMIT q->inputContextCreated();
    }

    Fcitx4InputContextProxy *q_ptr;
    Q_DECLARE_PUBLIC(Fcitx4InputContextProxy);

    Fcitx4Watcher *fcitxWatcher_;
    QDBusServiceWatcher watcher_;
    Fcitx4InputMethodProxy *improxy_ = nullptr;
    Fcitx4InputContextProxyImpl *icproxy_ = nullptr;
    QDBusPendingCallWatcher *createInputContextWatcher_ = nullptr;
    QString display_;
};
} // namespace fcitx

#endif // _FCITX4INPUTCONTEXTPROXY_P_H_
