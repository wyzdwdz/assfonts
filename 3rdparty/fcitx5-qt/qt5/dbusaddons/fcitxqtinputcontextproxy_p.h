/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#ifndef _DBUSADDONS_FCITXQTINPUTCONTEXTPROXY_P_H_
#define _DBUSADDONS_FCITXQTINPUTCONTEXTPROXY_P_H_

#include "fcitxqtinputcontextproxy.h"
#include "fcitxqtinputcontextproxyimpl.h"
#include "fcitxqtinputmethodproxy.h"
#include "fcitxqtwatcher.h"
#include <QDBusServiceWatcher>
#include <cstddef>

namespace fcitx {

class FcitxQtInputContextProxyPrivate {
public:
    FcitxQtInputContextProxyPrivate(FcitxQtWatcher *watcher,
                                    FcitxQtInputContextProxy *q)
        : q_ptr(q), fcitxWatcher_(watcher), watcher_(q) {
        registerFcitxQtDBusTypes();
        QObject::connect(fcitxWatcher_, &FcitxQtWatcher::availabilityChanged, q,
                         [this]() { availabilityChanged(); });
        watcher_.setWatchMode(QDBusServiceWatcher::WatchForUnregistration);
        QObject::connect(&watcher_, &QDBusServiceWatcher::serviceUnregistered,
                         q, [this]() {
                             cleanUp();
                             availabilityChanged();
                         });
        availabilityChanged();
    }

    ~FcitxQtInputContextProxyPrivate() {
        Q_Q(FcitxQtInputContextProxy);
        if (isValid()) {
            icproxy_->DestroyIC();
        }
        QObject::disconnect(
            q, &FcitxQtInputContextProxy::virtualKeyboardVisibilityChanged,
            nullptr, nullptr);
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
        delete introspectWatcher_;
        introspectWatcher_ = nullptr;
        delete queryWatcher_;
        queryWatcher_ = nullptr;
        setVirtualKeyboardVisible(false);
        supportInvokeAction_ = false;
    }

    void createInputContext() {
        Q_Q(FcitxQtInputContextProxy);
        if (!fcitxWatcher_->availability()) {
            return;
        }

        cleanUp();

        auto service = fcitxWatcher_->serviceName();
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
        improxy_ = new FcitxQtInputMethodProxy(
            owner, "/org/freedesktop/portal/inputmethod", connection, q);
        FcitxQtStringKeyValueList list;
        FcitxQtStringKeyValue arg;
        arg.setKey("program");
        arg.setValue(info.fileName());
        list << arg;
        if (!display_.isEmpty()) {
            FcitxQtStringKeyValue arg2;
            arg2.setKey("display");
            arg2.setValue(display_);
            list << arg2;
        }

        // Qt has good support for showing virtual keyboard, so we should
        // disable the default behavior supported by fcitx5
        FcitxQtStringKeyValue clientControlVirtualkeyboardShow;
        clientControlVirtualkeyboardShow.setKey(
            "clientControlVirtualkeyboardShow");
        clientControlVirtualkeyboardShow.setValue("true");
        list << clientControlVirtualkeyboardShow;
        // Qt has poor support for hiding virtual keyboard, so we should enable
        // the default behavior supported by fcitx5
        FcitxQtStringKeyValue clientControlVirtualkeyboardHide;
        clientControlVirtualkeyboardHide.setKey(
            "clientControlVirtualkeyboardHide");
        clientControlVirtualkeyboardHide.setValue("false");
        list << clientControlVirtualkeyboardHide;

        auto result = improxy_->CreateInputContext(list);
        createInputContextWatcher_ = new QDBusPendingCallWatcher(result);
        QObject::connect(createInputContextWatcher_,
                         &QDBusPendingCallWatcher::finished, q,
                         [this]() { createInputContextFinished(); });
    }

    void createInputContextFinished() {
        Q_Q(FcitxQtInputContextProxy);
        if (createInputContextWatcher_->isError()) {
            cleanUp();
            Q_EMIT q->inputContextCreationFailed();
            return;
        }

        QDBusPendingReply<QDBusObjectPath, QByteArray> reply(
            *createInputContextWatcher_);
        icproxy_ = new FcitxQtInputContextProxyImpl(improxy_->service(),
                                                    reply.value().path(),
                                                    improxy_->connection(), q);
        QObject::connect(icproxy_, &FcitxQtInputContextProxyImpl::CommitString,
                         q, &FcitxQtInputContextProxy::commitString);
        QObject::connect(icproxy_, &FcitxQtInputContextProxyImpl::CurrentIM, q,
                         &FcitxQtInputContextProxy::currentIM);
        QObject::connect(icproxy_,
                         &FcitxQtInputContextProxyImpl::DeleteSurroundingText,
                         q, &FcitxQtInputContextProxy::deleteSurroundingText);
        QObject::connect(icproxy_, &FcitxQtInputContextProxyImpl::ForwardKey, q,
                         &FcitxQtInputContextProxy::forwardKey);
        QObject::connect(icproxy_,
                         &FcitxQtInputContextProxyImpl::UpdateFormattedPreedit,
                         q, &FcitxQtInputContextProxy::updateFormattedPreedit);
        QObject::connect(icproxy_,
                         &FcitxQtInputContextProxyImpl::UpdateClientSideUI, q,
                         &FcitxQtInputContextProxy::updateClientSideUI);
        QObject::connect(icproxy_,
                         &FcitxQtInputContextProxyImpl::NotifyFocusOut, q,
                         &FcitxQtInputContextProxy::notifyFocusOut);
        QObject::connect(
            icproxy_,
            &FcitxQtInputContextProxyImpl::VirtualKeyboardVisibilityChanged, q,
            [this](bool visible) {
                if (queryWatcher_) {
                    queryWatcher_->deleteLater();
                    queryWatcher_ = nullptr;
                }
                setVirtualKeyboardVisible(visible);
            });

        delete createInputContextWatcher_;
        createInputContextWatcher_ = nullptr;
        Q_EMIT q->inputContextCreated(reply.argumentAt<1>());

        introspect();
        virtualKeyboardVisibilityQuery();
    }

    void introspect() {
        Q_Q(FcitxQtInputContextProxy);
        if (introspectWatcher_) {
            delete introspectWatcher_;
            introspectWatcher_ = nullptr;
        }
        QDBusMessage call = QDBusMessage::createMethodCall(
            icproxy_->service(), icproxy_->path(),
            "org.freedesktop.DBus.Introspectable", "Introspect");

        introspectWatcher_ = new QDBusPendingCallWatcher(
            fcitxWatcher_->connection().asyncCall(call));
        QObject::connect(introspectWatcher_, &QDBusPendingCallWatcher::finished,
                         q, [this]() { introspectFinished(); });
    }

    void introspectFinished() {
        if (introspectWatcher_->isFinished() &&
            !introspectWatcher_->isError()) {
            QDBusPendingReply<QString> reply = *introspectWatcher_;

            if (reply.value().contains("InvokeAction")) {
                supportInvokeAction_ = true;
            }
        }
        delete introspectWatcher_;
        introspectWatcher_ = nullptr;
    }

    void virtualKeyboardVisibilityQuery() {
        Q_Q(FcitxQtInputContextProxy);
        if (queryWatcher_) {
            delete queryWatcher_;
            queryWatcher_ = nullptr;
        }

        queryWatcher_ =
            new QDBusPendingCallWatcher(icproxy_->IsVirtualKeyboardVisible());
        QObject::connect(
            queryWatcher_, &QDBusPendingCallWatcher::finished, q,
            [this]() { virtualKeyboardVisibilityQueryFinished(); });
    }

    void virtualKeyboardVisibilityQueryFinished() {
        if (queryWatcher_ && queryWatcher_->isFinished() &&
            !queryWatcher_->isError()) {
            QDBusPendingReply<bool> reply = *queryWatcher_;
            setVirtualKeyboardVisible(reply.value());
        }
        delete queryWatcher_;
        queryWatcher_ = nullptr;
    }

    void setVirtualKeyboardVisible(bool visible) {
        Q_Q(FcitxQtInputContextProxy);
        if (isVirtualKeyboardVisible_ != visible) {
            isVirtualKeyboardVisible_ = visible;
            Q_EMIT q->virtualKeyboardVisibilityChanged(
                isVirtualKeyboardVisible_);
        }
    }

    FcitxQtInputContextProxy *q_ptr;
    Q_DECLARE_PUBLIC(FcitxQtInputContextProxy);

    FcitxQtWatcher *fcitxWatcher_;
    QDBusServiceWatcher watcher_;
    FcitxQtInputMethodProxy *improxy_ = nullptr;
    FcitxQtInputContextProxyImpl *icproxy_ = nullptr;
    bool isVirtualKeyboardVisible_ = false;
    bool supportInvokeAction_ = false;
    QDBusPendingCallWatcher *createInputContextWatcher_ = nullptr;
    QDBusPendingCallWatcher *introspectWatcher_ = nullptr;
    QDBusPendingCallWatcher *queryWatcher_ = nullptr;
    QString display_;
};
} // namespace fcitx

#endif // _DBUSADDONS_FCITXQTINPUTCONTEXTPROXY_P_H_
