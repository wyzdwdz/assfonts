/*
 * SPDX-FileCopyrightText: 2011~2023 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef FCITXWATCHER_H_
#define FCITXWATCHER_H_

#include <QDBusConnection>
#include <QObject>

class QFileSystemWatcher;
class QDBusServiceWatcher;

namespace fcitx {

// A FcitxQtConnection replacement, to implement compatibility with fcitx 5.
// Since we have three thing to monitor, the situation becomes much more
// complexer.
class Fcitx4Watcher : public QObject {
    Q_OBJECT
public:
    explicit Fcitx4Watcher(QDBusConnection sessionBus,
                           QObject *parent = nullptr);
    ~Fcitx4Watcher();
    void watch();
    void unwatch();

    bool availability() const;

    QDBusConnection connection() const;
    QString service() const;

Q_SIGNALS:
    void availabilityChanged(bool);

private Q_SLOTS:
    void dbusDisconnected();
    void socketFileChanged();
    void imChanged(const QString &service, const QString &oldOwner,
                   const QString &newOwner);

private:
    QString address();
    void watchSocketFile();
    void unwatchSocketFile();
    void createConnection();
    void cleanUpConnection();
    void setAvailability(bool availability);
    void updateAvailability();

    QFileSystemWatcher *fsWatcher_ = nullptr;
    QDBusServiceWatcher *serviceWatcher_ = nullptr;
    QDBusConnection *connection_;
    QDBusConnection sessionBus_;
    QString socketFile_;
    QString serviceName_;
    bool availability_ = false;
    bool mainPresent_ = false;
    bool watched_ = false;
    QString uniqueConnectionName_;
};

} // namespace fcitx

#endif // FCITXWATCHER_H_
