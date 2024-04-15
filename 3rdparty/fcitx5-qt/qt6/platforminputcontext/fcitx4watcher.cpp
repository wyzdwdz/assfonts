/*
 * SPDX-FileCopyrightText: 2011~2023 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "fcitx4watcher.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>
#include <QDebug>
#include <QDir>
#include <QFileSystemWatcher>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

// utils function in fcitx-utils and fcitx-config
bool _pid_exists(pid_t pid) {
    if (pid <= 0)
        return 0;
    return !(kill(pid, 0) && (errno == ESRCH));
}

int displayNumber() {
    QByteArray display(qgetenv("DISPLAY"));
    QByteArray displayNumber("0");
    int pos = display.indexOf(':');

    if (pos >= 0) {
        ++pos;
        int pos2 = display.indexOf('.', pos);
        if (pos2 > 0) {
            displayNumber = display.mid(pos, pos2 - pos);
        } else {
            displayNumber = display.mid(pos);
        }
    }

    bool ok;
    int d = displayNumber.toInt(&ok);
    if (ok) {
        return d;
    }
    return 0;
}

QString socketFile() {
    QString filename =
        QString("%1-%2")
            .arg(QString::fromLatin1(QDBusConnection::localMachineId()))
            .arg(displayNumber());

    QString home = QString::fromLocal8Bit(qgetenv("XDG_CONFIG_HOME"));
    if (home.isEmpty()) {
        home = QDir::homePath().append(QLatin1String("/.config"));
    }
    return QString("%1/fcitx/dbus/%2").arg(home).arg(filename);
}

namespace fcitx {

QString newUniqueConnectionName() {
    static int idx = 0;
    const auto newIdx = idx++;
    return QString("_fcitx4_%1").arg(newIdx);
}

Fcitx4Watcher::Fcitx4Watcher(QDBusConnection sessionBus, QObject *parent)
    : QObject(parent), connection_(nullptr), sessionBus_(sessionBus),
      socketFile_(socketFile()),
      serviceName_(QString("org.fcitx.Fcitx-%1").arg(displayNumber())),
      availability_(false), uniqueConnectionName_(newUniqueConnectionName()) {}

Fcitx4Watcher::~Fcitx4Watcher() {
    cleanUpConnection();
    unwatchSocketFile();
}

bool Fcitx4Watcher::availability() const { return availability_; }

QDBusConnection Fcitx4Watcher::connection() const {
    if (connection_) {
        return *connection_;
    }
    return sessionBus_;
}

QString Fcitx4Watcher::service() const {
    if (connection_) {
        return serviceName_;
    }
    if (mainPresent_) {
        return serviceName_;
    }
    return QString();
}

void Fcitx4Watcher::setAvailability(bool availability) {
    if (availability_ != availability) {
        availability_ = availability;
        Q_EMIT availabilityChanged(availability_);
    }
}

void Fcitx4Watcher::watch() {
    if (watched_) {
        return;
    }

    serviceWatcher_ = new QDBusServiceWatcher(this);
    connect(serviceWatcher_, &QDBusServiceWatcher::serviceOwnerChanged, this,
            &Fcitx4Watcher::imChanged);
    serviceWatcher_->setConnection(sessionBus_);
    serviceWatcher_->addWatchedService(serviceName_);

    if (sessionBus_.interface()->isServiceRegistered(serviceName_)) {
        mainPresent_ = true;
    }

    watchSocketFile();
    createConnection();
    updateAvailability();
    watched_ = true;
}

void Fcitx4Watcher::unwatch() {
    if (!watched_) {
        return;
    }

    delete serviceWatcher_;
    serviceWatcher_ = nullptr;
    unwatchSocketFile();
    cleanUpConnection();
    mainPresent_ = false;
    watched_ = false;
    updateAvailability();
}

QString Fcitx4Watcher::address() {
    QString addr;
    QByteArray addrVar = qgetenv("FCITX_DBUS_ADDRESS");
    if (!addrVar.isNull())
        return QString::fromLocal8Bit(addrVar);

    QFile file(socketFile_);
    if (!file.open(QIODevice::ReadOnly))
        return QString();

    const int BUFSIZE = 1024;

    char buffer[BUFSIZE];
    size_t sz = file.read(buffer, BUFSIZE);
    file.close();
    if (sz == 0)
        return QString();
    char *p = buffer;
    while (*p)
        p++;
    size_t addrlen = p - buffer;
    if (sz != addrlen + 2 * sizeof(pid_t) + 1)
        return QString();

    /* skip '\0' */
    p++;
    pid_t *ppid = (pid_t *)p;
    pid_t daemonpid = ppid[0];
    pid_t fcitxpid = ppid[1];

    if (!_pid_exists(daemonpid) || !_pid_exists(fcitxpid))
        return QString();

    addr = QLatin1String(buffer);

    return addr;
}

void Fcitx4Watcher::cleanUpConnection() {
    QDBusConnection::disconnectFromBus(uniqueConnectionName_);
    delete connection_;
    connection_ = nullptr;
}

void Fcitx4Watcher::socketFileChanged() {
    cleanUpConnection();
    createConnection();
}

void Fcitx4Watcher::createConnection() {
    QString addr = address();
    if (!addr.isNull()) {
        QDBusConnection connection(
            QDBusConnection::connectToBus(addr, uniqueConnectionName_));
        if (connection.isConnected()) {
            connection_ = new QDBusConnection(connection);
        } else {
            QDBusConnection::disconnectFromBus(uniqueConnectionName_);
        }
    }

    if (connection_) {
        connection_->connect("org.freedesktop.DBus.Local",
                             "/org/freedesktop/DBus/Local",
                             "org.freedesktop.DBus.Local", "Disconnected", this,
                             SLOT(dbusDisconnected()));
        unwatchSocketFile();
    }
    updateAvailability();
}

void Fcitx4Watcher::dbusDisconnected() {
    cleanUpConnection();
    watchSocketFile();
    // Try recreation immediately to avoid race.
    createConnection();
}

void Fcitx4Watcher::watchSocketFile() {
    if (socketFile_.isEmpty()) {
        return;
    }
    QFileInfo info(socketFile_);
    QDir dir(info.path());
    if (!dir.exists()) {
        QDir rt(QDir::root());
        rt.mkpath(info.path());
    }
    fsWatcher_ = new QFileSystemWatcher(this);
    fsWatcher_->addPath(info.path());
    if (info.exists()) {
        fsWatcher_->addPath(info.filePath());
    }

    connect(fsWatcher_, &QFileSystemWatcher::fileChanged, this,
            &Fcitx4Watcher::socketFileChanged);
    connect(fsWatcher_, &QFileSystemWatcher::directoryChanged, this,
            &Fcitx4Watcher::socketFileChanged);
}

void Fcitx4Watcher::unwatchSocketFile() {
    if (fsWatcher_) {
        fsWatcher_->disconnect(this);
        fsWatcher_->deleteLater();
        fsWatcher_ = nullptr;
    }
}

void Fcitx4Watcher::imChanged(const QString &service, const QString &,
                              const QString &newOwner) {
    if (service == serviceName_) {
        if (!newOwner.isEmpty()) {
            mainPresent_ = true;
        } else {
            mainPresent_ = false;
        }
    }

    updateAvailability();
}

void Fcitx4Watcher::updateAvailability() {
    setAvailability(mainPresent_ || connection_);
}

} // namespace fcitx
