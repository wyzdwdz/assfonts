/*
 * SPDX-FileCopyrightText: 2012~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _DBUSADDONS_FCITXQTWATCHER_H_
#define _DBUSADDONS_FCITXQTWATCHER_H_

#include "fcitx5qt5dbusaddons_export.h"

#include <QDBusConnection>
#include <QObject>

namespace fcitx {

class FcitxQtWatcherPrivate;

class FCITX5QT5DBUSADDONS_EXPORT FcitxQtWatcher : public QObject {
    Q_OBJECT
public:
    explicit FcitxQtWatcher(QObject *parent = nullptr);
    explicit FcitxQtWatcher(const QDBusConnection &connection,
                            QObject *parent = nullptr);
    ~FcitxQtWatcher();
    void watch();
    void unwatch();
    void setConnection(const QDBusConnection &connection);
    QDBusConnection connection() const;
    void setWatchPortal(bool portal);
    bool watchPortal() const;
    bool isWatching() const;
    bool availability() const;

    QString serviceName() const;

Q_SIGNALS:
    void availabilityChanged(bool);

private Q_SLOTS:
    void imChanged(const QString &service, const QString &oldOwner,
                   const QString &newOwner);

private:
    void setAvailability(bool availability);
    void updateAvailability();

    FcitxQtWatcherPrivate *const d_ptr;
    Q_DECLARE_PRIVATE(FcitxQtWatcher);
};
} // namespace fcitx

#endif // _DBUSADDONS_FCITXQTWATCHER_H_
